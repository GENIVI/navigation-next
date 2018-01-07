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
   @file        testTTSController.mm
   @defgroup    abpal
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */


#import "testTTSController.h"
#include "abpalttsengine.h"
#import <AVFoundation/AVFoundation.h>
using namespace abpal;
using namespace std;

static float ttsRate;
static float ttsPitch;

@interface testTTSController ()
@property (retain, nonatomic) IBOutlet UIPickerView *languagePicker;
@property (retain, nonatomic) IBOutlet UITextView *textSource;
@property (retain, nonatomic) IBOutlet UIButton *startBtn;
@property (retain, nonatomic) IBOutlet UIButton *endBtn;
@property (strong, nonatomic) NSArray* languageArray;
@property (nonatomic) TTSEnginePtr engine;
@property (retain, nonatomic) IBOutlet UIButton *txtBtn;
@property (nonatomic) int lastIndex;
@property (retain, nonatomic) IBOutlet UISlider *rateSlider;
@property (retain, nonatomic) IBOutlet UISlider *pitchSlider;
@property (retain, nonatomic) IBOutlet UILabel *pitchLabel;
@property (retain, nonatomic) IBOutlet UILabel *rateLabel;
@property (nonatomic) BOOL configChanged;
@end



@implementation testTTSController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        self.languageArray = [[NSArray alloc] initWithObjects:
                                              @"ar-SA",
                                              @"cs-CZ",
                                              @"da-DK",
                                              @"de-DE",
                                              @"el-GR",
                                              @"en-AU",
                                              @"en-GB",
                                              @"en-IE",
                                              @"en-US",
                                              @"en-ZA",
                                              @"es-ES",
                                              @"fi-FI",
                                              @"fr-CA",
                                              @"fr-FR",
                                              @"hi-IN",
                                              @"hu-HU",
                                              @"id-ID",
                                              @"it-IT",
                                              @"ko-KR",
                                              @"nl-BE",
                                              @"nl-NL",
                                              @"no-NO",
                                              @"pl-PL",
                                              @"pt-BR",
                                              @"pt-PT",
                                              @"ro-RO",
                                              @"ru-RU",
                                              @"sk-SK",
                                              @"sv-SE",
                                              @"th-TH",
                                              @"tr-TR",
                                              @"zh-CN",
                                              @"zh-HK",
                                              @"zh-TW", nil];
    }
    self.lastIndex = 8;
    //self.textSource.text = @"Old MacDonald had a farm,  And on that farm he had a cat.";
    ttsRate = AVSpeechUtteranceDefaultSpeechRate;
    ttsPitch = 1;
    self.configChanged = TRUE;
    return self;
}
- (IBAction)onPlay:(id)sender {
    int index = [self.languagePicker selectedRowInComponent:0];
    if (index != self.lastIndex || !self.engine || self.configChanged) {
        self.lastIndex = index;
        self.configChanged = false;
        NSString* lang = [self.languageArray objectAtIndex:self.lastIndex];
        self.engine = TTSEngine::CreateInstance([lang cStringUsingEncoding:NSUTF8StringEncoding], EOT_DuckOthers, EVT_Female);
    }
    if (!self.engine) {
        fprintf(stderr, "Failed to create tts engine!");
        return;
    }

    NSString* contents = [NSString stringWithFormat:@"%@", self.textSource.text];
    if (contents == nil) {
        fprintf(stderr, "Failed to get text from buffer\n");
        return;
    }
    string rawText([contents cStringUsingEncoding:NSUTF8StringEncoding]);
    string emptyString;
    TTSDataPtr data(new TTSData(emptyString, emptyString, rawText));
    self.engine->Play(data);
}
- (IBAction)onStop:(id)sender {
    if (self.engine) {
        self.engine->Stop();
    }
}
- (IBAction)onTxtBtnPressed:(id)sender {
    [[self view] endEditing:YES];
    [self.txtBtn setHidden:YES];
}

- (UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row
          forComponent:(NSInteger)component reusingView:(UIView *)view
{
    UILabel* label = [[UILabel alloc] initWithFrame:CGRectMake(5.0f, 0.0f, 300.0f, 30.0f)];
    if (pickerView.tag == 0)
    {
        label.text = [self.languageArray objectAtIndex:row];
    }

    label.font = [UIFont systemFontOfSize:14];
    label.backgroundColor = [UIColor clearColor];
    label.textAlignment = NSTextAlignmentCenter;
    return label;
}
- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view.

    [self.languagePicker selectRow:self.lastIndex inComponent:0 animated:TRUE];
    [self.txtBtn setHidden:YES];

    self.rateSlider.minimumValue = AVSpeechUtteranceMinimumSpeechRate;
    self.rateSlider.maximumValue = AVSpeechUtteranceMaximumSpeechRate;
    [self.rateSlider setValue:ttsRate];
    self.rateLabel.text = [[NSString alloc] initWithFormat:@"%.02f", ttsRate];
    self.pitchSlider.minimumValue = 0.5;
    self.pitchSlider.maximumValue = 2;
    [self.pitchSlider setValue:ttsPitch];
    self.pitchLabel.text = [[NSString alloc] initWithFormat:@"%.02f", ttsPitch];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)rateChanged:(id)sender
{
    ttsRate = self.rateSlider.value;
    self.rateLabel.text = [[NSString alloc] initWithFormat:@"%.2f", ttsRate];
    self.configChanged = TRUE;
}

- (IBAction)pitchChanged:(id)sender
{
    ttsPitch = self.pitchSlider.value;
    self.pitchLabel.text = [[NSString alloc] initWithFormat:@"%.2f", ttsPitch];
    self.configChanged = TRUE;
}
/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

- (void)dealloc {
    [_languagePicker release];
    [_textSource release];
    [_startBtn release];
    [_endBtn release];
    [_txtBtn release];
    [_rateSlider release];
    [_rateSlider release];
    [_pitchSlider release];
    [_rateLabel release];
    [_pitchLabel release];
    [super dealloc];
}
- (void)viewDidUnload {
    [self setLanguagePicker:nil];
    [self setTextSource:nil];
    [self setStartBtn:nil];
    [self setEndBtn:nil];
    [super viewDidUnload];
}


// returns the number of 'columns' to display.
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

// returns the # of rows in each component..
- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return self.languageArray.count;
}

- (void)textViewDidEndEditing:(UITextView *)textView
{
    [self.txtBtn setHidden:YES];
}

- (void)textViewDidBeginEditing:(UITextView *)textView
{
    [self.txtBtn setHidden:NO];
}



@end



// @note: This ConfigPlugin was used by NavkitUI to change TTS engine, and should be
//        removed in the final release!gs
@interface TTSEngineIphoneConfigPlugin : NSObject
@property(nonatomic) float rate;             // Values are pinned between AVSpeechUtteranceMinimumSpeechRate and AVSpeechUtteranceMaximumSpeechRate.
@property(nonatomic) float pitchMultiplier;  // [0.5 - 2] Default = 1
@end

@implementation TTSEngineIphoneConfigPlugin
-(id) init
{
    self = [super init];
    self.rate = ttsRate;
    self.pitchMultiplier = ttsPitch;
    return self;
}
@end

/*! @} */
