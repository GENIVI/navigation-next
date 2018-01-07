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

    @file     analyticsengine.cpp
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2014 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "analyticsengine.h"
#include "analyticsengineimpl.h"
#include "nbcontextprotected.h"
#include "palclock.h"
#include "nbmacros.h"
#include "contextbasedsingleton.h"

//#define ENABLE_ANALYTICS
using namespace nbcommon;

#ifdef ENABLE_ANALYTICS
struct InitializeParam
{
    InitializeParam(nbcommon::AnalyticsEngineImpl* impl, NB_Context* context, const std::string& dbPath, const std::string& schema)
    {
        this->impl = impl;
        this->context = context;
        this->dbPath = dbPath;
        this->schema = schema;
    }
    nbcommon::AnalyticsEngineImpl*  impl;
    NB_Context*                     context;
    std::string                     dbPath;
    std::string                     schema;
};

/*! Initialize some members in ccc thread, else app will crash*/
static void InitializeInCCCThread(PAL_Instance* /*pal*/, void* userData)
{
    InitializeParam* param = static_cast<InitializeParam*>(userData);
    nsl_assert(param);
    param->impl->Initialize(param->context, param->dbPath, param->schema);
    delete param;
}
#endif

NB_Error AnalyticsEngine::Initialize(const std::string& dbPath, const std::string& schema)
{
#ifdef ENABLE_ANALYTICS
    if(!m_context)
    {
        return NE_UNEXPECTED;
    }

    PAL_Instance* pal = NB_ContextGetPal(m_context);
    nsl_assert(pal);
    TaskId id;
    PAL_EventTaskQueueAdd(pal, InitializeInCCCThread, CCC_NEW InitializeParam(m_impl, m_context, dbPath, schema), &id);
#endif
    return NE_OK;
}

/*! This function may be executed in any thread, so NO CCC-SPECIFIC WORK SHOULD BE PUT HERE!!!*/
AnalyticsEngine::AnalyticsEngine(NB_Context* context):
    m_context(context)
{
#ifdef ENABLE_ANALYTICS
    m_impl = CCC_NEW AnalyticsEngineImpl();
    nsl_assert(m_impl);
#endif
}

AnalyticsEngine::~AnalyticsEngine()
{
#ifdef ENABLE_ANALYTICS
    if(m_context)
    {
        NB_ASSERT_CCC_THREAD(m_context);
        delete m_impl;
    }
#endif
}

NB_Error AnalyticsEngine::AddAnalyticsEvent(const std::string& event)
{
#ifdef ENABLE_ANALYTICS
    NB_Error error = m_impl->ValidateString(event);
    if(error != NE_OK)
    {
        return error;
    }
    return m_impl->AddEvent(event);
#else
   return NE_OK;
#endif
}

NB_Error AnalyticsEngine::NoticeAlive()
{
#ifdef ENABLE_ANALYTICS
    return m_impl->NoticeAlive();
#else
   return NE_OK;
#endif
}

NB_Error AnalyticsEngine::KeepAlive(bool alive)
{
#ifdef ENABLE_ANALYTICS
    return m_impl->KeepAlive(alive);
#else
   return NE_OK;
#endif
}

std::string AnalyticsEngine::GetSessionId()
{
#ifdef ENABLE_ANALYTICS
    m_impl->NoticeAlive();
    return m_impl->GetSessionId();
#else
   return "";
#endif
}

NB_Error AnalyticsEngine::MasterClear()
{
#ifdef ENABLE_ANALYTICS
    return m_impl->RemoveAllEvents();
#else
   return NE_OK;
#endif
}

NB_Error AnalyticsEngine::ForceUpload()
{
#ifdef ENABLE_ANALYTICS
    return m_impl->Upload(AnalyticsEngineImpl::UR_FORCE);
#else
   return NE_OK;
#endif
}

NB_Error AnalyticsEngine::SetDataRoaming(bool bRoaming)
{
#ifdef ENABLE_ANALYTICS
    return m_impl->SetDataRoaming(bRoaming);
#else
   return NE_OK;
#endif
}

NB_Error AnalyticsEngine::SetAnalyticsListener(AnalyticsListener* listener)
{
#ifdef ENABLE_ANALYTICS
    return m_impl->SetAnalyticsListener(listener);
#else
   return NE_OK;
#endif
}

AnalyticsEngine* AnalyticsEngine::GetInstance(NB_Context* context)
{
    shared_ptr<AnalyticsEngine> engine = ContextBasedSingleton<AnalyticsEngine>::getInstance(context);
    if (engine)
    {
        return engine.get();
    }
    else
    {
        return NULL;
    }
}

const AnalyticsWifiConfig* AnalyticsEngine::GetAnalyticsWifiConfig() const
{
#ifdef ENABLE_ANALYTICS
    return m_impl->GetAnalyticsWifiConfig();
#else
     static AnalyticsWifiConfig config;
     memset(&config, 0, sizeof(config));
     return &config;
#endif
}

/*! @} */
