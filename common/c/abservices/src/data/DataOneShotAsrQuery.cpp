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
 
 @file     DataOneShotAsrQuery.cpp
 @date     02/28/2012
 @defgroup AB One Shot Asr Data Query
 
 @brief    AB One Shot Asr Data Query
 
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

#include "DataOneShotAsrQuery.h"
#include "nbcontextprotected.h"  // To get data state
#include "abqalog.h"

using namespace std;


namespace aboneshotasr
{

static const char* const ONESHOTASR_QUERY_TARGET =  "asr-reco";

static const char* const LANGUAGE_EN_US =              "en-US";

static const byte LANGUAGE_UNDEFINED_ID =                    0;
static const byte LANGUAGE_OTHER_ID =                        1;
static const byte LANGUAGE_EN_US_ID =                        2;

static const char* const ASR_SCREEN_CAROUSEL =      "carousel";
static const char* const ASR_SCREEN_MAP =                "map";
static const char* const ASR_SCREEN_NAVIGATION =  "navigation";
static const char* const ASR_SCREEN_ADDRESS =        "address";
static const char* const ASR_SCREEN_AIRPORT =        "airport";
static const char* const ASR_SCREEN_POI =                "poi";

static const byte ASR_SCREEN_CAROUSEL_ID =                   0;
static const byte ASR_SCREEN_MAP_ID =                        1;
static const byte ASR_SCREEN_NAVIGATION_ID =                 2;
static const byte ASR_SCREEN_ADDRESS_ID =                    3;
static const byte ASR_SCREEN_AIRPORT_ID =                    4;
static const byte ASR_SCREEN_POI_ID =                        5;
static const byte ASR_SCREEN_UNDEFINED_ID =                  6;

static const char* const AUDIO_FORMAT_WAV8 =             "wav";
static const char* const AUDIO_FORMAT_WAV16 =          "wav16";
static const char* const AUDIO_FORMAT_SPX8 =             "spx";
static const char* const AUDIO_FORMAT_SPX16 =          "spx16";
static const char* const AUDIO_FORMAT_AMR =              "amr";
static const char* const AUDIO_FORMAT_QCP =              "qcp";

static const byte AUDIO_FORMAT_UNDEFINED_ID =                0;
static const byte AUDIO_FORMAT_OTHER_ID =                    1;
static const byte AUDIO_FORMAT_WAV16_ID =                    2;
static const byte AUDIO_FORMAT_WAV8_ID =                     3;
static const byte AUDIO_FORMAT_SPX16_ID =                    4;
static const byte AUDIO_FORMAT_SPX8_ID =                     5;
static const byte AUDIO_FORMAT_AMR_ID =                      6;
static const byte AUDIO_FORMAT_QCP_ID =                      7;


DataOneShotAsrQuery::DataOneShotAsrQuery(NB_Context* context) :
    m_context(context),
    m_sessionID(NULL),
    m_language(NULL),
    m_screen(NULL),
    m_audioFormat(NULL),
    m_sequenceID(0),
    m_endSpeech(0)
{
    m_dataState = NB_ContextGetDataState(m_context);
    Init();
}

DataOneShotAsrQuery::DataOneShotAsrQuery(DataOneShotAsrQuery& dataQuery) :
    m_context(dataQuery.m_context),
    m_dataState(dataQuery.m_dataState),
    m_sessionID(NULL),
    m_language(NULL),
    m_screen(NULL),
    m_audioFormat(NULL),
    m_sequenceID(dataQuery.m_sequenceID),
    m_endSpeech(dataQuery.m_endSpeech)
{
    if (Init() == NE_OK)
    {
        NB_Error err = NE_OK;

        err = data_string_copy(m_dataState, &m_sessionID, &(dataQuery.m_sessionID));
        err = err ? err : data_string_copy(m_dataState, &m_language, &(dataQuery.m_language));
        err = err ? err : data_string_copy(m_dataState, &m_screen, &(dataQuery.m_screen));
        err = err ? err : data_gps_copy(m_dataState, &m_gpsLocation, &(dataQuery.m_gpsLocation));
        err = err ? err : data_string_copy(m_dataState, &m_audioFormat, &(dataQuery.m_audioFormat));
        err = err ? err : data_blob_copy(m_dataState, &m_audioData, &(dataQuery.m_audioData));
        (void)err;
    }
}

DataOneShotAsrQuery::~DataOneShotAsrQuery(void)
{
    data_string_free(m_dataState, &m_sessionID);
    data_string_free(m_dataState, &m_language);
    data_string_free(m_dataState, &m_screen);
    data_gps_free(m_dataState, &m_gpsLocation);
    data_string_free(m_dataState, &m_audioFormat);
    data_blob_free(m_dataState, &m_audioData);
}

NB_Error
DataOneShotAsrQuery::Init(void)
{
    nsl_memset(&m_gpsLocation, 0, sizeof(data_gps));
    nsl_memset(&m_audioData, 0, sizeof(data_blob));

    NB_Error err = NE_OK;

    err = data_string_init(m_dataState, &m_sessionID);
    err = err ? err : data_string_init(m_dataState, &m_language);
    err = err ? err : data_string_init(m_dataState, &m_screen);
    err = err ? err : data_gps_init(m_dataState, &m_gpsLocation);
    err = err ? err : data_string_init(m_dataState, &m_audioFormat);
    err = err ? err : data_blob_init(m_dataState, &m_audioData);

    return err;
}

NB_Error
DataOneShotAsrQuery::SetSessionID(const string& sessionID)
{
    NB_Error err = NE_OK;
    err = data_string_set(m_dataState, &m_sessionID, sessionID.c_str());
    return err;
}

NB_Error
DataOneShotAsrQuery::SetLanguage(const string& language)
{
    NB_Error err = NE_OK;
    err = data_string_set(m_dataState, &m_language, language.c_str());
    return err;
}

NB_Error
DataOneShotAsrQuery::SetScreen(const string& screen)
{
    NB_Error err = NE_OK;
    err = data_string_set(m_dataState, &m_screen, screen.c_str());
    return err;
}

NB_Error
DataOneShotAsrQuery::SetGpsLocation(const NB_GpsLocation* location)
{
    NB_Error err = NE_OK;
    data_gps_from_gpsfix(m_dataState, &m_gpsLocation, const_cast<NB_GpsLocation*>(location));
    return err;
}

NB_Error
DataOneShotAsrQuery::SetAudioFormat(const string& audioFormat)
{
    NB_Error err = NE_OK;
    err = data_string_set(m_dataState, &m_audioFormat, audioFormat.c_str());
    return err;
}

NB_Error
DataOneShotAsrQuery::SetSequenceID(uint32 sequenceID)
{
    NB_Error err = NE_OK;
    m_sequenceID = sequenceID;
    return err;
}

void
DataOneShotAsrQuery::GetSessionID(string& sessionID) const
{
    sessionID = data_string_get(m_dataState, &m_sessionID);
}

uint32
DataOneShotAsrQuery::GetSequenceID(void) const
{
    return m_sequenceID;
}

uint32
DataOneShotAsrQuery::GetEndSpeech(void) const
{
    return m_endSpeech;
}

const char*
DataOneShotAsrQuery::GetTargetName(void) const
{
    return ONESHOTASR_QUERY_TARGET;
}

NB_Error
DataOneShotAsrQuery::SetAudioData(const byte* data, uint32 dataSize)
{
    NB_Error err = NE_OK;
    err = data_blob_set(m_dataState, &m_audioData, data, dataSize);
    return err;
}

NB_Error
DataOneShotAsrQuery::SetEndSpeech(uint32 endSpeech)
{
    NB_Error err = NE_OK;
    m_endSpeech = endSpeech;
    return err;
}

tpselt
DataOneShotAsrQuery::CreateTPSQuery(void) const
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("asr-stream-query");
    if (!te)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "user-lang", data_string_get(m_dataState, &m_language)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "session-id", data_string_get(m_dataState, &m_sessionID)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "asr-screen", data_string_get(m_dataState, &m_screen)))
    {
        goto errexit;
    }

    if (!te_setattru(te, "sequence-id", m_sequenceID))
    {
        goto errexit;
    }

    if (!te_setattru(te, "end-speech", m_endSpeech))
    {
        goto errexit;
    }

    // audio
    {
        ce = CreateAudioFieldTPSQuery();
        if (!ce || !te_attach(te, ce))
        {
            goto errexit;
        }

        ce = NULL;
    }

    // location
    if (m_gpsLocation.packed[0] != (byte)0xFF)
    {
        ce = data_gps_to_tps(m_dataState, const_cast<data_gps*>(&m_gpsLocation));
        if (!ce || !te_attach(te, ce))
        {
            goto errexit;
        }

        ce = NULL;
    }

    return te;

