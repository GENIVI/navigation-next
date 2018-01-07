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

static NBGM_GestureHandler* gInstance = NULL;

NBGM_GestureHandler::NBGM_GestureHandler(NBGM_MapView& mapView):mMapView(mapView), mFingerTouchCount(0), mHandleTap(false)
{
	mMapView.GetViewCenter(mCameraStatus.centerX, mCameraStatus.centerY);
	mCameraStatus.distance = mMapView.GetViewPointDistance();
	gInstance = this;
	InitGestures();
}

NBGM_GestureHandler::~NBGM_GestureHandler()
{
	Cleanup();
}

void NBGM_GestureHandler::OnScreenEvent(screen_event_t screen_event)
{
	int screen_val = SCREEN_EVENT_NONE;
	int rc = screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE, &screen_val);
	if (screen_val == SCREEN_EVENT_MTOUCH_TOUCH)
	{
		int pos[2];
		screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION, pos);
		mMapView.OnTouchEvent(static_cast<float>(pos[0]), static_cast<float>(pos[1]));

		mHandleTap = (++mFingerTouchCount == 1 );
	}
	if (screen_val == SCREEN_EVENT_MTOUCH_RELEASE)
	{
		mHandleTap = false;
		mFingerTouchCount--;
	}
	if (screen_val == SCREEN_EVENT_MTOUCH_TOUCH
			|| screen_val == SCREEN_EVENT_MTOUCH_MOVE
			|| screen_val == SCREEN_EVENT_MTOUCH_RELEASE)
	{
		mtouch_event_t mtouch_event;
		rc = (rc != 0)? rc:screen_get_mtouch_event(screen_event, &mtouch_event, 0);
		rc = (rc != 0)? rc:gestures_set_process_event(set, &mtouch_event, NULL);
		if(rc == 0 && screen_val == SCREEN_EVENT_MTOUCH_MOVE && mHandleTap)
		{
			int pos[2];
			screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION, pos);
			OnPan(pos[0], pos[1]);
		}
	}
}

void NBGM_GestureHandler::gesture_callback(gesture_base_t* gesture, mtouch_event_t* event, void* /*param*/, int async)
{
	NBGM_GestureHandler* self = (NBGM_GestureHandler*)(gInstance);
	self->OnGesture(gesture, event, async);
}

void NBGM_GestureHandler::InitGestures()
{
	set = gestures_set_alloc();
	if (NULL != set)
	{
		//setup pan
		tap_params_t tap_param;
		tap_param.max_displacement = 25;
		tap_param.max_hold_ms = 100;
		tap_gesture_alloc(&tap_param, gesture_callback, set);

		//setup double tap
		double_tap_params_t dp_param;
		dp_param.max_delay_ms = 300;
		dp_param.max_displacement = 25;
		dp_param.max_hold_ms = 120;
		double_tap_gesture_alloc(&dp_param, gesture_callback, set);

		//setup two finger tap
		tft_params_t tft_params;
		tft_params.max_release_interval = 15;
		tft_params.max_touch_interval = 15;
		tft_params.max_displacement = 25;
		tft_params.max_tap_time = 120;
		tft_gesture_alloc(&tft_params, gesture_callback, set);


//		tfpan_gesture_alloc(NULL, gesture_callback, set);
//		pinch_gesture_alloc(NULL, gesture_callback, set);
//		swipe_gesture_alloc(NULL, gesture_callback, set);
//		long_press_gesture_alloc(NULL, gesture_callback, set);
//		pt_gesture_alloc(NULL, gesture_callback, set);
//		rotate_gesture_alloc(NULL, gesture_callback, set);
//		triple_tap_gesture_alloc(NULL, gesture_callback, set);
	}
}

void NBGM_GestureHandler::Cleanup()
{
	if (NULL != set)
	{
		gestures_set_free(set);
		set = NULL;
	}
}

void NBGM_GestureHandler::OnGesture(gesture_base_t* gesture, mtouch_event_t* event, int async)
{
	if(async)
	{
		return;
	}
    switch (gesture->type)
    {
	case GESTURE_TWO_FINGER_PAN:
		{
			printf("GESTURE_TWO_FINGER_PAN\n");
		}
		break;
	case GESTURE_ROTATE:
		{
			printf("GESTURE_ROTATE\n");
		}
		break;
	case GESTURE_SWIPE:
		{
			printf("GESTURE_SWIPE\n");
		}
		break;
	case GESTURE_PINCH:
		{
			printf("GESTURE_PINCH\n");
		}
		break;
	case GESTURE_TAP:
		{
			printf("GESTURE_TAP\n");
		}
		break;
	case GESTURE_DOUBLE_TAP:
		{
			mCameraStatus.distance /= 1.2f;
			mMapView.SetViewPointDistance(mCameraStatus.distance);
			printf("GESTURE_DOUBLE_TAP\n");
		}
		break;
	case GESTURE_TRIPLE_TAP:
		{
			printf("GESTURE_TRIPLE_TAP\n");
		}
		break;
	case GESTURE_PRESS_AND_TAP:
		{
			printf("GESTURE_PRESS_AND_TAP\n");
		}
		break;
	case GESTURE_TWO_FINGER_TAP:
		{
			mCameraStatus.distance *= 1.2f;
			mMapView.SetViewPointDistance(mCameraStatus.distance);
			printf("GESTURE_TWO_FINGER_TAP\n");
		}
		break;
	case GESTURE_LONG_PRESS:
		{
			printf("GESTURE_LONG_PRESS\n");
		}
		break;
	default:
		printf("unknown gesture\n");
		break;
    }
    fflush(stdout);
}

void NBGM_GestureHandler::OnPan(int x, int y)
{
	mMapView.OnPaning(static_cast<float>(x), static_cast<float>(y));
}

