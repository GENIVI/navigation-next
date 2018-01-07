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

    @file     nimaudioplayer.cpp
    @date     02/27/2009
    @defgroup PAL Audio API Player

    @brief    Audio Player for Windows Mobile

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @( */

#include "nimaudioplayer.h"
#include "palstdlib.h"
#include "soundfile.h"

#define DB_LEVEL_OFF                -10000
#define DB_LEVEL_LOW                -6667
#define DB_LEVEL_MEDIUM             -3333
#define DB_LEVEL_HIGH               -1666
#define DB_LEVEL_VERYHIGH           0

#define WAVEOUT_LEVEL_OFF           0
#define WAVEOUT_LEVEL_LOW           0x66666666
#define WAVEOUT_LEVEL_MEDIUM        0x99999999
#define WAVEOUT_LEVEL_HIGH          0xCCCCCCCC
#define WAVEOUT_LEVEL_VERYHIGH      0xFFFFFFFF


//////////////////////////////////////////////////////////////////////////
// constructor/destructor
//////////////////////////////////////////////////////////////////////////

NIMAudioPlayer::NIMAudioPlayer(PAL_Instance *pPal, ABPAL_AudioFormat format) :
    NIMAudio(pPal, format),
    m_pGraph(NULL),
    m_pMediaControl(NULL),
    m_pBasicAudio(NULL),
    m_bVolumeReset(FALSE),
    m_oldVolume(0),
    m_oldSoundType(0),
    m_volumeLevel(ABPAL_AudioVolumeLevel_Medium),
    m_pEvent(NULL),
    m_Buffer(NULL),
    m_BufferSize(0),
    m_NotifyThread(0),
    m_NotifyThreadStopEvent(0),
    m_Callback(NULL),
    m_CallbackUserData(0)
{
    m_NotifyThreadStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    SetState(ABPAL_AudioState_Init);
}

NIMAudioPlayer::~NIMAudioPlayer()
{
    Stop();

    if (m_Buffer)
    {
        nsl_free(m_Buffer);
        m_Buffer = 0;
        m_BufferSize = 0;
    }

    if (m_bVolumeReset)
    {
        ResetDeviceVolume();
    }

    TearDownInterfaces();
    DeleteTemporaryFile();
    CloseHandle(m_NotifyThreadStopEvent);
}

//////////////////////////////////////////////////////////////////////////
// public functions
//////////////////////////////////////////////////////////////////////////

void NIMAudioPlayer::SetVolumeLevel(ABPAL_AudioVolumeLevel level)
{
    m_volumeLevel = level;
}

ABPAL_AudioVolumeLevel NIMAudioPlayer::GetVolumeLevel(void)
{
    return m_volumeLevel;
}

PAL_Error NIMAudioPlayer::Play(byte *buffer, int bufferSize, nb_boolean takeOwnership, ABPAL_AudioPlayerCallback* callback, void* callbackData)
{
    const DWORD CANCEL_PLAYBACK_TIMEOUT = 1000;
    
    PAL_Error err = PAL_ErrAudioGeneral;
    HRESULT hr = E_FAIL;
    DWORD notifyThreadId = 0;

    // check params
    if (!buffer || bufferSize == 0)
    {
        return PAL_ErrBadParam;
    }

    // check current state
    if (m_State == ABPAL_AudioState_Playing)
    {
        Stop();
        (void)WaitForSingleObject(m_NotifyThread, CANCEL_PLAYBACK_TIMEOUT);
    }
    
    if (m_State != ABPAL_AudioState_Init && m_State != ABPAL_AudioState_Ended)
    {
        return PAL_ErrAudioBusy;
    }

    m_Callback = callback;
    m_CallbackUserData = callbackData;

    // free previous buffer
    if (m_Buffer)
    {
        nsl_free(m_Buffer);
    }        
    m_Buffer = 0;
    m_BufferSize = 0;
    
    // take ownership or make copy
    if (takeOwnership)
    {
        m_Buffer = buffer; 
        m_BufferSize = bufferSize;  
    }
    else
    {
        m_Buffer = (byte*)nsl_malloc(bufferSize);
        if (!m_Buffer)
        {
            return PAL_ErrNoMem;
        }
        nsl_memcpy(m_Buffer, buffer, bufferSize);
        m_BufferSize = bufferSize;  
    }
    
    // render buffer for play
    hr = Render(buffer, bufferSize);

    if (SUCCEEDED(hr))
    {
        hr = SetDeviceVolumeLevel() ? S_OK : E_FAIL;
    }

    // start up thread for status notify
    if (SUCCEEDED(hr))
    {
        (void)ResetEvent(m_NotifyThreadStopEvent);
        m_NotifyThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NIMAudioPlayer::NotifyThreadProc, this, 0, &notifyThreadId);
        hr = m_NotifyThread ? S_OK : E_FAIL;
    }

    // play    
    if (SUCCEEDED(hr))
    {
        SetState(ABPAL_AudioState_Playing);
        hr = m_pMediaControl->Run();
    }

    if (SUCCEEDED(hr))
    {
        err = PAL_Ok;
    }
    return err;
}

