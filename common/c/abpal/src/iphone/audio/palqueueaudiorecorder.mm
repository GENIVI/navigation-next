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
 
 @file     palqueueaudiorecorder.m
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

#import "palqueueaudiorecorder.h"
#import "palaudiosessionsingleton.h"
#import "palstdlib.h"
#import "palfile.h"

// Default record buffer duration msec. This value should not be great than kAggregatedBufferDuration.
#define kDefaultBufferDuration 200

// Aggregated record buffer duration in msec. Used to calculate buffers count.
#define kAggregatedBufferDuration 2000

// Default Sample Rate
#define kDefaultSampleRate 8000.0

// Default Bits Per Sample
#define kDefaultBitsPerSample 8

// Header size to be cut off for raw audio format
#define kRawHeaderSize 4096

// Record file name used in record to file mode
#define kRecordFileName    @"tempsound.dat"

// Converts Decibels to Amp
#define DbToAmp(db)   (pow(10.0, 0.05 * (db)))


/*! Class for handling conversion from linear scale to dB. */
@interface PalMeterTable : NSObject
{
    float   m_minDecibels;
    float   m_maxDecibels;
    uint8*  m_table;
    float   m_scaleFactor;
    uint32  m_tableSize;
}

// Designated initializer
- (id) init:(float)minDecibels maxDecibels:(float)maxDecibels tableSize:(uint32)tableSize root:(float)root;
- (uint8) valueAt:(float)decibels;

@end

@implementation PalMeterTable

- (id) init
{
    if (self = [self init:-60.0 maxDecibels:0.0 tableSize:400.0 root:2.0])
    {
    }

    return self;
}

- (id) init:(float)minDecibels maxDecibels:(float)maxDecibels tableSize:(uint32)tableSize root:(float)root
{
    if (self = [super init])
    {
        m_minDecibels = minDecibels;
        m_maxDecibels = maxDecibels;
        m_tableSize = tableSize;
        m_scaleFactor = (m_tableSize - 1) / (m_minDecibels - m_maxDecibels);

        // Guard against incorrect values. The follow condition should be true: -infinity < m_minDecibels < m_maxDecibels <= 0.0
        if ((m_minDecibels >= 0.0) || (m_maxDecibels <= m_minDecibels) || (m_maxDecibels > 0.0))
        {
            [self release];
            self = nil;
            return self;
        }

        m_table = (uint8*)nsl_malloc(tableSize * sizeof(uint8));
        if (!m_table)
        {
            [self release];
            self = nil;
            return self;
        }

        double minAmp = DbToAmp(m_minDecibels);
        double maxAmp = DbToAmp(m_maxDecibels);
        double ampRange = maxAmp - minAmp;
        double invAmpRange = 1.0 / ampRange;
        double rroot = 1.0 / root;
        double decibelResolution = (m_minDecibels - m_maxDecibels) / (m_tableSize - 1);

        for (uint32 i = 0; i < m_tableSize; i++)
        {
            double decibels = i * decibelResolution + m_maxDecibels;
            double amp = DbToAmp(decibels);
            double adjAmp = (amp - minAmp) * invAmpRange;

            // Scale 0.0 - 1.0 to 0 - 100
            m_table[i] = (uint8)(100 * pow(adjAmp, rroot));

            if (m_table[i] > 100)
                m_table[i] = 100;
        }
    }

    return self;
}

- (uint8) valueAt:(float)decibels
{
    if (decibels <= m_minDecibels)
        return 0;

    if (decibels >= m_maxDecibels)
        return 100;

    int index = m_scaleFactor * (decibels - m_maxDecibels);

    return m_table[index];
}

- (void) dealloc
{
    if (m_table)
    {
        nsl_free(m_table);
        m_table = NULL;
    }

    [super dealloc];
}

@end

@interface PalAudioBuffer : NSObject
{
@private
    byte*        m_data;
    uint32       m_dataSize;
}

- (id) initWithData:(byte*)inData size:(uint32)inSize withCopy:(BOOL)copy;
- (BOOL) isEmpty;
- (BOOL) getData:(byte**)outData size:(uint32*)outSize; // Client become owner of the returned data
- (BOOL) appendData:(byte*)inData size:(uint32)inSize;

@end

@implementation PalAudioBuffer

- (id) initWithData:(byte*)inData size:(uint32)inSize withCopy:(BOOL)copy
{
    if (self = [super init])
    {
        m_data = NULL;
        m_dataSize = 0;

        if (inData && inSize)
        {
            if (copy)
            {
                m_data = (byte*)nsl_malloc(inSize);
                if (!m_data)
                {
                    [self release];
                    self = nil;
                }

                m_dataSize = inSize;
                nsl_memcpy(m_data, inData, m_dataSize);
            }
            else
            {
                m_data = inData;
                m_dataSize = inSize;
            }
        }
    }

    return self;
}

