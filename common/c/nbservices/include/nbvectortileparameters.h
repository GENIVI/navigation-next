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

@file     nbvectortileparameters.h

API to create parameter objects that is passed to handlers.

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef VECTORTILEPARAMETERS_H
#define VECTORTILEPARAMETERS_H

#include "paltypes.h"
#include "nbexp.h"
#include "nbvectortile.h"

/*! 
	@addtogroup nbvectortileparameters
	@{ 
*/

/*! @struct NB_VectorTileParameters
Opaque data structure that defines the parameters of a VectorTile operation 
*/
typedef struct _NB_VectorTileParameters NB_VectorTileParameters;


typedef struct 
{
    uint32	    minPpriority;
    uint32	    maxPriority;
    double	    minScale;

    uint32	    passes;
    uint32	    zoomLevel;
    nb_boolean	allow_tile_fetch;
} NB_VectorTileParametersConfiguration;


/*! Set defaults for configurations.

@param configuration A pointer to NB_VectorTileParametersConfiguration structure
@return None
*/
NB_DEC void
NB_VectorTileParametersConfigurationSetDefaults(
    NB_VectorTileParametersConfiguration* configuration);



/*! Create VectorTile parameters object 

Once the you have a NB_VectorTileParameters object, you can add tiles to the parameters by using
NB_VectorTileParametersAddTile function.

You must call NB_VectorTileParametersDestroy to dispose of the object.

@param context NAVBuilder Context Instance
@param configuration Parameters configuration
@param tileIndex A NB_VectorTileIndex to request. See definition for NB_VectorTileIndex.
@param parameters On success, a newly created NB_VectorTileParameters object; NULL otherwise.  
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileParametersCreate(
    NB_Context* context,
    NB_VectorTileParametersConfiguration* configuration,
    NB_VectorTileIndex* tileIndex,
    NB_VectorTileParameters** parameters
    );


/*! Add additional tiles to VectorTileParameters 

@param parameters NB_VectorTileParameters that was returned from a call to NB_VectorTileParametersCreate
@param tileIndex A NB_VectorTileIndex of the tile to add to the request parameters
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileParametersAddTile(
    NB_VectorTileParameters* parameters,
    NB_VectorTileIndex* tileIndex
    );


/*! Destroy a previously created NB_VectorTileParameters object

@param parameters A NB_VectorTileParameterss object created with a call to the NB_VectorTileParametersCreate function
@returns NB_Error
*/
NB_DEC NB_Error 
NB_VectorTileParametersDestroy(
    NB_VectorTileParameters* parameters
    );

/*! @} */

#endif // VECTORTILEPARAMETERS_H

