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

    @file     data_data_availability.c
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

#include "data_data_availability.h"

NB_Error
data_data_availability_init(data_util_state* state, data_data_availability* dataAvail)
{
    nsl_memset(dataAvail, 0, sizeof(data_data_availability));
    return NE_OK;
}

void
data_data_availability_free(data_util_state* state, data_data_availability* dataAvail)
{
}

NB_Error
data_data_availability_from_tps(data_util_state* state, data_data_availability* dataAvail, tpselt tpsElement)
{
    NB_Error err = NE_OK;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, err, dataAvail, data_data_availability);
    if (err == NE_OK)
    {
        dataAvail->no_map_data             = (boolean)(te_getchild(tpsElement, "no-map-data") != NULL);
        dataAvail->partial_visual_map_data = (boolean)(te_getchild(tpsElement, "partial-visual-map-data") != NULL);
        dataAvail->full_visual_map_data    = (boolean)(te_getchild(tpsElement, "full-visual-map-data") != NULL);
        dataAvail->partial_routing_data    = (boolean)(te_getchild(tpsElement, "partial-routing-data") != NULL);
        dataAvail->full_routing_data       = (boolean)(te_getchild(tpsElement, "full-routing-data") != NULL);
        dataAvail->partial_poi_data        = (boolean)(te_getchild(tpsElement, "partial-poi-data") != NULL);
        dataAvail->full_poi_data           = (boolean)(te_getchild(tpsElement, "full-poi-data") != NULL);
        dataAvail->partial_geocoding_data  = (boolean)(te_getchild(tpsElement, "partial-geocoding-data") != NULL);
        dataAvail->full_geocoding_data     = (boolean)(te_getchild(tpsElement, "full-geocoding-data") != NULL);
    }

errexit:
    if (err != NE_OK)
    {
        data_data_availability_free(state, dataAvail);
    }

    return err;
}

boolean
data_data_availability_equal(data_util_state* state, data_data_availability* dataAvail1, data_data_availability* dataAvail2)
{
    return (dataAvail1->no_map_data             == dataAvail2->no_map_data &&
            dataAvail1->partial_visual_map_data == dataAvail2->partial_visual_map_data &&
            dataAvail1->full_visual_map_data    == dataAvail2->full_visual_map_data &&
            dataAvail1->partial_routing_data    == dataAvail2->partial_routing_data &&
            dataAvail1->full_routing_data       == dataAvail2->full_routing_data &&
            dataAvail1->partial_geocoding_data  == dataAvail2->partial_geocoding_data &&
            dataAvail1->full_geocoding_data     == dataAvail2->full_geocoding_data &&
            dataAvail1->partial_poi_data        == dataAvail2->partial_poi_data &&
            dataAvail1->full_poi_data           == dataAvail2->full_poi_data);
}

NB_Error
data_data_availability_copy(data_util_state* state, data_data_availability* destinationDataAvail, data_data_availability* sourceDataAvail)
{
    NB_Error err = NE_OK;

    DATA_REINIT(state, err, destinationDataAvail, data_data_availability);
    if (err == NE_OK)
    {
        destinationDataAvail->no_map_data             = sourceDataAvail->no_map_data;
        destinationDataAvail->partial_visual_map_data = sourceDataAvail->partial_visual_map_data;
        destinationDataAvail->full_visual_map_data    = sourceDataAvail->full_visual_map_data;
        destinationDataAvail->partial_routing_data    = sourceDataAvail->partial_routing_data;
        destinationDataAvail->full_routing_data       = sourceDataAvail->full_routing_data;
        destinationDataAvail->partial_geocoding_data  = sourceDataAvail->partial_geocoding_data;
        destinationDataAvail->full_geocoding_data     = sourceDataAvail->full_geocoding_data;
        destinationDataAvail->partial_poi_data        = sourceDataAvail->partial_poi_data;
        destinationDataAvail->full_poi_data           = sourceDataAvail->full_poi_data;
    }

    return err;
}

/*! @} */
