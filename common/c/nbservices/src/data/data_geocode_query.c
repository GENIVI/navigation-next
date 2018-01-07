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
 * data_geocode_query.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_geocode_query.h"

NB_Error
data_geocode_query_init(data_util_state* pds, data_geocode_query* pgq)
{
    NB_Error err = NE_OK;

    err = err ? err : data_address_init(pds, &pgq->address);
    err = err ? err : data_sliceres_init(pds, &pgq->sliceres);
    pgq->positionValid = FALSE;
    err = err ? err : data_position_init(pds, &pgq->position);

    if (err)
        data_geocode_query_free(pds, pgq);

    return err;
}

void
data_geocode_query_free(data_util_state* pds, data_geocode_query* pgq)
{
    data_address_free(pds, &pgq->address);
    data_sliceres_free(pds, &pgq->sliceres);
    pgq->positionValid = FALSE;
    data_position_free(pds, &pgq->position);
}

tpselt
data_geocode_query_to_tps(data_util_state* pds, data_geocode_query* pgq)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("geocode-query");

    if (te == NULL)
        goto errexit;

    if ((ce = data_address_to_tps(pds, &pgq->address)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;

    if ((ce = data_sliceres_to_tps(pds, &pgq->sliceres)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;

    if (pgq->positionValid)
    {
        if ((ce = data_position_to_tps(pds, &pgq->position)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    te_dealloc(ce);
    return NULL;
}

boolean
data_geocode_query_equal(data_util_state* pds, data_geocode_query* pgq1, data_geocode_query* pgq2)
{
    return	(boolean) (data_address_equal(pds, &pgq1->address, &pgq2->address) &&
                       data_sliceres_equal(pds, &pgq1->sliceres, &pgq2->sliceres) &&
                       pgq1->positionValid == pgq2->positionValid &&
                       data_position_equal(pds, &pgq1->position, &pgq2->position));
}

NB_Error
data_geocode_query_copy(data_util_state* pds, data_geocode_query* pgq_dest, data_geocode_query* pgq_src)
{
    NB_Error err = NE_OK;

    data_geocode_query_free(pds, pgq_dest);

    err = err ? err : data_geocode_query_init(pds, pgq_dest);

    err = err ? err : data_address_copy(pds, &pgq_dest->address, &pgq_src->address);
    err = err ? err : data_sliceres_copy(pds, &pgq_dest->sliceres, &pgq_src->sliceres);
    pgq_dest->positionValid = pgq_src->positionValid;
    err = err ? err : data_position_copy(pds, &pgq_dest->position, &pgq_src->position);

    return err;
}

