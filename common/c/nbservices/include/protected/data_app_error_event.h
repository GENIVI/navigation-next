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

    @file     data_app_error_event.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

#ifndef DATA_APP_ERROR_EVENT_H
#define DATA_APP_ERROR_EVENT_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"
#include "data_error_category.h"
#include "data_pair.h"

typedef struct data_app_error_event_
{
    /* Child Elements */
    data_error_category error_category;
    struct CSL_Vector*  vec_parameters; /*!< Additional parameters associated with a specific error-category, see Protocol Specification */

    /* Attributes */
    uint32 error_code;                /*!< Client-side error code */
    data_string detailed_code;        /*!< Technical summary of the problem */
    data_string detailed_description; /*!< Details that will help debugging the error */

} data_app_error_event;

NB_Error    data_app_error_event_init(data_util_state* pds, data_app_error_event* appErrorEvent);
void        data_app_error_event_free(data_util_state* pds, data_app_error_event* appErrorEvent);

NB_Error    data_app_error_event_from_tps(data_util_state* pds, data_app_error_event* appErrorEvent, tpselt te);
tpselt      data_app_error_event_to_tps(data_util_state* pds, data_app_error_event* appErrorEvent);

boolean     data_app_error_event_equal(data_util_state* pds, data_app_error_event* left, data_app_error_event* right);
NB_Error    data_app_error_event_copy(data_util_state* pds, data_app_error_event* dest, data_app_error_event* src);

uint32      data_app_error_event_get_tps_size(data_util_state* pds, data_app_error_event* appErrorEvent);
void        data_app_error_event_to_buf(data_util_state* pds, data_app_error_event* appErrorEvent, struct dynbuf *buffer);
NB_Error    data_app_error_event_from_binary(data_util_state* pds, data_app_error_event* appErrorEvent, byte** ppdata, size_t* pdatalen);

/*! @} */

#endif
