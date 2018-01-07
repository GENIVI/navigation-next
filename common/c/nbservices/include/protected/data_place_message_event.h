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

    @file     data_place_message_event.h

    Interface to create Place-Message-Event TPS element.

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
#ifndef DATA_PLACE_MESSAGE_EVENT_
#define DATA_PLACE_MESSAGE_EVENT_

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_analytics_event_place.h"

typedef struct data_place_message_event_ {
    /* Child Elements */
    data_analytics_event_place analytics_event_place; /*!< The place sent or
                                                        received. */

    /* Attributes */
} data_place_message_event;

NB_Error data_place_message_event_init(data_util_state* pds,
            data_place_message_event* ppme);
void data_place_message_event_free(data_util_state* pds,
            data_place_message_event* ppme);

tpselt data_place_message_event_to_tps(data_util_state* pds,
            data_place_message_event* ppme);

boolean data_place_message_event_equal(data_util_state* pds,
        data_place_message_event* ppme1, data_place_message_event* ppme2);

NB_Error data_place_message_event_copy(data_util_state* pds,
        data_place_message_event* ppme_dest,
        data_place_message_event* ppme_src);

uint32 data_place_message_event_get_tps_size(data_util_state* pds,
            data_place_message_event* ppme);

void data_place_message_event_to_buf(data_util_state* pds,
            data_place_message_event* ppme,
            struct dynbuf* pdb);

NB_Error data_place_message_event_from_binary(data_util_state* pds,
            data_place_message_event* ppme,
            byte** pdata, size_t* pdatalen);
#endif
/*! @} */

