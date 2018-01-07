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

/*********************************************************************/
/*                                                                   */
/* (C) Copyright 2000 - 2005 by Networks In Motion, Inc.             */
/*                                                                   */
/* The information contained herein is confidential and proprietary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains proprietary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
 * data_roadinfo.h: created 2005/03/24 by Mark Goddard.
 */

#include "data_roadinfo.h"

NB_Error
data_roadinfo_init(data_util_state* pds, data_roadinfo* pri)
{
    NB_Error err = NE_OK;

    err = err ? err : data_route_number_info_init(pds, &pri->route_number_info);
    pri->unnamed = TRUE; // default to unnamed since a missing roadinfo should be "unnamed".
    pri->unpaved = 0;
    pri->trans = 0;
    pri->pronunPlayTime = 0;

    err = err ? err : data_string_init(pds, &pri->primary);
    err = err ? err : data_string_init(pds, &pri->secondary);
    err = err ? err : data_blob_init(pds, &pri->pronun);
    err = err ? err : data_country_info_init(pds, &pri->country_info);
    err = err ? err : data_exit_number_init(pds, &pri->exit_number);

    return err;
}

void
data_roadinfo_free(data_util_state* pds, data_roadinfo* pri)
{
    data_route_number_info_free(pds, &pri->route_number_info);
    pri->unnamed = FALSE;

    data_string_free(pds, &pri->primary);
    data_string_free(pds, &pri->secondary);
    data_blob_free(pds, &pri->pronun);
    data_country_info_free(pds, &pri->country_info);
    data_exit_number_free(pds, &pri->exit_number);
}

NB_Error
data_roadinfo_from_tps(data_util_state* pds, data_roadinfo* pri, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_roadinfo_free(pds, pri);

    err = data_roadinfo_init(pds, pri);

    if (err != NE_OK)
        goto errexit;

    ce = te_getchild(te, "route-number-info");

    if (ce != NULL)
        err = data_route_number_info_from_tps(pds, &pri->route_number_info, ce);

    if (err != NE_OK)
        goto errexit;

    ce = te_getchild(te, "unnamed");

    if (ce != NULL)    {
        char* value = NULL;
        size_t valueSize = 0;

        pri->unnamed = TRUE;
        if (te_getattr(ce, "trans", &value, &valueSize))
        {
            pri->trans = te_getattru(ce, "trans");
        }
        else
        {
            pri->trans = ROADINFO_INVALID_TRANS;
        }
    }
    else
        pri->unnamed = FALSE;

    pri->unpaved = (boolean)(te_getchild(te, "unpaved") != NULL);

    err = data_string_from_tps_attr(pds, &pri->primary, te, "primary");
    (void)data_string_from_tps_attr(pds, &pri->secondary, te, "secondary");
    (void)data_blob_from_tps_attr(pds, &pri->pronun, te, "pronun");

    ce = te_getchild(te, "country-info");

    if (ce != NULL)
        err = data_country_info_from_tps(pds, &pri->country_info, ce);

    ce = te_getchild(te, "exit-number");
    if (ce != NULL)
    {
        err = data_exit_number_from_tps(pds, &pri->exit_number, ce);
    }

errexit:
    if (err != NE_OK)
        data_roadinfo_free(pds, pri);
    return err;
}

NB_Error
data_roadinfo_copy(data_util_state* pds, data_roadinfo* pri_dest, data_roadinfo* pri_src)
{
    NB_Error err = NE_OK;

    data_roadinfo_free(pds, pri_dest);
    err = data_roadinfo_init(pds, pri_dest);

    err = err ? err : data_route_number_info_copy(pds, &pri_dest->route_number_info, &pri_src->route_number_info);

    pri_dest->unnamed = pri_src->unnamed;
    pri_dest->unpaved = pri_src->unpaved;
    pri_dest->trans   = pri_src->trans;
    pri_dest->pronunPlayTime = pri_src->pronunPlayTime;

    err = err ? err : data_string_copy(pds, &pri_dest->primary, &pri_src->primary);
    err = err ? err : data_string_copy(pds, &pri_dest->secondary, &pri_src->secondary);
    err = err ? err : data_blob_copy(pds, &pri_dest->pronun, &pri_src->pronun);
    err = err ? err : data_country_info_copy(pds, &pri_dest->country_info, &pri_src->country_info);
    err = err ? err : data_exit_number_copy(pds, &pri_dest->exit_number, &pri_src->exit_number);

    return err;
}
