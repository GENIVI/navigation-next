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

    @file     data_location_query.c
    
    Implemention for TPS queries to the Location servlet.
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_location_query.h"

NB_Error
data_location_query_init(data_util_state* pds, data_location_query* plq)
{
    NB_Error err = NE_OK;

    plq->vec_gsm = NULL;
    plq->vec_cdma = NULL;
    plq->vec_wifi = NULL;

    plq->vec_moving_wifi = NULL;
    plq->want_location_tiles = FALSE;
    plq->want_location_setting = FALSE;
    err = err ? err : data_proxy_api_key_init( pds, &plq->proxy_api_key );

    return err;
}

void
data_location_query_free(data_util_state* pds, data_location_query* plq)
{
    DATA_VEC_FREE( pds, plq->vec_gsm, data_gsm );

    DATA_VEC_FREE( pds, plq->vec_cdma, data_cdma );

    DATA_VEC_FREE( pds, plq->vec_wifi, data_wifi );

    DATA_VEC_FREE( pds, plq->vec_moving_wifi, data_location_moved_point );

    data_proxy_api_key_free( pds, &plq->proxy_api_key );
}

tpselt
data_location_query_to_tps(data_util_state* pds, data_location_query* plq)
{
    tpselt te = NULL;
    tpselt ce = NULL;
    tpselt cce = NULL;

    int i = 0;
    int l = 0;

    te = te_new("location-query");

    if (te == NULL)
    {
        goto errexit;
    }

    /* 
      network-type
        This element is optional only if one or more location-moved-point elements are present.
        Request can contain CDMA/GSM data and/or Wi-Fi data. 
    */
    if ( plq->vec_cdma != NULL )
    {
        l = CSL_VectorGetLength( plq->vec_cdma );
        for ( i = 0; i < l; i++ )
        {
            data_cdma* pcdma = CSL_VectorGetPointer( plq->vec_cdma, i );

            if ( ( ( cce = data_cdma_to_tps( pds, pcdma ) ) == NULL ) ||
                 ( ce == NULL && ( ce = te_new( "network-type" ) ) == NULL ) ||
                 !te_attach( ce, cce ) )
            {
                goto errexit;
            }
            cce = NULL;
       }
    }

    if ( plq->vec_gsm != NULL && ce == NULL )
    {
        l = CSL_VectorGetLength( plq->vec_gsm );
        for ( i = 0; i < l; i++ )
        {
            data_gsm* pgsm = CSL_VectorGetPointer( plq->vec_gsm, i );

            if ( ( ( cce = data_gsm_to_tps( pds, pgsm ) ) == NULL ) ||
                 ( ce == NULL && ( ce = te_new( "network-type" ) ) == NULL ) ||
                 !te_attach( ce, cce ) )
            {
                goto errexit;
            }
            cce = NULL;
        }
    }

    if ( ce != NULL )
    {
        if ( !te_attach( te, ce ) )
        {
            goto errexit;
        }
        ce = NULL;
    }

    if ( plq->vec_wifi != NULL )
    {
        l = CSL_VectorGetLength( plq->vec_wifi );
        for ( i = 0; i < l; i++ )
        {
            data_wifi* pwifi = CSL_VectorGetPointer( plq->vec_wifi, i );

            if ( ( ( cce = data_wifi_to_tps( pds, pwifi ) ) == NULL ) ||
                 ( ce == NULL && ( ce = te_new( "network-type" ) ) == NULL ) ||
                 !te_attach( ce, cce ) )
            {
                goto errexit;
            }
            cce = NULL;
        }
    }

    if ( ce != NULL )
    {
        if ( !te_attach( te, ce ) )
        {
            goto errexit;
        }
        ce = NULL;
    }

    if ( plq->want_location_tiles )
    {
        if ( ( ce = te_new( "want-location-tiles" ) ) == NULL || !te_attach( te, ce ) )
        {
            goto errexit;
        }
        ce = NULL;
    }

    if ( plq->want_location_setting )
    {
        if ( ( ce = te_new( "want-location-setting" ) ) == NULL || !te_attach( te, ce ) )
        {
            goto errexit;
        }
        ce = NULL;
    }

    ce = data_proxy_api_key_to_tps( pds, &plq->proxy_api_key );
    if ( ce != NULL )
    {
        if ( !te_attach( te, ce ) )
        {
            goto errexit; 
        }
        ce = NULL;
    }

    DATA_VEC_TO_TPS( pds, errexit, te, plq->vec_moving_wifi, data_location_moved_point );

    return te;

errexit:
    te_dealloc(te);
    te_dealloc(ce);
    te_dealloc(cce);
    return NULL;
}

