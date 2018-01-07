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

    @file     nbpositionprocessor.h

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef POSITIONPROCESSOR_H
#define POSITIONPROCESSOR_H

#include "nbnavigation.h"

/*!
    @addtogroup nbnavigation
    @{
*/

NB_DEC nb_boolean NB_PositionProcessorCheckFixQuality(NB_NavigationState* navigationState, const NB_GpsLocation* fix);


/*! Update the navigation state based on the specified fix

@param navigationState NB_NavigationState object representing the state of the current navigation session
@param location current GPS location information
@returns NB_Error
*/
NB_DEC NB_Error NB_PositionProcessorUpdate(NB_NavigationState* navigationState, NB_GpsLocation* location);

/*! Find the position on the route closest to a point

@param context The current NB_Context
@param navigationState NB_NavigationState object representing the state of the current navigation session
@param route NB_RouteInformation object for the route
@param point NB_LatitudeLongitude for the point
@param closestManeuver the index of the maneuver for the position closest to the point
@param closestSegment the index of the segment (within the closest maneuver) closest to the point
@param distanceToRoute the distance from the point to the closest route position (in meters)
*/
NB_DEC NB_Error NB_PositionProcessorFindClosestPositionOnRoute(NB_Context* context, NB_NavigationState* navigationState, NB_RouteInformation* route, NB_LatitudeLongitude* point, uint32* closestManeuver, uint32* closestSegment, double* distanceToRoute);




/*! @} */

#endif
