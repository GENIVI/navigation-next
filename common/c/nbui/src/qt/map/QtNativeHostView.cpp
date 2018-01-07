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
 @file     QtNativeHostView.cpp
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
#include "paldebuglog.h"
#include "palstdlib.h"
#include "palclock.h"
#include <assert.h>
#include "MapServicesConfiguration.h"
#include <nbnetwork.h>
#include <nbnetworkoptions.h>
#include <palfile.h>
#include "MetadataConfiguration.h"
#include <QtGui>
//#include "TiltGestureRecognizer.h"
//#include "pangesturerecognizer.h"
#include <QTimer>
//#include "LocateMeButton.h"
//#include <QHBoxLayout>
//#include <QVBoxLayout>
//#include <QApplication>
#include "MapViewProtocol.h"
#include "palmath.h"
#include "mapdecorationsignals.h"
#include <QLineF>
#include "renderthreadqt.h"
#include "MapNativeView.h"
#include "gesturedetector.h"

QList<QThread *> QtNativeHostView::threads;



#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define ANISOTROPIC_FILTERING "GL_EXT_texture_filter_anisotropic"
#define UNLOCK_CAMERA_ANGLE_THRESHOLD 15.0
#define MINIMUM_TIME_GAP_FOR_CACULATE_MOUSE_VELOCITY 4 //ms
#define GESTURE_MOVE_CONTAINER_SIZE 2

const double QtNativeHostView::INVALID_LATITUED = -999.;
/** Invalid longitude value, the camera will ignore this value*/
const double QtNativeHostView::INVALID_LONGTITUED = -999.;
/** Invalid zoomLevel value, the camera will ignore this value*/
const double QtNativeHostView::INVALID_ZOOMLEVEL_VALUE = -999.;
/** Invalid heading value, the camera will ignore this value*/
const double QtNativeHostView::INVALID_HEADING_VALUE = -999.;
/** Invalid tilt value, the camera will ignore this value*/
const double QtNativeHostView::INVALID_TILT_VALUE = -999.;

const int COMPASS_ANIMATION_TIME = 500;







QtNativeHostView::QtNativeHostView(bool enableAnisotropicFiltering, bool mapviewOnly, MapViewProtocol* protocol, void* renderThread)
    : m_renderThread((RenderThreadQt*)renderThread)
    , m_mapViewUI(NULL)
    , m_Parent(protocol->GetParent())
    , m_width(protocol->GetParent()->width())
    , m_height(protocol->GetParent()->height())
    , m_lastUpdateTime(0)
    , m_lock(NULL)
    , m_nativeContext(NULL)
//    , m_tiltGesureRecognizer(NULL)
//    , m_panGestureType(Qt::CustomGesture)
//    , m_panGesureRecognizer(NULL)
    , m_gestureType(Qt::CustomGesture)
    , m_firstUpTime(0)
    , m_secondDownTime(0)
    , m_firstDown(NULL)
    , m_secondDown(NULL)
    , m_isDoubleTapping(false)
    , m_mouseOriginalX(0.0f)
    , m_mouseOriginalY(0.0f)
    , m_mouseDown(false)
    , m_mouseMoveStarted(false)
    , m_renderStarted(false)
    , m_longPressTimer(NULL)
    , m_longPressTriggered(false)
//    , m_pDebugText(NULL)
//    , m_pTipInfo(NULL)
//    , m_centerLayout(NULL)
//    , m_bottomLayout(NULL)
//    , m_mainLayout(NULL)
//    , m_locateMeButton(NULL)
    , m_gpsMode(NGM_INVALID)
//    , m_mapLegendWidget(NULL)
    , m_enableAnisotropicFiltering( enableAnisotropicFiltering )
    , m_mapviewOnly(mapviewOnly)
{
    initializeMembers( protocol );
}

QtNativeHostView::QtNativeHostView(bool enableAnisotropicFiltering, bool mapviewOnly, MapViewProtocol* protocol)
    : m_mapViewUI(NULL)
    , m_Parent(protocol->GetParent())
    , m_width(protocol->GetParent()->width())
    , m_height(protocol->GetParent()->height())
    , m_lastUpdateTime(0)
    , m_lock(NULL)
    , m_nativeContext(NULL)
//    , m_tiltGesureRecognizer(NULL)
//    , m_panGestureType(Qt::CustomGesture)
//    , m_panGesureRecognizer(NULL)
    , m_gestureType(Qt::CustomGesture)
    , m_firstUpTime(0)
    , m_secondDownTime(0)
    , m_firstDown(NULL)
    , m_secondDown(NULL)
    , m_isDoubleTapping(false)
    , m_mouseOriginalX(0.0f)
    , m_mouseOriginalY(0.0f)
    , m_mouseDown(false)
    , m_mouseMoveStarted(false)
    , m_renderStarted(false)
    , m_longPressTimer(NULL)
    , m_longPressTriggered(false)
//    , m_pDebugText(NULL)
//    , m_pTipInfo(NULL)
//    , m_centerLayout(NULL)
//    , m_bottomLayout(NULL)
//    , m_mainLayout(NULL)
//    , m_locateMeButton(NULL)
    , m_gpsMode(NGM_INVALID)
//    , m_mapLegendWidget(NULL)
    , m_enableAnisotropicFiltering( enableAnisotropicFiltering )
    , m_mapviewOnly(mapviewOnly)
{
    initializeMembers( protocol );
}

QtNativeHostView::~QtNativeHostView()
{
    PAL_LockDestroy(m_lock);
    delete m_gestureDetector;
}

NB_Error
QtNativeHostView::Initialize(nb_threadId renderingThread)
{
//    makeCurrent();
//#ifdef Q_OS_WIN32
//    glewInit();
//#endif

//#ifdef GL_MULTISAMPLE
//    glEnable(GL_MULTISAMPLE);
//#endif

//    doneCurrent();

//    context()->moveToThread(static_cast<QThread*>(renderingThread));

    return NE_OK;
}

void QtNativeHostView::ready()
{
    qDebug() << "QtNativeHostView::ready() init surface w=" << width() << " h=" << height();
    m_renderThread->surface = new QOffscreenSurface();
    m_renderThread->surface->setFormat(m_renderThread->context->format());
    m_renderThread->surface->create();

    connect(window(), &QQuickWindow::sceneGraphInvalidated, m_renderThread, &RenderThreadQt::shutDown, Qt::QueuedConnection);

    m_width = (int)width();
    m_height = (int)height();
    m_renderThread->setSize(width(), height());
    m_renderThread->StartThread();
    update();
    m_nativeView->OnUIReady(m_renderThread);
}

