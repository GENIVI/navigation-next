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

 @file     NBSpecialRegionProcessorPrivate.h
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NBSPECIALREGIONPROCESSORPRIVATE_H
#define NBSPECIALREGIONPROCESSORPRIVATE_H

#include "paltypes.h"
#include "nbspecialregionprocessor.h"
#include "nbspecialregioninformationprivate.h"
#include "nbnavigation.h"

/*! struct of special region state item. */
typedef struct NB_SpecialRegionStateItem
{
    uint32         index;
    nb_boolean     ifIn;
    nb_boolean     isAlerted;
} NB_SpecialRegionStateItem;

/*! struct of special region state. */
typedef struct NB_SpecialRegionState
{
    NB_Context*       context;
    struct CSL_Vector* currentRegions;
} NB_SpecialRegionState;

/*! Create special region state

 @return NB_Error
 */
NB_DEC NB_Error
NB_SpecialRegionStateCreate(NB_Context* context,                             /*!< context NB_Context. */
                            NB_SpecialRegionState** specialRegionState       /*!< specialRegionState special region state be created. */
                            );

/*! Destroy the special region state.

 @return NB_Error.
 */
NB_DEC NB_Error
NB_SpecialRegionStateDestroy(NB_SpecialRegionState* specialRegionState       /*!< special region state object*/
                             );


/*! Call enhancedContentManager to download special region picture.

 @return NE_OK
 */
NB_DEC NB_Error
NB_SpecialRegionProcessorDownloadPicture(NB_RouteInformation* route,                   /*!< route information. */
                                         NB_SpecialRegionState* specialRegionState,    /*!< special region state. */
                                         NB_SpecialRegionInformation* information      /*!< special region information. */
                                         );

/*! Call back when special region state data change.
 */
typedef void (*NB_NavigationSpecialRegionCallbackFunctionProtected)(NB_SpecialRegionStateData* specialRegionData,    /*!< special region state data. */
                                                                    const char* sign_id,                             /*!< sign id. */
                                                                    void* userData                                   /*!< user data. */
                                                                    );

/*! Update special region state and callback if state change.

    This function can be called after gps location update and after a new route reply.
 @return NE_OK on success.
 */
NB_DEC NB_Error
NB_SpecialRegionProcessorUpdate(NB_SpecialRegionState* specialRegionState,                     /*!< special region state. */
                                NB_RouteInformation* route,                                    /*!< route information. */
                                NB_SpecialRegionInformation* information,                      /*!< special region information. */
                                NB_PositionRoutePositionInformation* currentPosition,          /*!< current position at maneuver. */
                                NB_NavigationSpecialRegionCallbackFunction callback,           /*!< special region state change callback. */
                                void* userData                                                 /*!< user private data for callback .*/
                                );

#endif

/*! @} */
