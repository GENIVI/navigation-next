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
    @file routeoptions.cpp
    @date 10/08/2014
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

#include "routeoptions.h"
namespace locationtoolkit
{
 /*! Creates a new instance of <code>RouteOptions</code>.  */
RouteOptions::RouteOptions(RouteType routeType, TransportationMode transportationMode,
                 quint32 avoid, QString pronunciationStyle /*= QString("")*/)
  : avoid(avoid),
    routeType(routeType),
    transportationMode(transportationMode)
{
    if (!pronunciationStyle.isEmpty())
    {
        this->pronunciationStyle = pronunciationStyle;
    }
    else
    {
        this->pronunciationStyle = "female-5-aac-v3";
    }
}

/*! Gets a set of features to avoid. */
quint32 RouteOptions::GetAvoidFeatures() const
{
    return avoid;
}
/*! Sets the features to avoid.*/
 void RouteOptions::SetAvoidFeatures(quint32 avoid)
{
    this->avoid = avoid;
}
/*! Gets the route type.  */
RouteType RouteOptions::GetRouteType() const
{
    return routeType;
}
/*! Sets the type of route.*/
void RouteOptions::SetRouteType(RouteType routeType)
{
    this->routeType = routeType;
}
/*! Gets the vehicle type for the route.*/
TransportationMode RouteOptions::GetTransportationMode() const
{
    return transportationMode;
}
/*! Sets the vehicle type for the route.*/
void RouteOptions::SetTransportationMode(TransportationMode transportationMode)
{
    this->transportationMode = transportationMode;
}
/*! Gets the pronunciation style*/
QString RouteOptions::GetPronunciationStyle() const
{
    return pronunciationStyle;
}
/*! Sets the pronunciation style*/
void RouteOptions::SetPronunciationStyle(QString style)
{
    this->pronunciationStyle = style;
}

bool RouteOptions::operator == ( const RouteOptions& optns ) const
{
    if(pronunciationStyle == optns.pronunciationStyle && avoid == optns.avoid &&
            transportationMode == optns.transportationMode && routeType == optns.routeType)
    {
        return true;
    }
    else
    {
        return false;
    }
}

}
