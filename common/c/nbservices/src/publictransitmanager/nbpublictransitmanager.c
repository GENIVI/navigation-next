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

    @file     nbpublictransitmanager.c

    Public transit data storage and query manager
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

#define PUBLIC_TRANSIT_SEARCH_SCHEME    "public-transit"
#define ROUTE_AGENCY_ID_FILTER_KEY      "unique_route_agency_id"
#define STOP_AGENCY_ID_FILTER_KEY       "unique_stop_agency_id"
#define PUBLIC_TRANSIT_POINT_TYPE_KEY   "point-type"
#define PUBLIC_TRANSIT_POINT_TYPE_START "start"
#define PUBLIC_TRANSIT_POINT_TYPE_END   "end"

static NB_Error AddTransitStop(NB_PublicTransitManager* pThis, NB_Place* place, NB_PublicTransitPlaceInfo* transitStop, uint32* placeID, nb_boolean isTransitPlaceComplete, nb_boolean merge);
static NB_Error AppendTransitStopToPlace(PublicTransitPlace* ptransitPlace, NB_PublicTransitStopInfo* agencyStop);
static NB_Error AddTransitStopArrayToPlace(NB_PublicTransitManager* pThis, PublicTransitPlace* ptransitPlace, NB_PublicTransitPlaceInfo* transitStop);
static void MergeStops(NB_PublicTransitManager*pThis, PublicTransitPlace* placeDestination, NB_PublicTransitPlaceInfo* transitStop);
static void RouteRequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData);
static nb_boolean AreAllRoutesCompleted(NB_PublicTransitManager*pThis, PublicTransitPlace* pTransitPlace);
static NB_Error ComposeSearchResults(NB_PublicTransitManager*pThis,  PublicTransitPlace* pTransitPlace);
static NB_Error AddRouteRequests(NB_PublicTransitManager*pThis, uint32* transitPlaceIds, int transitPlaceCount);
static NB_Error CompleteRequestStops(NB_PublicTransitManager*pThis, NB_NetworkRequestStatus status, NB_Error err);
static NB_Error AddStopRequest(NB_PublicTransitManager*pThis, PublicTransitPlace* pTransitPlace, NB_PublicTransitStopRequestCallbackFunction callback);
static void StopRequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err_in, uint8 up, int percent, void* userData);
static NB_Error RequestStops3LevelRequestCallback(NB_PublicTransitManager* pThis, PublicTransitPlace*transitPlace, NB_NetworkRequestStatus status, NB_Error err_in);
static NB_Error GetStopInfo3LevelRequestCallback(NB_PublicTransitManager* pThis, PublicTransitPlace*transitPlace, NB_NetworkRequestStatus status, NB_Error err_in);
static nb_boolean AreRequestsInProgress(NB_PublicTransitManager* pThis);
static void TransitOverlayStopsRequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err_in, uint8 up, int percent, void* userData);
static NB_Error AddTransitOverlayStopsRequest(NB_PublicTransitManager*pThis, const char* transitTypes[], int transitTypeCount);

static NB_Error InitializePlanTripData(NB_PublicTransitManager* pThis);
static void ResetPlanTripData(PlanTripData* pData);
static void ReleasePlanTripData(NB_PublicTransitManager* pThis);
static NB_Error PlanTripGetResultData(NB_PublicTransitManager* pThis, NB_SearchHandler* handler, uint32** pPlaceList, int* placeListCount);
static void PlanTripRequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData);
static NB_Error AddRoutes(NB_PublicTransitManager* pThis, NB_PublicTransitPlaceInfo* publicTransitPlace, uint32 placeID);
static NB_PublicTransitPlace* CreatePublicTransitPlace(NB_PublicTransitManager* pThis, PublicTransitPlace* pTransitPlace);
static int IdCompareFunction(const void* left, const void* right);

/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerCreate(NB_Context* context, const char* language, NB_PublicTransitManager** pData)
{
    NB_PublicTransitManager* pThis = NULL;

    if (context == NULL || language == NULL || pData == NULL)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(NB_PublicTransitManager))) == NULL)
    {
        goto nomem;
    }
    nsl_memset(pThis, 0, sizeof(NB_PublicTransitManager));

    if ((pThis->transitPlaceList = CSL_VectorAlloc(sizeof(PublicTransitPlace))) == NULL)
    {
        goto nomem;
    }

    if ((pThis->routeList = CSL_VectorAlloc(sizeof(PublicTransitRoute))) == NULL)
    {
        goto nomem;
    }
    if ((pThis->stopsList = CSL_VectorAlloc(sizeof(PublicTransitAgencyStop))) == NULL)
    {
        goto nomem;
    }

    if (language)
    {
        nsl_strlcpy(pThis->language, language, sizeof(pThis->language));
    }
    if ((pThis->placeIdList = CSL_VectorAlloc(sizeof(uint32))) == NULL)
    {
        goto nomem;
    }

    pThis->context = context;

    if (InitializePlanTripData(pThis) != NE_OK)
    {
        goto nomem;
    }

    *pData = pThis;

    return NE_OK;

nomem:
    NB_PublicTransitManagerDestroy(pThis);

    *pData = NULL;
    return NE_NOMEM;
}

/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerDestroy(NB_PublicTransitManager* pThis)
{
    NB_Error err = NE_OK;
    int len = 0;
    int i = 0;
    PublicTransitPlace* pPlace = NULL;
    PublicTransitRoute* pRoute = NULL;

    if (pThis != NULL)
    {
        // TODO: Cancel any outstanding queries
        NB_PublicTransitManagerCancelRequests(pThis);
        if (pThis->placeIdList)
        {
            CSL_VectorDealloc(pThis->placeIdList);
            pThis->placeIdList = NULL;
        }

        if (pThis->routeList)
        {
            len = CSL_VectorGetLength(pThis->routeList);
            for (i = 0; i < len; i++)
            {
                if ((pRoute = CSL_VectorGetPointer(pThis->routeList, i)) != NULL)
                {
                    if (pRoute && pRoute->transitPlaces)
                    {
                        CSL_VectorDealloc(pRoute->transitPlaces);
                        pRoute->transitPlaces = NULL;
                    }
                }
            }
            CSL_VectorDealloc(pThis->routeList);
            pThis->routeList = NULL;
        }

        if (pThis->stopsList)
        {
            CSL_VectorDealloc(pThis->stopsList);
            pThis->stopsList = NULL;
        }

        if (pThis->transitPlaceList)
        {
            len = CSL_VectorGetLength(pThis->transitPlaceList);
            for (i= 0; i < len; i++)
            {
                if ((pPlace = CSL_VectorGetPointer(pThis->transitPlaceList, i)) != NULL)
                {
                    if (pPlace->stops)
                    {
                        int j = 0;
                        int stopsCount = CSL_VectorGetLength(pPlace->stops); 
                        for (j = 0; j < stopsCount; j ++)
                        {
                            PublicTransitStop* pStop = CSL_VectorGetPointer(pPlace->stops, j);
                            if (pStop && pStop->hoursOfOperation)
                            {
                                CSL_VectorDealloc(pStop->hoursOfOperation);
                            }
                        }
                        CSL_VectorDealloc(pPlace->stops);
                        pPlace->stops = NULL;
                    }
                }
            }
            CSL_VectorDealloc(pThis->transitPlaceList);
            pThis->transitPlaceList = NULL;
        }

        ReleasePlanTripData(pThis);

        nsl_free(pThis);
    }

    return err;
}


