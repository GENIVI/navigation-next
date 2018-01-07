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
   @file        TileRefresher.cpp
   @defgroup    nbmap

   Description: Implementation of TileRefresher.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#include "TileRefresher.h"

extern "C"
{
#include "nbcontext.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbnetworkprotected.h"
#include "palclock.h"
#include "paltimer.h"
}
#include "StringUtility.h"
#include "TileImpl.h"

using namespace std;
using namespace nbmap;
using namespace nbcommon;

// Local Constants ..............................................................................

// 1 second equals to 1000 milliseconds
static const uint32 MILLISECONDS_PER_SECOND = 1000;

// Default refresh time.
static const uint32 DEFAULT_REFRESH_SECONDS = 20;

// Underscore used to append timestamp to the content ID of tile
static const char   UNDERSCORE[] = "_";

// Implementation of RefreshTileIndex

RefreshTileIndex::RefreshTileIndex(TileKeyPtr tileKey,
                                   shared_ptr <string> contentId,
                                   shared_ptr <string> cachingName)
        : m_tileKey(tileKey),
          m_contentId(contentId),
          m_cachingName(cachingName)
{
    // Nothing to do here.
}

RefreshTileIndex::~RefreshTileIndex()
{
    // Nothing to do here.
}


// Implementation of TileExpreListener.
TileExpireListener::TileExpireListener()
{
}

TileExpireListener::~TileExpireListener()
{
}

// Implementation of TileRefresher.
TileRefresher::TileRefresher(TileLayerInfoPtr info,
                             NB_Context* context)
{
    m_pContext        = context;
    m_tileLayerInfo   = info;
    m_timerActive     = false;

    SetRefreshInterval(DEFAULT_REFRESH_SECONDS);
}

TileRefresher::~TileRefresher()
{
    // Cancel the refresh timer if it is active.
    if (m_timerActive && m_pContext)
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (pal)
        {
            PAL_TimerCancel(pal, (PAL_TimerCallback*) RefreshTimerCallback, (void*) this);
            m_timerActive = false;
        }
    }

    // Need to clear all tiles of this layer from cache when the application starts.
    m_validTileIndexes.clear();
    m_expiredTileIndexes.clear();
}

void TileRefresher::RegisterListener(TileExpireListener* listener)
{
    m_listeners.insert(listener);
}

void TileRefresher::UnregisterListener(TileExpireListener* listener)
{
    m_listeners.erase(listener);

    // Cancel timer if no listener exists any more.
    if (m_listeners.empty())
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (pal)
        {
            PAL_TimerCancel(pal, (PAL_TimerCallback*) RefreshTimerCallback, (void*) this);
            m_timerActive = false;
        }
    }
}

NB_Error TileRefresher::AddTileToWatch(TilePtr tile)
{
    // Get the real tile key of this tile.
    TileKeyPtr tileKey = tile->GetTileKey();
    if (!tileKey)
    {
        return NE_INVAL;
    }

    // Get the content ID.
    shared_ptr<string> contentId = tile->GetContentID();
    if (!contentId)
    {
        return NE_INVAL;
    }

    // Check if this tile key already exists in expired tile indexes.
    TILE_REFRESH_MAP::iterator indexIterator = m_expiredTileIndexes.find(*tileKey);
    if (indexIterator != m_expiredTileIndexes.end())
    {
        RefreshTileIndexPtr index = indexIterator->second;
        if (index)
        {
            shared_ptr<string> savedContentId = index->m_contentId;
            if (savedContentId && (!(savedContentId->empty())))
            {
                // Use the content ID. Because it has been appended with timestamp before.
                *contentId = *savedContentId;
                return NE_OK;
            }
        }

        return NE_UNEXPECTED;
    }

    // Check if this tile key already exists in valid tile indexes.
    indexIterator = m_validTileIndexes.lower_bound(*tileKey);
    if ((indexIterator != m_validTileIndexes.end()) &&
        indexIterator->first == *tileKey)
    {
        RefreshTileIndexPtr index = indexIterator->second;
        if (index)
        {
            shared_ptr<string> savedContentId = index->m_contentId;
            if (savedContentId && (!(savedContentId->empty())))
            {
                // Use the content ID. Because it has been appended with timestamp before.
                *contentId = *savedContentId;
                return NE_OK;
            }
        }

        return NE_UNEXPECTED;
    }

    // @todo: we can avoid type casting if we are allowed to add GetCachingName() to Tile.h
    TileImpl* tileImpl = static_cast<TileImpl*>(tile.get());
    if (!tileImpl)
    {
        return NE_UNEXPECTED;
    }
    shared_ptr<string> cachingName = tileImpl->GetCachingName();
    if (!cachingName)
    {
        return NE_NOMEM;
    }

    /* Append the content ID with timestamp. Because NBGM saves tiles with content IDs
       in cache and does not update the tile data if content IDs are same. Even unload
       the tiles, unload operation only make the tiles not display and does not remove
       the tile data from NBGM cache.
    */

    // Format the timestamp to a string.

    contentId->append(UNDERSCORE + StringUtility::NumberToString(PAL_ClockGetGPSTime()));

    // Create a RefreshTileIndex object to be added.
    RefreshTileIndexPtr newIndex(new RefreshTileIndex(tileKey, contentId, cachingName));
    if (!newIndex)
    {
        return NE_NOMEM;
    }

    indexIterator = m_validTileIndexes.insert(indexIterator, make_pair(*tileKey, newIndex));
    if (indexIterator != m_validTileIndexes.end())
    {
        StartRefreshTimer();
        return NE_OK;
    }

    return NE_UNEXPECTED;
}

