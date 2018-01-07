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

    @file     nbgeocodeparams.c
    @defgroup nbparams Parameters

    This API is used to create Parameters objects.  Parameters objects are used to create and
    configure Handler objects to retrieve information from the server
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


#include "nbcontextprotected.h"
#include "nbgeocodeinformationprivate.h"
#include "nbgeocodeparameters.h"
#include "nbgeocodeparametersprivate.h"
#include "data_geocode_query.h"
#include "data_geocode_reply.h"
#include "datautil.h"


struct NB_GeocodeParameters
{
    NB_Context*         context;
    uint32              sliceSize;
    data_geocode_query  query;
};


static NB_Error CreateThis(NB_Context* context, const char* type, uint32 size, NB_GeocodeParameters** parameters);
static NB_Error InitializeAddress(NB_GeocodeParameters* pThis, NB_Address* address, data_util_state* dataState);


NB_DEF NB_Error
NB_GeocodeParametersCreateAddress(NB_Context* context, NB_Address* address, uint32 size, NB_GeocodeParameters** parameters)
{
    NB_GeocodeParameters* pThis = 0;
    NB_Error err = NE_OK;

    if (!address || !parameters)
    {
        return NE_INVAL;
    }

    err = CreateThis(context, "street", size, &pThis);
    err = err ? err : InitializeAddress(pThis, address, NB_ContextGetDataState(pThis->context));

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeParametersCreateAirport(NB_Context* context, const char* airport, uint32 size, NB_GeocodeParameters** parameters)
{
    NB_GeocodeParameters* pThis = 0;
    NB_Error err = NE_OK;

    if (!airport|| !parameters)
    {
        return NE_INVAL;
    }

    err = CreateThis(context, "airport", size, &pThis);
    err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.address.airport, airport);

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeParametersCreateFreeForm(NB_Context* context, const char* address, const char* country, uint32 size, NB_GeocodeParameters** parameters)
{
    NB_GeocodeParameters* pThis = 0;
    NB_Error err = NE_OK;

    if (!address || !parameters)
    {
        return NE_INVAL;
    }

    err = CreateThis(context, "freeform", size, &pThis);
    err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.address.freeform, address);
    if (country)
    {
        err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.address.country, country);
    }

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeParametersCreateIntersection(NB_Context* context, NB_Address* address, const char* crossStreet, uint32 size, NB_GeocodeParameters** parameters)
{
    NB_GeocodeParameters* pThis = 0;
    NB_Error err = NE_OK;

    if (!address || !crossStreet || !parameters)
    {
        return NE_INVAL;
    }

    err = CreateThis(context, "intersect", size, &pThis);
    err = err ? err : InitializeAddress(pThis, address, NB_ContextGetDataState(pThis->context));
    err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.address.xstr, crossStreet);

    if (!err)
    {
        *parameters = pThis;
    }
    else if (pThis)
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeParametersCreateIteration(NB_Context* context, NB_GeocodeInformation* information, NB_IterationCommand command, NB_GeocodeParameters** parameters)
{
    NB_Error err = NE_OK;
    NB_GeocodeParameters* pThis = 0;

    if (!context || !parameters)
    {
        return NE_INVAL;
    }

    err = NB_GeocodeParametersClone(NB_GeocodeInformationGetParameters(information), &pThis);
    if (!err)
    {
        int32 total = 0;
        uint32 previousStart = 0;
        uint32 previousEnd = 0;

        err = NB_GeocodeInformationGetSliceInformation(information, &total, &previousStart, &previousEnd);
        if (!err)
        {
            uint32 newStart = 0;

            switch (command)
            {
            case NB_IterationCommand_Next:
                if (total == -1 || (int32)previousEnd < total)
                {
                    newStart = previousEnd;
                }
                else
                {
                    err = NE_RANGE;
                }
                break;

            case NB_IterationCommand_Previous:
                if (previousStart != 0)
                {
                    newStart = previousStart - pThis->sliceSize;
                }
                else
                {
                    err = NE_RANGE;
                }
                break;

            case NB_IterationCommand_Start:
                newStart = 0;
                break;

            default:
                err = NE_INVAL;
            }

            if (!err)
            {
                pThis->query.sliceres.start = newStart;
                pThis->query.sliceres.end = newStart + pThis->sliceSize;

                *parameters = pThis;
            }
            else
            {
                (void)NB_GeocodeParametersDestroy(pThis);
            }
        }
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeParametersSetCountry(NB_GeocodeParameters* pThis, const char* country)
{
    if (!pThis || !country)
    {
        return NE_INVAL;
    }

    return data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.address.country, country);
}


NB_DEF NB_Error
NB_GeocodeParametersDestroy(NB_GeocodeParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_geocode_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);

    nsl_free(pThis);
    return NE_OK;
}


