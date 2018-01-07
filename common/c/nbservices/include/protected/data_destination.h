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

   @file     data_destination.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_DESTINATION_H
#define DATA_DESTINATION_H

#include "datautil.h"
#include "data_gps.h"
#include "data_point.h"
#include "vec.h"
#include "data_string.h"
#include "data_address.h"
#include "nbexp.h"


typedef struct data_destination_
{
    /* Child Elements */
    struct CSL_Vector*      vec_gps;
    data_point              point;
    data_address            address;

    /* Attributes */
    data_string             sa;
    data_string             str;

} data_destination;


NB_Error    data_destination_init(data_util_state* pds, data_destination* pod);
void        data_destination_free(data_util_state* pds, data_destination* pod);

tpselt      data_destination_to_tps(data_util_state* pds, data_destination* pod);

NB_Error    data_destination_copy(data_util_state* pds, data_destination* dst, data_destination* src);

NB_Error    data_destination_clear_gps(data_util_state* pds, data_destination* pod);
NB_Error    data_destination_add_gps(data_util_state* pds, data_destination* pod, data_gps* pg);
int         data_destination_num_gps(data_util_state* pds, data_destination* pod);
boolean     data_destination_equal(data_util_state* pds, data_destination* pod1, data_destination* pod2);
uint32      data_destination_get_tps_size(data_util_state* pds, data_destination* pod);
void        data_destination_to_buf(data_util_state* pds, data_destination* pod, struct dynbuf* pdb);
NB_Error    data_destination_from_binary(data_util_state* pds, data_destination* pod, byte** pdata, size_t* pdatalen);

#endif //DATA_DESTINATION_H
