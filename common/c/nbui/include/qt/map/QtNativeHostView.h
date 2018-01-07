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
 @file     QtNativeHostView.h
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

#ifndef _QT_NATIVE_HOST_VIEW_H_
#define _QT_NATIVE_HOST_VIEW_H_

#include <QtGlobal>
#ifdef Q_OS_WIN32
#include "GL/glew.h"
#endif
//#ifdef Q_OS_UNIX
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
//#endif
//#include <QGLWidget>
#include <QQuickItem>
#include <QtCore/QMutex>
#include <QtCore/QThread>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>

#include <QtQuick/QQuickWindow>
#include <qsgsimpletexturenode.h>

//#include <QLabel>
#include <QTime>
#include "NativeRenderContext.h"
#include "MapViewUIInterface.h"
#include "paltypes.h"
#include "pallock.h"
#include "nbgmrendercontext.h"
#include "MapViewInterface.h"
//#include "maplegendwidget.h"
//#include "navigationoptionbutton.h"

//#include "pangesturerecognizer.h"
//#include "TiltGestureRecognizer.h"
//#include "singletapgesturerecognizer.h"
//#include "doubletapgesturerecognizer.h"
//#include "twofingerstapgesturerecognizer.h"

class PanGesture;
class QGestureEvent;
class QPanGesture;
class QPinchGesture;
class QTapGesture;
class QTapAndHoldGesture;
class TiltGesture;
class TiltGestureRecognizer;
class QTimer;
class LocateMeButton;
class MapViewProtocol;
class MapNativeViewImpl;
class PanGestureRecognizer;

class MapNativeView;
class RenderThreadQt;
class GestureDetector;

struct PointTime
{
    QPoint pos;
    QTime time;
};

/*
    Define Host view to place map on Qt
*/
class QtNativeHostView : public QQuickItem
{
    Q_OBJECT

    static const int DOUBLE_TAP_TIMEOUT = 300;// millisecond
    static const int DOUBLE_TAP_SLOP_SQUARE = 100 * 100;
    static const int LONG_PRESS_TIME_OUT = 500;// millisecond

    /** Invalid latitude value, the camera will ignore this value*/
    static const double INVALID_LATITUED;
    /** Invalid longitude value, the camera will ignore this value*/
    static const double INVALID_LONGTITUED;
    /** Invalid zoomLevel value, the camera will ignore this value*/
    static const double INVALID_ZOOMLEVEL_VALUE;
    /** Invalid heading value, the camera will ignore this value*/
    static const double INVALID_HEADING_VALUE;
    /** Invalid tilt value, the camera will ignore this value*/
    static const double INVALID_TILT_VALUE;

    static const int COMPASS_ANIMATION_TIME = 500;

public:
    /*param:void* pDisplayType:
    The value of native system's window ID,in egl initialization process, API eglGetDisplay need this value as its parameter,
    normally, some systems(e.g. x11) can use an EGL macro named EGL_DEFAULT_DISPLAY to represent this value,in this case,"pDisplayType" should be NULL.
    but some other systems(e.g. wayland) will need a specific pointer, in this case, "pDisplayType" need a pointer value.
    param:NBGM_RenderSystemType renderSystemType
    The NBGM_RenderSystemType is defined in the file nbgmrendersystemtype.h(in nbgm module).
    */
    QtNativeHostView(bool enableAnisotropicFiltering, bool mapviewOnly, MapViewProtocol* protocol, void* renderThread);
    QtNativeHostView(bool enableAnisotropicFiltering, bool mapviewOnly, MapViewProtocol* protocol);
    ~QtNativeHostView();


public:
    /*! Initialize Host View. */
    NB_Error Initialize(nb_threadId renderingThread);
    /*! Draw a Frame. */
    void DrawFrame();
    /*! Render Map. */
    void Render(bool skipDrawFrame);
    /*! Get Host View's size. */
    void GetViewDimension(int& width, int& height);
    /*! Set Map's UI. */
    void SetUIInterface(MapViewUIInterface* mapView);
    /*! Set Host View's Size. */
    NB_Error SetViewSize(int width, int height);
    /*! Set Map's ViewPort. */
    NB_Error SetViewPort(int width, int height);
    /*! Set OpenGL Render Context. */
    void SetNativeRenderContext(NativeRenderContext* nativeContext);

    void ShowDebugView( bool show );
    void SetDebugText( QString text );

