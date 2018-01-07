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

    @file     data_use_navigator.h

    Interface to create Use-Navigator TPS element.

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

/*! @{ */
#ifndef DATA_USE_NAVIGATOR_H
#define DATA_USE_NAVIGATOR_H

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_place.h"
#include "data_route_style.h"

typedef struct data_use_navigator_
{
    /* Child Elements */
    data_place place;             // Destination for the navigation session
    data_route_style route_style; // The style of route requested

    /* Attributes */
    data_string platform;     // NBI platform that produced this call
    data_string carrier;      // Carrier for the device originating the call
    data_string sdk_version;  // NBI SDK version

} data_use_navigator;

NB_Error data_use_navigator_init(data_util_state* pds, data_use_navigator* pun);
void     data_use_navigator_free(data_util_state* pds, data_use_navigator* pun);

boolean  data_use_navigator_equal(data_util_state* pds, data_use_navigator* pun1, data_use_navigator* pun2);
NB_Error data_use_navigator_copy(data_util_state* pds, data_use_navigator* pun_dest, data_use_navigator* pun_src);

tpselt   data_use_navigator_to_tps(data_util_state* pds, data_use_navigator* pun);
uint32   data_use_navigator_get_tps_size(data_util_state* pds, data_use_navigator* pun);
void     data_use_navigator_to_buf(data_util_state* pds, data_use_navigator* pun, struct dynbuf* pdb);
NB_Error data_use_navigator_from_binary(data_util_state* pds, data_use_navigator* pun, byte** pdata, size_t* pdatalen);

#endif //DATA_USE_NAVIGATOR_H
/*! @} */
