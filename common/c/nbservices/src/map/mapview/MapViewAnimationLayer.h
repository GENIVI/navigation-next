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
    @file     MapViewAnimationLayer.h
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

#pragma once

#include "paltypes.h"
#include "base.h"
#include "Tile.h"
#include "smartpointer.h"
#include "nbgmtileanimationlayer.h"
#include <set>
#include <map>
#include <iostream>

using namespace std;

namespace nbmap
{

/*! Set animation parameters to default values. The application can overwrite these values using SetAnimationParameters() */
const uint32 DEFAULT_TOTAL_ANIMATION_TIME       = 6000;
const uint32 DEFAULT_LOOP_PAUSE                 = 750;
const double DEFAULT_MINIMUM_TILE_PERCENTAGE    = 65;

/*! Template functor to output smart pointer of strings 

    We could put this in a more common header file.

    Usage example:

    // Output all strings from a vector in a new line
    vector<shared_ptr<string> > myVector;
    OuputSharedPtrFunctor<shared_ptr<string> > functor(string("\r\n"));
    for_each(myVector.begin(), myVector.end(), functor);
*/
template<class T>
struct OuputSharedPtrFunctor
{
    OuputSharedPtrFunctor(const string& delimiter) : m_delimiter(delimiter){};

    void operator()(const T& element)
    {
        cout << element->c_str() << m_delimiter.c_str();
    }

    string m_delimiter;
};

/* Used for animation frame data.

    The pair consists of the current frame index for that animation layer and a vector of content-IDs for that frame.
*/
typedef pair<uint32, vector<shared_ptr<string> > > ANIMATION_DATA;

/* See MapViewInterface::SetAnimationLayerSettings() for description of the parameters */
class AnimationLayerParameters : public Base
{
public:
    AnimationLayerParameters()
    : m_totalAnimationTime(DEFAULT_TOTAL_ANIMATION_TIME),
      m_loopPause(DEFAULT_LOOP_PAUSE),
      m_minimumTilePercentage(DEFAULT_MINIMUM_TILE_PERCENTAGE){};

    AnimationLayerParameters(uint32 totalAnimationTime,
                             uint32 loopPause,
                             double minimumTilePercentage)
    : m_totalAnimationTime(totalAnimationTime),
    m_loopPause(loopPause),
    m_minimumTilePercentage(minimumTilePercentage){};

    uint32 m_totalAnimationTime;
    uint32 m_loopPause;
    double m_minimumTilePercentage;
};

/*! Animation layer used for overlay animations on the map.
 
    This class is used from the Frame class to handle all the animation logic. It handles the loading/unloading and drawing of
    the animated tiles to NBGM.
*/
class MapViewAnimationLayer
{
public:
    MapViewAnimationLayer();
    ~MapViewAnimationLayer();

    // Public functions ............................................................................................

    /* Set animation parameters for animation */
    void SetAnimationParameters(const AnimationLayerParameters& parameters);

    /*! Handle receiving of tile */
    void TileReceived(TilePtr tile);

    /*! Inform animation layer of removed tiles */
    void TilesRemoved(const vector<TilePtr>& tiles);

    /*! Update the timestamps for the frames */
    void UpdateFrameList(const vector<uint32>& frameList,           /*!< new list of timestamps (in GPS time) */
                         vector<shared_ptr<string> >& tilesToUnload /*!< On return the tiles (if any) to unload from NBGM */
                         );

    /*! Check if the animation layer has any tiles */
    bool HasAnyTiles() const;

    /*! Check if new frame data is available

        Call GetFrameData() if this function returns true.

        @param interval Time elapesed in milliseconds since last call

        @see GetFrameData
        @return 'true' if new data is available.
     */
    bool HasNewFrameData(int timeElapsed);

    /*! Returns the content-IDs of the tiles for the current frame to be displayed. 

        This function should only be called if HasNewFrameData() returns true.

        @see HasNewFrameData
        @return 'true' if we have any layer data, 'false' otherwise.
     */
    bool GetFrameData(ANIMATION_DATA& data,     /*!< On return: The frame index of the current frame and the associated contentIDs for that frame. */
                      uint32& timestamp,        /*!< On return: Timestamp of current frame index */
                      bool& animationCanPlay    /*!< Indicate if the animation can play */
                      );

    /*! Switch to the first frame.
    * @return 'true' if there are animating tiles or animating frame list.
    */
    bool SwitchToFirstFrame();

    /*! Switch to the next frame.
    * @return 'true' if there are animating tiles or animating frame list.
    */
    bool SwitchToNextFrame();

private:

    /*! Map of all tiles used for this animation layer.

        The outer map is sorted by tile keys. Each tile key has a map of frame tiles (the inner map)
        The inner map consists of timestamps (map-key) and contentIDs (map-value) for the frame-tiles.
     */
    typedef map<uint32, shared_ptr<string> > INNER_FRAME_TILE_MAP;
    typedef map<TileKey, INNER_FRAME_TILE_MAP> FRAME_TILE_MAP;
    FRAME_TILE_MAP m_frameTiles;

    // Private Functions .............................................................................................

    /*! Find a tile closest to the given timestamp 

        @return The content-ID of the tile, or an empty pointer if no tile was found.
     */
    shared_ptr<string> FindTile(const INNER_FRAME_TILE_MAP& tiles,      /*!< Map of tiles to search */
                                uint32 timestamp,                       /*!< Timestamp to use for search */
                                bool& animationCanPlay                  /*!< Indicate if the animation can play */
                                );
    
    
    // Private Members ...............................................................................................

    AnimationLayerParameters m_animationParameters;
    
    /*! Indicate if any tiles have changed for the animation */
    bool m_updated;

    /*! Time count, in milliseconds, used to determine if we need to switch to the next frame */
    uint32 m_timeCount;

    /*! Timestamps (in GPS time) of the current frames to animate */
    set<uint32> m_frameList;

    /*! This is the minimum frame count for each tile animation.

        It is a percentage of the total number of frames. It gets recalculated every time the frame-list gets updated.
        E.g. if we have 8 frames then this count could be 5, which would mean we animate a tile once we receive at least 5 frames for this tile.
     */
    uint32 m_minimumFrameCount;

    /*! The current displayed timestamp */
    uint32 m_frameTimestamp;

    /*! We could change the NBGM interface to take timestamps and remove this */
    int m_currentFrameIndex;

    /*! Animation frame data from last time. We only return data if it has changed from the last call */
    ANIMATION_DATA m_lastFrameData;
};

}

/*! @} */
