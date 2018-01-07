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

    @file nbenhancedvectormapprocessor.h
    @date 02/21/2012

*/
/*
    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NB_ENHANCED_VECTOR_MAP_PROCESSOR_H
#define NB_ENHANCED_VECTOR_MAP_PROCESSOR_H

#include "paltypes.h"
#include "nberror.h"
#include "nbexp.h"
#include "nbenhancedvectormaptypes.h"
#include "nbvectortile.h"

/*! Update internal status

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedVectorMapStateUpdate(NB_EnhancedVectorMapState* pState                           /*!< Enhanced vector map state instance */
                                );

/*! Set camera position

    Camera position is used for calculating visible tiles.

    @return NE_OK for success.
    @see NB_EnhancedVectorMapStateGetVisibleTileKeyList
    @see NB_EnhancedVectorMapStateGetVisibleTileNameList
*/
NB_DEC NB_Error
NB_EnhancedVectorMapStateSetCameraPosition(NB_EnhancedVectorMapState* state,                /*!< Enhanced vector map state instance */
                                           double latitude,                                 /*!< camera center latitude */
                                           double longitude,                                /*!< camera center longitude */
                                           double heading                                   /*!< camera heading */
                                           );

/*! Get visible tile keys

    After setting camera positing, you can get all visible tile keys no matter tile file is available or not.

    @return None
    @see NB_EnhancedVectorMapStateGetVisibleTileNameList
    @see NB_EnhancedVectorMapStateSetCameraPosition
*/
NB_DEC void
NB_EnhancedVectorMapStateGetVisibleTileKeyList(NB_EnhancedVectorMapState* state,            /*!< Enhanced vector map state instance */
                                               const NB_TileKey** tileKeys,                 /*!< returns tile key array */
                                               uint32* count                                /*!< returns tile key count */
                                               );

/*! To notify TileIds that already loaded

    Call this function to tell all tiles that loaded. Then NB_EnhancedVectorMap will use callback
    to notify which tile should be load or unload.

    @return None
    @see NB_EnhancedVectorMapStateGetVisibleTileKeyList
*/
NB_DEC void
NB_EnhancedVectorMapStateUpdateLoadedTileIdList(NB_EnhancedVectorMapState* state,           /*!< Enhanced vector map state instance */
                                                const NB_TileId* tileIds,                   /*!< TileId list */
                                                uint32 count                                /*!< TileId count */
                                                );

/*! Get visible quadrangle

    In perspective view, an area on XY plan can be seen on screen.
    This function is used for calculateing the area.

    @return None
    @see NB_EnhancedVectorMapStateGetVisibleTileNameList
    @see NB_EnhancedVectorMapStateSetCameraPosition
*/
NB_DEC void
NB_EnhancedVectorMapStateGetVisibleQuadrangle(NB_EnhancedVectorMapState* state,             /*!< Enhanced vector map state instance */
                                              NB_MercatorPoint visibleQuadrangle[4]         /*!< the 4 mercator vertex of visible area in the order of bottom left, top left, top right and bottom right. */
                                              );
#endif

/*! @} */
