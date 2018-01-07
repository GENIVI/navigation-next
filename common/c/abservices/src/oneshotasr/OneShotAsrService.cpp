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

 @file     OneShotAsrService.cpp
 @date     02/09/2012
 @defgroup AB One Shot Asr Service API

 @brief    AB One Shot Asr Service Implementation

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.           

 The information contained herein is confidential, proprietary 
 to TeleCommunication Systems, Inc., and considered a trade secret as 
 defined in section 499C of the penal code of the State of     
 California. Use of this information by anyone other than      
 authorized employees of TeleCommunication Systems is granted only    
 under a written non-disclosure agreement, expressly           
 prescribing the scope and manner of such use.                 

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "OneShotAsrServiceInterface.h"
#include "OneShotAsrParameters.h"
#include "SpeechProcessorInterface.h"
#include "SpeexEncoderInterface.h"
#include "SpeechStreamHandlerInterface.h"
#include "nbcontextprotected.h"       // To get pal instance
#include "pallock.h"
#include "abqalog.h"
#include <list>

// Used only for dev purpose. Should be commented out on checkin.
//#define FEATURE_DEBUG_OSASR

#ifdef FEATURE_DEBUG_OSASR
  #include "palclock.h"
  #include "palfile.h"
#endif //FEATURE_DEBUG_OSASR

using namespace std;
using namespace abspeechprocessor;
using namespace tpspeexencoder;


#define osasr_ceil(a)  ((((double)(a) - (double)((int32)(a))) > 0.0) ? ((int32)(a) + 1) : ((int32)(a)))


namespace aboneshotasr
{

/*! Default values for One Shot Asr config
 */
static const uint32 OSASR_CONFIG_DEFAULT_UPLOAD_DATA_SIZE =               32000;   // 32 kb
static const uint32 OSASR_CONFIG_DEFAULT_UPLOAD_INTERVAL =                  260;   // 260 msec
static const uint32 OSASR_CONFIG_DEFAULT_MAX_RECORD_TIME =                10000;   // 10 sec
static const uint32 OSASR_CONFIG_DEFAULT_NO_SPEECH_TIMEOUT =               1000;   // 1 sec
static const uint32 OSASR_CONFIG_DEFAULT_RECORDER_SAMPLE_RATE =            8000;   // 8 kHz
static const uint32 OSASR_CONFIG_DEFAULT_RECORDER_BITS_PER_SAMPLE =          16;   // 16 bit
static const uint32 OSASR_CONFIG_DEFAULT_RECORDER_BUFFER_DURATION =         200;   // 200 msec


/*! Audio Buffer to store audio data. For internal One Shot Asr usage only.
 */
class AudioBuffer : public Base
{
public:
    AudioBuffer(byte* data, uint32 dataSize, int32 index = 0); // AudioBuffer become owner of the passed data
    AudioBuffer(AudioBuffer& audioBuffer);
    ~AudioBuffer(void);

