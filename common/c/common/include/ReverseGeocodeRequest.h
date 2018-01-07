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
    @file     ReverseGeocodeRequest.h
    @defgroup nbcommon
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

#ifndef __REVERSEGEOCODEREQUEST_H__
#define __REVERSEGEOCODEREQUEST_H__

#include "nbcontext.h"
#include "CommonTypes.h"
#include <memory>
#include "smartpointer.h"

namespace nbcommon
{

class ReverseGeocodeRequestPrivate;
class ReverseGeocodeParameter;
class ReverseGeocodeConfiguration;
class ReverseGeocodeListener;

class ReverseGeocodeRequest
{
public:
    //@TODO: LatLonPoint need to be moved from search to common

    /*!
     * @brief Reverse geocode request.
     *
     * @param context
     * @param position
     * @param routeable
     * @param listener
     * @param config
     */
    ReverseGeocodeRequest(NB_Context* context,
                          const LatitudeLongitude* position,
                          const std::shared_ptr<ReverseGeocodeListener>& listener,
                          const ReverseGeocodeConfiguration& config);

    virtual ~ReverseGeocodeRequest();

    /*!
     * @brief Before StartRequest, user must set invocation context.
     *
     * @param inputSource  Specifies the source of the input content.
     * @param invocationMethod Specifies how the user initiated the request.
     * @param screenId  Specifies the user's screen.
     */
    void SetInvocationContext(const char* inputSource, const char* invocationMethod, const char* screenId);

    /*!
     * @brief Start the request.
     */
    void StartRequest();

    /*!
     * @brief Cancel the request.
     */
    void CancelRequest();

    /*!
     * @brief Checks whether request is in progress.
     *
     * @return true this request is in progress.
     */
    bool IsRequestInProgress();

private:
    ReverseGeocodeRequestPrivate* m_privateRequest;
    shared_ptr<ReverseGeocodeParameter> m_sparam;
};

} /* nbcommon */

#endif // __REVERSEGEOCODEREQUEST_H__

/*! @} */
