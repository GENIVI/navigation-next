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
 * data_route_number_info.h: created 2005/06/03 by Mark Goddard.
 */

#include "data_route_number_info.h"

NB_Error
data_route_number_info_init(data_util_state* pds, data_route_number_info* pri)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &pri->class_);
    err = err ? err : data_string_init(pds, &pri->name);
    err = err ? err : data_string_init(pds, &pri->dir);

    return err;
}

void
data_route_number_info_free(data_util_state* pds, data_route_number_info* pri)
{
    data_string_free(pds, &pri->class_);
    data_string_free(pds, &pri->name);
    data_string_free(pds, &pri->dir);
}

NB_Error
data_route_number_info_from_tps(data_util_state* pds, data_route_number_info* pri, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_route_number_info_free(pds, pri);

    err = data_route_number_info_init(pds, pri);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pri->class_, te, "class");
    err = err ? err : data_string_from_tps_attr(pds, &pri->name, te, "name");
    (void)data_string_from_tps_attr(pds, &pri->dir, te, "direction");

errexit:
    if (err != NE_OK)
        data_route_number_info_free(pds, pri);
    return err;
}

NB_Error
data_route_number_info_copy(data_util_state* pds, data_route_number_info* pri_dest, data_route_number_info* pri_src)
{
    NB_Error err = NE_OK;

    data_route_number_info_free(pds, pri_dest);
    err = data_route_number_info_init(pds, pri_dest);

    err = err ? err : data_string_copy(pds, &pri_dest->class_, &pri_src->class_);
    err = err ? err : data_string_copy(pds, &pri_dest->name, &pri_src->name);
    err = err ? err : data_string_copy(pds, &pri_dest->dir, &pri_src->dir);

    return err;
}
