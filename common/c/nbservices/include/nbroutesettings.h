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

    @file     nbroutesettings.h
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/


#ifndef NBROUTESETTINGS_H
#define NBROUTESETTINGS_H

/*! @{ */

typedef enum
{
	NB_RouteType_Fastest    = 0,
	NB_RouteType_Shortest,
	NB_RouteType_Easiest
} NB_RouteType;

typedef enum
{
    NB_RouteAvoid_None      = 0,
	NB_RouteAvoid_HOV       = 1<<0,
	NB_RouteAvoid_Toll      = 1<<1,
	NB_RouteAvoid_Highway   = 1<<2,
	NB_RouteAvoid_UTurn     = 1<<3,
	NB_RouteAvoid_Unpaved   = 1<<4,
	NB_RouteAvoid_Ferry     = 1<<5
} NB_RouteAvoid;

typedef enum
{
	NB_TransportationMode_Car   = 0,
	NB_TransportationMode_Truck,
	NB_TransportationMode_Bicycle,
	NB_TransportationMode_Pedestrian,
	NB_TransportationMode_PublicTransit
} NB_TransportationMode;

typedef enum
{
	NB_TrafficType_None             = 0,
	NB_TrafficType_HistoricalRoute  = 1<<0,
	NB_TrafficType_RealTimeRoute    = 1<<1,
	NB_TrafficType_Incidents        = 1<<2,
	NB_TrafficType_Notification     = 1<<3,
	NB_TrafficType_HistoricalSpeed  = 1<<4,
	NB_TrafficType_RealTimeSpeed    = 1<<5,

	NB_TrafficType_UseSpeeds        = NB_TrafficType_HistoricalRoute | NB_TrafficType_RealTimeRoute | NB_TrafficType_HistoricalSpeed | NB_TrafficType_RealTimeSpeed,
	NB_TrafficType_Route            = NB_TrafficType_HistoricalRoute | NB_TrafficType_RealTimeRoute,

	NB_TrafficType_Warn             = NB_TrafficType_HistoricalSpeed | NB_TrafficType_RealTimeSpeed | NB_TrafficType_Incidents | NB_TrafficType_Notification,
	NB_TrafficType_Avoid            = NB_TrafficType_Warn | NB_TrafficType_HistoricalRoute | NB_TrafficType_RealTimeRoute,
	NB_TrafficType_Off              = NB_TrafficType_None,

	NB_TrafficType_Default          = NB_TrafficType_Warn
} NB_TrafficType;

typedef struct
{
	NB_RouteType            route_type;
	NB_TransportationMode   vehicle_type;
	NB_RouteAvoid           route_avoid;
	NB_TrafficType          traffic_type;
} NB_RouteSettings;

/*! @} */

#endif
