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

/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "searchreversegeocoderequestimpl.h"
#include "searchutil.h"

using namespace locationtoolkit;

SearchReverseGeocodeRequestImpl::SearchReverseGeocodeRequestImpl(NB_Context* nbContext, const nbcommon::ReverseGeocodeConfiguration& config,
                                                                 const Coordinates& coordinates)
    :SearchRequest()
    ,mRequest(NULL)
    ,mListener(NULL)
    ,mState(NULL)
{
    mState = std::make_shared<RequestState>();
    mListener = std::make_shared< ReverseGeocodeListenerImpl>(this, mState);
    nbcommon::LatitudeLongitude latitudelongitude(coordinates.latitude, coordinates.longitude);
    mRequest = std::make_shared<nbcommon::ReverseGeocodeRequest>(nbContext, &latitudelongitude, mListener, config);
}

SearchReverseGeocodeRequestImpl::~SearchReverseGeocodeRequestImpl()
{
    mState->valid = false;
}

LTKError SearchReverseGeocodeRequestImpl::StartRequest()
{
    if(mRequest)
    {
        mRequest->StartRequest();
        return LTKError(LTKError::LTK_OK);
    }
    return LTKError(LTKError::LTK_ErrNoInit);
}

LTKError SearchReverseGeocodeRequestImpl::CancelRequest()
{
    if(mRequest)
    {
        mRequest->CancelRequest();
        return LTKError(LTKError::LTK_OK);
    }
    return LTKError(LTKError::LTK_ErrNoInit);
}

bool SearchReverseGeocodeRequestImpl::IsRequestInProgress()
{
    if(mRequest)
    {
        return mRequest->IsRequestInProgress();
    }
    return false;
}

SearchReverseGeocodeRequestImpl::ReverseGeocodeListenerImpl::ReverseGeocodeListenerImpl(SearchReverseGeocodeRequestImpl *requestImpl, const std::shared_ptr<RequestState>& state)
    :ReverseGeocodeListener()
    ,mRequestState(state)
    ,mRequestImpl(requestImpl)
{
}

SearchReverseGeocodeRequestImpl::ReverseGeocodeListenerImpl::~ReverseGeocodeListenerImpl()
{
}

void SearchReverseGeocodeRequestImpl::ReverseGeocodeListenerImpl::OnSuccess(const nbcommon::ReverseGeocodeRequest &request, const nbcommon::ReverseGeocodeInformation &information)
{
    if(mRequestState->valid && mRequestImpl)
    {
        QSharedPointer<SearchInformation> searchInfo(SearchTranslator::toSearchInformation(information));
        emit mRequestImpl->OnSuccess(searchInfo);
    }
}

void SearchReverseGeocodeRequestImpl::ReverseGeocodeListenerImpl::OnError(const nbcommon::ReverseGeocodeRequest &request, NB_Error error)
{
    if(mRequestState->valid && mRequestImpl)
    {
        emit mRequestImpl->OnError((int) error);
    }
}

void SearchReverseGeocodeRequestImpl::ReverseGeocodeListenerImpl::OnProgressUpdated(const nbcommon::ReverseGeocodeRequest &request, int percentage)
{
    if(mRequestState->valid && mRequestImpl)
    {
        emit mRequestImpl->OnProgressUpdated(percentage);
    }
}
