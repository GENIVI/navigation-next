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
    @file navuirouteoptions.cpp
    @date 10/24/2014
    @addtogroup navigationuikit
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

#include "navuirouteoptions.h"
namespace locationtoolkit
{
 /*! Creates a new instance of <code>RouteOptions</code>.  */
NavUIRouteOptions::NavUIRouteOptions(RouteType routeType, TransportationMode transportationMode,
                 quint32 avoid, QString pronunciationStyle /*= QString("")*/)
  : mAvoid(avoid),
    mRouteType(routeType),
    mTransportationMode(transportationMode)
{
    if (!pronunciationStyle.isEmpty())
    {
        mPronunciationStyle = pronunciationStyle;
    }
    else
    {
        mPronunciationStyle = "female-5-aac-v3";
    }
}

NavUIRouteOptions::NavUIRouteOptions()
   :mAvoid(0),
    mRouteType(RT_Fastest),
    mTransportationMode(TM_Car),
    mPronunciationStyle("female-5-aac-v3")
{

}

NavUIRouteOptions::~NavUIRouteOptions()
{

}

/*! Gets a set of features to avoid. */
quint32 NavUIRouteOptions::GetAvoidFeatures() const
{
    return mAvoid;
}
/*! Sets the features to avoid.*/
NavUIRouteOptions& NavUIRouteOptions::SetAvoidFeatures(quint32 avoid)
{
    mAvoid = avoid;
    return *this;
}
/*! Gets the route type.  */
NavUIRouteOptions::RouteType NavUIRouteOptions::GetRouteType() const
{
    return mRouteType;
}
/*! Sets the type of route.*/
NavUIRouteOptions& NavUIRouteOptions::SetRouteType(RouteType routeType)
{
    mRouteType = routeType;
    return *this;
}
/*! Gets the vehicle type for the route.*/
NavUIRouteOptions::TransportationMode NavUIRouteOptions::GetTransportationMode() const
{
    return mTransportationMode;
}
/*! Sets the vehicle type for the route.*/
NavUIRouteOptions& NavUIRouteOptions::SetTransportationMode(TransportationMode transportationMode)
{
    mTransportationMode = transportationMode;
    return *this;
}
/*! Gets the pronunciation style*/
QString NavUIRouteOptions::GetPronunciationStyle() const
{
    return mPronunciationStyle;
}
/*! Sets the pronunciation style*/
NavUIRouteOptions& NavUIRouteOptions::SetPronunciationStyle(QString style)
{
    mPronunciationStyle = style;
    return *this;
}

}
