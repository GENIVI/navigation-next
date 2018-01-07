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

/*
 * data_message_confirm_query.h: created 2006/09/29 by Chetan Nagaraj.
 */

#ifndef DATA_MESSAGE_CONFIRM_QUERY_
#define DATA_MESSAGE_CONFIRM_QUERY_

#include "datautil.h"
#include "data_message_confirm.h"
#include "data_retrieve_confirmed.h"
#include "vec.h"

typedef struct data_message_confirm_query_ {

	/* Child Elements */
	struct CSL_Vector*		vec_message_confirm;
	struct CSL_Vector*		vec_retrieve_confirmed;

	/* Attributes */

} data_message_confirm_query;

NB_Error	data_message_confirm_query_init(data_util_state* pds, data_message_confirm_query* pmcq);
void		data_message_confirm_query_free(data_util_state* pds, data_message_confirm_query* pmcq);
tpselt		data_message_confirm_query_to_tps(data_util_state* pds, data_message_confirm_query* pmcq);
NB_Error	data_message_confirm_query_add_message_confirm(data_util_state* pds, data_message_confirm_query* pmcq, data_message_confirm* pmc);
NB_Error	data_message_confirm_query_add_retrieve_confirmed(data_util_state* pds, data_message_confirm_query* pmcq, data_retrieve_confirmed* prc);
boolean		data_message_confirm_query_equal(data_util_state* pds, data_message_confirm_query* pmcq1, data_message_confirm_query* pmcq2);
NB_Error	data_message_confirm_query_copy(data_util_state* pds, data_message_confirm_query* pmcq_dest, data_message_confirm_query* pmcq_src);

#endif
