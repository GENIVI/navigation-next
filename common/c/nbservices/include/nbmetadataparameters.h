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

   @file     nbmetadataparameters.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBMETADATAPARAMETERS_H
#define NBMETADATAPARAMETERS_H

#include "nberror.h"
#include "nbcontext.h"
#include "nbenhancedcontenttypes.h"
#include "nbextappcontent.h"

/*!
    @addtogroup nbmetadataparameters
    @{
*/


/*! @struct NB_MetadataParameters
*/
typedef struct NB_MetadataParameters NB_MetadataParameters;


/*! Create parameters for a metadata operation

@param context              context instance
@param wantExtendedMaps     If present, the client can handle extended map tile configurations in the reply.
@param wantSharedMaps       If present, the client can handle extended maps
                            configurations that include want-extended-maps
                            functionality as well as the new tile-store-template in
                            the url-args-template included with the reply.
@param wantUnifiedMaps      If present, the client can handle the unified map tile
                            configurations in the reply.
@param language             The current language setting of the device.
@param screenWidth          The screen width of the device, specified in pixels.
@param screenHeight         The screen height of the device, specified in pixels.
@param screenResolution     The resolution of the screen, specified in DPI.
@param datastoreParameters  On success, a newly created NB_MetadataParameters object; NULL otherwise.  A valid object must be destroyed using NB_MetadataParametersDestroy().
@param timeStamp            Time stamp of last received metadata-source-reply (got set by
                            last reply from server, CCC do not care its unit).

@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataParametersCreate(NB_Context* context,
                                     nb_boolean wantExtendedMaps,
                                     nb_boolean wantSharedMaps,
                                     nb_boolean wantUnifiedMaps,
                                     const char* language,
                                     uint32 screenWidth,
                                     uint32 screenHeight,
                                     uint32 screenResolution,
                                     uint64 timeStamp,
                                     NB_MetadataParameters** parameters);


/*! Destroy a previously created NB_MetadataParameters object

@param parameters An NB_MetadataParameters object created with NB_MetadataParametersCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataParametersDestroy(NB_MetadataParameters* parameters);


/*! Add a wanted content in a NB_MetadataParameters object

@param parameters An NB_MetadataParameters object created with NB_MetadataParametersCreate()
@param country Country code-ISO 3166-1 alpha 3.
@param datasetId If empty, all datasets of the given type are returned.
@param type One of : "textures", "city models", "realistic signs"' or "junctions". If empty, all types with the given dataset are returned.
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataParametersAddWantedContent(NB_MetadataParameters* parameters,
                                               const char* country,
                                               const char* datasetId,
                                               NB_EnhancedContentDataType type);

/*! Add a want ext app contents in a NB_MetadataParameters object

@param parameters An NB_MetadataParameters object created with NB_MetadataParametersCreate()
@param appMask The int32 mask specified wanted ext app content from NB_ThirdPartyApp
@returns NB_Error
*/
NB_DEC NB_Error NB_MetadataParametersAddWantExtAppContents(NB_MetadataParameters* parameters, int32 appMask);

/*! @} */

#endif