/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerRequestPlanStops(NB_PublicTransitManager* pThis, uint32* transitPlaceIds, int transitPlaceCount, NB_PublicTransitRequestCallback* callback)
{
    NB_Error err = NE_OK;

    if (!pThis || !callback || !callback->callback || !transitPlaceIds || transitPlaceCount <= 0)
    {
        return NE_INVAL;
    }
    if (AreRequestsInProgress(pThis))
    {
        //there are some pending requests
        return NE_BUSY;
    }
    NB_PublicTransitManagerCancelRequests(pThis);

    pThis->requestCallback = callback->callback;
    pThis->requestCallbackData = callback->callbackData;

    err = AddRouteRequests(pThis, transitPlaceIds, transitPlaceCount);

    return err;
}

/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerRequestStops(NB_PublicTransitManager* pThis, uint32 placeId, NB_PublicTransitRequestCallback* callback)
{
    NB_Error err = NE_OK;
    PublicTransitPlace* pTransitPlace = NULL;

    if (!pThis || !callback || !callback->callback)
    {
        return NE_INVAL;
    }
    if ((pTransitPlace = GetPlaceByID(pThis, placeId)) == NULL)
    {
        //place is not added to PTM
        return NE_INVAL;
    }
    if (AreRequestsInProgress(pThis))
    {
        //there are some pending requiests
        return NE_BUSY;
    }
    NB_PublicTransitManagerCancelRequests(pThis);

    pThis->requestedPlaceId = placeId;
    pThis->requestCallback = callback->callback;
    pThis->requestCallbackData = callback->callbackData;

    if (pTransitPlace->isTransitPlaceComplete)
    {
        //do not need 2 level request, place is completed
        err = RequestStops3LevelRequestCallback(pThis, pTransitPlace, NB_NetworkRequestStatus_Success, err);
    }
    else
    {
        //send 3 level request (get all routes for selected place/stop)
        AddStopRequest(pThis, pTransitPlace, RequestStops3LevelRequestCallback);
    }
    return err;
}

NB_DEC NB_Error
NB_PublicTransitManagerRequestTransitOverlayStops(NB_PublicTransitManager* pThis, NB_BoundingBox* boundingBox, const char* transitTypes[], int transitTypeCount, NB_PublicTransitRequestCallback* callback)
{
    NB_Error err = NE_OK;

    if (!pThis || !callback || !callback->callback || !boundingBox)
    {
        return NE_INVAL;
    }
    if (AreRequestsInProgress(pThis))
    {
        //there are some pending requiests
        return NE_BUSY;
    }
    NB_PublicTransitManagerCancelRequests(pThis);

    pThis->queryBoundingBox = *boundingBox;
    pThis->requestCallback = callback->callback;
    pThis->requestCallbackData = callback->callbackData;


    err = AddTransitOverlayStopsRequest(pThis, transitTypes, transitTypeCount);

    return err;
}

/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerGetStopInfo(NB_PublicTransitManager* pThis, uint32 placeId, NB_PublicTransitGetStopInfoCallback* callback)
{
    NB_Error err = NE_OK;
    PublicTransitPlace* pTransitPlace = NULL;

    if (!pThis || !callback)
    {
        return NE_INVAL;
    }
    if (AreRequestsInProgress(pThis))
    {
        //there are some pending requiests
        return NE_BUSY;
    }
    if ((pTransitPlace = GetPlaceByID(pThis, placeId)) != NULL)
    {
        pThis->getStopInfoCallback.callback = callback->callback;
        pThis->getStopInfoCallback.callbackData = callback->callbackData;
        pThis->requestedPlaceId = placeId;
        if (pTransitPlace->isTransitPlaceComplete)
        {
            err = GetStopInfo3LevelRequestCallback(pThis, pTransitPlace,NB_NetworkRequestStatus_Success, err);
        }
        else
        {
            err = AddStopRequest(pThis, pTransitPlace, GetStopInfo3LevelRequestCallback);
        }
    }
    return err;
}

NB_DEC NB_Error
NB_PublicTransitManagerGetCurrentStopInfo(NB_PublicTransitManager* pThis, uint32 placeId, NB_PublicTransitPlace** place)
{
    NB_Error err = NE_NOENT;
    PublicTransitPlace* pTransitPlace = NULL;
    NB_PublicTransitPlace* nbplace = NULL;

    if (!pThis || !place)
    {
        return NE_INVAL;
    }

    if ((pTransitPlace = GetPlaceByID(pThis, placeId)) != NULL)
    {
            nbplace = CreatePublicTransitPlace(pThis, pTransitPlace);
            if (nbplace)
            {
                err = NE_OK;
                *place = nbplace;
            }
    }
    return err;
}

NB_DEC void
NB_PublicTransitManagerReleaseTransitPlace(NB_PublicTransitPlace* place)
{
    if (place)
    {
        if (place->stops)
        {
            nsl_free(place->stops);
        }
        nsl_free(place);
    }
}

/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerAddTransitStop(NB_PublicTransitManager* pThis, NB_Place* place, NB_PublicTransitPlaceInfo* transitStop, uint32* placeID)
{
    return AddTransitStop(pThis, place, transitStop, placeID, TRUE, FALSE);
}

