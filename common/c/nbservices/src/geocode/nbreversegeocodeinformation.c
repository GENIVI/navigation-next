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

    @file     nbreversegeocodeinformation.c
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
#include "nbreversegeocodeinformation.h"
#include "nbreversegeocodeinformationprivate.h"
#include "nbutilityprotected.h"
#include "data_reverse_geocode_reply.h"
#include "datautil.h"


struct NB_ReverseGeocodeInformation
{
    NB_Context*         context;
    NB_Location        location;
};


NB_Error
NB_ReverseGeocodeInformationCreate(NB_Context* context, tpselt reply, NB_ReverseGeocodeInformation** information)
{
    NB_ReverseGeocodeInformation* pThis = 0;
    NB_Error err = NE_OK;
    data_util_state* dataState = 0;
    data_reverse_geocode_reply replyData;
    nsl_memset(&replyData, 0, sizeof(data_reverse_geocode_reply));

    if (!context || !reply || !information)
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
    dataState = NB_ContextGetDataState(context);

    err = data_reverse_geocode_reply_from_tps(dataState, &replyData, reply);
    err = err ? err : SetNIMLocationFromLocation(&pThis->location, dataState, &replyData.location);
    data_reverse_geocode_reply_free(dataState, &replyData);

    if (!err)
    {
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}


NB_DEF NB_Error
NB_ReverseGeocodeInformationGetLocation(NB_ReverseGeocodeInformation* pThis, NB_Location* location)
{
    if (!pThis || !location)
    {
        return NE_INVAL;
    }

    *location = pThis->location;

    return NE_OK;
}


NB_DEF NB_Error
NB_ReverseGeocodeInformationDestroy(NB_ReverseGeocodeInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    nsl_free(pThis);

    return NE_OK;
}


/*! @} */