- (void) dealloc
{
    m_dataSize = 0;

    if (m_data)
    {
        nsl_free(m_data);
        m_data = NULL;
    }

    [super dealloc];
}

- (BOOL) isEmpty
{
    return (!m_data || !m_dataSize);
}

- (BOOL) getData:(byte**)outData size:(uint32*)outSize
{
    if (!outData || !outSize || [self isEmpty])
    {
        return NO;
    }

    *outData = m_data;
    *outSize = m_dataSize;
    m_data = NULL;
    m_dataSize = 0;

    return YES;
}

- (BOOL) appendData:(byte*)inData size:(uint32)inSize
{
    if (!inData || !inSize)
    {
        return NO;
    }

    {
        byte* data = (byte*)nsl_realloc(m_data, m_dataSize + inSize);
        if (!data)
        {
            return NO;
        }

        m_data = data;
    }

    nsl_memcpy(m_data + m_dataSize, inData, inSize);
    m_dataSize += inSize;

    return YES;
}

@end

@interface PalQueueTask : NSObject
{
@private
    id<NSObject>        m_object;
    id<NSObject>        m_target;
    SEL                 m_action;
    NSThread*           m_performThread;
}

@property (nonatomic, retain) id<NSObject> m_object;

- (id) initWithObject:(id<NSObject>)object target:(id<NSObject>)target action:(SEL)action performThread:(NSThread*)performThread;
- (void) startTask;
- (void) stopTask;
- (void) performTask:(id)sender; // private method

@end

@implementation PalQueueTask

@synthesize m_object;

- (id) initWithObject:(id<NSObject>)object target:(id<NSObject>)target action:(SEL)action performThread:(NSThread*)performThread
{
    if (self = [super init])
    {
        m_object = [object retain];
        m_target = target;
        m_action = action;
        m_performThread = performThread;
    }

    return self;
}

- (void) dealloc
{
    [self stopTask];
    [super dealloc];
}

- (void) startTask
{
    [self retain];
    [self performSelector:@selector(performTask:) onThread:m_performThread withObject:nil waitUntilDone:NO];
}

- (void) performTask:(id)sender
{
    if (m_target)
    {
        [m_target performSelector:m_action withObject:self];
    }

    [self release];
}

- (void) stopTask
{
    if (m_object)
    {
        [m_object release];
        m_object = nil;
    }

    m_target = nil;
}

@end

#pragma mark -
#pragma mark Private Interface

@interface PalQueueAudioRecorder()

- (BOOL) initQueueAudioRecorder:(PAL_Instance*)pal audioFormat:(ABPAL_AudioFormat)format;
- (BOOL) setupAudioFormat:(ABPAL_AudioFormat)format;
- (int) deriveRecordBufferSize:(uint32)mseconds;
- (BOOL) initAudioSession;
- (void) resetAudioSession;
- (PAL_Error) doPrepareToRecord:(BOOL)paused;
- (void) notifyClientWithRecorderState:(ABPAL_AudioState)state;

- (BOOL) enableLevelMetering;
- (void) disableLevelMetering;
- (BOOL) allocateLevelMeteringBuffers;
- (void) freeLeveMeteringBuffers;

- (BOOL) addRecordedBufferToQueue:(byte*)buffer bufferSize:(uint32)bufferSize withCopy:(BOOL)copy;
- (BOOL) appendRecordedBufferToLastChunkInQueue:(byte*)buffer bufferSize:(uint32)bufferSize;
- (void) freeRecordedBufferQueue;
- (void) closeRecordFile;
- (BOOL) storeRecordedFileToBufferQueue;
- (NSString*) getRecordFilePath;
- (BOOL) copyEncoderCookieToFile;
- (void) removeRecordedFile;
- (void) handleNewRecordedBuffer:(PalQueueTask*)task;

- (void) handleInputBuffer:(AudioQueueRef)inAQ
                    buffer:(AudioQueueBufferRef)inBuffer
                 startTime:(const AudioTimeStamp*)inStartTime
                numPackets:(UInt32)inNumPackets
         packetDescription:(const AudioStreamPacketDescription*)inPacketDesc;

- (void) handlePropertyChanged:(AudioQueueRef)inAQ property:(AudioQueuePropertyID)inID;

@end


