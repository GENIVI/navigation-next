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
#include "maneuverimp.h"
#include <vector>

namespace locationtoolkit
{
ManeuverImpl::ManeuverImpl(const nbnav::Maneuver& maneuver):mStackAdvise(false),
                                                           mIsDestination(false)
{
    mManeuverId = (qint32)maneuver.GetManeuverID();
    //get polyline
    const std::vector<nbnav::Coordinates>& polyline = maneuver.GetPolyline();
    for(size_t i = 0; i < polyline.size(); i++)
    {
        Coordinates *coordinate = new Coordinates();
        coordinate->latitude = polyline[i].latitude;
        coordinate->longitude = polyline[i].longitude;
        mPolyline.push_back(coordinate);
    }
    mRouteTTF = maneuver.GetRoutingTTF().c_str();
    mDistance = (qreal)maneuver.GetDistance();
    mCommand = maneuver.GetCommand().c_str();
    mPrimaryStreet = maneuver.GetPrimaryStreet().c_str();
    mSecondaryStreet = maneuver.GetSecondaryStreet().c_str();
    mTime = (qreal)maneuver.GetTime();
    nbnav::Coordinates point = maneuver.GetPoint();
    mPoint.latitude = point.latitude;
    mPoint.longitude = point.longitude;
    //FormattedTextBlock mMiManeuverText;
    //FormattedTextBlock mKmManeuverText;
    mMiDescription = maneuver.GetDescription(false, false).c_str();
    mKmDescription = maneuver.GetDescription(true, false).c_str();
    mTrafficDelay = (qreal)maneuver.GetTrafficDelay();
    mStackAdvise = maneuver.GetStackAdvise();
    mExitNumber = maneuver.GetExitNumber().c_str();
    mIsDestination = maneuver.IsDestination();
}

ManeuverImpl::~ManeuverImpl()
{
    QVector<Coordinates*>::iterator it;
    for(it = mPolyline.begin(); it != mPolyline.end(); )
    {
        delete (*it);
        it = mPolyline.erase(it);
    }
}

qint32 ManeuverImpl::GetManeuverID() const
{
    return mManeuverId;
}

const QVector<Coordinates*>& ManeuverImpl::GetPolyline() const
{
    return mPolyline;
}

const QString& ManeuverImpl::GetRoutingTTF() const
{
    return mRouteTTF;
}

qreal ManeuverImpl::GetDistance() const
{
    return mDistance;
}

const QString& ManeuverImpl::GetPrimaryStreet() const
{
    return mPrimaryStreet;
}

const QString& ManeuverImpl::GetSecondaryStreet() const
{
    return mSecondaryStreet;
}

qreal ManeuverImpl::GetTime() const
{
    return mTime;
}

const QString& ManeuverImpl::GetCommand() const
{
    return mCommand;
}

const Coordinates& ManeuverImpl::GetPoint() const
{
    return mPoint;
}

//FormattedTextBlock ManeuverImpl::GetManeuverText(QBool isMetric) const
//{
//    return (isMetric == true) ? mMiManeuverText : mKmManeuverText;
//}

const QString& ManeuverImpl::GetDescription(bool isMetric) const
{
    return (isMetric == false) ? mMiDescription : mKmDescription;
}

qreal ManeuverImpl::GetTrafficDelay() const
{
    return mTrafficDelay;
}

bool ManeuverImpl::GetStackAdvise() const
{
    return mStackAdvise;
}

const QString& ManeuverImpl::GetExitNumber() const
{
    return mExitNumber;
}

bool ManeuverImpl::IsDestination() const
{
    return mIsDestination;
}
}
