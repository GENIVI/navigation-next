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

   @file     data_poiext_event.h
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

#ifndef DATA_POIEXT_EVENT_H
#define DATA_POIEXT_EVENT_H

#include "nbexp.h"
#include "datautil.h"
#include "data_analytics_event_place.h"
#include "data_string.h"
#include "nbextappcontent.h"


typedef struct data_poiext_event_
{
    /* Elements */
    data_analytics_event_place     analytics_event_place;

    /* Attributes */
    data_string                    name;

} data_poiext_event;


NB_Error  data_poiext_event_init(data_util_state* pds, data_poiext_event* ppe);
void      data_poiext_event_free(data_util_state* pds, data_poiext_event* ppe);

tpselt    data_poiext_event_to_tps(data_util_state* pds, data_poiext_event* ppe);
boolean   data_poiext_event_equal(data_util_state* pds, data_poiext_event* ppe1, data_poiext_event* ppe2);
NB_Error  data_poiext_event_copy(data_util_state* pds, data_poiext_event* ppe_dst, data_poiext_event* ppe_src);
uint32    data_poiext_event_get_tps_size(data_util_state* pds, data_poiext_event* ppe);

void      data_poiext_event_to_buf(data_util_state* pds, data_poiext_event* ppe, struct dynbuf* pdb);
NB_Error  data_poiext_event_from_binary(data_util_state* pds, data_poiext_event* ppe, byte** pdata, size_t* pdatalen);

NB_Error  data_poiext_event_set_app(data_util_state* pds, data_poiext_event* ppe, NB_ThirdPartyApp app);

#endif //DATA_POIEXT_EVENT_H

/*! @} */