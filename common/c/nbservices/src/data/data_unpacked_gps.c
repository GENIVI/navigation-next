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

 @file     data_unpacked_gps.c
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/* @{ */

#include <math.h>
#include "data_unpacked_gps.h"

NB_Error data_unpacked_gps_init(data_util_state* pds, data_unpacked_gps* pdug)
{

    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pdug->point, data_point);
    if (err != NE_OK)
    {
        return err;
    }

    pdug->time = 0;
    pdug->heading = 0;
    pdug->speed = 0;
    pdug->altitude = 0;
    pdug->umag = 0;
    pdug->uang = 0;
    pdug->ua = 0;
    pdug->up = 0;

    return err;
}

void data_unpacked_gps_free(data_util_state* pds, data_unpacked_gps* pdug)
{
    DATA_FREE(pds, &pdug->point, data_point);

    pdug->time = 0;
    pdug->heading = 0;
    pdug->speed = 0;
    pdug->altitude = 0;
    pdug->umag = 0;
    pdug->uang = 0;
    pdug->ua = 0;
    pdug->up = 0;
}

tpselt data_unpacked_gps_to_tps(data_util_state* pds, data_unpacked_gps* pdug)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("gps");
    if (te == NULL)
    {
        goto errexit;
    }

    if ((ce = data_point_to_tps(pds, &pdug->point)) != NULL && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if (!te_setattru64(te, "time", pdug->time))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "heading", pdug->heading))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "speed", pdug->speed))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "altitude", pdug->altitude))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "umag", pdug->umag))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "uang", pdug->uang))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "ua", pdug->uang))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "up", pdug->up))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

#define EPSINON 1e-6
boolean data_unpacked_gps_equal(data_util_state* pds, data_unpacked_gps* pdug1, data_unpacked_gps* pdug2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pdug1->point, &pdug2->point, data_point);
    if (!ret)
    {
        goto errexit;
    }

    if (llabs(pdug1->time - pdug2->time))
    {
        goto errexit;
    }

    if (fabsf(pdug1->heading - pdug2->heading) > EPSINON)
    {
        goto errexit;
    }

    if (fabsf(pdug1->speed - pdug2->speed) > EPSINON)
    {
        goto errexit;
    }

    if (fabsf(pdug1->altitude - pdug2->altitude) > EPSINON)
    {
        goto errexit;
    }

    if (fabsf(pdug1->umag - pdug2->umag) > EPSINON)
    {
        goto errexit;
    }

    if (fabsf(pdug1->uang - pdug2->uang) > EPSINON)
    {
        goto errexit;
    }

    if (fabsf(pdug1->up - pdug2->up) > EPSINON)
    {
        goto errexit;
    }

    return (boolean)ret;

errexit:

    ret = FALSE;
    return (boolean)ret;
}
#undef EPSINON

NB_Error data_unpacked_gps_copy(data_util_state* pds, data_unpacked_gps* pdug_dest, data_unpacked_gps* pdug_src)
{
    NB_Error err = NE_OK;

    DATA_COPY(pds, err, &pdug_dest->point, &pdug_src->point, data_point);
    if (err != NE_OK)
    {
        goto errexit;
    }

    pdug_dest->time = pdug_src->time;
    pdug_dest->heading = pdug_src->heading;
    pdug_dest->speed = pdug_src->speed;
    pdug_dest->altitude = pdug_src->altitude;
    pdug_dest->umag = pdug_src->umag;
    pdug_dest->uang = pdug_src->uang;
    pdug_dest->ua = pdug_src->ua;
    pdug_dest->up = pdug_src->up;

errexit:

    return err;
}

void data_unpacked_gps_from_gpsfix(data_util_state* pds, data_unpacked_gps* pg, NB_GpsLocation* pfix)
{
    if (pfix->status != NE_OK)
    {
        return;
    }

    pg->point.lat = pfix->latitude;
    pg->point.lon = pfix->longitude;
    pg->altitude = (float)pfix->altitude;
    pg->heading = (float)pfix->heading;
    pg->speed = (float)pfix->horizontalVelocity;
    pg->time = pfix->gpsTime;
    pg->ua = (float)pfix->horizontalUncertaintyAlongAxis;
    pg->uang = (float)pfix->horizontalUncertaintyAngleOfAxis;
    pg->umag = 0;
    pg->up = (float)pfix->horizontalUncertaintyAlongPerpendicular;
}

/* @} */
