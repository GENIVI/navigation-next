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
 * data_message.h: created 2006/09/29 by Chetan Nagaraj.
 */

#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include "datautil.h"
#include "data_string.h"
#include "data_formatted_text.h"
#include "data_message_nag.h"
#include "data_url.h"

typedef struct data_message_ {

	/* Child Elements */
	data_formatted_text	formatted_text;
	data_message_nag	message_nag;
	data_url			url;

	/* Attributes */
	boolean			confirm;
	boolean			exit_on_decline;

	data_string		id;
	data_string		title;
	data_string		type;
	data_string		language;
	data_string		accept_text;
	data_string		center_text;
	data_string		decline_text;

	uint32 time;

} data_message;

NB_Error	data_message_init(data_util_state* pds, data_message* pm);
void		data_message_free(data_util_state* pds, data_message* pm);
NB_Error	data_message_from_tps(data_util_state* pds, data_message* pm, tpselt te);
boolean		data_message_equal(data_util_state* pds, data_message* pm1, data_message* pm2);
NB_Error	data_message_copy(data_util_state* pds, data_message* pm_dest, data_message* pm_src);
void		data_message_to_buf(data_util_state* pds, data_message* pm, struct dynbuf* pdb);
NB_Error	data_message_from_binary(data_util_state* pds, data_message* pm, byte** pdata, size_t* pdatalen);
int			data_message_compare(const data_message* pA, const data_message* pB);

#endif
