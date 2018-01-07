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
 
 @file     DataOneShotAsrQuery.h
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

#ifndef DATAONESHOTASRQUERY_H
#define DATAONESHOTASRQUERY_H


#include "nberror.h"
#include "paltypes.h"
#include "base.h"
#include "nbcontext.h"
#include "datautil.h"
#include "tpselt.h"
#include "nbgpstypes.h"
#include "data_blob.h"
#include "data_string.h"
#include "data_gps.h"
#include <string>


namespace aboneshotasr
{

class DataOneShotAsrQuery : public Base
{
public:
    DataOneShotAsrQuery(NB_Context* context);
    DataOneShotAsrQuery(DataOneShotAsrQuery& dataQuery);
    ~DataOneShotAsrQuery(void);

    NB_Error SetSessionID(const std::string& sessionID);
    NB_Error SetLanguage(const std::string& language);
    NB_Error SetScreen(const std::string& screen);
    NB_Error SetGpsLocation(const NB_GpsLocation* location);
    NB_Error SetAudioFormat(const std::string& audioFormat);
    NB_Error SetSequenceID(uint32 sequenceID);
    NB_Error SetAudioData(const byte* data, uint32 dataSize);
    NB_Error SetEndSpeech(uint32 endSpeech);

    void GetSessionID(std::string& sessionID) const;
    uint32 GetSequenceID(void) const;
    uint32 GetEndSpeech(void) const;
    const char* GetTargetName(void) const;

    tpselt CreateTPSQuery(void) const;
    void QALogQuery(void) const;

protected:
    NB_Error Init(void);
    tpselt CreateAudioFieldTPSQuery(void) const;

    // QA log staff
    byte ConvertLanguageStringToByteType(const char* language) const;
    byte ConvertScreenStringToByteType(const char* screen) const;
    byte ConvertAudioFormatStringToByteType(const char* audioFormat) const;

private:
    NB_Context*         m_context;
    data_util_state*    m_dataState;
    data_string         m_sessionID;
    data_string         m_language;
    data_string         m_screen;
    data_gps            m_gpsLocation;
    data_string         m_audioFormat;
    uint32              m_sequenceID;
    data_blob           m_audioData;
    uint32              m_endSpeech;
};

} //aboneshotasr

#endif //DATAONESHOTASRQUERY_H

/*! @} */
