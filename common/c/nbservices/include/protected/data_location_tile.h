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

    @file     data_location_tile.h
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

#ifndef DATA_LOCATION_TILE_H
#define DATA_LOCATION_TILE_H

#include "datautil.h"
#include "data_string.h"
#include "data_blob.h"

typedef struct data_location_tile_
{
    /* Child Elements */
   
    /* Attributes */
    uint32          tile_x;           /*!< x coordinate of the tile. */
    uint32          tile_y;           /*!< y coordinate of the tile. */
    data_blob       db_fragment;      /*!< Database fragment containing the Cell ID and Wi-Fi data for the tile. */

} data_location_tile;

NB_Error    data_location_tile_init(data_util_state* state, data_location_tile* plt);
void        data_location_tile_free(data_util_state* state, data_location_tile* plt);

NB_Error    data_location_tile_from_tps(data_util_state* state, data_location_tile* plt, tpselt tpsElement);

boolean     data_location_tile_equal(data_util_state* state, data_location_tile* plt1, data_location_tile* plt2);
NB_Error    data_location_tile_copy(data_util_state* state, data_location_tile* dest_plt, data_location_tile* src_plt);

#endif // DATA_LOCATION_TILE_H

/*! @} */
