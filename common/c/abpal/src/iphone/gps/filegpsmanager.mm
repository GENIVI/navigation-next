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

 @file filegpsmanager.m
 @date 9/13/10

 FileGPS manager

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
#import "filegpsmanager.h"
#import "filegpshandlersingleton.h"
#import "palclock.h"
#import <string>

using namespace std;

static const double unctbl[] = { 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0,
    48.0, 64.0, 96.0, 128.0, 192.0, 256.0, 384.0, 512.0, 768.0, 1024.0, 1536.0, 2048.0, 3072.0, 4096.0,
    6144.0, 8192.0, 12288.0, -1.0, -2.0 };

static double uncdecode(uint8 n)
{
    if (n >= 0 && n < (sizeof(unctbl) / sizeof(*unctbl)))
    {
        return unctbl[n];
    }
    else
    {
        return unctbl[(sizeof(unctbl) / sizeof(*unctbl))-1];
    }
}

@implementation FileGpsManager


-(id)init:(PAL_Instance*) pal
{
    if (self = [super init])
    {
        _timer = nil;
        gpsFileHandler = FileGpsHandlerFactor::getInstance();

        pal_Instance = pal;
        palfile = nil;
    }

    return self;
}

-(void) addObserver:(id<GpsContextCallBackProtocol>) observer
{
    gpsContext_Instance = observer;
    gpsCriteria = [observer palCriteria];
}

-(PAL_Error) cancelGetLocation
{
    [_timer invalidate];
    _timer = nil;
    return PAL_Ok;
}

-(PAL_Error) endTracking
{
    [_timer invalidate];
    _timer = nil;
    return PAL_Ok;
}

-(BOOL) isServiceEnable
{
    return YES;
}

-(void) dealloc
{
    if (_timer)
    {
        [_timer invalidate];
    }

    if (palfile)
    {
        PAL_FileClose(palfile);
    }

    [super dealloc];
}

-(PAL_Error) start
{
    ABPAL_GpsState state = [gpsContext_Instance requestState];

    [_timer invalidate];
    _timer = nil;

    if (PGS_Oneshot == state)
    {
        _timer = [NSTimer timerWithTimeInterval:2
                                         target:self
                                       selector:@selector(timerExpired:)
                                       userInfo:nil
                                        repeats:NO];
    }
    else if (PGS_Tracking == state)
    {
        _timer = [NSTimer timerWithTimeInterval:(gpsCriteria.desiredInterval)
                                         target:self
                                       selector:@selector(timerExpired:)
                                       userInfo:nil
                                        repeats:YES];
    }
    [[NSRunLoop currentRunLoop] addTimer:_timer forMode:NSRunLoopCommonModes];
    return PAL_Ok;
}

-(void) updateGpsCriteria:(ABPAL_GpsCriteria) criteria
{
    if (PGS_Tracking==[gpsContext_Instance requestState])
    {
        [_timer invalidate];
        _timer = nil;
        _timer = [NSTimer timerWithTimeInterval:(criteria.desiredInterval)
                                         target:self
                                       selector:@selector(timerExpired:)
                                       userInfo:nil
                                        repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:_timer forMode:NSRunLoopCommonModes];
    }
}

-(void) timerExpired:(NSTimer*) timer
{
    ABPAL_GpsLocation *location = (ABPAL_GpsLocation*)malloc(sizeof(ABPAL_GpsLocation));
    PAL_Error err = [self readNextPosition:(*location)];
    [gpsContext_Instance locationCallBack:(location) errorCode:err errorDetail:nil];
    free(location);

    if (PGS_Oneshot == [gpsContext_Instance requestState])
    {
        _timer = nil;
    }
}

-(void) upDateGpsFileName:(const char*) name
{
    const char* fileName = [gpsFileHandler fileName];

    if (fileName && name)
    {
        string nameLast(fileName);
        string nameNew(name);

        transform(nameLast.begin(), nameLast.end(), nameLast.begin(), ::tolower);
        transform(nameNew.begin(), nameNew.end(), nameNew.begin(), ::tolower);

        if (nameLast.compare(nameNew) != 0)
        {
            [gpsFileHandler setFileName:name];
            [self upDateGpsOffset:0];
        }
    }
    else
    {
        [gpsFileHandler setFileName:name];
        [self upDateGpsOffset:0];
    }
}

