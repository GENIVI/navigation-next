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

 @file     palaudiorecorder.h
 @date     09/01/2010
 @defgroup PAL Audio API

 @brief    Platform-independent audio API

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
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "abpalaudio.h"

/*! PAL audio recorder class, recording state drivened. */
@interface PalAudioRecorder : NSObject <AVAudioRecorderDelegate>
{
    PAL_Instance* palInstance;                      /*!< PAL instance. */
    ABPAL_AudioFormat recorderFormat;               /*!< Recorder format. */
    ABPAL_AudioRecorderCallback* recorderCallBack;  /*!< Recorder callback function. */
    void* userData;                                 /*!< Recorder callback data. */
    AVAudioRecorder* recorder;                      /*!< Recorder does recording work. */
    ABPAL_AudioState recorderState;                 /*!< Recording state. */
    byte* recorderBuffer;                           /*!< Recorded buffer. */
    int	recorderbufferSize;                         /*!< Recorded buffer size. */
    NSString* recordFileName;                       /*!< Recording file to keep recording buffer when recording. */
    ABPAL_AudioSession* m_audioSession;             /*!< PAL Audio Session. */
    ABPAL_AudioSessionType m_audioSessionType;      /*!< PAL Audio Session type. */
}

/*! Init recorder. */
-(id) initWithPal:(PAL_Instance*)pal audioFormat:(ABPAL_AudioFormat)format;

/*! Begin recording. */
-(PAL_Error) startRecord:(ABPAL_AudioRecorderCallback*)callback userData:(void*)data;

/*! Prepare to record. */
-(PAL_Error) prepareToRecord;

/*! Stop recording. */
-(PAL_Error) stop;

/*! Get recorded buffer. */
-(PAL_Error) getRecordedBuffer:(byte**)buffer size:(uint32*)bufferSize;

/*! Get recording state. */
-(ABPAL_AudioState) state;

/*! Setups audio session. ABPAL_AudioSession_Record by default. */
-(PAL_Error) setAudioSessionType:(ABPAL_AudioSessionType)audioSessionType;

@end