static void PalQueueAudioRecorder_HandleInputBuffer(void*                         inUserData,
                                             AudioQueueRef                        inAQ,
                                             AudioQueueBufferRef                  inBuffer,
                                             const AudioTimeStamp*                inStartTime,
                                             UInt32                               inNumPackets,
                                             const AudioStreamPacketDescription*  inPacketDesc)
{
    // Invoked from internal iOS thread!
    PalQueueAudioRecorder* palQueueAudioRecorder = (PalQueueAudioRecorder*)(inUserData);
    [palQueueAudioRecorder handleInputBuffer:inAQ
                                      buffer:inBuffer
                                   startTime:inStartTime
                                  numPackets:inNumPackets
                           packetDescription:inPacketDesc];
}

static void PalQueueAudioRecorder_HandlePropertyChanged(void*                inUserData,
                                                        AudioQueueRef        inAQ,
                                                        AudioQueuePropertyID inID)
{
    PalQueueAudioRecorder* palQueueAudioRecorder = (PalQueueAudioRecorder*)inUserData;
    [palQueueAudioRecorder handlePropertyChanged:inAQ property:inID];
}

@implementation PalQueueAudioRecorder

#pragma mark -
#pragma mark Public Methods

- (id) initWithPal:(PAL_Instance*)pal audioFormat:(ABPAL_AudioFormat)format
{
    if (self = [super init])
    {
        if (![self initQueueAudioRecorder:pal audioFormat:format])
        {
            [self release];
            self = nil;
        }
    }

    return self;
}

- (void) dealloc
{
    if (m_queue)
    {
        AudioQueueStop(m_queue, true);
        AudioQueueDispose(m_queue, true);
        m_queue = NULL;
    }

    if (m_buffers)
    {
        nsl_free(m_buffers);
        m_buffers = NULL;
    }

    [self resetAudioSession];
    [self closeRecordFile];
    [self removeRecordedFile];
    [self freeLeveMeteringBuffers];
    [self freeRecordedBufferQueue];

    [m_meterTable release];
    [m_recorderTaskQueue release];

    if (m_queueAccessLock)
    {
        PAL_LockDestroy(m_queueAccessLock);
        m_queueAccessLock = NULL;
    }

    if (m_loudnessLevelAccessLock)
    {
        PAL_LockDestroy(m_loudnessLevelAccessLock);
        m_loudnessLevelAccessLock = NULL;
    }

    [super dealloc];
}

- (PAL_Error) startRecord:(ABPAL_AudioRecorderCallback*)callback userData:(void*)data;
{
    // Save callback function and data
    m_recorderCallBack = callback;
    m_userData = data;

    PAL_Error err = [self doPrepareToRecord:NO];
    return err;
}

- (PAL_Error) prepareToRecord
{
    PAL_Error err = [self doPrepareToRecord:YES];
    return err;
}

- (PAL_Error) stop
{
    PAL_LockLock(m_loudnessLevelAccessLock);

    // If in ready to record state just reset audio session, close record file and notify client with ended state
    if (m_recorderState == ABPAL_AudioState_ReadyToRecord)
    {
        [self resetAudioSession];
        [self closeRecordFile];

        m_recorderState = ABPAL_AudioState_Ended;
        [self notifyClientWithRecorderState:m_recorderState];
    }

    // Continue only in recording state
    if (m_recorderState != ABPAL_AudioState_Recording)
    {
        PAL_LockUnlock(m_loudnessLevelAccessLock);
        return PAL_Ok;
    }

    // Set recorder state as stopped
    PAL_LockLock(m_queueAccessLock);
    m_recorderState = ABPAL_AudioState_Stopped;
    // Add null last chunk to grow it in further in stream recording mode
    if (m_streamRecordingMode)
    {
        [self addRecordedBufferToQueue:NULL bufferSize:0 withCopy:NO];
    }
    PAL_LockUnlock(m_queueAccessLock);

    PAL_Error err = PAL_Ok;

    // Synchronously stops recording
    if (AudioQueueStop(m_queue, true))
    {
        err = PAL_ErrAudioGeneral;
        m_recorderState = ABPAL_AudioState_Error;
    }

    [self resetAudioSession];
    [self disableLevelMetering];

    PAL_LockUnlock(m_loudnessLevelAccessLock);

    if (!m_streamRecordingMode) // Write to file mode
    {
        [self copyEncoderCookieToFile];
        [self closeRecordFile];
        [self storeRecordedFileToBufferQueue];
        [self removeRecordedFile];
    }

    // Set recorder state as ended
    m_recorderState = ABPAL_AudioState_Ended;
    [self notifyClientWithRecorderState:m_recorderState];

    return err;
}

