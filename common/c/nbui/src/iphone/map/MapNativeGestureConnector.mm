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

#import "MapNativeGestureConnector.h"
#include "MapViewUIInterface.h"

@interface MapNativeGestureConnector()
{
    MapViewUIInterface* gestureMapViewUI; /* pointer on MapViewUIInterface */
    MapView_GestureParameters* gestureParameters;
}
@end

@implementation MapNativeGestureConnector

- (id)init:(id)mapViewUI
{
    self = [super init];
    if (self == nil)
    {
        return nil;
    }

    gestureParameters = new MapView_GestureParameters;
    memset(gestureParameters, 0, sizeof(MapView_GestureParameters));

    gestureMapViewUI = (__bridge MapViewUIInterface*)mapViewUI;

    return self;
}

- (void)handleViewGesture:(GestureParameters*)parameters
{
    memset(gestureParameters, 0, sizeof(MapView_GestureParameters));

    switch (parameters->gestureType)
    {
        case TT_None:
        {
            gestureParameters->gestureType = MapViewTransformation_None;
            break;
        }
        case TT_Move:
        {
            gestureParameters->gestureType = MapViewTransformation_Move;
            break;
        }
        case TT_TiltAngle:
        {
            gestureParameters->gestureType = MapViewTransformation_TiltAngle;
            break;
        }
        case TT_RotateAngle:
        {
            gestureParameters->gestureType = MapViewTransformation_RotateAngle;
            break;
        }
        case TT_Scale:
        {
            gestureParameters->gestureType = MapViewTransformation_Scale;
            break;
        }
        case TT_Tap:
        {
            gestureParameters->gestureType = MapViewTransformation_Tap;
            break;
        }
        case TT_DoubleTap:
        {
            gestureParameters->gestureType = MapViewTransformation_DoubleTap;
            break;
        }
        case TT_TwoFingerTap:
        {
            gestureParameters->gestureType = MapViewTransformation_TwoFingerTap;
            break;
        }
        case TT_LongPress:
        {
            gestureParameters->gestureType = MapViewTransformation_LongPress;
            break;
        }
        default:
        {
            gestureParameters->gestureType = MapViewTransformation_None;
            break;
        }
    }

    switch (parameters->state)
    {
        case GSP_Possible:
        {
            gestureParameters->state = MapView_GestureStatePossible;
            break;
        }
        case GSP_Began:
        {
            gestureParameters->state = MapView_GestureStateBegan;
            break;
        }
        case GSP_Changed:
        {
            gestureParameters->state = MapView_GestureStateChanged;
            break;
        }
        case GSP_Ended:
        {
            gestureParameters->state = MapView_GestureStateEnded;
            break;
        }
        case GSP_Cancelled:
        {
            gestureParameters->state = MapView_GestureStateCancelled;
            break;
        }
        case GSP_Failed:
        {
            gestureParameters->state = MapView_GestureStateFailed;
            break;
        }
        case GSP_Recognized:
        {
            gestureParameters->state = MapView_GestureStateRecognized;
            break;
        }
        default:
        {
            gestureParameters->state = MapView_GestureStateFailed;
            break;
        }
    }

    gestureParameters->scale = parameters->scale;
    gestureParameters->rotaionAngle = parameters->rotaionAngle;
    gestureParameters->tiltAngle = parameters->tiltAngle;

    gestureParameters->locationInView.x = parameters->locationInView.x;
    gestureParameters->locationInView.y = parameters->locationInView.y;

    gestureParameters->velocityInView.x = parameters->velocityInView.x;
    gestureParameters->velocityInView.y = parameters->velocityInView.y;

    gestureParameters->scaleVelocity = parameters->scaleVelocity;
    gestureParameters->rotationVelocity = parameters->rotationVelocity;

    gestureParameters->viewSize.x = parameters->viewSize.x;
    gestureParameters->viewSize.y = parameters->viewSize.y;

    gestureMapViewUI->UI_HandleViewGesture(gestureParameters);
}

- (void)dealloc
{
    delete gestureParameters;
    [super dealloc];
}

@end
