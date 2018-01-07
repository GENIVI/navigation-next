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

 @file gpscontext.m
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

#import "gpscontext.h"
#import "realgpssingleton.h"
#import "filegpsmanager.h"
#import "gpsconfiguration.h"


const uint InitlizeTimeInterval = 0.25; // initlize time interval

static ABPAL_GpsLocation latestLocation;
static bool              latestLocationValid = NO;

@interface GPSContext()

@property (retain) NSTimer* invalidHeadingTimer;

-(void)invalidHeadingTimerExpired:(NSTimer*) timer;
-(void)invalidateHeadingTimer;

@end

@implementation GPSContext

@synthesize stateChangeCallBack;
@synthesize initlizeCallBack;
@synthesize gpsCallBack;
@synthesize userData;
@synthesize palcriteria;
@synthesize timer;
@synthesize timeout_timer;
@synthesize isCurrentMagneticHeading;
@synthesize stopUseInvalidMagneticHeading;
@synthesize currentCompassHeading;
@synthesize currentCompassHeadingAccuracy;
@synthesize invalidHeadingTimer;

-(id)init:(PAL_Instance*)pal
{
    if (self=[super init])
    {
        gpsState = PGS_Undefined;
        hasFix = NO;
        pal_Instance = pal; // for simulation gps
        stateChangeCallBack = nil;
        initlizeCallBack = nil;
        gpsCallBack = nil;
        userData = nil;
        gpsManager = nil;
        timer = nil;
        timeout_timer = nil;
        lastLocationInfo = nil;
        lastLocation = nil;
        trackingInterval = 0;
    }
    return self;
}

-(void) dealloc
{
    if (gps_Type == Gps_Simulation)
    {
        [gpsManager cancelGetLocation];
        [gpsManager endTracking];
        [gpsManager release];
    }

    delete []lastLocationInfo;

    if (nil != lastLocation) {
        free(lastLocation);
    }

    if (timer)
    {
        [timer invalidate];
        [timer release];
    }
    if (timeout_timer)
    {
        [timeout_timer invalidate];
        [timeout_timer release];
    }

    [super dealloc];
}

-(void) stateChanged:(NSTimer*) ptimer
{
    switch (gpsState)
    {
        case PGS_Initialized:
        {
            if (timeout_timer)
            {
                self.timeout_timer = nil;
            }
            self.initlizeCallBack(self.userData, PAL_Ok,nil);
        }
            break;

        case PGS_Tracking:
        {
            if (timeout_timer)
            {
                self.timeout_timer = nil;
            }

            if(firstTimeTracking && latestLocationValid)
            {
                firstTimeTracking = false;
                lastLocationError = PAL_Ok;
                self.gpsCallBack(self.userData, PAL_Ok, &latestLocation, nil);
            }
            else
            {
                lastLocationError = PAL_ErrGpsTimeout;
                self.gpsCallBack(self.userData, PAL_ErrGpsTimeout, nil, nil);
            }
        }
            break;

        case PGS_Oneshot:
        {
            if (timeout_timer)
            {
                self.timeout_timer = nil;
            }

            [gpsManager cancelRequest:self];
            if(latestLocationValid)
            {
                self.gpsCallBack(self.userData, PAL_Ok, &latestLocation, nil);
            }
            else
            {
                self.gpsCallBack(self.userData, PAL_ErrGpsTimeout, nil, nil);
            }
        }
            break;

        default:
            break;
    }
}

