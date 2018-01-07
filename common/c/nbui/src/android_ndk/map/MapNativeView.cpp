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
#include <jni.h>
#include "palevent.h"
#include "MapNativeView.h"
#include "nbgm.h"
#include "nbgmmapview.h"
#include "palstdlib.h"
#include "PinImpl.h"
#include "nbgmandroidrendercontext.h"
#include "AndroidNativeHostView.h"
#include "PinImpl.h"
#include "smartpointer.h"
#include "BubbleInterface.h"

#include <android/log.h>
#define LOG_TAG    "NBUI"

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_LINE   __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, "%s, %d", __func__, __LINE__)

typedef enum
{
    TIP_NO_TRAFFIC_INFO = 1,
    TIP_ZOOM_IN_FOR_TRAFFIC,
    TIP_ZOOM_OUT_FOR_ANIMATION
}TIP_INFO;

MapNativeView::MapNativeView(MapViewUIInterface* mapView, int zorder, bool thirdPartyUsage)
: m_pMapView(mapView),
  m_enableDebugInformation(false),
  m_pDebugView(NULL),
  m_pNativeHostView(NULL)
{
//    AndroidNativeHostView* hostView = new AndroidNativeHostView();
//    hostView->SetUIInterface(m_pMapView);
    m_pNativeHostView  = NULL;
};

MapNativeView::~MapNativeView()
{
};

/* See description in header */
NB_Error
MapNativeView::ShowView()
{
    return NE_NOSUPPORT;
}

/* See header for description */
void*
MapNativeView::GetNativeContext()
{
    if (m_pNativeHostView)
    {
        return (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->GetMapViewContext();
    }
}

NB_Error
MapNativeView::Initialize(int& width, int& height, nb_threadId renderingThread, PAL_Instance* pal)
{
    AndroidNativeHostView* nativeHostView = (static_cast<AndroidNativeHostView*>(m_pNativeHostView));
    NB_Error error = nativeHostView->Initialize(pal);
    if (error != NE_OK)
    {
        return error;
    }

    NBGM_AndroidRenderContext* nativeContext = new NBGM_AndroidRenderContext(nativeHostView->GetWindow(), pal, nativeHostView->GetRenderConfig());
    m_nbgmRenderContext.reset(nativeContext);
    nativeHostView->GetViewDimension(width, height);

    return NE_OK;
}

void
MapNativeView::Finallize()
{
    //Before release host view, make sure render context is shutdown.
    //But do not call NBGM_RenderContext::Shutdown() here because here is main thread
    if (m_pNativeHostView)
    {
//        delete static_cast<AndroidNativeHostView *>(m_pNativeHostView);
        m_pNativeHostView = NULL;
    }
}

/* See header for description */
void
MapNativeView::UpdateHostView()
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->Render(false);
    }
}

/* See header for description */
void
MapNativeView::EnableDebugInfo(bool isEnabled)
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->EnableDebugInfo(isEnabled);
    }
}

/* See header for description */
void
MapNativeView::SetDebugInfo(const char* text)
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->SetDebugInfo(text);
    }
}

void
MapNativeView::CreateDebugView()
{
}

static shared_ptr<nbmap::BubbleInterface>
GetBubbleObject(shared_ptr<nbmap::Pin> pin)
{
    if (!pin)
    {
        LOGE("%s:%s:%d", __FILE__, __func__, __LINE__);
        return shared_ptr<nbmap::BubbleInterface>();
    }

    nbmap::PinImpl<nbmap::BubbleInterface>* pinImpl = static_cast<nbmap::PinImpl<nbmap::BubbleInterface>*>(pin.get());
    if (!pinImpl)
    {
        LOGE("%s:%s:%d", __FILE__, __func__, __LINE__);
        return shared_ptr<nbmap::BubbleInterface>();
    }

    return pinImpl->GetBubble();
}


void
MapNativeView::ShowPinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation)
{
    shared_ptr<nbmap::BubbleInterface> bubble = GetBubbleObject(pin);
    if (bubble == NULL)
    {
        bubble = m_pMapView->UI_GetBubble(pin->GetPinID());
    }
    if(bubble)
    {
        bubble->Show(x, y, orientation);
    }
}

void
MapNativeView::HidePinBubble(shared_ptr<nbmap::Pin> pin)
{
    shared_ptr<nbmap::BubbleInterface> bubble = GetBubbleObject(pin);
    if (bubble == NULL)
    {
        bubble = m_pMapView->UI_GetBubble(pin->GetPinID());
    }
    if(bubble)
    {
        bubble->Hide();
    }
}

void
MapNativeView::UpdatePinBubble(shared_ptr<nbmap::Pin> pin, float x, float y, bool orientation)
{
    shared_ptr<nbmap::BubbleInterface> bubble = GetBubbleObject(pin);
    if (bubble == NULL)
    {
        bubble = m_pMapView->UI_GetBubble(pin->GetPinID());
    }
    if(bubble)
    {
        bubble->Update(x, y, orientation);
    }
}