void TileRefresher::StartRefreshTimer()
{
    // Start the timer if it is inactive and there are tiles needed to refresh.
    if (m_pContext && (!m_timerActive))
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (pal)
        {
            m_timerActive = true;
            PAL_Error palError =
                    PAL_TimerSet(pal, (int32) (MILLISECONDS_PER_SECOND * m_secondsInterval),
                                 (PAL_TimerCallback*) RefreshTimerCallback, (void*) this);
            if (palError != PAL_Ok)
            {
                m_timerActive = false;
            }
        }
    }
}

void TileRefresher::RefreshTimerCallback(PAL_Instance* pal,
                                         TileRefresher* refresher,
                                         PAL_TimerCBReason reason)
{
    if (pal && refresher && (reason == PTCBR_TimerFired))
    {
        // Refresh expired tiles.
        refresher->CheckOutdatedTiles();

        // Set the flag of timer active to false.
        refresher->m_timerActive = false;

        // Start the timer again if needed.
        refresher->StartRefreshTimer();
    }
}

void TileRefresher::CheckOutdatedTiles()
{
    vector<TileKeyPtr> tilesToRefresh;

    CachePtr           cache;
    shared_ptr<string> type;
    if (m_tileLayerInfo)
    {
        cache = m_tileLayerInfo->cache;
        type  = m_tileLayerInfo->tileDataType;
    }

    if (!(m_expiredTileIndexes.empty()))
    {
        TILE_REFRESH_MAP::const_iterator indexIterator = m_expiredTileIndexes.begin();
        TILE_REFRESH_MAP::const_iterator indexEnd      = m_expiredTileIndexes.end();
        for (; indexIterator != indexEnd; ++indexIterator)
        {
            RefreshTileIndexPtr index = indexIterator->second;
            if (index)
            {
                // Remove the tile from cache.
                shared_ptr<string>& cachingName = index->m_cachingName;
                if (cache && cachingName && (!(cachingName->empty())))
                {
                    cache->RemoveData(type, cachingName);
                }

                // Add the tile key to refresh.
                TileKeyPtr tileKey = index->m_tileKey;
                if (tileKey)
                {
                    tilesToRefresh.push_back(tileKey);
                }
            }
        }
        // Remove all expired tile indexes.
        m_expiredTileIndexes.clear();
    }

    // Move all valid tile indexes to the vector contained expired tile indexes.
    m_expiredTileIndexes.swap(m_validTileIndexes);

    // Notify to update tiles.
    set<TileExpireListener*>::const_iterator listenerIterator = m_listeners.begin();
    set<TileExpireListener*>::const_iterator listenerEnd = m_listeners.end();
    for (; listenerIterator != listenerEnd; ++listenerIterator)
    {
        TileExpireListener* listener = *listenerIterator;
        if (listener)
        {
            listener->TilesExpired(tilesToRefresh);
        }
    }
}

/* See description in header file. */
void TileRefresher::SetRefreshInterval(uint32 secondsInterval)
{
    /*
      For the sake of efficiency, we use two vectors to hold valid tiles and expired tiles,
      and when ever a timer expired, the expired tiles will be returned to listeners, and
      the valid tiles would be moved into expired queue. This ensures each tile would only
      be valid for at most `secondsInterval` seconds.
    */
    m_secondsInterval = secondsInterval / 2;
    if (!m_secondsInterval)
    {
        m_secondsInterval = 1;
    }
}

/*! @} */