-(void) trackingTimerExpried:(NSTimer*) ptimer
{
    if (timer == ptimer)
    {
        if ((hasFix || self.stopUseInvalidMagneticHeading) && lastLocation)
        {
            if (gps_Type == Gps_Real)
            {
                BOOL isResultHeadingValid = NO;
                lastLocation->gpsHeading = lastLocation->heading;
                lastLocation->compassHeading = self.currentCompassHeading;
                lastLocation->compassAccuracy = self.currentCompassHeadingAccuracy;
                lastLocation->heading = [self headingFromLocation:lastLocation criteria:self.palcriteria isResultHeadingValid:&isResultHeadingValid];
                if (isResultHeadingValid)
                {
                    lastLocation->valid |= PGV_Heading;
                }
                else
                {
                    //resetting PGV_Heading flag value of lastLocation->valid to FALSE
                    lastLocation->valid &= ~(0 | PGV_Heading);
                }
            }
            self.gpsCallBack(self.userData, PAL_Ok, lastLocation, nil);
            hasFix = NO;
        }
        else if (lastLocationError != PAL_Ok)
        {
            self.gpsCallBack(self.userData, lastLocationError, nil, nil);
            hasFix = NO;
        }
    }
}
-(double)headingFromLocation:(ABPAL_GpsLocation*)newLocation criteria:(ABPAL_GpsCriteria)criteria isResultHeadingValid:(BOOL*)isResultHeadingValid
{
    double resultHeading = 0;
    if (newLocation->horizontalVelocity >= criteria.desiredHeadingSpeedLimitation)
    {
        //nav & fmm
        if (newLocation->heading >= 0)
        {
            //use gps heading
            resultHeading = newLocation->heading;
            *isResultHeadingValid = YES;
            self.isCurrentMagneticHeading = NO;
            self.stopUseInvalidMagneticHeading = NO;
            //stop timer
            [self invalidateHeadingTimer];
        }
        else
        {
            *isResultHeadingValid = NO;
        }
    }
    else
    {
        self.isCurrentMagneticHeading = YES;
        //we use this value as additional flag for enabling\disabling showing of unreliable heading
        if (criteria.desiredCalibrationPopup == YES)
        {
            //follow me map mode
            if (self.currentCompassHeadingAccuracy >= 0.0)
            {
                //use compass heading
                *isResultHeadingValid = YES;
                resultHeading = self.currentCompassHeading;
                self.stopUseInvalidMagneticHeading = NO;
                //stop timer
                [self invalidateHeadingTimer];
            }
            else
            {
                //invalid heading was received
                if (self.stopUseInvalidMagneticHeading)
                {
                    resultHeading = INVALID_HEADING;
                    *isResultHeadingValid = NO;
                }
                else
                {
                    *isResultHeadingValid = YES;
                    //start timer
                    if (!self.invalidHeadingTimer || ![self.invalidHeadingTimer isValid])
                    {
                        self.invalidHeadingTimer = [NSTimer timerWithTimeInterval:INVALID_HEADING_TIMEOUT
                                                                           target:self
                                                                         selector:@selector(invalidHeadingTimerExpired:)
                                                                         userInfo:nil
                                                                          repeats:NO];
                        [[NSRunLoop currentRunLoop] addTimer:self.invalidHeadingTimer forMode:NSRunLoopCommonModes];
                    }
                    resultHeading = self.currentCompassHeading;
                }
            }
        }
        else
        {
            //navigation
            //here we shall use magnetic heading only regardless of the reliability value
            resultHeading = self.currentCompassHeading;
            *isResultHeadingValid = YES;
        }
    }
    return resultHeading;
}

#pragma mark -
#pragma mark Invalid Heading Timer Methods

- (void)invalidHeadingTimerExpired:(NSTimer*) timer
{
    [self invalidateHeadingTimer];
    self.stopUseInvalidMagneticHeading = YES;
}

-(void)invalidateHeadingTimer
{
    if (invalidHeadingTimer)
    {
        [invalidHeadingTimer invalidate];
        self.invalidHeadingTimer = nil;
    }
}

#pragma mark -
#pragma mark GpsContextCallBackProtocol

-(void) locationCallBack:(ABPAL_GpsLocation *) location
               errorCode:(PAL_Error) error
             errorDetail:(const char*) errorInfo
{
    lastLocationError = error;

    if (nil!= lastLocationInfo)
    {
        delete []lastLocationInfo;
        lastLocationInfo = nil;
    }

    if (nil != errorInfo)
    {
        int length = strlen(errorInfo);
        lastLocationInfo = new char[length+1];
        memset(lastLocationInfo, 0, length+1);
        memcpy(lastLocationInfo, errorInfo, length);
    }

    if (nil != lastLocation)
    {
        free(lastLocation);
        lastLocation = nil;
    }

    if(nil != location)
    {
        lastLocation = (ABPAL_GpsLocation*)(malloc(sizeof(ABPAL_GpsLocation)));
        memset(lastLocation, 0, sizeof(ABPAL_GpsLocation));
        memcpy(lastLocation, location, sizeof(ABPAL_GpsLocation));
        hasFix = YES;
    }

    if(location)
    {
        latestLocation = *location;
        latestLocationValid = YES;
    }
    firstTimeTracking = NO;

    if (gpsState == PGS_Oneshot)
    {

        if (timeout_timer)
        {
            [timeout_timer invalidate];
            self.timeout_timer = nil;
        }
        self.gpsCallBack(self.userData, error, location, errorInfo);

        if (gps_Type == Gps_Real)
        {
            [gpsManager cancelRequest:self];
        }
    }
    else // tracking gps
    {
        if (timeout_timer)
        {
            [timeout_timer invalidate];
            self.timeout_timer = nil;
        }

        if ([gpsManager isServiceEnable])
        {
            self.timeout_timer = [NSTimer timerWithTimeInterval:trackingInterval
                                                         target:self
                                                       selector:@selector(stateChanged:)
                                                       userInfo:nil
                                                        repeats:NO];
            [[NSRunLoop currentRunLoop] addTimer:self.timeout_timer forMode:NSRunLoopCommonModes];
        }
    }

}

