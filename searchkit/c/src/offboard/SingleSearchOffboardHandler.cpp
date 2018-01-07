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
    @file     SingleSearchOffboardHandler.cpp
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "SingleSearchOffboardHandler.h"
#include "nbcontextprotected.h"
#include <palstdlib.h>
#include "TpsSingleSearchSourceHandler.h"
#include "SingleSearchRequestPrivate.h"
#include "CommonFunction.h"
#include "SingleSearchOffboardInformation.h"
#include "AnalyticsSearchEventManager.h"

using namespace protocol;
using namespace nbmap;
using namespace nbsearch;

SingleSearchOffboardHandler::SingleSearchOffboardHandler(
        SingleSearchHandlerPrivate* outer,
        NB_Context* context,
        const SingleSearchRequest* user,
        const std::shared_ptr<SingleSearchListener>& listener,
        const std::shared_ptr<SingleSearchParameters>& param,
        const std::shared_ptr<SingleSearchState>& state)
    : m_outer(outer)
    , m_context(context)
    , m_param(param)
    , m_userRequest(user)
    , m_listener(listener)
    , m_singleSearchAsyncCallback(new SingleSearchAsyncCallback(this))
    , m_state(state)
{

    m_UITaskQueue = shared_ptr<UiTaskQueue>(new UiTaskQueue(NB_ContextGetPal(m_context)));
}

SingleSearchOffboardHandler::~SingleSearchOffboardHandler()
{
    if (m_handler != NULL)
    {
        m_handler->Cancel();
        m_handler.reset();
    }
}

void SingleSearchOffboardHandler::SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId)
{
    m_inputSource = std::string(inputSource);
    m_invocationMethod = std::string(invocationMethod);
    m_screenId = std::string(screenId);
}

NB_Error SingleSearchOffboardHandler::Request(NB_Context* context)
{
    if (m_param == NULL)
    {
        return NE_BADDATA;
    }
    NB_Error err = NE_OK;

    if (m_handler == NULL)
    {
        TpsAtlasbookProtocol tpsAtlasbookProtocol(context);
        m_handler = tpsAtlasbookProtocol.GetSingleSearchSourceHandler();
        if (!m_handler)
        {
            return NE_NOMEM;
        }
    }
    if (!m_screenId.empty() && !m_invocationMethod.empty() && !m_inputSource.empty())
    {
        shared_ptr<protocol::InvocationContext> invocationContext = GetInvocationConext("search", m_inputSource.c_str(), m_invocationMethod.c_str(), m_screenId.c_str());
        if (invocationContext)
        {
            static_cast<OffBoardSingleSearchSourceHandler*>(m_handler.get())->SetInvocationContext(invocationContext);
        }
    }

    m_handler->Request(m_param->CreateParameters(), m_singleSearchAsyncCallback);

    return err;
}

void SingleSearchOffboardHandler::Cancel()
{
    if (m_handler)
    {
        m_handler->Cancel();
    }
}

bool SingleSearchOffboardHandler::IsInProgress()
{
    if (m_handler)
    {
        return static_cast<OffBoardSingleSearchSourceHandler*>(m_handler.get())->IsInProgress();
    }
    else
    {
        return false;
    }
}

void SingleSearchOffboardHandler::UI_NotifySuccess(const OffboardSingleSearchTask* pTask)
{
    const OffboardSingleSearchUITaskSuccess* task = static_cast<const OffboardSingleSearchUITaskSuccess*>(pTask);
    if (m_listener && m_outer->IsRequestAlive())
    {
        m_listener->OnSuccess(*this->m_userRequest, *task->m_information);
    }
    delete task->m_information;
}

void SingleSearchOffboardHandler::UI_NotifyError(const OffboardSingleSearchTask* pTask)
{
    const OffboardSingleSearchUITaskError* task = static_cast<const OffboardSingleSearchUITaskError*>(pTask);
    if (m_listener && m_outer->IsRequestAlive())
    {
        m_listener->OnError(*this->m_userRequest, task->m_error);
    }
}

void SingleSearchOffboardHandler::UI_NotifyProgress(const OffboardSingleSearchTask* pTask)
{
    const OffboardSingleSearchUITaskProgress* task = static_cast<const OffboardSingleSearchUITaskProgress*>(pTask);
    if (m_listener && m_outer->IsRequestAlive())
    {
        m_listener->OnProgressUpdated(*this->m_userRequest, task->m_percentage);
    }
}

void SingleSearchOffboardHandler::NotifySuccess(protocol::SingleSearchSourceParametersSharedPtr param, protocol::SingleSearchSourceInformationSharedPtr information)
{
    AnalyticsManager::GetInstance(m_context)->LogSearchQueryEvent(param);
    SingleSearchInformation* info = new SingleSearchInformationImpl(information, AnalyticsManager::GetInstance(m_context)->GetSearchQueryEventId());
    if (m_UITaskQueue)
    {
        // if SingleSearchOnboardHandler was delete before UI_NotifySuccess, the app will crash
        m_UITaskQueue->AddTask(new OffboardSingleSearchUITaskSuccess(this, &SingleSearchOffboardHandler::UI_NotifySuccess, info, m_state));
    }
}

void SingleSearchOffboardHandler::NotifyError(NB_Error error)
{
    if (m_UITaskQueue)
    {
        // if SingleSearchOnboardHandler was delete before UI_NotifyError, the app will crash
        m_UITaskQueue->AddTask(new OffboardSingleSearchUITaskError(this, &SingleSearchOffboardHandler::UI_NotifyError, error, m_state));
    }
}

void SingleSearchOffboardHandler::NotifyProgress(int percentage)
{
    if (m_UITaskQueue)
    {
        // if SingleSearchOnboardHandler was delete before UI_NotifyProgress, the app will crash
        m_UITaskQueue->AddTask(new OffboardSingleSearchUITaskProgress(this, &SingleSearchOffboardHandler::UI_NotifyProgress, percentage, m_state));
    }
}


//SingleSearchAsyncCallback
SingleSearchAsyncCallback::SingleSearchAsyncCallback(SingleSearchOffboardHandler* offboardHandler)
    : m_handler(offboardHandler)
{
}

SingleSearchAsyncCallback::~SingleSearchAsyncCallback()
{
    m_handler = NULL;
}

void SingleSearchAsyncCallback::Success(SingleSearchSourceParametersSharedPtr request,
                                        SingleSearchSourceInformationSharedPtr response)
{
    if (m_handler)
    {
        m_handler->NotifySuccess(request, response);
    }
}
void SingleSearchAsyncCallback::Error(SingleSearchSourceParametersSharedPtr request, NB_Error error)
{
    if (m_handler)
    {
        m_handler->NotifyError(error);
    }
}

bool SingleSearchAsyncCallback::Progress(int percentage)
{
    if (m_handler)
    {
        m_handler->NotifyProgress(percentage);
    }
    return true;
}

void SingleSearchAsyncCallback::Reset()
{
}

/*! @} */
