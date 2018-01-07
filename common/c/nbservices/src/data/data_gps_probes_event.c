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

    @file     data_gps_probes_event.c
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
#include "data_gps_probes_event.h"

NB_Error
data_gps_probes_event_init(data_util_state* pds, data_gps_probes_event* pgpe)
{
    NB_Error err = NE_OK;

    pgpe->nav_session_id = 0;

    DATA_INIT(pds, err, &pgpe->packing_version, data_string);
    data_string_set(pds, &pgpe->packing_version, "1.1"); /* version 1.1 */

    pgpe->size = sizeof(pgpe->nav_session_id);
    pgpe->size += data_string_get_tps_size(pds, &pgpe->packing_version);

    pgpe->gps_locations = CSL_VectorAlloc(sizeof(NB_GpsLocation));
    if (pgpe->gps_locations == NULL)
    {
        data_gps_probes_event_free(pds, pgpe);
        err = NE_NOMEM;
    }

    return err;
}

void
data_gps_probes_event_free(data_util_state* pds, data_gps_probes_event* pgpe)
{
    if (pgpe->gps_locations)
    {
        CSL_VectorDealloc(pgpe->gps_locations);
        pgpe->gps_locations = NULL;
    }

    pgpe->nav_session_id = 0;

    DATA_FREE(pds, &pgpe->packing_version, data_string);

    pgpe->size = 0;
}

