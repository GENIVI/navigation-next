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
 * data_proxpoi_reply.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_PROXPOI_REPLY_H
#define DATA_PROXPOI_REPLY_H

#include "datautil.h"
#include "data_proxmatch.h"
#include "data_proxmatch_content.h"
#include "data_proxmatch_summary.h"
#include "data_proxmatch_ambiguous.h"
#include "data_iter_result.h"
#include "data_file.h"
#include "data_fuel_pricesummary.h"
#include "data_spelling_suggestion.h"
#include "data_search_cookie.h"
#include "data_search_event_cookie.h"
#include "data_proxpoi_error.h"
#include "nbexp.h"

typedef struct data_proxpoi_reply_ {

	/* Child Elements */

	struct CSL_Vector*				vec_proxmatch;
    struct CSL_Vector*             vec_proxmatch_content;
    struct CSL_Vector*             vec_proxmatch_summary;
    struct CSL_Vector*             vec_proxmatch_ambiguous;
	data_iter_result		iter_result;
    struct CSL_Vector*             vec_files;
	data_fuel_pricesummary	fuel_pricesummary;
    data_event_content      event_content;
    data_spelling_suggestion       spelling_suggestion;
    data_search_cookie             search_cookie;
    data_search_event_cookie       search_event_cookie;
    data_proxpoi_error             proxpoi_error;

	/* Attributes */

} data_proxpoi_reply;

NB_Error	data_proxpoi_reply_init(data_util_state* pds, data_proxpoi_reply* pdat);
void		data_proxpoi_reply_free(data_util_state* pds, data_proxpoi_reply* pdat);

NB_Error	data_proxpoi_reply_from_tps(data_util_state* pds, data_proxpoi_reply* ppr, tpselt te);

boolean		data_proxpoi_reply_equal(data_util_state* pds, data_proxpoi_reply* lhs, data_proxpoi_reply* rhs);
NB_Error	data_proxpoi_reply_copy(data_util_state* pds, data_proxpoi_reply* dst, data_proxpoi_reply* src);

// @todo: (BUG 55802) better way to accomplish this?
NB_Error	data_proxpoi_reply_from_any_tps_with_proxmatch(data_util_state* pds, data_proxpoi_reply* ppr, tpselt te);

#endif
