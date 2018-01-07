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

    @file     data_content_path.h
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

#ifndef DATA_CONTENT_PATH_H
#define DATA_CONTENT_PATH_H

#include "nbexp.h"
#include "data_point.h"
#include "data_string.h"
#include "data_blob.h"
#include "datautil.h"

typedef struct data_content_path_
{
    /* Child Elements */
    data_point      start_point;            /*!< If present, the route starts at this point along the given spline.*/
    data_point      end_point;              /*!< If present, the route ends at this point along the given spline.*/

    /* Attributes */
    data_string     id;                     /*!< The identifier used to load the path data spline data file. (spline id) */
    data_blob       route_spline_packed;    /*!< Content of the spline binary data file. */

} data_content_path;

NB_Error    data_content_path_init(data_util_state* state, data_content_path* contentPath);
void        data_content_path_free(data_util_state* state, data_content_path* contentPath);

NB_Error    data_content_path_from_tps(data_util_state* state, data_content_path* contentPath, tpselt tpsElement);

NB_Error    data_content_path_copy(data_util_state* state, data_content_path* destinationContentPath, data_content_path* sourceContentPath);

#endif // DATA_CONTENT_PATH_H

/*! @} */
