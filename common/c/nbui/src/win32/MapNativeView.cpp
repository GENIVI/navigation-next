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
    @file     MapNativeView.cpp
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "MapNativeView.h"
#include "NBGMMapNativeView.h"
#include "NBGMRenderContextImpl.h"
#include "MapViewUIInterface.h"

using namespace nbmap;

MapNativeView::MapNativeView(MapViewUIInterface* mapView, int zorder, bool thirdPartyUsage)
 :  m_pMapView(mapView),
    m_pNativeHostView(NULL),
    m_renderInterface(NULL),
    m_width(0),
    m_height(0),
    m_lastTimeval(0),

    m_pDebugView(NULL),
    m_enableDebugInformation(false),
    m_zorder(zorder),

    m_pLocationBubble(NULL),
    m_pMapLegendView(NULL),
    m_pTipView(NULL),
    m_thirdPartyUsage(thirdPartyUsage)
{
    // Create the native view
    NBGMMapNativeView* pNBGMMapNativeView = new NBGMMapNativeView(mapView, this, zorder);
    if (pNBGMMapNativeView)
    {
        // Create the NBGM Render Context
        // Why do we need the create function? We could just create it here?
        m_nbgmRenderContext = shared_ptr<NBGM_RenderContext>(pNBGMMapNativeView->CreateNativeRenderContext());

        // Keep the pointer to the native view
        // The pointer is a void pointer to make the interface platform independent.
        m_pNativeHostView = (void*)pNBGMMapNativeView;
    }
}

MapNativeView::~MapNativeView()
{
}
 
/* See description in header file */
NB_Error 
MapNativeView::Initialize(int& width, int& height, PAL_Instance* pal)
{
    if (! m_pNativeHostView)
    {
        return NE_NOMEM;
    }

    return ((NBGMMapNativeView*)m_pNativeHostView)->Initialize();
}

/* See description in header file */
void
MapNativeView::Finallize()
{
    if (! m_pNativeHostView)
    {
        return;
    }

    ((NBGMMapNativeView*)m_pNativeHostView)->Finallize();
}

/* See description in header file */
NB_Error
MapNativeView::ShowView()
{
    return NE_OK;
}

/* See description in header file */
NB_Error 
MapNativeView::SetMapViewProtocol(void* protocol)
{
    // @TODO
    m_pMapView->UI_MapNativeViewCreated();
    return NE_OK;
}

/* See description in header file */
void*
MapNativeView::GetNativeContext()
{
    // @todo

    return NULL;
}

/* See description in header file */
void 
MapNativeView::ShowPinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation)
{
}

/* See description in header file */
void 
MapNativeView::HidePinBubble(shared_ptr<nbmap::Pin> pin)
{
}

/* See description in header file */
void 
MapNativeView::UpdatePinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation)
{
}

/* See description in header file */
void 
MapNativeView::RemovePinBubble(shared_ptr<nbmap::Pin> pin)
{
}

/* See description in header file */
void 
MapNativeView::ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend)
{
}

/* See description in header file */
void 
MapNativeView::HideMapLegend()
{
}

/* See description in header file */
void 
MapNativeView::UpdateTrafficTip(TrafficTipType type)
{
}

/* See description in header file. */
void
MapNativeView::UpdateAnimationTip(AnimationTipType type)
{
}

/* See description in header file */
void 
MapNativeView::CreateTimestampLabel()
{
}

/* See description in header file */
void 
MapNativeView::UpdateTimestampLabel(uint32 gpsTime)
{
}

/* See description in header file */
void 
MapNativeView::RemoveTimestampLabel()
{
}

/* See description in header file. */
void
MapNativeView::UpdateButtonState(bool animationCanPlay)
{
}

/* See description in header file */
void 
MapNativeView::ShowLocationBubble(float x, float y, bool orientation, double latitude, double longitude)
{
}

/* See description in header file */
void 
MapNativeView::HideLocationBubble()
{
}

/* See description in header file */
void 
MapNativeView::UpdateLocationBubble(float x, float y, bool orientation, double latitude, double longitude)
{
}

/* See description in header file */
shared_ptr<NBGM_RenderContext> 
MapNativeView::GetRenderContext() const
{
    return m_nbgmRenderContext;
}
    
/* See description in header file */
void 
MapNativeView::UpdateHostView()
{
    if (! m_pNativeHostView)
    {
        return;
    }

    ((NBGMMapNativeView*)m_pNativeHostView)->Render();
}

/* See description in header file */
void 
MapNativeView::EnableDebugInfo(bool isEnabled)
{
}

/* See description in header file */
void 
MapNativeView::SetDebugInfo(const char* text)
{
}

/* See description in header file */
void 
MapNativeView::setZorder(int zorder)
{
}

/* See description in header file */
void 
MapNativeView::SetViewSize(int width, int height)
{
}

/* See description in header file */
void 
MapNativeView::resetScreen()
{
}

/* See description in header file */
void 
MapNativeView::CreateFontBuffer(PAL_Instance* pal, MapFontType fontType, uint8** buffer, uint32& bufferSize)
{
}

/* See description in header file */
void 
MapNativeView::UpdateTilt(float tilt)
{
}

/* See description in header file */
void 
MapNativeView::RenderingAllowedNotified()
{
}

/* See description in header file */
void
MapNativeView::OnMapInitialized()
{
}

void MapNativeView::UpdateSurface()
{
}

void MapNativeView::DestroySurface()
{
}

void MapNativeView::SetGpsMode(NB_GpsMode mode)
{
}
/*! @} */
