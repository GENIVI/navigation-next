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

    @file     nbutil.h
*/
/*
    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBUTILITY_H
#define	NBUTILITY_H

#include "paltypes.h"
#include "nblocation.h"
#include "nbplace.h"
#include "nbextappcontent.h"
#include "navpublictypes.h"
#include "data_place.h"
#include "data_locmatch.h"
#include "data_location.h"
#include "data_phone.h"
#include "data_category.h"
#include "data_origin.h"
#include "data_destination.h"
#include "data_route_style.h"
#include "data_nav_query.h"
#include "data_fuelproduct.h"
#include "data_fueltype.h"
#include "data_price.h"
#include "data_weather_conditions.h"
#include "data_weather_forecast.h"
#include "data_event.h"
#include "data_event_content.h"
#include "data_event_performance.h"
#include "data_proxmatch_content.h"
#include "data_proxmatch_summary.h"
#include "data_search_filter.h"
#include "data_proxpoi_reply.h"
#include "data_hours_of_operation.h"
#include "data_extapp_content.h"
#include "navtypes.h"
#include "nbutilityprotected.h"               /* utility functions shared with AB services */
#include "nbroutesettings.h"
#include "nbimage.h"
#include "data_image.h"

/*! @{ */

boolean	NIMLocationValid(NB_Location* pLocation);
void	ClearNIMLocation(NB_Location* pLocation);
void	ClearLocationIfGPSPlace(NB_Place* pPlace);
void	ClearNIMFuelProduct(NB_FuelProduct* pFuelProduct);
void	ClearNIMTrafficIncidentDetails(NB_TrafficIncident* pIncident);

NB_DEC NB_Error SetNIMPlaceFromPlace(NB_Place* pPlace, data_util_state* pDUS, data_place* pDataPlace);
NB_Error SetNIMFuelProdFromFuelProd(NB_FuelProduct* pFuelProduct, data_util_state* pDUS, data_fuelproduct* pDataFuelProd);
NB_Error SetNIMTrafficIncidentDetailsFromTrafficIncident(NB_TrafficIncident* pIncident, data_util_state* pDUS, data_traffic_incident* pDataTrafficIncident);
NB_Error SetNIMWeatherConditionsFromWeatherConditions(NB_WeatherConditions* pWeatherConditions, data_util_state* pDUS, data_weather_conditions* pDataWeatherConditions);
NB_Error SetNIMWeatherForecastFromWeatherForecast(NB_WeatherForecast* pWeatherforecast, data_util_state* pDUS, data_weather_forecast* pDataWeatherForecast);
NB_Error SetNIMSummaryResultFromSummaryResult(NB_SummaryResult* pSummaryResult, data_util_state* pDUS, data_proxmatch_summary* pDataSummary);

NB_Error CreatePublicTransitStopsFromVector(NB_PublicTransitStopInfo** pPublicTransitStops, int* publicTransitStopCount, data_util_state* pDUS, struct CSL_Vector* transitpoiVector);
NB_PublicTransitStopType ConvertPublicTransitRouteType(const char *routeType);

NB_Error CreateNIMEventsFromVector(NB_Event** eventArray, int* eventCount, data_util_state* pDUS, struct CSL_Vector* eventVector);
NB_Error CreateNIMEventFromContent(NB_Event** event, data_util_state* pDUS, data_event_content* content);
void FreeNIMEvents(NB_Event* eventArray, int eventCount, data_util_state* pDUS);

NB_Error CreateNIMExtendedPlace(data_util_state* pDUS, data_proxmatch* pproxmatch, NB_ExtendedPlace** extendedPlace);
void FreeNIMExtendedPlace(data_util_state* pDUS, NB_ExtendedPlace* extendedPlace);
NB_Error NB_ExtendedPlaceClone(data_util_state* dataState, NB_ExtendedPlace* sourceExtendedPlace, NB_ExtendedPlace** cloneExtendedPlace);

data_search_filter* NIMCloneSearchFilter(data_util_state* dataState, data_search_filter* searchFilter);
void NIMFreeSearchFilter(data_util_state* dataState, data_search_filter* searchFilter);

boolean	is_server_error(NB_Error err);

int PlaceEqual(NB_Place*, NB_Place*);

NB_Error SetOriginFromPlace(data_util_state* pds, data_origin* pod, NB_Place* pPlace);
NB_Error SetDestinationFromPlace(data_util_state* pds, data_destination* pod, NB_Place* pPlace);

void	CopyPlace(NB_Place* pDest, NB_Place* pSource, boolean replaceemptyonly);

boolean PlaceHasPhoneNumber(NB_Place* place);

const char* GetImageFormatString(NB_ImageFormat format);
NB_ImageFormat GetImageFormat(const char* formatString);

/*! @} */

#endif