    byte*       GetData(void) const;
    uint32      GetDataSize(void) const;
    int32       GetIndex(void) const;
    bool        IsEmpty(void) const;
    void        SetIndex(int32 index);
    bool        AppendBuffer(AudioBuffer* audioBuffer);

private:
    byte*       m_data;
    uint32      m_dataSize;
    int32       m_index;
};

AudioBuffer::AudioBuffer(byte* data, uint32 dataSize, int32 index) :
    m_data(data),
    m_dataSize(dataSize),
    m_index(index)
{
}

AudioBuffer::AudioBuffer(AudioBuffer& audioBuffer) :
    m_data(NULL),
    m_dataSize(0),
    m_index(audioBuffer.GetIndex())
{
    if (!audioBuffer.IsEmpty())
    {
        m_data = reinterpret_cast<byte*>(nsl_malloc(audioBuffer.GetDataSize()));
        if (m_data)
        {
            m_dataSize = audioBuffer.GetDataSize();
            nsl_memcpy(m_data, audioBuffer.GetData(), m_dataSize);
        }
    }
}

AudioBuffer::~AudioBuffer(void)
{
    if (m_data)
    {
        nsl_free(m_data);
        m_data = NULL;
        m_dataSize = 0;
    }
}

byte*
AudioBuffer::GetData(void) const
{
    return m_data;
}

uint32
AudioBuffer::GetDataSize(void) const
{
    return m_dataSize;
}

int32
AudioBuffer::GetIndex(void) const
{
    return m_index;
}

bool
AudioBuffer::IsEmpty(void) const
{
    return (!GetData() || !GetDataSize());
}

void
AudioBuffer::SetIndex(int32 index)
{
    m_index = index;
}

bool
AudioBuffer::AppendBuffer(AudioBuffer* audioBuffer)
{
    if (!audioBuffer || audioBuffer->IsEmpty())
    {
        return FALSE;
    }

    // Extend buffer to hold passed buffer
    {
        byte* data = reinterpret_cast<byte*>(nsl_realloc(m_data, m_dataSize + audioBuffer->GetDataSize()));
        if (!data)
        {
            return FALSE;
        }

        m_data = data;
    }

    nsl_memcpy(m_data + m_dataSize, audioBuffer->GetData(), audioBuffer->GetDataSize());
    m_dataSize += audioBuffer->GetDataSize();

    return TRUE;
}

/*! OneShotAsrService states enum
 */
enum OneShotAsrState
{
    OSASR_State_Idle = 0,                  /*! OSASR Service instantiated with client config. Service is ready to start. */
    OSASR_State_PrepareToRecording,        /*! Prepare to recording. Initialazing recorder with play ready to record tone. */
    OSASR_State_Recording,                 /*! Recorder is started but begin of speech is not detected. */
    OSASR_State_Listening,                 /*! Begin of speech detected. Waiting for end of speech that can be initiated by follow ways: max recording timeout, speech processor, client action. */
    OSASR_State_WaitingLastBuffer,         /*! Begin of speech detected. End of speech initiated by client. Recorder is stopped. Waiting for last buffer from recorder to declare end of speech. */
    OSASR_State_Recognition,               /*! End of speech detected. Sending last chunck to server and waiting results. */
    OSASR_State_Destroy                    /*! OSASR Service is going to destroy. */
};

/*! OneShotAsrService QA log event enum
 */
enum OneShotAsrQaLogEvent
{
    OSASR_QaLogEvent_Initialized = 0,
    OSASR_QaLogEvent_RecordingStarted,
    OSASR_QaLogEvent_RecordingStopped,
    OSASR_QaLogEvent_ListeningStarted,
    OSASR_QaLogEvent_ListeningStopped,
    OSASR_QaLogEvent_NoSpeechDetected,
    OSASR_QaLogEvent_StoppedByTheUser,
    OSASR_QaLogEvent_RecognitionCanceled
};

class OneShotAsrService : public OneShotAsrServiceInterface,
                          public SpeechStreamListenerInterface
{
public:
    OneShotAsrService(void);
    NB_Error Init(NB_Context* context,
                  OneShotAsrConfig* config,
                  OneShotAsrListenerInterface* listener);
    virtual ~OneShotAsrService(void);

    // Overrides OneShotAsrServiceInterface
    virtual void Release(void);
    virtual NB_Error StartOneShotAsr(OneShotAsrParametersInterface* parameters);
    virtual NB_Error Cancel(void);
    virtual NB_Error StopRecording(void);
    virtual NB_Error GetInformation(OneShotAsrInformationInterface** information);
    virtual NB_Error GetLoudnessLevel(OneShotAsrLoudnessLevel* loudnessLevel);

protected:
    // State machine staff
    NB_Error SetState(OneShotAsrState state, OneShotAsrAnnounce idleStateProceedAnnounce = OSASR_Announce_End); // Note: For idle and destroy states return value could be ignored.
    void ResetToIdleState(void);

    // Audio Session staff
    PAL_Error StartPlayAndRecordAudioSession(void);
    void StopPlayAndRecordAudioSession(void);

    // Audio Player staff
    PAL_Error PlayAnnounce(OneShotAsrAnnounce announce);
    void StopPlayer(void);
    void DestroyPlayer(void);
    static void AudioPlayerCallback(void* userData, ABPAL_AudioState state);
    void HandleAudioPlayerCallback(ABPAL_AudioState state);

    // Audio Recorder staff
    PAL_Error PrepareToRecording(void);
    PAL_Error StartRecorder(void);
    void AtomicStopRecorder(void);
    void DestroyRecorder(void);
    static void AudioRecorderCallback(void* userData, ABPAL_AudioState state);
    void HandleAudioRecorderCallback(ABPAL_AudioState state);

    // Audio buffer processing pipeline staff
    void DestroySendAudioBuffer(void);
    int32 GetBeginOfSpeechBufferIndex(void);
    int32 GetMaxRecordBufferIndex(void);
    int32 GetNoSpeechBufferIndex(void);
    int32 GetUploadBufferIndex(void);
    void AudioBufferProcessingPipeline(AudioBuffer& audioBuffer, bool lastBuffer);
    NB_Error AudioBufferProcessingPipelineSpeechDetection(AudioBuffer& audioBuffer);
    NB_Error AudioBufferProcessingPipelineEncode(AudioBuffer& audioBuffer);
    NB_Error AudioBufferProcessingPipelineTryToSend(void);

    // Overrides SpeechStreamParametersInterface
    virtual void SpeechStreamHandleEvent(SpeechStreamEvent event, uint32 param);

    // Common staff
    void DestroyClientParameters(void);
    void DestroyReplyInformation(void);

    // QA log staff
    void LogOneShotAsrEvent(OneShotAsrQaLogEvent event);

    // Dev debug staff
#ifdef FEATURE_DEBUG_OSASR
    PAL_Error StoreToFileAudioBuffer(AudioBuffer* audioBuffer);
    int32 WriteWavHeader(PAL_File* palFile, int32 rate, int32 channels, int32 format);
    void DestroyStoreToFileBuffer(void);
#endif //FEATURE_DEBUG_OSASR

private:
    NB_Context*                                        m_context;                               /*! NB context instance. */
    PAL_Instance*                                      m_pal;                                   /*! Pal instance. */
    OneShotAsrListenerInterface*                       m_listener;                              /*! OSASR listener, implemented by client. */
    OneShotAsrConfig                                   m_config;                                /*! OSASR configuration, retrieved from client. */
    OneShotAsrParameters*                              m_clientParameters;                      /*! OSASR parameters to start retrieved from client. */
    SpeechStreamInformation*                           m_replyInformation;                      /*! Speech stream information returned from server. */
    AudioBuffer*                                       m_announceBuffer[OSASR_Announce_End];    /*! Buffer to store announce data retrieved from client. */
    ABPAL_AudioPlayer*                                 m_player;                                /*! Pal audio player. */
    ABPAL_AudioRecorder*                               m_recorder;                              /*! Pal audio recorder. See m_recorderAccessLock. */
    ABPAL_AudioSession*                                m_audioSession;                          /*! Shared play and record pal audio session for recorder and player to reduce audio session switch. */
    PAL_Lock*                                          m_recorderAccessLock;                    /*! All access to recorder should be done thru this locker. Since it can be accessed from different threads. */
    OneShotAsrState                                    m_state;                                 /*! OSASR current state. */
    SpeechProcessorInterface*                          m_speechProcessor;                       /*! Speech processor instance used for voice detection. */
    SpeexEncoderInterface*                             m_speexEncoder;                          /*! Speex encoder used to encode recorded buffer to 3rd party speex audio format. */
    SpeechStreamHandlerInterface*                      m_speechStreamHandler;                   /*! Speech stream handler. */

    int32                                              m_recordedBufferIndex;                   /*! Current recorded audio buffer index. */
    int32                                              m_noSpeechBufferSequence;                /*! Length of not audible buffer sequence. Used to detect no speech. */
    int32                                              m_audioBufferSendIndex;                  /*! Audio buffer send to server index. */
    AudioBuffer*                                       m_audioBufferToSend;                     /*! Audio buffer to send on server. */

    // Dev debug staff
#ifdef FEATURE_DEBUG_OSASR
    AudioBuffer*                                       m_storeToFileBuffer;                     /*! Buffer to keep all recorded buffer to write to file. */
#endif //FEATURE_DEBUG_OSASR
};

OneShotAsrService::OneShotAsrService(void) :
    m_context(NULL),
    m_pal(NULL),
    m_listener(NULL),
    m_clientParameters(NULL),
    m_replyInformation(NULL),
    m_player(NULL),
    m_recorder(NULL),
    m_audioSession(NULL),
    m_recorderAccessLock(NULL),
    m_state(OSASR_State_Idle),
    m_speechProcessor(NULL),
    m_speexEncoder(NULL),
    m_speechStreamHandler(NULL),
    m_recordedBufferIndex(0),
    m_noSpeechBufferSequence(0),
    m_audioBufferSendIndex(0),
#ifdef FEATURE_DEBUG_OSASR
    m_storeToFileBuffer(NULL),
#endif //FEATURE_DEBUG_OSASR
    m_audioBufferToSend(NULL)
{
    // Null configs
    nsl_memset(&m_config, 0, sizeof(OneShotAsrConfig));
}

NB_Error
OneShotAsrService::Init(NB_Context* context,
                        OneShotAsrConfig* config,
                        OneShotAsrListenerInterface* listener
                        )
{
    m_context = context;
    m_listener = listener;

    // Copy client configs
    nsl_memcpy(&m_config, config, sizeof(OneShotAsrConfig));

    // Null announce data
    for (int i = OSASR_Announce_RecordingTone; i < OSASR_Announce_End; i++)
    {
        m_announceBuffer[i] = NULL;
    }

    // Replace null configs with default values
    if (m_config.uploadDataSize == 0)
    {
        m_config.uploadDataSize = OSASR_CONFIG_DEFAULT_UPLOAD_DATA_SIZE;
    }

    if (m_config.uploadInterval == 0)
    {
        m_config.uploadInterval = OSASR_CONFIG_DEFAULT_UPLOAD_INTERVAL;
    }

    if (m_config.maxRecordTime == 0)
    {
        m_config.maxRecordTime = OSASR_CONFIG_DEFAULT_MAX_RECORD_TIME;
    }

    if (m_config.beginOfSpeechTimeout == 0)
    {
        /* By default begin of speech timeout is disabled with using value great than max allowed recording time.
           So, max allowed recording timeout fairs early than begin of speech timeout. */
        m_config.beginOfSpeechTimeout = m_config.maxRecordTime + 1;
    }

    if (m_config.noSpeechTimeout == 0)
    {
        m_config.noSpeechTimeout = OSASR_CONFIG_DEFAULT_NO_SPEECH_TIMEOUT;
    }

    if (m_config.recorderSampleRate == 0)
    {
        m_config.recorderSampleRate = OSASR_CONFIG_DEFAULT_RECORDER_SAMPLE_RATE;
    }

    if (m_config.recorderBitsPerSample == 0)
    {
        m_config.recorderBitsPerSample = OSASR_CONFIG_DEFAULT_RECORDER_BITS_PER_SAMPLE;
    }

    if (m_config.recorderBufferDuration == 0)
    {
        m_config.recorderBufferDuration = OSASR_CONFIG_DEFAULT_RECORDER_BUFFER_DURATION;
    }

    // Check supported audio format types
    if (m_config.audioEncoderFormat != ABPAL_AudioFormat_WAV &&
        m_config.audioEncoderFormat != ABPAL_AudioFormat_RAW &&
        m_config.audioEncoderFormat != ABPAL_AudioFormat_SPX)
    {
        return NE_NOSUPPORT;
    }

    // Get pal
    m_pal = NB_ContextGetPal(m_context);
    if (!m_pal)
    {
        return NE_NOMEM;
    }

    PAL_Error palErr = PAL_Ok;

    // Create locker to control access to recorder
    palErr = PAL_LockCreate(m_pal, &m_recorderAccessLock);
    if (palErr != PAL_Ok)
    {
        return NE_NOMEM;
    }

    // Create speech stream handler
    NB_Error err = CreateSpeechStreamHandler(m_context, this, &m_speechStreamHandler);

    return err;
}

OneShotAsrService::~OneShotAsrService(void)
{
    DestroyPlayer();

    PAL_LockLock(m_recorderAccessLock);
    DestroyRecorder();
    PAL_LockUnlock(m_recorderAccessLock);

    StopPlayAndRecordAudioSession();

    if (m_speechProcessor)
    {
        m_speechProcessor->Release();
        m_speechProcessor = NULL;
    }

    if (m_speexEncoder)
    {
        m_speexEncoder->Release();
        m_speexEncoder = NULL;
    }

    if (m_speechStreamHandler)
    {
        m_speechStreamHandler->Release();
        m_speechStreamHandler = NULL;
    }

    for (int i = 0; i < OSASR_Announce_End; i++)
    {
        if (m_announceBuffer[i])
        {
            delete m_announceBuffer[i];
            m_announceBuffer[i] = NULL;
        }
    }

    DestroyClientParameters();
    DestroyReplyInformation();
    DestroySendAudioBuffer();

    if (m_recorderAccessLock)
    {
        PAL_LockDestroy(m_recorderAccessLock);
        m_recorderAccessLock = NULL;
    }

#ifdef FEATURE_DEBUG_OSASR
    DestroyStoreToFileBuffer();
#endif //FEATURE_DEBUG_OSASR
}

void
OneShotAsrService::Release(void)
{
    SetState(OSASR_State_Destroy);
    delete this;
}

NB_Error
OneShotAsrService::StartOneShotAsr(OneShotAsrParametersInterface* parameters)
{
    if (!parameters)
    {
        return NE_BADDATA;
    }

    // Continue only in idle state
    if (m_state != OSASR_State_Idle)
    {
        return NE_UNEXPECTED;
    }

    // Destroy previous parameters and information
    DestroyClientParameters();
    DestroyReplyInformation();

    // The only thing that could be active in idle satet it's player. We need stop it before prepare to record.
    StopPlayer();

    // Copy client parameters
    m_clientParameters = new OneShotAsrParameters(*(static_cast<OneShotAsrParameters*>(parameters)));

    // Mark current state as initialized in QA logs
    LogOneShotAsrEvent(OSASR_QaLogEvent_Initialized);

    // Proceed to prepare to recording state
    NB_Error err = SetState(OSASR_State_PrepareToRecording);
    if (err != NE_OK)
    {
        SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
    }

    return err;
}

NB_Error
OneShotAsrService::Cancel(void)
{
    NB_Error err = NE_OK;

    switch (m_state)
    {
        case OSASR_State_PrepareToRecording:
        case OSASR_State_Recording:
        case OSASR_State_Listening:
        case OSASR_State_WaitingLastBuffer:
        case OSASR_State_Recognition:
        {
            if (m_state == OSASR_State_Recognition)
            {
                LogOneShotAsrEvent(OSASR_QaLogEvent_RecognitionCanceled);
            }
            else
            {
                LogOneShotAsrEvent(OSASR_QaLogEvent_StoppedByTheUser);
            }

            SetState(OSASR_State_Idle);
            break;
        }

        case OSASR_State_Idle:
        {
            LogOneShotAsrEvent(OSASR_QaLogEvent_StoppedByTheUser);

            // The only thing that could be active while in idle state it's player
            StopPlayer();
            break;
        }

        default:
        {
            // Ignore this action in other states
            err = NE_UNEXPECTED;
            break;
        }
    }

    return err;
}

NB_Error
OneShotAsrService::StopRecording(void)
{
    NB_Error err = NE_OK;

    switch (m_state)
    {
        case OSASR_State_PrepareToRecording:
        {
            LogOneShotAsrEvent(OSASR_QaLogEvent_StoppedByTheUser);
            SetState(OSASR_State_Idle);
            break;
        }

        case OSASR_State_Recording:
        case OSASR_State_Listening:
        {
            LogOneShotAsrEvent(OSASR_QaLogEvent_StoppedByTheUser);

            err = SetState(OSASR_State_WaitingLastBuffer);
            if (err != NE_OK)
            {
                SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
            }
            break;
        }

        default:
        {
            // Ignore this action in other states
            err = NE_UNEXPECTED;
            break;
        }
    }

    return err;
}

NB_Error
OneShotAsrService::GetInformation(OneShotAsrInformationInterface** information)
{
    if (!information)
    {
        return NE_BADDATA;
    }

    if (!m_replyInformation)
    {
        return NE_NOENT;
    }

    // Client become owner of the reply information and will free it by self
    *information = static_cast<OneShotAsrInformationInterface*>(m_replyInformation);
    m_replyInformation = NULL;

    return NE_OK;
}

NB_Error
OneShotAsrService::GetLoudnessLevel(OneShotAsrLoudnessLevel* loudnessLevel)
{
    if (!loudnessLevel)
    {
        return NE_BADDATA;
    }

    PAL_Error err = PAL_Ok;
    int32 level = 0;

    PAL_LockLock(m_recorderAccessLock);

    if (m_recorder)
    {
        err = ABPAL_AudioRecorderGetParameter(m_recorder, ABPAL_AudioParameter_LoudnessLevel, &level, NULL);
    }

    PAL_LockUnlock(m_recorderAccessLock);

    if (err == PAL_Ok)
    {
        loudnessLevel->loudnessLevel = (uint32)level;
        return NE_OK;
    }

    return NE_UNEXPECTED;
}

NB_Error
OneShotAsrService::SetState(OneShotAsrState state, OneShotAsrAnnounce idleStateProceedAnnounce)
{
    if (state == m_state)
    {
        return NE_OK;
    }

#ifdef FEATURE_DEBUG_OSASR
    printf("OneShotAsrService state transaction: [%d] -> [%d]\n", (int)m_state, (int)state);
#endif //FEATURE_DEBUG_OSASR

    OneShotAsrState previousState = m_state;
    m_state = state;

    switch (m_state)
    {
        case OSASR_State_Idle:
        case OSASR_State_Destroy:
        {
            ResetToIdleState();
            if (state == OSASR_State_Idle)
            {
                PlayAnnounce(idleStateProceedAnnounce);
            }
            break;
        }

        case OSASR_State_PrepareToRecording:
        {
            StartPlayAndRecordAudioSession();

            if (PAL_Ok == PlayAnnounce(OSASR_Announce_RecordingTone))
            {
                PrepareToRecording();
            }
            else
            {
                // If player is failed just proceed to recording state
                NB_Error err = SetState(OSASR_State_Recording);
                if (err != NE_OK)
                {
                    return err;
                }
            }
            break;
        }

        case OSASR_State_Recording:
        {
            m_recordedBufferIndex = 0;
            m_audioBufferSendIndex = 0;

            PAL_Error err = StartRecorder();
            if (err != PAL_Ok)
            {
                // We can not continue if recorder is failed
                return NE_BADDATA;
            }

            m_listener->OneShotAsrHandleEvent(OSASR_Event_Recording, NE_OK);
            break;
        }

        case OSASR_State_Listening:
        {
            m_noSpeechBufferSequence = 0;

            LogOneShotAsrEvent(OSASR_QaLogEvent_ListeningStarted);

            m_listener->OneShotAsrHandleEvent(OSASR_Event_Listening, NE_OK);
            break;
        }

        case OSASR_State_WaitingLastBuffer:
        case OSASR_State_Recognition:
        {
            // Avoid twice execution follow code in case proceeding from OSASR_State_WaitingLastBuffer to OSASR_State_Recognition
            if (previousState != OSASR_State_WaitingLastBuffer)
            {
                LogOneShotAsrEvent(OSASR_QaLogEvent_ListeningStopped);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_Recognition, NE_OK);

                AtomicStopRecorder();
                PlayAnnounce(OSASR_Announce_RecognitionTone);
                StopPlayAndRecordAudioSession();
            }
            break;
        }

        default:
            break;
    }