QSGNode *QtNativeHostView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    TextureNode *node = static_cast<TextureNode *>(oldNode);

    if (!m_renderThread->context) {
        qDebug() << "QtNativeHostView::updatePaintNode init context";
        QOpenGLContext *current = window()->openglContext();
        // Some GL implementations requres that the currently bound context is
        // made non-current before we set up sharing, so we doneCurrent here
        // and makeCurrent down below while setting up our own context.
        current->doneCurrent();

        m_renderThread->context = new QOpenGLContext();
        m_renderThread->context->setFormat(current->format());
        m_renderThread->context->setShareContext(current);
        m_renderThread->context->create();
        m_renderThread->context->moveToThread(m_renderThread);

        current->makeCurrent(window());

        QMetaObject::invokeMethod(this, "ready");
        return 0;
    }

    if (!node) {
        qDebug() << "QtNativeHostView::updatePaintNode init node";
        node = new TextureNode(window());

        /* Set up connections to get the production of FBO textures in sync with vsync on the
         * rendering thread.
         *
         * When a new texture is ready on the rendering thread, we use a direct connection to
         * the texture node to let it know a new texture can be used. The node will then
         * emit pendingNewTexture which we bind to QQuickWindow::update to schedule a redraw.
         *
         * When the scene graph starts rendering the next frame, the prepareNode() function
         * is used to update the node with the new texture. Once it completes, it emits
         * textureInUse() which we connect to the FBO rendering thread's renderNext() to have
         * it start producing content into its current "back buffer".
         *
         * This FBO rendering pipeline is throttled by vsync on the scene graph rendering thread.
         */
        connect(m_renderThread, &RenderThreadQt::textureReady, node, &TextureNode::newTexture, Qt::DirectConnection);
        connect(node, &TextureNode::pendingNewTexture, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::prepareNode, Qt::DirectConnection);
        //connect(node, &TextureNode::textureInUse, m_renderThread, &RenderThreadQt::renderNext, Qt::QueuedConnection);

        // Get the production of FBO textures started..
        //QMetaObject::invokeMethod(m_renderThread, "renderNext", Qt::QueuedConnection);
    }

    node->setRect(boundingRect());

    return node;
}

void
QtNativeHostView::DrawFrame()
{
    if( m_mapViewUI != NULL )
    {
        uint32 cTime= PAL_ClockGetTimeMs();
        uint32 interval = cTime - m_lastUpdateTime;

        m_mapViewUI->UI_Render(interval);

        m_lastUpdateTime = cTime;
    }
}

void
QtNativeHostView::Render(const bool skipDrawFrame)
{
    DrawFrame();
    m_renderStarted = true;
}

void
QtNativeHostView::GetViewDimension(int& width, int& height)
{
    width = m_width;
    height = m_height;
}

void
QtNativeHostView::SetUIInterface(MapViewUIInterface* mapView)
{
    m_mapViewUI = mapView;
//    if(m_mapLegendWidget)
//    {
//        m_mapLegendWidget->SetMapUIInterface(m_mapViewUI);
//    }
    if( m_mapViewUI != NULL )
    {
        m_mapViewUI->UI_SetViewPort(m_width, m_height);
    }
}

NB_Error
QtNativeHostView::SetViewSize(int width, int height)
{
    NB_Error err = NE_OK;
    m_width = width;
    m_height = height;
    if(m_mapViewUI)
    {
        err = m_mapViewUI->UI_SetViewSize(width, height);
    }
    return err;
}

NB_Error
QtNativeHostView::SetViewPort(int width, int height)
{
    NB_Error err = NE_OK;

    m_width = width;
    m_height = height;
    if(m_mapViewUI)
    {
        err = m_mapViewUI->UI_SetViewPort(width, height);
    }
    return err;
}

NB_Error
QtNativeHostView::HandleViewGesture(MapView_GestureParameters* params)
{
    if(!m_mapNativeViewimpl || !m_mapViewUI || !params )
    {
        return NE_OK;
    }

    params->viewSize.x = width();
    params->viewSize.y = height();

    NB_Error err = NE_OK;
    if(m_mapNativeViewimpl->IsCameraLocked()
            && (params->gestureType == MapViewTransformation_Move
                || params->gestureType == MapViewTransformation_RotateAngle))
    {
        if( !IsGestureExtendUnlockThreshold(*params) ){
            return err;
        }
        err = m_mapViewUI->UI_HandleViewGesture(&m_gestureBeginParameters);
        m_mapNativeViewimpl->UnLockCameraPosition();
    }

    if(err == NE_OK)
    {
        err = m_mapViewUI->UI_HandleViewGesture(params);
    }
    return err;
}

void
QtNativeHostView::SetNativeRenderContext(NativeRenderContext* nativeContext)
{
    m_nativeContext = nativeContext;
}

bool QtNativeHostView::isAnisotropicSupported()
{
    const char* glExt = (const char*) glGetString(GL_EXTENSIONS);
    if (!glExt)
    {
        return false;
    }

    QString exts(glExt);
    if(exts.indexOf( ANISOTROPIC_FILTERING ) == -1)
    {
        return false;
    }
    return true;

}

void QtNativeHostView::initializeGL()
{
    //do nothing
}

void QtNativeHostView::resizeGL(int w, int h)
{
    //do nothing
}

void QtNativeHostView::paintGL()
{
    //do nothing
}

void QtNativeHostView::glInit()
{
    //do nothing
}

void QtNativeHostView::glDraw()
{
    //do nothing
}

void QtNativeHostView::initializeOverlayGL()
{
    //do nothing
}

void QtNativeHostView::resizeOverlayGL(int w, int h)
{
    //do nothing
}

void QtNativeHostView::paintOverlayGL()
{
    //do nothing
}

void QtNativeHostView::updateGL()
{
    //do nothing
}

void QtNativeHostView::updateOverlayGL()
{
    //do nothing
}

bool QtNativeHostView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
    {
//        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    else if(event->type() == QEvent::Resize)
    {
        m_width = m_Parent->width();
        m_height = m_Parent->height();
        //setGeometry(m_Parent->rect());
        SetViewSize(m_width, m_height);

        return true;
    }
    else if (event->type() == QEvent::TouchEnd)
    {
        QTouchEvent* touch = (QTouchEvent*)event;
        if (touch->touchPoints().size() == 1)
        {
            const float CLICK_THRESHOLD = 10.0f;
            const QTouchEvent::TouchPoint& point = touch->touchPoints().first();
            if (QLineF(point.pos(), point.startPos()).length() < CLICK_THRESHOLD)
            {
                ClickCompass(point.pos().x(), point.pos().y());
            }
        }
        m_gestureDetector->touchEvent(touch);
        event->accept();
        return true;
    }
    else if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchCancel)
    {
        QTouchEvent* touch = (QTouchEvent*)event;
        m_gestureDetector->touchEvent(touch);
        event->accept();
        return true;
    }

    return QQuickItem::event(event);
}


