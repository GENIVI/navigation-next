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

    @file     data_location_reply.c
    
    Implementation for TPS replies from the Location servlet.
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_location_reply.h"

NB_Error
data_location_reply_init(data_util_state* pds, data_location_reply* plr)
{
    NB_Error err = NE_OK;

    err = err ? err : data_location_position_init(pds, &plr->cellid_position);
    err = err ? err : data_location_error_init(pds, &plr->cellid_error);
    plr->location_tiles = NULL;
    plr->use_location_setting = FALSE;
    
    return err;
}

void
data_location_reply_free(data_util_state* pds, data_location_reply* plr)
{
    data_location_position_free(pds, &plr->cellid_position);
    data_location_error_free(pds, &plr->cellid_error);
    DATA_VEC_FREE( pds,  plr->location_tiles, data_location_tile );
    plr->use_location_setting = FALSE;
}

NB_Error
data_location_reply_from_tps(data_util_state* pds, data_location_reply* plr, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce = 0;
    int iterator = 0;

    if ( te == NULL )
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_location_reply_free( pds, plr );
    err = data_location_reply_init( pds, plr );

    if (err != NE_OK)
    {
        return err;
    }

    ce = te_getchild(te, "networklocation-position");
    if (ce != NULL)
    {
        err = err ? err : data_location_position_from_tps(pds, &plr->cellid_position, ce);
    }

    ce = te_getchild(te, "networklocation-error");
    if (ce != NULL)
    {
        err = err ? err : data_location_error_from_tps(pds, &plr->cellid_error, ce);
    }

    ce = te_getchild(te, "use-location-setting");
    if (ce != NULL)
    {
        plr->use_location_setting = TRUE;
    }

    while ( ( ce = te_nextchild( te, &iterator ) ) != NULL )
    {
        if ( ce && ( nsl_stricmp( te_getname( ce ), "location-tile" ) == 0 ) )
        {            
            if ( plr->location_tiles == NULL )
            {
                DATA_VEC_ALLOC( err, plr->location_tiles, data_location_tile );
            }

            DATA_FROM_TPS_ADD_TO_VEC( pds, err, ce, plr->location_tiles, data_location_tile );

            if ( err != NE_OK )
            {
                goto errexit;
            }
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_location_reply_free(pds, plr);
    }
    return err;
}

/*! @} */
