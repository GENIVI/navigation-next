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
   @file        CommonMaterialLayer.cpp
   @defgroup    nbmap

   Description:

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
#include "CommonMaterialLayer.h"
#include "nbmacros.h"
extern "C"
{
#include "nbcontextprotected.h"
#include "cslqalog.h"
#include "nbqalog.h"
#include "palclock.h"
}

using namespace std;
using namespace nbmap;

#define DUMMY_VALUE       0xFFFFFFFF


static const string COMMON_MATERIAL_DEFAULT_KEY("-1");

CommonMaterialLayer::CommonMaterialLayer(TileManagerPtr tileManagerPtr,
                                         uint32 layerIdDigital,
                                         NB_Context* context)
        : UnifiedLayer(tileManagerPtr, layerIdDigital, context)
{
    SetDisplayable(false);
    m_templateParameters = shared_ptr<map<string, string> >(new map<string, string>);
    if (m_templateParameters)
    {
        m_templateParameters->insert(pair<string, string>(PARAMETER_KEY_TILE_X,
                                                          COMMON_MATERIAL_DEFAULT_KEY));
        m_templateParameters->insert(pair<string, string>(PARAMETER_KEY_TILE_Y,
                                                          COMMON_MATERIAL_DEFAULT_KEY));
        m_templateParameters->insert(pair<string, string>(PARAMETER_KEY_ZOOM_LEVEL,
                                                          COMMON_MATERIAL_DEFAULT_KEY));
    }
    m_requestedTileKey = TileKeyPtr(new TileKey((int)DUMMY_VALUE,
                                                (int)DUMMY_VALUE,
                                                (int)m_tileLayerInfo->layerID));
    m_dataTileKey = TileKeyPtr(new TileKey((int)DUMMY_VALUE,
                                           (int)DUMMY_VALUE,
                                           DUMMY_VALUE));
}

CommonMaterialLayer::~CommonMaterialLayer()
{
}

void
CommonMaterialLayer::GetTile(TileKeyPtr /*tileKey*/, TileKeyPtr /*convertedTileKey*/,
                             AsyncTileRequestWithRequestPtr /*callback*/,
                             uint32 /*priority*/, bool /*cachedOnly*/)
{
    // Do not support, please call GetMaterial instead!
}

void
CommonMaterialLayer::GetMaterial(shared_ptr <AsyncCallbackWithRequest<TileKeyPtr,TilePtr> > callback,
                                 uint32 priority)
{

    if (!IsEnabled()) // do not report error if layer is not enabled.
    {
        return;
    }

    NB_Error error = NE_OK;
    do
    {
        if (!m_tileManager || !m_requestedTileKey || !m_dataTileKey)
        {
            error = NE_NOTINIT;
            break;
        }

        TileRequestPtr request(new TileRequest(m_requestedTileKey, m_dataTileKey, callback));
        if (request && m_pCallback)
        {
            m_pCallback->StoreTileRequest(request);

            // QaLog
            if (CSL_QaLogIsVerboseLoggingEnabled(NB_ContextGetQaLog(m_pContext)))
            {
                NB_QaLogTileRequest(m_pContext, DUMMY_VALUE, DUMMY_VALUE, DUMMY_VALUE);
            }

            // Request a tile by the tile manager.
            m_tileManager->GetTile(m_templateParameters,
                                   m_pCallback,
                                   priority);
        }
        else
        {
            error = NE_NOMEM;
        }
    } while (0);

    if (callback && error != NE_OK)
    {
        callback->Error(m_requestedTileKey, error);
    }
}

/* See description in header file. */
void CommonMaterialLayer::ProcessSpecialCharacteristicsOfLayer()
{
    m_tileLayerInfo->materialType     = GetCharacteristics(TILE_ADDITIONAL_KEY_TOD_MODE);
    shared_ptr<string> value = GetCharacteristics(MATERIAL_TYPE);
    if (value)
    {
        m_tileLayerInfo->materialCategory.reset(CCC_NEW string(*value));
    }

    m_tileLayerInfo->materialBaseType = GetCharacteristics(MATERIAL_BASE_TYPE);
    m_tileLayerInfo->materialThemeType = GetCharacteristics(MATERIAL_THEME_TYPE);
}

/*! @} */
