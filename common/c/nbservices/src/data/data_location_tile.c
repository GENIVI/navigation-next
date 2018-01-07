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

    @file     data_location_tile.c
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

#include "data_location_tile.h"

NB_Error
data_location_tile_init( data_util_state* state, data_location_tile* plt )
{
    plt->tile_x = 0;
    plt->tile_y = 0;

    return data_blob_init( state, &plt->db_fragment );
}

void
data_location_tile_free( data_util_state* state, data_location_tile* plt )
{
    data_blob_free( state, &plt->db_fragment );
    plt->tile_x = 0;
    plt->tile_y = 0;
}

NB_Error
data_location_tile_from_tps( data_util_state* state, data_location_tile* plt, tpselt tpsElement )
{
    NB_Error err = NE_OK;
    
    err = data_blob_from_tps_attr( state, &plt->db_fragment, tpsElement, "db-fragment" );
    plt->tile_x = te_getattru( tpsElement, "tile-x" );
    plt->tile_y = te_getattru( tpsElement, "tile-y" );

    if (err != NE_OK)
    {
        data_location_tile_free(state, plt);
    }

    return err;
}

boolean
data_location_tile_equal( data_util_state* state, data_location_tile* plt1, data_location_tile* plt2 )
{
    return (boolean) ( data_blob_equal( state, &plt1->db_fragment, &plt2->db_fragment ) &&
         plt1->tile_x == plt2->tile_x &&
         plt1->tile_y == plt2->tile_y );
}

NB_Error
data_location_tile_copy( data_util_state* state, data_location_tile* destinationplt, data_location_tile* sourceplt )
{
    NB_Error err = NE_OK;

    err = data_blob_copy( state, &destinationplt->db_fragment, &sourceplt->db_fragment );

    if (err == NE_OK)
    {
        destinationplt->tile_x = sourceplt->tile_x;
        destinationplt->tile_y = sourceplt->tile_y;
    }

    return err;
}

/*! @} */