-(void) headingCallBack:(double) heading
               accuracy:(double) accuracy
{
    self.currentCompassHeading = heading;
    self.currentCompassHeadingAccuracy = accuracy;
    if (lastLocationError == PAL_Ok)
    {
        hasFix = YES;
    }
}

-(ABPAL_GpsCriteria) palCriteria
{
    return self.palcriteria;
}

-(ABPAL_GpsState) requestState
{
    return gpsState;
}
#pragma mark -
#pragma mark GpsContextProtocol

-(PAL_Error)initGpsConfig:(const GpsConfiguration *) config
         initlizeCallBack:(ABPAL_GpsInitializeCallback *) userCallback
      deviceStateCallback:(ABPAL_GpsDeviceStateCallback *) stateCallback
                 userData:(const void *) data
{

    PAL_Error result = PAL_Failed;

    if (gpsManager != nil)
    {
        result = PAL_ErrGpsInvalidState;
    }
    else
    {
        self.initlizeCallBack = userCallback;
        self.stateChangeCallBack = stateCallback;
        self.userData = data;

        if ([config providerType] == GpsProviderTypeGpsId)
        {
            gps_Type = Gps_Real;
            gpsManager = RealGpsFactory::getInstance(pal_Instance);
        }
        else
        {
            gps_Type = Gps_Simulation;
            gpsManager = id<GpsManagerProtocol>([[FileGpsManager alloc] init:pal_Instance]);
            latestLocationValid = NO;
            if (gpsManager)
            {
                result = [gpsManager setSimulationConfig:config];
            }
        }

        if (nil != gpsManager)
        {
            result = PAL_Ok;
        }

        gpsState = PGS_Initialized;

        self.timeout_timer = [NSTimer timerWithTimeInterval:(InitlizeTimeInterval)
                                                     target:self
                                                   selector:@selector(stateChanged:)
                                                   userInfo:nil
                                                    repeats:NO];
        [[NSRunLoop currentRunLoop] addTimer:self.timeout_timer forMode:NSRunLoopCommonModes];
    }
    return result;
}

-(ABPAL_GpsState) handlerState
{
    return gpsState;
}

-(PAL_Error) getLocation:(const ABPAL_GpsCriteria *) criteria
        locationCallback:(ABPAL_GpsLocationCallback *) callBack
                userData:(const void *) data
{
    PAL_Error result = PAL_ErrBadParam;

    if (gpsState != PGS_Initialized)
    {
        return PAL_ErrNoInit;
    }

    self.palcriteria = *criteria;
    self.gpsCallBack = callBack;
    self.userData = data;
    gpsState = PGS_Oneshot;

    [gpsManager addObserver:self];
    result = [gpsManager start];

    // Only start a timeout timer when service enabled
    if ([gpsManager isServiceEnable])
    {
        self.timeout_timer = [NSTimer timerWithTimeInterval:(palcriteria.desiredTimeout)
                                                     target:self
                                                   selector:@selector(stateChanged:)
                                                   userInfo:nil
                                                    repeats:NO];
        [[NSRunLoop currentRunLoop] addTimer:self.timeout_timer forMode:NSRunLoopCommonModes];
    }
    return result;
}

