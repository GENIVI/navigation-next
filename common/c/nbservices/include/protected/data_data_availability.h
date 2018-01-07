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

    @file     data_data_availability.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunications Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_DATA_AVAILABILITY_H
#define DATA_DATA_AVAILABILITY_H

#include "datautil.h"

typedef struct data_data_availability_
{
    /* Child Elements */
    boolean   no_map_data;             /*!< Empty element present if the country does not have map data available */
    boolean   partial_visual_map_data; /*!< Empty element present if the country has partial coverage for visual map data */
    boolean   full_visual_map_data;    /*!< Empty element present if the country has full coverage for visual map data */
    boolean   partial_routing_data;    /*!< Empty element present if the country has partial coverage for routing data */
    boolean   full_routing_data;       /*!< Empty element present if the country has full coverage for routing data */
    boolean   partial_geocoding_data;  /*!< Empty element present if the country has partial coverage for geocoding data */
    boolean   full_geocoding_data;     /*!< Empty element present if the country has full coverage for geocoding data */
    boolean   partial_poi_data;        /*!< Empty element present if the country has partial coverage for POI data */
    boolean   full_poi_data;           /*!< Empty element present if the country has full coverage for POI data */

    /* Attributes */

} data_data_availability;

NB_Error    data_data_availability_init(data_util_state* state, data_data_availability* dataAvail);
void        data_data_availability_free(data_util_state* state, data_data_availability* dataAvail);

NB_Error    data_data_availability_from_tps(data_util_state* state, data_data_availability* dataAvail, tpselt tpsElement);

boolean     data_data_availability_equal(data_util_state* state, data_data_availability* dataAvail1, data_data_availability* dataAvail2);
NB_Error    data_data_availability_copy(data_util_state* state, data_data_availability* destinationDataAvail, data_data_availability* sourceDataAvail);

#endif // DATA_DATA_AVAILABILITY_H

/*! @} */
