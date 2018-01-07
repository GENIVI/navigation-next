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

#include "searchrequestimpl.h"
#include "searchutil.h"

using namespace locationtoolkit;

SearchRequestImpl::SearchRequestImpl(NB_Context* nbContext, const QString& keyword, const QString& category,
                              const Coordinates& current, const Coordinates& search,
                              const nbsearch::SingleSearchConfiguration& config)
    :SearchRequest()
    ,mState(new RequestState())
    ,mRequest(NULL)
    ,mListener(NULL)
{
    mListener = std::make_shared<SingleSearchListenerImpl>(this, mState);
    nbsearch::LatLonPoint currentLocation(current.latitude, current.longitude);
    nbsearch::LatLonPoint searchLocation(search.latitude, search.longitude);
    mRequest = std::make_shared<nbsearch::SingleSearchRequest>(nbContext, keyword.toStdString().c_str(), category.toStdString().c_str(),
                                                 &currentLocation, &searchLocation, mListener, config);
}

SearchRequestImpl::SearchRequestImpl(NB_Context *nbContext, const SearchFilter &filter,
                              const Coordinates &current, const Coordinates &search,
                              const nbsearch::SingleSearchConfiguration &config)
    :SearchRequest()
    ,mState(new RequestState())
    ,mRequest(NULL)
    ,mListener(NULL)
{
    mListener = std::make_shared<SingleSearchListenerImpl>(this, mState);
    nbsearch::LatLonPoint currentLocation(current.latitude, current.longitude);
    nbsearch::LatLonPoint searchLocation(search.latitude, search.longitude);
    nbsearch::SearchFilter* filterImpl = SearchTranslator::toSearchFilter(filter);
    mRequest = std::make_shared<nbsearch::SingleSearchRequest>(nbContext, filterImpl, &currentLocation, &searchLocation,
                                                 mListener, config);
    delete filterImpl;
}

SearchRequestImpl::SearchRequestImpl(NB_Context *nbContext, const nbcommon::StaticPoi& staticPOI,
                              const Coordinates &current, const Coordinates &search,
                              const nbsearch::SingleSearchConfiguration &config)
    :SearchRequest()
    ,mState(new RequestState())
    ,mRequest(NULL)
    ,mListener(NULL)
{
    mListener = std::make_shared<SingleSearchListenerImpl>(this, mState);
    nbsearch::LatLonPoint currentLocation(current.latitude, current.longitude);
    nbsearch::LatLonPoint searchLocation(search.latitude, search.longitude);
    mRequest = std::make_shared<nbsearch::SingleSearchRequest>(nbContext, &staticPOI, &currentLocation, &searchLocation,
                                                 mListener, config);
}

SearchRequestImpl::SearchRequestImpl(NB_Context *nbContext, const SearchLocation& location,
                              const nbsearch::SingleSearchConfiguration &config)
    :SearchRequest()
    ,mState(new RequestState())
    ,mRequest(NULL)
    ,mListener(NULL)
{
    mListener = std::make_shared<SingleSearchListenerImpl>(this, mState);
    nbsearch::Location* locationImpl = SearchTranslator::toLocation(location);
    mRequest = std::make_shared<nbsearch::SingleSearchRequest>(nbContext, locationImpl, mListener, config);
    delete locationImpl;
}\

SearchRequestImpl::SearchRequestImpl(NB_Context *nbContext, SearchRequestImpl *request, const SearchInformation &information, const SearchDirection &direction)
    :SearchRequest()
    ,mState(new RequestState())
    ,mRequest(NULL)
    ,mListener(NULL)
{
    mListener = std::make_shared<SingleSearchListenerImpl>(this, mState);
    nbsearch::SearchDirection nbDirection;
    switch(direction)
    {
    case SD_Start:
        nbDirection = nbsearch::SD_Start;
        break;
    case SD_Prev:
        nbDirection = nbsearch::SD_Prev;
        break;
    case SD_Next:
        nbDirection = nbsearch::SD_Next;
        break;
    default:
        break;
    }

    SingelSearchInformationImpl* informationImpl = new SingelSearchInformationImpl(information);
    mRequest = std::make_shared<nbsearch::SingleSearchRequest>(nbContext, request->mRequest.get(), informationImpl, mListener, nbDirection);
    delete informationImpl;
}

SearchRequestImpl::~SearchRequestImpl()
{
    mState->valid = false;
}

LTKError
SearchRequestImpl::StartRequest()
{
    if(mRequest)
    {
        mRequest->StartRequest();
        return LTKError(LTKError::LTK_OK);
    }
    return LTKError(LTKError::LTK_ErrNoInit);
}

LTKError
SearchRequestImpl::CancelRequest()
{
    if(mRequest)
    {
        mRequest->CancelRequest();
        return LTKError(LTKError::LTK_OK);
    }
    return LTKError(LTKError::LTK_ErrNoInit);
}

bool
SearchRequestImpl::IsRequestInProgress()
{
    if(mRequest)
    {
        return mRequest->IsRequestInProgress();
    }
    return false;
}

SearchRequestImpl::SingleSearchListenerImpl::SingleSearchListenerImpl(SearchRequestImpl *requestImpl, const std::shared_ptr<RequestState>& state)
    :SingleSearchListener()
    , mRequestImpl(requestImpl)
    , mRequestState(state)
{
}

SearchRequestImpl::SingleSearchListenerImpl::~SingleSearchListenerImpl()
{
}

void
SearchRequestImpl::SingleSearchListenerImpl::OnSuccess(const nbsearch::SingleSearchRequest &request, const nbsearch::SingleSearchInformation &information)
{
    if(mRequestState->valid && mRequestImpl)
    {
        QSharedPointer<SearchInformation> searchInfo(SearchTranslator::toSearchInformation(information));
        emit mRequestImpl->OnSuccess(searchInfo);
    }
}

void
SearchRequestImpl::SingleSearchListenerImpl::OnError(const nbsearch::SingleSearchRequest &request, NB_Error error)
{
    if(mRequestState->valid && mRequestImpl)
    {
        emit mRequestImpl->OnError((int) error);
    }
}

void
SearchRequestImpl::SingleSearchListenerImpl::OnProgressUpdated(const nbsearch::SingleSearchRequest &request, int percentage)
{
    if(mRequestState->valid && mRequestImpl)
    {
        emit mRequestImpl->OnProgressUpdated(percentage);
    }
}


