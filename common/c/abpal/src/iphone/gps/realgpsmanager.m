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

 @file realgpsmamaner.m
 @date 9/13/10

 location manager of iPhone

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

#import "realgpsmanager.h"

#define NOTIFYING_DISPERSION    5


const int KArrayCount=10;


@implementation RealGpsManager

@synthesize contextArray;
@synthesize prevCompassHeading;

-(id)init:(PAL_Instance*) pal
{
    if (self = [super init])
    {
        gpsManager = nil;
        gpsShutdown = NO;
        gpsDofix = FALSE;
        gpsDoingfix = FALSE;
        contextArray = [[NSMutableArray alloc] initWithCapacity:KArrayCount];
        positionSource = PPS_sourceAny;

        // Get system version, should use double to save it, if we use float will lost some accuracy, e.g. return 4.199999998
        iOSVersion = [[[UIDevice currentDevice] systemVersion] doubleValue];

        NSDateFormatter* startDateFormatter = [[NSDateFormatter alloc] init];
        NSTimeZone* gmt = [NSTimeZone timeZoneWithName:@"GMT"];
        [startDateFormatter setTimeZone:gmt];

        //get gps start date 01-06-1980
        [startDateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss"];
        gpsStartDate = [[startDateFormatter dateFromString:@"1980-01-06 00:00:00"] retain];
        [startDateFormatter release];

        // Create lock
        PAL_Error err = PAL_LockCreate(pal, &pal_lock);

        //Get CCC thread
        currentThread = [[NSThread currentThread] retain];

        if(err != PAL_Ok || !pal_lock)
        {
            [self release];
            self = nil;
        }
    }
    return self;
}

-(void) addObserver:(id<GpsContextCallBackProtocol>) observer
{
    PAL_LockLock(pal_lock);

    [self.contextArray addObject:observer];
    ABPAL_GpsCriteria criteria = [observer palCriteria];
    positionSource = criteria.desiredPositionSource;

    PAL_LockUnlock(pal_lock);
}

-(PAL_Error) cancelRequest:(id<GpsContextCallBackProtocol>) observer
{
    PAL_LockLock(pal_lock);

    unsigned long count = [contextArray count];
    for (int i = 0; i<count; ++i)
    {
        [contextArray removeObject:observer];
    }

    if ([contextArray count] == 0)
    {
        [self stop];
    }

    PAL_LockUnlock(pal_lock);

    return PAL_Ok;
}

-(void) dealloc
{
    if(!gpsThread.isFinished )
        [self shutdown];
    [gpsThread release];
    [currentThread release];
    [contextArray release];
    [gpsStartDate release];
    [gpsManager release];

    if (pal_lock)
    {
        PAL_LockDestroy(pal_lock);
        pal_lock = nil;
    }

    [super dealloc];
}

-(void) stop
{
    gpsDofix = NO;
    [self shutdown];

    [gpsThread release];
    gpsThread = nil;
}

- (void)shutdown
{
    gpsShutdown = YES;
    int retry = 10;
    while(gpsThread && !gpsThread.isFinished && retry != 0)
    {
        [NSThread sleepForTimeInterval:0.2];
        --retry;
    }
}

-(void) forceFix
{
    gpsForcefix = TRUE;
}

-(PAL_Error) start
{
    PAL_LockLock(pal_lock);

    // Every time start a new request, need reset gpsShutdown flag to NO to avoid checkState() will shutdown
    // GPS work thread - it means: checkState() request, shutdown then, so no GPS callback will comeback.
    gpsDofix = YES;
    gpsDoingfix = NO;
    gpsShutdown = NO;

    // delay initialization
    if (!gpsThread)
    {
        if(!UseMainThread)
        {
            gpsThread = [[NSThread alloc] initWithTarget:self selector:@selector(coreLocationThread:) object:nil];
            [gpsThread start];
        }
    }
    PAL_LockUnlock(pal_lock);

    return PAL_Ok;
}

- (void) setupLocationManager
{
    gpsManager = [[CLLocationManager alloc] init];

    if ([CLLocationManager authorizationStatus] == kCLAuthorizationStatusNotDetermined)
    {
        if ([gpsManager respondsToSelector:@selector(requestWhenInUseAuthorization)])
        {
            [gpsManager requestWhenInUseAuthorization];
        }
    }

    if (positionSource == PPS_sourceMostAccurateOnly)
    {
        if (iOSVersion >= 4.0)
        {
            gpsManager.desiredAccuracy= kCLLocationAccuracyBestForNavigation;
        }
        else
        {
            gpsManager.desiredAccuracy= kCLLocationAccuracyBest;
        }
    }
    else if (positionSource == PPS_sourceHighAccuracy)
    {
        gpsManager.desiredAccuracy = kCLLocationAccuracyNearestTenMeters;
    }
    else
    {
        gpsManager.desiredAccuracy = kCLLocationAccuracyThreeKilometers;
    }

    gpsManager.delegate = self;
    gpsStateChange = [NSTimer scheduledTimerWithTimeInterval:CheckStateTimeout
                                                     target:self
                                                   selector:@selector(checkState:)
                                                   userInfo:nil
                                                    repeats:YES];
}

- (void) coreLocationThread:(void*)unused
{
    gpsRunLoop = CFRunLoopGetCurrent();
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [self setupLocationManager];
    CFRunLoopRun();
    [pool release];
}

- (void) checkState:(NSTimer*)timer
{
    if (gpsDofix && !gpsDoingfix)
    {
        [gpsManager startUpdatingLocation];
        [gpsManager startUpdatingHeading];
        gpsDoingfix = YES;
    }

    if (!gpsDofix && gpsDoingfix)
    {
        [gpsManager stopUpdatingLocation];
        [gpsManager stopUpdatingHeading];
        gpsDoingfix = NO;
    }

    if (gpsShutdown)
    {
        // Stop timer
        [gpsStateChange invalidate];
        gpsStateChange = nil;

        // Stop request if current is requesting GPS when shutdown
        if (gpsDoingfix)
        {
            [gpsManager stopUpdatingLocation];
            [gpsManager stopUpdatingHeading];
            gpsDoingfix = NO;
        }

        [gpsManager release];
        gpsManager = nil;

        CFRunLoopStop( CFRunLoopGetCurrent());
//        gpsShutdown = NO;
    }
}

-(void)resetCompass;
{
    if (gpsManager && gpsDoingfix)
    {
        [gpsManager stopUpdatingHeading];
        [gpsManager startUpdatingHeading];
    }
}


-(BOOL) sameLocationFrom:(CLLocation*)lhs to:(CLLocation*)rhs {
    BOOL ret = YES;

    if (!rhs)
    {
        return NO;
    }
    double lhsts = [lhs.timestamp timeIntervalSince1970];
    double rhsts = [rhs.timestamp timeIntervalSince1970];

    ret = ret && lhs.coordinate.latitude == rhs.coordinate.latitude;
    ret = ret && lhs.coordinate.longitude == rhs.coordinate.longitude;
    ret = ret && lhs.speed == rhs.speed;
    ret = ret && lhs.horizontalAccuracy == rhs.horizontalAccuracy;
    ret = ret && lhs.course == rhs.course;
    ret = ret && lhs.altitude == rhs.altitude;
    ret = ret && lhs.verticalAccuracy == rhs.verticalAccuracy;
    ret = ret && lhsts == rhsts;

    return ret;
}

-(void) handleLocationMsg:(CLLocation *) newLocation
{
    for (int i =0; i<[contextArray count];)
    {
        id<GpsContextCallBackProtocol> context = [contextArray objectAtIndex:i];
        ABPAL_GpsCriteria criteria = [context palCriteria];
        NSTimeInterval locationAge = -newLocation.timestamp.timeIntervalSinceNow;
        CLLocationAccuracy haccuracy = newLocation.horizontalAccuracy;
        CLLocationAccuracy vaccuracy = newLocation.verticalAccuracy;
        if ((locationAge <= criteria.desiredMaxAge)
            && (haccuracy <= criteria.desiredAccuracy)
            && (haccuracy >= 0.0))
        {
            if([context requestState] != PGS_Oneshot)
            {
                ++i;
            }

            ABPAL_GpsLocation location;
            memset(&location, 0, sizeof(location));

            // Apple returns 2D variance, CCC requires SD along x and y. They are not matching.
            //    Set horizontalUncertaintyAlongAxis = accuracy, horizontalUncertaintyAlongPerpendicular = 0
            location.valid = PGV_Latitude | PGV_Longitude | PGV_HorizontalUncertainty | PGV_AxisUncertainty | PGV_PerpendicularUncertainty;

            if (vaccuracy >= 0.0)
            {
                location.valid |= PGV_Altitude | PGV_VerticalUncertainty;
            }

            if (newLocation.course >= 0.0)
            {
                location.valid |= PGV_Heading;
            }

            if (newLocation.speed >= 0.0)
            {
                location.valid |= PGV_HorizontalVelocity;
            }

            location.gpsTime = [newLocation.timestamp timeIntervalSinceDate:gpsStartDate];
            location.latitude = newLocation.coordinate.latitude;
            location.longitude = newLocation.coordinate.longitude;
            location.altitude = newLocation.altitude;
            location.horizontalVelocity = newLocation.speed;
            location.heading = newLocation.course;
            location.horizontalUncertaintyAlongAxis = haccuracy;
            location.horizontalUncertaintyAlongPerpendicular = haccuracy;
            location.horizontalUncertaintyAngleOfAxis = 0;
            location.verticalUncertainty = vaccuracy;

            [context locationCallBack:&location errorCode:PAL_Ok errorDetail:nil];
        }
        else
        {
            ++i;
        }
    }
}

-(void) handleLocationError:(NSError *) error
{
    for (int i =0; i<[contextArray count];)
    {
        id<GpsContextCallBackProtocol> context = [contextArray objectAtIndex:i];

        if([context requestState] != PGS_Oneshot)
        {
            ++i;
        }

        if ([self isServiceEnable])
        {
            [context locationCallBack:nil errorCode:PAL_ErrGpsGeneralFailure errorDetail:[[error localizedDescription] UTF8String]];
        }
        else
        {
            [context locationCallBack:nil errorCode:PAL_ErrGpsLocationDisabled errorDetail:[[error localizedDescription] UTF8String]];
        }
    }
}
-(void) handleHeadingMsg:(CLHeading *) newHeading
{
    if (ABS(newHeading.magneticHeading - self.prevCompassHeading) > NOTIFYING_DISPERSION)
    {
        //the value in [0.0;360.0)
        double currentCompassHeading = newHeading.magneticHeading;
        double currentCompassHeadingAccuracy = newHeading.headingAccuracy;

        float headingCorrection = 0.0;
        UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
        if (orientation == UIDeviceOrientationLandscapeLeft)
        {
            headingCorrection = 90.0;
        }
        else if (orientation == UIDeviceOrientationLandscapeRight)
        {
            headingCorrection = -90.0;
        }
        else if (orientation == UIDeviceOrientationPortraitUpsideDown)
        {
            headingCorrection = 180.0;
        }
        //the value in [-90.0;540.0)
        currentCompassHeading = currentCompassHeading + headingCorrection;
        //the value in [270.0;900.0)
        currentCompassHeading += 360;
        float compassIndex = currentCompassHeading / 360.0;
        //the value in [0.0;360.0)
        currentCompassHeading = (compassIndex - truncf(compassIndex)) * 360;

        for (int i =0; i<[contextArray count];i++)
        {
            id<GpsContextCallBackProtocol> context = [contextArray objectAtIndex:i];
            if([context requestState] == PGS_Tracking)
            {
                [context headingCallBack:currentCompassHeading accuracy:currentCompassHeadingAccuracy];
            }
        }
        self.prevCompassHeading = currentCompassHeading;
    }
}

-(BOOL) isServiceEnable
{
    BOOL result = NO;
    BOOL serviceEnabled = [CLLocationManager locationServicesEnabled];
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    result = (serviceEnabled
              && (status == kCLAuthorizationStatusAuthorizedAlways
                  || status == kCLAuthorizationStatusAuthorizedWhenInUse)) ? YES : NO;
    return result;
}

- (void) updateGpsCriteria:(ABPAL_GpsCriteria) criteria
{
    positionSource = criteria.desiredPositionSource;
    if (positionSource == PPS_sourceMostAccurateOnly)
    {
        if (iOSVersion >= 4.0)
        {
            gpsManager.desiredAccuracy= kCLLocationAccuracyBestForNavigation;
        }
        else
        {
            gpsManager.desiredAccuracy= kCLLocationAccuracyBest;
        }
    }
    else if (positionSource == PPS_sourceHighAccuracy)
    {
        gpsManager.desiredAccuracy = kCLLocationAccuracyNearestTenMeters;
    }
    else
    {
        gpsManager.desiredAccuracy = kCLLocationAccuracyThreeKilometers;
    }
}

#pragma mark -
#pragma mark CLLocationManagerDelegate

- (void)locationManager:(CLLocationManager *)manager
    didUpdateToLocation:(CLLocation *)newLocation
           fromLocation:(CLLocation *)oldLocation
{
    [self performSelector:@selector(handleLocationMsg:) onThread:currentThread withObject:newLocation waitUntilDone:NO];
}

- (void)locationManager:(CLLocationManager *)manager
       didFailWithError:(NSError *)error
{
    // switch to the calling thread from main, not necessarily ui thread
//    NSLog(@"GPS Error: %d, %@", [error code], [error description]);
    [self performSelectorOnMainThread:@selector(handleLocationError:) withObject:error waitUntilDone:NO];
}

- (void)locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status
{
    if (status == kCLAuthorizationStatusDenied)
    {
        [self performSelector:@selector(handleLocationError:) onThread:currentThread withObject:nil waitUntilDone:NO];
    }
}

- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)newHeading
{
    [self performSelector:@selector(handleHeadingMsg:) onThread:currentThread withObject:newHeading waitUntilDone:NO];
}

- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager *)manager
{
	// suppressing calibration popup entirely.
/*
    BOOL desiredCalibrationPopup = YES;

    if ([contextArray count] == 0)
    {
        desiredCalibrationPopup = NO;
    }

    for (int i =0; i<[contextArray count];i++)
    {
        id<GpsContextCallBackProtocol> context = [contextArray objectAtIndex:i];
        if ([context requestState] == PGS_Tracking)
        {
            ABPAL_GpsCriteria criteria = [context palCriteria];
            //if at least one context with NO was found then NO.
            desiredCalibrationPopup &= criteria.desiredCalibrationPopup && [context isCurrentMagneticHeading];
        }
        else
        {
            desiredCalibrationPopup = NO;
        }
    }
    return desiredCalibrationPopup;
*/
    return NO;
}
@end
