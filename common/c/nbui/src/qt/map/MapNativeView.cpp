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
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "QtNativeHostView.h"
//#include <QHBoxLayout>
#include "palevent.h"
#include "NativeRenderContext.h"
#include "MapNativeView.h"
#include "nbgm.h"
#include "nbgmmapview.h"
#include "palstdlib.h"
#include "PinImpl.h"
#include "NBUIConfig.h"
#include <QVariant>
#include "MapViewProtocol.h"
#include <QDebug>
//#include <QApplication>
#include <QScreen>

#define ANTIALIASING_PPROPERTY_NAME "enableFullScreenAntiAliasing"
#define ANISOTROPIC_PROPERTY_NAME "enableAnisotropicFiltering"
#define MAPVIEW_ONLY_PROPERTY_NAME "mapviewOnly"

#ifdef __QNXNTO__
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 1)
#include <QWindow>
#include <pthread.h>
#include <screen/screen.h>
// OpenGL not working workaround from QNX.
// glContainer is the direct parent widget of QGLWidget
void FixGLWidgetContainer(QWidget* glContainer)
{
    qWarning() << "FixGLWidgetContainer begin " << glContainer;
    if (glContainer == NULL)
    {
        return;
    }
    screen_window_t window = reinterpret_cast<screen_window_t>(glContainer->windowHandle()->winId());
    screen_context_t context;
    if (0 != screen_get_window_property_pv(window, SCREEN_PROPERTY_CONTEXT, reinterpret_cast<void**>(&context))) {
        return;
    }
    qWarning() << "FixGLWidgetContainer line" << __LINE__ << "context=" << context;

    int buffer_count;
    if (0 != screen_get_window_property_iv(window, SCREEN_PROPERTY_RENDER_BUFFER_COUNT, &buffer_count)) {
        return;
    }
    qWarning() << "FixGLWidgetContainer line" << __LINE__ << "buffer_count=" << buffer_count;
    if (buffer_count != 2) {
        return;
    }
    qWarning() << "FixGLWidgetContainer line" << __LINE__;
    screen_buffer_t buffers[2];
    if (0 != screen_get_window_property_pv(window, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)buffers)) {
        return;
    }
    qWarning() << "FixGLWidgetContainer line" << __LINE__;

    int rectangle[] = { 0, 0, 1, 1 };
    if (0 != screen_post_window(window, buffers[0], 1, rectangle, 0)) {
        return;
    }
    qWarning() << "FixGLWidgetContainer line" << __LINE__;

    int value = SCREEN_TRANSPARENCY_DISCARD;
    if (0 != screen_set_window_property_iv(window, SCREEN_PROPERTY_TRANSPARENCY, &value)) {
        return;
    }
    qWarning() << "FixGLWidgetContainer line" << __LINE__;

    if (0 != screen_flush_context(context, 0)) {
        return;
    }
    qWarning() << "FixGLWidgetContainer end " << glContainer;
}

void showChildren(QWidget *widget)
{
    for (auto child: widget->children()) {
        QWidget *childWidget = qobject_cast<QWidget *>(child);
        if (childWidget && childWidget->windowHandle()) {
            childWidget->windowHandle()->show();
            qWarning() << "show widget " << childWidget;
            showChildren(childWidget);
        }
    }
}
#endif
#endif

MapNativeView::MapNativeView(MapViewUIInterface* mapView, int zorder, bool thirdPartyUsage, void* threadId)
    : m_pMapView(mapView),
      m_zorder(zorder),
      m_thirdPartyUsage(thirdPartyUsage),
      m_pNativeHostView(NULL),
      m_pDebugView(NULL),
      m_enableDebugInformation(false),
      m_renderThreadId(threadId)
{
}

MapNativeView::~MapNativeView()
{
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
    return static_cast<QtNativeHostView*>(m_pNativeHostView);
}

/* See description in header */
NB_Error
MapNativeView::Initialize(int& width, int& height, nb_threadId renderingThread, PAL_Instance* pal)
{
#ifdef __QNXNTO__
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 1)
    // @TODO: should we call it here?
    QWidget* p = static_cast<QtNativeHostView*>(m_pNativeHostView)->parentWidget();
    while (p && !p->isTopLevel())
    {
        FixGLWidgetContainer(p);
        p = p->parentWidget();
    }
#endif
#endif
    NB_Error error = (static_cast<QtNativeHostView*>(m_pNativeHostView))->Initialize(renderingThread);


    if (error != NE_OK)
    {
        return error;
    }

    (static_cast<QtNativeHostView*>(m_pNativeHostView))->GetViewDimension(width, height);

    return NE_OK;
}

/* See description in header */
void
MapNativeView::Finallize()
{
    if (m_pNativeHostView)
    {
        m_pNativeHostView = NULL;
    }
}

