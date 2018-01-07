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

    @file     nbpointsofinterestinformation.c
    */
    /*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "palmath.h"

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbutility.h"
#include "nbpointsofinterestinformation.h"
#include "nbsearchinformation.h"
#include "nbsearchinformationprivate.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstateprivate.h"

struct NB_PointsOfInterestInformation
{
    NB_Context*         context;
    struct CSL_Vector*  vec_pois;
};

static NB_PointsOfInterestInformation* AllocatePointOfInterestInformation(NB_Context* context);
static NB_PointOfInterest* GetPointOfInterestByIndex(NB_PointsOfInterestInformation* information, uint32 index);

#if 0
static nb_boolean IsPointOfInterestAlreadyPresent(NB_PointsOfInterestInformation* information, NB_PointOfInterest* poi);
#endif

NB_DEF NB_Error
NB_PointsOfInterestInformationCreate(NB_Context* context, NB_PointsOfInterestInformation** information)
{
    NB_PointsOfInterestInformation* pThis = 0;

    if (!context || !information)
    {
        return NE_INVAL;
    }

    pThis = AllocatePointOfInterestInformation(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }
    *information = pThis;

    return NE_OK;
}

NB_DEF NB_Error 
NB_PointsOfInterestInformationDestroy(NB_PointsOfInterestInformation* information)
{
    if (!information)
    {
        return NE_INVAL;
    }

    if (information->vec_pois) 
    {
        int count = CSL_VectorGetLength(information->vec_pois);
        int i = 0;
        
        for (i = 0; i < count; i++)
        {
            NB_PointOfInterest* poi = CSL_VectorGetPointer(information->vec_pois, i);
            if (poi && poi->extendedPlace)
            {
                FreeNIMExtendedPlace(NB_ContextGetDataState(information->context), poi->extendedPlace);
            }
        }
        CSL_VectorDealloc(information->vec_pois);
    }

    nsl_free(information);

    return NE_OK;
}

NB_DEF
NB_Error NB_PointsOfInterestInformationUpdateWithSearch(NB_PointsOfInterestInformation* information, NB_Navigation* navigation, NB_SearchInformation* search)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;
    int newSearchCount = 0;
    int i = 0;

    if (!information || !search)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(information->context);

    err = NB_SearchInformationGetResultCount(search, &newSearchCount);
    if (err)
    {
        return err;
    }

    // remove POIs in earlier search result
    while (CSL_VectorGetLength(information->vec_pois))
    {
        NB_PointOfInterest* poi = CSL_VectorGetPointer(information->vec_pois, 0);

        if (poi->extendedPlace)
        {
            FreeNIMExtendedPlace(dataState, poi->extendedPlace);
        }

        CSL_VectorRemove(information->vec_pois, 0);
    }

    // append new POI search results
    for (i = 0; i < newSearchCount; i++)
    {
        NB_PointOfInterest poi = {{{ 0 }}};
        NB_ExtendedPlace* extendedPlace = NULL;
        double distance = 0.0;

        err =  NB_SearchInformationGetPlace(search, i, &poi.place, &distance, &extendedPlace);

        if (extendedPlace)
        {
            err = err ? err : NB_ExtendedPlaceClone(dataState, extendedPlace, &poi.extendedPlace);
        }

        if (navigation)
        {
            poi.routeDistanceRemaining = navigation->poiLastQueryRouteRemaining - distance;
        }

        if (!err)
        {
            CSL_VectorAppend(information->vec_pois, &poi);
        }
    }

    return err;
}

NB_DEF
NB_Error NB_PointsOfInterestInformationUpdateWithSingleSearch(NB_PointsOfInterestInformation* information, NB_Navigation* navigation, NB_SingleSearchInformation* search)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;

    uint32 newSearchCount = 0;
    int i = 0;

    if (!information || !search)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(information->context);

    err = NB_SingleSearchInformationGetResultCount(search, &newSearchCount);
    if (err)
    {
        return err;
    }

    // remove POIs in earlier search result
    while (CSL_VectorGetLength(information->vec_pois))
    {
        NB_PointOfInterest* poi = CSL_VectorGetPointer(information->vec_pois, 0);

        if (poi->extendedPlace)
        {
            FreeNIMExtendedPlace(dataState, poi->extendedPlace);
        }

        CSL_VectorRemove(information->vec_pois, 0);
    }

    // append new POI search results
    for (i = 0; i < (int)newSearchCount; i++)
    {
        NB_PointOfInterest poi = {{{ 0 }}};
        NB_ExtendedPlace* extendedPlace = NULL;
        double distance = 0.0;

        err =  NB_SingleSearchInformationGetPlace(search, i, &poi.place, &distance, &extendedPlace);

        if (extendedPlace)
        {
            err = err ? err : NB_ExtendedPlaceClone(dataState, extendedPlace, &poi.extendedPlace);
        }

        if (navigation)
        {
            poi.routeDistanceRemaining = navigation->poiLastQueryRouteRemaining - distance;
        }

        err = err ? err : CSL_VectorAppend(information->vec_pois, &poi) ? NE_OK : NE_NOMEM;

    }

    return err;
}

NB_DEF 
NB_Error NB_PointsOfInterestInformationGetPointOfInterestCount(NB_PointsOfInterestInformation* information, int* count)
{    
    if (!information || !count)
    {
        return NE_INVAL;
    }

    *count = (uint32) CSL_VectorGetLength(information->vec_pois);

    return NE_OK;
}

NB_DEF NB_Error 
NB_PointsOfInterestInformationGetPointOfInterest(NB_PointsOfInterestInformation* information, uint32 index, NB_PointOfInterest* poiOut)
{
    NB_PointOfInterest* poi = 0;

    if (!information || !poiOut)
    {
        return NE_INVAL;
    }

    poi = GetPointOfInterestByIndex(information, index);
    if (poi)
    {
        nsl_memcpy(poiOut, poi, sizeof(*poi));
    }
    else
    {
        return NE_INVAL;
    }
    return NE_OK;
}

static NB_PointsOfInterestInformation*
AllocatePointOfInterestInformation(NB_Context* context)
{
    NB_PointsOfInterestInformation* pThis = NULL;

    if ((pThis = nsl_malloc(sizeof(NB_PointsOfInterestInformation))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(NB_PointsOfInterestInformation));

    pThis->context = context;
    pThis->vec_pois = CSL_VectorAlloc(sizeof(NB_PointOfInterest));
    
    return pThis;
}

static NB_PointOfInterest* 
GetPointOfInterestByIndex(NB_PointsOfInterestInformation* information, uint32 index)
{
    if (!information)
    {
        return NULL;
    }

    if (index > (uint32) CSL_VectorGetLength(information->vec_pois))
    {
        return NULL;
    }

    return (NB_PointOfInterest*) CSL_VectorGetPointer(information->vec_pois, index);
}

#if 0
static nb_boolean
IsPointOfInterestAlreadyPresent(NB_PointsOfInterestInformation* information, NB_PointOfInterest* poi)
{
    int count = 0;
    int index = 0;
    
    if  (!information || !poi)
    {
        return FALSE;
    }

    count = CSL_VectorGetLength(information->vec_pois);
    for (index = 0; index < count; index++)
    {
        NB_PointOfInterest* poiCurrent = CSL_VectorGetPointer(information->vec_pois, index);
        
        if (poi->place.location.latitude == poiCurrent->place.location.latitude
            && poi->place.location.longitude == poiCurrent->place.location.longitude
            && !nsl_strcmp(poi->place.name, poiCurrent->place.name))
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif

/*! @} */
