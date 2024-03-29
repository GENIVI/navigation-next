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
    @file routesummaryrequestimpl.cpp
    @date 11/11/2014
    @addtogroup navigationkit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

#include "routesummaryrequest.h"
#include "routesummaryrequestimpl.h"
#include "signal/routesummarysignals.h"
#include "NavApiRouteSummaryRequest.h"
#include "ltkcontextimpl.h"
#include "NavApiTypes.h"
#include "internal/typetranslater.h"

namespace locationtoolkit
{
RouteSummaryRequestImpl::RouteSummaryRequestImpl(NB_Context* context,
                                                 const nbnav::Place origin,
                                                 const std::vector<nbnav::Place>& destination,
                                                 const QString& workFolder):
                        nbnav::RouteSummaryRequest(context, origin, destination, &mRouteSummeryHelperListener, workFolder.toStdString())
{
    mRouteSummeryHelperListener.SetRouteSummaryRequest(this);
}

RouteSummaryRequestImpl::~RouteSummaryRequestImpl()
{
}

void RouteSummaryRequestImpl::StartRequest()
{
    nbnav::RouteSummaryRequest::StartRequest();
}

bool RouteSummaryRequestImpl::IsRequestInProgress()
{
    return nbnav::RouteSummaryRequest::IsRequestInProgress();
}

void RouteSummaryRequestImpl::CancelRequest()
{
    nbnav::RouteSummaryRequest::CancelRequest();
}

const RouteSummarySignals& RouteSummaryRequestImpl::GetSignals() const
{
    return mRouteSummeryHelperListener.GetSignals();
}

RouteSummaryRequest* RouteSummaryRequest::CreateInstance(LTKContext& ltkcontext, const Place& origin, const Place& destination, const QString& workFolder)
{
    QVector<Place> places;
    places.push_back(destination);
    return RouteSummaryRequest::CreateInstance(ltkcontext, origin, places, workFolder);
}

RouteSummaryRequest* RouteSummaryRequest::CreateInstance(LTKContext& ltkcontext, const Place& origin, const QVector<Place>& destination, const QString& workFolder)
{
    nbnav::Place nbOrigin;
    TypeTranslater::Translate(origin, nbOrigin);

    std::vector<nbnav::Place> destinations;
    for(int i = 0; i < destination.size(); i++)
    {
        nbnav::Place nbDest;
        TypeTranslater::Translate(destination[i], nbDest);
        destinations.push_back(nbDest);
    }

    LTKContextImpl& contextImpl = static_cast<LTKContextImpl &>( ltkcontext );
    RouteSummaryRequest *pRouteSummaryRequest = new RouteSummaryRequestImpl(contextImpl.GetNBContext(), nbOrigin, destinations, workFolder);
    return pRouteSummaryRequest;
}

}
