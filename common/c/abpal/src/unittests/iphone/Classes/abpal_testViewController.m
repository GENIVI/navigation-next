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

#define NAVBAR_TINT_BLUE [UIColor colorWithRed:21.0/255.0 green:101.0/255.0 blue:180.0/255.0 alpha:1.0]

#import "abpal_testViewController.h"
#import "gpscontroller.h"
#import "testAudioController.h"
#import "testTTSController.h"
#import "main.h"

@implementation abpal_testViewController

-(IBAction)testGps:(id) sender
{
    GpsController *t = [[GpsController alloc] initWithNibName:@"testGpsController" bundle:nil];
    [self.navigationController pushViewController:t animated:TRUE];
    [t release];
}

-(IBAction)testAudio:(id) sender
{
    AudioController *t = [[AudioController alloc] initWithNibName:@"testAudioController" bundle:nil];
    [self.navigationController pushViewController:t animated:TRUE];
    [t release];
}

- (IBAction)testTTS:(id)sender {
    testTTSController* ttsController = [[testTTSController alloc] initWithNibName:@"testTTSController" bundle:nil];
    [self.navigationController pushViewController:ttsController animated:TRUE];
    //[ttsController release];
}

-(IBAction)testMain:(id) sender
{
    test_main();
}
/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
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
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self.navigationController setNavigationBarHidden:TRUE animated:FALSE];
}

- (void)viewWillDisappear:(BOOL)animated
{
    NSString *title = nil;

    title = NSLocalizedString(@"TEST", @"");

    // Navigation Bar
    [self.navigationController.navigationBar setTintColor:NAVBAR_TINT_BLUE];
    [self.navigationController setNavigationBarHidden:FALSE animated:TRUE];
    [self.navigationController.navigationBar.topItem setTitle:title];

}

@end
