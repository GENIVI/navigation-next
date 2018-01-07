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

    @file     data_poi_content.h
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

/*! @{ */

#ifndef POI_CONTENT_H
#define POI_CONTENT_H

#include "datautil.h"
#include "data_string.h"
#include "data_formatted_text.h"
#include "data_tagline.h"
#include "data_overall_rating.h"
#include "data_golden_cookie.h"
#include "data_place_event_cookie.h"
#include "data_pair.h"
#include "data_icon.h"
#include "data_hours_of_operation_for_search.h"
#include "data_vendor_content.h"
#include "nbexp.h"

typedef struct data_poi_content_
{
    // Elements
    data_formatted_text     formatted_text;
    data_tagline            tagline;
    data_overall_rating     overall_rating;
    data_golden_cookie      golden_cookie;
    data_place_event_cookie place_event_cookie;
    data_icon               icon;
    data_hours_of_operation_for_search hours_of_operation;
    boolean                 has_vendor_content;
    struct CSL_Vector*      vec_vendor_content;
    boolean                 hasHoursOfOperation;       /*!< Is the hours of operation present? */
    struct CSL_Vector*      vec_pairs;

    // Attributes
    data_string id;

} data_poi_content;

NB_DEC NB_Error    data_poi_content_init(data_util_state* pds, data_poi_content* ppc);
NB_DEC void        data_poi_content_free(data_util_state* pds, data_poi_content* ppc);
NB_DEC NB_Error    data_poi_content_from_tps(data_util_state* pds, data_poi_content* ppc, tpselt te);
NB_DEC boolean	   data_poi_content_equal(data_util_state* pds, data_poi_content* ppc1, data_poi_content* ppc2);
NB_DEC NB_Error    data_poi_content_copy(data_util_state* pds, data_poi_content* ppc_dest, data_poi_content* ppc_src);

/*! @} */

#endif //POI_CONTENT_H
