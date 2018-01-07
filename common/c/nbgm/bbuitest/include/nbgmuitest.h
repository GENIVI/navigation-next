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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */


#ifndef _NBGM_UI_TEST_H_
#define _NBGM_UI_TEST_H_

#include <bb/cascades/Application>
#include <screen/screen.h>
#include <QThread>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>

#include "pal.h"
#include "palevent.h"
#include "nbgm.h"
#include "nbgmmapview.h"
#include "snapscreenshotcallback.h"
#include <bb/cascades/Container>
#include <bb/cascades/DoubleTapHandler>
#include <bb/cascades/PinchHandler>
#include <bb/cascades/NavigationPane>

class NBGM_Egl;
class NBGM_GestureHandler;
class TileManager;
class QTimer;

class NBGMUITest: public QObject
{
     Q_OBJECT
public:
    NBGMUITest();
    virtual  ~NBGMUITest();

public:
    bool Initialize(bb::cascades::NavigationPane* navPane);
    void Finalize();

private:
    bool InitializeMapView();
    void FinalizeMapView();

    void UpdateTiles();

public slots:
    void onOpenNBMFile();
    void onUnloadAllFile();
    void onScreenshot();

    void onTouchEvent(bb::cascades::TouchEvent* event);
    void onDoubleTap(bb::cascades::DoubleTapEvent*);
    void onPinchStart(bb::cascades::PinchEvent*);
    void onPinchUpdate(bb::cascades::PinchEvent*);
    void onPinchEnd(bb::cascades::PinchEvent*);
    void onPinchCancel();
    void onBackClickedAction();

    void onFileSelected(const QStringList&);

    void onTimer();

private:
    friend class InitMapViewTask;
    friend class FinalizeMapViewTask;

    bb::cascades::NavigationPane* mNavigationPane;

    screen_context_t        mScreenCtx;
    NBGM_GestureHandler*    mGestureHandler;

    NBGM_Egl*               mEGL;
    PAL_Instance*           mPalInstance;
    PAL_Event*              mInitializeEvent;
    PAL_Event*              mFinalizeEvent;
    PAL_Event*              mEixtEvent;
    NBGM_Instance*          mNBGMInstance;

    NBGM_MapView*           mMapView;
    TaskQueue*              mRenderTaskQueue;

    std::list<std::string>  mLoadTile;
    bool                    mTiltFlag;
    float                   mTouchPointY;

    TileManager*            mTileManager;
    bool                    mResetMapCenter;

    SnapScreenshotCallback  mSnapScreenshotCallback;

    QTimer*                 mTimer;
    bool                    mTileInvalide;
};

#endif // ifndef _NBGM_UI_TEST_H_
