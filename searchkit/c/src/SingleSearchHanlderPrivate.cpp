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
    @file     SingleSearchHandlerPrivate.cpp
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

#include "SingleSearchHandlerPrivate.h"
#include "offboard/SingleSearchOffboardHandler.h"
#include "onboard/SingleSearchOnboardHandler.h"
#include <palstdlib.h>
#include "TpsSingleSearchSourceHandler.h"
#include "SingleSearchRequestPrivate.h"
#include "CommonFunction.h"
#include "HybridManager.h"

using namespace protocol;
using namespace nbmap;
using namespace nbsearch;

SingleSearchHandlerPrivate::SingleSearchHandlerPrivate(
        SingleSearchRequestPrivate *requestPrivate,
        NB_Context* context,
        const SingleSearchRequest* user,
        const std::shared_ptr<SingleSearchListener>& listener,
        const std::shared_ptr<SingleSearchParameters>& param,
        const std::shared_ptr<SingleSearchState>& state)
    : m_requestPrivate(requestPrivate),
      m_state(state)
{
    HybridMode mode = HybridManager::GetInstance(context)->GetMode();
    if (mode == HBM_OFFBOARD_ONLY && !param->m_onboardOnly) {
        m_handler = new SingleSearchOffboardHandler(this, context, user, listener, param, m_state);
    } else {
        m_handler = new SingleSearchOnboardHandler(this, context, user, listener, param, m_state);
    }
}

SingleSearchHandlerPrivate::~SingleSearchHandlerPrivate()
{
    m_requestPrivate = NULL;
    delete m_handler;
}

bool SingleSearchHandlerPrivate::IsRequestAlive()
{
    return m_state->isValid();
}

void SingleSearchHandlerPrivate::SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId)
{
    m_handler->SetInvocationContext(inputSource, invocationMethod, screenId);
}

NB_Error SingleSearchHandlerPrivate::Request(NB_Context* context)
{
    return m_handler->Request(context);
}

void SingleSearchHandlerPrivate::Cancel()
{
    m_handler->Cancel();
}

bool SingleSearchHandlerPrivate::IsInProgress()
{
    return m_handler->IsInProgress();
}
/*! @} */
