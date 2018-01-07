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
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
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
 * data_asr_stats_query.h: created created 2008/04/29 by NikunK.
 */

#ifndef DATA_ASR_STATS_QUERY_
#define DATA_ASR_STATS_QUERY_

#include "datautil.h"
#include "data_string.h"
#include "data_asr_field_data.h"
#include "vec.h"
#include "abexp.h"

typedef struct data_asr_stats_query_ {

	/* Child Elements */
	struct CSL_Vector*		vec_field_data;

	/* Attributes */
	data_string		user_lang;
	data_string		asr_engine;
	data_string		asr_id;
	data_string		asr_timing;

} data_asr_stats_query;

NB_Error	data_asr_stats_query_init(data_util_state* pds, data_asr_stats_query* pgq);
void		data_asr_stats_query_free(data_util_state* pds, data_asr_stats_query* pgq);

tpselt		data_asr_stats_query_to_tps(data_util_state* pds, data_asr_stats_query* pgq);
NB_Error	data_asr_stats_set_id(data_util_state* pds, data_asr_stats_query* pgq, const char* asr_id);
NB_Error	data_asr_stats_query_add_field_data(data_util_state* pds, data_asr_stats_query* pgq, const char* fieldName, const char* fieldType, const char* fieldText, uint32 cursorPosition,
																						 const char* utteranceId, const char* utteranceEncoding, byte* pUtteranceData, uint32 pUtteranceSize);
NB_Error	data_asr_stats_query_clear_fields(data_util_state* pds, data_asr_stats_query* pgq);
NB_Error    data_asr_stats_query_set_timing(data_util_state* pds, data_asr_stats_query* pgq, const char* timing);
boolean		data_asr_stats_query_equal(data_util_state* pds, data_asr_stats_query* pgq1, data_asr_stats_query* pgq2);
NB_Error	data_asr_stats_query_copy(data_util_state* pds, data_asr_stats_query* pgq_dest, data_asr_stats_query* pgq_src);

#endif