    return NE_OK;
}

void
OneShotAsrService::ResetToIdleState(void)
{
    StopPlayer();
    AtomicStopRecorder();
    StopPlayAndRecordAudioSession();

    m_speechStreamHandler->CancelRequest();

    m_recordedBufferIndex  = 0;
    m_noSpeechBufferSequence = 0;
    m_audioBufferSendIndex = 0;

    DestroySendAudioBuffer();

#ifdef FEATURE_DEBUG_OSASR
    DestroyStoreToFileBuffer();
#endif //FEATURE_DEBUG_OSASR
}

PAL_Error
OneShotAsrService::StartPlayAndRecordAudioSession(void)
{
    PAL_Error palErr = PAL_Ok;

    if (!m_audioSession)
    {
        palErr = ABPAL_AudioSessionCreate(m_pal, ABPAL_AudioSession_PlayAndRecord, &m_audioSession);
    }

    return palErr;
}

void
OneShotAsrService::StopPlayAndRecordAudioSession(void)
{
    if (m_audioSession)
    {
        ABPAL_AudioSessionDestroy(m_audioSession);
        m_audioSession = NULL;
    }
}

PAL_Error
OneShotAsrService::PlayAnnounce(OneShotAsrAnnounce announce)
{
    // Destroy player if it was already created to avoid memory leak
    DestroyPlayer();

    // Is param valid
    if (announce >= OSASR_Announce_End)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error err = ABPAL_AudioPlayerCreate(m_pal, ABPAL_AudioFormat_UNK, &m_player);
    if (err == PAL_Ok)
    {
        // Ignore error code, since it can be unsupported on some platforms and default session will be used in this case
        ABPAL_AudioPlayerSetParameter(m_player, ABPAL_AudioParameter_AudioSession, ABPAL_AudioSession_PlayAndRecord, 0);

        // Request client to provide announce audio data if it was not provided yet
        if (!m_announceBuffer[announce])
        {
            byte* data = NULL;
            uint32 dataSize = 0;

            if (m_listener->OneShotAsrGetAnnounceData(announce, &data, &dataSize) == NE_OK)
            {
                if (data && dataSize)
                {
                    // Store announce data for further usage
                    m_announceBuffer[announce] = new AudioBuffer(data, dataSize, (int32)announce);
                }
            }
        }

        AudioBuffer* announceBuffer = m_announceBuffer[announce];

        // Play announce if it's available
        if (announceBuffer)
        {
            err = ABPAL_AudioPlayerPlay(m_player, announceBuffer->GetData() , announceBuffer->GetDataSize(), FALSE, AudioPlayerCallback, this);
        }
        else
        {
            err = PAL_ErrNoData;
        }
    }

    if (err != PAL_Ok)
    {
        DestroyPlayer();
    }

    return err;
}

