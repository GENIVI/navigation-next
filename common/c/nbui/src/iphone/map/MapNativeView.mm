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
    @file     MapNativeView.mm
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
#include "PinImpl.h"
#include "nbgm.h"
#include "nbgmmapview.h"
#include "nbgmrendercontextimpl.h"
#import  "MapNativeHostView.h"
#import  "MapNativeDebugView.h"
#import  "MapPoiBubble.h"
#include "NBUIConfig.h"
#include "paldisplay.h"
#import "UIMapViewProtocol.h"
#import "BubbleInterface.h"

/* Constants used by tip label */
static NSString* map3DKitBundleDir = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Mapkit3D.bundle"];
static NSBundle* bundle = [NSBundle bundleWithPath:map3DKitBundleDir];
static NSString* const TIP_NO_TRAFFIC_INFO = NSLocalizedStringFromTableInBundle(@"IDS_NO_TRAFFIC_INFO", @"Localizable", bundle, nil);
static NSString* const TIP_ZOOM_IN_FOR_TRAFFIC = NSLocalizedStringFromTableInBundle(@"IDS_ZOOM_IN_FOR_TRAFFIC", @"Localizable", bundle, nil);
static NSString* const TIP_NO_DOPPLER = NSLocalizedStringFromTableInBundle(@"IDS_NO_DOPPLER_LAYER_ON_THIS", @"Localizable", bundle, nil);

static shared_ptr<nbmap::BubbleInterface> GetBubbleObject(shared_ptr<nbmap::Pin> pin)
{
    if(!pin)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }

    nbmap::PinImpl<nbmap::BubbleInterface>* pinImpl = static_cast<nbmap::PinImpl<nbmap::BubbleInterface>*>(pin.get());
    if(!pinImpl)
    {
        return shared_ptr<nbmap::BubbleInterface>();
    }
    return pinImpl->GetBubble();
}

MapNativeView::MapNativeView(MapViewUIInterface* mapView, int zorder, bool thirdPartyUsage)
        : m_pMapView(mapView),
          m_pNativeHostView(NULL),
          m_pDebugView(NULL),
          m_enableDebugInformation(false),
          m_zorder(zorder),
          m_thirdPartyUsage(thirdPartyUsage)
{
    // Don't initialize anything here, do it in the Initialize() call.
};

MapNativeView::~MapNativeView()
{
    // Don't do anything here, do all the cleanup in Finallize().
};

/* See description in header */
NB_Error
MapNativeView::Initialize(int& width,int& height, nb_threadId renderingThread,PAL_Instance*)
{
    // Create the host view which contains the OpenGL view/context.
    CGRect rootViewBounds = [((id<UIMapViewProtocol>)m_pNativeHostView) getRootViewBounds];
    float scaleFactor = [UIScreen mainScreen].scale;

    width = (int)rootViewBounds.size.width * scaleFactor;
    height = (int)rootViewBounds.size.height * scaleFactor;

    MapNativeHostView* hostView= [[MapNativeHostView alloc] initWithFrame:rootViewBounds
                                                                  mapView:(void*)m_pMapView
                                                                 protocol:m_pNativeHostView];

    NBGM_NativeRenderContext* nativeRenderContext = [hostView createRenderContext];
    m_nbgmRenderContext.reset(new NBGM_RenderContextImpl(nativeRenderContext, NBUIConfig::GetRenderSystemType()));
    m_pNativeHostView  = hostView;

    //let's add here
    m_pMapView->UI_SetViewPort(width, height);
    return NE_OK;
}

/* See description in header */
void
MapNativeView::Finallize()
{
    m_pDebugView = nil;

    //Before releasing the host view, make sure the render context is shutdown.
    //But do not call NBGM_RenderContext::Shutdown() here because this is being callind from the UI thread
    if (m_pNativeHostView)
    {
        [(MapNativeHostView*)m_pNativeHostView release];
        m_pNativeHostView = nil;
    }
}

void
MapNativeView::OnMapInitialized()
{
    if (m_pNativeHostView)
    {
        MapNativeHostView* hostView = (MapNativeHostView*) m_pNativeHostView;
        [hostView onMapInitialized];
    }
}

/* See description in header */
NB_Error
MapNativeView::ShowView()
{
    // @todo:
    // Create the UIView if not created yet. Show it.

    return NE_NOSUPPORT;
}

/* See header for description */
void*
MapNativeView::GetNativeContext()
{
    return m_pNativeHostView;
}

/* See description in header file. */
void
MapNativeView::ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend)
{
    // Forward to map host view
    if (m_pNativeHostView)
    {
        MapNativeHostView* hostView = (MapNativeHostView*) m_pNativeHostView;
        [hostView showMapLegend:mapLegend];
    }
}

/* See description in header file. */
void
MapNativeView::HideMapLegend()
{
    // Forward to map host view
    if (m_pNativeHostView)
    {
        MapNativeHostView* hostView = (MapNativeHostView*) m_pNativeHostView;
        [hostView hideMapLegend];
    }
}

