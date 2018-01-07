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

    @file     data_nav_invocation_config.h

    Interface to create Nav-Invocation-Config TPS element.

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
#ifndef DATA_NAV_INVOCATION_CONFIG
#define DATA_NAV_INVOCATION_CONFIG

#include "nbexp.h"
#include "datautil.h"

typedef struct data_nav_invocation_config_ {
    /* Child Elements */

    /* Attributes */
    uint32  priority;                   /*!< The priority of this category of events relative to other categories
                                          of events.  Valid range is 0 to 10, where 10 is the highest priority.
                                          Priority 0 means that no events of in this category shall be collected,
                                          and all events cached on the client within this category shall be deleted
                                          without being uploaded to the server. */

} data_nav_invocation_config;

NB_Error    data_nav_invocation_config_init(data_util_state* pds, data_nav_invocation_config* pnic);
void        data_nav_invocation_config_free(data_util_state* pds, data_nav_invocation_config* pnic);

NB_Error    data_nav_invocation_config_from_tps(data_util_state* pds, data_nav_invocation_config* pnic, tpselt te);

boolean     data_nav_invocation_config_equal(data_util_state* pds, data_nav_invocation_config* pnic1,
                data_nav_invocation_config* pnic2);

NB_Error    data_nav_invocation_config_copy(data_util_state* pds, data_nav_invocation_config* pnic_dest,
                data_nav_invocation_config* pnic_src);

#endif //DATA_NAV_INVOCATION_CONFIG
/*! @} */