- (ABPAL_AudioState) state
{
    return m_recorderState;
}

- (PAL_Error) getRecordedBuffer:(byte**)buffer bufferSize:(uint32*)size
{
    if (!buffer || !size)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error palErr = PAL_ErrNoData;
    PalQueueTask* firstChunk = nil;
    PalAudioBuffer* audioBuffer = nil;

    PAL_LockLock(m_queueAccessLock);
    if ([m_recorderTaskQueue count])
    {
        // Get buffer from queue head
        firstChunk = (PalQueueTask*)[[[m_recorderTaskQueue objectAtIndex:0] retain] autorelease];
        [m_recorderTaskQueue removeObjectAtIndex:0];
    }
    PAL_LockUnlock(m_queueAccessLock);

    if (firstChunk)
    {
        audioBuffer = (PalAudioBuffer*)firstChunk.m_object;

        // Guard against null data
        if (![audioBuffer isEmpty])
        {
            byte* chunkData = NULL;
            uint32 chunkDataSize = 0;
            byte* data = NULL;
            uint32 dataSize = 0;

            [audioBuffer getData:&chunkData size:&chunkDataSize];

            // Cut off header for raw format in write to file mode
            if (!m_streamRecordingMode && (m_audioFormat == ABPAL_AudioFormat_RAW) && (chunkDataSize > kRawHeaderSize))
            {
                // TODO:: The codes are incorrent because of dataSize == 0,
                // guys who work on this part should take a investigation for it.
                data = (byte*)nsl_malloc(dataSize);
                if (data)
                {
                    dataSize = chunkDataSize - kRawHeaderSize;
                    nsl_memcpy(data, chunkData + kRawHeaderSize, dataSize);
                    palErr = PAL_Ok;
                }

                nsl_free(chunkData);
                chunkData = NULL;
                chunkDataSize = 0;
            }
            else
            {
                dataSize = chunkDataSize;
                data = chunkData;

                chunkData = NULL;
                chunkDataSize = 0;
                palErr = PAL_Ok;
            }

            if (palErr == PAL_Ok)
            {
                // Client responsible for freeing data
                *buffer = data;
                *size = dataSize;

                data = NULL;
                dataSize = 0;
            }
        }

        [firstChunk stopTask];
        firstChunk = nil;
    }

    return palErr;
}

- (PAL_Error) getLoudnessLevel:(uint32*)loudnessLevel
{
    PAL_LockLock(m_loudnessLevelAccessLock);

    PAL_Error palErr = PAL_Ok;
    UInt32 size = 0;

    if (m_recorderState != ABPAL_AudioState_Recording)
    {
        palErr = PAL_ErrAudioGeneral;
        goto error;
    }

    if (!loudnessLevel)
    {
        palErr = PAL_ErrBadParam;
        goto error;
    }

    if (![self enableLevelMetering])
    {
        palErr = PAL_ErrAudioGeneral;
        goto error;
    }

    if (!m_meterTable)
    {
        m_meterTable = [[PalMeterTable alloc] init:-60.0 maxDecibels:-10.0 tableSize:400.0 root:2.0];
    }

    size = m_recordFormat.mChannelsPerFrame * sizeof(AudioQueueLevelMeterState);
    if (AudioQueueGetProperty(m_queue, kAudioQueueProperty_CurrentLevelMeterDB, m_chanelsLevel, &size))
    {
        palErr = PAL_ErrAudioGeneral;
        goto error;
    }

    *loudnessLevel = [m_meterTable valueAt:m_chanelsLevel[0].mAveragePower];

    PAL_LockUnlock(m_loudnessLevelAccessLock);
    return PAL_Ok;

error:

    PAL_LockUnlock(m_loudnessLevelAccessLock);
    return palErr;
}

- (PAL_Error) setAudioSessionType:(ABPAL_AudioSessionType)audioSessionType
{
    // Only Record and PlayAndRecord audio sessions are allowed
    switch (audioSessionType)
    {
        case ABPAL_AudioSession_Record:
        case ABPAL_AudioSession_PlayAndRecord:
            break;

        default:
            return PAL_ErrBadParam;
    }

    m_audioSessionType = audioSessionType;

    return PAL_Ok;
}

- (PAL_Error) setStreamRecordingModeWithBufferDuration:(uint32)duration
{
    if (m_streamRecordingMode)
        return PAL_Ok;

    if (m_buffers)
        return PAL_ErrAudioGeneral;

    // Limit buffer duration to be [50, kAggregatedBufferDuration]
    if (((duration > 0) && (duration < 50)) || duration > kAggregatedBufferDuration)
        return PAL_ErrAudioGeneral;

    m_streamRecordingMode = YES;

    // Keep default value if passed duration is 0
    if (duration != 0)
        m_bufferDuration = duration;

    // Store client thread id
    m_clientThread = [NSThread currentThread];

    return PAL_Ok;
}

