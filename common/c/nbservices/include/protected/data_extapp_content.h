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

 @file     data_extapp_content.h
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_EXTAPP_CONTENT_H
#define DATA_EXTAPP_CONTENT_H

#include "datautil.h"
#include "data_string.h"


typedef struct data_extapp_content_
{
    /* Child Elements */
    struct CSL_Vector*    vec_pairs;          /*! The key/value pairs provided to invoke 3rd party app. */

    /* Attributes */
    data_string           name;               /*! Feature code of 3rd party app. */

} data_extapp_content;


NB_Error
data_extapp_content_init(data_util_state* pds, data_extapp_content* peac);

void
data_extapp_content_free(data_util_state* pds, data_extapp_content* peac);

tpselt
data_extapp_content_to_tps(data_util_state* pds, data_extapp_content* peac);

NB_Error
data_extapp_content_from_tps(data_util_state* pds, data_extapp_content* peac, tpselt te);

boolean
data_extapp_content_equal(data_util_state* pds, data_extapp_content* peac1, data_extapp_content* peac2);

NB_Error
data_extapp_content_copy(data_util_state* pds, data_extapp_content* peac_dest, data_extapp_content* peac_src);

#endif //DATA_EXTAPP_CONTENT_H

/*! @} */