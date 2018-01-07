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
   @file         GeographicLayer.cpp
   @defgroup     nbmap
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#include "GeographicLayer.h"
#include "StringUtility.h"
#include "nbmacros.h"

using namespace nbmap;

//@todo: move this value to a common place.
static const uint32 GEOGRAPHIC_LAYER_DRAW_ORDER = 160;
static const uint32 GEOGRAPHIC_LAYER_REFERENCE_TILE_GRID_LEVEL = 12;

// Local classes .........................................................................
class EventTask_GeographicUpdateParam
{
public:
    EventTask_GeographicUpdateParam(GeographicLayer& layer,
                                    Geographic* graphic,
                                    shared_ptr<bool> isValid)
            : m_layer(layer),
              m_graphic(graphic),
              m_isValid(isValid) {}
    virtual ~EventTask_GeographicUpdateParam(){}
    GeographicLayer& m_layer;
    Geographic*      m_graphic;
    shared_ptr<bool> m_isValid;
private:
    NB_DISABLE_COPY(EventTask_GeographicUpdateParam);
};

class EventTask_GeographicToggledParam
{
public:
    EventTask_GeographicToggledParam(GeographicLayer& layer,
                                     Geographic* graphic,
                                     bool enabled,
                                     shared_ptr<bool> isValid)
            : m_layer(layer),
              m_graphic(graphic),
              m_isEnabled(enabled),
              m_isValid(isValid) {}
    virtual ~EventTask_GeographicToggledParam(){}
    GeographicLayer& m_layer;
    Geographic*      m_graphic;
    bool             m_isEnabled;
    shared_ptr<bool> m_isValid;
private:
    NB_DISABLE_COPY(EventTask_GeographicToggledParam);
};

// Implementation of GeographicLayer.

/* See description in header file. */
GeographicLayer::GeographicLayer(shared_ptr<string> layerId,
                                 uint32 layerIdDigital,
                                 NB_Context* context,
                                 GeographicLayerListener* listener)
        : UnifiedLayer(TileManagerPtr(), layerIdDigital, context),
          m_listener(listener),
          m_currentSubDraworder(0),
          m_isValid(new bool(true))
{
    //@todo: Add memory cache if needed.
    if (m_tileLayerInfo)
    {
        layerId->append(string("_") +  nbcommon::StringUtility::NumberToString(layerIdDigital));
        m_tileLayerInfo->drawOrder        = GEOGRAPHIC_LAYER_DRAW_ORDER;
        m_tileLayerInfo->tileDataType     = layerId;
        m_tileLayerInfo->refTileGridLevel = GEOGRAPHIC_LAYER_REFERENCE_TILE_GRID_LEVEL;
        m_tileLayerInfo->materialCategory.reset(CCC_NEW string(*layerId));
    }

    m_isOverlay     = false;
    m_isRasterLayer = false;

    // Update characteristics.
    SetCharacteristics(TILE_ADDITIONAL_KEY_OPTIONAL, "");
    if (layerId)
    {
        SetCharacteristics(TILE_ADDITIONAL_KEY_NAME, *layerId);
    }
    ProcessCharacteristics();

    // Set this layer as enabled by default!
    SetEnabled(true);
}

/* See description in header file. */
GeographicLayer::~GeographicLayer()
{
    if (m_isValid)
    {
        *m_isValid = false;
    }
}

/* See description in header file. */
void GeographicLayer::GraphicUpdated(Geographic* graphic)
{
    EventTask_GeographicUpdateParam* param =
            CCC_NEW EventTask_GeographicUpdateParam(*this, graphic, m_isValid);
    if (param)
    {
        uint32 taskId   = 0;
        PAL_Error error =
                PAL_EventTaskQueueAdd(NB_ContextGetPal(m_pContext),
                                      &GeographicLayer::CCC_UpdateGraphic, param,
                                      &taskId);
        if (error != PAL_Ok)
        {
            delete param;
        }
    }
}

/* See description in header file. */
void GeographicLayer::UnregisterListener()
{
    m_listener = NULL;
}

/* See description in header file. */
void GeographicLayer::CCC_UpdateGraphic(PAL_Instance* /*pal*/, void* userData)
{
    EventTask_GeographicUpdateParam* param =
            static_cast<EventTask_GeographicUpdateParam*>(userData);
    if (!param)
    {
        return;
    }

    if (param->m_isValid && *param->m_isValid)
    {
        GeographicUpdateType result =
                param->m_layer.ProcessUpdatedGraphic(param->m_graphic);

        if (param->m_layer.m_listener)
        {
            param->m_layer.m_listener->GeographicUpdated(param->m_graphic, result);
        }

        param->m_layer.PostGraphicUpdated(param->m_graphic);
    }

    delete param;
}

/*! Derived class should overwrite this function to do necessary updates. */
void GeographicLayer::PostGraphicUpdated(Geographic* /*graphic*/)
{
}

/* See description in header file. */
void GeographicLayer::GraphicEnabled(Geographic* graphic, bool enabled)
{
    EventTask_GeographicToggledParam* param =
            CCC_NEW EventTask_GeographicToggledParam(*this, graphic, enabled, m_isValid);
    if (param)
    {
        uint32 taskId   = 0;
        PAL_Error error =
                PAL_EventTaskQueueAdd(NB_ContextGetPal(m_pContext),
                                      &GeographicLayer::CCC_ToggleGraphic, param,
                                      &taskId);
        if (error != PAL_Ok)
        {
            delete param;
        }
    }
}

/* See description in header file. */
void GeographicLayer::CCC_ToggleGraphic(PAL_Instance* /*pal*/, void* userData)
{
    EventTask_GeographicToggledParam* param =
            static_cast<EventTask_GeographicToggledParam*>(userData);
    if (!param)
    {
        return;
    }

    if (param->m_isValid && *param->m_isValid)
    {
        param->m_layer.SetEnabled(param->m_isEnabled);

        if ( param->m_layer.m_listener)
        {
            param->m_layer.m_listener->GeographicToggled(param->m_graphic);
        }
    }

    delete param;
}



/*! @} */