- (PAL_Error) setSampleRate:(uint32)sampleRate
{
    if (sampleRate != 8000 && sampleRate != 16000)
        return PAL_ErrUnsupported;

    m_sampleRate = sampleRate;

    return PAL_Ok;
}

- (PAL_Error) getSampleRate:(uint32*)sampleRate
{
    if (!sampleRate)
        return PAL_ErrBadParam;

    *sampleRate = (uint32)m_recordFormat.mSampleRate;

    return PAL_Ok;
}

- (PAL_Error) setBitsPerSample:(uint32)bitsPerSample
{
    if (bitsPerSample != 8 && bitsPerSample != 16)
        return PAL_ErrUnsupported;

    m_bitsPerSample = bitsPerSample;

    return PAL_Ok;
}

- (PAL_Error) getBitsPerSample:(uint32*)bitsPerSample
{
    if (!bitsPerSample)
        return PAL_ErrBadParam;

    *bitsPerSample = (uint32)m_recordFormat.mBytesPerFrame * 8;

    return PAL_Ok;
}

#pragma mark -
#pragma mark Private Methods

- (BOOL) initQueueAudioRecorder:(PAL_Instance*)pal audioFormat:(ABPAL_AudioFormat)format
{
    if (!pal)
        return NO;

    m_pal = pal;

    // Create locker to control access to recorded buffer queue
    if (PAL_LockCreate(m_pal, &m_queueAccessLock) != PAL_Ok)
        return NO;

    // Create locker to control access to loudness level
    if (PAL_LockCreate(m_pal, &m_loudnessLevelAccessLock) != PAL_Ok)
        return NO;

    // Record is default audio session type for recorder
    m_audioSessionType = ABPAL_AudioSession_Record;

    // Set current state as Init
    m_recorderState = ABPAL_AudioState_Init;

    // Set default buffer duration. Write to file mode by default.
    m_streamRecordingMode = NO;
    m_bufferDuration = kDefaultBufferDuration;
    m_sampleRate = kDefaultSampleRate;
    m_bitsPerSample = kDefaultBitsPerSample;
    m_audioFormat = format;

    // Queue to keep recorded buffers
    m_recorderTaskQueue = [[NSMutableArray alloc] init];

    return YES;
}

- (BOOL) setupAudioFormat:(ABPAL_AudioFormat)format
{
    // Only support raw/wav audio recorder format for iPhone project
    switch (format)
    {
        case ABPAL_AudioFormat_UNK:
        case ABPAL_AudioFormat_RAW:
        case ABPAL_AudioFormat_WAV:
            break;

        default:
            return NO;
    };

    nsl_memset(&m_recordFormat, 0, sizeof(m_recordFormat));

    if (m_audioFormat == ABPAL_AudioFormat_RAW)
    {
        m_recordFormat.mFormatID = kAudioFormatULaw;
    }
    else
    {
        m_recordFormat.mFormatID = kAudioFormatLinearPCM;
    }

    m_recordFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    m_recordFormat.mSampleRate = m_sampleRate;
    m_recordFormat.mBitsPerChannel = m_bitsPerSample;
    m_recordFormat.mChannelsPerFrame = 1; // Check next line and rework getLoudnessLevel if mChannelsPerFrame will become another than 1
    m_recordFormat.mBytesPerPacket = m_recordFormat.mBytesPerFrame = (m_recordFormat.mBitsPerChannel / 8);// * m_recordFormat.mChannelsPerFrame;
    m_recordFormat.mFramesPerPacket = 1;

    return YES;
}

- (int) deriveRecordBufferSize:(uint32)mseconds
{
    int packets = 0;
    int bytes = 0;
    float seconds = (float)mseconds / 1000.0;
    int frames = (int)ceil(seconds * m_recordFormat.mSampleRate);

    if (m_recordFormat.mBytesPerFrame > 0)
    {
        bytes = frames * m_recordFormat.mBytesPerFrame;
    }
    else
    {
        UInt32 maxPacketSize = 0;

        if (m_recordFormat.mBytesPerPacket > 0)
        {
            // Constant packet size
            maxPacketSize = m_recordFormat.mBytesPerPacket;
        }
        else
        {
            UInt32 propertySize = sizeof(maxPacketSize);
            if (AudioQueueGetProperty(m_queue, kAudioQueueProperty_MaximumOutputPacketSize, &maxPacketSize, &propertySize))
                return 0;
        }

        if (m_recordFormat.mFramesPerPacket > 0)
        {
            packets = frames / m_recordFormat.mFramesPerPacket;
        }
        else
        {
            // Worst-case scenario: 1 frame in a packet
            packets = frames;
        }

        // Sanity check
        if (packets == 0)
        {
            packets = 1;
        }

        bytes = packets * maxPacketSize;
    }

    return bytes;
}

