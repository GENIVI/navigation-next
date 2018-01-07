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
/* (C) Copyright 2007 by Networks In Motion, Inc.                */
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
 * data_traffic_notify_reply.h: created 2007/11/27 by Mark Goddard.
 */

#ifndef DATA_TRAFFIC_NOTIFY_REPLY_H
#define DATA_TRAFFIC_NOTIFY_REPLY_H

#include "datautil.h"
#include "data_traffic_record_identifier.h"
#include "data_file.h"
#include "data_pronun_list.h"
#include "nbexp.h"

typedef struct data_traffic_notify_reply_ {

	/* Child Elements */
	struct CSL_Vector*			vec_file;
	data_traffic_record_identifier	traffic_record_identifier;
	data_pronun_list			pronun_list;

	/* Attributes */

} data_traffic_notify_reply;

NB_Error	data_traffic_notify_reply_init(data_util_state* pds, data_traffic_notify_reply* tnr);
void		data_traffic_notify_reply_free(data_util_state* pds, data_traffic_notify_reply* tnr);

NB_Error	data_traffic_notify_reply_from_tps(data_util_state* pds, data_traffic_notify_reply* tnr, tpselt te);

NB_Error	data_traffic_notify_reply_copy(data_util_state* pds, data_traffic_notify_reply* tnr_dest, data_traffic_notify_reply* tnr_src);

void		data_traffic_notify_reply_file_foreach(data_util_state* pds, data_traffic_notify_reply* tnr, 
												   void (*cb)(data_file* pf, void* pUser), void* pUser);
void		data_traffic_notify_reply_discard_files(data_util_state* pds, data_traffic_notify_reply* tnr);


#endif // DATA_TRAFFIC_NOTIFY_QUERY_H

