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

/*
 * nbgmgesturehandler.h.h
 *
 *  Created on: May 8, 2013
 *      Author: btian
 */

#ifndef GESTUREHANDLER_H_
#define GESTUREHANDLER_H_
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <fcntl.h>
#include <img/img.h>
#include <math.h>
#include <screen/screen.h>
#include <string.h>
#include <bb/cascades/Container>
#include <bb/cascades/PinchHandler>


class NBGM_MapView;

struct MapViewCameraStatus
{
	double centerX;
	double centerY;
	double distance;
	float rotate;
	float tilt;
};

class NBGM_GestureHandler
{
public:
	NBGM_GestureHandler(NBGM_MapView& mapView);
	~NBGM_GestureHandler();

public:
	void OnScreenEvent(bb::cascades::TouchEvent* event);
	void onDoubleTap();



    void onPinchStart(bb::cascades::PinchEvent*);
    void onPinchUpdate(bb::cascades::PinchEvent*);
    void onPinchEnd(bb::cascades::PinchEvent*);
    void onPinchCancel();

private:
    void onRotateStart();
    void onRotateUpdate(float angle);
    void onRotateEnd(float angle);

    void onScaleStart();
    void onScaleUpdate(float scale);
    void onScaleEnd(float scale);

    void onTiltStart();
    bool onTiltUpdate(float angle);
    void onTiltEnd(float angle);



	void OnPan(int x, int y);
	void OnTwoFingerTap();

private:

	NBGM_MapView& mMapView;
	MapViewCameraStatus mCameraStatus;

    bool mPinchFlag;
	bool mTouched;
    bool mTiltFlag;
    float mTouchPointY;
};


#endif /* GESTUREHANDLER_H_ */