- (void) handleInputBuffer:(AudioQueueRef)inAQ
                    buffer:(AudioQueueBufferRef)inBuffer
                 startTime:(const AudioTimeStamp*)inStartTime
                numPackets:(UInt32)inNumPackets
         packetDescription:(const AudioStreamPacketDescription*)inPacketDesc
{
    // Invoked from internal iOS thread!
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    PAL_LockLock(m_queueAccessLock);

    if (inNumPackets > 0)
    {
        if (m_streamRecordingMode) //Stream mode
        {
            if (inBuffer->mAudioData && inBuffer->mAudioDataByteSize)
            {
                if (m_recorderState == ABPAL_AudioState_Recording)
                {
                    [self addRecordedBufferToQueue:(byte*)(inBuffer->mAudioData) bufferSize:(uint32)(inBuffer->mAudioDataByteSize) withCopy:YES];

                    // Notify client with new buffer available
                    PalQueueTask* task = (PalQueueTask*)[m_recorderTaskQueue lastObject];
                    [task startTask];
                }
                else if (m_recorderState == ABPAL_AudioState_Stopped)
                {
                    [self appendRecordedBufferToLastChunkInQueue:(byte*)(inBuffer->mAudioData) bufferSize:(uint32)(inBuffer->mAudioDataByteSize)];
                }
            }
        }
        else // Write to file mode
        {
            if (!AudioFileWritePackets(m_recordFile, FALSE, inBuffer->mAudioDataByteSize, inPacketDesc, m_recordPacket, &inNumPackets, inBuffer->mAudioData))
            {
                m_recordPacket += inNumPackets;
            }
        }
    }

    // If we're not stopping, re-enqueue the buffer so that it gets filled again
    if (m_recorderState == ABPAL_AudioState_Recording)
    {
        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }

    PAL_LockUnlock(m_queueAccessLock);
    [pool release];
}

- (void) handlePropertyChanged:(AudioQueueRef)inAQ property:(AudioQueuePropertyID)inID
{
    // We are interesting only in IsRunning property state
    switch (inID)
    {
        case kAudioQueueProperty_IsRunning:
        {
            UInt32 state = 0;
            UInt32 stateSize = sizeof(state);

            AudioQueueGetProperty(inAQ, kAudioQueueProperty_IsRunning, &state, &stateSize);

            if (!state)
            {
                // If audio queue is stoped let's stop recorder anyway we will not catch recorded buffer any more
                [self stop];
            }

            break;
        }

        default:
            break;
    }
}

- (BOOL) initAudioSession
{
    [self resetAudioSession];

    PalAudioSessionManager* palAudioSessionManager = PalAudioSessionFactory::getInstance(m_pal);
    m_audioSession = (ABPAL_AudioSession*)[palAudioSessionManager OpenAudioSession:m_audioSessionType andRecorder:self];

    return (m_audioSession != NULL);
}

- (void) resetAudioSession
{
    if (m_audioSession)
    {
        ABPAL_AudioSessionDestroy(m_audioSession);
        m_audioSession = NULL;
    }
}

