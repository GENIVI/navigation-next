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

    @file     data_content_path.c
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

#include "data_content_path.h"

NB_Error
data_content_path_init(data_util_state* state, data_content_path* contentPath)
{
    NB_Error err = NE_OK;

    err = err ? err : data_point_init(state, &contentPath->start_point);
    err = err ? err : data_point_init(state, &contentPath->end_point);

    err = err ? err : data_string_init(state, &contentPath->id);
    err = err ? err : data_blob_init(state, &contentPath->route_spline_packed);

    return NE_OK;
}

void
data_content_path_free(data_util_state* state, data_content_path* contentPath)
{
    data_point_free(state, &contentPath->start_point);
    data_point_free(state, &contentPath->end_point);

    data_string_free(state, &contentPath->id);
    data_blob_free(state, &contentPath->route_spline_packed);
}

NB_Error
data_content_path_from_tps(data_util_state* state, data_content_path* contentPath, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt childElement = 0;

    if (!tpsElement)
    {
        return NE_INVAL;
    }

    data_content_path_free(state, contentPath);

    err = data_content_path_init(state, contentPath);
    if (err != NE_OK)
    {
        return err;
    }

    childElement = te_getchild(tpsElement, "start-point");
    if (childElement)
    {
        err = data_point_from_tps(state, &contentPath->start_point, childElement);
    }

     if (err == NE_OK)
     {
         childElement = te_getchild(tpsElement, "end-point");
         if (childElement)
         {
             err = data_point_from_tps(state, &contentPath->end_point, childElement);
         }

         err = err ? err : data_string_from_tps_attr(state, &contentPath->id, tpsElement, "id");

         err = err ? err : data_blob_from_tps_attr(state, &contentPath->route_spline_packed, tpsElement, "route-spline-packed");
     }

    if (err != NE_OK)
    {
        data_content_path_free(state, contentPath);
    }

    return err;
}

NB_Error
data_content_path_copy(data_util_state* state, data_content_path* destinationContentPath, data_content_path* sourceContentPath)
{
    NB_Error err = NE_OK;

    err = err ? err : data_point_copy(state, &destinationContentPath->start_point, &sourceContentPath->start_point);
    err = err ? err : data_point_copy(state, &destinationContentPath->end_point, &sourceContentPath->end_point);
    err = err ? err : data_string_copy(state, &destinationContentPath->id, &sourceContentPath->id);
    err = err ? err : data_blob_copy(state, &destinationContentPath->route_spline_packed, &sourceContentPath->route_spline_packed);

    return err;
}

/*! @} */
