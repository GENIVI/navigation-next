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

 @file     data_nav_reply_mult_form.h
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

#ifndef DATA_NAV_REPLY_MULT_FORM_H
#define DATA_NAV_REPLY_MULT_FORM_H

#include "datautil.h"
#include "vec.h"
#include "data_file.h"
#include "data_location.h"
#include "data_nav_route.h"
#include "data_box.h"
#include "paldebuglog.h"
#include "data_pronun_list.h"

typedef struct data_nav_reply_mult_form_
{
    /* Child Elements */
    struct CSL_Vector*            vec_file;
    struct CSL_Vector*            vec_routes;
    data_pronun_list              pronun_list;

    data_location                 origin_location;
    data_location                 destination_location;

    data_label_point              current_label_point;  //applicable for detour case only

    boolean                       route_error;
    data_box                      route_extents;

    NB_RouteSettings              route_settings;
} data_nav_reply_mult_form;


NB_Error    data_nav_reply_mult_form_init(data_util_state* pds, data_nav_reply_mult_form* pnr);

void        data_nav_reply_mult_form_free(data_util_state* pds, data_nav_reply_mult_form* pnr);

NB_Error    data_nav_reply_mult_form_from_tps(data_util_state* pds, data_nav_reply_mult_form* pnr, tpselt te, uint32 polyline_pack_passes);

NB_Error    data_nav_reply_mult_form_copy(data_util_state* pds, data_nav_reply_mult_form* pnr_dest,
                                data_nav_reply_mult_form* pnr_src);

#endif
