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

    @file     data_extended_content_region.h
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

#ifndef DATA_EXTENDED_CONTENT_REGION_H
#define DATA_EXTENDED_CONTENT_REGION_H

#include "nbexp.h"
#include "data_string.h"
#include "datautil.h"
#include "data_projection_parameters.h"

typedef struct data_extended_content_region_
{
    /* Child Elements */
    CSL_Vector* vec_content_path;                     /*!< The set of content paths (splines) that run along the
                                                           navigation route within the extended content region. */
    data_projection_parameters projection_parameters; /*!< Projection parameters for this content. */

    /* Attributes */
    data_string     id;                 /*!< The identifier used to retrieve the extended content from the content server. */
    data_string     version;            /*!< The version of this content. This is the version of the individual
                                             content file, not the dataset manifest version. */
    data_string     dataset_id;         /*!< The id associated with the dataset where this content can be found. */
    data_string     type;               /*!< The type of extended content. One of: "city-model", or "junction-model". */

    uint32      start_maneuver_index;   /*!< The maneuver index for the point where the route enters this content
                                             region or the start of the route if it begins in this region.*/
    double      start_maneuver_offset;  /*!< The distance in meters from the start of the maneuver referenced by
                                             start-maneuver-index index for the point where the route enters this
                                             content region or the start of the route if it begins in this region. */
    uint32      end_maneuver_index;     /*!< The maneuver index for the point where the route leaves this content
                                             region or the end of the route if it begins in this region. */
    double      end_maneuver_offset;    /*!< The distance in meters from the start of the maneuver referenced by
                                             end-maneuver-index index for the point where the route leaves this
                                             content region or the start of the route if it ends in this region. */
    data_string projection;             /*!< Map projection type. */

} data_extended_content_region;

NB_Error    data_extended_content_region_init(data_util_state* state, data_extended_content_region* extendedContentRegion);
void        data_extended_content_region_free(data_util_state* state, data_extended_content_region* extendedContentRegion);

NB_Error    data_extended_content_region_from_tps(data_util_state* state, data_extended_content_region* extendedContentRegion, tpselt tpsElement);

NB_Error    data_extended_content_region_copy(data_util_state* state, data_extended_content_region* destinationExtendedContentRegion, data_extended_content_region* sourceExtendedContentRegion);

#endif // DATA_EXTENDED_CONTENT_REGION_H

/*! @} */
