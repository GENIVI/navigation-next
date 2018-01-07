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
 
 @file     OneShotAsrParameters.cpp
 @date     02/09/2012
 @defgroup AB One Shot Asr Parameters API
 
 @brief    AB One Shot Asr Parameters Implementation
 
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

#include "OneShotAsrParameters.h"
#include "palstdlib.h"
#include "DataOneShotAsrQuery.h"

using namespace std;


namespace aboneshotasr
{

// Audio sample rate configs
static const uint32 WIDEBAND_SAMPLE_RATE =               16000;   // 16 kHz
static const char* const WIDEBAND_CODE =                  "16";

// Audio format strings
static const char* const AUDIO_FORMAT_WAV =              "wav";
static const char* const AUDIO_FORMAT_SPX =              "spx";
static const char* const AUDIO_FORMAT_AMR =              "amr";
static const char* const AUDIO_FORMAT_QCP =              "qcp";

// Asr screen strings
static const char* const ASR_SCREEN_CAROUSEL =      "carousel";
static const char* const ASR_SCREEN_MAP =                "map";
static const char* const ASR_SCREEN_ADDRESS =        "address";
static const char* const ASR_SCREEN_POI =                "poi";


OneShotAsrParameters::OneShotAsrParameters(NB_Context* context) :
    m_context(context)
{
    m_dataQuery = new DataOneShotAsrQuery(m_context);
}

OneShotAsrParameters::OneShotAsrParameters(OneShotAsrParameters& parameters) :
    m_context(parameters.m_context)
{
    m_dataQuery = new DataOneShotAsrQuery(*(parameters.m_dataQuery));
}

OneShotAsrParameters::~OneShotAsrParameters(void)
{
    if (m_dataQuery)
    {
        delete m_dataQuery;
        m_dataQuery = NULL;
    }
}

void
OneShotAsrParameters::Release(void)
{
    delete this;
}

NB_Error
OneShotAsrParameters::Init(const string& sessionID, const string& language, OneShotAsrEntryPoint screen, const NB_GpsLocation* location)
{
    NB_Error err = NE_OK;

    err = m_dataQuery->SetSessionID(sessionID);
    err = err ? err : m_dataQuery->SetLanguage(language);

    if (err == NE_OK)
    {
        string strScreen;

        switch (screen)
        {
            case OSASR_EntryPoint_Carousel:
            {
                strScreen = ASR_SCREEN_CAROUSEL;
                break;
            }

            case OSASR_EntryPoint_Map:
            {
                strScreen = ASR_SCREEN_MAP;
                break;
            }

            case OSASR_EntryPoint_EnterAddress:
            {
                strScreen = ASR_SCREEN_ADDRESS;
                break;
            }

            case OSASR_EntryPoint_Places:
            {
                strScreen = ASR_SCREEN_POI;
                break;
            }

            default:
                break;
        }

        err = m_dataQuery->SetScreen(strScreen);
    }

    if (location)
    {
        err = err ? err : m_dataQuery->SetGpsLocation(location);
    }

    return err;
}

tpselt
OneShotAsrParameters::CreateTPSQuery(void) const
{
    tpselt te = NULL;
    te = m_dataQuery->CreateTPSQuery();
    return te;
}

void
OneShotAsrParameters::QALogQuery(void) const
{
    m_dataQuery->QALogQuery();
}

const char*
OneShotAsrParameters::GetTargetName(void) const
{
    return m_dataQuery->GetTargetName();
}

uint32
OneShotAsrParameters::GetSequenceID(void) const
{
    return m_dataQuery->GetSequenceID();
}

uint32
OneShotAsrParameters::GetEndSpeech(void) const
{
    return m_dataQuery->GetEndSpeech();
}

NB_Error
OneShotAsrParameters::SetAudioFormat(ABPAL_AudioFormat audioFormat, uint32 bitsPerSample, uint32 sampleRate)
{
    NB_Error err = NE_NOSUPPORT;

    string strAudioFormat;

    switch (audioFormat)
    {
        case ABPAL_AudioFormat_WAV:
        case ABPAL_AudioFormat_RAW:
        {
            strAudioFormat = AUDIO_FORMAT_WAV;
            err = NE_OK;

            if (sampleRate == WIDEBAND_SAMPLE_RATE)
            {
                strAudioFormat.append(WIDEBAND_CODE);
            }
            break;
        }

        case ABPAL_AudioFormat_SPX:
        {
            strAudioFormat = AUDIO_FORMAT_SPX;
            err = NE_OK;

            if (sampleRate == WIDEBAND_SAMPLE_RATE)
            {
                strAudioFormat.append(WIDEBAND_CODE);

            }
            break;
        }

        case ABPAL_AudioFormat_AMR:
        {
            strAudioFormat = AUDIO_FORMAT_AMR;
            err = NE_OK;
            break;
        }

        case ABPAL_AudioFormat_QCP:
        {
            strAudioFormat = AUDIO_FORMAT_QCP;
            err = NE_OK;
            break;
        }

        default:
            break;
    }

    err = err ? err : m_dataQuery->SetAudioFormat(strAudioFormat);

    return err;
}

NB_Error
OneShotAsrParameters::SetAudioData(const byte* data, uint32 dataSize, uint32 sequenceID, nb_boolean endSpeech)
{
    // Only last audio buffer could be empty
    if ((!data || !dataSize) && !endSpeech)
    {
        return NE_BADDATA;
    }

    NB_Error err = NE_OK;

    err = err ? err : m_dataQuery->SetSequenceID(sequenceID);
    err = err ? err : m_dataQuery->SetEndSpeech((uint32)endSpeech);
    err = err ? err : m_dataQuery->SetAudioData(data, dataSize);

    return err;
}

void
OneShotAsrParameters::GetSessionID(string& sessionID) const
{
    m_dataQuery->GetSessionID(sessionID);
}

NB_Error
CreateOneShotAsrParameters(NB_Context* context, OneShotAsrParametersInterface** parameters)
{
    if (!context || !parameters)
    {
        return NE_BADDATA;
    }

    *parameters = new OneShotAsrParameters(context);

    return NE_OK;
}

} //aboneshotasr

/*! @} */