/* See description in header file. */
void
MapNativeView::UpdateTrafficTip(TrafficTipType type)
{
    if (m_pNativeHostView)
    {
        NSString *tipString = nil;
        MapNativeHostView* hostView = (MapNativeHostView*) m_pNativeHostView;
        switch (type)
        {
            case TTT_None:
            {
                [hostView hideTipView];
                break;
            }
            case TTT_ZoomIn:
            {
                tipString = TIP_ZOOM_IN_FOR_TRAFFIC;
                break;
            }
            case TTT_NoData:
            {
                tipString = TIP_NO_TRAFFIC_INFO;
                break;
            }
            default: // Invalid type, do nothing.
            {
                break;
            }
        }
        if (tipString)
        {
            [hostView updateTipView:tipString];
        }
    }
}

/* See description in header file. */
void
MapNativeView::UpdateAnimationTip(AnimationTipType type)
{
    if (!m_pNativeHostView)
    {
        return;
    }

    MapNativeHostView* hostView = static_cast<MapNativeHostView*>(m_pNativeHostView);
    NSString* tipString = nil;

    switch (type)
    {
        case ATT_None:
        {
            break;
        }
        case ATT_ZoomOut:
        {
            tipString = TIP_NO_DOPPLER;
            break;
        }
        default:
        {
            break;
        }
    }

    if (tipString)
    {
        [hostView updateTipView:tipString];
    }
}

/* See description in header file. */
void
MapNativeView::CreateTimestampLabel()
{

}

/* See description in header file. */
void
MapNativeView::UpdateTimestampLabel(uint32 gpsTime)
{
    // Forward to map host view
    if (m_pNativeHostView)
    {
        MapNativeHostView* hostView = (MapNativeHostView*) m_pNativeHostView;
        [hostView updateTimestampView:gpsTime];
    }
}

/* See description in header file. */
void
MapNativeView::RemoveTimestampLabel()
{

}

/* See description in header file. */
void
MapNativeView::UpdateButtonState(bool animationCanPlay)
{
    // Forward to map host view
    if (m_pNativeHostView)
    {
        MapNativeHostView* hostView = (MapNativeHostView*) m_pNativeHostView;
        if(animationCanPlay)
        {
            [hostView updateButtonState:YES];
        }
        else
        {
            [hostView updateButtonState:NO];
        }
    }
}

/* See description in header file. */
void
MapNativeView::ShowLocationBubble(float x, float y, bool orientation,
                                  double latitude, double longitude)
{
    // Forward to map host view
    if (m_pNativeHostView)
    {
        UpdateLocationBubble(x, y, orientation, latitude, longitude);
    }
}

/* See description in header file. */
void
MapNativeView::HideLocationBubble()
{
    if (m_pNativeHostView)
    {
        MapNativeHostView* NBGMView = (MapNativeHostView*) m_pNativeHostView;
        [NBGMView removeLocationBubble];
    }
}

/* See description in header file. */
void
MapNativeView::UpdateLocationBubble(float x, float y, bool orientation,
                                    double latitude, double longitude)
{
    // This function get called in UI_Thread.
    if (m_pNativeHostView)
    {
        BubbleViewParamers *parameters = [[BubbleViewParamers alloc]init];
        parameters.x = x;
        parameters.y = y;
        parameters.orientation = orientation;

        MapNativeHostView* NBGMView = (MapNativeHostView*) m_pNativeHostView;
        [NBGMView updateLocationBubble:parameters lat:latitude lon:longitude];
    }
}

void
MapNativeView::UpdateHostView()
{
    MapNativeHostView* NBGMView = (MapNativeHostView*) m_pNativeHostView;
    [NBGMView update];
}

/* See header for description */
shared_ptr<NBGM_RenderContext>
MapNativeView::GetRenderContext() const
{
    return m_nbgmRenderContext;
}

/* See header for description */
void
MapNativeView::EnableDebugInfo(bool isEnabled)
{
    if (m_pNativeHostView)
    {
        MapNativeHostView* NBGMView = (MapNativeHostView*) m_pNativeHostView;
        [NBGMView enableDebugView:isEnabled ? YES : NO];
    }
}

/* See header for description */
void
MapNativeView::SetDebugInfo(const char* text)
{
    if (m_pNativeHostView)
    {
        MapNativeHostView* NBGMView = (MapNativeHostView*) m_pNativeHostView;
        [NBGMView setDebugInfo:text];
    }
}

void
MapNativeView::CreateDebugView()
{
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
MapNativeView::UpdateSurface()
{
}

void
MapNativeView::DestroySurface()
{
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

void
MapNativeView::UpdateTilt(float tilt)
{
    //do nothing
}

void
MapNativeView::SetViewSize(int width, int height)
{
    //do nothing
}

void
MapNativeView::resetScreen()
{
    //do nothing
}

NB_Error
MapNativeView::SetMapViewProtocol(void* protocol)
{
    //TODO: Temp solution. Use m_pNativeHostView to save protocal temporarily.
    m_pNativeHostView = protocol;
    m_pMapView->UI_MapNativeViewCreated();
    return NE_OK;
}

void
MapNativeView::RenderingAllowedNotified()
{
    //do nothing
}

void
MapNativeView::SetGpsMode(NB_GpsMode mode)
{
    //do nothing
}

/*! @} */