/* See nbpublictransitmanager.h for description */
NB_DEC NB_Error
NB_PublicTransitManagerCreatePlanTrip(NB_PublicTransitManager* pThis, NB_Place* start, NB_Place* end, uint32 sliceSize, NB_PublicTransitPlanTripCallbackFunction callback, void* cbData)
{
    NB_Error err = NE_OK;
    NB_SearchRegion region = {0};
    NB_SearchParameters* pSearchParameters = NULL;
    PlanTripData* pPlanTripData = &pThis->planTripData;

    if (!pThis || !start || !end )
    {
        return NE_INVAL;
    }

    if ( NB_SearchHandlerIsRequestInProgress(pPlanTripData->pStartSearchHandler) || NB_SearchHandlerIsRequestInProgress(pPlanTripData->pEndSearchHandler))
    {
        return NE_BUSY;
    }

    ResetPlanTripData(pPlanTripData);

    pPlanTripData->callBack = callback;
    pPlanTripData->cbData = cbData;

    //Send 1st level search base on start place
    region.type = NB_ST_Center;
    region.center.latitude = start->location.latitude;
    region.center.longitude = start->location.longitude;
    err = err ? err : NB_SearchParametersCreatePublicTransit(pThis->context, &region, NULL, NULL, 0, sliceSize, pThis->language, &pSearchParameters);
    err = err ? err : NB_SearchParametersAddSearchFilterKeyValue(pSearchParameters, PUBLIC_TRANSIT_POINT_TYPE_KEY, PUBLIC_TRANSIT_POINT_TYPE_START);
    err = err ? err : NB_SearchHandlerStartRequest(pPlanTripData->pStartSearchHandler, pSearchParameters);
    NB_SearchParametersDestroy(pSearchParameters);
    pSearchParameters = NULL;

    //Send 1st level search base on end place
    region.type = NB_ST_Center;
    region.center.latitude = end->location.latitude;
    region.center.longitude = end->location.longitude;
    err = err ? err : NB_SearchParametersCreatePublicTransit(pThis->context, &region, NULL, NULL, 0, sliceSize, pThis->language, &pSearchParameters);
    err = err ? err : NB_SearchParametersAddSearchFilterKeyValue(pSearchParameters, PUBLIC_TRANSIT_POINT_TYPE_KEY, PUBLIC_TRANSIT_POINT_TYPE_END);
    err = err ? err : NB_SearchHandlerStartRequest(pPlanTripData->pEndSearchHandler, pSearchParameters);
    NB_SearchParametersDestroy(pSearchParameters);
    pSearchParameters = NULL;

    return err;
}

NB_DEC NB_Error
NB_PublicTransitManagerCancelRequests(NB_PublicTransitManager*   pThis)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->searchHandler)
    {
        err = NB_SearchHandlerDestroy(pThis->searchHandler);
        pThis->searchHandler = NULL;
    }
    return err;
}

static NB_Error AddTransitStop(NB_PublicTransitManager* pThis, NB_Place* place, NB_PublicTransitPlaceInfo* publicTransitPlaceInfo,
                               uint32* placeID, nb_boolean isTransitPlaceComplete, nb_boolean merge)
{
    NB_Error err = NE_OK;
    int len = 0;
    int i = 0;
    PublicTransitPlace transitPlace = {0};
    PublicTransitPlace* pTransitPlace = NULL;
    nb_boolean placeMatch = FALSE;
    if (!pThis || !place || !publicTransitPlaceInfo || !placeID)
    {
        return NE_INVAL;
    }

    //if place without stops do not store it
    if (!publicTransitPlaceInfo->transitStops)
    {
        return NE_INVAL;
    }

    CopyPlace(&transitPlace.place, place, FALSE);
    *placeID = 0;
    // For each transit stop
    len = CSL_VectorGetLength(pThis->transitPlaceList);
    for(i= 0; i < len; i++)
    {
        if ((pTransitPlace = GetPlaceByID(pThis, i)) != NULL)
        {
            if (pTransitPlace && merge && PlacesMatch(pTransitPlace, &transitPlace))
            {
                *placeID = i;
                placeMatch = TRUE;
                if (isTransitPlaceComplete == TRUE)
                {
                    pTransitPlace->isTransitPlaceComplete = TRUE;
                }
                MergeStops(pThis, pTransitPlace, publicTransitPlaceInfo);
                break;
            }
        }
        else
        {
            break;
        }
    }
    // If we have not a match
    if (err == NE_OK && !placeMatch)
    {
        transitPlace.isTransitPlaceComplete = isTransitPlaceComplete;
        transitPlace.id = GetNewPlaceId(pThis);
        err = AddTransitStopArrayToPlace(pThis, &transitPlace, publicTransitPlaceInfo);
        if (err == NE_OK && AddPlaceToCache(pThis, &transitPlace))
        {
            *placeID = transitPlace.id;
        }
    }
    return err;
}

static NB_Error AddRoutes(NB_PublicTransitManager* pThis, NB_PublicTransitPlaceInfo* publicTransitPlace, uint32 placeID)
{
    NB_Error err = NE_OK;
    PublicTransitPlace* pTransitPlace = NULL;

    if (!pThis || !publicTransitPlace)
    {
        return NE_INVAL;
    }

    //if place without stops do not store it
    if (!publicTransitPlace->transitStops || (pTransitPlace = GetPlaceByID(pThis, placeID)) == NULL)
    {
        return NE_INVAL;
    }

    err = AddTransitStopArrayToPlace(pThis, pTransitPlace, publicTransitPlace);
    return err;
}

static NB_Error AppendTransitStopToPlace(PublicTransitPlace* ptransitPlace, NB_PublicTransitStopInfo* agencyStop)
{
    PublicTransitStop ptmTransitStop = {0};

    if(!ptransitPlace)
    {
        return NE_INVAL;
    }
    if (ptransitPlace->stops == NULL)
    {
        ptransitPlace->stops = CSL_VectorAlloc(sizeof(PublicTransitStop));
    }
    if (ptransitPlace->stops == NULL)
    {
        return NE_NOMEM;
    }

    ptransitPlace->stopType |= agencyStop->stopType;
    ptmTransitStop.agencyStopId = agencyStop->stopId;
    ptmTransitStop.routeId = agencyStop->routeId;

    // add hours info if present
    CopyNBHours(agencyStop, &ptmTransitStop);

    if (!CSL_VectorAppend(ptransitPlace->stops, &ptmTransitStop))
    {
        CSL_VectorDealloc(ptransitPlace->stops);
        ptransitPlace->stops = NULL;
        return NE_NOMEM;
    }

    return NE_OK;
}

static nb_boolean AreRequestsInProgress(NB_PublicTransitManager* pThis)
{
    nb_boolean inProgress = FALSE;

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!inProgress && pThis->searchHandler)
    {
        inProgress = NB_SearchHandlerIsRequestInProgress(pThis->searchHandler);
    }
    return inProgress;
}
static NB_Error AddTransitStopArrayToPlace(NB_PublicTransitManager* pThis, PublicTransitPlace* ptransitPlace, NB_PublicTransitPlaceInfo* transitStop)
{
    NB_Error err = NE_OK;
    int i = 0;
    NB_PublicTransitStopInfo* agencyStop = NULL;
    if (!pThis || !ptransitPlace || !transitStop)
    {
        return NE_INVAL;
    }

    for(i = 0; i < transitStop->transitStopCount; i++)
    {
        agencyStop = transitStop->transitStops+ i;
        if (agencyStop)
        {
            if (GetStopByID(pThis, agencyStop->stopId))
            {
                AppendRouteList(pThis, agencyStop, ptransitPlace->id);
            }
            else
            {
                if (AddAgencyStopToCache(pThis, agencyStop, ptransitPlace->id) == NULL)
                {
                    return NE_INVAL;
                }
            }
            err = AppendTransitStopToPlace(ptransitPlace, agencyStop);
        }
        else
        {
            err = NE_NOENT;
            break;
        }
    }
    return err;
}

