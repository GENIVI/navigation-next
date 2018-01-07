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
    @file routeinformationimp.cpp
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
#include "internal/routeinformationimp.h"
#include "internal/trafficeventimp.h"
#include "internal/typetranslater.h"
#include "data/routeinformation.h"
#include "data/trafficevent.h"

namespace locationtoolkit
{
RouteInformationImpl::RouteInformationImpl(const nbnav::RouteInformation &routeinfo)
{
    mManeuverList = new ManeuverListImpl(routeinfo.GetManeuverList());
    mRouteID = routeinfo.GetRouteID().c_str();
    mDistance = routeinfo.GetDistance();
    mTime = routeinfo.GetTime();
    mDelay = routeinfo.GetDelay();
    TypeTranslater::Translate(routeinfo.GetRouteError(), mRouteError);

    nbnav::Place origin = routeinfo.GetOrigin();
    TypeTranslater::Translate(origin, mOrigin);
    nbnav::Place destination = routeinfo.GetDestination();
    TypeTranslater::Translate(destination, mDestination);

    std::vector<nbnav::Coordinates> polyline =  routeinfo.GetPolyline();
    for(size_t i = 0; i < polyline.size(); i++)
    {
        mRoutePolylines.push_back(new Coordinates(polyline[i].latitude, polyline[i].longitude));
    }

    std::vector<nbnav::RouteProperty> properties = routeinfo.GetRouteProperties();
    for(size_t j = 0; j < properties.size(); j++)
    {
        RouteInformation::RouteProperty property = (RouteInformation::RouteProperty)properties[j];
        mRouteProperties.push_back(property);
    }

    mInitialInstruction = routeinfo.GetInitialGuidanceText().c_str();
    mStartupInstruction = routeinfo.GetStartupGuidanceText().c_str();
    mDescription = routeinfo.GetRouteDescriptions().c_str();

    unsigned int count = routeinfo.GetNumberOfTrafficEvent();
    for(size_t k = 0; k < count; k++)
    {
        const nbnav::TrafficEvent* trafficevent = routeinfo.GetTrafficEvent(k);
        if(trafficevent)
        {
            TrafficEvent *event = new TrafficEventImp(*trafficevent);
            if(event) mTrafficEvents.push_back(event);
        }
    }

    mTrafficColor = routeinfo.GetTrafficColor();
    mStreetSide = (DestinationStreetSide)routeinfo.GetDestinationStreetSide();
    nbnav::Coordinates coordinate = routeinfo.GetLabelPoint();
    mLabelPoint.latitude = coordinate.latitude;
    mLabelPoint.longitude = coordinate.longitude;
    mFirstMajorRoad = routeinfo.GetFirstMajorRoad().c_str();

    const nbnav::BoundingBox& boundingbox = routeinfo.GetBoundingBox();
    mBoundingBox.point1.latitude = boundingbox.point1.latitude;
    mBoundingBox.point1.longitude = boundingbox.point1.longitude;
    mBoundingBox.point2.latitude = boundingbox.point2.latitude;
    mBoundingBox.point2.longitude = boundingbox.point2.longitude;
}

RouteInformationImpl::RouteInformationImpl()
{

}

RouteInformationImpl::~RouteInformationImpl()
{
    QVector<Coordinates*>::iterator coord_it;
    QVector<TrafficEvent*>::iterator event_it;

    delete mManeuverList;

    for( coord_it = mRoutePolylines.begin(); coord_it != mRoutePolylines.end(); coord_it++)
    {
        delete (*coord_it);
    }
    mRoutePolylines.clear();

    for(event_it = mTrafficEvents.begin(); event_it != mTrafficEvents.end(); event_it++)
    {
        delete (*event_it);
    }
    mTrafficEvents.clear();
}

const ManeuverList* RouteInformationImpl::GetManeuverList() const
{
    return mManeuverList;
}

const QString& RouteInformationImpl::GetRouteID() const
{
    return mRouteID;
}

qreal RouteInformationImpl::GetDistance() const
{
    return mDistance;
}

quint32 RouteInformationImpl::GetTime() const
{
    return mTime;
}

quint32 RouteInformationImpl::GetDelay() const
{
    return mDelay;
}

LTKError RouteInformationImpl::GetRouteError() const
{
    return mRouteError;
}

const Place& RouteInformationImpl::GetOrigin() const
{
    return mOrigin;
}

const Place& RouteInformationImpl::GetDestination() const
{
    return mDestination;
}

const BoundingBox& RouteInformationImpl::GetBoundingBox() const
{
    return mBoundingBox;
}

const QVector<Coordinates*>& RouteInformationImpl::GetPolyline() const
{
    return mRoutePolylines;
}

const QString& RouteInformationImpl::GetInitialGuidanceText() const
{
    return mInitialInstruction;
}

const QString& RouteInformationImpl::GetRouteDescriptions() const
{
    return mDescription;
}

const QVector<RouteInformation::RouteProperty>& RouteInformationImpl::GetRouteProperties() const
{
    return mRouteProperties;
}

const TrafficEvent* RouteInformationImpl::GetTrafficEvent(quint32 index) const
{
    if(index < (quint32)mTrafficEvents.size())
    {
        return mTrafficEvents[index];
    }

    return NULL;
}

quint32 RouteInformationImpl::GetNumberOfTrafficEvent() const
{
    return (quint32)mTrafficEvents.size();
}

char RouteInformationImpl::GetTrafficColor() const
{
    return mTrafficColor;
}

const QString& RouteInformationImpl::GetStartupGuidanceText() const
{
    return mStartupInstruction;
}

DestinationStreetSide RouteInformationImpl::GetDestinationStreetSide() const
{
    return mStreetSide;
}

const Coordinates& RouteInformationImpl::GetLabelPoint() const
{
    return mLabelPoint;
}

const QString& RouteInformationImpl::GetFirstMajorRoad() const
{
    return mFirstMajorRoad;
}
}