void
MapNativeView::RemovePinBubble(nbmap::PinPtr pin)
{
    shared_ptr<nbmap::BubbleInterface> bubble = GetBubbleObject(pin);
    if (bubble == NULL)
    {
        bubble = m_pMapView->UI_GetBubble(pin->GetPinID());
    }
    if(bubble)
    {
        bubble->Remove();
    }
}
/* See header for description */
shared_ptr<NBGM_RenderContext>
MapNativeView::GetRenderContext() const
{
    return m_nbgmRenderContext;
}

/* See description in header file. */
//@TODO: Change void* to proper data struct.
void MapNativeView::ShowMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegend)
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->ShowMapLegend(mapLegend);
    }
}


/* See description in header file. */
void MapNativeView::HideMapLegend()
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->HideMapLegend();
    }
}

void MapNativeView::ShowLocationBubble(float x, float y, bool orientation,
                                       double latitude, double longitude)
{

}

void MapNativeView::HideLocationBubble()
{

}

void MapNativeView::UpdateLocationBubble(float x, float y, bool orientation,
                                         double latitude, double longitude)
{

}

/* See description in header file. */
void
MapNativeView::UpdateTrafficTip(TrafficTipType type)
{
    AndroidNativeHostView* hostView = static_cast<AndroidNativeHostView*>(m_pNativeHostView);

    if (!hostView)
    {
        return;
    }
    int tipStringId = 0;
    switch (type)
    {
        case TTT_None:
        {
            return;
        }
        case TTT_ZoomIn:
        {
            tipStringId = TIP_ZOOM_IN_FOR_TRAFFIC;
            break;
        }
        case TTT_NoData:
        {
            tipStringId = TIP_NO_TRAFFIC_INFO;
            break;
        }
        default: // Invalid type, do nothing.
        {
            break;
        }
    }
    if (tipStringId != 0)
    {
        hostView->ShowTip(tipStringId);
    }
}

/* See description in header file. */
void
MapNativeView::UpdateAnimationTip(AnimationTipType type)
{
    AndroidNativeHostView* hostView = static_cast<AndroidNativeHostView*>(m_pNativeHostView);

    if (!hostView)
    {
        return;
    }
    int tipStringId = 0;
    switch (type)
    {
        case ATT_None:
        {
            return;
        }
        case ATT_ZoomOut:
        {
            tipStringId = TIP_ZOOM_OUT_FOR_ANIMATION;
            break;
        }
        default: // Invalid type, do nothing.
        {
            break;
        }
    }
    if (tipStringId != 0)
    {
        hostView->ShowTip(tipStringId);
    }
}

void
MapNativeView::CreateTimestampLabel()
{

}

void
MapNativeView::UpdateTimestampLabel(uint32 gpsTime)
{
     if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->UpdateTimestampLabel(gpsTime);
    }
}

void
MapNativeView::RemoveTimestampLabel()
{

}

/* See description in header file. */
void
MapNativeView::UpdateButtonState(bool animationCanPlay)
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->UpdateButtonState(animationCanPlay);
    }
}

void
MapNativeView::resetScreen()
{
}

NB_Error
MapNativeView::SetMapViewProtocol(void *protocol)
{
    AndroidNativeHostView* nativeHostView = new AndroidNativeHostView();
    if (nativeHostView)
    {
        nativeHostView->SetUIInterface(m_pMapView);
        nativeHostView->SetMapSurfaceViewContext(protocol);
    }
    m_pNativeHostView = nativeHostView;
    return NE_OK;
}

void
MapNativeView::UpdateTilt(float tilt)
{
}

void
MapNativeView::SetViewSize(int width, int height)
{
    if (m_pNativeHostView)
    {
        (static_cast<AndroidNativeHostView*>(m_pNativeHostView))->SetViewSize(width, height);
    }
}

void
MapNativeView::RenderingAllowedNotified()
{
}

void
MapNativeView::OnMapInitialized()
{
}

void
MapNativeView::UpdateSurface()
{
    if (m_pNativeHostView)
    {
        NBGM_AndroidRenderContext *context = static_cast<NBGM_AndroidRenderContext*>(m_nbgmRenderContext.get());
        if(context)
        {
            context->UpdateSurface((static_cast<AndroidNativeHostView*>(m_pNativeHostView))->GetWindow());
        }
    }
}

void
MapNativeView::DestroySurface()
{
    if (m_pNativeHostView)
    {
        NBGM_AndroidRenderContext *context = static_cast<NBGM_AndroidRenderContext*>(m_nbgmRenderContext.get());
        if(context)
        {
            context->DestroySurface();
        }
    }
}

void
MapNativeView::SetGpsMode(NB_GpsMode mode)
{
}