void MergeStops(NB_PublicTransitManager*pThis, PublicTransitPlace* placeDestination, NB_PublicTransitPlaceInfo* transitStop)
{
    PublicTransitAgencyStop* ptmAgencyStop = NULL;
    NB_PublicTransitStopInfo* agencyStop = NULL;
    int i = 0;

    if (placeDestination == NULL || transitStop == NULL ||
        transitStop->transitStops == NULL || transitStop->transitStopCount == 0)
        return;

    for (i=0; i < transitStop->transitStopCount; i++)
    {
        agencyStop = transitStop->transitStops+ i;
        if (!GetStopByID(pThis, agencyStop->stopId))
        {
            ptmAgencyStop = AddAgencyStopToCache(pThis, agencyStop, placeDestination->id);
            if (ptmAgencyStop)
            {
                AppendTransitStopToPlace(placeDestination, agencyStop);
            }
        }
        else if (agencyStop->routeId != 0)
        {
            // don't attempt to merge incomplete data
            // otherwise, see if the result matches an existing stop
            // a match is stopid==stopid && (routeid==0 || routeid==routeid)
            int i = 0;
            PublicTransitStop *curStop = NULL;
            int len = CSL_VectorGetLength(placeDestination->stops);
            for (i = 0; i < len; i++)
            {
                curStop = CSL_VectorGetPointer(placeDestination->stops, i);
                if (curStop->agencyStopId == agencyStop->stopId &&
                    (curStop->routeId == 0 || curStop->routeId == agencyStop->routeId))
                {
                    break;
                }
            }

            if (i == len)
            {
                // we've not seen this route/stop before, add it
                AppendRouteList(pThis, agencyStop, placeDestination->id);
                AppendTransitStopToPlace(placeDestination, agencyStop);
            }
            else if (curStop && curStop->routeId == 0)
            {
                // we have partial info, update it
                AppendRouteList(pThis, agencyStop, placeDestination->id);
                curStop->routeId = agencyStop->routeId;
                CopyNBHours(agencyStop, curStop);
            }
        }
    }
}

static nb_boolean AreAllRoutesCompleted(NB_PublicTransitManager*pThis, PublicTransitPlace* pTransitPlace)
{
    int len = 0;
    int i = 0;
    PublicTransitStop* pTransitStop = NULL;
    PublicTransitRoute* pTransitRoute = NULL;
    if (pThis == NULL || pTransitPlace == NULL)
        return FALSE;

    len = CSL_VectorGetLength(pTransitPlace->stops);
    for (i = 0; i < len; i++)
    {
        pTransitStop = CSL_VectorGetPointer(pTransitPlace->stops, i);
        pTransitRoute = GetRouteByID(pThis, pTransitStop->routeId);
        if (!pTransitRoute->isRouteComplete)
        {
            return FALSE;
         }
    }
    return TRUE;
}

static int IdCompareFunction(const void* left, const void* right)
{
    const uint32* _left  = (uint32*)left;
    const uint32* _right = (uint32*)right;

    if (*_left < *_right)
    {
        return -1;
    }
    if (*_left > *_right)
    {
        return 1;
    }
    return 0;
}
static NB_Error ComposeSearchResults(NB_PublicTransitManager*pThis,  PublicTransitPlace* pTransitPlace)
{
    NB_Error err = NE_OK;
    int stopsCount = 0;
    int placeCount = 0;
    int i = 0;
    int j = 0;
    PublicTransitStop* pTransitStop = NULL;
    PublicTransitRoute* pTransitRoute = NULL;
    uint32* pPlaceId = NULL;

    stopsCount = CSL_VectorGetLength(pTransitPlace->stops);
    for (i = 0; i < stopsCount; i++)
    {
        pTransitStop = CSL_VectorGetPointer(pTransitPlace->stops, i);
        pTransitRoute = GetRouteByID(pThis, pTransitStop->routeId);
        if (pTransitRoute && pTransitRoute->isRouteComplete)
        {
            //add all plces/stops along the route to place search results
            placeCount = CSL_VectorGetLength(pTransitRoute->transitPlaces);

            for (j = 0; j < placeCount; j++)
            {
                pPlaceId = CSL_VectorGetPointer(pTransitRoute->transitPlaces, j);
                if (!CSL_VectorAppend(pThis->placeIdList, pPlaceId))
                {
                    return NE_NOMEM;
                }
            }
        }
    }
    //remove duplicates
    CSL_VectorRemoveDuplicates(pThis->placeIdList, &IdCompareFunction);

    return err;
}

static NB_Error CompleteRequestStops(NB_PublicTransitManager*pThis, NB_NetworkRequestStatus status, NB_Error err)
{
    int i = 0;
    int len = 0;
    int* pid = NULL;
    uint32* pIds = NULL;

    if (!pThis || !pThis->requestCallback)
    {
        return NE_INVAL;
    }

    if (!(status == NB_NetworkRequestStatus_Success && err == NE_OK))
    {
        //We shall not fill anything to the result list if the request failed
        //Clear the list in case of anything cached has been filled to the list before
        CSL_VectorRemoveAll(pThis->placeIdList);
        pThis->requestCallback(status, err, NULL, 0, pThis->requestCallbackData);
        pThis->requestCallback = NULL;
        pThis->requestCallbackData = NULL;
        return NE_OK;
    }

    len = CSL_VectorGetLength(pThis->placeIdList);
    if (len != 0)
    {
        pIds = nsl_malloc(sizeof(uint32)*len);
        if (!pIds)
        {
            err = NE_NOMEM;
        }
        else
        {
            for (i = 0; i < len; i++)
            {
                pid = CSL_VectorGetPointer(pThis->placeIdList, i);
                pIds[i] = *pid;
            }
        }
    }

    pThis->requestCallback(status, err, pIds, len, pThis->requestCallbackData);
    CSL_VectorRemoveAll(pThis->placeIdList);
    if (pIds)
    {
        nsl_free(pIds);
    }
    pThis->requestCallback = NULL;
    pThis->requestCallbackData = NULL;
    return NE_OK;
}

