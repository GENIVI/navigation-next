/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------
    @file     MapViewAnimationLayer.cpp
 */
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/
/*! @{ */

#include "MapViewAnimationLayer.h"
#include <iterator>
#include <algorithm>
#include <set>

using namespace nbmap;

// MapViewAnimationLayer ..................................................................................................

MapViewAnimationLayer::MapViewAnimationLayer()
  :   m_updated(false),
      m_timeCount(0),
      m_minimumFrameCount(0),
      m_frameTimestamp(0),
      m_currentFrameIndex(0)
{
}

MapViewAnimationLayer::~MapViewAnimationLayer()
{
}

// Public functions ........................................................................................................

/* See header for description */
void
MapViewAnimationLayer::SetAnimationParameters(const AnimationLayerParameters& parameters)
{
    m_animationParameters = parameters;
}

/* See header for description */
void
MapViewAnimationLayer::TileReceived(TilePtr tile)
{
    if (!tile)
    {
        return;
    }

    // This only gets called for animation tiles, so always set the updated flag
    /*
        We could improve the logic in the future to only set flags for the frame that gets updated.
     */
    m_updated = true;

    INNER_FRAME_TILE_MAP newInnerMap;

    // Insert new outer-map, the map returns the existing entry if it already exists
    pair<FRAME_TILE_MAP::iterator, bool> outerMapResult = m_frameTiles.insert(make_pair(*tile->GetTileKey(), newInnerMap));

    // Actual reference to the inner map (either a new empty map or the existing one)
    INNER_FRAME_TILE_MAP& innerMap = outerMapResult.first->second;

    // Insert the content-ID to the inner map. The inner map is sorted by timestamps
    innerMap.insert(make_pair(tile->GetTimeStamp(), tile->GetContentID()));
}

/* See header for description */
void
MapViewAnimationLayer::TilesRemoved(const vector<TilePtr>& tiles)
{
    // For all tiles
    vector<TilePtr>::const_iterator tilesEnd = tiles.end();
    for (vector<TilePtr>::const_iterator tilesCurrent = tiles.begin(); tilesCurrent != tilesEnd; ++tilesCurrent)
    {
        const TileKey& key = *(*tilesCurrent)->GetTileKey();

        // Erase all animation tiles (content-IDs) associated with that tile-key
        if (m_frameTiles.erase(key) == 1)
        {
            m_updated = true;
        }
    }
}

/* See header for description */
void
MapViewAnimationLayer::UpdateFrameList(const vector<uint32>& frameList, vector<shared_ptr<string> >& tilesToUnload)
{
    if (frameList.empty())
    {
        return;
    }

    /* We could improve logic to update more efficient. For now just always set the updated flag */
    m_updated = true;

    /* Copy vector to set */
    m_frameList.clear();
    copy(frameList.begin(), frameList.end(), inserter(m_frameList, m_frameList.end()));

    /* The minimum frame count, is the minimum number of tiles needed before we display a tile. It is a percentage of the current frame list. */
    m_minimumFrameCount = (uint32)((double)m_frameList.size() * (m_animationParameters.m_minimumTilePercentage / 100.0) + 0.5);

    // If it is the initial frame list update then we start from the first frame. If we already have an animation active then
    // we just continue with our current timestamp.
    if (m_frameTimestamp == 0)
    {
        m_frameTimestamp = *m_frameList.begin();
        m_currentFrameIndex = 0;
    }
    
    if (! m_frameTiles.empty())
    {
        /*
            We have to throw out any old tiles with old timestamps
         */
        uint32 first = *m_frameList.begin();
        
        // For all tile keys
        /*
            :TRICKY: 
            removing elements from a maps is tricky, consult C++ documentation before modifying this code.
         */
        for (FRAME_TILE_MAP::iterator key = m_frameTiles.begin(); key != m_frameTiles.end(); ++key)
        {
            INNER_FRAME_TILE_MAP& tileMap = key->second;
            
            // For all tiles for that key
            for (INNER_FRAME_TILE_MAP::iterator tile = tileMap.begin(); tile != tileMap.end(); /* Don't increment here */)
            {
                // Remove all elements up to the first timestamp
                if (tile->first < first)
                {
                    // Add content-ID to returned vector
                    tilesToUnload.push_back(tile->second);
                    
                    // Remove the element. :TRICKY: Use post-iterator.
                    tileMap.erase(tile++);
                }
                else
                {
                    // Exit inner loop. We don't have to check any preceeding tiles, since the timestamps are ascending.
                    break;
                }
            }
        }
    }
}

/* See header for description */
bool
MapViewAnimationLayer::HasAnyTiles() const
{
    for (FRAME_TILE_MAP::const_iterator key = m_frameTiles.begin(); key != m_frameTiles.end(); ++key)
    {
        if (! key->second.empty())
        {
            return true;
        }
    }

    return false;
}

