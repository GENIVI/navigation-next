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

    @file     data_gps_offset.c
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
#include "data_gps_offset.h"

NB_Error
data_gps_offset_init(data_util_state* pds, data_gps_offset* pgo)
{
    NB_Error err = NE_OK;

    nsl_memset(pgo->packed, 0x0, sizeof(pgo->packed));

    return err;
}

void
data_gps_offset_free(data_util_state* pds, data_gps_offset* pgo)
{
    nsl_memset(pgo->packed, 0x0, sizeof(pgo->packed));
    /* nothing to free */
}

NB_Error
data_gps_offset_from_tps(data_util_state* pds, data_gps_offset* pgo, tpselt te)
{
    NB_Error err = NE_OK;
    char* data;
    size_t size;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_gps_offset_free(pds, pgo);

    err = data_gps_offset_init(pds, pgo);

    if (err != NE_OK)
        return err;

    if (!te_getattr(te, "packed", &data, &size) ||
            data == NULL ||
            (size != GPS_OFFSET_PACKED_LEN))
    {
        goto errexit;
    }

    nsl_memcpy(pgo->packed, data, sizeof(pgo->packed));

errexit:
    if (err != NE_OK)
        data_gps_offset_free(pds, pgo);
    return err;
}

tpselt
data_gps_offset_to_tps(data_util_state* pds, data_gps_offset* pgo)
{
    tpselt te;

    te = te_new("gps-offset");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattr(te, "packed", (const char*) pgo->packed,
                sizeof(pgo->packed)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_gps_offset_equal(data_util_state* pds, data_gps_offset* pgo1,
        data_gps_offset* pgo2)
{
    if (nsl_memcpy(pgo1->packed, pgo2->packed, sizeof(pgo1->packed)) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

NB_Error
data_gps_offset_copy(data_util_state* pds, data_gps_offset* pgo_dest,
        data_gps_offset* pgo_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pgo_dest, data_gps_offset);

    nsl_memcpy(pgo_dest->packed, pgo_src->packed, sizeof(pgo_src->packed));

    return err;
}

uint32
data_gps_offset_get_tps_size(data_util_state* pds, data_gps_offset* pgo)
{
    return sizeof(pgo->packed);
}

#define GPS_TIME_OFFSET_MAX     0xff

#define GPS_LAT_OFFSET_MIN      -0.032767
#define GPS_LAT_OFFSET_MAX      0.032767

#define GPS_LON_OFFSET_MIN      -0.032767
#define GPS_LON_OFFSET_MAX      0.032767

NB_DEF NB_Error
data_gps_offset_from_gpsfix(data_util_state* pds, data_gps_offset* pgo,
        NB_GpsLocation* gps_origin, NB_GpsLocation* gps)
{
    byte time_offset_packed;
    int16 lat_offset_packed;
    int16 lon_offset_packed;
    uint16  gps_heading;
    uint16  gps_hvel;
    uint8   gps_uncang;
    uint8   gps_unca;
    uint8   gps_uncp;

    int time_offset;
    double lat_offset;
    double lon_offset;

    nsl_assert(pgo != NULL);
    nsl_assert(gps_origin != NULL);
    nsl_assert(gps != NULL);

    if (gps->gpsTime < gps_origin->gpsTime)
    {
        return NE_BADDATA;
    }

    time_offset = gps->gpsTime - gps_origin->gpsTime;
    if (time_offset > GPS_TIME_OFFSET_MAX)
    {
        return NE_BADDATA;
    }
    time_offset_packed = (byte)time_offset;

    lat_offset = gps->latitude - gps_origin->latitude;
    if ((lat_offset < GPS_LAT_OFFSET_MIN) ||
        (lat_offset > GPS_LAT_OFFSET_MAX))
    {
        return NE_BADDATA;
    }
    lat_offset_packed = (int16)(lat_offset / 0.000001); /* microdegrees */
    lat_offset_packed = nsl_htons(lat_offset_packed);

    lon_offset = gps->longitude - gps_origin->longitude;
    if ((lon_offset < GPS_LON_OFFSET_MIN) ||
        (lon_offset > GPS_LON_OFFSET_MAX))
    {
        return NE_BADDATA;
    }
    lon_offset_packed = (int16)(lon_offset / 0.000001); /* microdegrees */
    lon_offset_packed = nsl_htons(lon_offset_packed);

    if (gps->valid & NGV_Heading)
    {
        gps_heading = (uint16) (gps->heading / 0.3515625);
    }
    else
    {
        gps_heading = 0xFFFF;
    }
    gps_heading = nsl_htons(gps_heading);

    if (gps->valid & NGV_HorizontalVelocity)
    {
        gps_hvel = (uint16) (gps->horizontalVelocity / 0.25);
    }
    else
    {
        gps_hvel = 0xFFFF;
    }
    gps_hvel = nsl_htons(gps_hvel);

    if (gps->valid & NGV_HorizontalUncertainty)
    {
        gps_uncang = (uint8) (gps->horizontalUncertaintyAngleOfAxis / 5.625);
    }
    else
    {
        gps_uncang = 0xFF;
    }

    if (gps->valid & NGV_AxisUncertainty)
    {

        gps_unca = uncencode(gps->horizontalUncertaintyAlongAxis);
    }
    else
    {
        gps_unca = 0xFF;
    }

    if (gps->valid & NGV_PerpendicularUncertainty)
    {
        gps_uncp = uncencode(gps->horizontalUncertaintyAlongPerpendicular);
    }
    else
    {
        gps_uncp = 0xFF;
    }

    /* pack */
    nsl_memcpy(&pgo->packed[0], &time_offset_packed, 1);
    nsl_memcpy(&pgo->packed[1], &lat_offset_packed, 2);
    nsl_memcpy(&pgo->packed[3], &lon_offset_packed, 2);
    nsl_memcpy(&pgo->packed[5], &gps_heading, 2);
    nsl_memcpy(&pgo->packed[7], &gps_hvel, 2);
    pgo->packed[9] = gps_uncang;
    pgo->packed[10] = gps_unca;
    pgo->packed[11] = gps_uncp;

    return NE_OK;
}
/*! @} */

