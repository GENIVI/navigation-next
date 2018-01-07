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

#ifndef SEARCHREVERSEGEOCODEREQUESTIMPL_H
#define SEARCHREVERSEGEOCODEREQUESTIMPL_H

#include "searchreversegeocoderequest.h"
#include "ReverseGeocodeConfiguration.h"
#include "ReverseGeocodeInformation.h"
#include "ReverseGeocodeListener.h"
#include "ReverseGeocodeRequest.h"

namespace locationtoolkit
{

class SearchReverseGeocodeRequestImpl : public SearchRequest
{
public:
    SearchReverseGeocodeRequestImpl(NB_Context* nbContext, const nbcommon::ReverseGeocodeConfiguration& config,
                                    const Coordinates& coordinates);
    virtual ~SearchReverseGeocodeRequestImpl();

public:
    virtual LTKError StartRequest();
    virtual LTKError CancelRequest();
    virtual bool IsRequestInProgress();

private:
    struct RequestState {
        RequestState():valid(true){}
        bool valid;
    };

    class ReverseGeocodeListenerImpl : public nbcommon::ReverseGeocodeListener
    {
    public:
        ReverseGeocodeListenerImpl(SearchReverseGeocodeRequestImpl* requestImpl, const std::shared_ptr<RequestState>& state);
        virtual ~ReverseGeocodeListenerImpl();

    public:
        virtual void OnSuccess(const nbcommon::ReverseGeocodeRequest& request,
                               const nbcommon::ReverseGeocodeInformation& information);
        virtual void OnError(const nbcommon::ReverseGeocodeRequest& request, NB_Error error);
        virtual void OnProgressUpdated(const nbcommon::ReverseGeocodeRequest& request, int percentage);

    private:
        std::shared_ptr<RequestState> mRequestState;
        SearchReverseGeocodeRequestImpl* mRequestImpl;
    };

private:
    std::shared_ptr<RequestState> mState;
    std::shared_ptr<nbcommon::ReverseGeocodeRequest> mRequest;
    std::shared_ptr<nbcommon::ReverseGeocodeListener> mListener;
};

}
#endif // SEARCHREVERSEGEOCODEREQUESTIMPL_H

/*! @} */
