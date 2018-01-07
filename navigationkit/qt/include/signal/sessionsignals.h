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
    @file sessionsignals.h
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
---------------------------------------------------------------------------*/

/*! @{ */
#ifndef LOCATIONTOOLKIT_SESSION_SIGNALS_H
#define LOCATIONTOOLKIT_SESSION_SIGNALS_H


#include <QtGlobal>
#include <QObject>
#include <QVector>
#include <QSharedPointer>
#include "routeinformation.h"
#include "navigationtypes.h"
#include "ltkerror.h"

namespace locationtoolkit
{
/**
 * SessionSignals
 *
 * Provides the session related notifications like route updates and errors
 */
class SessionSignals: public QObject
{
    Q_OBJECT
public:
    ~SessionSignals(){}
Q_SIGNALS:
    /**
     * Emit when the locations being received by the navigation session are not considered on route to destination.
     * @return None
     */
    void OffRoute();

    /**
     * Emit when the locations being received by the navigation session are considered on route to destination.
     * @return None
     */
    void OnRoute();

    /**
     * Emit when received the list of possible routes for the current navigation session.
     *
     * More than one route will be passed only if the appropriate multiple routes preference is enabled.
     * The default route has zero index.
     * Client needs copy the vector if it wants to work with the classes
     * outside of slot which emitted by the signal.
     *
     * @param reason
     * @param routes array of {@link RouteInformation} representing the list of possible routes.
     * @return None
     */
    void RouteReceived(RouteRequestReason reason, const QVector< QSharedPointer<RouteInformation> >& routes);

    /**
     * Emit when new route requested
     * @param reason why new route is requested.
     * @return None
     */
    void RouteRequested(RouteRequestReason reason);

    /**
     * Emit when the route generation in progress
     * @param progress
     * @return None
     */
    void RouteProgress(qint32 progress);

    void RouteError(LTKError error);

    /**
     * Emit when the current navigation arriving to destination.
     *
     * @param streetSide The enum value of destination side.
     * @return None
     */
    void ArrivingToDestination(DestinationStreetSide streetSide);

    /**
     * Emit when the current navigation session ends. The client will not receive any route updates.
     * This navigation session needs to be stopped by the client.
     *
     * @return None
     */
    void RouteFinish();

    /**
     * Emit when the heading to origin off route.
     *
     * @param headingToRoute The heading of current place to origin.
     * @return None
     */
    void OffRoutePositionUpdate(qreal headingToRoute);
private:
    explicit SessionSignals(QObject* parent): QObject(parent){}
    friend class SessionHelperListener;
};
}  // namespace locationtoolkit
#endif
/*! @} */
