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

    @file     data_realistic_sign.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_realistic_sign.h"

NB_Error
data_realistic_sign_init(data_util_state* pds, data_realistic_sign* prs)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &prs->id);
    err = err ? err : data_string_init(pds, &prs->portrait_id);
    err = err ? err : data_string_init(pds, &prs->type);
    err = err ? err : data_string_init(pds, &prs->dataset_id);
    err = err ? err : data_string_init(pds, &prs->version);

    prs->position = 0.0;

    return err;
}

void
data_realistic_sign_free(data_util_state* pds, data_realistic_sign* prs)
{
    data_string_free(pds, &prs->id);
    data_string_free(pds, &prs->portrait_id);
    data_string_free(pds, &prs->type);
    data_string_free(pds, &prs->dataset_id);
    data_string_free(pds, &prs->version);
}

NB_Error
data_realistic_sign_from_tps(data_util_state* pds, data_realistic_sign* prs, tpselt te)
{
    NB_Error err = NE_OK;

    if (!te)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_realistic_sign_free(pds, prs);

    err = data_realistic_sign_init(pds, prs);

    err = err ? err : data_string_from_tps_attr(pds, &prs->id, te, "id");
    err = err ? err : data_string_from_tps_attr(pds, &prs->portrait_id, te, "portrait-id");
    err = err ? err : data_string_from_tps_attr(pds, &prs->type, te, "type");
    err = err ? err : data_string_from_tps_attr(pds, &prs->dataset_id, te, "dataset-id");
    err = err ? err : data_string_from_tps_attr(pds, &prs->version, te, "version");
    ( void )te_getattrd(te, "position", &prs->position);

errexit:
    if (err != NE_OK)
    {
        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_realistic_sign (error_code: %d)", err));
        data_realistic_sign_free(pds, prs);
    }

    return err;
}

boolean
data_realistic_sign_equal(data_util_state* pds, data_realistic_sign* prs1, data_realistic_sign* prs2)
{
    return (boolean)(data_string_equal(pds, &prs1->id, &prs2->id)
        && data_string_equal(pds, &prs1->portrait_id, &prs2->portrait_id)
        && data_string_equal(pds, &prs1->type, &prs2->type)
        && data_string_equal(pds, &prs1->dataset_id, &prs2->dataset_id)
        && data_string_equal(pds, &prs1->version, &prs2->version)
        && prs1->position == prs2->position);
}

NB_Error
data_realistic_sign_copy(data_util_state* pds, data_realistic_sign* prs_dest, data_realistic_sign* prs_src)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_copy(pds, &prs_dest->id, &prs_src->id);
    err = err ? err : data_string_copy(pds, &prs_dest->portrait_id, &prs_src->portrait_id);
    err = err ? err : data_string_copy(pds, &prs_dest->type, &prs_src->type);
    err = err ? err : data_string_copy(pds, &prs_dest->dataset_id, &prs_src->dataset_id);
    err = err ? err : data_string_copy(pds, &prs_dest->version, &prs_src->version);

    if (!err) 
    {
        prs_dest->position = prs_src->position;
    }

    return err;
}

/*! @} */
