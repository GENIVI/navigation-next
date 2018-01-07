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
/*
#include <bps/screen.h>
#include <bps/bps.h>
#include <EGL/egl.h>
#include <screen/screen.h>
#include <sys/platform.h>
#include "input/screen_helpers.h"
*/

#include "NBGMMapNativeView.h"
#include "MapNativeView.h"
#include "MapNativeViewInterfaces.h"
#include "nbgmrendercontextimpl.h"
#include "paldisplay.h"

#include "MapViewUIInterface.h"
#include "nbgmnativerendercontext.h"
#include "RenderInterface.h"
#include "RenderEnginePrivate.hpp"
#include "NBUIConfig.h"

extern "C"
{
#include "palclock.h"
}

using namespace TCS;

#define SCREEN_WIDTH PAL_DisplayGetWidth()
#define SCREEN_HIGHT PAL_DisplayGetHeight()

MapNativeView::MapNativeView(MapViewUIInterface* mapView, int zorder, bool thirdPartyUsage)
: m_pMapView(mapView),
  m_zorder(zorder),
  m_thirdPartyUsage(thirdPartyUsage),
  m_pNativeHostView(NULL),
  m_enableDebugInformation(false),
  m_renderInterface(NULL),
  m_width(SCREEN_WIDTH),
  m_height(SCREEN_HIGHT),
  m_lastTimeval(0)
{
    if (!m_thirdPartyUsage)
    {
        m_pNativeHostView = new NBGMMapNativeView(m_pMapView, this, m_zorder);
        m_nbgmRenderContext = shared_ptr<NBGM_RenderContext>(new NBGM_RenderContextImpl(((NBGMMapNativeView*)m_pNativeHostView)->CreateNativeRenderContext(), NBUIConfig::GetRenderSystemType()));
    }
}

MapNativeView::~MapNativeView()
{
    Finallize();
}

NB_Error
MapNativeView::ShowView()
{
    return NE_NOSUPPORT;
}

NB_Error
MapNativeView::Initialize(int& width,int& height, PAL_Instance* pal)
{
    if (!m_thirdPartyUsage)
    {
        if (m_pNativeHostView == NULL)
        {
            return NE_NOMEM;
        }
        return ((NBGMMapNativeView*)m_pNativeHostView)->Initialize();
    }
    else
    {
        m_pMapView->UI_SetViewSize(m_width, m_height);

        return NE_OK;
    }
}

void
MapNativeView::Finallize()
{
    if (!m_thirdPartyUsage)
    {
        if (m_pNativeHostView != NULL)
        {
            ((NBGMMapNativeView*)m_pNativeHostView)->Finallize();
            delete  ((NBGMMapNativeView*)m_pNativeHostView);
            m_pNativeHostView = NULL;
        }
    }
    else
    {
        RenderEnginePrivate* privateRenderEngine = static_cast<RenderEnginePrivate*>(m_renderInterface);
        if (privateRenderEngine)
        {
            privateRenderEngine->Destroy();
            privateRenderEngine = NULL;
        }
    }
}

NB_Error
MapNativeView::SetMapViewProtocol(void *protocol)
{
    if (!m_thirdPartyUsage)
    {
        if(m_pNativeHostView)
        {
            ((NBGMMapNativeView *)m_pNativeHostView)->setMapNativeView((MapNativeViewInterfaces*) protocol);
        }

        return NE_OK;
    }
    else
    {
        RenderInterface* renderInterface = static_cast<RenderInterface*>(protocol);
        m_renderInterface = protocol;
        if(!m_nbgmRenderContext)
        {
            m_nbgmRenderContext = shared_ptr<NBGM_RenderContext>(new NBGM_RenderContextImpl(new NBGM_NativeRenderContext(renderInterface), NBUIConfig::GetRenderSystemType()));
        }

        return NE_OK;
    }

    m_pMapView->UI_MapNativeViewCreated();
}

void*
MapNativeView::GetNativeContext()
{
    return NULL;
}

void
MapNativeView::ShowPinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation)
{

}

void
MapNativeView::HidePinBubble(shared_ptr<nbmap::Pin> pin)
{

}