-(void) upDateGpsOffset:(int) offset
{
    [gpsFileHandler setGpsFileOffset:offset];
}

-(PAL_Error) setSimulationConfig:(const GpsConfiguration *) config
{
    PAL_Error err = PAL_Ok;

    NSString* filename = [NSString stringWithUTF8String:[config fileName]];
    // Open file and keep file handler
    err = PAL_FileOpen(pal_Instance, [filename UTF8String], PFM_Read, &palfile);
    if (err != PAL_Ok)
    {
        filename = [self getFilePathFromDocumentFolder:[filename UTF8String]];
        err = PAL_FileOpen(pal_Instance, [filename UTF8String], PFM_Read, &palfile);
    }

    if (err != PAL_Ok)
    {
        return err;
    }

    [self upDateGpsFileName:[filename UTF8String]];


    uint32 filesize = 0;
    err = PAL_FileGetSize(pal_Instance, [filename UTF8String], &filesize);
    if (err != PAL_Ok)
    {
        return err;
    }

    maxCount = filesize/sizeof(GPSFileRecord);

    if (maxCount < 1)
    {
        return PAL_ErrNoData;
    }

    return PAL_Ok;
}


-(PAL_Error)readNextPosition:(ABPAL_GpsLocation&) location
{
    PAL_Error err = PAL_Ok;

    int current = [gpsFileHandler gpsFileOffset];

    if (current >= maxCount)
    {
        current = 0;
    }

    // Check whether pal file handler exists
    if (palfile)
    {
        int32 offset = current * sizeof(GPSFileRecord);
        err = PAL_FileSetPosition(palfile, PFSO_Start, offset);
        if (err == PAL_Ok)
        {
            uint32 bytesReaded = 0;
            GPSFileRecord record;
            memset(&record,0,sizeof(record));
            err = PAL_FileRead(palfile, (uint8*)(&record), sizeof(GPSFileRecord), &bytesReaded);

            if (err == PAL_Ok)
            {
                [self convert:record abpalLocation:location];
                ++current;
                [self upDateGpsOffset:current];
            }
        }

    }
    else
    {
        err = PAL_Failed;
    }


    return err;
}

-(void)convert:(const GPSFileRecord&) record abpalLocation:(ABPAL_GpsLocation&) location
{
    const double KWGSCoefficient    = 0.00000536441803;
    const double KHeadingScale        = 0.3515625;
    const double KSpeedScale        = 0.25;

    const int KFakeNumOfSatellites    = 4;

    location.status = PAL_Ok;
    location.gpsTime = PAL_ClockGetGPSTime();
    location.valid = record.fValid;
    location.latitude = record.dwLat * KWGSCoefficient;
    location.longitude = record.dwLon * KWGSCoefficient;
    location.heading = record.wHeading * KHeadingScale;
    location.compassHeading = location.heading;
    location.compassAccuracy = 0;
    location.horizontalVelocity = record.wVelocityHor * KSpeedScale;
    location.altitude = record.wAltitude * KWGSCoefficient;

    location.verticalVelocity = record.bVelocityVer / 2;
    location.horizontalUncertaintyAngleOfAxis = record.bHorUncAngle;
    location.horizontalUncertaintyAlongAxis = uncdecode(record.bHorUnc);
    location.horizontalUncertaintyAlongPerpendicular = uncdecode(record.bHorUncPerp);
    location.verticalUncertainty = record.wVerUnc;
    location.utcOffset = 0;
    location.numberOfSatellites = KFakeNumOfSatellites;

}

- (NSString*)getFilePathFromDocumentFolder:(const char*)filename
{
    NSString* string = [NSString stringWithUTF8String:filename];
    NSArray* array = [string componentsSeparatedByString:@"/"];
    NSString* gpsFilename = [array objectAtIndex:(array.count-1)];
    array = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* docPath = [array objectAtIndex:0];
    NSString* gpsFileFullName = [docPath stringByAppendingFormat:@"/%@", gpsFilename];
    return gpsFileFullName;
}

@end