-(PAL_Error) cancelGetLocation
{
    PAL_Error result = PAL_Ok;

    if (timeout_timer)
    {
        [timeout_timer invalidate];
        self.timeout_timer = nil;
    }

    if (timer)
    {
        [timer invalidate];
        self.timer = nil;
    }

    if(gps_Type == Gps_Simulation)
    {
        result = [gpsManager cancelGetLocation];
        [gpsManager addObserver:nil];
    }
    else
    {
        result= [gpsManager cancelRequest:self];
    }


    return result;
}

-(PAL_Error) beginTracking:(const ABPAL_GpsCriteria *) criteria
          locationCallback:(ABPAL_GpsLocationCallback *) callBack
                  userData:(const void *) data
              trackingInfo:(ABPAL_GpsTrackingInfo *) trackInfo
{
    PAL_Error result = PAL_ErrBadParam;

    if (gpsState != PGS_Initialized)
    {
        return PAL_ErrNoInit;
    }

    self.palcriteria = *criteria;
    self.gpsCallBack = callBack;
    self.userData = data;
    trackInfo->Id = 1; //todo

    gpsState = PGS_Tracking;

    [gpsManager addObserver:self];
    result = [gpsManager start];

    trackingInterval = self.palcriteria.desiredTimeout;
    if ([gpsManager isServiceEnable])
    {
        self.timeout_timer = [NSTimer timerWithTimeInterval:trackingInterval
                                                     target:self
                                                   selector:@selector(stateChanged:)
                                                   userInfo:nil
                                                    repeats:NO];
        [[NSRunLoop currentRunLoop] addTimer:self.timeout_timer forMode:NSRunLoopCommonModes];
    }
    self.timer = [NSTimer timerWithTimeInterval:self.palcriteria.desiredInterval
                                         target:self
                                       selector:@selector(trackingTimerExpried:)
                                       userInfo:nil
                                        repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:self.timer forMode:NSRunLoopCommonModes];
    firstTimeTracking = YES;

    return result;
}

-(PAL_Error) endTracking:(const ABPAL_GpsTrackingInfo *) trackintInfo
{
    PAL_Error result = PAL_Ok;

    if (timer)
    {
        [timer invalidate];
        self.timer =nil;
    }

    if (timeout_timer)
    {
        [timeout_timer invalidate];
        self.timeout_timer =nil;
    }
    if(gps_Type == Gps_Simulation)
    {
        result = [gpsManager endTracking];
        [gpsManager addObserver:nil];
    }
    else
    {
        result= [gpsManager cancelRequest:self];
    }

    return result;
}

-(PAL_Error) gpsSusPend:(const ABPAL_GpsTrackingInfo *) trackintInfo
{
    PAL_Error result = PAL_Ok;

    //todo

    return result;
}

-(PAL_Error) gpsResume:(const ABPAL_GpsTrackingInfo *) trackintInfo
{
    PAL_Error result = PAL_Ok;

    // todo
    return result;
}

-(PAL_Error) updateCriteria:(const ABPAL_GpsCriteria *) criteria trackingInfo:(const ABPAL_GpsTrackingInfo *) trackInfo
{
    PAL_Error result = PAL_Ok;

    self.palcriteria = *criteria;
    [gpsManager updateGpsCriteria:palcriteria];

    if (gpsState == PGS_Tracking)
    {
        if (gps_Type == Gps_Real)
        {
            [gpsManager resetCompass];
        }
        if(trackingInterval != palcriteria.desiredTimeout)
        {
            trackingInterval = palcriteria.desiredTimeout;
            if (timeout_timer)
            {
                [timeout_timer invalidate];
                self.timeout_timer = nil;
            }

            if ([gpsManager isServiceEnable])
            {
                self.timeout_timer = [NSTimer timerWithTimeInterval:palcriteria.desiredTimeout
                                                             target:self
                                                           selector:@selector(stateChanged:)
                                                           userInfo:nil
                                                            repeats:NO];
                [[NSRunLoop currentRunLoop] addTimer:self.timeout_timer forMode:NSRunLoopCommonModes];
            }
        }

        if (timer)
        {
            [timer invalidate];
            self.timer = nil;
        }
        self.timer = [NSTimer timerWithTimeInterval:self.palcriteria.desiredInterval
                                             target:self
                                           selector:@selector(trackingTimerExpried:)
                                           userInfo:nil
                                            repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:self.timer forMode:NSRunLoopCommonModes];
    }

    return result;
}

@end