void
OneShotAsrService::StopPlayer(void)
{
    if (m_player)
    {
        ABPAL_AudioPlayerCancel(m_player);
    }
}

void
OneShotAsrService::DestroyPlayer(void)
{
    if (m_player)
    {
        ABPAL_AudioPlayerDestroy(m_player);
        m_player = NULL;
    }
}

void
OneShotAsrService::AudioPlayerCallback(void* userData, ABPAL_AudioState state)
{
    if (userData)
    {
        OneShotAsrService* self = static_cast<OneShotAsrService*>(userData);
        self->HandleAudioPlayerCallback(state);
    }
}

void
OneShotAsrService::HandleAudioPlayerCallback(ABPAL_AudioState state)
{
    switch (state)
    {
        case ABPAL_AudioState_Ended:
        case ABPAL_AudioState_Error:
        {
            if (m_state == OSASR_State_PrepareToRecording)
            {
                NB_Error err = SetState(OSASR_State_Recording);
                if (err != NE_OK)
                {
                    // If recorder is failed reset to idle state and notify client with error occured
                    SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
                    m_listener->OneShotAsrHandleEvent(OSASR_Event_GeneralError, static_cast<uint32>(err));
                }
            }
            break;
        }

        default:
            break;
    }
}

PAL_Error
OneShotAsrService::PrepareToRecording(void)
{
    PAL_LockLock(m_recorderAccessLock);

    DestroyRecorder();

    PAL_Error err = PAL_Ok;

    if (!m_recorder)
    {
        err = ABPAL_AudioRecorderCreate(m_pal, ABPAL_AudioFormat_UNK, &m_recorder);
        if (err == PAL_Ok)
        {
            // Setup recorder. Ignore return error codes, since some of the parameters can be unsupported on some platforms and default internal recorder values will be used in this case.
            ABPAL_AudioRecorderSetParameter(m_recorder, ABPAL_AudioParameter_AudioSession, ABPAL_AudioSession_PlayAndRecord, 0);
            ABPAL_AudioRecorderSetParameter(m_recorder, ABPAL_AudioParameter_StreamRecordingMode, (int32)m_config.recorderBufferDuration, 0);
            ABPAL_AudioRecorderSetParameter(m_recorder, ABPAL_AudioParameter_SampleRate, (int32)m_config.recorderSampleRate, 0);
            ABPAL_AudioRecorderSetParameter(m_recorder, ABPAL_AudioParameter_BitsPerSample, (int32)m_config.recorderBitsPerSample, 0);

            // Prepare recorder to start recording as soon as possible on ABPAL_AudioRecorderRecord call. Ignore return error code since it can be unsupported.
            ABPAL_AudioRecorderPrepareToRecord(m_recorder);

            int32 bitsPerSample = 0;
            int32 sampleRate = 0;

            // Retrieve bits per sample and sample rate from recorder, since it can be differ wih setuped above values.
            ABPAL_AudioRecorderGetParameter(m_recorder, ABPAL_AudioParameter_BitsPerSample, &bitsPerSample, NULL);
            ABPAL_AudioRecorderGetParameter(m_recorder, ABPAL_AudioParameter_SampleRate, &sampleRate, NULL);

            // Create appropriate Speech Processor if it didn't yet
            if (m_speechProcessor == NULL)
            {
                // Convert public SpeechDetectionAlgorithm type to private SpeechProcessorAlgorithm type
                SpeechProcessorAlgorithm spAlgorithm = SPA_Unknown;
                switch (m_config.speechDetectionAlgorithm)
                {
                    case SDA_Amplitude:
                        spAlgorithm = SPA_Amplitude;
                        break;

                    case SDA_Spectrum:
                        spAlgorithm = SPA_Spectrum;
                        break;

                    default:
                        break;
                }

                // Create speech processor
                // TODO: Bit-order need to be retrieved from pal recorder. Let's suppose that recorder uses little endian.
                if (CreateSpeechProcessor(m_context, spAlgorithm, bitsPerSample, sampleRate, FALSE /* Little Endian */, &m_speechProcessor) != NE_OK)
                {
                    // We can not continue without speech processor
                    err = PAL_ErrUnsupported;
                }
            }
            else
            {
                // Reset speech processor history if it was used recently
                m_speechProcessor->ResetHistory();
            }

            // Create appropriate encoder. Only speex encoder supported for now.
            if (err == PAL_Ok)
            {
                if (m_config.audioEncoderFormat == ABPAL_AudioFormat_SPX)
                {
                    if (m_speexEncoder == NULL)
                    {
                        err = CreateSpeexEncoder(m_pal, bitsPerSample, sampleRate, FALSE /* Little Endian */, &m_speexEncoder);
                    }
                    else
                    {
                        // Reset speex encoder history if it was used recently
                        m_speexEncoder->ResetHistory();
                    }
                }
            }

            // Setup audio format to client parameters
            if (err == PAL_Ok)
            {
                if (m_clientParameters->SetAudioFormat(m_config.audioEncoderFormat, bitsPerSample, sampleRate) != NE_OK)
                {
                    err = PAL_ErrUnsupported;
                }
            }
        }
    }

    if (err != PAL_Ok)
    {
        DestroyRecorder();
    }

    PAL_LockUnlock(m_recorderAccessLock);

    return err;
}

