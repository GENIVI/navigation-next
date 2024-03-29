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

/*!--------------------------------------------------------------------------

    @file     data_fileset_reply.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef DATA_FILESET_REPLY_H
#define DATA_FILESET_REPLY_H

/*! @{ */

#include "abexp.h"
#include "datautil.h"
#include "data_fileset.h"

typedef struct data_fileset_reply_ {

	/* Child Elements */
	struct CSL_Vector*		vec_filesets;

	/* Attributes */

} data_fileset_reply;

NB_Error	    data_fileset_reply_init(data_util_state* pds, data_fileset_reply* pmr);
void		    data_fileset_reply_free(data_util_state* pds, data_fileset_reply* pmr);
NB_Error	    data_fileset_reply_from_tps(data_util_state* pds, data_fileset_reply* pmr, tpselt te);

uint32          data_fileset_reply_num_filesets(data_fileset_reply* fsr);
data_fileset*   data_fileset_reply_get_fileset(data_fileset_reply* fsr, int n);


/*! @} */

#endif // DATA_FILESET_REPLY_H
