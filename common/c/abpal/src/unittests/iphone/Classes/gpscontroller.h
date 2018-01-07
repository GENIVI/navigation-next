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

#import <UIKit/UIKit.h>
#include "abpalgps.h"

@interface GpsController : UIViewController {

    UIButton *latBtn;
    UIButton *lonBtn;
    UIButton *statusBtn;
    UIButton *timeBtn;
    UIButton *clearBtn;

    BOOL     real;

    BOOL     tracking;
    PAL_Instance* pal;

    ABPAL_GpsContext* gpsContext;
    ABPAL_GpsTrackingInfo trackingInfo;
}

@property (nonatomic,assign) BOOL tracking;
@property (nonatomic, retain) IBOutlet UIButton *latBtn;
@property (nonatomic, retain) IBOutlet UIButton *lonBtn;
@property (nonatomic, retain) IBOutlet UIButton *statusBtn;
@property (nonatomic, retain) IBOutlet UIButton *timeBtn;
@property (nonatomic, retain) IBOutlet UIButton *clearBtn;

-(IBAction) realBtn;
-(IBAction) simuBtn;
-(IBAction) getLocationBtn;
-(IBAction) trackingBtn;
-(IBAction) cancelGetBtn;
-(IBAction) cancelTrackingBtn;
-(IBAction) clearScreen;

-(void) testGetLocation;
-(void) testTraking;

-(void) initRealGps;
-(void) initSimuGps;

@end