static NB_Error AddRouteRequests(NB_PublicTransitManager*pThis, uint32* transitPlaceIds, int transitPlaceCount)
{
    NB_Error err = NE_OK;
    NB_RequestHandlerCallback callbackData = {0};
    NB_SearchHandler*   pSearchHandler = NULL;
    CSL_Vector*         routeIdList = NULL;
    int i = 0;
    int len = 0;

    if (pThis == NULL || transitPlaceIds == NULL)
        return NE_INVAL;

    CSL_VectorRemoveAll(pThis->placeIdList);
    //From transit place array, find all routes which we need to request
    routeIdList = CSL_VectorAlloc(sizeof(uint32));
    for (i = 0; i < transitPlaceCount; i++)
    {
        PublicTransitPlace* pTransitPlace = GetPlaceByID(pThis, *(transitPlaceIds + i));
        if (pTransitPlace != NULL)
        {
            int j = 0;
            len = CSL_VectorGetLength(pTransitPlace->stops);
            for (j = 0; j< len; j++)
            {
                PublicTransitStop* pTransitStop = CSL_VectorGetPointer(pTransitPlace->stops, j);
                if (pTransitStop != NULL)
                {
                    PublicTransitRoute* pTransitRoute = GetRouteByID(pThis, pTransitStop->routeId);
                    if (pTransitRoute != NULL)
                    {
                        if (pTransitRoute->isRouteComplete == FALSE)
                        {
                            //Add pending route id to the list
                            if (!CSL_VectorAppend(routeIdList, &pTransitStop->routeId))
                            {
                                CSL_VectorDealloc(routeIdList);
                                return NE_NOMEM;
                            }
                        }
                        else
                        {
                            //Firstly, add the cached result to result list
                            int placeCount = CSL_VectorGetLength(pTransitRoute->transitPlaces);
                            int k = 0;
                            for (k = 0; k < placeCount; k++)
                            {
                                uint32* pPlaceId = CSL_VectorGetPointer(pTransitRoute->transitPlaces, k);
                                if (!CSL_VectorAppend(pThis->placeIdList, pPlaceId))
                                {
                                    CSL_VectorDealloc(routeIdList);
                                    return NE_NOMEM;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //Remove duplicate results in the result list
    CSL_VectorRemoveDuplicates(pThis->placeIdList, IdCompareFunction);

    //Remove duplicate route ids in the pending request id list
    CSL_VectorRemoveDuplicates(routeIdList, IdCompareFunction);
    len = CSL_VectorGetLength(routeIdList);
    if (len > 0)
    {
        //send 2 level requests (get all place/stops along the route)
        callbackData.callback = RouteRequestHandlerCallback;
        callbackData.callbackData = pThis;

        err = NB_SearchHandlerCreate(pThis->context, &callbackData, &pSearchHandler);
        if (err == NE_OK)
        {
            NB_SearchParameters* searchParameters = NULL;
            // this request doesn't use location but NB_SearchParameters requires it
            // server will report an error if the lat/lon of the place is not specified, in other words,lat/lon could not be zero
            // so just use the first transit place location
            PublicTransitPlace* pTransitPlace = GetPlaceByID(pThis, *transitPlaceIds);
            if (pTransitPlace)
            {
                NB_SearchRegion region = {0};
                region.type = NB_ST_Center;
                region.center.latitude = pTransitPlace->place.location.latitude;
                region.center.longitude = pTransitPlace->place.location.longitude;

                err = NB_SearchParametersCreatePOI(pThis->context, &region, NULL, PUBLIC_TRANSIT_SEARCH_SCHEME, NULL, 0, 0, NB_EXT_None, pThis->language, &searchParameters);

                for (i = 0; i < len; i++)
                {
                    uint32* routeId = CSL_VectorGetPointer(routeIdList, i);
                    char routeIdText[10] = {0};
                    nsl_sprintf(routeIdText, "%d", *routeId);
                    err = err ? err : NB_SearchParametersAddSearchFilterKeyValue(searchParameters, ROUTE_AGENCY_ID_FILTER_KEY, routeIdText);
                }

                err = err ? err : NB_SearchHandlerStartRequest(pSearchHandler, searchParameters);
                NB_SearchParametersDestroy(searchParameters);
                if (err == NE_OK)
                {
                    pThis->searchHandler = pSearchHandler;
                }
            }
        }
    }
    else
    {
        //All routes are complete, return immediately
        CompleteRequestStops(pThis, NB_NetworkRequestStatus_Success, err);
    }
    CSL_VectorDealloc(routeIdList);

    return err;
}

static void RouteRequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err_in, uint8 up, int percent, void* userData)
{
    NB_Error err = NE_OK;
    NB_PublicTransitManager* pThis = (NB_PublicTransitManager*)userData;
    NB_SearchInformation*   pSearchInformation = NULL;
    NB_PublicTransitPlaceInfo stop = {0};
    NB_Place place = {0};
    uint32 placeId = 0;


    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }
    else if (status == NB_NetworkRequestStatus_Success)
    {
        NB_SearchHandlerGetSearchInformation(handler, &pSearchInformation);
    }
    else
    {
        err = NE_NET;
    }

     //handle response
    if (pSearchInformation)
    {
        int resultIndex = 0;
        int count = 0;
        int i = 0;

        err = NB_SearchInformationGetResultCount(pSearchInformation, &count);

        for (resultIndex = 0; resultIndex < count && err == NE_OK; resultIndex++)
        {
            err = NB_SearchInformationGetPublicTransitPlace(pSearchInformation, resultIndex, &stop);
            err = err ? err : NB_SearchInformationGetPlace(pSearchInformation, resultIndex, &place, NULL, NULL);
            err = err ? err : AddTransitStop(pThis, &place, &stop, &placeId, FALSE, TRUE);
            if (err == NE_OK)
            {
                //Compose result place list, add result place ids to the result list, the cached place ids have been added to the list before
                CSL_VectorAppend(pThis->placeIdList, &placeId);

                //Set route complete to TRUE
                for (i = 0; i < stop.transitStopCount; i++)
                {
                    NB_PublicTransitStopInfo* transitStopInfo = stop.transitStops + i;
                    if (transitStopInfo != NULL)
                    {
                        PublicTransitRoute* pTransitRoute = GetRouteByID(pThis, transitStopInfo->routeId);
                        if (pTransitRoute != NULL)
                        {
                            pTransitRoute->isRouteComplete = TRUE;
                        }
                    }
                }
                NB_SearchInformationReleasePublicTransitPlace(&stop);
            }
        }

        //remove duplicates
        CSL_VectorRemoveDuplicates(pThis->placeIdList, &IdCompareFunction);

        NB_SearchInformationDestroy(pSearchInformation);
    }
    if (!AreRequestsInProgress(pThis))
    {
        //all route search requests are handled, we have to fire client callback
        CompleteRequestStops(pThis, status, err_in ? err_in : err);
    }
}

static NB_Error AddStopRequest(NB_PublicTransitManager*pThis, PublicTransitPlace* pTransitPlace, NB_PublicTransitStopRequestCallbackFunction callback)
{
    NB_Error err = NE_OK;
    int len = 0;
    int i = 0;
    char stopIdText[10] = {0};

    if (pThis == NULL || pTransitPlace == NULL || callback == NULL)
        return NE_INVAL;

    if (pTransitPlace->stops && CSL_VectorGetLength(pTransitPlace->stops) > 0)
    {
        //send 3 level requests (get all routes for particular stop)
        NB_RequestHandlerCallback callbackData = {0};
        NB_SearchHandler*   pSearchHandler = NULL;
        NB_SearchParameters* searchParameters = NULL;
        NB_SearchRegion region = {0};
        CSL_Vector* stopIdList = NULL;

        stopIdList = CSL_VectorAlloc(sizeof(uint32));
        if (!stopIdList)
        {
            return NE_NOMEM;
        }

        for (i = 0; i < CSL_VectorGetLength(pTransitPlace->stops); i++)
        {
            PublicTransitStop* pTransitStop = NULL;
            pTransitStop = CSL_VectorGetPointer(pTransitPlace->stops, i);
            if (pTransitPlace)
            {
                if(!CSL_VectorAppend(stopIdList, &pTransitStop->agencyStopId))
                {
                    CSL_VectorDealloc(stopIdList);
                    return NE_NOMEM;
                }
            }
        }
        CSL_VectorRemoveDuplicates(stopIdList, IdCompareFunction);

        len = CSL_VectorGetLength(stopIdList);
        if (len == 0)
        {
            err = NE_API;
        }

        callbackData.callback = StopRequestHandlerCallback;
        callbackData.callbackData = pThis;
        pThis->requestedPlaceId = pTransitPlace->id;

        err = err ? err : NB_SearchHandlerCreate(pThis->context, &callbackData, &pSearchHandler);
        if (err == NE_OK)
        {
            //TODO: data for request have to be updated
            region.type = NB_ST_Center;
            region.center.latitude = pTransitPlace->place.location.latitude;
            region.center.longitude = pTransitPlace->place.location.longitude;

            err = NB_SearchParametersCreatePOI(pThis->context, &region, NULL, PUBLIC_TRANSIT_SEARCH_SCHEME, NULL, 0, 0, NB_EXT_None, pThis->language, &searchParameters);
            
            //add all unique stops IDs as filters
            for (i = 0; i < len; i++)
            {
                uint32 stopId;
                CSL_VectorGet(stopIdList, i, &stopId);
                nsl_sprintf(stopIdText, "%d", stopId);
                err = err ? err : NB_SearchParametersAddSearchFilterKeyValue(searchParameters, STOP_AGENCY_ID_FILTER_KEY, stopIdText);
            }
            err = err ? err : NB_SearchHandlerStartRequest(pSearchHandler, searchParameters);
            NB_SearchParametersDestroy(searchParameters);
            if (err == NE_OK)
            {
                pThis->searchHandler = pSearchHandler;
                pThis->stopRequestCallback = callback;
            }
        }
        CSL_VectorDealloc(stopIdList);
    }
    return err;
}

static void StopRequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err_in, uint8 up, int percent, void* userData)
{
    NB_Error err = NE_OK;
    NB_PublicTransitManager* pThis = (NB_PublicTransitManager*)userData;
    NB_SearchInformation*   pSearchInformation = NULL;
    NB_PublicTransitPlaceInfo stop = {0};
    PublicTransitPlace* transitPlace = NULL;

    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }
    else if (status == NB_NetworkRequestStatus_Success)
    {
        NB_SearchHandlerGetSearchInformation(handler, &pSearchInformation);
    }
    else
    {
        err = NE_NET;
    }

    CSL_VectorRemoveAll(pThis->placeIdList);

    //handle response
    if (pSearchInformation)
    {
        int resultIndex = 0;
        int count = 0;
        PublicTransitPlace* pTransitPlace = NULL;

        err = NB_SearchInformationGetResultCount(pSearchInformation, &count);

        pTransitPlace = GetPlaceByID(pThis, pThis->requestedPlaceId);

        // clear the incomplete information
        if (pTransitPlace && pTransitPlace->stops != NULL)
        {
            CSL_VectorDealloc(pTransitPlace->stops);
            pTransitPlace->stops = NULL;
            pTransitPlace->stopType = 0;
        }

        // process results and add to place route info
        for (resultIndex = 0; resultIndex < count && err == NE_OK; resultIndex++)
        {
            err = NB_SearchInformationGetPublicTransitPlace(pSearchInformation, resultIndex, &stop);
            err = err ? err : AddRoutes(pThis, &stop, pThis->requestedPlaceId);
            NB_SearchInformationReleasePublicTransitPlace(&stop);
        }

        if(err == NE_OK)
        {
            //Set place complete to TRUE
            transitPlace = CSL_VectorGetPointer(pThis->transitPlaceList, pThis->requestedPlaceId);
            transitPlace->isTransitPlaceComplete = TRUE;
        }

        NB_SearchInformationDestroy(pSearchInformation);
    }

    if (status == NB_NetworkRequestStatus_Success && err == NE_OK)
    {
        //all stops search requests are handled, we have to run the second level request or fire client callback
        transitPlace = CSL_VectorGetPointer(pThis->transitPlaceList, pThis->requestedPlaceId);
        err = pThis->stopRequestCallback(pThis, transitPlace, status, err_in);
    }
    else
    {
        CompleteRequestStops(pThis, status, err_in ? err_in : err);
    }
}

static NB_Error RequestStops3LevelRequestCallback(NB_PublicTransitManager* pThis, PublicTransitPlace*transitPlace, NB_NetworkRequestStatus status, NB_Error err_in)
{
    NB_Error err = NE_OK;

    if (AreAllRoutesCompleted(pThis, transitPlace))
    {
        err = ComposeSearchResults(pThis, transitPlace);
        err = CompleteRequestStops(pThis, status, err);
    }
    else
    {
        AddRouteRequests(pThis, &(transitPlace->id), 1);
    }
    return err ? err : err_in;
}

static NB_PublicTransitPlace* CreatePublicTransitPlace(NB_PublicTransitManager* pThis, PublicTransitPlace* pTransitPlace)
{
    NB_PublicTransitPlace* pPlace = NULL;
    PublicTransitStop* transitStop = NULL;
    PublicTransitAgencyStop* agencyStop = NULL;
    PublicTransitRoute* agencyRoute = NULL;
    NB_PublicTransitStop* stop = NULL;
    uint32 stopType = 0;
    int i = 0;

    if (pThis == NULL || pTransitPlace == NULL)
    {
        return NULL;
    }

    pPlace = (NB_PublicTransitPlace*)nsl_malloc(sizeof(NB_PublicTransitPlace));
    if (pPlace != NULL)
    {
        nsl_memset(pPlace, 0, sizeof(NB_PublicTransitPlace));
        CopyPlace(&pPlace->place, &pTransitPlace->place, FALSE);
        pPlace->id = pTransitPlace->id;
        pPlace->stopType = pTransitPlace->stopType;
        pPlace->isTransitPlaceComplete = pTransitPlace->isTransitPlaceComplete;
        pPlace->countOfStops = CSL_VectorGetLength(pTransitPlace->stops);
        pPlace->stops = (NB_PublicTransitStop*)nsl_malloc(sizeof(NB_PublicTransitStop) * pPlace->countOfStops);
        if (pPlace->stops != NULL)
        {
            for (i = 0; i < pPlace->countOfStops; i++)
            {
                int hoursCount = 0;
                stopType = 0;
                transitStop = CSL_VectorGetPointer(pTransitPlace->stops, i);
                agencyStop = GetStopByID(pThis, transitStop->agencyStopId);
                stop = pPlace->stops + i;
                stop->stopId = agencyStop->id;
                stop->agencyId = agencyStop->agencyId;
                stop->sequence = agencyStop->sequence;
                nsl_strncpy(stop->agencyName, agencyStop->agencyName, NB_PLACE_NAME_LEN);
                agencyRoute = GetRouteByID( pThis, transitStop->routeId);
                stop->routeId = transitStop->routeId;
                stop->routeTextColor = agencyRoute->textColor;
                stop->routeColor = agencyRoute->color;
                stop->routeType= agencyRoute->type;
                stopType |= ConvertPublicTransitRouteType(agencyRoute->type.code);
                nsl_strncpy(stop->routeShortName, agencyRoute->shortName, NB_PLACE_NAME_LEN);
                nsl_strncpy(stop->routeLongName, agencyRoute->longName, NB_PLACE_NAME_LEN);

                // copy hours
                hoursCount = CSL_VectorGetLength(transitStop->hoursOfOperation);
                if (hoursCount > 0)
                {
                    int j = 0;
                    stop->hoursOfOperation = (NB_PublicTransitHours*)nsl_malloc(sizeof(NB_PublicTransitHours) * hoursCount);
                    if (stop->hoursOfOperation != NULL)
                    {
                        stop->hoursOfOperationCount = hoursCount;
                        for (j = 0; j < stop->hoursOfOperationCount; j ++)
                        {
                            NB_PublicTransitHours* stopRouteHours = (NB_PublicTransitHours*)CSL_VectorGetPointer(transitStop->hoursOfOperation, j);
                            if (stopRouteHours != NULL)
                            {
                                CopyHoursStructure(&stop->hoursOfOperation[j], stopRouteHours);
                            }
                        }
                    }
                }
            }
            if (pPlace->stopType == 0)
            {
                pPlace->stopType = stopType;
            }
        }
    }

    return pPlace;
}

static NB_Error GetStopInfo3LevelRequestCallback(NB_PublicTransitManager* pThis, PublicTransitPlace* pTransitPlace, NB_NetworkRequestStatus status, NB_Error err_in)
{
    NB_Error err = NE_OK;
    NB_PublicTransitPlace* pPlace = NULL;

    if (pTransitPlace)
    {
            pPlace = CreatePublicTransitPlace(pThis, pTransitPlace);
            if (pPlace)
            {
                if (pThis->getStopInfoCallback.callback)
                {
                    pThis->getStopInfoCallback.callback(pThis->getStopInfoCallback.callbackData, pPlace);
                }
                pThis->getStopInfoCallback.callback = NULL;
                pThis->getStopInfoCallback.callbackData = NULL;
          }
    }
    return err;
}

static NB_Error AddTransitOverlayStopsRequest(NB_PublicTransitManager*pThis, const char* transitTypes[], int transitTypeCount)
{
    NB_Error err = NE_OK;
    NB_RequestHandlerCallback callbackData = {0};
    NB_SearchHandler*   pSearchHandler = NULL;
    NB_SearchParameters* searchParameters = NULL;
    NB_SearchRegion region = {0};

    if (pThis == NULL || transitTypes == NULL)
    {
        return NE_INVAL;
    }

    callbackData.callback = TransitOverlayStopsRequestHandlerCallback;
    callbackData.callbackData = pThis;

    err = NB_SearchHandlerCreate(pThis->context, &callbackData, &pSearchHandler);
    if (err == NE_OK)
    {
        region.type = NB_ST_BoundingBox;
        region.boundingBox = pThis->queryBoundingBox;

        err = NB_SearchParametersCreatePublicTransitOverlay(pThis->context, &region, NULL, transitTypes, transitTypeCount, 0, 0, pThis->language, &searchParameters);
        err = err ? err : NB_SearchHandlerStartRequest(pSearchHandler, searchParameters);
        NB_SearchParametersDestroy(searchParameters);
        if (err == NE_OK)
        {
            pThis->searchHandler = pSearchHandler;
        }
    }
    return err;
}

static void TransitOverlayStopsRequestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err_in, uint8 up, int percent, void* userData)
{
    NB_Error err = NE_OK;
    NB_PublicTransitManager* pThis = (NB_PublicTransitManager*)userData;
    NB_SearchInformation*   pSearchInformation = NULL;
    NB_PublicTransitPlaceInfo stop = {0};
    NB_Place place = {0};
    uint32 placeId = 0;

    if (status == NB_NetworkRequestStatus_Progress)
    {
        return;
    }
    else if (status == NB_NetworkRequestStatus_Success)
    {
        NB_SearchHandlerGetSearchInformation(handler, &pSearchInformation);
    }
    else
    {
        err = NE_NET;
    }

    CSL_VectorRemoveAll(pThis->placeIdList);

    //handle response
    if (pSearchInformation)
    {
        int resultIndex = 0;
        int count = 0;
        err = NB_SearchInformationGetResultCount(pSearchInformation, &count);
        if (err == NE_OK)
        {
            for (resultIndex = 0; resultIndex < count; resultIndex++)
            {
                err = err ? err : NB_SearchInformationGetPublicTransitPlace(pSearchInformation, resultIndex, &stop);
                if (stop.transitStops == NULL)
                {
                    //skip search results without sops information
                    continue;
                }
                err = err ? err : NB_SearchInformationGetPlace(pSearchInformation, resultIndex, &place, NULL, NULL);
                err = err ? err : AddTransitStop(pThis, &place, &stop, &placeId, FALSE, TRUE);
                NB_SearchInformationReleasePublicTransitPlace(&stop);
                //add search results
                if (err == NE_OK)
                {
                    CSL_VectorAppend(pThis->placeIdList, &placeId);
                }

            }
            //remove duplicates
            CSL_VectorRemoveDuplicates(pThis->placeIdList, &IdCompareFunction);
        }
        NB_SearchInformationDestroy(pSearchInformation);
    }
    CompleteRequestStops(pThis, status, err_in ? err_in : err);

}

static NB_Error PlanTripGetResultData(NB_PublicTransitManager* pThis, NB_SearchHandler* handler, uint32** pPlaceList, int* placeListCount)
{
    NB_Error err = NE_OK;
    NB_SearchInformation* pSearchInformation = NULL;
    uint32* placeList = NULL;
    int resultCount = 0;
    int i = 0;

    if (!handler || !pThis || !pPlaceList || !placeListCount)
    {
        return NE_INVAL;
    }

    *pPlaceList = NULL;
    *placeListCount = 0;

    err = err ? err : NB_SearchHandlerGetSearchInformation(handler, &pSearchInformation);
    err = err ? err : NB_SearchInformationGetResultCount(pSearchInformation, &resultCount);

    if (resultCount > 0)
    {
        placeList = nsl_malloc(resultCount * sizeof(uint32));
        if (placeList == NULL)
        {
            err = NE_NOMEM;
        }
    }

    for (i = 0; i< resultCount; i++)
    {
        NB_PublicTransitPlaceInfo transitStop = {0};
        NB_Place place = {0};
        uint32 placeID = 0;

        if (err != NE_OK)
        {
            break;
        }

        err = err ? err : NB_SearchInformationGetPublicTransitPlace(pSearchInformation, i, &transitStop);
        err = err ? err : NB_SearchInformationGetPlace(pSearchInformation, i ,&place, NULL, NULL);

        //Append the result stops into PT manager
        err = err ? err : AddTransitStop(pThis, &place, &transitStop, &placeID, TRUE, TRUE);

        NB_SearchInformationReleasePublicTransitPlace(&transitStop);

        //Append placeID which was returned by PT manger to result list for client
        placeList[i] = placeID;
    }

    *pPlaceList = placeList;
    *placeListCount = resultCount;

    if (pSearchInformation != NULL)
    {
        NB_Error e = NB_SearchInformationDestroy(pSearchInformation);
        err = err ? err : e;
    }

    return err;
}

static void PlanTripStartRequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_PublicTransitManager* pThis = (NB_PublicTransitManager*)userData;
    PlanTripData* pPlanTripData = &pThis->planTripData;

    if (status == NB_NetworkRequestStatus_Progress)
        return;
    if (status == NB_NetworkRequestStatus_Success)
    {
        if (err == NE_OK)
        {
            err = PlanTripGetResultData(pThis, handler, &pPlanTripData->startTransitPlaceList, &pPlanTripData->startListCount);
        }
    }
    else
    {
        err = NE_NET;
    }

    pPlanTripData->startSearchDone = TRUE;

    if(pPlanTripData->startSearchDone == TRUE && pPlanTripData->endSearchDone == TRUE)
    {
        //Invoke the user callback function
        if (pPlanTripData->callBack != NULL)
        {
            (pPlanTripData->callBack)(err,
                pPlanTripData->startTransitPlaceList,
                pPlanTripData->startListCount,
                pPlanTripData->endTransitPlaceList,
                pPlanTripData->endListCount,
                pPlanTripData->cbData);
        }
    }
}

static void PlanTripEndRequestHandlerCallback(void* handler, NB_RequestStatus status, NB_Error err, nb_boolean up, int percent, void* userData)
{
    NB_PublicTransitManager* pThis = (NB_PublicTransitManager*)userData;
    PlanTripData* pPlanTripData = &pThis->planTripData;

    if (status == NB_NetworkRequestStatus_Progress)
        return;
    if (status == NB_NetworkRequestStatus_Success)
    {
        if (err == NE_OK)
        {
            err = PlanTripGetResultData(pThis, handler, &pPlanTripData->endTransitPlaceList, &pPlanTripData->endListCount);
        }
    }
    else
    {
        err = NE_NET;
    }

    pPlanTripData->endSearchDone = TRUE;

    if(pPlanTripData->startSearchDone == TRUE && pPlanTripData->endSearchDone == TRUE)
    {
        //Invoke the user callback function
        if (pPlanTripData->callBack != NULL)
        {
            (pPlanTripData->callBack)(err,
                pPlanTripData->startTransitPlaceList,
                pPlanTripData->startListCount,
                pPlanTripData->endTransitPlaceList,
                pPlanTripData->endListCount,
                pPlanTripData->cbData);
        }
    }
}

static NB_Error InitializePlanTripData(NB_PublicTransitManager* pThis)
{
    NB_Error err = NE_OK;
    NB_RequestHandlerCallback callBack = {0};
    PlanTripData* pData = NULL;

    if (pThis == NULL)
    {
        return NE_INVAL;
    }

    pData = &pThis->planTripData;
    if (pData == NULL)
    {
        return NE_INVAL;
    }

    callBack.callback = PlanTripStartRequestHandlerCallback;
    callBack.callbackData = pThis;
    err = err ? err : NB_SearchHandlerCreate(pThis->context, &callBack, &pData->pStartSearchHandler);

    callBack.callback = PlanTripEndRequestHandlerCallback;
    callBack.callbackData = pThis;
    err = err ? err : NB_SearchHandlerCreate(pThis->context, &callBack, &pData->pEndSearchHandler);

    ResetPlanTripData(pData);

    return err;
}

static void ReleasePlanTripData(NB_PublicTransitManager* pThis)
{
    PlanTripData* pData = NULL;

    if (pThis == NULL)
    {
        return;
    }

    pData = &pThis->planTripData;
    if (pData == NULL)
    {
        return;
    }

    if (pData->pStartSearchHandler)
    {
        NB_SearchHandlerDestroy(pData->pStartSearchHandler);
        pData->pStartSearchHandler = NULL;
    }

    if (pData->pEndSearchHandler)
    {
        NB_SearchHandlerDestroy(pData->pEndSearchHandler);
        pData->pEndSearchHandler = NULL;
    }
}

static void ResetPlanTripData(PlanTripData* pData)
{
    if(pData)
    {
        pData->language = NULL;
        pData->startTransitPlaceList = NULL;
        pData->startListCount = 0;
        pData->endTransitPlaceList = NULL;
        pData->endListCount = 0;
        pData->callBack = NULL;
        pData->cbData = NULL;
        pData->startSearchDone = FALSE;
        pData->endSearchDone = FALSE;
    }
}

/*! @} */
