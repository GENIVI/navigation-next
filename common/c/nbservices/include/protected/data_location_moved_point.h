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

    @file     data_location_moved_point.h

    Interface for wifi TPS element for the Location servlet.    
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

#ifndef DATA_LOCATION_MOVED_POINT_H
#define DATA_LOCATION_MOVED_POINT_H

#include "nbexp.h"
#include "data_string.h"
#include "datautil.h"


typedef struct data_location_moved_point_
{
    /* Child Elements */

    /* Attributes */
    data_string     macAddress;     /*!< The mac address of the wifi node acquired from the device. */
    uint8           cause;          /*!< The cause code for the invalidation. */
    uint32          timeStamp;      /*!< Time (UTC) when the invalidation happened. */


} data_location_moved_point;

NB_Error    data_location_moved_point_init(data_util_state* pds, data_location_moved_point* plp);
void        data_location_moved_point_free(data_util_state* pds, data_location_moved_point* plp);

tpselt      data_location_moved_point_to_tps(data_util_state* pds, data_location_moved_point* plp);

boolean     data_location_moved_point_equal(data_util_state* pds, data_location_moved_point* plp1, data_location_moved_point* plp2);
NB_Error    data_location_moved_point_copy(data_util_state* pds, data_location_moved_point* plp_dest, data_location_moved_point* plp_src);

#endif // DATA_LOCATION_MOVED_POINT_H

/*! @} */
