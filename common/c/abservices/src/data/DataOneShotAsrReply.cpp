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
 
 @file     DataOneShotAsrReply.cpp
 @date     03/02/2012
 @defgroup AB One Shot Asr Data Reply
 
 @brief    AB One Shot Asr Data Reply
 
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

#include "DataOneShotAsrReply.h"
#include "nbcontextprotected.h"  // To get data state
#include "abqalog.h"

using namespace std;


namespace aboneshotasr
{

DataOneShotAsrReply::DataOneShotAsrReply(NB_Context* context) :
    m_context(context),
    m_completionCode(0)
{
    m_dataState = NB_ContextGetDataState(m_context);
}

DataOneShotAsrReply::~DataOneShotAsrReply(void)
{
    FreeResults();
}

NB_Error
DataOneShotAsrReply::ParseTPSReply(tpselt te)
{
    // Free previous results
    FreeResults();

    if (!te)
    {
        return NE_BADDATA;
    }

    // Get completion code
    m_completionCode = te_getattru(te, "completion-code");

    // Get session id
    {
        const char* sessionID = te_getattrc(te, "session-id");
        if (sessionID)
        {
            m_sessionID = sessionID;
        }
    }

    // Set provider session id
    {
        const char* providerSessionID = te_getattrc(te, "provider-session-id");
        if (providerSessionID)
        {
            m_providerSessionID = providerSessionID;
        }
    }

    // Get results
    {
        int iter = 0;
        tpselt ce = NULL;

        while ((ce = te_nextchild(te, &iter)) != NULL)
        {
            if (nsl_strcmp(te_getname(ce), "asr-result") == 0)
            {
                const char* text = te_getattrc(ce, "text");
                if (text)
                {
                    m_results.push_back(string(text));
                }
            }
        }
    }

    return NE_OK;
}

uint32
DataOneShotAsrReply::GetCompletionCode(void) const
{
    return m_completionCode;
}

void
DataOneShotAsrReply::GetSessionID(std::string& sessionID) const
{
    sessionID = m_sessionID;
}

void
DataOneShotAsrReply::GetProviderSessionID(std::string& providerSessionID) const
{
    providerSessionID = m_providerSessionID;
}

void
DataOneShotAsrReply::GetResults(std::vector<std::string>& results) const
{
    results = m_results;
}

void
DataOneShotAsrReply::FreeResults(void)
{
    m_completionCode = 0;
    m_sessionID.clear();
    m_providerSessionID.clear();
    m_results.clear();
}

void
DataOneShotAsrReply::QALogReply(void) const
{
    if ((NB_ContextGetQaLog(m_context) != NULL))
    {
        AB_QaLogSpeechStreamReply(m_context, m_sessionID.c_str(), m_completionCode, m_providerSessionID.c_str(), m_results.size());

        for (uint32 i = 0; i < m_results.size(); i++)
        {
            AB_QaLogOneShotAsrResult(m_context, m_sessionID.c_str(), i + 1, m_results[i].c_str());
        }
    }
}

} //aboneshotasr

/*! @} */
