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
/* (C) Copyright 2014 by Networks In Motion, Inc.                */
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

#ifndef DATA_INVOCATION_CONTEXT_H_
#define DATA_INVOCATION_CONTEXT_H_

#include "nbexp.h"
#include "nberror.h"
#include "datautil.h"
#include "data_string.h"

typedef struct data_invocation_context {

    data_string input_source;
    data_string invocation_method;
    data_string screen_id;
    data_string source_module;

} data_invocation_context;

NB_Error data_invocation_context_init(data_util_state* pds,
                                      data_invocation_context* pdic);

void data_invocation_context_free(data_util_state* pds,
                                  data_invocation_context* pdic);

tpselt data_invocation_context_to_tps(data_util_state* pds,
                                      data_invocation_context* pdic);

boolean data_invocation_context_equal(data_util_state* pds,
                                      data_invocation_context* pdic1, data_invocation_context* pdic2);

NB_Error data_invocation_context_copy(data_util_state* pds,
                                      data_invocation_context* pdic_dest, data_invocation_context* pdic_src);

uint32 data_invocation_context_get_tps_size(data_util_state* pds, data_invocation_context* pdic);

void data_invocation_context_to_buf(data_util_state* pds, data_invocation_context* pdic, struct dynbuf* pdb);

NB_Error data_invocation_context_from_binary(data_util_state* pds, data_invocation_context* pdic, byte** pdata, size_t* pdatalen);

#endif /* DATA_INVOCATION_CONTEXT_H_ */
