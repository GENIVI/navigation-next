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
 * data_formatted_text.h: created 2006/10/10 by Chetan Nagaraj.
 */

#ifndef DATA_FORMATTED_TEXT_H
#define DATA_FORMATTED_TEXT_H

#include "datautil.h"
#include "data_format_element.h"
#include "vec.h"
#include "nbexp.h"

typedef struct {

	/* Child Elements */

	/* Attributes */
	struct CSL_Vector* vec_format_elements;

} data_formatted_text;

NB_DEC NB_Error	data_formatted_text_init(data_util_state* pds, data_formatted_text* pft);
NB_DEC void		data_formatted_text_free(data_util_state* pds, data_formatted_text* pft);
NB_DEC NB_Error	data_formatted_text_from_tps(data_util_state* pds, data_formatted_text* pft, tpselt te);
NB_DEC boolean		data_formatted_text_equal(data_util_state* pds, data_formatted_text* pft1, data_formatted_text* pft2);
NB_DEC NB_Error	data_formatted_text_copy(data_util_state* pds, data_formatted_text* pft_dest, data_formatted_text* pft_src);
NB_DEC void		data_formatted_text_to_buf(data_util_state* pds, data_formatted_text* pft, struct dynbuf* pdb);
NB_DEC NB_Error	data_formatted_text_from_binary(data_util_state* pds, data_formatted_text* pft, byte** ppdata, size_t* pdatalen);
uint32   data_formatted_text_get_tps_size(data_util_state* pds, data_formatted_text* pft);

#endif

