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
 * gesturehandler.cpp
 *
 *  Created on: May 8, 2013
 *      Author: btian
 */
#include "nbgmgesturehandler.h"
#include "nbgmmapview.h"
#include <bb/cascades/PinchEvent>
#include "nbremath.h"

static NBGM_GestureHandler* gInstance = NULL;
static const float Max_Angle = 60;
using namespace bb::cascades;

NBGM_GestureHandler::NBGM_GestureHandler(NBGM_MapView& mapView):mMapView(mapView)
{
	mMapView.GetViewCenter(mCameraStatus.centerX, mCameraStatus.centerY);
	mCameraStatus.distance = mMapView.GetViewPointDistance();
	mCameraStatus.rotate = 0;
    mCameraStatus.tilt = 0;
    mTouched = false;
    mPinchFlag = false;
}

NBGM_GestureHandler::~NBGM_GestureHandler()
{
}

void NBGM_GestureHandler::OnScreenEvent(bb::cascades::TouchEvent* event)
{
    if(mPinchFlag)
    {
        mTouched = false;
        return;
    }
	switch(event->touchType())
	{
	case bb::cascades::TouchType::Down:
		{
		    mTouched = true;
			mMapView.OnTouchEvent(event->localX(), event->localY());
			mMapView.Invalidate();

		}
			break;
	case bb::cascades::TouchType::Move:
		{
		    if(mTouched)
		    {
                OnPan(event->localX(), event->localY());

		    }
		}
		break;
    case bb::cascades::TouchType::Up:
        {
            mTouched = false;
        }
        break;
	default:
		break;
	}
}


void NBGM_GestureHandler::onPinchStart(PinchEvent* event)
{
    mPinchFlag = true;
    if(NBRE_Math::Abs(event->touchPointY(0)-event->touchPointY(1)) < 100)
    {
        mTiltFlag = true;
        mTouchPointY = event->midPointY();
    }
}

void NBGM_GestureHandler::onPinchUpdate(PinchEvent* event)
{
    onScaleUpdate(event->pinchRatio());
    onRotateUpdate(event->rotation());
    if(mTiltFlag)
    {

        if(!onTiltUpdate((event->midPointY() - mTouchPointY)/5))
        {
            mTouchPointY = event->midPointY();
        }
    }
}

void NBGM_GestureHandler::onPinchEnd(PinchEvent* event)
{
    mPinchFlag = false;
    onRotateEnd(event->rotation());
    onScaleEnd(event->pinchRatio());
    if(mTiltFlag)
    {
        onTiltEnd((event->midPointY() - mTouchPointY)/5);

    }
}

void NBGM_GestureHandler::onPinchCancel()
{
    mPinchFlag = false;
    mMapView.SetViewPointDistance(mCameraStatus.distance);
    mMapView.SetRotateAngle(mCameraStatus.rotate);
    mMapView.SetRotateAngle(mCameraStatus.tilt);
}




void NBGM_GestureHandler::OnPan(int x, int y)
{
	mMapView.OnPaning(static_cast<float>(x), static_cast<float>(y));
	mMapView.Invalidate();
}

void NBGM_GestureHandler::onDoubleTap()
{
	mCameraStatus.distance /= 2.0f;
	mMapView.SetViewPointDistance(mCameraStatus.distance);
	mMapView.Invalidate();
}

void NBGM_GestureHandler::OnTwoFingerTap()
{
	mCameraStatus.distance *= 2.0f;
	mMapView.SetViewPointDistance(mCameraStatus.distance);
	mMapView.Invalidate();
}



void NBGM_GestureHandler::onScaleUpdate(float scaleFactor)
{
    mMapView.SetViewPointDistance(mCameraStatus.distance/scaleFactor);
    mMapView.Invalidate();
}

void NBGM_GestureHandler::onRotateUpdate(float angle)
{
    mMapView.SetRotateAngle(mCameraStatus.rotate+angle);
    mMapView.Invalidate();
}

bool NBGM_GestureHandler::onTiltUpdate(float angle)
{
    if(mCameraStatus.tilt+angle < 0)
    {
        onTiltEnd(angle);
        return false;

    }
    if(mCameraStatus.tilt+angle > Max_Angle)
    {
        onTiltEnd(angle);
        return false;
    }
    mMapView.SetTiltAngle(mCameraStatus.tilt+angle);
    mMapView.Invalidate();
    return true;
}

void NBGM_GestureHandler::onScaleEnd(float scale)
{
    mCameraStatus.distance /= scale;
}

void NBGM_GestureHandler::onRotateEnd(float angle)
{
    mCameraStatus.rotate += angle;
}

void NBGM_GestureHandler::onTiltEnd(float angle)
{
    if(mCameraStatus.tilt+angle < 0)
    {
        mCameraStatus.tilt = 0;
        mMapView.SetTiltAngle(mCameraStatus.tilt);
        return;

    }

    if(mCameraStatus.tilt+angle > Max_Angle)
    {
        mCameraStatus.tilt = Max_Angle;
        mMapView.SetTiltAngle(mCameraStatus.tilt);
        return;
    }

    mCameraStatus.tilt += angle;

}


