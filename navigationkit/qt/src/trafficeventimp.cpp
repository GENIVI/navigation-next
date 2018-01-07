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
    @file trafficeventimp.cpp
    @date 10/13/2014
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
#include "internal/trafficeventimp.h"
#include <time.h>

namespace locationtoolkit
{
TrafficEventImp::TrafficEventImp(const nbnav::TrafficEvent& trafficEvent)
{
    mDelay = quint32(trafficEvent.GetDelay());
    mDescription = QString::fromStdString(trafficEvent.GetDescription());
    mDistanceToTrafficItem = qreal(trafficEvent.GetDistanceToTrafficItem());
    time_t tt = trafficEvent.GetEndTime();
    struct tm *et = localtime(&tt);
    mEndTime = QTime(et->tm_hour, et->tm_min, et->tm_sec);
    mManeuverNumber = quint32(trafficEvent.GetManeuverNumber());
    tt = trafficEvent.GetReportTime();
    struct tm *rt = localtime(&tt);
    mReportTime = QTime(rt->tm_hour, rt->tm_min, rt->tm_sec);
    mRoadName = QString::fromStdString(trafficEvent.GetRoad());
    mSeverity = TrafficSeverity(trafficEvent.GetSeverity());
    mStartFromTrip = qreal(trafficEvent.GetStartFromTrip());
    mLength = qreal(trafficEvent.GetLength());
    mType = Type(trafficEvent.GetType());
}

TrafficEventImp::TrafficEventImp()
{
}

TrafficEventImp::~TrafficEventImp()
{
}

quint32 TrafficEventImp::GetDelay() const
{
    return mDelay;
}

const QString& TrafficEventImp::GetDescription() const
{
    return mDescription;
}

const QTime& TrafficEventImp::GetEndTime() const
{
    return mEndTime;
}

qreal TrafficEventImp::GetDistanceToTrafficItem() const
{
    return mDistanceToTrafficItem;
}

qreal TrafficEventImp::GetLength() const
{
    return mLength;
}

quint32 TrafficEventImp::GetManeuverNumber() const
{
    return mManeuverNumber;
}

const QTime& TrafficEventImp::GetReportTime() const
{
    return mReportTime;
}

const QString& TrafficEventImp::GetRoadName() const
{
    return mRoadName;
}

TrafficEvent::TrafficSeverity TrafficEventImp::GetSeverity() const
{
    return mSeverity;
}

qreal TrafficEventImp::GetStartFromTrip() const
{
    return mStartFromTrip;
}

TrafficEvent::Type TrafficEventImp::GetType() const
{
    return mType;
}
}
