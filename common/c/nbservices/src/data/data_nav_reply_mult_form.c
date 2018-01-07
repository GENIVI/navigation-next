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

 @file     data_nav_reply_mult_form.c
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

#include "data_nav_reply_mult_form.h"
#include "csltypes.h"

NB_Error
data_nav_reply_mult_form_init(data_util_state* pds, data_nav_reply_mult_form* pnr)
{
    NB_Error err = NE_OK;

    pnr->vec_routes = CSL_VectorAlloc(sizeof(data_nav_route));
    if (pnr->vec_routes == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->vec_file = CSL_VectorAlloc(sizeof(data_file));
    if (pnr->vec_file == NULL)
    {
        err =  NE_NOMEM;
        goto errexit;
    }

    err = data_location_init(pds, &pnr->origin_location);
    err = err ? err : data_location_init(pds, &pnr->destination_location);
    err = err ? err : data_box_init(pds, &pnr->route_extents);
    err = err ? err : data_label_point_init(pds, &pnr->current_label_point);
    err = err ? err : data_pronun_list_init(pds, &pnr->pronun_list);

    if (err != NE_OK)
    {
        goto errexit;
    }

    nsl_memset(&pnr->route_settings, 0, sizeof(pnr->route_settings));
    pnr->route_error = FALSE;

errexit:
    if (err != NE_OK)
    {
        data_nav_reply_mult_form_free(pds, pnr);
    }

    return err;
}

void
data_nav_reply_mult_form_free(data_util_state* pds, data_nav_reply_mult_form* pnr)
{
    DATA_VEC_FREE(pds, pnr->vec_routes, data_nav_route);
    DATA_VEC_FREE(pds, pnr->vec_file, data_file);

    data_location_free(pds, &pnr->origin_location);
    data_location_free(pds, &pnr->destination_location);
    data_box_free(pds, &pnr->route_extents);
    data_label_point_free(pds, &pnr->current_label_point);
    data_pronun_list_free(pds, &pnr->pronun_list);
}

NB_Error
data_nav_reply_mult_form_copy(data_util_state* pds, data_nav_reply_mult_form* pnr_dest, data_nav_reply_mult_form* pnr_src)
{
    NB_Error err = NE_OK;

    data_nav_reply_mult_form_free(pds, pnr_dest);
    err = data_nav_reply_mult_form_init(pds, pnr_dest);
    if (err != NE_OK)
    {
        goto errexit;
    }

    DATA_VEC_COPY(pds, err, pnr_dest->vec_routes, pnr_src->vec_routes, data_nav_route);
    DATA_VEC_COPY(pds, err, pnr_dest->vec_file, pnr_src->vec_file, data_file);

    err = data_box_copy(pds, &pnr_dest->route_extents, &pnr_src->route_extents);
    err = err ? err : data_location_copy(pds, &pnr_dest->origin_location, &pnr_src->origin_location);
    err = err ? err : data_location_copy(pds, &pnr_dest->destination_location, &pnr_src->destination_location);
    err = err ? err : data_label_point_copy(pds, &pnr_dest->current_label_point, &pnr_src->current_label_point);
    err = err ? err : data_pronun_list_copy(pds, &pnr_dest->pronun_list, &pnr_src->pronun_list);
    pnr_dest->route_error = pnr_src->route_error;
    pnr_dest->route_settings = pnr_src->route_settings;

errexit:
    if (err != NE_OK)
    {
        data_nav_reply_mult_form_free(pds, pnr_dest);
    }

    return err;
}


NB_Error
data_nav_reply_mult_form_from_tps(data_util_state* pds, data_nav_reply_mult_form* pnr, tpselt te, uint32 polyline_pack_passes)
{
    NB_Error err = NE_OK;

    int iter = 0;

    tpselt ce = NULL;

    data_nav_route    nav_route;
    data_file         file;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    err = data_nav_route_init(pds, &nav_route);
    if (err != NE_OK)
        goto errexit;

    err = data_file_init(pds, &file);
    if (err != NE_OK)
        goto errexit;


    data_nav_reply_mult_form_free(pds, pnr);

    err = data_nav_reply_mult_form_init(pds, pnr);
    if (err != NE_OK)
        goto errexit;

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        const char * name = te_getname(ce);

        if (nsl_strcmp(name, "nav-route") == 0)
        {
            err = data_nav_route_from_tps(pds, &nav_route, ce, polyline_pack_passes);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pnr->vec_routes, &nav_route))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    data_nav_route_init(pds, &nav_route); // clear out the nav_route since we have copied it (shallow copy)
                else
                    data_nav_route_free(pds, &nav_route); // free the nav_route if it was not copied
            }
            if (err != NE_OK)
                goto errexit;
        }
        else if (nsl_strcmp(name, "file") == 0)
        {
            err = data_file_from_tps(pds, &file, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pnr->vec_file, &file))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    nsl_memset(&file, 0, sizeof(file)); // clear out the file since we have copied it (shallow copy)
                else
                    data_file_free(pds, &file); // free the file if it was not copied
            }

            if (err != NE_OK)
                goto errexit;
        }
        else if (nsl_strcmp(name, "route-extents") == 0)
        {
            tpselt box = te_getchild(ce, "box");
            err = err ? err : data_box_from_tps(pds, &pnr->route_extents, box);
        }
    }

    ce = te_getchild(te, "origin-location");
    if (ce != NULL)
        err = err ? err : data_location_from_tps(pds, &pnr->origin_location, ce);
    else
        data_location_free(pds, &pnr->origin_location);

    ce = te_getchild(te, "destination-location");
    if (ce != NULL)
        err = err ? err : data_location_from_tps(pds, &pnr->destination_location, ce);
    else
        data_location_free(pds, &pnr->destination_location);

    ce = te_getchild(te, "current-route-label-point");
    if (ce != NULL)
        err = err ? err : data_label_point_from_tps(pds, &pnr->current_label_point, ce);
    else
        data_label_point_free(pds, &pnr->current_label_point);

    ce = te_getchild(te, "pronun-list");
    if (ce != NULL)
        err = err ? err : data_pronun_list_from_tps(pds, &pnr->pronun_list, ce);
    else
        data_pronun_list_free(pds, &pnr->pronun_list);
    if (te_getchild(te, "route-error"))
    {
        pnr->route_error = TRUE;
    }

    if (err != NE_OK)
    {
        goto errexit;
    }

errexit:
    data_nav_route_free(pds, &nav_route);
    data_file_free(pds, &file);

    if (err != NE_OK)
    {
        data_nav_reply_mult_form_free(pds, pnr);
    }
    return err;
}