PAL_Error
OneShotAsrService::StartRecorder(void)
{
    PAL_Error err = PAL_Ok;

    PAL_LockLock(m_recorderAccessLock);

    if (!m_recorder)
    {
        err = PrepareToRecording();
    }

    if (m_recorder && (err == PAL_Ok))
    {
        err = ABPAL_AudioRecorderRecord(m_recorder, NULL, 0, AudioRecorderCallback, this);
    }

    if (err == PAL_Ok)
    {
        LogOneShotAsrEvent(OSASR_QaLogEvent_RecordingStarted);
    }
    else
    {
        DestroyRecorder();
    }

    PAL_LockUnlock(m_recorderAccessLock);

    return err;
}

void
OneShotAsrService::AtomicStopRecorder(void)
{
    PAL_LockLock(m_recorderAccessLock);

    if (m_recorder)
    {
        bool recorderIsStopped = (ABPAL_AudioRecorderGetState(m_recorder) == ABPAL_AudioState_Recording);

        ABPAL_AudioRecorderStop(m_recorder);

        if (recorderIsStopped)
        {
            LogOneShotAsrEvent(OSASR_QaLogEvent_RecordingStopped);
        }
    }

    PAL_LockUnlock(m_recorderAccessLock);
}

void
OneShotAsrService::DestroyRecorder(void)
{
    if (m_recorder)
    {
        ABPAL_AudioRecorderDestroy(m_recorder);
        m_recorder = NULL;
    }
}

void
OneShotAsrService::AudioRecorderCallback(void* userData, ABPAL_AudioState state)
{
    if (userData)
    {
        OneShotAsrService* self = static_cast<OneShotAsrService*>(userData);
        self->HandleAudioRecorderCallback(state);
    }
}

