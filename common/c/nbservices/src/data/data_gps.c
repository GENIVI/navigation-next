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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_gps.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_gps.h"

static void
data_gps_clear(data_util_state* pds, data_gps* pg)
{
    
    nsl_memset(pg->packed, 0xFF, sizeof(pg->packed));
}

NB_DEF NB_Error 
data_gps_init(data_util_state* pds, data_gps* pg)
{

    data_gps_clear(pds, pg);
    return NE_OK;
}

NB_DEF void
data_gps_free(data_util_state* pds, data_gps* pg)
{

    data_gps_clear(pds, pg);
}

NB_DEF tpselt
data_gps_to_tps(data_util_state* pds, data_gps* pg)
{
    tpselt te;

    te = te_new("gps");

    if (te == NULL)
        goto errexit;

    if (!te_setattr(te, "packed", (const char*) pg->packed, sizeof(pg->packed)))
        goto errexit;

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_DEF NB_Error
data_gps_from_tps(data_util_state* pds, data_gps* pg, tpselt te)
{
    NB_Error err = NE_OK;
    char* data;
    size_t size;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_gps_free(pds, pg);

    err = data_gps_init(pds, pg);

    if (err != NE_OK)
        return err;

    if (!te_getattr(te, "packed", &data, &size) || data == NULL || size != GPS_PACKED_LEN)
        goto errexit;

    nsl_memcpy(pg->packed, data, sizeof(pg->packed));

    return NE_OK;
errexit:
    data_gps_free(pds, pg);
    return NE_BADDATA;
}

NB_DEF boolean
data_gps_equal(data_util_state* pds, data_gps* pg1, data_gps* pg2)
{
    return (boolean) (nsl_memcpy(pg1->packed, pg2->packed, sizeof(pg1->packed)) == 0);
}

NB_DEF NB_Error
data_gps_copy(data_util_state* pds, data_gps* pg_dest, data_gps* pg_src)
{
    NB_Error err = NE_OK;

    data_gps_free(pds, pg_dest);
    data_gps_init(pds, pg_dest);

    nsl_memcpy(pg_dest->packed, pg_src->packed, sizeof(pg_src->packed));

    return err;
}

NB_DEF void
data_gps_from_gpsfix(data_util_state* pds, data_gps* pg, NB_GpsLocation* pfix)
{
    gpsfix_pack(pfix, pg->packed, sizeof(pg->packed));
}

NB_DEF uint32
data_gps_get_tps_size(data_util_state* pds, data_gps* pg)
{
    return sizeof(pg->packed);
}

NB_DEF void
data_gps_to_buf(data_util_state* pds, data_gps* pg, struct dynbuf* pdb)
{
    int len = 0;

    len = sizeof(pg->packed);

    dbufcat(pdb, (const byte*)&len, sizeof(len));
    dbufcat(pdb, (const byte*)pg->packed, len);
}

NB_DEF NB_Error
data_gps_from_binary(data_util_state* pds, data_gps* pg, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    int len = 0;

    if (*pdatalen < sizeof(len))
    {
        return NE_BADDATA;
    }

    nsl_memcpy(&len, *pdata, sizeof(len));

    *pdata += sizeof(len);
    *pdatalen -= sizeof(len);

    if (len > 0)
    {
        nsl_memcpy(pg->packed, *pdata, len);
        *pdata += len;
        *pdatalen -= len;
    }

    return err;
}