nb_boolean
NIMAudioPlayer::Stop(void)
{
    HRESULT hr = S_OK;
    
    if (m_pMediaControl)
    {
        hr = m_pMediaControl->Stop();
    }

    SetEvent(m_NotifyThreadStopEvent);
    
    return SUCCEEDED(hr);
}


//////////////////////////////////////////////////////////////////////////
// protected functions
//////////////////////////////////////////////////////////////////////////

void
NIMAudioPlayer::SetState(ABPAL_AudioState state)
{
    m_State = state;
    if (m_Callback)
    {
        (m_Callback)(m_CallbackUserData, m_State);
    }
}

//////////////////////////////////////////////////////////////////////////
// private functions
//////////////////////////////////////////////////////////////////////////

void
NIMAudioPlayer::DeleteTemporaryFile(void)
{
    (void)DeleteFile(m_TempFileName);
    m_TempFileName[0] = 0;
}

HRESULT
NIMAudioPlayer::SetupInterfaces(void)
{
    HRESULT hr = S_OK;
    HANDLE hEvent = NULL;

    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (void **)&m_pGraph);

    if (SUCCEEDED(hr))
    {
        hr = m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pEvent);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pGraph->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pGraph->QueryInterface(IID_IBasicAudio, (void**)&m_pBasicAudio);
    }

    if (m_pEvent != NULL)
    {
        hr = m_pEvent->GetEventHandle((OAEVENT*)&hEvent);

        if (m_MediaEventHandle != NULL)
        {
            CloseHandle(m_MediaEventHandle);
        }

        m_MediaEventHandle = NULL;

        if (!DuplicateHandle(GetCurrentProcess(), hEvent, GetCurrentProcess(), &m_MediaEventHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

void
NIMAudioPlayer::TearDownInterfaces(void)
{
    if (m_pEvent != NULL)
    {
        m_pEvent->Release();
        m_pEvent = NULL;
    }

    if (m_pMediaControl != NULL)
    {
        m_pMediaControl->Release();
        m_pMediaControl = NULL;
    }

    if (m_pBasicAudio != NULL)
    {
        m_pBasicAudio->Release();
        m_pBasicAudio = NULL;
    }

    if (m_pGraph != NULL)
    {
        m_pGraph->Release();
        m_pGraph = NULL;
    }

    if (m_MediaEventHandle != NULL)
    {
        CloseHandle(m_MediaEventHandle);
        m_MediaEventHandle = NULL;
    }
}

void
NIMAudioPlayer::GetCurrentDeviceVolume(void)
{
    SNDFILEINFO sndInfo = {0};

    // get current device volume & state    
    SndGetSound (SND_EVENT_ALL, &sndInfo);

    switch (sndInfo.sstType)
    {
    case SND_SOUNDTYPE_FILE:
    case SND_SOUNDTYPE_ON:		
        m_oldSoundType = SND_SOUNDTYPE_ON;
        break;
    case SND_SOUNDTYPE_VIBRATE:	
        m_oldSoundType = SND_SOUNDTYPE_VIBRATE;
        break;
    case SND_SOUNDTYPE_NONE:
        m_oldSoundType = SND_SOUNDTYPE_NONE;
        break;
    }
    waveOutGetVolume (HWAVEOUT(WAVE_MAPPER), &m_oldVolume);
}

void
NIMAudioPlayer::ResetDeviceVolume(void)
{
    SNDFILEINFO sndInfo = {0};
    sndInfo.sstType = (SND_SOUNDTYPE)m_oldSoundType;

    // reset device volume & state to previous settings
    SndSetSound(SND_EVENT_ALL, &sndInfo, false);
    waveOutSetVolume((HWAVEOUT)WAVE_MAPPER, m_oldVolume);
    
    m_bVolumeReset = FALSE;
}

nb_boolean
NIMAudioPlayer::SetDeviceVolumeLevel(void)
{
    long dbLevel = 0;
    int waveoutLevel = 0;
    SNDFILEINFO sndInfo = {0};	

    // translate ABPAL_AudioVolumeLevel
    switch (m_volumeLevel)
    {
    case ABPAL_AudioVolumeLevel_VeryHigh:
        dbLevel = DB_LEVEL_VERYHIGH;
        waveoutLevel = WAVEOUT_LEVEL_VERYHIGH;
        break;
    case ABPAL_AudioVolumeLevel_High:
        dbLevel = DB_LEVEL_HIGH;
        waveoutLevel = WAVEOUT_LEVEL_HIGH;
        break;
    case ABPAL_AudioVolumeLevel_Medium:
        dbLevel = DB_LEVEL_MEDIUM;
        waveoutLevel = WAVEOUT_LEVEL_MEDIUM;
        break;
    case ABPAL_AudioVolumeLevel_Low:
        dbLevel = DB_LEVEL_LOW;
        waveoutLevel = WAVEOUT_LEVEL_LOW;
        break;
    case ABPAL_AudioVolumeLevel_Off:
    default:
        dbLevel = DB_LEVEL_OFF;
        waveoutLevel = WAVEOUT_LEVEL_OFF;
        break;
    }

    // get current device volume settings & state
    GetCurrentDeviceVolume();
    m_bVolumeReset = TRUE;
    
    // turn on sound if off/vibrate
    if (((SND_SOUNDTYPE)m_oldSoundType == SND_SOUNDTYPE_VIBRATE ) ||
        ((SND_SOUNDTYPE)m_oldSoundType == SND_SOUNDTYPE_NONE))
    {
        sndInfo.sstType = SND_SOUNDTYPE_ON;
        SndSetSound(SND_EVENT_ALL, &sndInfo, false);
    }

    if (m_pBasicAudio)
    {
        (void)m_pBasicAudio->put_Volume(dbLevel);
    }

    (void)waveOutSetVolume((HWAVEOUT)WAVE_MAPPER, waveoutLevel);
    
    return TRUE;   
}

HRESULT
NIMAudioPlayer::Render(const byte *buffer, uint32 bufferSize)
{
    HRESULT hr = S_OK;

    TCHAR programPath[MAX_PATH + 1] = { 0 };

    HANDLE hfile = NULL;
    DWORD nWritten = 0;
    
    TearDownInterfaces();
    DeleteTemporaryFile();

    hr = SetupInterfaces();
    
    if (SUCCEEDED(hr))
    {
        if (!GetTempPath(MAX_PATH, programPath))
        {
            _tcscpy(programPath, _T(".\\"));
        }
        _stprintf(m_TempFileName, _T("%s%s.%s"), programPath, _T("NIM"), GetAudioFormatFileExtension(m_Format));

        hfile = CreateFile(m_TempFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hfile ==  INVALID_HANDLE_VALUE)
        {
            hr =  E_FAIL;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!WriteFile(hfile, buffer, bufferSize, &nWritten, NULL))
        {
            hr = E_FAIL;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        FlushFileBuffers(hfile);
    }

    if (!hfile || !CloseHandle(hfile))
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pGraph->RenderFile(m_TempFileName, NULL);
    }
    else
    {
        TearDownInterfaces();
    }
    
    return hr;
}

const TCHAR*
NIMAudioPlayer::GetAudioFormatFileExtension(ABPAL_AudioFormat format)
{
    switch (format)
    {
    case ABPAL_AudioFormat_AMR:
        return _T("AMR");
    case ABPAL_AudioFormat_CMX:
        return _T("CMX");
    case ABPAL_AudioFormat_QCP:
        return _T("QCP");
    case ABPAL_AudioFormat_WAV:
        return _T("WAV");
    case ABPAL_AudioFormat_AU:
        return _T("AU");
    case ABPAL_AudioFormat_GSM:
        return _T("GSM");
    case ABPAL_AudioFormat_WVE:
        return _T("WVE");
    case ABPAL_AudioFormat_RAW:
        return _T("RAW");
    case ABPAL_AudioFormat_AAC:
        return _T("AAC");
    case ABPAL_AudioFormat_MP3:
        return _T("MP3");
    case ABPAL_AudioFormat_OGG:
        return _T("OGG");
    default:
        return _T("");
    }
}

DWORD
NIMAudioPlayer::NotifyThreadProc(LPVOID pParameter)
{
    NIMAudioPlayer* pThis = static_cast<NIMAudioPlayer*>(pParameter);
    pThis->NotifyThread();
    pThis->m_NotifyThread = NULL;
    return (DWORD)1;
}

void
NIMAudioPlayer::NotifyThread(void)
{
    nb_boolean done = FALSE;

    HANDLE events[] = { m_MediaEventHandle, m_NotifyThreadStopEvent };
    int eventCount = sizeof(events) / sizeof(events[0]);

    while (!done)
    {
        DWORD ret = WaitForMultipleObjects(eventCount, events, FALSE, INFINITE);

        switch (ret)
        {
            case WAIT_OBJECT_0:         // media event pending
                while (!done)
                {
                    HRESULT hr =  E_FAIL;
                    long mediaEventCode = 0;
                    long param1 = 0;
                    long param2 = 0;

                    if (m_pEvent)
                    {
                        hr = m_pEvent->GetEvent(&mediaEventCode, &param1, &param2, 0);
                    }

                    if (!SUCCEEDED(hr))
                    {
                        // no event -- return to wait state
                        break;
                    }

                    (void)m_pEvent->FreeEventParams(mediaEventCode, param1, param2);

                    switch (mediaEventCode)
                    {
                        case EC_ERRORABORT:
                            SetState(ABPAL_AudioState_Error);
                            // fall through intentional

                        case EC_COMPLETE:
                            done = TRUE;
                            break;

                        default:
                            break;
                    }
                }
                break;

            case WAIT_OBJECT_0 + 1:     // stop event fired
                SetState(ABPAL_AudioState_Cancel);
                done = TRUE;
                break;
          
            default:                    // error occurred
                SetState(ABPAL_AudioState_Error);
                done = TRUE;
                break;
        }
    }

    ResetDeviceVolume();

    SetState(ABPAL_AudioState_Ended);
}

/*! @} */
