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

    @file     data_wifi_capabilities.c
    
    Implemention for Wifi-Capabilities TPS element.
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

#include "data_wifi_capabilities.h"

NB_Error
data_wifi_capabilities_init(data_util_state* pds, data_wifi_capabilities* pwc)
{
    NB_Error err = NE_OK;

    DATA_VEC_ALLOC(err, pwc->vec_pairs, data_pair);

    return err;
}

void
data_wifi_capabilities_free(data_util_state* pds, data_wifi_capabilities* pwc)
{
    DATA_VEC_FREE(pds, pwc->vec_pairs, data_pair);
}

tpselt
data_wifi_capabilities_to_tps(data_util_state* pds, data_wifi_capabilities* pwc)
{
    tpselt tpsElement = NULL;

    tpsElement = te_new("wifi-capabilities");

    if (tpsElement == NULL)
    {
        goto errexit;
    }

    DATA_VEC_TO_TPS(pds, errexit, tpsElement, pwc->vec_pairs, data_pair);

    return tpsElement;
 
errexit:

    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_wifi_capabilities_equal(data_util_state* pds, data_wifi_capabilities* pwc1,
                             data_wifi_capabilities* pwc2)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, pwc1->vec_pairs, pwc2->vec_pairs, data_pair);

    return (boolean) ret;
}

NB_Error
data_wifi_capabilities_copy(data_util_state* pds, data_wifi_capabilities* pwc_dest,
                            data_wifi_capabilities* pwc_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pwc_dest, data_wifi_capabilities);

    DATA_VEC_COPY(pds, err, pwc_dest->vec_pairs, pwc_src->vec_pairs, data_pair);

    return err;
}

uint32
data_wifi_capabilities_get_tps_size(data_util_state* pds, data_wifi_capabilities* pwc)
{
    uint32 size = 0;
    int i = 0;
    int length = CSL_VectorGetLength(pwc->vec_pairs);

    for(i = 0; i < length; i++)
    {
        size += data_pair_get_tps_size(pds, CSL_VectorGetPointer(pwc->vec_pairs, i));
    }

    return size;
}

void
data_wifi_capabilities_to_buf(data_util_state* pds, data_wifi_capabilities* pwc, struct dynbuf* pdb)
{
    uint32 length = CSL_VectorGetLength(pwc->vec_pairs);

    if (length  > 0)
    {
        uint32 i = 0;

        dbufcat(pdb, (const byte*)&length, sizeof(length));

        for (i = 0; i < length; i++)
        {
            data_pair* pair =
                CSL_VectorGetPointer(pwc->vec_pairs, i);

            data_pair_to_buf(pds, pair, pdb);
        }
    }
    else
    {
        length = 0;
        dbufcat(pdb, (const byte*)&length, sizeof(length));
    }
}

NB_Error
data_wifi_capabilities_from_binary(data_util_state* pds, data_wifi_capabilities* pwc, byte** pdata,
                                   size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 length = 0;

    err = err ? err : data_uint32_from_binary(pds, &length, pdata, pdatalen);

    if (err == NE_OK && length > 0)
    {
        uint32 i = 0;
        
        for (i = 0; i < length ; i++)
        {
            data_pair pair;

            if (*pdatalen >= sizeof(pair))
            {
                err = err ? err : data_pair_init(pds, &pair);
                err = err ? err : data_pair_from_binary(pds, &pair, pdata, pdatalen);

                if(err)
                {
                    break;
                }

                if (!CSL_VectorAppend(pwc->vec_pairs, &pair))
                {
                    err = NE_NOMEM;
                    break;
                }
            }
            else
            {
                err = NE_BADDATA;
                break;
            }
        }
    }

    return err;
}

NB_Error
data_wifi_capabilities_add_pair(data_util_state* pds, data_wifi_capabilities* pwc,
                                const char* key, const char* value)
{
    NB_Error err = NE_OK;

    data_pair pair = {0};

    err = err ? err : data_pair_init(pds, &pair);

    err = err ? err : data_pair_set(pds, &pair, key, value);

    if(err == NE_OK)
    {
        if (!CSL_VectorAppend(pwc->vec_pairs, &pair))
        {
            err = NE_NOMEM;
        }
    }

    return err;
}

/*! @} */