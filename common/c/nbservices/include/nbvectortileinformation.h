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

@file     nbvectortileinformation.h

This API provides for retrieving and using vector tiles
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

#ifndef VECTORTILEINFORMATION_H
#define VECTORTILEINFORMATION_H

#include "paltypes.h"
#include "nbexp.h"
#include "nbvectortile.h"


/*! 
	@addtogroup nbvectortileinformation
	@{ 
*/

// Public Types ..................................................................................

/*! @struct NB_VectorTileInformation
Information about the results of a VectorTile request
*/
typedef struct _NB_VectorTileInformation NB_VectorTileInformation;


/*! Returns a vector tile

You must call NB_VectorTileInformationGetTileCount to get the
total number of tiles that were downloaded. You must
call this function multiple times specifying a index to retrieve
all the tiles that were downloaded.

Any tile that you received must be destroyed after its use by
calling NB_VectorTileInformationDestroyTile.

@param pThis A NB_VectorTileInformation objects
@param tileIndex The index (starting from 0) of the vector tile to retrieve.
@param tile Requested vector tile
@returns NB_Error
@see NB_VectorTileInformationGetTileCount
*/
NB_DEC NB_Error 
NB_VectorTileInformationGetTile(
	NB_VectorTileInformation* pThis,
	uint32	tileIndex,
	NB_VectorTile** tile
	);


/*! Destroys a vector tile

Each tile obtained from NB_VectorTileInformationGetTile must be destroyed
after use using this function.

@param tile The tile object to destroy
@returns NB_Error
@see NB_VectorTileInformationGetTile
*/
NB_DEC NB_Error 
NB_VectorTileInformationDestroyTile(
	NB_VectorTile* tile
	);


/*! Get the number of downloaded tiles.

@param information A NB_VectorTileInformation objects
@param count The number of tiles downloaded
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileInformationGetTileCount(
    NB_VectorTileInformation* information,
    uint32* count
    );
    

/*! Destroy a previously created VectorTileInformation object

@param information A NB_VectorTileHandler object created with NB_VectorTileHandlerGetVectorTileInformation()
@returns NB_Error
*/
NB_DEC NB_Error 
NB_VectorTileInformationDestroy(
	NB_VectorTileInformation* information
	);


/*! @} */

#endif // VECTORTILEINFORMATION_H
