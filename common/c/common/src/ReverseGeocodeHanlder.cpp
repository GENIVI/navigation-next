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
    @file     ReverseGeocodeHandler.cpp
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

#include "ReverseGeocodeHandler.h"
#include "palstdlib.h"
#include "TpsReverseGeocodeHandler.h"
#include "ReverseGeocodeRequestPrivate.h"
#include "CommonFunction.h"

using namespace protocol;
using namespace nbmap;
using namespace nbcommon;

ReverseGeocodeHandler::ReverseGeocodeHandler(shared_ptr<ReverseGeocodeAsyncCallback> handlerCallback)
    : m_reverseGeocodeAsyncCallback(handlerCallback), m_param(NULL)
{
}

ReverseGeocodeHandler::~ReverseGeocodeHandler()
{
    if (m_handler != NULL)
    {
        m_handler.reset();
    }
}

void ReverseGeocodeHandler::SetInvocationContext(NB_Context* context, const char* inputSource, const char* invocationMethod, const char* screenId)
{
    if (!context)
    {
        return;
    }
    shared_ptr<protocol::InvocationContext> invocationContext = GetInvocationConext("reverse-geocode", inputSource, invocationMethod, screenId);
    if (invocationContext)
    {
        if (m_handler == NULL)
        {
            TpsAtlasbookProtocol tpsAtlasbookProtocol(context);
            m_handler = tpsAtlasbookProtocol.GetReverseGeocodeHandler();
            if (!m_handler)
            {
                return;
            }
        }
        static_cast<OffBoardReverseGeocodeHandler*>(m_handler.get())->SetInvocationContext(invocationContext);
    }
}

NB_Error ReverseGeocodeHandler::Request(NB_Context* context)
{
    if (m_sparam == NULL)
    {
        return NE_BADDATA;
    }
    NB_Error err = NE_OK;

    if (m_handler == NULL)
    {
        TpsAtlasbookProtocol tpsAtlasbookProtocol(context);
        m_handler = tpsAtlasbookProtocol.GetReverseGeocodeHandler();
        if (!m_handler)
        {
            return NE_NOMEM;
        }
    }

    m_handler->Request(m_sparam->CreateParameters(), m_reverseGeocodeAsyncCallback);

    return err;
}

void ReverseGeocodeHandler::Cancel()
{
    if (m_handler)
    {
        m_handler->Cancel();
    }
}

bool ReverseGeocodeHandler::IsInProgress()
{
    return static_cast<OffBoardReverseGeocodeHandler*>(m_handler.get())->IsInProgress();
}

void ReverseGeocodeHandler::SetParameter(const::shared_ptr<ReverseGeocodeParameter>& sparam)
{
    m_sparam = sparam;
}
/*! @} */
