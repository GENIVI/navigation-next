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
 * data_blob.h: created 2004/12/12 by Mark Goddard.
 */

#ifndef DATA_BLOB_
#define DATA_BLOB_

#include "datautil.h"
#include "dynbuf.h"
#include "nbexp.h"

typedef struct data_blob_ {
	
	byte*	data;
	size_t	size;

} data_blob;

NB_DEC NB_Error	data_blob_init(data_util_state* pds, data_blob* ps);
NB_DEC void		data_blob_free(data_util_state* pds, data_blob* ps);

NB_DEC NB_Error	data_blob_set(data_util_state* pds, data_blob* ps, const byte* data, size_t size);
NB_DEC NB_Error	data_blob_set_cstr(data_util_state* pds, data_blob* ps, const char* data);
NB_DEC NB_Error	data_blob_from_tps_attr(data_util_state* pds, data_blob* ps, tpselt te, const char* name);
NB_DEC NB_Error	data_blob_alloc(data_util_state* pds, data_blob* ps, size_t size);

NB_DEC boolean		data_blob_equal(data_util_state* pds, data_blob* ps1, data_blob* ps2);
NB_DEC NB_Error	data_blob_copy(data_util_state* pds, data_blob* ps_dest, data_blob* ps_src);

NB_DEC void		data_blob_to_buf(data_util_state* pds, data_blob* ps, struct dynbuf *dbp);

NB_DEC NB_Error	data_blob_from_binary(data_util_state* pds, data_blob* ps,
				byte** ppdata, size_t* pdatalen);
#endif

