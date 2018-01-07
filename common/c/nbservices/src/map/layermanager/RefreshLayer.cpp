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

    @file       RefreshLayer.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

extern "C"
{
#include "palclock.h"
#include "paltimer.h"
#include "nbcontextprotected.h"
}

#include "RefreshLayer.h"
#include "RefreshTileCallback.h"
#include "StringUtility.h"
#include <sstream>

/*! @{ */

using namespace nbcommon;
using namespace nbmap;
using namespace std;

// Local Constants ..............................................................................

// RefreshLayerListener Public functions ........................................................

/* See header file for description */
RefreshLayerListener::~RefreshLayerListener()
{
    // Nothing to do here.
}


// RefreshLayer Public functions ................................................................

/* See header file for description */
RefreshLayer::RefreshLayer(TileManagerPtr tileManager,
                           uint32 layerIdDigital,
                           NB_Context* context)
        : UnifiedLayer(tileManager, layerIdDigital, context),
          m_tileRefresher(new TileRefresher(m_tileLayerInfo, context)),
          m_thisValid(new bool(true))
{
    m_createdByOffboardLayerProvider = true;
    m_isOverlay                      = true;
}

/* See header file for description */
RefreshLayer::RefreshLayer(TileManagerPtr tileManager,
                           uint32 layerIdDigital,
                           uint32 secondsInterval,
                           NB_Context* context)
        : UnifiedLayer(tileManager, layerIdDigital, context),
          m_tileRefresher(new TileRefresher(m_tileLayerInfo, context)),
          m_thisValid(new bool(true))
{
    m_createdByOffboardLayerProvider = true;
    m_isOverlay                      = true;
    m_tileRefresher->SetRefreshInterval(secondsInterval);
}

RefreshLayer::RefreshLayer(const RefreshLayer& layer)
        : UnifiedLayer(layer),
          m_tileRefresher(layer.m_tileRefresher), // use same refresher.
          m_thisValid(new bool (true)),
          m_createdByOffboardLayerProvider(false)
{
    m_tileRefresher->RegisterListener(this);
    m_isOverlay = layer.m_isOverlay;
}

/* See header file for description */
RefreshLayer::~RefreshLayer()
{
    m_tileRefresher->UnregisterListener(this);
    // Set the valid flag to false.
    if (m_thisValid)
    {
        *m_thisValid = false;
    }
}

/* See header file for description */
UnifiedLayerPtr RefreshLayer::Clone()
{
    return CloneWithTemplate<RefreshLayer>(this);
}

/* See header file for description */
void
RefreshLayer::RegisterListener(RefreshLayerListener* listener)
{
    if (listener)
    {
        m_listeners.insert(listener);
    }
}

/* See header file for description */
void
RefreshLayer::UnregisterListener(RefreshLayerListener* listener)
{
    if (listener)
    {
        m_listeners.erase(listener);
    }
}

/* See header file for description */
NB_Error
RefreshLayer::AddValidTile(TilePtr tile)
{
    if (!tile)
    {
        return NE_INVAL;
    }

    if (!m_tileRefresher)
    {
        return NE_NOTINIT;
    }

    return m_tileRefresher->AddTileToWatch(tile);
}

/* See description in Layer.h */
void
RefreshLayer::GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                      AsyncTileRequestWithRequestPtr callback,
                      uint32 priority, bool cachedOnly)
{
    if (!IsEnabled())
    {
        return;
    }

    // Create a callback to refresh tiles.
    RefreshTileCallbackPtr refreshTileCallback(new RefreshTileCallback(callback, m_thisValid, this));
    if (!refreshTileCallback)
    {
        if (callback)
        {
            callback->Error(tileKey, NE_NOMEM);
        }
        return;
    }

    // Call the function GetTile of base class.
    UnifiedLayer::GetTile(tileKey, convertedTileKey, refreshTileCallback, priority, cachedOnly);
}

/* See header file for description */
bool RefreshLayer::IsCreatedByOffboardLayerManager()
{
    return m_createdByOffboardLayerProvider;
}


// RefreshLayer Private functions ...............................................................

/*! Refresh expired tiles

    @return None
*/
void
RefreshLayer::RefreshTiles()
{
}

void RefreshLayer::TilesExpired(const vector<TileKeyPtr>& tileKeys)
{
    if (IsEnabled())
    {
        set<RefreshLayerListener*>::const_iterator listenerIterator = m_listeners.begin();
        set<RefreshLayerListener*>::const_iterator listenerEnd = m_listeners.end();
        for (; listenerIterator != listenerEnd; ++listenerIterator)
        {
            RefreshLayerListener* listener = *listenerIterator;
            if (listener)
            {
                listener->RefreshTiles(tileKeys, this);
            }
        }
    }
}

/* See description in header file. */
void RefreshLayer::ProcessSpecialCharacteristicsOfLayer()
{
    shared_ptr<string> value = GetCharacteristics(TILE_ADDITIONAL_KEY_REFRESH_INTERVAL);
    if (value && !value->empty())
    {
        uint32 secondsInterval = 0;
        nbcommon::StringUtility::StringToNumber(*value, secondsInterval);

        if (secondsInterval && m_tileRefresher)
        {
            m_tileRefresher->SetRefreshInterval(secondsInterval);
        }
    }
}


/*! @} */
