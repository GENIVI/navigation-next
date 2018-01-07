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

    @file     nblocationparameters.c

    This API is used to create Parameters objects.  Parameters objects are used to create and
    configure Handler objects to retrieve information from the server
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/


#include "nblocationparameters.h"
#include "nblocationparametersprivate.h"
#include "nbcontextprotected.h"
#include "nbqalog.h"
#include "data_location_query.h"
#include "data_location_reply.h"

/*! @{ */

struct NB_LocationParameters
{
    NB_Context*           context;
    data_location_query   query;
};

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationParametersCreate(NB_Context* context, NB_LocationParameters** parameters)
{
    NB_LocationParameters* pThis = 0;
    NB_Error err = NE_OK;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    err = data_location_query_init(NB_ContextGetDataState(context), &pThis->query);
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

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationParametersDestroy(NB_LocationParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_location_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationParametersAddGsm(
    NB_LocationParameters* pThis,
    uint32 mobileCountryCode,
    uint32 mobileNetworkCode,
    uint32 locationAreaCode,
    uint32 cellId,
    int16 signalStrength)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!data_location_query_add_gsm(NB_ContextGetDataState(pThis->context), &pThis->query, mobileCountryCode, mobileNetworkCode, locationAreaCode, cellId, signalStrength))
    {
        return NE_INVAL;
    }

    return NE_OK;
}

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationParametersAddCdma(
    NB_LocationParameters* pThis,
    uint32 systemId,
    uint32 networkId,
    uint32 cellId,
    int16 signalStrength)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!data_location_query_add_cdma(NB_ContextGetDataState(pThis->context), &pThis->query, systemId, networkId, cellId, signalStrength))
    {
        return NE_INVAL;
    }

    return NE_OK;
}

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationParametersAddWifi(
    NB_LocationParameters* pThis,
    const char* macAddress,
    int16 signalStrength)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!data_location_query_add_wifi(NB_ContextGetDataState(pThis->context), &pThis->query, macAddress, signalStrength))
    {
        return NE_INVAL;
    }

    return NE_OK;
}

/* See header file for description. */ 
NB_DEF tpselt
NB_LocationParametersToTPSQuery(NB_LocationParameters* pThis)
{
    int l = 0;
    int i = 0;
    
    if (pThis)
    {
        if (pThis->query.vec_gsm != NULL)
        {
            l = CSL_VectorGetLength(pThis->query.vec_gsm);
            for (i = 0; i < l; i++)
            {
                data_gsm *pg = CSL_VectorGetPointer(pThis->query.vec_gsm, i);
                NB_QaLogCellIdRequest(
                    pThis->context, NB_QLLNT_GSM, pg->signalStrength,
                    pg->mcc, pg->mnc, pg->lac, pg->cellid);
            }
        }

        if (pThis->query.vec_cdma != NULL)
        {
            l = CSL_VectorGetLength(pThis->query.vec_cdma);
            for (i = 0; i < l; i++)
            {
                data_cdma *pc = CSL_VectorGetPointer(pThis->query.vec_cdma, i);
                NB_QaLogCellIdRequest(
                    pThis->context, NB_QLLNT_CDMA, pc->signalStrength,
                    pc->sid, pc->nid, 0, pc->cellid);
            }
        }

        if (pThis->query.vec_wifi != NULL)
        {
            l = CSL_VectorGetLength(pThis->query.vec_wifi);
            for (i = 0; i < l; i++)
            {
                data_wifi *pw = CSL_VectorGetPointer(pThis->query.vec_wifi, i);
                NB_QaLogCellIdWifiRequest(pThis->context, pw->signalStrength, pw->macAddress);
            }
        }

        return data_location_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}

NB_DEF NB_Error
NB_LocationParametersSetProxyApiKey( NB_LocationParameters* parameters, const char* key )
{    
    NB_Error err = NE_OK;

    if ( !parameters || !key )
    {
        return NE_INVAL;
    }
    data_proxy_api_key_free( NB_ContextGetDataState( parameters->context ),
            &parameters->query.proxy_api_key );

    err = data_proxy_api_key_init( NB_ContextGetDataState( parameters->context ),
            &parameters->query.proxy_api_key );

    err = err ? err : data_string_set( NB_ContextGetDataState( parameters->context ),
            &parameters->query.proxy_api_key.apiKey, key );

    if ( err != NE_OK )
    {
        data_proxy_api_key_free( NB_ContextGetDataState( parameters->context ),
              &parameters->query.proxy_api_key );
    }
    return err;
}

NB_DEF NB_Error
NB_LocationParametersAskWanted( NB_LocationParameters* parameters,
                                boolean wantLocationTiles,
                                boolean wantLocationSetting
                              )
{
    if ( !parameters )
    {
        return NE_INVAL;
    }

    parameters->query.want_location_tiles = wantLocationTiles;
    parameters->query.want_location_setting = wantLocationSetting;
    return NE_OK;
}

NB_DEF NB_Error
NB_LocationParametersAddMovingWifi( NB_LocationParameters* parameters,
                                    const char* macAddress,
                                    NB_CauseLocationMovedPointInvalidation cause,
                                    uint32 time
                                  )
{
    if ( !parameters )
    {
        return NE_INVAL;
    }

    if ( !data_location_query_add_location_moved_point( NB_ContextGetDataState( parameters->context ), 
                  &parameters->query, macAddress, (uint8) cause, time ) )
    {
        return NE_INVAL;
    }

    return NE_OK;
}
/*! @} */