    void SetLocateMeButtonShowMode(NB_GpsMode mode);

    void ShowMapLegend(shared_ptr <nbmap::MapLegendInfo> mapLegend);
    void HideMapLegend();

    void UpdateButtonState(bool animationCanPlay);
    void UpdateTimestampLabel(uint32 gpsTime);

    void ShowToolTip(const QString& tip);

    void SetNativeView(MapNativeView* v);

Q_SIGNALS:
    void SigShowDebugView(bool);
    void SigSetDebugText(QString);
    void SigRenderContextReady();

    /*! Overwrite QGLWidget Virtual Interfaces. */
private:
    void initializeMembers( MapViewProtocol* protocol );
    bool isAnisotropicSupported();
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void glInit();
    void glDraw();
    void initializeOverlayGL();
    void resizeOverlayGL(int w, int h);
    void paintOverlayGL();
    void updateGL();
    void updateOverlayGL();
    bool event(QEvent *event);
    bool gestureEvent(QGestureEvent *event);
    virtual void paintEvent(QPaintEvent* e);

    void mousePressEvent(QMouseEvent* mouseEvent);
    void mouseMoveEvent(QMouseEvent* mouseEvent);
    void mouseReleaseEvent(QMouseEvent* mouseEvent);
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent);
    void wheelEvent(QWheelEvent* event);

private Q_SLOTS:
    void mouseLongPressTriggered();
    void OnShowDebugView(bool show);
    void OnSetDebugText(QString text);
    void SwitchGpsMode();
    // slots for signals in MapDecorationSignals
    void OnShowLocateMeButton( bool show );
    void OnShowZoomButton( bool show );
    void OnShowLayerOptionButton( bool show );
    // slots for zoom in/out and choose layer buttons
    void OnZoomInClicked();
    void OnZoomOutClicked();
    void OnChooseLayerClicked();

    void onPan(QPointF position);
    void onPanStart(QPointF position);
    void onPanEnd(QPointF position, QPointF velocity);
    void onLongPress(QPointF pos);
    void onTap(QPointF pos);
    void onDoubleTap(QPointF pos);
    void onTwoFingerTap(QPointF pos);
    void onZoomStart(QPointF pos);
    void onZoom(QPointF pos, float scale);
    void onZoomEnd(QPointF pos, float scale, float speed);
    void onRotateStart(QPointF pos);
    void onRotate(QPointF pos, float angle);
    void onRotateEnd(QPointF pos, float angle, float speed);
    void onTiltStart();
    void onTilt(float pixels);
    void onTiltEnd();

private:
    /*! Process Pan Gesture. */
    void panTriggered(PanGesture *gesture);

    /*! Process Pinch Gesture. */
    void pinchTriggered(QPinchGesture *gesture);

    /*! Process Tap Gesture. */
    void tapTriggered(QTapGesture *tap);

    /*! Process Single Tap Gesture. */
    void singleTapTriggered(QTapGesture *gesture);

    /*! Process Double Tap Gesture. */
    void doubleTapTriggered(QTapGesture *gesture);

    /*! Process Tap And Hold Gesture. */
    void tapAndHoldTriggered(QTapAndHoldGesture *tapAndHold);

    /*! Process Rotate Gesture. */
    void roateTriggered(QPinchGesture *gesture);

    /*! Process Scale Gesture. */
    void scaleTriggered(QPinchGesture *gesture);

    /*! Process Tilt Gesture. */
    void tiltTriggered(TiltGesture *tilt);

    /*! Judge It Is Single Tap or Double Tap Gesture. */
    bool isConsideredDoubleTap();

    /*! Process Map's Gesture. */
    NB_Error HandleViewGesture(MapView_GestureParameters* params);

    /*! Reset Map's Gesture Parameters. */
    void ResetGestureParameters(MapView_GestureParameters& parameters);

    bool IsCompassHitted(float screenX, float screenY);

    bool IsNorth(float angle);

    bool ClickCompass(float screenX, float screenY);

    bool IsGestureExtendUnlockThreshold( const MapView_GestureParameters& params );

    bool eventFilter(QObject *obj, QEvent *ev);

    static QList<QThread *> threads;

public Q_SLOTS:
    void ready();

protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
private:
    RenderThreadQt *m_renderThread;

