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

    @file nbenhancedvectormapmanager.h
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

#ifndef NB_ENHANCED_VECTOR_MAP_MANAGER_H
#define NB_ENHANCED_VECTOR_MAP_MANAGER_H

#include "nbexp.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbcontext.h"
#include "nbenhancedvectormaptypes.h"

/*! Create a new instance of a NB_EnhancedVectorMapManager object

    Use this function to create a new instance of a NB_EnhancedVectorMapManager object.
    Call NB_EnhancedVectorMapManagerDestroy() to destroy the object.

    @return NE_OK for success, NE_NOMEM if memory allocation failed
    @see NB_EnhancedVectorMapManagerDestroy
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerCreate(NB_Context* pContext,                                             /*!< Pointer to current context */
                                  const NB_EnhancedVectorMapManagerConfig* pConfig,                 /*!< Enhanced vector map manager configuration parameters */
                                  NB_EnhancedVectorMapManager** pManager                            /*!< On success, returns pointer to enhanced vector map manager object */
                                  );

/*! Destroy an instance of a NB_EnhancedVectorMapManager object

    Use this function to destroy an existing instance of a NB_EnhancedVectorMapManager object created by
    NB_EnhancedVectorMapManagerCreate().

    @return NE_OK for success
    @see NB_EnhancedVectorMapManagerCreate
*/
NB_DEC void
NB_EnhancedVectorMapManagerDestroy(NB_EnhancedVectorMapManager* pManager                            /*!< Enhanced vector map manager to destroy */
                                   );

/*! Clear old meta data and other resource.

    @return NE_OK for success
*/
NB_DEC void
NB_EnhancedVectorMapManagerClear(NB_EnhancedVectorMapManager* pManager                              /*!< Enhanced vector map manager to destroy */
                                 );

/*! Start to request metadata from server

    NE_BUSY will be returned if a same request is existing.

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerRequestMetadata(NB_EnhancedVectorMapManager* pManager,                   /*!< Enhanced vector map manager instance */
                                           EnhancedVectorMapMetadataRequestCallback callback        /*!< callback */
                                           );

/*! Cancel a existing metadata request

    NE_UNEXPECTED will be returned if no request existing.

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerCancelMetadataRequest(NB_EnhancedVectorMapManager* pManager              /*!< Enhanced vector map manager instance */
                                                 );

/*! Start to request common material from server

    You can only start request when metadata is ready. or NE_UNEXPECTED will be returned.
    NE_BUSY will be returned if a same request is existing.

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerRequestCommonMaterial(NB_EnhancedVectorMapManager* pManager,             /*!< Enhanced vector map manager instance */
                                                 EnhancedVectorMapCommonMaterialRequestCallback callback    /*!< callback */
                                                 );

/*! Cancel a existing common material request

    NE_UNEXPECTED will be returned if no request existing.

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerCancelCommonMaterialRequest(NB_EnhancedVectorMapManager* pManager        /*!< Enhanced vector map manager instance */
                                                       );

/*! Set enabled layers

    Bits set to 1 is enabled layer, or, layer will not be download when request tile.
    All layers are initially enabled

    @return None
    @see NB_TileLayerType
*/
NB_DEC void
NB_EnhancedVectorMapManagerSetEnabledLayers(NB_EnhancedVectorMapManager* pManager,                  /*!< Enhanced vector map manager instance */
                                            uint32 enabledLayers                                    /*!< enabled layer flags */
                                            );

/*! Get layer draw order

    @return NE_OK for success
    @see NB_TileLayerType
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerGetLayerDrawOrder(NB_EnhancedVectorMapManager* pManager,                 /*!< Enhanced vector map manager instance */
                                             NB_TileLayerType layerType,                            /*!< layer type */
                                             uint32* drawOrder,                                     /*!< layer draw order */
                                             uint32* labelDrawOrder                                 /*!< layer label draw order */
                                             );

/*! Get day material file name

    @return NULL if material file not ready
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerGetDayMaterial(NB_EnhancedVectorMapManager* pManager,                    /*!< Enhanced vector map manager instance */
                                          void** data,                                              /*!< Returns material data, user have to call nsl_free to release this data */
                                          uint32* size                                              /*!< Returns data size */
                                          );

/*! Get night material file name

    @return NULL if material file not ready
*/
NB_DEC NB_Error
NB_EnhancedVectorMapManagerGetNightMaterial(NB_EnhancedVectorMapManager* pManager,                  /*!< Enhanced vector map manager instance */
                                            void** data,                                            /*!< Returns material data, user have to call nsl_free to release this data */
                                            uint32* size                                            /*!< Returns data size */
                                            );

#endif

/*! @} */
