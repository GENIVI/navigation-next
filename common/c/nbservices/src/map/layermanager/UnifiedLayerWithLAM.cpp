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
   @file        UnifiedLayerWithLAM.cpp
   @defgroup    nbmap

   Description: Implementation of UnifiedLayerWithLAM.

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

#include "UnifiedLayerWithLAM.h"
#include "LayerAvailabilityMatrixLayer.h"

using namespace nbmap;

/* See description in header file. */
UnifiedLayerWithLAM::UnifiedLayerWithLAM(TileManagerPtr tileManager,
                                         uint32 layerIdDigital,
                                         NB_Context* context,
                                         const shared_ptr<UnifiedLayer>* lamLayer)
        : UnifiedLayer(tileManager, layerIdDigital, context),
          m_pLamLayer(lamLayer),
          m_isValid(new bool(true))
{
}


/* See description in header file. */
UnifiedLayerWithLAM::UnifiedLayerWithLAM(const UnifiedLayerWithLAM& layer)
        : UnifiedLayer(layer),
          m_pLamLayer(layer.m_pLamLayer),
          m_isValid(new bool(true))
{
}

/* See description in header file. */
UnifiedLayerWithLAM::~UnifiedLayerWithLAM()
{
    *m_isValid = false;
}

/* See description in header file. */
void UnifiedLayerWithLAM::GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                                  AsyncTileRequestWithRequestPtr callback,
                                  uint32 priority, bool /*cachedOnly*/)
{
    //@note: currently, parameter cachedOnly should never be applied to vector tiles.
    LayerAvailabilityMatrixLayer* lamLayer = NULL;
    if (m_pLamLayer && *m_pLamLayer && m_isValid && *m_isValid &&
        ((lamLayer = static_cast<LayerAvailabilityMatrixLayer*>(m_pLamLayer->get()))) != NULL)
    {
        TileRequestEntry entry(tileKey, convertedTileKey, this, callback, priority, m_isValid);
        lamLayer->GetTileWithLamOptimized(entry);
    }
    else if (callback)
    {
        callback->Error(tileKey, NE_NOENT);
    }
}

/* See description in header file. */
void
UnifiedLayerWithLAM::StartRealDownload(TileKeyPtr tileKey,
                                       TileKeyPtr convertedTileKey,
                                       AsyncTileRequestWithRequestPtr callback,
                                       uint32 priority)
{
    UnifiedLayer::GetTile(tileKey, convertedTileKey, callback, priority);
}

/* See description in header file. */
shared_ptr <UnifiedLayer> UnifiedLayerWithLAM::Clone()
{
    return CloneWithTemplate<UnifiedLayerWithLAM>(this);
}

/*! @} */