/* See header for description */
bool
MapViewAnimationLayer::HasNewFrameData(int timeElapsed)
{
    if (m_frameList.empty() || m_frameTiles.empty())
    {
        return false;
    }

    m_timeCount += timeElapsed;

    // Time for each frame
    uint32 millisecondToNextFrame = m_animationParameters.m_totalAnimationTime / m_frameList.size();

    // If we are at the end of the animation, then we add the loop-pause
    set<uint32>::iterator nextFrameIterator = m_frameList.lower_bound(m_frameTimestamp + 1);
    if (nextFrameIterator == m_frameList.end())
    {
        millisecondToNextFrame += m_animationParameters.m_loopPause;
    }

    // Do we need to advance to the next frame
    if (m_timeCount >= millisecondToNextFrame)
    {
        // Use modulo-operation so that we don't accumulate frames. If we get behind then we just wait for the next frame.
        m_timeCount = m_timeCount % millisecondToNextFrame;

        // Get next timestamp (if any)
        if (nextFrameIterator != m_frameList.end())
        {
            m_frameTimestamp = *nextFrameIterator;
            m_currentFrameIndex++;
        }
        else
        {
            // Start from the beginning again.
            m_frameTimestamp = *m_frameList.begin();
            m_currentFrameIndex = 0;
        }

        m_updated = true;
    }

    // This flag gets set if a new tile was received, or if we advance to the next frame (see above)
    if (m_updated)
    {
        m_updated = false;
        return true;
    }

    return false;
}

/* See header for description */
bool
MapViewAnimationLayer::GetFrameData(ANIMATION_DATA& data, uint32& timestamp, bool& animationCanPlay)
{
    /*
        We could improve the logic and check if a tile for the current (or next) frame has been updated and only proceed
        in that case. For now we just have an updated-flag and check each time we load/unload a new tile, even if it is
        from a frame that is not currently displayed.
    */
    if (m_frameList.empty() || m_frameTiles.empty())
    {
        return false;
    }

    /*
        We could change the NBGM interfaces to use timestamps instead of frame indices.
    */
    // For all tile keys
    bool animationStatus = false;
    FRAME_TILE_MAP::const_iterator keyEnd = m_frameTiles.end();
    for (FRAME_TILE_MAP::const_iterator key = m_frameTiles.begin(); key != keyEnd; ++key)
    {
        // Find the closest tile (if any) to display in this frame
        shared_ptr<string> contentID = FindTile(key->second, m_frameTimestamp, animationStatus);
        // At least for 1 tile client downloads 65% of the frames, animation can play.
        if (animationStatus == true)
        {
            animationCanPlay = true;
        }
        if (contentID)
        {
            // Add the content ID to the returned vector
            data.second.push_back(contentID);
        }
    }

    if (!animationCanPlay)
    {
        // Start from the beginning again.
        m_frameTimestamp = *m_frameList.begin();
        m_currentFrameIndex = 0;
        m_timeCount = 0;
    }
    timestamp = m_frameTimestamp;
    data.first = m_currentFrameIndex;

    // Did the frame data change from last time
    if (data != m_lastFrameData)
    {
        // Save for next time
        m_lastFrameData = data;
        return true;
    }

    return false;
}

// Private Functions .................................................................................................................

/* See header for description */
shared_ptr<string>
MapViewAnimationLayer::FindTile(const INNER_FRAME_TILE_MAP& tiles, uint32 timestamp, bool& animationCanPlay)
{
    // Check if we have enough frames for that tile key. We only display a tile if we have a minimum number of frames
    if (tiles.size() < m_minimumFrameCount)
    {
        //return the tile of first frame
        INNER_FRAME_TILE_MAP::const_iterator firstTile = tiles.find(*m_frameList.begin());
        if (firstTile != tiles.end())
        {
            return firstTile->second;
        }
        // return empty pointer
        return shared_ptr<string>();
    }

    animationCanPlay = true;

    // Find first tile that is equal or greater than the timestamp
    INNER_FRAME_TILE_MAP::const_iterator tile = tiles.lower_bound(timestamp);
    if (tile != tiles.end())
    {
        // If it is the first frame, or it matches the timestamp, use it
        if ((tile == tiles.begin()) || (tile->first == timestamp))
        {
            return tile->second;
        }

        INNER_FRAME_TILE_MAP::const_iterator previous(tile);
        --previous;
        
        /* Return the tile that is closed in time to the timestamp we're searching for */
        if ((timestamp - previous->first) < (tile->first - timestamp))
        {
            return previous->second;
        }
        else
        {
            return tile->second;
        }
    }
    else
    {
        // Return the last tile
        return tiles.rbegin()->second;
    }

    // This is never reached.
}

/* See header for description */
bool
MapViewAnimationLayer::SwitchToFirstFrame()
{
    if (m_frameList.empty() || m_frameTiles.empty())
    {
        return false;
    }

    m_timeCount = 0;
    m_frameTimestamp = *m_frameList.begin();
    m_currentFrameIndex = 0;
    m_updated = true;

    return true;
}

/* See header for description */
bool
MapViewAnimationLayer::SwitchToNextFrame()
{
    if (m_frameList.empty() || m_frameTiles.empty())
    {
        return false;
    }

    set<uint32>::iterator nextFrameIterator = m_frameList.lower_bound(m_frameTimestamp + 1);
    if(nextFrameIterator == m_frameList.end())
    {
        m_frameTimestamp = *m_frameList.begin();
        m_currentFrameIndex = 0;
    }
    else
    {
        m_frameTimestamp = *nextFrameIterator;
        m_currentFrameIndex++;
    }

    m_updated = true;
    m_timeCount = 0;

    return true;
}

/*! @} */