void
OneShotAsrService::HandleAudioRecorderCallback(ABPAL_AudioState state)
{
    switch (state)
    {
        case ABPAL_AudioState_StreamRecordBufferAvailable:
        case ABPAL_AudioState_Ended:
        case ABPAL_AudioState_Error:
        {
            if (m_state == OSASR_State_Recording || m_state == OSASR_State_Listening || m_state == OSASR_State_WaitingLastBuffer)
            {
                byte* buffer = NULL;
                uint32 bufferSize = 0;

                PAL_LockLock(m_recorderAccessLock);
                if (m_recorder)
                {
                    // Get recorded buffer. One Shot Asr Service become owner of the data and will free it by self.
                    ABPAL_AudioRecorderGetRecordedData(m_recorder, &buffer, &bufferSize);
                }
                PAL_LockUnlock(m_recorderAccessLock);

#ifdef FEATURE_DEBUG_OSASR
                printf("Record buffer available %d: size(%d)\n", (int)m_recordedBufferIndex, bufferSize);
#endif //FEATURE_DEBUG_OSASR

                AudioBuffer recordedBuffer(buffer, bufferSize, ++m_recordedBufferIndex);
                bool bufferIsLast = (state == ABPAL_AudioState_Ended || state == ABPAL_AudioState_Error);

                // Run audio buffer processing pipeline
                AudioBufferProcessingPipeline(recordedBuffer, bufferIsLast);
            }
            break;
        }

        default:
            break;
    }
}

void
OneShotAsrService::DestroySendAudioBuffer(void)
{
    if (m_audioBufferToSend)
    {
        delete m_audioBufferToSend;
        m_audioBufferToSend = NULL;
    }
}

int32
OneShotAsrService::GetBeginOfSpeechBufferIndex(void)
{
    return (int32)osasr_ceil((double)m_config.beginOfSpeechTimeout / (double)m_config.recorderBufferDuration);
}

int32
OneShotAsrService::GetMaxRecordBufferIndex(void)
{
    return (int32)osasr_ceil((double)m_config.maxRecordTime / (double)m_config.recorderBufferDuration);
}

int32
OneShotAsrService::GetNoSpeechBufferIndex(void)
{
    return (int32)osasr_ceil((double)m_config.noSpeechTimeout / (double)m_config.recorderBufferDuration);
}

int32
OneShotAsrService::GetUploadBufferIndex(void)
{
    return (int32)osasr_ceil((double)m_config.uploadInterval / (double)m_config.recorderBufferDuration);
}

void
OneShotAsrService::AudioBufferProcessingPipeline(AudioBuffer& audioBuffer, bool lastBuffer)
{
    // Process buffers only in recording, listening and waiting last buffer states
    if (m_state != OSASR_State_Recording && m_state != OSASR_State_Listening && m_state != OSASR_State_WaitingLastBuffer)
    {
        return;
    }

    // Declare end of speech and proceed to recognition state in last buffer or max allowed recording time is reached cases
    if (lastBuffer || (audioBuffer.GetIndex() >= GetMaxRecordBufferIndex()))
    {
        NB_Error err = NE_OK;

        err = SetState(OSASR_State_Recognition);
        if (err != NE_OK)
        {
            SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
            m_listener->OneShotAsrHandleEvent(OSASR_Event_GeneralError, static_cast<uint32>(err));
            return;
        }
    }

    // Speech detection is needed only in recording and listening states
    if (m_state == OSASR_State_Recording || m_state == OSASR_State_Listening)
    {
        AudioBufferProcessingPipelineSpeechDetection(audioBuffer);
    }

    // Run audio format encoder and try to send collected buffers to server only in recording, listening, wating last buffer and recognition states
    if (m_state == OSASR_State_Recording || m_state == OSASR_State_Listening || m_state == OSASR_State_WaitingLastBuffer || m_state == OSASR_State_Recognition)
    {
        AudioBufferProcessingPipelineEncode(audioBuffer);
        AudioBufferProcessingPipelineTryToSend();
    }
}

NB_Error
OneShotAsrService::AudioBufferProcessingPipelineSpeechDetection(AudioBuffer& audioBuffer)
{
    bool bufferIsAudible = FALSE;
    NB_Error err = NE_OK;

    // Run speech detection
    {
        SpeechProcessorResult speechDetectionResult = m_speechProcessor->ProcessBuffer(audioBuffer.GetData(), audioBuffer.GetDataSize());
        bufferIsAudible = (speechDetectionResult == SPR_SpeechDetected);
    }

    if (bufferIsAudible)
    {
        // We have detected speech in passed buffer
        if (m_state == OSASR_State_Recording) // Begin of speech is not detected yet
        {
            // Begin of speech detected. Proceed to listening state.
            err = SetState(OSASR_State_Listening);
            if (err != NE_OK)
            {
                SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_GeneralError, static_cast<uint32>(err));
                return err;
            }
        }
        else if (m_state == OSASR_State_Listening) // Begin of speech was already detected
        {
            // Reset no speech buffer sequence
            m_noSpeechBufferSequence = 0;
        }
    }
    else
    {
        // There is no speech in passed buffer
        if (m_state == OSASR_State_Recording) // Begin of speech is not detected yet
        {
            // If begin of speech still was not detected let's check begin of speech timeout
            if (audioBuffer.GetIndex() >= GetBeginOfSpeechBufferIndex())
            {
                LogOneShotAsrEvent(OSASR_QaLogEvent_NoSpeechDetected);

                // Begin of speech timeout faired. Proceed to idle state with recognition error notification.
                SetState(OSASR_State_Idle, OSASR_Announce_BeginOfSpeechTimeoutTone);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_BeginOfSpeechTimeout, 0);
            }
        }
        else if (m_state == OSASR_State_Listening) // Begin of speech was already detected
        {
            // Increment no speech buffer sequence
            m_noSpeechBufferSequence++;

            // Check condition on max allowed no speech buffer sequence
            if (m_noSpeechBufferSequence >= GetNoSpeechBufferIndex())
            {
                LogOneShotAsrEvent(OSASR_QaLogEvent_NoSpeechDetected);

                // No speech timer faired. Declare end of speech and proceed to recognition state.
                err = SetState(OSASR_State_Recognition);
                if (err != NE_OK)
                {
                    SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
                    m_listener->OneShotAsrHandleEvent(OSASR_Event_GeneralError, static_cast<uint32>(err));
                    return err;
                }
            }
        }
    }

    return err;
}