errexit:

    te_dealloc(te);
    te = NULL;

    te_dealloc(ce);
    ce = NULL;

    return NULL;
}

tpselt
DataOneShotAsrQuery::CreateAudioFieldTPSQuery(void) const
{
    tpselt te = NULL;

    te = te_new("audio");
    if (!te)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "format", data_string_get(m_dataState, &m_audioFormat)))
    {
        goto errexit;
    }

    if (!te_setattr(te, "data", reinterpret_cast<char*>(m_audioData.data), m_audioData.size))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    te = NULL;

    return NULL;
}

byte
DataOneShotAsrQuery::ConvertLanguageStringToByteType(const char* language) const
{
    if ((language == NULL) || (nsl_strlen(language) == 0))
    {
        return LANGUAGE_UNDEFINED_ID;
    }
    else if (nsl_stricmp(language, LANGUAGE_EN_US) == 0)
    {
        return LANGUAGE_EN_US_ID;
    }
    else
    {
        return LANGUAGE_OTHER_ID;
    }
}

byte
DataOneShotAsrQuery::ConvertScreenStringToByteType(const char* screen) const
{
    if ((screen == NULL) || (nsl_strlen(screen) == 0))
    {
        return ASR_SCREEN_UNDEFINED_ID;
    }
    else if (nsl_stricmp(screen, ASR_SCREEN_CAROUSEL) == 0)
    {
        return ASR_SCREEN_CAROUSEL_ID;
    }
    else if (nsl_stricmp(screen, ASR_SCREEN_MAP) == 0)
    {
        return ASR_SCREEN_MAP_ID;
    }
    else if (nsl_stricmp(screen, ASR_SCREEN_NAVIGATION) == 0)
    {
        return ASR_SCREEN_NAVIGATION_ID;
    }
    else if (nsl_stricmp(screen, ASR_SCREEN_ADDRESS) == 0)
    {
        return ASR_SCREEN_ADDRESS_ID;
    }
    else if (nsl_stricmp(screen, ASR_SCREEN_AIRPORT) == 0)
    {
        return ASR_SCREEN_AIRPORT_ID;
    }
    else if (nsl_stricmp(screen, ASR_SCREEN_POI) == 0)
    {
        return ASR_SCREEN_POI_ID;
    }
    else
    {
        return ASR_SCREEN_UNDEFINED_ID;
    }
}

