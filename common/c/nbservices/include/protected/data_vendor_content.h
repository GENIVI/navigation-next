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

#ifndef DATA_VENDOR_CONTENT_H
#define DATA_VENDOR_CONTENT_H

#include "data_string.h"
#include "data_overall_rating.h"

typedef struct data_vendor_content_ {

    /* Child Elements */
    struct CSL_Vector*      vec_pairs;

    boolean                 has_overall_rating;
    data_overall_rating     overall_rating;

    /* Attributes */
    data_string             name;

} data_vendor_content;

NB_DEC NB_Error    data_vendor_content_init(data_util_state* pds, data_vendor_content* dvc);
NB_DEC void        data_vendor_content_free(data_util_state* pds, data_vendor_content* dvc);

NB_DEC NB_Error    data_vendor_content_from_tps(data_util_state* pds, data_vendor_content* dvc, tpselt te);

NB_DEC boolean     data_vendor_content_equal(data_util_state* pds, data_vendor_content* dvc1, data_vendor_content* dvc2);
NB_DEC NB_Error    data_vendor_content_copy(data_util_state* pds, data_vendor_content* dvc_dest, data_vendor_content* dvc_src);

#endif /* DATA_VENDOR_CONTENT_H */