NB_Error
OneShotAsrService::AudioBufferProcessingPipelineEncode(AudioBuffer& audioBuffer)
{
    if (m_config.audioEncoderFormat == ABPAL_AudioFormat_SPX) // Speex
    {
        // Encode passed buffer
        byte* encodedData = NULL;
        uint32 encodedDataSize = 0;
        PAL_Error palErr = PAL_Ok;

        // Encode to speex
        palErr = m_speexEncoder->EncodeBuffer(audioBuffer.GetData(),
                                              audioBuffer.GetDataSize(),
                                              (m_state == OSASR_State_Recognition),
                                              &encodedData,
                                              &encodedDataSize);

        // If encoder is failed let's ignore passed buffer
        if (palErr == PAL_Ok)
        {
            AudioBuffer* encodedBuffer = new AudioBuffer(encodedData, encodedDataSize);

            if (m_audioBufferToSend)
            {
                if (m_audioBufferToSend->AppendBuffer(encodedBuffer))
                {
                    m_audioBufferToSend->SetIndex(m_audioBufferToSend->GetIndex() + 1 /* Collected buffers count */);
                }

                delete encodedBuffer;
                encodedBuffer = NULL;
            }
            else
            {
                m_audioBufferToSend = encodedBuffer;
                m_audioBufferToSend->SetIndex(1 /* Collected buffers count */);
                encodedBuffer = NULL;
            }
        }
    }
    else if (m_config.audioEncoderFormat == ABPAL_AudioFormat_WAV || m_config.audioEncoderFormat == ABPAL_AudioFormat_RAW) // wav or raw
    {
        // Append passed buffer
        if (m_audioBufferToSend)
        {
            if (m_audioBufferToSend->AppendBuffer(&audioBuffer))
            {
                m_audioBufferToSend->SetIndex(m_audioBufferToSend->GetIndex() + 1 /* Collected buffers count */);
            }
        }
        else
        {
            m_audioBufferToSend = new AudioBuffer(audioBuffer);
            m_audioBufferToSend->SetIndex(1);
        }
    }

    return NE_OK;
}

NB_Error
OneShotAsrService::AudioBufferProcessingPipelineTryToSend(void)
{
    bool needToSend = FALSE;
    bool lastBuffer = (m_state == OSASR_State_Recognition);
    NB_Error err = NE_OK;

    // Send buffer to server if in recognition state or one of the send condition is acquired
    if (lastBuffer)
    {
        needToSend = TRUE;
    }
    else if (m_audioBufferToSend)
    {
        if ((m_audioBufferToSend->GetIndex() >= GetUploadBufferIndex() || m_audioBufferToSend->GetDataSize() >= m_config.uploadDataSize))
        {
            needToSend = TRUE;
        }
    }

    if (needToSend)
    {
#ifdef FEATURE_DEBUG_OSASR
        {
            if (m_audioBufferToSend)
            {
                if (!m_storeToFileBuffer)
                {
                    m_storeToFileBuffer = new AudioBuffer(*m_audioBufferToSend);
                }
                else
                {
                    m_storeToFileBuffer->AppendBuffer(m_audioBufferToSend);
                }
            }

            if (lastBuffer && m_storeToFileBuffer)
            {
                StoreToFileAudioBuffer(m_storeToFileBuffer);
                DestroyStoreToFileBuffer();
            }
        }
#endif //FEATURE_DEBUG_OSASR

        byte* audioData = NULL;
        uint32 audioDataSize = 0;

        if (m_audioBufferToSend)
        {
            audioData = m_audioBufferToSend->GetData();
            audioDataSize = m_audioBufferToSend->GetDataSize();
        }

#ifdef FEATURE_DEBUG_OSASR
        printf("Try to send buffer(%d): size (%d) last:(%d)\n", (int)m_audioBufferSendIndex, audioDataSize, (int)lastBuffer);
#endif //FEATURE_DEBUG_OSASR

        // Copy client parameters
        OneShotAsrParameters parameters(*m_clientParameters);

        // Set audio buffer to parameters
        err = parameters.SetAudioData(audioData, audioDataSize, m_audioBufferSendIndex, lastBuffer);
        if (err == NE_OK)
        {
            // Start speech stream request
            err = m_speechStreamHandler->StartRequest(static_cast<SpeechStreamParametersInterface*>(&parameters));

            // Let's ignore audio buffer if start request is failed and try with another buffer next time
            if (err == NE_OK)
            {
#ifdef FEATURE_DEBUG_OSASR
                printf("Buffer(%d) was sent successfully\n", (int)m_audioBufferSendIndex);
#endif //FEATURE_DEBUG_OSASR

                // Increment audio buffer sequence ID
                m_audioBufferSendIndex++;
            }
        }

        if (err != NE_OK)
        {
            // Send request for last buffer is failed. We need reset to idle state with error.
            if (lastBuffer)
            {
                SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_GeneralError, static_cast<uint32>(err));
            }
        }

        // We don't need collected audio buffer any more. Let's free it to hold new one next time.
        DestroySendAudioBuffer();
    }

    return err;
}

void
OneShotAsrService::SpeechStreamHandleEvent(SpeechStreamEvent event, uint32 param)
{
    // Handle speech stream events only in recording/listening/recognition and waiting last buffer states
    if (m_state == OSASR_State_Recording || m_state == OSASR_State_Listening || m_state == OSASR_State_WaitingLastBuffer || m_state == OSASR_State_Recognition)
    {
        switch (event)
        {
            case SpeechStreamEvent_ServerError:
            {
                ServerCompletionCode completionCode = static_cast<ServerCompletionCode>(param);
                OneShotAsrAnnounce announce = OSASR_Announce_End;

                switch (completionCode)
                {
                    case ServerCompletionCode_NoSpeechWasDetected:
                    {
                        announce = OSASR_Announce_NoSpeechWasDetectedTone;
                        break;
                    }

                    case ServerCompletionCode_NoValidResults:
                    {
                        announce = OSASR_Announce_NoValidResultsTone;
                        break;
                    }

                    case ServerCompletionCode_ParserReturnNoResults:
                    {
                        announce = OSASR_Announce_ParserReturnNoResultsTone;
                        break;
                    }

                    case ServerCompletionCode_ServerError:
                    case ServerCompletionCode_ServerTimeout:
                    case ServerCompletionCode_AudioIsTooLong:
                    case ServerCompletionCode_InvalidAudioFormat:
                    {
                        announce = OSASR_Announce_OtherServerErrorTone;
                        break;
                    }

                    default:
                	{
                        announce = OSASR_Announce_OtherServerErrorTone;
                        break;
                    }
                }

                // Store reply information
                DestroyReplyInformation();
                m_speechStreamHandler->GetReplyInformation(&m_replyInformation);

                // Reset to idle state with announce
                SetState(OSASR_State_Idle, announce);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_ServerError, static_cast<uint32>(completionCode));

                // Make reply information available only for duration of the callback
                DestroyReplyInformation();
                break;
            }

            case SpeechStreamEvent_GeneralError:
            {
                // Reset to idle state with announce
                SetState(OSASR_State_Idle, OSASR_Announce_GeneralErrorTone);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_GeneralError, param);
                break;
            }

            case SpeechStreamEvent_ResultsAvailable:
            {
                // Store reply information
                DestroyReplyInformation();
                m_speechStreamHandler->GetReplyInformation(&m_replyInformation);

                // Reset to idle state with announce
                SetState(OSASR_State_Idle, OSASR_Announce_ResultsAvailableTone);
                m_listener->OneShotAsrHandleEvent(OSASR_Event_ResultsAvailable, NE_OK);

                // Make reply information available only for duration of the callback
                DestroyReplyInformation();
                break;
            }

            default:
                break;
        }
    }
}