- (PAL_Error) doPrepareToRecord:(BOOL)paused
{
    if (m_recorderState == ABPAL_AudioState_Recording)
        return PAL_ErrAudioBusy;

    if (m_recorderState == ABPAL_AudioState_ReadyToRecord)
    {
        PAL_Error err = PAL_ErrAudioGeneral;

        if (!paused)
        {
            // Start recording as soon as possible if in ready to record state
            if (!AudioQueueStart(m_queue, NULL))
            {
                err = PAL_Ok;
                m_recorderState = ABPAL_AudioState_Recording;
            }
        }

        return err;
    }

    PAL_Error err = PAL_ErrAudioGeneral;
    m_recorderState = ABPAL_AudioState_Error;

    if (!m_queue)
    {
        // Specify the recording format
        if (![self setupAudioFormat:m_audioFormat])
            return err;

        // Create the queue.
        if (AudioQueueNewInput(&m_recordFormat, PalQueueAudioRecorder_HandleInputBuffer, self /* userData */,
                               NULL /* run loop */, NULL /* run loop mode */, 0 /* flags */, &m_queue))
            return err;

        // Add property linster to catch stop recorder by iOS
        AudioQueueAddPropertyListener(m_queue, kAudioQueueProperty_IsRunning, PalQueueAudioRecorder_HandlePropertyChanged, self);

        // Get the record format back from the queue's audio converter
        // the file may require a more specific stream description than was necessary to create the encoder.
        UInt32 size = sizeof(m_recordFormat);
        if (AudioQueueGetProperty(m_queue, kAudioQueueProperty_StreamDescription, &m_recordFormat, &size))
            return err;

        m_sampleRate = m_recordFormat.mSampleRate;
        m_bitsPerSample = m_recordFormat.mBytesPerFrame * 8;
    }

    // Allocates buffers if it didn't yet
    if (!m_buffers)
    {
        m_buffersCount = (uint32)ceil(kAggregatedBufferDuration / m_bufferDuration);
        m_buffers = (AudioQueueBufferRef*)nsl_malloc(sizeof(AudioQueueBufferRef) * m_buffersCount);
        if (!m_buffers)
            return PAL_ErrNoMem;

        int bufferByteSize = [self deriveRecordBufferSize:m_bufferDuration];
        for (int bufferIndex = 0; bufferIndex < m_buffersCount; bufferIndex++)
        {
            if (AudioQueueAllocateBuffer(m_queue, bufferByteSize, &m_buffers[bufferIndex]))
                return PAL_ErrAudioGeneral;
        }
    }

    // Enqueue buffers
    for (int bufferIndex = 0; bufferIndex < m_buffersCount; bufferIndex++)
    {
        if (AudioQueueEnqueueBuffer(m_queue, m_buffers[bufferIndex], 0, NULL))
            return err;
    }

    if (!m_streamRecordingMode) // Write to file mode
    {
        [self closeRecordFile];

        CFURLRef url = CFURLCreateWithString(kCFAllocatorDefault, (CFStringRef)[self getRecordFilePath], NULL);

        if (AudioFileCreateWithURL(url, kAudioFileCAFType, &m_recordFormat, kAudioFileFlags_EraseFile, &m_recordFile))
        {
            CFRelease(url);
            return err;
        }

        CFRelease(url);

        [self copyEncoderCookieToFile];
        m_recordPacket = 0;
    }

    // Setup audio session
    if (![self initAudioSession])
        return err;

    [self freeRecordedBufferQueue];

    if (paused)
    {
        err = PAL_Ok;
        m_recorderState = ABPAL_AudioState_ReadyToRecord;
    }
    else
    {
        if (!AudioQueueStart(m_queue, NULL))
        {
            err = PAL_Ok;
            m_recorderState = ABPAL_AudioState_Recording;
        }
    }

    if (err != PAL_Ok)
    {
        [self resetAudioSession];
        [self closeRecordFile];
    }

    return err;
}

- (void) notifyClientWithRecorderState:(ABPAL_AudioState)state
{
    if (m_recorderCallBack)
    {
        m_recorderCallBack(m_userData, state);
    }
}

- (BOOL) enableLevelMetering
{
    if (!m_chanelsLevel)
    {
        // Enable level metering
        UInt32 enableLevelMetering = 1;
        if (AudioQueueSetProperty(m_queue, kAudioQueueProperty_EnableLevelMetering, &enableLevelMetering, sizeof(UInt32)))
            return NO;

        return [self allocateLevelMeteringBuffers];
    }

    return YES;
}

- (void) disableLevelMetering
{
    if (m_chanelsLevel)
    {
        // Disable level metering
        UInt32 disableLevelMetering = 0;
        AudioQueueSetProperty(m_queue, kAudioQueueProperty_EnableLevelMetering, &disableLevelMetering, sizeof(UInt32));

        [self freeLeveMeteringBuffers];
    }
}

- (BOOL) allocateLevelMeteringBuffers
{
    [self freeLeveMeteringBuffers];

    m_chanelsLevel = (AudioQueueLevelMeterState*)nsl_malloc(m_recordFormat.mChannelsPerFrame * sizeof(AudioQueueLevelMeterState));

    return (m_chanelsLevel != NULL);
}

- (void) freeLeveMeteringBuffers
{
    if (m_chanelsLevel)
    {
        nsl_free(m_chanelsLevel);
        m_chanelsLevel = NULL;
    }
}

