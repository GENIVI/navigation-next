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

    @file nbenhancedvectormapprocessorprivate.h
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

#ifndef NB_ENHANCED_VECTOR_MAP_PROCESSOR_PRIVATE_H
#define NB_ENHANCED_VECTOR_MAP_PROCESSOR_PRIVATE_H

#include "nbcontext.h"
#include "nbnavigation.h"
#include "nbenhancedcontentprocessorprivate.h"
#include "nbenhancedcontenttypes.h"

/*! Create a NB_NavEnhancedVectorMapState object

    @return NE_OK for success
*/
NB_DEC NB_Error
NB_EnhancedVectorMapStateCreate(NB_Context* pContext,                                       /*!< Pointer to current context */
                                NB_Navigation* pNavigation,                                 /*!< Navigation session instance */
                                NB_EnhancedVectorMapConfiguration* cnofiguration,           /*!< the initial configuration */
                                NB_EnhancedVectorMapState** pState                          /*!< On success, returns pointer to enhanced vector map manager object */
                                );

/*! Create a NB_NavEnhancedVectorMapState object

    @return NE_OK for success
*/
NB_DEC void
NB_EnhancedVectorMapStateDestroy(NB_EnhancedVectorMapState* pState                          /*!< Enhanced vector map state to destroy */
                                 );

/*! Return the bounding box index list based on the given area/polygon.

    This function returns bounding boxes where the area lies.
    User should call nsl_free with 'boundingBoxIndexes' if it is not NULL

    @return None
*/
NB_EXT_C void
NB_GetECMBoundingBoxes(const NB_Vector* area,                                   /*!< Area/Polygon to check */
                       uint32 pointCount,                                       /*!< Number of points in area/polygon */
                       NB_EnhancedCityModelBoundingBoxParameters* ecmBoundingBoxes,/*!< city bounding box parameters */
                       uint32* boundingBoxesCount,                              /*!< return count of bounding boxes */
                       uint32** boundingBoxIndexes                              /*!< return bounding box index array */
                       );

#endif

/*! @} */

