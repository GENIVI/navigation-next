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

    @file     nbanalyticstestprotected.h
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

#ifndef NBANALYTICSPROTECTED_H_
#define NBANALYTICSPROTECTED_H_

#include "nbanalytics.h"
#include "data_search_filter.h"
#include "nbsearchparameters.h"

/*!
    @addtogroup nbanalyticsprivate
    @{
*/

/*! @struct NB_AnalyticsWifiConfig
Analytics Wi-Fi Configuration from LBS server
*/
typedef struct NB_AnalyticsWifiConfig
{
    uint32  max_horizontal_uncertainty; /*!< Maximum horizontal uncertainty in meters allowed for the GPS fix
                                          associated with a set of Wi-Fi probes. */

    uint32  max_speed;                  /*!< Maximum allowed device speed in m/sec as reported in the GPS fix.
                                          Fixes with speed higher than this number shall be skipped. */

    uint32  collection_interval;        /*!< Minimum time in seconds allowed between two consecutive Wi-Fi probe
                                          collections. */

    uint32  minimum_distance_delta;     /*!< Minimum distance in meters from previous Wi-Fi probe collection point.
                                          If the distance is less than this number, then skip the collection until
                                          the next time slot. */
} NB_AnalyticsWifiConfig;

/*! Add search query event (protected API)

This function should be triggered by a local search request for the first page
of results.

@param context NB_Context
@param searchFilter data_search_filter object for the search
@param seachScheme scheme of the search
@param point Center of proximity search
@param searchInputMethod User input method of the search
@param searchEventCookie Information to be sent back to server when reporting an search event.
If provided, a copy of the cookie will be allocated (and later freed) for Analytics use.
@param pEventId upon success return of this function, the unique search query
       event ID will be stored at location pointed by pEventId. The unique
       search event ID should be passed to other related analytics search
       events so that those events can be associated with this search.
@returns NB_Error
*/

NB_DEC NB_Error
NB_AnalyticsAddSearchQueryEventProtected(
    NB_Context* context,
    data_search_filter* searchFilter,
    const char* searchScheme,
    NB_LatitudeLongitude* point,
    NB_SearchInputMethod searchInputMethod,
    NB_SearchEventCookie* searchEventCookie,
    uint32* pEventId);

/*! Returns boolean value - TRUE if Wifi probes allowed in the server config

@param context NB_Context
@returns nb_boolean
*/
NB_DEC nb_boolean NB_AnalyticsIsWifiProbesAllowed(NB_Context* context);

/*! Returns server wifi config

@param context NB_Context
@returns NB_AnalyticsWifiConfig
*/
NB_DEC const NB_AnalyticsWifiConfig* NB_AnalyticsGetWifiConfig(NB_Context* context);

/* @} */

#endif
