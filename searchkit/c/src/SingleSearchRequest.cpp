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
    @file     SingleSearchRequest.cpp
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

#include "SingleSearchRequest.h"
#include "SingleSearchHandler.h"
#include "SingleSearchParameters.h"
#include "SingleSearchRequestPrivate.h"
#include "SingleSearchInformation.h"

namespace nbsearch
{

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         const char* keyWord,
                                         const char* categoryCode,
                                         const LatLonPoint* currentPoint,
                                         const LatLonPoint* searchPoint,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<SingleSearchParametersByResultStyle>(configuration);

    m_param->SetOriginPoint(currentPoint->GetLatitude(), currentPoint->GetLongitude());
    m_param->SetSearchPoint(searchPoint->GetLatitude(), searchPoint->GetLongitude());
    if (keyWord)
        m_param->m_name = std::string(keyWord);
    if (categoryCode)
        m_param->m_categoryCode = std::string(categoryCode);
    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         const SearchFilter* searchFilter,
                                         const LatLonPoint* currentPoint,
                                         const LatLonPoint* searchPoint,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<SingleSearchParametersBySearchFilter>(configuration);

    m_param->SetOriginPoint(currentPoint->GetLatitude(), currentPoint->GetLongitude());
    m_param->SetSearchPoint(searchPoint->GetLatitude(), searchPoint->GetLongitude());
    m_param->SetSearchFilter(searchFilter);
    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         SingleSearchRequest* request,
                                         const SingleSearchInformation* information,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         SearchDirection direction)
{
    m_param = std::shared_ptr<SingleSearchParameters>(request->m_param->clone());
    m_param->SetIteratorCommand(direction, information->GetResultState()->GetSerialized());
    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         MovieShowingType showType,
                                         const LatLonPoint* currentPoint,
                                         const LatLonPoint* searchPoint,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<MovieSingleSearchParameters>(configuration, showType);

    m_param->SetOriginPoint(currentPoint->GetLatitude(), currentPoint->GetLongitude());
    m_param->SetSearchPoint(searchPoint->GetLatitude(), searchPoint->GetLongitude());

    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         SingleSearchResultIdType idType,
                                         const char* id,
                                         uint32 startTime,
                                         const LatLonPoint* currentPoint,
                                         const LatLonPoint* searchPoint,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<MovieSingleSearchParameters>(configuration, idType, id, startTime);

    m_param->SetOriginPoint(currentPoint->GetLatitude(), currentPoint->GetLongitude());
    m_param->SetSearchPoint(searchPoint->GetLatitude(), searchPoint->GetLongitude());

    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         const nbcommon::StaticPoi* staticPoi,
                                         const LatLonPoint* currentPoint,
                                         const LatLonPoint* searchPoint,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<StaticPoiSingleSearchParameters>(configuration, staticPoi->m_name, staticPoi->m_identify);

    m_param->SetOriginPoint(currentPoint->GetLatitude(), currentPoint->GetLongitude());
    m_param->SetSearchPoint(searchPoint->GetLatitude(), searchPoint->GetLongitude());

    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         const BoundingBox& box,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<WeatherSearchParameters>(configuration);
    m_param->m_leftTopPoint.SetLatitude(box.GetTopLeft().GetLatitude());
    m_param->m_leftTopPoint.SetLongitude(box.GetTopLeft().GetLongitude());
    m_param->m_rightBottomPoint.SetLatitude(box.GetBottomRight().GetLatitude());
    m_param->m_rightBottomPoint.SetLongitude(box.GetBottomRight().GetLongitude());
    m_param->m_type = configuration.m_weatherType;

    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         const SearchFilter* searchFilter,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    m_param = std::make_shared<WeatherSearchParameters>(configuration);

    m_param->m_type = configuration.m_weatherType;
    m_param->SetSearchFilter(searchFilter);

    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}

SingleSearchRequest::SingleSearchRequest(NB_Context* context,
                                         const nbsearch::Location* location,
                                         const std::shared_ptr<SingleSearchListener>& listener,
                                         const SingleSearchConfiguration& configuration)
{
    AddressSearchParameters* param = new AddressSearchParameters(configuration);
    ((AddressSearchParameters*)param)->SetSearchAddress(location);
    m_param = std::shared_ptr<SingleSearchParameters>(param);

    m_privateRequest = SingleSearchRequestPrivate::NewInstance(context, this, listener, m_param);
}


SingleSearchRequest::~SingleSearchRequest()
{
    m_privateRequest->Destroy();
}

void SingleSearchRequest::SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId)
{
    if (m_privateRequest)
    {
        m_privateRequest->SetInvocationContext(inputSource, invocationMethod, screenId);
    }
}

void SingleSearchRequest::StartRequest()
{
    m_privateRequest->Request();
}

void SingleSearchRequest::CancelRequest()
{
    m_privateRequest->Cancel();
}

bool SingleSearchRequest::IsRequestInProgress()
{
    return m_privateRequest->IsRequestInProgress();
}

}
/*! @} */
