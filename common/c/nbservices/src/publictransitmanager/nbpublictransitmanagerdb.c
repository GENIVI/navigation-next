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

    @file     nbpublictransitmanagerdb.c

    Public transit db data storage manager
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*!
    @addtogroup nbpublictransitmanager
    @{
*/

#include "nbpublictransitmanager.h"
#include "nbpublictransitmanagerprivate.h"
#include "nbsearchparameters.h"
#include "nbutility.h"

static void AddPlaceIDToRoute(NB_PublicTransitManager*pThis, PublicTransitRoute* transitRoute, uint32 placeID);

PublicTransitPlace* GetPlaceByID(NB_PublicTransitManager*pThis, uint32 id)
{
    uint32 placeCount = 0;
    uint32 index = 0;
    if (!pThis )
    {
        return NULL;
    }
    placeCount = CSL_VectorGetLength(pThis->transitPlaceList);
    for (index = 0; index < placeCount; index++)
    {
        PublicTransitPlace* place = CSL_VectorGetPointer(pThis->transitPlaceList, index);
        if (id == place->id)
        {
            return place;
        }
    }
    return NULL;
}

int AddPlaceToCache(NB_PublicTransitManager*pThis, PublicTransitPlace* place)
{
    if (!pThis || !place )
    {
        return 0;
    }
    return CSL_VectorAppend(pThis->transitPlaceList, place);
}

PublicTransitRoute* GetRouteByID(NB_PublicTransitManager* pThis, uint32 id)
{
    int i = 0;
    int routeCount = 0;
    PublicTransitRoute* ptransitRoute = NULL;
    if (!pThis )
    {
        return NULL;
    }
    routeCount = CSL_VectorGetLength(pThis->routeList);
    for(i=0; i < routeCount;i++)
    {
        ptransitRoute = CSL_VectorGetPointer(pThis->routeList, i);
        if (ptransitRoute && ptransitRoute->id == id)
        {
            return ptransitRoute;
        }
    }
    return NULL;
}
PublicTransitRoute* AddRouteToCache(NB_PublicTransitManager* pThis, NB_PublicTransitStopInfo* agencyRoute)
{
    PublicTransitRoute transitRoute = {0};
    if (!pThis || !agencyRoute)
    {
        return NULL;
    }

    // Note: must check for existing before adding a new route
    transitRoute.id = agencyRoute->routeId;
    nsl_strncpy(transitRoute.shortName, agencyRoute->routeShortName, NB_PLACE_NAME_LEN);
    nsl_strncpy(transitRoute.longName, agencyRoute->routeLongName, NB_PLACE_NAME_LEN);
    transitRoute.type= agencyRoute->routeType;
    transitRoute.color = agencyRoute->routeColor;
    transitRoute.textColor= agencyRoute->routeTextColor;
    transitRoute.isRouteComplete = FALSE;
    if (!CSL_VectorAppend(pThis->routeList, &transitRoute))
    {
        return NULL;
    }
    //return the last item in vector
    return CSL_VectorGetPointer(pThis->routeList, CSL_VectorGetLength(pThis->routeList) - 1);
}

PublicTransitAgencyStop* GetStopByID(NB_PublicTransitManager*pThis, uint32 id)
{
    int stopCount = 0;
    int i = 0;
    PublicTransitAgencyStop* ptmAgencyStop = NULL;
    if (!pThis )
    {
        return NULL;
    }
    stopCount = CSL_VectorGetLength(pThis->stopsList);
    for(i=0; i < stopCount;i++)
    {
        ptmAgencyStop = CSL_VectorGetPointer(pThis->stopsList, i);
        if (ptmAgencyStop && ptmAgencyStop->id == id)
        {
            return ptmAgencyStop;
        }
    }
    return NULL;
}

PublicTransitAgencyStop* AddAgencyStopToCache(NB_PublicTransitManager* pThis, NB_PublicTransitStopInfo* agencyStop, uint32 placeID)
{
    PublicTransitAgencyStop ptmAgencyStop = {0};
    if (!pThis || !agencyStop )
    {
        return NULL;
    }
    AppendRouteList(pThis, agencyStop, placeID);
    ptmAgencyStop.id = agencyStop->stopId;
    if (CSL_VectorAppend(pThis->stopsList, &ptmAgencyStop) == TRUE)
    {
        //return the last item in vector
        return CSL_VectorGetPointer(pThis->stopsList, CSL_VectorGetLength(pThis->stopsList) - 1);
    }
    else
    {
        return NULL;
    }
}

nb_boolean PlacesMatch(PublicTransitPlace* place1, PublicTransitPlace* place2)
{
    int i = 0;
    int j = 0;

    if (place1 == NULL || place2 == NULL)
        return FALSE;

    // See if they share a NB_TransitStop_PTM id
    for (i = 0; i < CSL_VectorGetLength(place1->stops); i++)
    {
        // Maybe we can sort the lists by their ID to search faster
        for (j = 0; j < CSL_VectorGetLength(place2->stops); j++)
        {

        }
    }

    // If places have same lat/lon we will say they match
    return (nb_boolean)(place1->place.location.latitude == place2->place.location.latitude
                     && place1->place.location.longitude == place2->place.location.longitude);
}

NB_Error CopyNBHours(NB_PublicTransitStopInfo* agencyStop, PublicTransitStop* ptmTransitStop)
{
    NB_Error err = NE_OK;
    if (agencyStop->hoursOfOperation && agencyStop->hoursOfOperationCount > 0 &&
        ptmTransitStop->hoursOfOperation == NULL)
    {
        NB_PublicTransitHours hours = {0};
        int i = 0;

        ptmTransitStop->hoursOfOperation = CSL_VectorAlloc(sizeof(NB_PublicTransitHours));
        if (ptmTransitStop->hoursOfOperation == NULL)
        {
            return NE_NOMEM;
        }

        for (i = 0; i < agencyStop->hoursOfOperationCount; i++)
        {
            NB_Hours* pHours = agencyStop->hoursOfOperation + i;
            nsl_strncpy(hours.operatingDays, pHours->operatingDays, NB_PTM_TRANSIT_TIME_LEN);
            nsl_strncpy(hours.startTime, pHours->startTime, NB_PTM_TRANSIT_TIME_LEN);
            nsl_strncpy(hours.endTime, pHours->endTime, NB_PTM_TRANSIT_TIME_LEN);
            nsl_strncpy(hours.headwaySeconds, pHours->headwaySeconds, NB_PTM_TRANSIT_TIME_LEN);
            if (!CSL_VectorAppend(ptmTransitStop->hoursOfOperation, &hours))
            {
                CSL_VectorDealloc(ptmTransitStop->hoursOfOperation);
                return NE_NOMEM;
            }
        }
    }

    return err;
}

void CopyHoursStructure(NB_PublicTransitHours* dest, NB_PublicTransitHours* src)
{
    if (dest && src)
    {
        nsl_strncpy(dest->operatingDays, src->operatingDays, NB_PTM_TRANSIT_TIME_LEN);
        nsl_strncpy(dest->startTime, src->startTime, NB_PTM_TRANSIT_TIME_LEN);
        nsl_strncpy(dest->endTime, src->endTime, NB_PTM_TRANSIT_TIME_LEN);
        nsl_strncpy(dest->headwaySeconds, src->headwaySeconds, NB_PTM_TRANSIT_TIME_LEN);
    }
}

static void AddPlaceIDToRoute(NB_PublicTransitManager* pThis, PublicTransitRoute* transitRoute, uint32 placeID)
{
    int i = 0;
    int len = 0;
    nb_boolean isNew = TRUE;

    if (!transitRoute->transitPlaces)
    {
        transitRoute->transitPlaces = CSL_VectorAlloc(sizeof(uint32));
    }
    if (transitRoute->transitPlaces)
    {
        len = CSL_VectorGetLength(transitRoute->transitPlaces);
        for (i = 0; i < len; i++)
        {
            if (placeID == *(uint32*)CSL_VectorGetPointer(transitRoute->transitPlaces, i))
            {
                isNew = FALSE;
                break;
            }
        }
        if (isNew)
        {
            if (!CSL_VectorAppend(transitRoute->transitPlaces, &placeID))
            {
                return;
            }
        }
    }
}

uint32 GetNewPlaceId(NB_PublicTransitManager*pThis)
{
    if (!pThis)
    {
        return 0;
    }
    return pThis->nextUniqueID++;
}
uint32 AppendRouteList(NB_PublicTransitManager* pThis, NB_PublicTransitStopInfo* agencyStop, uint32 placeID)
{
    PublicTransitRoute* transitRoute = NULL;

    transitRoute = GetRouteByID(pThis, agencyStop->routeId); //
    if (!transitRoute)
    {
        transitRoute = AddRouteToCache(pThis, agencyStop);
    }
    // Add transit place ID to this route
    AddPlaceIDToRoute(pThis, transitRoute, placeID);

    return transitRoute->id;
}

/*! @} */
