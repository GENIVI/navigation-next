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

 @file gpscontext.h
 @date 9/13/10

 GPS context

 */
/*
 (C) Copyright 2010 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#import <Foundation/Foundation.h>
#import "abpalgps.h"
#import "gpsprotocol.h"



typedef enum
{
    Gps_Simulation,
    Gps_Real,

}GPSFileType;


@interface GPSContext : NSObject <GpsContextProtocol,GpsContextCallBackProtocol>{

    ABPAL_GpsDeviceStateCallback*       stateChangeCallBack;
    ABPAL_GpsInitializeCallback*        initlizeCallBack;
    ABPAL_GpsLocationCallback*          gpsCallBack;

    PAL_Instance*                       pal_Instance;
    const void*                         userData;
    ABPAL_GpsCriteria                   palcriteria;
    GPSFileType                         gps_Type;

    id<GpsManagerProtocol>              gpsManager;

    NSTimer*                            timer;
    NSTimer*                            timeout_timer;
    ABPAL_GpsState                      gpsState;

    BOOL                                hasFix;
    ABPAL_GpsLocation*                  lastLocation;
    PAL_Error                           lastLocationError;
    char*                               lastLocationInfo;

    NSTimeInterval                      trackingInterval;

    BOOL                                hasCallBack; // for tracking
    BOOL                                firstTimeTracking;  // only for tracking
}

@property (nonatomic, assign) ABPAL_GpsDeviceStateCallback *stateChangeCallBack;
@property (nonatomic, assign) ABPAL_GpsInitializeCallback  *initlizeCallBack;
@property (nonatomic, assign) ABPAL_GpsLocationCallback   *gpsCallBack;
@property (nonatomic, assign) ABPAL_GpsCriteria  palcriteria;
@property (nonatomic, assign) const void *userData;
@property (nonatomic, retain) NSTimer*                            timer;
@property (nonatomic, retain) NSTimer*                            timeout_timer;
@property BOOL                                 isCurrentMagneticHeading;
@property BOOL                                 stopUseInvalidMagneticHeading;
@property double                               currentCompassHeading;
@property double                               currentCompassHeadingAccuracy;

-(id)init:(PAL_Instance*) pal;
-(double)headingFromLocation:(ABPAL_GpsLocation*)newLocation criteria:(ABPAL_GpsCriteria)criteria isResultHeadingValid:(BOOL*)isResultHeadingValid;

@end