NB_DEF NB_Error
NB_GeocodeParametersSetGeographicPosition(NB_GeocodeParameters* pThis, NB_LatitudeLongitude* point, uint32 accuracy, uint32 time)
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;

    if (!pThis || !point)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(pThis->context);

    result = result ? result : data_string_set(state, &pThis->query.position.variant, "geographic-position");

    if (result == NE_OK)
    {
        pThis->query.position.geographic_position.latitude = point->latitude;
        pThis->query.position.geographic_position.longitude = point->longitude;
        pThis->query.position.geographic_position.accuracy = accuracy;
        pThis->query.position.geographic_position.time = time;
        pThis->query.position.boxValid = FALSE;

        pThis->query.positionValid = TRUE;
    }

    return result;
}


tpselt
NB_GeocodeParametersToTPSQuery(NB_GeocodeParameters* pThis)
{
    if (pThis)
    {
        return data_geocode_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}


NB_Error NB_GeocodeParametersClone(NB_GeocodeParameters* pThis, NB_GeocodeParameters** clone)
{
    NB_Error err = NE_OK;
    NB_GeocodeParameters* pClone = 0;

    if (!pThis || !clone)
    {
        return NE_INVAL;
    }
    *clone = 0;

    pClone = nsl_malloc(sizeof(*pClone));
    if (!pClone)
    {
        return NE_NOMEM;
    }
    nsl_memset(pClone, 0, sizeof(*pClone));

    err = data_geocode_query_copy(NB_ContextGetDataState(pThis->context), &pClone->query, &pThis->query);
    if (!err)
    {
        pClone->context = pThis->context;
        pClone->sliceSize = pThis->sliceSize;
        *clone = pClone;
    }
    else
    {
        nsl_free(pClone);
    }

    return err;
}


NB_Error CreateThis(NB_Context* context, const char* type, uint32 size, NB_GeocodeParameters** parameters)
{
    NB_Error err = NE_OK;
    NB_GeocodeParameters* pThis = 0;
    
    *parameters = 0;
    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;
    pThis->sliceSize = size;

    err = data_geocode_query_init(NB_ContextGetDataState(pThis->context), &pThis->query);
    if (!err)
    {
        err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.address.type, type);

        pThis->query.sliceres.start = 0;
        pThis->query.sliceres.end = size;

        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}


NB_Error InitializeAddress(NB_GeocodeParameters* pThis, NB_Address* address, data_util_state* dataState)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_set(dataState, &pThis->query.address.sa, address->number);
    err = err ? err : data_string_set(dataState, &pThis->query.address.str, address->street);
    err = err ? err : data_string_set(dataState, &pThis->query.address.city, address->city);
    err = err ? err : data_string_set(dataState, &pThis->query.address.county, address->county);
    err = err ? err : data_string_set(dataState, &pThis->query.address.state, address->state);
    err = err ? err : data_string_set(dataState, &pThis->query.address.postal, address->postal);
    err = err ? err : data_string_set(dataState, &pThis->query.address.country, address->country);

    return err;
}


/*! @} */
