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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

typedef enum TransformationType
{
    TT_None = 0,
    TT_Move = 1,
    TT_TiltAngle = 2,
    TT_RotateAngle = 3,
    TT_Scale = 4,
    TT_Tap = 5,
    TT_DoubleTap = 6,
    TT_TwoFingerTap = 7,
    TT_LongPress = 8
} TransformationType;

typedef enum GestureRecognizerState
{
    GSP_Possible = 0,
    GSP_Began = 1,
    GSP_Changed = 2,
    GSP_Ended = 3,
    GSP_Cancelled = 4,
    GSP_Failed = 5,
    GSP_Recognized = 6
} GestureRecognizerState;

typedef struct GestureParameters
{
    TransformationType        gestureType;
    GestureRecognizerState    state;
    CGPoint                   locationInView;
    CGPoint                   velocityInView;
    CGPoint                   viewSize;
    float                     scale;
    float                     rotaionAngle;
    float                     tiltAngle;
    float                     scaleVelocity;
    float                     rotationVelocity;
}GestureParameters;

@interface MapNativeGestureConnector : NSObject

- (id)init:(id)mapViewUI;
- (void)handleViewGesture:(GestureParameters*)parameters;
- (void)dealloc;

@end