/* See description in header file. */
void
MapNativeView::UpdateHostView()
{
    if (m_pNativeHostView)
    {
        (static_cast<QtNativeHostView*>(m_pNativeHostView))->Render(false);
    }
}

/* See header for description */
void
MapNativeView::EnableDebugInfo(bool isEnabled)
{
    qDebug() << "MapNativeView::EnableDebugInfo(" << isEnabled << ")";
    if( m_pNativeHostView != NULL )
    {
        static_cast<QtNativeHostView*>(m_pNativeHostView)->ShowDebugView(isEnabled);
    }
    m_enableDebugInformation = isEnabled;
}

/* See header for description */
void
MapNativeView::SetDebugInfo(const char* text)
{
    if( m_pNativeHostView != NULL )
    {
        static_cast<QtNativeHostView*>(m_pNativeHostView)->SetDebugText( QString(text) );
    }

    if (m_enableDebugInformation)
    {
        qWarning() << text;
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
        return shared_ptr<nbmap::BubbleInterface>();
    }

    nbmap::PinImpl<nbmap::BubbleInterface>* pinImpl = static_cast<nbmap::PinImpl<nbmap::BubbleInterface>*>(pin.get());
    if (!pinImpl)
    {
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
    //Just stub to avoid build errors. Need to implement for QNX.
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
void
MapNativeView::ShowMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegend)
{
    if( m_pNativeHostView )
    {
        QtNativeHostView* pHostView = static_cast<QtNativeHostView*>( m_pNativeHostView );
        pHostView->ShowMapLegend( mapLegend );
    }
}


/* See description in header file. */
void
MapNativeView::HideMapLegend()
{
    if( m_pNativeHostView )
    {
        QtNativeHostView* pHostView = static_cast<QtNativeHostView*>( m_pNativeHostView );
        pHostView->HideMapLegend();
    }
}


/* See description in header file. */
void
MapNativeView::ShowLocationBubble(float x, float y, bool orientation,
                                  double latitude, double longitude)
{

}


/* See description in header file. */
void
MapNativeView::HideLocationBubble()
{

}

/* See description in header file. */
void
MapNativeView::UpdateLocationBubble(float x, float y, bool orientation,
                                    double latitude, double longitude)
{

}

/* See description in header file. */
void
MapNativeView::UpdateTrafficTip(TrafficTipType type)
{
    if( m_pNativeHostView )
    {
        QtNativeHostView* pHostView = static_cast<QtNativeHostView*>( m_pNativeHostView );
        QString tipString;
        switch (type)
        {
            case TTT_None:
            {
                tipString.clear();
            }
            break;
            case TTT_ZoomIn:
            {
                tipString = QObject::tr("Zoom in for traffic");
            }
            break;
            case TTT_ZoomOut:
            {
                tipString = QObject::tr("Zoom out for traffic");
            }
            break;
            case TTT_NoData:
            {
                tipString = QObject::tr("No data for traffic");
            }
            break;
            default:
            break;
        }
        pHostView->ShowToolTip(tipString);
    }
}

/* See description in header file. */
void
MapNativeView::UpdateAnimationTip(AnimationTipType type)
{
    QtNativeHostView* pHostView = static_cast<QtNativeHostView*>( m_pNativeHostView );

    switch ( type )
    {
    case ATT_ZoomOut:
        pHostView->ShowToolTip( QObject::tr("No weather information on this zoom level. Please zoom out") );
        break;
    case ATT_None:
        pHostView->ShowToolTip( "" );
        break;
    default:
        break;
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
    static_cast<QtNativeHostView*>(m_pNativeHostView)->UpdateTimestampLabel(gpsTime);
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
    static_cast<QtNativeHostView*>(m_pNativeHostView)->UpdateButtonState(animationCanPlay);
}

void
MapNativeView::resetScreen()
{
    //do nothing
}

NB_Error
MapNativeView::SetMapViewProtocol(void* protocol)
{
    if(NULL != protocol)
    {
        MapViewProtocol* mapViewProtocol = reinterpret_cast<MapViewProtocol*>(protocol);

        QQuickItem* parent = mapViewProtocol->GetParent();

//        QHBoxLayout* mapWidgetLayout = new QHBoxLayout( parent );
//        mapWidgetLayout->setMargin(0);

        QtNativeHostView* nativeHostView = NULL;

//        bool enableAntiAliasing = parent->property( ANTIALIASING_PPROPERTY_NAME ).toBool();
//        bool enableAnisotropicFiltering = parent->property( ANISOTROPIC_PROPERTY_NAME ).toBool();
//        bool mapviewOnly = parent->property( MAPVIEW_ONLY_PROPERTY_NAME).toBool();

//        QGLFormat format(QGL::DoubleBuffer);

//        format.setRedBufferSize(5);
//        format.setGreenBufferSize(6);
//        format.setBlueBufferSize(5);
//        format.setAlphaBufferSize(0);
//        if( enableAntiAliasing )
//        {
//            format.setOption(QGL::SampleBuffers);
//        }

        nativeHostView = new QtNativeHostView(false, true, mapViewProtocol, m_renderThreadId);
        nativeHostView->SetNativeView(this);
        //mapWidgetLayout->addWidget(nativeHostView);
        //parent->setLayout(mapWidgetLayout);

//        NativeRenderContext* nativeContext = new NativeRenderContext(nativeHostView,
//                                                                     NBUIConfig::GetRenderSystemType());
//        m_nbgmRenderContext.reset(nativeContext);
//        m_pNativeHostView = nativeHostView;

//        static_cast<QtNativeHostView*>(m_pNativeHostView)->SetUIInterface(m_pMapView);
//        static_cast<QtNativeHostView*>(m_pNativeHostView)->SetNativeRenderContext(nativeContext);

        m_pNativeHostView = nativeHostView;
        //defer it to QtNativeHostView::ready()
        //m_pMapView->UI_MapNativeViewCreated();
    }

    return NE_OK;
}

void
MapNativeView::OnUIReady(void* glContext)
{
    NativeRenderContext* nativeContext = new NativeRenderContext(glContext, NBUIConfig::GetRenderSystemType());
    m_nbgmRenderContext.reset(nativeContext);

    static_cast<QtNativeHostView*>(m_pNativeHostView)->SetUIInterface(m_pMapView);
    static_cast<QtNativeHostView*>(m_pNativeHostView)->SetNativeRenderContext(nativeContext);

    m_pMapView->UI_MapNativeViewCreated();
}

void
MapNativeView::UpdateTilt(float tilt)
{
    //do nothing
}

void
MapNativeView::SetViewSize(int width, int height)
{
    if (m_pNativeHostView)
    {
        //static_cast<NativeRenderContext*>(m_nbgmRenderContext.get())->UpdateSurface(static_cast<QtNativeHostView*>(m_pNativeHostView));
        (static_cast<QtNativeHostView*>(m_pNativeHostView))->SetViewSize(width, height);
    }
}

void
MapNativeView::RenderingAllowedNotified()
{
    //do nothing
}

void
MapNativeView::OnMapInitialized()
{
}


void
MapNativeView::UpdateSurface()
{
}

void
MapNativeView::DestroySurface()
{
    if (m_pNativeHostView)
    {
        static_cast<QtNativeHostView*>(m_pNativeHostView)->SetUIInterface( NULL );
    }
}

void
MapNativeView::SetGpsMode(NB_GpsMode mode)
{
    if (m_pNativeHostView)
    {
        static_cast<QtNativeHostView*>(m_pNativeHostView)->SetLocateMeButtonShowMode(mode);
    }
}

void
MapNativeView::SetDisplayScreen(int screenIndex)
{
//    if (!m_pNativeHostView || screenIndex < 0 || screenIndex >= QApplication::screens().size())
//    {
//        return;
//    }

#ifdef __QNXNTO__
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 1)

    NativeRenderContext* renderContext = (NativeRenderContext*)m_nbgmRenderContext.get();
    //renderContext->EnterBackground(true);

    qDebug() << "screens: " << QApplication::screens().size();
    foreach (QScreen* s, QApplication::screens())
    {
        qDebug() << s->geometry();
    }

    // @Notes: Secondary display fix From QNX engineer
    // Qt is recreating all the windows when the screen switch is made. 
    // This isn�t unexpected but it does mean that the fix has to be reapplied in this situation. 
    // Unfortunately, that isn't quite enough to get things back in a working state.
    // Qt is also making all the native child windows invisible. 
    // No idea why it's doing this or why it doesn�t restore the old visibility when it finishes the switch.
    // But it doesn't really matter since you can just make the windows visible again.
    //    
    QWidget* w = static_cast<QtNativeHostView*>(m_pNativeHostView);

    QWindow* ww = w->topLevelWidget()->windowHandle();
    // switch display screen
    if (ww)
    {
        qWarning() << "widget=" << ww << ", switch to display " << screenIndex;
        ww->setScreen(QApplication::screens().at(screenIndex));
    }

    qWarning() << "fix child visibility " << w;
    showChildren(w->topLevelWidget());
    ww->show();

    QWidget* p = w->parentWidget();
    while (p && !p->isTopLevel())
    {
        FixGLWidgetContainer(p);
        p = p->parentWidget();
    }

    qWarning() << "end switch" << w;

    //renderContext->EnterBackground(false);

#endif
#endif
}

/*! @} */