- (BOOL) addRecordedBufferToQueue:(byte*)buffer bufferSize:(uint32)bufferSize withCopy:(BOOL)copy
{
    PalAudioBuffer* audioBuffer = [[[PalAudioBuffer alloc] initWithData:buffer size:bufferSize withCopy:copy] autorelease];
    PalQueueTask* task = [[[PalQueueTask alloc] initWithObject:audioBuffer target:self action:@selector(handleNewRecordedBuffer:) performThread:m_clientThread] autorelease];
    [m_recorderTaskQueue addObject:task];
    return YES;
}

- (BOOL) appendRecordedBufferToLastChunkInQueue:(byte*)buffer bufferSize:(uint32)bufferSize
{
    if (buffer && bufferSize && [m_recorderTaskQueue count])
    {
        PalQueueTask* task = (PalQueueTask*)[m_recorderTaskQueue lastObject];
        PalAudioBuffer* audioBuffer = (PalAudioBuffer*)task.m_object;
        if (audioBuffer)
        {
            [audioBuffer appendData:buffer size:bufferSize];
            return YES;
        }
    }

    return NO;
}

- (BOOL) storeRecordedFileToBufferQueue
{
    [self freeRecordedBufferQueue];

    byte* buffer = NULL;
    uint32 bufferSize = 0;

    if (PAL_FileLoadFile(m_pal, [[self getRecordFilePath] UTF8String], &buffer, &bufferSize) == PAL_Ok)
    {
        if ([self addRecordedBufferToQueue:buffer bufferSize:bufferSize withCopy:NO])
        {
            return YES;
        }
    }

    return NO;
}

- (void) freeRecordedBufferQueue
{
    if ([m_recorderTaskQueue count])
    {
        for (id object in m_recorderTaskQueue)
        {
            PalQueueTask* task = (PalQueueTask*)object;
            [task stopTask];
        }

        [m_recorderTaskQueue removeAllObjects];
    }
}

- (void) handleNewRecordedBuffer:(PalQueueTask*)task
{
    BOOL taskIsValid = NO;

    PAL_LockLock(m_queueAccessLock);
    if (task && [m_recorderTaskQueue count])
    {
        for (id object in m_recorderTaskQueue)
        {
            if (object == task) // Task is valid
            {
                taskIsValid = YES;
                break;
            }
        }
    }
    PAL_LockUnlock(m_queueAccessLock);

    if (taskIsValid)
    {
        [self notifyClientWithRecorderState:ABPAL_AudioState_StreamRecordBufferAvailable];
    }
}

- (void) closeRecordFile
{
    if (m_recordFile)
    {
        AudioFileClose(m_recordFile);
        m_recordFile = NULL;
    }
}

- (NSString*) getRecordFilePath
{
    NSString *rootPath = NSTemporaryDirectory();
    NSString *filePath = [rootPath stringByAppendingString:kRecordFileName];
    return filePath;
}

- (BOOL) copyEncoderCookieToFile
{
    UInt32 propertySize = 0;
    char* magicCookie = NULL;
    BOOL result = NO;
    UInt32 magicCookieSize = 0;
    UInt32 willEatTheCookie = 0;

    if (!m_recordFile)
        goto error;

    // Get the magic cookie, if any, from the converter
    if (AudioQueueGetPropertySize(m_queue, kAudioQueueProperty_MagicCookie, &propertySize))
        goto error;

    if (propertySize <= 0)
        goto error;

    // We can get a noErr result and also a propertySize == 0
    // If the file format does support magic cookies, but this file doesn't have one.

    magicCookie = (char*)nsl_malloc(propertySize * sizeof(char));
    if (!magicCookie)
        goto error;

    if (AudioQueueGetProperty(m_queue, kAudioQueueProperty_MagicCookie, magicCookie, &propertySize))
        goto error;

    magicCookieSize = propertySize;

    // The converter wants to give us one; will the file take it?
    if (AudioFileGetPropertyInfo(m_recordFile, kAudioFilePropertyMagicCookieData, NULL, &willEatTheCookie))
        goto error;

    if (willEatTheCookie)
    {
        if (AudioFileSetProperty(m_recordFile, kAudioFilePropertyMagicCookieData, magicCookieSize, magicCookie))
            goto error;
    }

    goto success;

    error:
    {
        result = NO;
        goto cleanup;
    }

    success:
    {
        result = YES;
        goto cleanup;
    }

    cleanup:
    {
        if (magicCookie)
        {
            nsl_free(magicCookie);
            magicCookie = NULL;
        }

        return result;
    }
}

- (void) removeRecordedFile
{
    if (PAL_FileExists(m_pal, [[self getRecordFilePath] UTF8String]) == PAL_Ok)
    {
        PAL_FileRemove(m_pal, [[self getRecordFilePath] UTF8String]);
    }
}

@end
