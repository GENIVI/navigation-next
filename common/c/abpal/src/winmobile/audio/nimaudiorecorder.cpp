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

@file     NimAudioRecorder.cpp
@date     03/10/2009
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

#include "NimAudioRecorder.h"
#include "abpalutil.h"

#define WAIT_DATA_EXTRA                 1000    // Wait an extra second
#define DEFAULT_SAMPLES_PER_SECOND      11025   // Default is 11.025KHz
#define DEFAULT_BITS_PER_SAMPLE         16      // Default to 16 bits per sample
#define DEFAULT_FORMAT                  ABPAL_AudioFormat_AMR


NIMAudioRecorder::NIMAudioRecorder(PAL_Instance* pPal, ABPAL_AudioFormat format) :
    NIMAudio(pPal, format),
    m_WaveIn(NULL),
    m_Duration(0),
    m_Channels(Mono),
    m_SamplesPerSecond(DEFAULT_SAMPLES_PER_SECOND),
    m_BitsPerSample(DEFAULT_BITS_PER_SAMPLE)
{
    SetState(ABPAL_AudioState_Init);
}

NIMAudioRecorder::~NIMAudioRecorder(void)
{
    Stop();
}

PAL_Error NIMAudioRecorder::Record(byte** buffer, int bufferSize, ABPAL_AudioRecorderCallback* callback,
                                    void* userData)
{
    PAL_Error       err = PAL_ErrAudioGeneral;
    DWORD           dwDeviceId = WAVE_MAPPER;   // We will capture from any compatible device
    WAVEFORMATEX    waveFormat;

    // Ensure that we have a valid buffer to record to
    if (!*buffer || bufferSize == 0)
    {
        SetState(ABPAL_AudioState_Error);
        return PAL_ErrBadParam;
    }

    // Make oem-specific adjustments for recording
    DoOemSpecificAdjustments(TRUE);
    
    // Get the starting location of the buffer
    char *pBuffer = (char *)*buffer;

    // Setup the Wave Format structure
    memset(&waveFormat, 0, sizeof(WAVEFORMATEX));
    waveFormat.cbSize = 0;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.wBitsPerSample = (WORD) m_BitsPerSample;
    waveFormat.nSamplesPerSec = m_SamplesPerSecond;
    waveFormat.nChannels = (WORD) m_Channels;
    waveFormat.nBlockAlign = (WORD)(m_Channels * (m_BitsPerSample / 8));
    waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * m_SamplesPerSecond;

    // Copy the waveFormat to the first part of our buffer
    memcpy(pBuffer, &waveFormat, sizeof(WAVEFORMATEX));

    // Move the buffer to just after the header
    pBuffer += sizeof(WAVEFORMATEX);

    // open the wave capture device
    MMRESULT mRes = waveInOpen(&m_WaveIn, dwDeviceId, &waveFormat, (DWORD)m_MediaEventHandle, NULL, CALLBACK_EVENT);
    if (mRes != MMSYSERR_NOERROR) 
    {
        SetState(ABPAL_AudioState_Error);
        return PAL_ErrAudioDeviceOpen;
    }

    // Setup the Wave Header
    memset(&m_WaveHdr, 0, sizeof(m_WaveHdr));
    m_WaveHdr.dwBufferLength = bufferSize  - sizeof(WAVEFORMATEX);
    m_WaveHdr.lpData = pBuffer;

    // Prepare the capture buffer
    mRes = waveInPrepareHeader(m_WaveIn, &m_WaveHdr, sizeof(m_WaveHdr));
    if (mRes == MMSYSERR_NOERROR)
    {
        // Set the error in case of failure
        err = PAL_ErrAudioPrepError;
        // Set the buffer for the device to use
        mRes = waveInAddBuffer(m_WaveIn, &m_WaveHdr, sizeof(m_WaveHdr));
        if (mRes == MMSYSERR_NOERROR)
        {    
            mRes = waveInStart(m_WaveIn);
            if (mRes == MMSYSERR_NOERROR)
            {
                // Calculate the number of milliseconds we can record
                DWORD dwDuration = (DWORD)(((UINT64)m_WaveHdr.dwBufferLength) * 1000 / waveFormat.nAvgBytesPerSec);
                m_Duration = dwDuration;

                SetState(ABPAL_AudioState_Recording);

                DWORD dwWait= WaitForSingleObject(m_MediaEventHandle, dwDuration + WAIT_DATA_EXTRA);
                if (dwWait != WAIT_OBJECT_0) 
                {
                    waveInReset(m_WaveIn);
                }
                else
                {
                    err = PAL_Ok;
                }
            }
        }

        // Cleanup the buffer
        waveInUnprepareHeader(m_WaveIn, &m_WaveHdr, sizeof(m_WaveHdr));
    }

    // Close the device
    waveInClose(m_WaveIn);

    if (err == PAL_Ok)
    {
        SetState(ABPAL_AudioState_Ended);
    }

    return err;
}

bool
NIMAudioRecorder::Stop()
{
    // Reset oem-specific adjustments
    DoOemSpecificAdjustments(FALSE);
    
    // Reset the recorder
    MMRESULT mRes = waveInReset(m_WaveIn);

    if (mRes == MMSYSERR_NOERROR)
    {
        // Cleanup our header
        waveInUnprepareHeader(m_WaveIn, &m_WaveHdr, sizeof(m_WaveHdr));
    }

    return (mRes == MMSYSERR_NOERROR) ? TRUE : FALSE;
}

//---------------------------------------------------------------------------
// SPECIFIC OEM DEVICE FIXES
//
// @todo: better way to handle these device-specific variations
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
const WCHAR* SAMSUNG_SCHI910 =          L"SAMSUNG SCH-i910";
#define WPDM_PRIVATE                        (WM_USER + 10)
#define WPDM_PRIVATE_WRITE_AC97_FIX         (WPDM_PRIVATE + 55)
#define WPDM_PRIVATE_WRITE_AC97             (WPDM_PRIVATE + 0)
//---------------------------------------------------------------------------

void
NIMAudioRecorder::DoOemSpecificAdjustments(BOOL set)
{
    if (IsOemEqual(SAMSUNG_SCHI910))
    {
        if (set)
        {
            SendMessageToAudioDriver(WPDM_PRIVATE_WRITE_AC97_FIX, 0x5C, 0x0000);
        }
        else
        {
            SendMessageToAudioDriver(WPDM_PRIVATE_WRITE_AC97_FIX, 0x5C, (DWORD)-1);	
            SendMessageToAudioDriver(WPDM_PRIVATE_WRITE_AC97, 0x5C, 0x0008);
        }
        // @review: yield for messages
        Sleep(100);
    }
}

bool
NIMAudioRecorder::SendMessageToAudioDriver(DWORD pdmCode, DWORD param1, DWORD param2)
{
    MMRESULT msgResult = MMSYSERR_ERROR;

    UINT i = 0;
    UINT deviceCount = waveOutGetNumDevs();

    for (i = 0; i < deviceCount; i++)
    {
        WAVEOUTCAPS waveoutcaps = {0};

        waveOutGetDevCaps(i, &waveoutcaps, sizeof(WAVEOUTCAPS));

        if(!_tcscmp(waveoutcaps.szPname, TEXT("Audio Output")))
        {
            msgResult = waveOutMessage((HWAVEOUT)i, pdmCode, param1, param2);
        }
    }
    return (msgResult == MMSYSERR_NOERROR);
}

/*! @} */