void QtNativeHostView::onPanStart(QPointF position)
{
    qDebug("pan start");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Move;
    parameters.state = MapView_GestureStateBegan;
    parameters.locationInView.x = position.x();
    parameters.locationInView.y = position.y();
    parameters.velocityInView.x = 0;
    parameters.velocityInView.y = 0;
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onPan(QPointF position)
{
    qDebug("pan move");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Move;
    parameters.state = MapView_GestureStateChanged;
    parameters.locationInView.x = position.x();
    parameters.locationInView.y = position.y();
    parameters.velocityInView.x = 0;
    parameters.velocityInView.y = 0;
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onPanEnd(QPointF position, QPointF velocity)
{
    qDebug("pan end");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Move;
    parameters.state = MapView_GestureStateEnded;
    parameters.locationInView.x = position.x();
    parameters.locationInView.y = position.y();
    parameters.velocityInView.x = velocity.x();
    parameters.velocityInView.y = velocity.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onLongPress(QPointF pos)
{
    qDebug("long press");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_LongPress;
    parameters.state = MapView_GestureStateEnded;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onTap(QPointF pos)
{
    qDebug("tap");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Tap;
    parameters.state = MapView_GestureStateEnded;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onDoubleTap(QPointF pos)
{
    qDebug("double tap");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_DoubleTap;
    parameters.state = MapView_GestureStateEnded;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onTwoFingerTap(QPointF pos)
{
    qDebug("two finger tap");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_TwoFingerTap;
    parameters.state = MapView_GestureStateEnded;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onZoomStart(QPointF pos)
{
    qDebug("zoom start");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Scale;
    parameters.state = MapView_GestureStateBegan;
    parameters.scale = 1.0f;
    parameters.scaleVelocity = 0;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onZoom(QPointF pos, float scale)
{
    qDebug("zoom move");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Scale;
    parameters.state = MapView_GestureStateChanged;
    parameters.scale = scale;
    parameters.scaleVelocity = 0;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onZoomEnd(QPointF pos, float scale, float speed)
{
    qDebug("zoom end");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_Scale;
    parameters.state = MapView_GestureStateEnded;
    parameters.scale = scale;
    parameters.scaleVelocity = speed;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onRotateStart(QPointF pos)
{
    qDebug("rotate start");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_RotateAngle;
    parameters.state = MapView_GestureStateBegan;
    parameters.rotaionAngle = 0;
    parameters.rotationVelocity = 0;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onRotate(QPointF pos, float angle)
{
    qDebug("rotate move");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_RotateAngle;
    parameters.state = MapView_GestureStateChanged;
    parameters.rotaionAngle = -angle;
    parameters.rotationVelocity = 0;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onRotateEnd(QPointF pos, float angle, float speed)
{
    qDebug("rotate end");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_RotateAngle;
    parameters.state = MapView_GestureStateEnded;
    parameters.rotaionAngle = -angle;
    parameters.rotationVelocity = speed;
    parameters.locationInView.x = pos.x();
    parameters.locationInView.y = pos.y();
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onTiltStart()
{
    qDebug("tilt start");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_TiltAngle;
    parameters.state = MapView_GestureStateBegan;
    parameters.tiltAngle = 0;
    parameters.locationInView.x = 0;
    parameters.locationInView.y = 0;
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onTilt(float pixels)
{
    qDebug("tilt move");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_TiltAngle;
    parameters.state = MapView_GestureStateChanged;
    parameters.tiltAngle = pixels;
    parameters.locationInView.x = 0;
    parameters.locationInView.y = 0;
    HandleViewGesture(&parameters);
}

void QtNativeHostView::onTiltEnd()
{
    qDebug("tilt end");
    MapView_GestureParameters parameters;
    parameters.gestureType = MapViewTransformation_TiltAngle;
    parameters.state = MapView_GestureStateEnded;
    parameters.tiltAngle = 0;
    parameters.locationInView.x = 0;
    parameters.locationInView.y = 0;
    HandleViewGesture(&parameters);
}

//MapView_GestureRecognizerState GetMapViewGestureType(QGesture* gesture)
//{
//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        return MapView_GestureStateBegan;
//    }
//    case Qt::GestureUpdated:
//    {
//        return MapView_GestureStateChanged;
//    }
//    case Qt::GestureFinished:
//    {
//        return MapView_GestureStateEnded;
//    }
//    case Qt::GestureCanceled:
//    {
//        return MapView_GestureStateCancelled;
//    }
//    default:
//    {
//        return MapView_GestureStatePossible;
//    }
//    }
//}

bool QtNativeHostView::gestureEvent(QGestureEvent *gestureEvent)
{
//    TiltGesture* tilt = (TiltGesture*)gestureEvent->gesture(m_tiltGestureType);
//    PanGesture* pan = (PanGesture*)gestureEvent->gesture(m_panGestureType);
//    SingleTapGesture* tap = (SingleTapGesture*)gestureEvent->gesture(m_tapGestureType);
//    DoubleTapGesture* doubleTap = (DoubleTapGesture*)gestureEvent->gesture(m_doubleTapGestureType);
//    TwoFingersTapGesture* tap2 = (TwoFingersTapGesture*)gestureEvent->gesture(m_twoFingerTapGestureType);
//    QTapAndHoldGesture* longPress = (QTapAndHoldGesture*)gestureEvent->gesture(Qt::TapAndHoldGesture);

//    MapView_GestureParameters parameters;

//    if (tilt && ((m_gestureType == Qt::CustomGesture && tilt->state() == Qt::GestureStarted)
//                 || m_gestureType == m_tiltGestureType))
//    {
//        parameters.state = GetMapViewGestureType(tilt);
//        parameters.locationInView.x = tilt->centerPoint().x();
//        parameters.locationInView.y = tilt->centerPoint().y();
//        if (tilt->motionState() == TiltGesture::Tilt)
//        {
//            parameters.gestureType = MapViewTransformation_TiltAngle;
//            parameters.tiltAngle = tilt->angle();
//        }
//        else if (tilt->motionState() == TiltGesture::Scale)
//        {
//            parameters.gestureType = MapViewTransformation_Scale;
//            parameters.scale = tilt->totalScaleFactor();
//            if (tilt->state() == Qt::GestureFinished)
//            {
//                parameters.scaleVelocity = tilt->scaleSpeed();
//            }
//            else
//            {
//                parameters.scaleVelocity = 0;
//            }
//        }
//        else if (tilt->motionState() == TiltGesture::Rotate)
//        {
//            parameters.gestureType = MapViewTransformation_RotateAngle;
//            parameters.rotaionAngle = -tilt->totalRotationAngle();
//            if (tilt->state() == Qt::GestureFinished)
//            {
//                parameters.rotationVelocity = tilt->rotateSpeed();
//            }
//            else
//            {
//                parameters.rotationVelocity = 0;
//            }
//        }

//        m_gestureType = m_tiltGestureType;
//    }
//    else if (pan && ((m_gestureType == Qt::CustomGesture && pan->state() == Qt::GestureStarted)
//                     || m_gestureType == m_panGestureType))
//    {
//        parameters.gestureType = MapViewTransformation_Move;
//        parameters.state = GetMapViewGestureType(pan);
//        parameters.locationInView.x = pan->offset().x();
//        parameters.locationInView.y = pan->offset().y();
//        if (pan->state() == Qt::GestureFinished)
//        {
//            parameters.velocityInView.x = pan->velocity().x();
//            parameters.velocityInView.y = pan->velocity().y();
//        }
//        else
//        {
//            parameters.velocityInView.x = 0;
//            parameters.velocityInView.y = 0;
//        }
//        m_gestureType = m_panGestureType;
//    }

//    if (longPress && longPress->state() == Qt::GestureFinished)
//    {
//        parameters.gestureType = MapViewTransformation_LongPress;
//        parameters.state = GetMapViewGestureType(longPress);
//        QPointF p = mapFromGlobal(longPress->position().toPoint());
//        parameters.locationInView.x = p.x();
//        parameters.locationInView.y = p.y();
//    }

//    if (tap && tap->state() == Qt::GestureFinished)
//    {
//        parameters.gestureType = MapViewTransformation_Tap;
//        parameters.state = GetMapViewGestureType(tap);
//        parameters.locationInView.x = tap->position().x();
//        parameters.locationInView.y = tap->position().y();
//    }

//    if (tap2 && tap2->state() == Qt::GestureFinished)
//    {
//        parameters.gestureType = MapViewTransformation_TwoFingerTap;
//        parameters.state = GetMapViewGestureType(tap2);
//        parameters.locationInView.x = tap2->position().x();
//        parameters.locationInView.y = tap2->position().y();
//    }
//    else if (doubleTap && doubleTap->state() == Qt::GestureFinished)
//    {
//        parameters.gestureType = MapViewTransformation_DoubleTap;
//        parameters.state = GetMapViewGestureType(doubleTap);
//        parameters.locationInView.x = doubleTap->position().x();
//        parameters.locationInView.y = doubleTap->position().y();
//    }

//    if (parameters.gestureType != MapViewTransformation_None)
//    {
//        HandleViewGesture(&parameters);
//    }

//    foreach (QGesture* gesture, gestureEvent->gestures())
//    {
//        if (m_gestureType != Qt::CustomGesture && gesture->gestureType() != m_gestureType)
//        {
//            gestureEvent->ignore(gesture->gestureType());
//        }

//        switch (gesture->state())
//        {
//        case Qt::GestureStarted:
//            break;
//        case Qt::GestureUpdated:
//            break;
//        case Qt::GestureFinished:
//        case Qt::GestureCanceled:
//            m_gestureType = Qt::CustomGesture;
//            break;
//        }
//    }
//    if (m_gestureType != Qt::CustomGesture && gestureEvent->gesture(m_gestureType) == NULL)
//    {
//        m_gestureType = Qt::CustomGesture;
//    }

    return true;
}

//void QtNativeHostView::panTriggered(PanGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_Move;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        return;
//    }
//    }

//    // @todo maybe need to verify xp() and yp()

//    parameters.velocityInView.x = 0;
//    parameters.velocityInView.y = 0;
//    parameters.locationInView.x = gesture->offset().x();
//    parameters.locationInView.y = gesture->offset().y();
//    // @todo to verify whether it need to judge compass-clicking

//    HandleViewGesture(&parameters);
//}

//void QtNativeHostView::pinchTriggered(QPinchGesture *gesture)
//{
//    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
//    if (changeFlags & QPinchGesture::RotationAngleChanged)
//    {
//        roateTriggered(gesture);
//    }

//    if (changeFlags & QPinchGesture::ScaleFactorChanged)
//    {
//        scaleTriggered(gesture);
//    }

//    if(changeFlags & QPinchGesture::CenterPointChanged)
//    {
//        // @todo need to make it clear
//    }
//}

//void QtNativeHostView::tapTriggered(QTapGesture *gesture)
//{
//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        if(m_firstDown == NULL)
//        {
//            m_firstDown = gesture;
//        }
//        else
//        {
//            if(m_secondDown != NULL)
//            {
//                m_firstDown = m_secondDown;
//            }

//            m_secondDown = gesture;
//            m_secondDownTime = PAL_ClockGetGPSTime();
//        }

//        if(m_firstDown != NULL && m_secondDown != NULL)
//        {
//            if(isConsideredDoubleTap())
//            {
//                m_isDoubleTapping = true;
//                doubleTapTriggered(gesture);
//            }
//            else
//            {
//                singleTapTriggered(gesture);
//            }
//        }
//        else
//        {
//            // This is a first tap
//            // @todo Set timer to process first tap, timeout : DOUBLE_TAP_TIMEOUT
//        }
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        m_firstUpTime = PAL_ClockGetGPSTime();
//        if(m_isDoubleTapping)
//        {
//            doubleTapTriggered(gesture);
//        }
//        else
//        {
//            singleTapTriggered(gesture);
//        }

//        m_isDoubleTapping = false;

//        break;
//    }
//    case Qt::GestureUpdated:
//    case Qt::GestureCanceled:
//    case Qt::NoGesture:
//    default:
//    {
//        break;
//    }
//    }

//}

//bool QtNativeHostView::isConsideredDoubleTap()
//{
//    if (m_secondDownTime - m_firstUpTime > DOUBLE_TAP_TIMEOUT)
//    {
//        return false;
//    }

//    int deltaX = (int) m_firstDown->position().x() - (int) m_secondDown->position().x();
//    int deltaY = (int) m_firstDown->position().x() - (int) m_secondDown->position().x();

//    return (deltaX * deltaX + deltaY * deltaY < DOUBLE_TAP_SLOP_SQUARE);
//}

//void QtNativeHostView::singleTapTriggered(QTapGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_Tap;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        break;
//    }
//    }

//    // @todo maybe need to verify xp() and yp()
//    parameters.locationInView.x = gesture->position().x();
//    parameters.locationInView.y = gesture->position().y();

//    HandleViewGesture(&parameters);
//}

//void QtNativeHostView::doubleTapTriggered(QTapGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_DoubleTap;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        break;
//    }
//    }

//    // @todo maybe need to verify xp() and yp()
//    parameters.locationInView.x = gesture->position().x();
//    parameters.locationInView.y = gesture->position().y();

//    HandleViewGesture(&parameters);
//}

//void QtNativeHostView::tapAndHoldTriggered(QTapAndHoldGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_LongPress;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        break;
//    }
//    }

//    QPointF point = mapFromGlobal(gesture->position().toPoint());

//    parameters.locationInView.x = point.x();
//    parameters.locationInView.y = point.y();

//    HandleViewGesture(&parameters);
//}

//void QtNativeHostView::roateTriggered(QPinchGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_RotateAngle;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        return;
//    }
//    }

//    // @todo maybe need to verify xp() and yp()
//    parameters.locationInView.x = gesture->hotSpot().x();
//    parameters.locationInView.y = gesture->hotSpot().y();

//    parameters.rotaionAngle += gesture->totalRotationAngle();
//    parameters.rotationVelocity = 10;// @todo hardcode, qt not supplying. Need to make it clear that what does rotationVelocity do

//    HandleViewGesture(&parameters);
//}

//void QtNativeHostView::scaleTriggered(QPinchGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_Scale;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        // @todo need make it clear that when m_scaleFactor is reset
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        return;
//    }
//    }

//    parameters.locationInView.x = gesture->hotSpot().x();
//    parameters.locationInView.y = gesture->hotSpot().y();

//    parameters.scale = gesture->totalScaleFactor();// @todo or m_scaleFactor, need to verify
//    parameters.scaleVelocity = 10;// @todo hardcode, qt not supplying. Need to make it clear that what does rotationVelocity do

//    HandleViewGesture(&parameters);
//}

//void QtNativeHostView::tiltTriggered(TiltGesture *gesture)
//{
//    MapView_GestureParameters parameters;
//    parameters.gestureType = MapViewTransformation_TiltAngle;

//    switch (gesture->state())
//    {
//    case Qt::GestureStarted:
//    {
//        parameters.state = MapView_GestureStateBegan;
//        break;
//    }
//    case Qt::GestureUpdated:
//    {
//        parameters.state = MapView_GestureStateChanged;
//        break;
//    }
//    case Qt::GestureFinished:
//    {
//        parameters.state = MapView_GestureStateEnded;
//        break;
//    }
//    case Qt::GestureCanceled:
//    {
//        parameters.state = MapView_GestureStateCancelled;
//        break;
//    }
//    case Qt::NoGesture:
//    default:
//    {
//        return;
//    }
//    }

//    parameters.tiltAngle = gesture->m_tiltAngle;

//    HandleViewGesture(&parameters);
//}


static float CalculateDistance(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return static_cast<float>(sqrt(dx*dx + dy*dy));
}

static float CalculateScaleFactor(float distance, int width, int height)
{
    return (1+ distance*2/sqrt(width*width + height*height));
}

static float CalculateAngle(float x, float y)
{
    return atan2(y, x) * (180 / M_PI);
}

//void QtNativeHostView::ResetGestureParameters(MapView_GestureParameters& parameters)
//{
//    parameters.gestureType = MapViewTransformation_None;
//    parameters.locationInView.x = 0.0f;
//    parameters.locationInView.y = 0.0f;
//    parameters.rotaionAngle = 0.0f;
//    parameters.rotationVelocity = 0.0f;
//    parameters.scale = 0.0f;
//    parameters.scaleVelocity = 0.0f;
//    parameters.state = MapView_GestureStatePossible;
//    parameters.tiltAngle = 0.0f;
//    parameters.velocityInView.x = 0.0f;
//    parameters.velocityInView.y = 0.0f;
//    parameters.viewSize.x = 0.0f;
//    parameters.viewSize.y = 0.0f;
//}

void QtNativeHostView::mousePressEvent(QMouseEvent* mouseEvent)
{
#ifndef SUPPORTS_QT_GESTURE
    m_mouseDown = true;
    m_mouseOriginalX = mouseEvent->x();
    m_mouseOriginalY = mouseEvent->y();
    PointTime currentPointTime;
    currentPointTime.pos = mouseEvent->pos();
    currentPointTime.time = QTime::currentTime();
    m_gestureMoveRecord.push_back( currentPointTime );

    //ResetGestureParameters(m_parameters);
    if(mouseEvent->modifiers() == Qt::NoModifier)
    {
        m_longPressTimer->start();
    }

    mouseEvent->accept();
#endif
}

void QtNativeHostView::mouseMoveEvent(QMouseEvent* mouseEvent)
{
#ifndef SUPPORTS_QT_GESTURE
    if(!m_mouseDown)
    {
        return;
    }

    m_longPressTimer->stop();

    if(!m_mouseMoveStarted)
    {
        m_parameters.state = MapView_GestureStateBegan;
        m_mouseMoveStarted = true;
    }
    else
    {
        m_parameters.state = MapView_GestureStateChanged;
    }

    if(mouseEvent->modifiers() == Qt::NoModifier)
    {
        // simulate pan gesture

        m_parameters.gestureType = MapViewTransformation_Move;
        m_parameters.locationInView.x = mouseEvent->x();
        m_parameters.locationInView.y = mouseEvent->y();

        PointTime currentPointTime;
        currentPointTime.pos = mouseEvent->pos();
        currentPointTime.time = QTime::currentTime();
        m_gestureMoveRecord.push_back( currentPointTime );
        if( m_gestureMoveRecord.size() > GESTURE_MOVE_CONTAINER_SIZE )
        {
            m_gestureMoveRecord.pop_front();
        }
    }
    else if(mouseEvent->modifiers() == Qt::ControlModifier)
    {
        // simulate tilt gesture
        m_parameters.gestureType = MapViewTransformation_TiltAngle;
        m_parameters.tiltAngle = ((mouseEvent->y() - m_mouseOriginalY) / m_height) * 180;
    }
    else if(mouseEvent->modifiers() == Qt::ShiftModifier)
    {
        // simulate scale gesture
        m_parameters.gestureType = MapViewTransformation_Scale;
        m_parameters.locationInView.x = m_mouseOriginalX;
        m_parameters.locationInView.y = m_mouseOriginalY;

        float distance = CalculateDistance(m_mouseOriginalX, m_mouseOriginalY,
                                           mouseEvent->x(), mouseEvent->y());

        float scaleFactor = CalculateScaleFactor(distance, m_width, m_height);

        if(mouseEvent->y() > m_mouseOriginalY)
        {
            m_parameters.scale = 3.0f * scaleFactor -2;
        }
        else
        {
            m_parameters.scale = 1 + 0.75*(1 - scaleFactor);
        }
    }
    else if(mouseEvent->modifiers() == (Qt::ControlModifier | Qt::AltModifier))
    {
        // simulate rotate gesture for linux platform


        m_parameters.gestureType = MapViewTransformation_RotateAngle;
        m_parameters.locationInView.x = m_mouseOriginalX;
        m_parameters.locationInView.y = m_mouseOriginalY;
        m_parameters.rotaionAngle = CalculateAngle(mouseEvent->y() - m_mouseOriginalY, mouseEvent->x() - m_mouseOriginalX);
    }
    else
    {
        return;
    }

    HandleViewGesture(&m_parameters);

    mouseEvent->accept();
#endif
}

void QtNativeHostView::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
#ifndef SUPPORTS_QT_GESTURE
    if(m_longPressTriggered)
    {
        m_longPressTriggered = false;
        return;
    }

    m_longPressTimer->stop();

    if(!m_mouseMoveStarted)
    {
//        if(ClickCompass(mouseEvent->x(), mouseEvent->y()))
//        {
//            return;
//        }

        m_parameters.gestureType = MapViewTransformation_Tap;
        m_parameters.locationInView.x = mouseEvent->x();
        m_parameters.locationInView.y = mouseEvent->y();
    }
    else
    {
        m_parameters.state = MapView_GestureStateEnded;
        if( m_parameters.gestureType == MapViewTransformation_Move )
        {
            if( m_gestureMoveRecord.size() > 0 )
            {
                int elapsed = m_gestureMoveRecord[0].time.msecsTo( QTime::currentTime() );
                if( elapsed > MINIMUM_TIME_GAP_FOR_CACULATE_MOUSE_VELOCITY )
                {
                    m_parameters.velocityInView.x = ( mouseEvent->x() - m_gestureMoveRecord[0].pos.x() ) * 1000 / elapsed;
                    m_parameters.velocityInView.y = ( mouseEvent->y() - m_gestureMoveRecord[0].pos.y() ) * 1000 / elapsed;
                }
            }

            m_gestureMoveRecord.clear();
        }
    }
    HandleViewGesture(&m_parameters);

    m_mouseMoveStarted = false;
    m_mouseDown = false;

    mouseEvent->accept();
#endif
}

void QtNativeHostView::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
#ifndef SUPPORTS_QT_GESTURE
    MapView_GestureParameters parameters;

    if(mouseEvent->modifiers() == Qt::ControlModifier)
    {
        parameters.gestureType = MapViewTransformation_TwoFingerTap;
        parameters.locationInView.x = mouseEvent->x();
        parameters.locationInView.y = mouseEvent->y();
    }
    else if(mouseEvent->modifiers() == Qt::NoModifier)
    {
        parameters.gestureType = MapViewTransformation_DoubleTap;
        parameters.locationInView.x = mouseEvent->x();
        parameters.locationInView.y = mouseEvent->y();
    }
    else
    {
        return;
    }

    HandleViewGesture(&parameters);

    mouseEvent->accept();
#endif
}

void QtNativeHostView::wheelEvent(QWheelEvent* event)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonDblClick,
                           event->pos(),
                           Qt::LeftButton,
                           Qt::NoButton,
                           Qt::NoModifier);
    if(event->delta() < 0)
    {
        mouseEvent.setModifiers(Qt::ControlModifier);
    }

    mouseDoubleClickEvent(&mouseEvent);
    event->accept();
}

void QtNativeHostView::paintEvent(QPaintEvent* e)
{
    if(m_mapViewUI && m_renderStarted)
    {
        DrawFrame();
    }
    //QGLWidget::paintEvent(e);
}

void QtNativeHostView::mouseLongPressTriggered()
{
    m_longPressTriggered = true;
    if(m_longPressTimer)
    {
        m_longPressTimer->stop();
    }
    MapView_GestureParameters parameters;

    parameters.gestureType = MapViewTransformation_LongPress;
    parameters.locationInView.x = m_mouseOriginalX;
    parameters.locationInView.y = m_mouseOriginalY;

    HandleViewGesture(&parameters);
}

void QtNativeHostView::ShowDebugView(bool show )
{
    emit SigShowDebugView( show );
}

void QtNativeHostView::SetDebugText( QString text )
{
    emit SigSetDebugText( text );
}

void QtNativeHostView::OnShowDebugView(bool show)
{
//    if( show == true )
//    {
//        m_pDebugText->show();
//    }
//    else
//    {
//        m_pDebugText->hide();
//    }
}

void QtNativeHostView::OnSetDebugText(QString text)
{
//    m_pDebugText->setText( text );
//    m_pDebugText->adjustSize();
//    update();
}

void QtNativeHostView::SetLocateMeButtonShowMode(NB_GpsMode mode)
{
    m_gpsMode = mode;

//    if(m_locateMeButton != NULL)
//    {
//        switch (m_gpsMode)
//        {
//        case NGM_STAND_BY:
//        {
//            m_locateMeButton->SetShowMode(LocateMeButton::SM_LOCATE_ME);;
//            break;
//        }
//        case NGM_FOLLOW_ME_ANY_HEADING:
//        {
//            m_locateMeButton->SetShowMode(LocateMeButton::SM_FOLLOW_ME_ANYHEADING);
//            break;
//        }
//        case NGM_FOLLOW_ME:
//        {
//            m_locateMeButton->SetShowMode(LocateMeButton::SM_FOLLOW_ME);
//            break;
//        }
//        default:
//        {
//            break;
//        }
//        }
//    }
}

void QtNativeHostView::ShowMapLegend(shared_ptr<nbmap::MapLegendInfo> mapLegend)
{
//    if(m_mapLegendWidget)
//    {
//        m_mapLegendWidget->ShowMapLegend(mapLegend);
//    }
}

void QtNativeHostView::HideMapLegend()
{
//    if(m_mapLegendWidget)
//    {
//        m_mapLegendWidget->HideMapLegend();
//    }
}

void QtNativeHostView::UpdateButtonState(bool animationCanPlay)
{
//    if(m_mapLegendWidget)
//    {
//        m_mapLegendWidget->UpdateButtonState(animationCanPlay);
//    }
}

void QtNativeHostView::UpdateTimestampLabel(uint32 gpsTime)
{
//    if(m_mapLegendWidget)
//    {
//        m_mapLegendWidget->UpdateTimestampLabel(gpsTime);
//    }
}

void QtNativeHostView::initializeMembers(MapViewProtocol* protocol)
{
    PAL_LockCreate(NULL, &m_lock);
    if( protocol->GetParent() )
    {
        QQuickItem* parent = protocol->GetParent();
        setSize(QSize(parent->width(), parent->height()));
        setParentItem(parent);
    }

    m_mapNativeViewimpl = protocol->GetMapNativeViewImpl();
    m_longPressTimer = new QTimer(this);
    m_longPressTimer->setInterval(LONG_PRESS_TIME_OUT);
    QObject::connect(m_longPressTimer, SIGNAL(timeout()), this, SLOT(mouseLongPressTriggered()));

    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_gestureDetector = new GestureDetector();

    // Added by priority desc
    PinchRecognizer* pinch = new PinchRecognizer(this);
    m_gestureDetector->addGestureRecognizer(pinch);
    connect(pinch, &PinchRecognizer::zoom, this, &QtNativeHostView::onZoom);
    connect(pinch, &PinchRecognizer::zoomStart, this, &QtNativeHostView::onZoomStart);
    connect(pinch, &PinchRecognizer::zoomEnd, this, &QtNativeHostView::onZoomEnd);
    connect(pinch, &PinchRecognizer::rotate, this, &QtNativeHostView::onRotate);
    connect(pinch, &PinchRecognizer::rotateStart, this, &QtNativeHostView::onRotateStart);
    connect(pinch, &PinchRecognizer::rotateEnd, this, &QtNativeHostView::onRotateEnd);
    connect(pinch, &PinchRecognizer::tilt, this, &QtNativeHostView::onTilt);
    connect(pinch, &PinchRecognizer::tiltStart, this, &QtNativeHostView::onTiltStart);
    connect(pinch, &PinchRecognizer::tiltEnd, this, &QtNativeHostView::onTiltEnd);

    TwoFingerTapRecognizer* twoFingerTap = new TwoFingerTapRecognizer(this);
    m_gestureDetector->addGestureRecognizer(twoFingerTap);
    connect(twoFingerTap, &TwoFingerTapRecognizer::twoFingerTap, this, &QtNativeHostView::onTwoFingerTap);

    PanRecognizer* pan = new PanRecognizer(this);
    m_gestureDetector->addGestureRecognizer(pan);
    connect(pan, &PanRecognizer::panStart, this, &QtNativeHostView::onPanStart);
    connect(pan, &PanRecognizer::pan, this, &QtNativeHostView::onPan);
    connect(pan, &PanRecognizer::panEnd, this, &QtNativeHostView::onPanEnd);

    TapRecognizer* tap = new TapRecognizer(this);
    m_gestureDetector->addGestureRecognizer(tap);
    connect(tap, &TapRecognizer::tap, this, &QtNativeHostView::onTap);
    connect(tap, &TapRecognizer::doubleTap, this, &QtNativeHostView::onDoubleTap);

    LongPressRecognizer* longPress = new LongPressRecognizer(this);
    m_gestureDetector->addGestureRecognizer(longPress);
    connect(longPress, &LongPressRecognizer::longPress, this, &QtNativeHostView::onLongPress);

//    if(!m_mapviewOnly)
//    {

//        m_pDebugText = new QLabel( this );
//        m_pDebugText->setStyleSheet( "background-color: rgba(255, 255, 255, 255);" );
//        m_pDebugText->setText( " " );
//        m_pDebugText->hide();

//        m_pTipInfo = new QLabel( this );
//        m_pTipInfo->setStyleSheet( "font-size:20px;color:rgba(255, 255, 255, 255);background-color: rgba(0, 0, 0, 255);" );
//        m_pTipInfo->setText( " " );
//        m_pTipInfo->hide();

//        connect( this, SIGNAL(SigShowDebugView(bool)), this, SLOT(OnShowDebugView(bool)) );
//        connect( this, SIGNAL(SigSetDebugText(QString)), this, SLOT(OnSetDebugText(QString)) );

//        m_locateMeButton = new LocateMeButton(this);
//        QObject::connect(m_locateMeButton, SIGNAL(clicked()), this, SLOT(SwitchGpsMode()));
//        m_locateMeButton->SetEnabled(true);
//        m_locateMeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

//        m_navZoomInButton = new NavigationOptionButton( this, "icon_zoom_in.png" );
//        m_navZoomOutButton = new NavigationOptionButton( this, "icon_zoom_out.png" );
//        m_navLayerOptionButton = new NavigationOptionButton( this, "icon_layers.png" );

//        m_mapLegendWidget = new MapLegendWidget(this);
//        m_mapLegendWidget->hide();
//        m_mapLegendWidget->setMaximumSize( m_mapLegendWidget->rect().size() );

//        m_topLayout = new QHBoxLayout;
//        m_topLayout->setAlignment( Qt::AlignCenter );
//        m_topLayout->addWidget( m_pTipInfo);

//        m_centerLayout = new QVBoxLayout;
//        m_centerLayout->setAlignment( Qt::AlignRight | Qt::AlignHCenter );
//        m_centerLayout->addWidget( m_navLayerOptionButton );
//        m_centerLayout->addWidget( m_navZoomInButton );
//        m_centerLayout->addWidget( m_navZoomOutButton );

//        QObject::connect( m_navLayerOptionButton, SIGNAL(clicked()), this, SLOT(OnChooseLayerClicked()) );
//        QObject::connect( m_navZoomInButton, SIGNAL(clicked()), this, SLOT(OnZoomInClicked()) );
//        QObject::connect( m_navZoomOutButton, SIGNAL(clicked()), this, SLOT(OnZoomOutClicked()) );

//        m_bottomLayout = new QHBoxLayout;
//        m_bottomLayout->setAlignment( Qt::AlignBottom );
//        m_bottomLayout->setMargin( m_mapLegendWidget->rect().height() );
//        m_bottomLayout->addWidget( m_locateMeButton );
//        m_bottomLayout->addStretch();
//        m_bottomLayout->addWidget( m_mapLegendWidget );
//        m_bottomLayout->addStretch();

//        m_mainLayout = new QVBoxLayout;
//        m_mainLayout->addLayout(m_topLayout);
//        m_mainLayout->addLayout(m_centerLayout);
//        m_mainLayout->addLayout(m_bottomLayout);
//        m_mainLayout->setStretch( 0, 1 );
//        m_mainLayout->setStretch( 1, 1 );
//        m_mainLayout->setStretch( 2, 1 );
//        setLayout( m_mainLayout );

//        if(m_mapNativeViewimpl)
//        {
//            locationtoolkit::MapDecorationSignals& decorationSignals = m_mapNativeViewimpl->GetMapDecorationSignals();
//            connect( &decorationSignals, SIGNAL(ShowLocateMeButton(bool)), this, SLOT(OnShowLocateMeButton(bool)) );
//            connect( &decorationSignals, SIGNAL(ShowZoomButton(bool)), this, SLOT(OnShowZoomButton(bool)) );
//            connect( &decorationSignals, SIGNAL(ShowLayerOptionButton(bool)), this, SLOT(OnShowLayerOptionButton(bool)) );
//        }
//    }

//    setAttribute(Qt::WA_AcceptTouchEvents);
//    m_panGesureRecognizer = new PanGestureRecognizer();
//    m_panGestureType = QGestureRecognizer::registerRecognizer(m_panGesureRecognizer);

//    m_tiltGesureRecognizer = new TiltGestureRecognizer();
//    m_tiltGestureType = QGestureRecognizer::registerRecognizer(m_tiltGesureRecognizer);

//    m_tapGesureRecognizer = new SingleTapGestureRecognizer();
//    m_tapGestureType = QGestureRecognizer::registerRecognizer(m_tapGesureRecognizer);

//    m_doubleTapGestureRecognizer = new DoubleTapGestureRecognizer();
//    m_doubleTapGestureType = QGestureRecognizer::registerRecognizer(m_doubleTapGestureRecognizer);

//    m_twoFingerTapRecognizer = new TwoFingersTapGestureRecognizer();
//    m_twoFingerTapGestureType = QGestureRecognizer::registerRecognizer(m_twoFingerTapRecognizer);

//    windowHandle()->installEventFilter(this);
//    grabGesture(m_panGestureType);
//    grabGesture(m_tiltGestureType);
//    grabGesture(m_tapGestureType);
//    grabGesture(m_doubleTapGestureType);
//    grabGesture(Qt::TapAndHoldGesture);
//    grabGesture(m_twoFingerTapGestureType);

//    show();
}

void QtNativeHostView::SwitchGpsMode()
{
    if(m_mapNativeViewimpl != NULL)
    {
        switch (m_gpsMode)
        {
        case NGM_STAND_BY:
        {
            m_mapNativeViewimpl->SetGpsMode(NGM_FOLLOW_ME_ANY_HEADING, true);
            break;
        }
        case NGM_FOLLOW_ME_ANY_HEADING:
        {
            m_mapNativeViewimpl->SetGpsMode(NGM_FOLLOW_ME, true);
            break;
        }
        case NGM_FOLLOW_ME:
        {
            m_mapNativeViewimpl->SetGpsMode(NGM_FOLLOW_ME_ANY_HEADING, true);
            break;
        }
        default:
        {
            break;
        }
        }
    }
}

void QtNativeHostView::OnShowLocateMeButton(bool show)
{
//    if(!m_locateMeButton)
//    {
//        return;
//    }
//    if( show )
//    {
//        m_locateMeButton->show();
//        m_locateMeButton->SetEnabled(true);
//    }
//    else
//    {
//        m_locateMeButton->hide();
//        m_locateMeButton->SetEnabled(false);
//    }
}

void QtNativeHostView::OnShowZoomButton(bool show)
{
//    if(!m_navZoomInButton || !m_navZoomOutButton)
//    {
//        return;
//    }
//    if( show )
//    {
//        m_navZoomInButton->show();
//        m_navZoomOutButton->show();
//    }
//    else
//    {
//        m_navZoomInButton->hide();
//        m_navZoomOutButton->hide();
//    }
}

void QtNativeHostView::OnShowLayerOptionButton(bool show)
{
//    if(!m_navLayerOptionButton)
//    {
//        return;
//    }
//    if( show )
//    {
//        m_navLayerOptionButton->show();
//    }
//    else
//    {
//        m_navLayerOptionButton->hide();
//    }
}

void QtNativeHostView::OnZoomInClicked()
{
    if(m_mapNativeViewimpl)
    {
        m_mapNativeViewimpl->ZoomIn();
    }
}

void QtNativeHostView::OnZoomOutClicked()
{
    if(m_mapNativeViewimpl)
    {
        m_mapNativeViewimpl->ZoomOut();
    }
}

void QtNativeHostView::OnChooseLayerClicked()
{
//    if(m_mapNativeViewimpl)
//    {
//        m_mapNativeViewimpl->ShowLayerOption();
//    }
}

bool QtNativeHostView::IsCompassHitted(float screenX, float screenY)
{
    float leftBottomX = 0.0f;
    float leftBottomY = 0.0f;
    float rightTopX = 0.0f;
    float rightTopY = 0.0f;
    m_mapViewUI->UI_GetCompassBoundingBox(leftBottomX, leftBottomY,rightTopX, rightTopY);

    float compassX = m_width - 64;
    float compassY = 64;

    if(screenX >= leftBottomX && screenX <= rightTopX &&
            screenY >= leftBottomY && screenY <= rightTopY)
    {
        return true;
    }

    return false;
}


static bool floatEqual(float f1, float f2, float epsilon)
{
    return (f2 >= f1-epsilon && f2 <= f1+epsilon);
}

bool
QtNativeHostView::IsNorth(float angle)
{
    while(angle >= 360.0f)
    {
        angle -= 360.0f;
    }

    while(angle < 0.0f)
    {
        angle += 360.0f;
    }

    return floatEqual(angle, 0.f, 0.01f) || floatEqual(angle, 360.f, 0.01f);
}

bool QtNativeHostView::ClickCompass(float screenX, float screenY)
{
    if(IsCompassHitted(screenX, screenY))
    {
        if(m_mapNativeViewimpl)
        {
            m_mapNativeViewimpl->OnCompassClicked(m_mapViewUI->UI_GetCameraRotationAngle());
        }

        return true;
    }

    return false;
}

bool QtNativeHostView::IsGestureExtendUnlockThreshold(const MapView_GestureParameters &params)
{
    if( params.state == MapView_GestureStateBegan )
    {
        m_gestureBeginParameters = params;
    }
    else if( params.state == MapView_GestureStateChanged )
    {
        if( params.gestureType == MapViewTransformation_Move )
        {
            int x = m_gestureBeginParameters.locationInView.x;
            int y = m_gestureBeginParameters.locationInView.y;
            double trueLength = nsl_sqrt(nsl_pow( params.locationInView.x - x, 2) +
                                     nsl_pow( params.locationInView.y - y, 2));
            if( trueLength > m_width / 10 )
            {
                return true;
            }
        }
        else if( params.gestureType == MapViewTransformation_RotateAngle )
        {
            float r = m_gestureBeginParameters.rotaionAngle;
            if( params.rotaionAngle - r > UNLOCK_CAMERA_ANGLE_THRESHOLD ||
                params.rotaionAngle - r < -UNLOCK_CAMERA_ANGLE_THRESHOLD )
            {
                return true;
            }
        }
    }

    return false;
}

void QtNativeHostView::ShowToolTip(const QString& tip)
{
//    if( m_pTipInfo )
//    {
//        if(  tip.isNull() )
//        {
//            m_pTipInfo->hide();
//        }
//        else
//        {
//            m_pTipInfo->setText( tip );
//            m_pTipInfo->adjustSize();
//            m_pTipInfo->show();
//        }
//    }
}

bool QtNativeHostView::eventFilter(QObject *obj, QEvent *ev)
{
 /*   if (m_nativeContext)
    {
        if (obj == windowHandle()) {
            switch (ev->type()) {
            case QEvent::Hide: qWarning() << Q_FUNC_INFO << "hide"; m_nativeContext->EnterBackground(true); break;
            case QEvent::Show: qWarning() << Q_FUNC_INFO << "show"; break;
            case QEvent::Expose: qWarning() << Q_FUNC_INFO << "expose"; m_nativeContext->EnterBackground(false); break;
            default: break;
            }
        }
    }
    return QGLWidget::eventFilter(obj, ev);
*/
	return true;
}

void QtNativeHostView::SetNativeView(MapNativeView* v)
{
    m_nativeView = v;
}
void QtNativeHostView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry.size() != oldGeometry.size() && !oldGeometry.size().isNull())
    {
        m_width = m_Parent->width();
        m_height = m_Parent->height();
        m_renderThread->setSize(width(), height());
        SetViewSize(m_width, m_height);
    }
}
/*! @} */

