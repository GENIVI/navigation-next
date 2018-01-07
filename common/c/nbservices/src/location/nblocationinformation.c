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

    @file     nblocationinformation.c

*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "nblocationinformation.h"
#include "nblocationinformationprivate.h"
#include "nbcontextprotected.h"
#include "nbqalog.h"
#include "csltypes.h"
#include "cslnetwork.h"
#include "data_location_reply.h"

/*! @{ */

struct NB_LocationInformation
{
    NB_Context*                     context;
    data_location_reply             reply;
};

/*! Creates a new location tile object.

@param context The NB context
@param tile The location tile
@param tileData The TPS location tile structure
@return NB_Error
*/
static NB_Error SetLocationTile( NB_Context* context, NB_LocationTile* tile, const data_location_tile* tileData );

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationInformationCreate(NB_Context* context, tpselt reply, NB_LocationInformation** information)
{
    NB_LocationInformation* pThis = 0;
    NB_Error err = NE_OK;
    
    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    err = data_location_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);

    if (!err)
    {
        pThis->context = context;
        *information = pThis;

        NB_QaLogCellIdReply(
            context,
            pThis->reply.cellid_position.lat,
            pThis->reply.cellid_position.lon,
            pThis->reply.cellid_position.accuracy,
            pThis->reply.cellid_error.code
        );
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationInformationDestroy(NB_LocationInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_location_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationInformationGetNetworkPosition(
    NB_LocationInformation* pThis,
    NB_LocationRequestStatus* requestStatus,
    NB_LatitudeLongitude* position,
    uint32* accuracyMeters,
    char* networkLocationSource,
    uint32 networkLocationSourceSize)
{
    NB_Error err = NE_OK;
    
    if (!pThis || !requestStatus || !position || !accuracyMeters)
    {
        return NE_INVAL;
    }

    if (pThis->reply.cellid_error.code == 0)
    {
        *requestStatus = NB_LRS_Success;
        position->latitude = pThis->reply.cellid_position.lat;
        position->longitude = pThis->reply.cellid_position.lon;
        *accuracyMeters = pThis->reply.cellid_position.accuracy;

        if (networkLocationSource && networkLocationSourceSize > 1)
        {
            data_string_get_copy(NB_ContextGetDataState(pThis->context), &pThis->reply.cellid_position.sourceType, networkLocationSource, networkLocationSourceSize);
        }
    }
    else
    {
        *requestStatus = pThis->reply.cellid_error.code;
        position->latitude = INVALID_LATLON;
        position->longitude = INVALID_LATLON;
        *accuracyMeters = 0;
    }
    
    return err;
}

NB_DEF NB_Error
NB_LocationInformationGetUseLocationSetting( NB_LocationInformation* information,
                                             nb_boolean* useLocationSetting
                                           )
{
    if ( !information || !useLocationSetting )
    {
        return NE_INVAL;
    }

    *useLocationSetting = information->reply.use_location_setting;

    return NE_OK;
}

NB_DEF NB_Error
NB_LocationInformationGetLocationTilesCount( NB_LocationInformation* information, 
                                                int32* count
                                              )
{
    if ( !information || !count || !information->reply.location_tiles )
    {
        return NE_INVAL;
    }

    *count = CSL_VectorGetLength( information->reply.location_tiles );
    return NE_OK;
}

NB_DEF NB_Error
NB_LocationInformationGetLocationTile( NB_LocationInformation* information,
                                       int32 index,
                                       NB_LocationTile* tile
                                     )
{
    data_location_tile* tmpTile = NULL;

    if ( !information || !tile || !information->reply.location_tiles )
    {
        return NE_INVAL;
    }

    if ( index < 0 || index >= CSL_VectorGetLength( information->reply.location_tiles ) )
    {
        return NE_RANGE;
    }

    tmpTile = (data_location_tile *)CSL_VectorGetPointer( information->reply.location_tiles, index );
    if ( tmpTile == NULL )
    {
        return NE_NOENT;
    }

    return SetLocationTile( information->context, tile, tmpTile );
}

/* See header file for description. */ 
NB_DEF NB_Error
NB_LocationInformationGetRequestStatus( NB_LocationInformation* pThis,
                                        NB_LocationRequestStatus* requestStatus
                                      )
{
    NB_Error err = NE_OK;
    
    if ( !pThis || !requestStatus )
    {
        return NE_INVAL;
    }

    if (pThis->reply.cellid_error.code == 0)
    {
        *requestStatus = NB_LRS_Success;
    }
    else
    {
        *requestStatus = pThis->reply.cellid_error.code;
    }
    
    return err;
}


NB_Error
SetLocationTile( NB_Context* context, NB_LocationTile* tile, const data_location_tile* tileData )
{

    if ( context == NULL || tile == NULL || tileData == NULL )
    {
        return NE_INVAL;
    }

    nsl_memset( tile, 0, sizeof( NB_LocationTile ) );
    tile->tileX = tileData->tile_x;
    tile->tileY = tileData->tile_y;
    if ( tileData->db_fragment.data != NULL && tileData->db_fragment.size > 0 )
    {
        tile->dbFragment = tileData->db_fragment.data;
        tile->dbFragmentSize = (uint32)tileData->db_fragment.size;
    }

    return NE_OK;
}

/*! @} */