void
OneShotAsrService::DestroyClientParameters(void)
{
    if (m_clientParameters)
    {
        m_clientParameters->Release();
        m_clientParameters = NULL;
    }
}

void
OneShotAsrService::DestroyReplyInformation(void)
{
    if (m_replyInformation)
    {
        m_replyInformation->Release();
        m_replyInformation = NULL;
    }
}

void
OneShotAsrService::LogOneShotAsrEvent(OneShotAsrQaLogEvent event)
{
    if (m_clientParameters && (NB_ContextGetQaLog(m_context) != NULL))
    {
        string sessionID;
        m_clientParameters->GetSessionID(sessionID);

        AB_QaLogOneShotAsrEvent(m_context, sessionID.c_str(), (uint32)event);
    }
}


#ifdef FEATURE_DEBUG_OSASR
PAL_Error
OneShotAsrService::StoreToFileAudioBuffer(AudioBuffer* audioBuffer)
{
    if (!m_storeToFileBuffer)
    {
        return PAL_ErrNoData;
    }

    PAL_ClockDateTime dateTime;
    PAL_ClockGetDateTime(&dateTime);
    char fileName[80] = {0};
    string dirName = "OneShotAsrAudio";

    PAL_Error err = PAL_FileExists(m_pal, dirName.c_str());

    if (err == PAL_ErrFileNotExist)
    {
        err = PAL_FileCreateDirectory(m_pal, dirName.c_str());
    }
    else
    {
        err = PAL_Ok;
    }

    if (err == PAL_Ok)
    {
        string fileExtension = "dat";

        if (m_config.audioEncoderFormat == ABPAL_AudioFormat_WAV || m_config.audioEncoderFormat == ABPAL_AudioFormat_RAW)
        {
            fileExtension = "wav";
        }
        else if (m_config.audioEncoderFormat == ABPAL_AudioFormat_SPX)
        {
            fileExtension = "spx";
        }

        sprintf(fileName, "%s/%04d%02d%02d_%02d%02d%02d.%s", (char*)dirName.c_str(), dateTime.year, dateTime.month, dateTime.day,
                dateTime.hour, dateTime.minute, dateTime.second, fileExtension.c_str());

        PAL_File* file = NULL;
        err = PAL_FileOpen(m_pal, fileName, PFM_Create, &file);
        if (err == PAL_Ok)
        {
            uint32 written = 0;

            if (m_config.audioEncoderFormat == ABPAL_AudioFormat_WAV)
            {
                int32 bitsPerSample = 0;
                int32 sampleRate = 0;

                PAL_LockLock(m_recorderAccessLock);
                ABPAL_AudioRecorderGetParameter(m_recorder, ABPAL_AudioParameter_BitsPerSample, &bitsPerSample, NULL);
                ABPAL_AudioRecorderGetParameter(m_recorder, ABPAL_AudioParameter_SampleRate, &sampleRate, NULL);
                PAL_LockUnlock(m_recorderAccessLock);

                written = WriteWavHeader(file, sampleRate, 1 /* Number of chunnels */, bitsPerSample);
            }

            err = PAL_FileWrite(file, m_storeToFileBuffer->GetData(), m_storeToFileBuffer->GetDataSize(), &written);
            PAL_FileClose(file);
            file = NULL;

            if (written)
            {
                err = PAL_Ok;
                printf("Audio file stored at: (%s)\n", fileName);
            }
        }
    }

    return err;
}

int32
OneShotAsrService::WriteWavHeader(PAL_File* palFile, int32 rate, int32 channels, int32 format)
{
    int32 itmp = 0;
    int16 stmp = 0;
    uint32 totalWritten = 0;
    uint32 written = 0;

    const char* riff = "RIFF";
    PAL_FileWrite(palFile, (uint8*)riff, nsl_strlen(riff), &written);
    totalWritten += written;

    itmp = 0x7fffffff;
    PAL_FileWrite(palFile, (uint8*)&itmp, sizeof(itmp), &written);
    totalWritten += written;

    const char* wave = "WAVEfmt ";
    PAL_FileWrite(palFile, (uint8*)wave, nsl_strlen(wave), &written);
    totalWritten += written;

    itmp = 16;
    PAL_FileWrite(palFile, (uint8*)&itmp, sizeof(itmp), &written);
    totalWritten += written;

    stmp = 1;
    PAL_FileWrite(palFile, (uint8*)&stmp, sizeof(stmp), &written);
    totalWritten += written;

    stmp = channels;
    PAL_FileWrite(palFile, (uint8*)&stmp, sizeof(stmp), &written);
    totalWritten += written;

    itmp = rate;
    PAL_FileWrite(palFile, (uint8*)&itmp, sizeof(itmp), &written);
    totalWritten += written;

    itmp = 2 * rate * channels;
    PAL_FileWrite(palFile, (uint8*)&itmp, sizeof(itmp), &written);
    totalWritten += written;

    stmp = 2 * channels;
    PAL_FileWrite(palFile, (uint8*)&stmp, sizeof(stmp), &written);
    totalWritten += written;

    stmp = format;
    PAL_FileWrite(palFile, (uint8*)&stmp, sizeof(stmp), &written);
    totalWritten += written;

    const char* data = "data";
    PAL_FileWrite(palFile, (uint8*)data, nsl_strlen(data), &written);
    totalWritten += written;

    itmp = 0x7fffffff;
    PAL_FileWrite(palFile, (uint8*)&itmp, sizeof(itmp), &written);
    totalWritten += written;

    return totalWritten;
}

void
OneShotAsrService::DestroyStoreToFileBuffer(void)
{
    if (m_storeToFileBuffer)
    {
        delete m_storeToFileBuffer;
        m_storeToFileBuffer = NULL;
    }
}
#endif //FEATURE_DEBUG_OSASR

NB_Error
CreateOneShotAsrService(NB_Context* context,
                        OneShotAsrConfig* config,
                        OneShotAsrListenerInterface* listener,
                        OneShotAsrServiceInterface** asrService
                        )
{
    if (!context || !config || !listener || !asrService)
    {
        return NE_BADDATA;
    }

    NB_Error err = NE_OK;

    OneShotAsrService* asr = new OneShotAsrService;
    err = asr->Init(context, config, listener);

    if (err == NE_OK)
    {
        *asrService = asr;
        asr = NULL;
    }
    else
    {
        delete asr;
        asr = NULL;
    }

    return err;
}

} //aboneshotasr

/*! @} */
