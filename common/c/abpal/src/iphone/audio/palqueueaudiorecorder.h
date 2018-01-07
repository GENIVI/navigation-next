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
 
 @file     palqueueaudiorecorder.h
 @date     01/27/2012
 @defgroup PAL Audio API
 
 @brief    Platform-independent audio API
 
 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.           
 
 The information contained herein is confidential, proprietary 
 to Networks In Motion, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of Networks In Motion is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 
 
 ---------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "abpalaudio.h"
#import "pallock.h"

// Forward declaration for PalMeterTable
@class PalMeterTable;

/*! PAL queue audio recorder class. */
@interface PalQueueAudioRecorder : NSObject
{
    PAL_Instance*                      m_pal;                             /*!< PAL instance. */
    AudioQueueRef                      m_queue;                           /*!< Recorder queue. */
    AudioQueueBufferRef*               m_buffers;                         /*!< Recorder input buffers array. */
    uint32                             m_buffersCount;                    /*!< Recorder input buffers array size. */
    AudioStreamBasicDescription        m_recordFormat;                    /*!< Record format. */
    ABPAL_AudioFormat                  m_audioFormat;                     /*!< PAL Record format. */
    ABPAL_AudioState                   m_recorderState;                   /*!< Recorder state. */
    ABPAL_AudioSession*                m_audioSession;                    /*!< PAL Audio Session. */
    ABPAL_AudioSessionType             m_audioSessionType;                /*!< PAL Audio Session type. */
    AudioQueueLevelMeterState*         m_chanelsLevel;                    /*!< Recorder queue channels level. */
    ABPAL_AudioRecorderCallback*       m_recorderCallBack;                /*!< Recorder callback function. */
    void*                              m_userData;                        /*!< Recorder callback data. */
    AudioFileID                        m_recordFile;                      /*!< Record file. */
    SInt64                             m_recordPacket;                    /*!< Current packet to be recorded. */
    bool                               m_streamRecordingMode;             /*!< Flag to indicate that recorder works in stream mode. Otherwise it works in file mode. */
    uint32                             m_bufferDuration;                  /*!< Record buffer duration in msecs. */
    uint32                             m_sampleRate;                      /*!< Sample rate for audio format in Hz. */
    uint32                             m_bitsPerSample;                   /*!< Bits per sample for audio format. */
    PalMeterTable*                     m_meterTable;                      /*!< Meter Table to calculate loudness level. */
    PAL_Lock*                          m_queueAccessLock;                 /*!< Locker for recorded buffer queue access. */
    PAL_Lock*                          m_loudnessLevelAccessLock;         /*!< Loudness level access locker. */
    NSMutableArray*                    m_recorderTaskQueue;               /*!< Recorded buffer queue. */
    NSThread*                          m_clientThread;                    /*!< Client thread id. */
}

/*! Init recorder. */
- (id) initWithPal:(PAL_Instance*)pal audioFormat:(ABPAL_AudioFormat)format;

/*! Begin recording. */
- (PAL_Error) startRecord:(ABPAL_AudioRecorderCallback*)callback userData:(void*)data;

/*! Prepares for recording. Call startRecord to begin recording. */
- (PAL_Error) prepareToRecord;

/*! Stop recording. */
- (PAL_Error) stop;

/*! Returns current recorder state. */
- (ABPAL_AudioState) state;

/*! Returns recorded buffer. Caller is responsible for freeing data. */
- (PAL_Error) getRecordedBuffer:(byte**)buffer bufferSize:(uint32*)size;

/*! Returns current loudness level for first channel as int in 0 - 100. Can be accessed from different threads. */
- (PAL_Error) getLoudnessLevel:(uint32*)loudnessLevel;

/*! Setups audio session. ABPAL_AudioSession_Record by default. */
- (PAL_Error) setAudioSessionType:(ABPAL_AudioSessionType)audioSessionType;

/*! Setups stream recording mode with record buffer duration in msecs, instead default write to file mode.
    Recorded buffers become available on ABPAL_AudioState_StreamRecordBufferAvailable notification thru getRecordedBuffer method.
    Latest available buffer after stop recording become available on ABPAL_AudioState_Ended notification.
    There is no way to turn of stream mode back to write to file mode. If this mode is needed client should call it before startRecord/prepareToRecord, otherwise it's fail.
    Pass duration as 0 to keep default value which is 200 msec. Client should call this method only once, other calls will no effects. */
- (PAL_Error) setStreamRecordingModeWithBufferDuration:(uint32)duration;

/*! Setups sample rate in Hz for audio format. 8 kHz by default. Client should call this method before startRecord/prepareToRecord, otherwise it will no effectes. */
- (PAL_Error) setSampleRate:(uint32)sampleRate;

/*! Returns sample rate in Hz for audio format currently setuped. */
- (PAL_Error) getSampleRate:(uint32*)sampleRate;

/*! Setups bits per sample for audio format. 8 bit by default. Client should call this method before startRecord/prepareToRecord, otherwise it will no effectes. */
- (PAL_Error) setBitsPerSample:(uint32)bitsPerSample;

/*! Returns bits per sample for audio format currently setuped. */
- (PAL_Error) getBitsPerSample:(uint32*)bitsPerSample;

@end