byte
DataOneShotAsrQuery::ConvertAudioFormatStringToByteType(const char* audioFormat) const
{
    if ((audioFormat == NULL) || (nsl_strlen(audioFormat) == 0))
    {
        return AUDIO_FORMAT_UNDEFINED_ID;
    }
    else if (nsl_stricmp(audioFormat, AUDIO_FORMAT_WAV8) == 0)
    {
        return AUDIO_FORMAT_WAV8_ID;
    }
    else if (nsl_stricmp(audioFormat, AUDIO_FORMAT_WAV16) == 0)
    {
        return AUDIO_FORMAT_WAV16_ID;
    }
    else if (nsl_stricmp(audioFormat, AUDIO_FORMAT_SPX8) == 0)
    {
        return AUDIO_FORMAT_SPX8_ID;
    }
    else if (nsl_stricmp(audioFormat, AUDIO_FORMAT_SPX16) == 0)
    {
        return AUDIO_FORMAT_SPX16_ID;
    }
    else if (nsl_stricmp(audioFormat, AUDIO_FORMAT_AMR) == 0)
    {
        return AUDIO_FORMAT_AMR_ID;
    }
    else if (nsl_stricmp(audioFormat, AUDIO_FORMAT_QCP) == 0)
    {
        return AUDIO_FORMAT_QCP_ID;
    }
    else
    {
        return AUDIO_FORMAT_OTHER_ID;
    }
}

void
DataOneShotAsrQuery::QALogQuery(void) const
{
    if (NB_ContextGetQaLog(m_context) != NULL)
    {
        AB_QaLogSpeechStreamRequest(m_context,
                                    data_string_get(m_dataState, &m_sessionID),
                                    ConvertLanguageStringToByteType(data_string_get(m_dataState, &m_language)),
                                    ConvertScreenStringToByteType(data_string_get(m_dataState, &m_screen)),
                                    m_sequenceID,
                                    (byte)m_endSpeech,
                                    ConvertAudioFormatStringToByteType(data_string_get(m_dataState, &m_audioFormat)),
                                    m_audioData.size);
    }
}

} //aboneshotasr

/*! @} */
