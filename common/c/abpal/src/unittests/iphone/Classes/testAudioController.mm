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

#include "CUnit.h"
#import "testAudioController.h"
#include "palfile.h"


@implementation AudioController

static void AudioPlayerCallback(void* userData, ABPAL_AudioState state);
static void RecoderCallback(void* userData,ABPAL_AudioState state);


-(IBAction) playBtn
{
    const char *path = [[[NSBundle mainBundle] pathForResource:@"sound" ofType:@"amr"] cStringUsingEncoding:NSUTF8StringEncoding];

    PAL_Error error = ABPAL_AudioPlayerCreate(pal,ABPAL_AudioFormat_AMR,&player);

    if (error != PAL_Ok)
    {

    }
    else
    {
        PAL_File* file;
        PAL_FileOpen(pal,path,PFM_Read,&file);

        uint32 size1;
        PAL_FileGetSize(pal,path,&size1);

        if (palyBuffer)
        {
            nsl_free(palyBuffer);
            palyBuffer = NULL;
        }
        palyBuffer = (byte*)nsl_malloc(size1);

        uint32 readBytes;
        PAL_FileRead(file,palyBuffer,size1,&readBytes);
        PAL_FileClose(file);
        ABPAL_AudioPlayerPlay(player,palyBuffer,size1,FALSE,AudioPlayerCallback,self);
    }

}

-(IBAction) stopPlayBtn
{
    ABPAL_AudioPlayerCancel(player);
    ABPAL_AudioPlayerDestroy(player);

    nsl_free(palyBuffer);
    palyBuffer = NULL;
}

-(IBAction) record
{
    ABPAL_AudioRecorderCreate(pal,ABPAL_AudioFormat_RAW,&recoder);
    ABPAL_AudioRecorderRecord(recoder, NULL, 0, RecoderCallback,self);
}

-(IBAction) playRecorder
{
    ABPAL_AudioPlayerCreate(pal,ABPAL_AudioFormat_AMR,&player);
    ABPAL_AudioPlayerPlay(player,palyBuffer,size,FALSE,AudioPlayerCallback,self);

}

-(IBAction) stopPlayRecorder
{
    ABPAL_AudioPlayerCancel(player);
    ABPAL_AudioPlayerDestroy(player);
    nsl_free(palyBuffer);
    palyBuffer = NULL;
    size =0;
}

-(IBAction) stopRecord
{
    ABPAL_AudioRecorderStop(recoder);

    if (palyBuffer != NULL)
    {
        nsl_free(palyBuffer);
        palyBuffer = NULL;
    }

    //ABPAL_AudioRecorderGetRecordedData(recoder,&palyBuffer,&size);

}


void AudioPlayerCallback(void* userData, ABPAL_AudioState state)
{
    // TODO
}

-(void) getData
{
    ABPAL_AudioRecorderGetRecordedData(recoder,&palyBuffer,&size);
}

void RecoderCallback(void* userData,ABPAL_AudioState state)
{
    if (state == ABPAL_AudioState_Ended)
    {
        AudioController* self = static_cast<AudioController*>(userData);
        [self getData];
    }
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    PAL_Config palConfig = {0};   // dummy config
    pal = PAL_Create(&palConfig);
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
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    PAL_Destroy(pal);
    [super dealloc];
}


@end
