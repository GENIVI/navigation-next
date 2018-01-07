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

    @file     data_content_displayed.h

    Interface for Content Displayed TPS element.

*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef data_content_displayed_H
#define data_content_displayed_H

#include "datautil.h"
#include "data_string.h"

typedef struct data_content_displayed_
{
    /* Child Elements */

    /* Attributes */
    data_string     type;
    data_string     country;

} data_content_displayed;

NB_Error    data_content_displayed_init(data_util_state* pds, data_content_displayed* pcd);
void        data_content_displayed_free(data_util_state* pds, data_content_displayed* pcd);

tpselt      data_content_displayed_to_tps(data_util_state* pds, data_content_displayed* pcd);

boolean     data_content_displayed_equal(data_util_state* pds, data_content_displayed* pcd1, data_content_displayed* pcd2);
NB_Error    data_content_displayed_copy(data_util_state* pds, data_content_displayed* pcd_dest, data_content_displayed* pcd_src);
int32       data_content_displayed_get_tps_size(data_util_state* pds, data_content_displayed* pcd);
NB_Error    data_content_displayed_from_binary(data_util_state* pds, data_content_displayed* pcd, byte** pdata, size_t* pdatalen);
void        data_content_displayed_to_buf(data_util_state* pds, data_content_displayed* pcd, struct dynbuf* pdb);
#endif

/*! @} */