void
MapNativeView::UpdatePinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation)
{

}

void
MapNativeView::RemovePinBubble(shared_ptr<nbmap::Pin> pin)
{
}

void
MapNativeView::ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend)
{
}

void
MapNativeView::HideMapLegend()
{
}

void
MapNativeView::UpdateTrafficTip(TrafficTipType type)
{
    if (!m_thirdPartyUsage)
    {
        ((NBGMMapNativeView*)m_pNativeHostView)->UpdateTrafficTip(type);
    }
}

/* See description in header file. */
void
MapNativeView::UpdateAnimationTip(AnimationTipType type)
{
}

void
MapNativeView::CreateTimestampLabel()
{
}

void
MapNativeView::UpdateTimestampLabel(uint32 gpsTime)
{
}

void
MapNativeView::RemoveTimestampLabel()
{
}

void
MapNativeView::UpdateButtonState(bool animationCanPlay)
{
}

void
MapNativeView::ShowLocationBubble(float x, float y, bool orientation, double latitude, double longitude)
{
}

void
MapNativeView::HideLocationBubble()
{
}

void
MapNativeView::UpdateLocationBubble(float x, float y, bool orientation, double latitude, double longitude)
{
}

void
MapNativeView::CreateFontBuffer(PAL_Instance* pal, MapFontType fontType, uint8** buffer, uint32& bufferSize)
{
}

void
MapNativeView::SetViewSize(int width, int height)
{
    if (!m_thirdPartyUsage)
    {
        if(m_pNativeHostView!=NULL)
        {
            ((NBGMMapNativeView *)m_pNativeHostView)->SetViewSize(width, height);
        }
    }
    else
    {
        m_width = width;
        m_height = height;
    }
}

void
MapNativeView::UpdateHostView()
{
    if (!m_thirdPartyUsage)
    {
        if(m_pNativeHostView!=NULL)
        {
            ((NBGMMapNativeView *)m_pNativeHostView)->Update();
        }
    }
    else
    {
        uint32 currentTime       = PAL_ClockGetTimeMs();
        int    deltaMilliSeconds = currentTime - m_lastTimeval;
        m_lastTimeval            = currentTime;
        m_pMapView->UI_SetViewPort(m_width, m_height);
        m_pMapView->UI_Render(deltaMilliSeconds);
    }
}

shared_ptr<NBGM_RenderContext>
MapNativeView::GetRenderContext() const
{
    return m_nbgmRenderContext;
}

/* See header for description */
void
MapNativeView::EnableDebugInfo(bool isEnabled)
{
    if (!m_thirdPartyUsage)
    {
        m_enableDebugInformation = isEnabled;
    }
}

/* See header for description */
void
MapNativeView::SetDebugInfo(const char* text)
{
    if (!m_thirdPartyUsage)
    {
        if ( m_enableDebugInformation )
        {
            ((NBGMMapNativeView*)m_pNativeHostView)->ShowZoomLevelInfo(text);
        }
    }
}

void
MapNativeView::setZorder(int zorder)
{
    m_zorder = zorder;
}

void
MapNativeView::resetScreen()
{
    if (!m_thirdPartyUsage)
    {
        if(m_pNativeHostView)
        {
            ((NBGMMapNativeView *)m_pNativeHostView)->resetScreen();
        }
    }
    else
    {
        RenderEnginePrivate* privateRenderEngine = static_cast<RenderEnginePrivate*>(m_renderInterface);
        if (privateRenderEngine)
        {
            privateRenderEngine->resetScreen();
        }
    }
}


void
MapNativeView::UpdateTilt(float tilt)
{
    if (!m_thirdPartyUsage)
    {
        ((NBGMMapNativeView*)m_pNativeHostView)->UpdateTilt(tilt);
    }
}

void
MapNativeView::RenderingAllowedNotified()
{
    if(m_thirdPartyUsage)
    {
        RenderEnginePrivate* privateRenderEngine = static_cast<RenderEnginePrivate*>(m_renderInterface);
        if (privateRenderEngine)
        {
            privateRenderEngine->updateViewProperties();
        }
    }
}
/*! @} */
