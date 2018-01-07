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

    @file     nbgeocodeinfo.c
    @defgroup geocodeinfo GeocodeInformation

    Information about the results of a geocode request
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
#include "nbgeocodeinformation.h"
#include "nbgeocodeinformationprivate.h"
#include "nbgeocodeparameters.h"
#include "nbgeocodeparametersprivate.h"
#include "nbutilityprotected.h"
#include "data_geocode_reply.h"
#include "datautil.h"
#include "vec.h"


struct NB_GeocodeInformation
{
    NB_Context*             context;
    data_geocode_reply      reply;
    NB_GeocodeParameters*   parameters;
};


NB_Error
NB_GeocodeInformationCreate(NB_Context* context, tpselt reply, NB_GeocodeParameters* parameters, NB_GeocodeInformation** information)
{
    NB_GeocodeInformation* pThis = 0;
    NB_Error err = NE_OK;

    if (!context || !reply || !parameters || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    err = data_geocode_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);
    if (!err)
    {
        err = NB_GeocodeParametersClone(parameters, &pThis->parameters);
    }

    if (!err)
    {
        pThis->context = context;
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeInformationGetSliceInformation(NB_GeocodeInformation* pThis, int32* total, uint32* start, uint32* end)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (total)
    {
        *total = pThis->reply.sliceres.total;
    }

    if (start)
    {
        *start = pThis->reply.sliceres.start;
    }

    if (end)
    {
        *end = pThis->reply.sliceres.end;
    }

    return NE_OK;
}


NB_DEF NB_Error
NB_GeocodeInformationGetLocation(NB_GeocodeInformation* pThis, uint32 index, NB_Location* location)
{
    return NB_GeocodeInformationGetLocationWithAccuracy(pThis, index, location, 0);
}


NB_DEF NB_Error
NB_GeocodeInformationGetLocationWithAccuracy(NB_GeocodeInformation* pThis, uint32 index, NB_Location* location, uint32* accuracy)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    data_locmatch* result = 0;

    if (!pThis || !location)
    {
        return NE_INVAL;
    }

    if (index >= (pThis->reply.sliceres.end - pThis->reply.sliceres.start))
    {
        return NE_RANGE;
    }

    /* just in case server returns invalid [start, end) in sliceres */
    if (index >= (uint32)CSL_VectorGetLength(pThis->reply.vec_locmatch))
    {
        return NE_RANGE;
    }

    nsl_memset(location, 0, sizeof(*location));

    dataState = NB_ContextGetDataState(pThis->context);
    result = (data_locmatch*)CSL_VectorGetPointer(pThis->reply.vec_locmatch, index);

    err = SetNIMLocationFromLocMatch(location, dataState, result);

    if (accuracy)
    {
        *accuracy = result->accuracy;
    }

    return err;
}


NB_DEF NB_Error
NB_GeocodeInformationGetLocationExtAppContent(NB_GeocodeInformation* pThis, uint32 index, NB_ExtAppContent* extAppContent)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    data_locmatch* result = 0;

    if (!pThis || !extAppContent)
    {
        return NE_INVAL;
    }

    if (index >= (pThis->reply.sliceres.end - pThis->reply.sliceres.start))
    {
        return NE_RANGE;
    }

    if (index >= (uint32)CSL_VectorGetLength(pThis->reply.vec_locmatch))
    {
        return NE_RANGE;
    }

    dataState = NB_ContextGetDataState(pThis->context);
    result = (data_locmatch*)CSL_VectorGetPointer(pThis->reply.vec_locmatch, index);
    if (!result)
    {
        return NE_NOENT;
    }

    nsl_memset(extAppContent, 0, sizeof(*extAppContent));

    err = SetNIMExtAppContentFromExtAppContentVector(extAppContent, dataState, result->vec_extapp_content);

    return err;
}


NB_DEF NB_Error
NB_GeocodeInformationDestroy(NB_GeocodeInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_geocode_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    if (pThis->parameters)
    {
        NB_GeocodeParametersDestroy(pThis->parameters);
    }

    nsl_free(pThis);

    return NE_OK;
}


NB_GeocodeParameters*
NB_GeocodeInformationGetParameters(NB_GeocodeInformation* pThis)
{
    if (pThis)
    {
        return pThis->parameters;
    }

    return 0;
}


/*! @} */
