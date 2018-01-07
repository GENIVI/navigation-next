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

 @file sessionhelperlistener.cpp
 @date 09/22/2014
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
 *
 */
#include "internal/sessionhelperlistener.h"
#include "internal/routeinformationimp.h"
#include "internal/typetranslater.h"

#include "navigationtypes.h"

namespace locationtoolkit
{
SessionHelperListener::SessionHelperListener(QObject* parent): QObject(parent),mSessionSignals(parent)
{
}

SessionHelperListener::~SessionHelperListener()
{

}

void SessionHelperListener::OffRoute()
{
    emit mSessionSignals.OffRoute();
}

void SessionHelperListener::OnRoute()
{
    emit mSessionSignals.OnRoute();
}

void SessionHelperListener::RouteReceived(nbnav::SessionListener::RouteRequestReason reason,
                                          vector<nbnav::RouteInformation>& routes)
{
    mNbNavRoutes.clear();

    QVector< QSharedPointer<RouteInformation> > v;

    for(vector<nbnav::RouteInformation>::iterator it = routes.begin();
        it != routes.end(); it++)
    {
        QSharedPointer<RouteInformation> info(new RouteInformationImpl(*it));
        v.push_back(info);
        mNbNavRoutes.push_back(*it);
    }
    emit mSessionSignals.RouteReceived((locationtoolkit::RouteRequestReason)reason, v);
}

void SessionHelperListener::RouteRequested(nbnav::SessionListener::RouteRequestReason reason)
{
    emit mSessionSignals.RouteRequested((locationtoolkit::RouteRequestReason)reason);
}

void SessionHelperListener::RouteProgress(int progress)
{
    emit mSessionSignals.RouteProgress((qint32)progress);
}

void SessionHelperListener::RouteError(nbnav::NavigateRouteError nbError)
{
    LTKError ltkError;
    TypeTranslater::Translate(nbError, ltkError);
    emit mSessionSignals.RouteError(ltkError);
}

void SessionHelperListener::ArrivingToDestination(nbnav::DestinationStreetSide streetSide)
{
    emit mSessionSignals.ArrivingToDestination((DestinationStreetSide)streetSide);
}

void SessionHelperListener::RouteFinish()
{
    emit mSessionSignals.RouteFinish();
}

void SessionHelperListener::OffRoutePositionUpdate(double headingToRoute)
{
    emit mSessionSignals.OffRoutePositionUpdate((qreal)headingToRoute);
}

const SessionSignals& SessionHelperListener::GetSignals() const
{
    return mSessionSignals;
}

const nbnav::RouteInformation* SessionHelperListener::GetNbNavRouteInfomation(const std::string & RouteID)const
{
    vector<nbnav::RouteInformation>::const_iterator it = mNbNavRoutes.begin();
    for(;it != mNbNavRoutes.end(); it++)
    {
        if( it->GetRouteID() == RouteID )
        {
            return &(*it);
        }
    }
    return NULL;
}
}  // namespace locationtoolkit
