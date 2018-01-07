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

    @file     data_raster_tile_data_source_query.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */


#include "data_raster_tile_data_source_query.h"

#define RASTER_TILE_DATA_SOURCE_QUERY "maptile-source-query"



tpselt      
data_raster_tile_data_source_query_create_tps(uint32 screenWidth, uint32 screenHeight, uint32 screenResolution)
{
    /*
      want-shared-maps configurations includes want-extended-maps 
      functionality as well as the new content-id-template 
      in the url-args-template included with the reply.
      To avoid breaks on servers that don't support want-shared-maps request bost elements.
    */
    tpselt      te;
    char*       lang = "en-US";
    tpselt      ce = NULL;
    tpselt      ne = NULL;
    if( (te = te_new(RASTER_TILE_DATA_SOURCE_QUERY)) != NULL &&
        te_setattru(te, "screen-width", screenWidth) &&
        te_setattru(te, "screen-height", screenHeight) &&
        te_setattru(te, "screen-resolution", screenResolution) &&
        te_setattrc(te, "language", lang) &&
        (ce = te_new("want-shared-maps")) != NULL &&
        (ne = te_new("want-extended-maps")) != NULL &&
        te_attach(te,ne)&&
        te_attach(te,ce))
    {
        return te;
    }
    te_dealloc(te);
    te_dealloc(ce);
    te_dealloc(ne);
    return NULL;
}


/*! @} */
