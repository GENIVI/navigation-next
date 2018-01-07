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

    @file     data_app_discovery.h

    Interface to create App-Discovery TPS element.

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

#ifndef DATA_APP_DISCOVERY_H
#define DATA_APP_DISCOVERY_H

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_string.h"

typedef struct data_app_discovery_
{
    /* Child Elements */
    boolean app_present;

    /* Attributes */
    data_string platform;    // NBI platform that produced this call
    data_string carrier;     // Carrier for the device originating the call
    data_string sdk_version; // NBI SDK version

} data_app_discovery;

NB_Error data_app_discovery_init(data_util_state* pds, data_app_discovery* pad);
void     data_app_discovery_free(data_util_state* pds, data_app_discovery* pad);

boolean  data_app_discovery_equal(data_util_state* pds, data_app_discovery* pad1, data_app_discovery* pad2);
NB_Error data_app_discovery_copy(data_util_state* pds, data_app_discovery* pad_dest, data_app_discovery* pad_src);

tpselt   data_app_discovery_to_tps(data_util_state* pds, data_app_discovery* pad);
uint32   data_app_discovery_get_tps_size(data_util_state* pds, data_app_discovery* pad);
void     data_app_discovery_to_buf(data_util_state* pds, data_app_discovery* pad, struct dynbuf* pdb);
NB_Error data_app_discovery_from_binary(data_util_state* pds, data_app_discovery* pad, byte** pdata, size_t* pdatalen);

#endif //DATA_APP_DISCOVERY_H
/*! @} */
