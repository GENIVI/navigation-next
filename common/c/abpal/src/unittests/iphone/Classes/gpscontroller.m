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

Unit Tests for AB PAL

This file contains all unit tests for the AB PAL components
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

#import "gpscontroller.h"
#include "palfile.h"

#define kRealGpsTest
#define kSimuGpsTest

@implementation GpsController

@synthesize latBtn,lonBtn,statusBtn,timeBtn,clearBtn;
@synthesize tracking;



static void GpsInitializeCallback(const void* userData, PAL_Error error, const char* errorInfoXml)
{
   if (error == PAL_Ok)
   {
       GpsController *selfPointer = (GpsController*)userData;

       [selfPointer clearScreen];
       [selfPointer.statusBtn setTitle:@"init ok" forState:UIControlStateNormal];
       NSLog(@"test get location");

   }
    else {

        GpsController *selfPointer = (GpsController*)userData;
        [selfPointer clearScreen];

        [selfPointer.statusBtn setTitle:@"init err" forState:UIControlStateNormal];
        NSLog(@"gps init failed");
    }

}

static void GpsDeviceStateCallback(const void* userData, const char* stateChangeXml, PAL_Error error, const char* errorInfoXml)
{

}

static void GpsLocationCallback(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml)
{
    if (error == PAL_Ok)
    {
        GpsController *selfPointer = (GpsController*)userData;


        NSString *latstring = [[NSString alloc] initWithFormat:@"%.8f",location->latitude];
        NSString *lonstring = [[NSString alloc] initWithFormat:@"%.8f",location->longitude];
        NSString *timetring = [[NSString alloc] initWithFormat:@"%d",location->gpsTime];


        [selfPointer.latBtn setTitle:latstring forState:UIControlStateNormal];
        [selfPointer.lonBtn setTitle:lonstring forState:UIControlStateNormal];
        [selfPointer.timeBtn setTitle:timetring forState:UIControlStateNormal];
        [selfPointer.statusBtn setTitle:@"OK" forState:UIControlStateNormal];

        [timetring release];
        [latstring release];
        [lonstring release];



        NSLog(@"test get location");

    }
    else {
        GpsController *selfPointer = (GpsController*)userData;
        NSString *string = [[NSString alloc] initWithFormat:@"%d",error];

        [selfPointer.statusBtn setTitle:string forState:UIControlStateNormal];

        [string release];
        NSLog(@"gps init failed");
    }

}


-(IBAction) realBtn
{
    [self initRealGps];
    real = TRUE;
}

-(IBAction) simuBtn
{
    [self initSimuGps];
    real = FALSE;
}

-(IBAction) getLocationBtn
{
    [self testGetLocation];
}

-(IBAction) trackingBtn
{
    tracking = YES;
    [self testTraking];
}

-(IBAction) cancelGetBtn
{
    if (!tracking)
    {
        PAL_Error err = PAL_Ok;
        err = ABPAL_GpsDestroy(gpsContext);
        assert(err==PAL_Ok);
        PAL_Destroy(pal);
    }

}

-(IBAction) clearScreen
{
    [self.latBtn setTitle:nil forState:UIControlStateNormal];
    [self.lonBtn setTitle:nil forState:UIControlStateNormal];
    [self.timeBtn setTitle:nil forState:UIControlStateNormal];
    [self.statusBtn setTitle:nil forState:UIControlStateNormal];
}
-(IBAction) cancelTrackingBtn
{
    if (tracking)
    {
        PAL_Error err = PAL_Ok;
        err = ABPAL_GpsEndTracking(gpsContext, &trackingInfo);
        assert(err==PAL_Ok);
        err = ABPAL_GpsDestroy(gpsContext);
        assert(err==PAL_Ok);
        PAL_Destroy(pal);
    }
}

-(void) initSimuGps
{
    const char *path = [[[NSBundle mainBundle] pathForResource:@"TEST" ofType:@"GPS"] cStringUsingEncoding:NSUTF8StringEncoding];
    const ABPAL_GpsConfig config[] =
    {
        "gpsfilename", path,
    };

    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    pal = NULL;

    gpsContext = NULL;

    PAL_Config palConfig = {0};   // dummy config
    pal = PAL_Create(&palConfig);
   // pal = PAL_CreateInstance();
    assert(pal!=nil);
    err = ABPAL_GpsCreate(pal, &gpsContext);
    assert(pal != PAL_Ok);
    assert(gpsContext);
    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, self);
    assert(err == PAL_Ok);
}

-(void) initRealGps
{

    const ABPAL_GpsConfig config[] =
    {
        "providertype",     "gpsid",
    };

    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    pal = NULL;

    gpsContext = NULL;

    PAL_Config palConfig = {0};   // dummy config
    pal = PAL_Create(&palConfig);
    //pal = PAL_CreateInstance();
    assert(pal!=nil);
    err = ABPAL_GpsCreate(pal, &gpsContext);
    assert(pal != PAL_Ok);
    assert(gpsContext);
    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, self);
    assert(err == PAL_Ok);
}

-(void) testGetLocation
{
    PAL_Error err = PAL_Ok;
    ABPAL_GpsCriteria criteria = { 0 };


    criteria.desiredAccuracy = 50;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 16;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 30;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsGetLocation(gpsContext, &criteria, GpsLocationCallback, self);

    assert(err == PAL_Ok);

}

-(void) testTraking
{
    PAL_Error err = PAL_Ok;
    ABPAL_GpsCriteria criteria = { 0 };

    criteria.desiredAccuracy = 50;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 16;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 60;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsBeginTracking(gpsContext, FALSE, &criteria, GpsLocationCallback, self, &trackingInfo);

    assert(err==PAL_Ok);
}

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    real = TRUE;
    [super viewDidLoad];
}


/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];

    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    self.latBtn = nil;
    self.lonBtn = nil;
    self.statusBtn = nil;
    self.timeBtn = nil;
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [timeBtn release];
    [latBtn release];
    [lonBtn release];
    [statusBtn release];
    [super dealloc];
}


@end
