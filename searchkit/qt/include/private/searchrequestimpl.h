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

/*! @{ */

#ifndef SINGLESEARCHREQUESTIMPL_H
#define SINGLESEARCHREQUESTIMPL_H

#include "../include/singlesearchrequest.h"
#include "SingleSearchRequest.h"
#include "SingleSearchListener.h"

namespace locationtoolkit
{

class SearchRequestImpl : public SearchRequest
{
public:
    SearchRequestImpl(NB_Context* nbContext, const QString& keyword, const QString& category,
                   const Coordinates& current, const Coordinates& search,
                   const nbsearch::SingleSearchConfiguration& config);
    SearchRequestImpl(NB_Context *nbContext, const SearchFilter& filter,
                    const Coordinates &current, const Coordinates &search,
                    const nbsearch::SingleSearchConfiguration &config);
    SearchRequestImpl(NB_Context *nbContext, const nbcommon::StaticPoi& staticPOI,
                    const Coordinates &current, const Coordinates &search,
                    const nbsearch::SingleSearchConfiguration &config);
    SearchRequestImpl(NB_Context *nbContext, const SearchLocation& location,
                    const nbsearch::SingleSearchConfiguration &config);
    SearchRequestImpl(NB_Context* nbContext, SearchRequestImpl* request,
                    const SearchInformation& information, const SearchDirection& direction);
    virtual ~SearchRequestImpl();

public:
    virtual LTKError StartRequest();
    virtual LTKError CancelRequest();
    virtual bool IsRequestInProgress();

private:
    struct RequestState {
        RequestState():valid(true){}
        bool valid;
    };

    class SingleSearchListenerImpl : public nbsearch::SingleSearchListener
    {
    public:
        SingleSearchListenerImpl(SearchRequestImpl* requestImpl, const std::shared_ptr<RequestState>& state);
        virtual ~SingleSearchListenerImpl();

    public:
        virtual void OnSuccess(const nbsearch::SingleSearchRequest& request,
                               const nbsearch::SingleSearchInformation& information);
        virtual void OnError(const nbsearch::SingleSearchRequest& request, NB_Error error);
        virtual void OnProgressUpdated(const nbsearch::SingleSearchRequest& request, int percentage);

    private:
        SearchRequestImpl* mRequestImpl;
        std::shared_ptr<RequestState> mRequestState;
    };

private:
    std::shared_ptr<RequestState> mState;
    std::shared_ptr<nbsearch::SingleSearchRequest> mRequest;
    std::shared_ptr<nbsearch::SingleSearchListener> mListener;
};

}

#endif // SINGLESEARCHREQUESTIMPL_H

/*! @} */
