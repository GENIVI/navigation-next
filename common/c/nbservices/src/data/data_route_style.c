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
 * data_route_style.c: created 2005/01/06 by Mark Goddard.
 */

#include "data_route_style.h"

static void
data_route_style_clear(data_util_state* pds, data_route_style* prs)
{
    prs->avoid_hov = FALSE;
    prs->avoid_toll = FALSE;
    prs->avoid_highway = FALSE;
    prs->avoid_uturn = FALSE;
    prs->avoid_ferry = FALSE;
}

NB_Error
data_route_style_init(data_util_state* pds, data_route_style* prs)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &prs->optimize);
    err = err ? err : data_string_init(pds, &prs->vehicle_type);

    data_route_style_clear(pds, prs);

    if (err)
        data_route_style_free(pds, prs);

    return err;
}

void
data_route_style_free(data_util_state* pds, data_route_style* prs)
{
    data_string_free(pds, &prs->optimize);
    data_string_free(pds, &prs->vehicle_type);

    data_route_style_clear(pds, prs);
}

NB_Error
data_route_style_copy(data_util_state* pds, data_route_style* dst, data_route_style* src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_copy(pds, &dst->optimize,  &src->optimize);
    err = err ? err : data_string_copy(pds, &dst->vehicle_type, &src->vehicle_type);
    
    dst->avoid_ferry = src->avoid_ferry;
    dst->avoid_highway = src->avoid_highway;
    dst->avoid_hov = src->avoid_hov;
    dst->avoid_toll = src->avoid_toll;
    dst->avoid_unpaved = src->avoid_unpaved;
    dst->avoid_uturn = src->avoid_uturn;
    
    return err;
}

