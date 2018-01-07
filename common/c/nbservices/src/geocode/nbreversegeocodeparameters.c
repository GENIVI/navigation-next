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

    @file     nbreversegeocodeparams.c
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
#include "nbreversegeocodeparameters.h"
#include "nbreversegeocodeparametersprivate.h"
#include "data_reverse_geocode_query.h"
#include "datautil.h"


struct NB_ReverseGeocodeParameters {
    NB_Context*                 context;
    NB_LatitudeLongitude        point;
    boolean                     hasMapScale;
    double                      mapScale;
    data_reverse_geocode_query  query;
};


NB_DEF NB_Error
NB_ReverseGeocodeParametersCreate(NB_Context* context, NB_LatitudeLongitude* point, nb_boolean routable, NB_ReverseGeocodeParameters** parameters)
{
    return NB_ReverseGeocodeParametersCreateWithScale(context, point, routable, 0.0, parameters);
}


NB_DEF NB_Error
NB_ReverseGeocodeParametersCreateWithScale(NB_Context* context, NB_LatitudeLongitude* point, nb_boolean routable, double scale, NB_ReverseGeocodeParameters** parameters)
{
    NB_ReverseGeocodeParameters* pThis = 0;
    data_util_state* dataState = 0;
    NB_Error err = NE_OK;

    if (!point || !parameters)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    dataState = NB_ContextGetDataState(pThis->context);
    err = data_reverse_geocode_query_init(dataState, &pThis->query);

    pThis->query.scale = scale;
    pThis->query.routeable = routable;

    data_string_set(dataState, &pThis->query.position.variant , "point");
    pThis->query.position.point.lat = point->latitude;
    pThis->query.position.point.lon = point->longitude;

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_ReverseGeocodeParametersSetAvoidLocalRoad(NB_Context* context, NB_ReverseGeocodeParameters* parameters, nb_boolean avoidLocalRoad)
{
    if (!context || !parameters)
    {
        return NE_INVAL;
    }

    parameters->query.avoidLocalRoad = (boolean)avoidLocalRoad;

    return NE_OK;
}

NB_DEF NB_Error
NB_ReverseGeocodeParametersDestroy(NB_ReverseGeocodeParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_reverse_geocode_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}

tpselt NB_ReverseGeocodeParametersToTPSQuery(NB_ReverseGeocodeParameters* pThis)
{
    return data_reverse_geocode_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
}