boolean
data_location_query_add_gsm(data_util_state* pds, data_location_query* plq, uint32 mcc, uint32 mnc, uint32 lac, uint32 cellid, int16 signalStrength)
{
    NB_Error err = NE_OK;
    data_gsm  gsm = { 0 };

    err = err ? err : data_gsm_init(pds, &gsm);

    gsm.mcc = mcc;
    gsm.mnc = mnc;
    gsm.lac = lac;
    gsm.cellid = cellid;
    gsm.signalStrength = signalStrength;

    if (plq->vec_gsm == NULL)
    {
        DATA_VEC_ALLOC( err, plq->vec_gsm, data_gsm );
    }

    err = err ? err : CSL_VectorAppend(plq->vec_gsm, &gsm) ? NE_OK : NE_NOMEM;

    if (err)
    {
        data_gsm_free(pds, &gsm);
    }
    
    return (boolean)(err == NE_OK);
}

boolean
data_location_query_add_cdma(data_util_state* pds, data_location_query* plq, uint32 sid, uint32 nid, uint32 cellid, int16 signalStrength)
{
    NB_Error err = NE_OK;
    data_cdma cdma = { 0 };

    err = err ? err : data_cdma_init(pds, &cdma);
    
    cdma.sid = sid;
    cdma.nid = nid;
    cdma.cellid = cellid;
    cdma.signalStrength = signalStrength;

    if (plq->vec_cdma == NULL)
    {
        DATA_VEC_ALLOC( err, plq->vec_cdma, data_cdma );
    }

    err = err ? err : CSL_VectorAppend(plq->vec_cdma, &cdma) ? NE_OK : NE_NOMEM;

    if (err)
    {
        data_cdma_free(pds, &cdma);
    }

    return (boolean)(err == NE_OK);
}

boolean
data_location_query_add_wifi(data_util_state* pds, data_location_query* plq, const char* macAddress, int16 signalStrength)
{
    NB_Error err = NE_OK;
    data_wifi wifi = { 0 };

    err = err ? err : data_wifi_init(pds, &wifi);
    
    err = err ? err : data_string_set(pds, &wifi.macAddress, macAddress);
    wifi.signalStrength = signalStrength;

    if (plq->vec_wifi == NULL)
    {
        DATA_VEC_ALLOC( err, plq->vec_wifi, data_wifi );
        if (plq->vec_wifi == NULL)
        {
            err = NE_NOMEM;
        }
    }

    err = err ? err : CSL_VectorAppend(plq->vec_wifi, &wifi) ? NE_OK : NE_NOMEM;

    if (err)
    {
        data_wifi_free(pds, &wifi);
    }
    
    return (boolean)(err == NE_OK);
}

boolean
data_location_query_add_location_moved_point( data_util_state* pds,
                                              data_location_query* plq,
                                              const char* macAddress,
                                              uint8 cause,
                                              uint32 timeStamp
                                             )
{
    NB_Error err = NE_OK;
    data_location_moved_point wifi = { 0 };

    if ( !macAddress || timeStamp == 0 )
    {
        return FALSE;
    }

    err = err ? err : data_location_moved_point_init(pds, &wifi);

    err = err ? err : data_string_set(pds, &wifi.macAddress, macAddress);
    wifi.cause = cause;
    wifi.timeStamp = timeStamp;

    if (plq->vec_moving_wifi == NULL)
    {
        DATA_VEC_ALLOC( err, plq->vec_moving_wifi, data_location_moved_point );
    }

    err = err ? err : CSL_VectorAppend(plq->vec_moving_wifi, &wifi) ? NE_OK : NE_NOMEM;

    if (err)
    {
        data_location_moved_point_free(pds, &wifi);
    }

    return (boolean)(err == NE_OK);
}

/*! @} */