private:
    MapViewUIInterface*         m_mapViewUI;

    // GL Surface width/height
    int                         m_width;
    int                         m_height;

    uint32                      m_lastUpdateTime;

    PAL_Lock*                   m_lock;

    NativeRenderContext*        m_nativeContext;


    uint32                      m_firstUpTime;
    uint32                      m_secondDownTime;
    QTapGesture*                m_firstDown;
    QTapGesture*                m_secondDown;

    bool                        m_isDoubleTapping;

    float                       m_mouseOriginalX;
    float                       m_mouseOriginalY;
    QTime                       m_cacuVelocityTime;
    bool                        m_mouseDown;
    bool                        m_mouseMoveStarted;
    bool                        m_renderStarted;
    QTimer*                     m_longPressTimer;
    bool                        m_longPressTriggered;
    MapView_GestureParameters   m_parameters;
//    QLabel*                     m_pDebugText;
    QQuickItem*                    m_Parent;
//    LocateMeButton*             m_locateMeButton;
    NB_GpsMode                  m_gpsMode;
//    MapLegendWidget*            m_mapLegendWidget;
    bool                        m_enableAnisotropicFiltering;
    bool                        m_mapviewOnly;
//    QLabel*                     m_pTipInfo;
//    QHBoxLayout*                m_topLayout;
//    QVBoxLayout*                m_centerLayout;
//    QHBoxLayout*                m_bottomLayout;
//    QVBoxLayout*                m_mainLayout;
    MapNativeViewImpl*          m_mapNativeViewimpl;
    MapView_GestureParameters   m_gestureBeginParameters;

//    NavigationOptionButton*     m_navZoomInButton;
//    NavigationOptionButton*     m_navZoomOutButton;
//    NavigationOptionButton*     m_navLayerOptionButton;
    QVector<PointTime>          m_gestureMoveRecord;

//    Qt::GestureType             m_tiltGestureType;
//    TiltGestureRecognizer*      m_tiltGesureRecognizer;

//    Qt::GestureType             m_panGestureType;
//    PanGestureRecognizer*       m_panGesureRecognizer;

//    Qt::GestureType             m_tapGestureType;
//    SingleTapGestureRecognizer* m_tapGesureRecognizer;

//    Qt::GestureType             m_doubleTapGestureType;
//    DoubleTapGestureRecognizer* m_doubleTapGestureRecognizer;

//    Qt::GestureType                 m_twoFingerTapGestureType;
//    TwoFingersTapGestureRecognizer* m_twoFingerTapRecognizer;

    GestureDetector*            m_gestureDetector;

    Qt::GestureType             m_gestureType;
    MapNativeView*              m_nativeView;

};

class TextureNode : public QObject, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    TextureNode(QQuickWindow *window)
        : m_id(0)
        , m_size(0, 0)
        , m_texture(0)
        , m_window(window)
    {
        // Our texture node must have a texture, so use the default 0 texture.
        m_texture = m_window->createTextureFromId(0, QSize(1, 1));
        setTexture(m_texture);
        setFiltering(QSGTexture::Linear);
        setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    }

    ~TextureNode()
    {
        delete m_texture;
    }

signals:
    void textureInUse();
    void pendingNewTexture();

public slots:

    // This function gets called on the FBO rendering thread and will store the
    // texture id and size and schedule an update on the window.
    void newTexture(int id, const QSize &size) {
        m_mutex.lock();
        m_id = id;
        m_size = size;
        m_mutex.unlock();

        // We cannot call QQuickWindow::update directly here, as this is only allowed
        // from the rendering thread or GUI thread.
        emit pendingNewTexture();
    }


    // Before the scene graph starts to render, we update to the pending texture
    void prepareNode() {
        m_mutex.lock();
        int newId = m_id;
        QSize size = m_size;
        m_id = 0;
        m_mutex.unlock();
        if (newId) {
            delete m_texture;
            // note: include QQuickWindow::TextureHasAlphaChannel if the rendered content
            // has alpha.
            m_texture = m_window->createTextureFromId(newId, size);
            setTexture(m_texture);

            markDirty(DirtyMaterial);

            // This will notify the rendering thread that the texture is now being rendered
            // and it can start rendering to the other one.
            emit textureInUse();
        }
    }

private:

    int m_id;
    QSize m_size;

    QMutex m_mutex;

    QSGTexture *m_texture;
    QQuickWindow *m_window;
};

#endif /* _QT_NATIVE_HOST_VIEW_H_ */
/*! @} */