tpselt
data_route_style_to_tps(data_util_state* pds, data_route_style* prs)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("route-style");

    if (te == NULL)
        goto errexit;

    if (prs->avoid_hov) {

        if ((ce = te_new("avoid")) != NULL && te_setattrc(ce, "value", "hov") && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (prs->avoid_toll) {

        if ((ce = te_new("avoid")) != NULL && te_setattrc(ce, "value", "toll") && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (prs->avoid_highway) {

        if ((ce = te_new("avoid")) != NULL && te_setattrc(ce, "value", "highway") && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (prs->avoid_uturn) {

        if ((ce = te_new("avoid")) != NULL && te_setattrc(ce, "value", "uturn") && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (prs->avoid_unpaved) {

        if ((ce = te_new("avoid")) != NULL && te_setattrc(ce, "value", "unpaved") && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (prs->avoid_ferry) {

        if ((ce = te_new("avoid")) != NULL && te_setattrc(ce, "value", "ferry") && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (!te_setattrc(te, "optimize", data_string_get(pds, &prs->optimize)))
        goto errexit;

    if (!te_setattrc(te, "vehicle-type", data_string_get(pds, &prs->vehicle_type)))
        goto errexit;

    return te;

errexit:

    if (te) te_dealloc(te);
    if (ce) te_dealloc(ce);

    return NULL;
}

NB_Error
data_route_style_from_tps(data_util_state* pds, data_route_style* prs, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if(!te)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_route_style_free(pds, prs);

    err = data_route_style_init(pds, prs);
    if (err != NE_OK)
    {
        goto errexit;
    }

    ce = te_getchild(te, "optimize");

    data_string_set(pds, &prs->optimize, te_getattrc(te, "optimize"));
    data_string_set(pds, &prs->vehicle_type, te_getattrc(te, "vehicle-type"));

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        const char* name = te_getname(ce);

        if (nsl_strcmp(name, "avoid") == 0)
        {
            const char* value = te_getattrc(ce, "value");

            if (nsl_strcmp(value, "hov") == 0)
            {
                prs->avoid_hov = TRUE;
            }
            else if (nsl_strcmp(value, "highway") == 0)
            {
                prs->avoid_highway = TRUE;
            }
            else if (nsl_strcmp(value, "toll") == 0)
            {
                prs->avoid_toll = TRUE;
            }
            else if (nsl_strcmp(value, "uturn") == 0)
            {
                prs->avoid_uturn = TRUE;
            }
            else if (nsl_strcmp(value, "unpaved") == 0)
            {
                prs->avoid_unpaved = TRUE;
            }
            else if (nsl_strcmp(value, "ferry") == 0)
            {
                prs->avoid_ferry = TRUE;
            }
        }
    }

errexit:
    if(err != NE_OK)
    {
        data_route_style_free(pds, prs);
    }

    return err;
}

boolean
data_route_style_equal(data_util_state* pds, data_route_style* prs1, data_route_style* prs2)
{
    return (boolean) ( (prs1->avoid_hov == prs2->avoid_hov) &&
        (prs1->avoid_toll == prs2->avoid_toll) &&
        (prs1->avoid_highway == prs2->avoid_highway) &&
        (prs1->avoid_uturn == prs2->avoid_uturn) &&
        (prs1->avoid_unpaved == prs2->avoid_unpaved) &&
        (prs1->avoid_ferry == prs2->avoid_ferry) &&
        data_string_equal(pds, &prs1->optimize, &prs2->optimize) &&
        data_string_equal(pds, &prs1->vehicle_type, &prs2->vehicle_type) );
}

uint32
data_route_style_get_tps_size(data_util_state* pds, data_route_style* prs)
{
    uint32 size = 0;

    size += sizeof(prs->avoid_hov);
    size += sizeof(prs->avoid_toll);
    size += sizeof(prs->avoid_highway);
    size += sizeof(prs->avoid_uturn);
    size += sizeof(prs->avoid_unpaved);
    size += sizeof(prs->avoid_ferry);

    size += data_string_get_tps_size(pds, &prs->optimize);
    size += data_string_get_tps_size(pds, &prs->vehicle_type);

    return size;
}

void
data_route_style_to_buf(data_util_state* pds, data_route_style* prs, struct dynbuf* pdb)
{
    dbufcat(pdb, (const byte*) &prs->avoid_hov, sizeof(prs->avoid_hov));
    dbufcat(pdb, (const byte*) &prs->avoid_toll, sizeof(prs->avoid_toll));
    dbufcat(pdb, (const byte*) &prs->avoid_highway, sizeof(prs->avoid_highway));
    dbufcat(pdb, (const byte*) &prs->avoid_uturn, sizeof(prs->avoid_uturn));
    dbufcat(pdb, (const byte*) &prs->avoid_unpaved, sizeof(prs->avoid_unpaved));
    dbufcat(pdb, (const byte*) &prs->avoid_ferry, sizeof(prs->avoid_ferry));

    data_string_to_buf(pds, &prs->optimize, pdb);
    data_string_to_buf(pds, &prs->vehicle_type, pdb);
}

NB_Error
data_route_style_from_binary(data_util_state* pds, data_route_style* prs, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, prs, data_route_style);

    err = (err) ? err : data_boolean_from_binary(pds, &prs->avoid_hov, pdata, pdatalen);
    err = (err) ? err : data_boolean_from_binary(pds, &prs->avoid_toll, pdata, pdatalen);
    err = (err) ? err : data_boolean_from_binary(pds, &prs->avoid_highway, pdata, pdatalen);
    err = (err) ? err : data_boolean_from_binary(pds, &prs->avoid_uturn, pdata, pdatalen);
    err = (err) ? err : data_boolean_from_binary(pds, &prs->avoid_unpaved, pdata, pdatalen);
    err = (err) ? err : data_boolean_from_binary(pds, &prs->avoid_ferry, pdata, pdatalen);

    err = (err) ? err : data_string_from_binary(pds, &prs->optimize, pdata, pdatalen);
    err = (err) ? err : data_string_from_binary(pds, &prs->vehicle_type, pdata, pdatalen);

    return err;
}

NB_Error
data_route_style_from_nb(data_util_state* pds, data_route_style* prs, NB_RouteType routeType, NB_TransportationMode vehicleType, NB_RouteAvoid routeAvoid)
{
    NB_Error err = NE_OK;
    const char* optimize = NULL;
    const char* vehicle = NULL;

    err = data_route_style_init(pds, prs);

    switch (routeType) 
    {
    case NB_RouteType_Fastest:
        optimize = "fastest";
        break;
    case NB_RouteType_Shortest:
        optimize = "shortest";
        break;
    case NB_RouteType_Easiest:
        optimize = "easiest";
        break;
    default:
        optimize = "fastest";
    }

    switch (vehicleType)
    {
        case NB_TransportationMode_Car:
            vehicle = "car";
            break;
        case NB_TransportationMode_Truck:
            vehicle = "truck";
            break;
        case NB_TransportationMode_Bicycle:
            vehicle = "bicycle";
            break;
        case NB_TransportationMode_Pedestrian:
            vehicle = "pedestrian";
            break;
        default:
            vehicle = "car";
    }

    err = err ? err : data_string_set(pds, &prs->optimize, optimize);
    err = err ? err : data_string_set(pds, &prs->vehicle_type, vehicle);

    prs->avoid_hov      = (routeAvoid & NB_RouteAvoid_HOV) ? TRUE : FALSE;
    prs->avoid_toll     = (routeAvoid & NB_RouteAvoid_Toll) ? TRUE : FALSE;
    prs->avoid_highway  = (routeAvoid & NB_RouteAvoid_Highway) ? TRUE : FALSE;
    prs->avoid_uturn    = (routeAvoid & NB_RouteAvoid_UTurn) ? TRUE : FALSE;
    prs->avoid_unpaved  = (routeAvoid & NB_RouteAvoid_Unpaved) ? TRUE : FALSE;
    prs->avoid_ferry    = (routeAvoid & NB_RouteAvoid_Ferry) ? TRUE : FALSE;

    if (err != NE_OK)
        data_route_style_free(pds, prs);

    return err;
}