tpselt
data_gps_probes_event_to_tps(data_util_state* pds, data_gps_probes_event* pgpe)
{
    tpselt te = NULL;
    tpselt ce = NULL;
    int i = 0;
    data_unpacked_gps unpacked_gps;
    data_gps_offset gps_offset;
    NB_GpsLocation* lastLocation;
    int len = CSL_VectorGetLength(pgpe->gps_locations);

    te = te_new("gps-probes-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if (len == 0)  /* at least 1 gps has to be present */
    {
        goto errexit;
    }

    if (!te_setattru(te, "nav-session-id", pgpe->nav_session_id))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "packing-version",
                data_string_get(pds, &pgpe->packing_version)))
    {
        goto errexit;
    }

    if (data_unpacked_gps_init(pds, &unpacked_gps) != NE_OK)
    {
        goto errexit;
    }

    lastLocation =
        (NB_GpsLocation*) CSL_VectorGetPointer(pgpe->gps_locations, len-1);

    data_unpacked_gps_from_gpsfix(pds, &unpacked_gps, lastLocation);

    if ((ce = data_unpacked_gps_to_tps(pds, &unpacked_gps)) != NULL && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    data_unpacked_gps_free(pds, &unpacked_gps);

    for (i = len - 2; i >= 0; i--)
    {
        NB_GpsLocation* currentLocation =
            (NB_GpsLocation*) CSL_VectorGetPointer(pgpe->gps_locations, i);

        if (data_gps_offset_init(pds, &gps_offset) != NE_OK)
        {
            goto errexit;
        }

        data_gps_offset_from_gpsfix(pds, &gps_offset, lastLocation,
                currentLocation);

        ce = data_gps_offset_to_tps(pds, &gps_offset);
        if ((ce != NULL) && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            data_gps_offset_free(pds, &gps_offset);
            goto errexit;
        }

        data_gps_offset_free(pds, &gps_offset);
        lastLocation = currentLocation;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_gps_probes_event_equal(data_util_state* pds,
        data_gps_probes_event* pgpe1, data_gps_probes_event* pgpe2)
{
    int ret = TRUE;
    int len1 = CSL_VectorGetLength(pgpe1->gps_locations);
    int len2 = CSL_VectorGetLength(pgpe2->gps_locations);

    ret = ret ? (pgpe1->nav_session_id == pgpe2->nav_session_id): ret;

    DATA_EQUAL(pds, ret, &pgpe1->packing_version, &pgpe2->packing_version,
            data_string);

    ret = ret ? (len1 == len2): ret;

    if (ret)
    {
        int i;
        for (i=0; i< len1; i++)
        {
            if (nsl_memcmp(CSL_VectorGetPointer(pgpe1->gps_locations, i),
                           CSL_VectorGetPointer(pgpe2->gps_locations, i),
                           sizeof(NB_GpsLocation)) != 0)
            {
                ret = FALSE;
                break;
            }
        }
    }


    return (boolean) ret;
}

NB_Error
data_gps_probes_event_copy(data_util_state* pds,
        data_gps_probes_event* pgpe_dest, data_gps_probes_event* pgpe_src)
{
    NB_Error err = NE_OK;
    struct CSL_Vector* copiedVec = NULL;

    DATA_REINIT(pds, err, pgpe_dest, data_gps_probes_event);
    pgpe_dest->nav_session_id = pgpe_src->nav_session_id;

    DATA_COPY(pds, err, &pgpe_dest->packing_version, &pgpe_src->packing_version,
            data_string);
    if (err)
    {
        return err;
    }

    if (CSL_VectorCopy(&copiedVec, pgpe_src->gps_locations, NULL, NULL))
    {
        CSL_VectorDealloc(pgpe_dest->gps_locations);
        pgpe_dest->gps_locations = copiedVec;
        pgpe_dest->size = pgpe_src->size;
    }
    else
    {
        if (copiedVec)
        {
            CSL_VectorDealloc(copiedVec);
            copiedVec = NULL;
        }
        err = NE_NOMEM;
    }

    return err;
}

uint32
data_gps_probes_event_get_tps_size(data_util_state* pds,
            data_gps_probes_event* pgpe)
{
    return pgpe->size;
}

NB_Error
data_gps_probes_event_add_gps(data_util_state* pds,
            data_gps_probes_event* pgpe,
            uint32 nav_session_id,
            NB_GpsLocation* location)
{
    NB_Error err = NE_OK;
    int len = CSL_VectorGetLength(pgpe->gps_locations);

    if (len == 0)   /* the first GPS location */
    {
        if(!CSL_VectorAppend(pgpe->gps_locations, location))
        {
            return NE_NOMEM;
        }
        pgpe->nav_session_id = nav_session_id;
        pgpe->size += GPS_PACKED_LEN;

        return NE_OK;
    }
    else if (nav_session_id != pgpe->nav_session_id)
    {
        return NE_BADDATA;  /* a new event is needed for a new nav session. */
    }
    else
    {
        data_gps_offset gps_offset;

        NB_GpsLocation* lastLocation =
            (NB_GpsLocation*) CSL_VectorGetPointer(pgpe->gps_locations, len-1);

        DATA_INIT(pds, err, &gps_offset, data_gps_offset);
        if (err)
        {
            return err;
        }

        /* verify whether current location can be packed to gps-offset */
        err = data_gps_offset_from_gpsfix(pds, &gps_offset, location,
                lastLocation);
        DATA_FREE(pds, &gps_offset, data_gps_offset);
        if (err)
        {
            return err; /* offset too large */
        }

        if (!CSL_VectorAppend(pgpe->gps_locations, location))
        {
            return NE_NOMEM;
        }

        pgpe->size += GPS_OFFSET_PACKED_LEN;

        return NE_OK;
    }
}

uint32
data_gps_probes_event_get_gps_count(data_gps_probes_event* pgpe)
{
    return CSL_VectorGetLength(pgpe->gps_locations);
}

NB_GpsLocation*
data_gps_probes_event_get_gps(data_gps_probes_event* pgpe, int index)
{
    int len = CSL_VectorGetLength(pgpe->gps_locations);

    if ( (index >= 0) && (index < len))
    {
        return (NB_GpsLocation*) CSL_VectorGetPointer(pgpe->gps_locations,
                                                      index);
    }

    return NULL;
}

void
data_gps_probes_event_to_buf(data_util_state* pds,
            data_gps_probes_event* pgpe,
            struct dynbuf* pdb)
{
    int len = CSL_VectorGetLength(pgpe->gps_locations);

    if (len  > 0)
    {
        int i=0;

        dbufcat(pdb, (const byte*)&len, sizeof(len));

        for (i = 0; i < len; i++)
        {
            NB_GpsLocation* location =
                (NB_GpsLocation*) CSL_VectorGetPointer(pgpe->gps_locations, i);

            dbufcat(pdb, (const byte*)location, sizeof(*location));
        }
    }
    else
    {
        len = 0;
        dbufcat(pdb, (const byte*)&len, sizeof(len));
    }

    dbufcat(pdb, (const byte*)&pgpe->nav_session_id,
            sizeof(pgpe->nav_session_id));

    data_string_to_buf(pds, &pgpe->packing_version, pdb);

    dbufcat(pdb, (const byte*)&pgpe->size, sizeof(pgpe->size));
}

NB_Error
data_gps_probes_event_from_binary(data_util_state* pds,
            data_gps_probes_event* pgpe,
            byte** pdata, size_t* pdatalen)
{

    NB_Error err = NE_OK;
    int len = 0;

    err = err ? err : data_uint32_from_binary(pds, (uint32*)&len,
                                              pdata, pdatalen);

    if (err)
    {
        return err;
    }

    if (len > 0)
    {
        int i;
        NB_GpsLocation location;

        for (i=0; i < len ; i++)
        {
            if (*pdatalen >= sizeof(location))
            {
                nsl_memcpy(&location, *pdata, sizeof(location));
                (*pdata) += sizeof(location);
                (*pdatalen) -= sizeof(location);

                if (!CSL_VectorAppend(pgpe->gps_locations, &location))
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

    err = err ? err : data_uint32_from_binary(pds, &pgpe->nav_session_id,
                                              pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pgpe->packing_version,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pgpe->size,
                                              pdata, pdatalen);

    return err;
}

/*! @} */

