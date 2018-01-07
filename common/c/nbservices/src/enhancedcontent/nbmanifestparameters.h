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

    @file     nbmanifestparameters.h
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

#ifndef NBMANIFESTPARAMETERS_H
#define NBMANIFESTPARAMETERS_H

#include "nberror.h"
#include "nbcontext.h"
#include "tpselt.h"
#include "nbmetadatatypes.h"

/*!
    @addtogroup nbmanifestparameters
    @{
*/


/*! @struct NB_ManifestParameters
*/
typedef struct NB_ManifestParameters NB_ManifestParameters;


/*! Create parameters for a manifest operation

@param context context instance
@param datastoreParameters On success, a newly created NB_ManifestParameters object; NULL otherwise.  A valid object must be destroyed using NB_ManifestParametersDestroy().
@returns NB_Error
*/
NB_Error NB_ManifestParametersCreate(NB_Context* context, NB_ManifestParameters** parameters);


/*! Destroy a previously created NB_ManifestParameters object

@param parameters An NB_ManifestParameters object created with NB_ManifestParametersCreate()
@returns NB_Error
*/
NB_Error NB_ManifestParametersDestroy(NB_ManifestParameters* parameters);


/*! Get the tps network query from the parameters

@param parameters The parameters to get the query from
@returns The TPS element created from the parameters.  A non-NULL value must be deleted using tps_dealloc
*/
tpselt NB_ManifestParametersToTPSQuery(NB_ManifestParameters* parameters);


/*! Add a wanted content in a NB_ManifestParameters object

@param parameters An NB_ManifestParameters object created with NB_ManifestParametersCreate()
@param country Country code - ISO 3166-1 alpha 3.
@param datasetId Dataset id. Empty isn't recommended and may cause network timeout.
@param type One of : "textures", "city models", "realistic signs"' or "junctions". If empty, all types with the given dataset are returned.
@returns NB_Error
*/
NB_Error NB_ManifestParametersAddWantedContent(NB_ManifestParameters* parameters,
                                               const char* country,
                                               const char* datasetId,
                                               NB_EnhancedContentDataType type);


/*! @} */

#endif
