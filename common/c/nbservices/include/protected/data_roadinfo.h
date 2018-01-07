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

#ifndef DATA_ROADINFO_H
#define DATA_ROADINFO_H

#include "datautil.h"
#include "data_string.h"
#include "data_blob.h"
#include "data_route_number_info.h"
#include "data_country_info.h"
#include "data_exit_number.h"
#include "nbexp.h"

#define ROADINFO_INVALID_TRANS (uint32)(-1)

typedef struct data_roadinfo_
{

    /* Child Elements */
    data_route_number_info	route_number_info;
    boolean					unnamed;
    boolean					unpaved;
    uint32					trans;
    boolean					toward;
    data_country_info		country_info;
    data_exit_number        exit_number;

    /* Attributes */
    data_string		primary;
    data_string		secondary;
    data_blob		pronun;
    /* this only valid when onboard mode. */
    int             pronunPlayTime;

} data_roadinfo;

NB_Error	data_roadinfo_init(data_util_state* pds, data_roadinfo* pri);
void		data_roadinfo_free(data_util_state* pds, data_roadinfo* pri);

NB_Error	data_roadinfo_from_tps(data_util_state* pds, data_roadinfo* pri, tpselt te);
NB_Error	data_roadinfo_copy(data_util_state* pds, data_roadinfo* pri_dest, data_roadinfo* pri_src);

#endif
