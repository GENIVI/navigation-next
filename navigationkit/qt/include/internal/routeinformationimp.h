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
    @file routeinformationimp.h
    @date 10/15/2014
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
#ifndef LOCATIONTOOLKIT_ROUTE_INFORMATION_IMP_H
#define LOCATIONTOOLKIT_ROUTE_INFORMATION_IMP_H

#include "NavApiRouteInformation.h"
#include "data/routeinformation.h"
#include "internal/maneuverlistimp.h"

namespace locationtoolkit
{
class RouteInformationImpl: public RouteInformation
{
public:
    RouteInformationImpl(const nbnav::RouteInformation &routinfo);

    RouteInformationImpl();

    virtual ~RouteInformationImpl();

    virtual const ManeuverList* GetManeuverList() const;

    virtual const QString& GetRouteID() const;

    virtual qreal GetDistance() const;

    virtual quint32 GetTime() const;

    virtual quint32 GetDelay() const;

    virtual LTKError GetRouteError() const;

    virtual const Place& GetOrigin() const;

    virtual const Place& GetDestination() const;

    virtual const BoundingBox& GetBoundingBox() const;

    virtual const QVector<Coordinates*>& GetPolyline() const;

    virtual const QString& GetInitialGuidanceText() const;

    virtual const QString& GetRouteDescriptions() const;

    virtual const QVector<RouteProperty>& GetRouteProperties() const;

    virtual const TrafficEvent* GetTrafficEvent(quint32 index) const;

    virtual quint32 GetNumberOfTrafficEvent() const;

    virtual char GetTrafficColor() const;

    virtual const QString& GetStartupGuidanceText() const;

    virtual DestinationStreetSide GetDestinationStreetSide() const;

    virtual const Coordinates& GetLabelPoint() const;

    virtual const QString& GetFirstMajorRoad() const;
private:
    ManeuverList*              mManeuverList;
    QString                    mRouteID;
    qreal                      mDistance;
    quint32                    mTime;
    quint32                    mDelay;
    LTKError                   mRouteError;
    Place                      mOrigin;
    Place                      mDestination;
    BoundingBox                mBoundingBox;
    QVector<Coordinates*>      mRoutePolylines;
    QVector<RouteInformation::RouteProperty>     mRouteProperties;
    QString                    mInitialInstruction;
    QString                    mStartupInstruction;
    QString                    mDescription;
    QVector<TrafficEvent*>     mTrafficEvents;
    char                       mTrafficColor;
    DestinationStreetSide      mStreetSide;
    Coordinates                mLabelPoint;
    QString                    mFirstMajorRoad;
};
}
#endif
