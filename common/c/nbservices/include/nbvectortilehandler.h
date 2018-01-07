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

    @file     nbvectortilehandler.h

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


#ifndef VECTORTILEHANDLER_H
#define VECTORTILEHANDLER_H

#include "nbhandler.h"
#include "nbvectortileparameters.h"
#include "nbvectortileinformation.h"

/*!
	@addtogroup nbvectortilehandler
	@{
*/


/*! @struct NB_VectorTileHandler
A NB_VectorTileHandler is used to get a vector tile
*/
typedef struct _NB_VectorTileHandler NB_VectorTileHandler;


/*! Create and initialize a new VectorTileHandler object

You must call NB_VectorTileHandlerDestroy when you are done with using the NB_VectorTileHandler object.

@param context NB_Context
@param callback Callback used for tile downloads.
@param handler On success, the newly created handler; NULL otherwise.  
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileHandlerCreate(
    NB_Context* context,
    NB_RequestHandlerCallback* callback,
    NB_VectorTileHandler** handler
    );     


/*! Destroy a previously created VectorTileHandler object

@param pThis A VectorTileHandler object created with NB_VectorTileHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error
NB_VectorTileHandlerDestroy(
    NB_VectorTileHandler* pThis
    );


/*! Start a network request to get vector tiles

@param pThis A NB_VectorTileHandler object
@param parameters NB_VectorTileParameters specifying vector tile request parameters
@returns NB_Error 
*/
NB_DEC NB_Error 
NB_VectorTileHandlerStartRequest(
	NB_VectorTileHandler* pThis,
	NB_VectorTileParameters* parameters
	);


/*! Cancel a previously started request

@param pThis A NB_VectorTileHandler object
@returns NB_Error
*/
NB_DEC NB_Error 
NB_VectorTileHandlerCancelRequest(
	NB_VectorTileHandler* pThis
	);


/*! Check if a download request is in progress

@param pThis A NB_VectorTileHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean 
NB_VectorTileHandlerIsRequestInProgress(
	NB_VectorTileHandler* pThis
	);


/*! Retrieve downloaded vector tiles.

    Call this function once all the vector tiles have been downloaded.

@param pThis A NB_VectorTileHandler object
@param information A NB_VectorTileInformation object with the tiles requested
@return NB_Error
*/
NB_DEC NB_Error
NB_VectorTileHandlerGetTileInformation(
    NB_VectorTileHandler* pThis,
	NB_VectorTileInformation** information
    );

/*! @} */

#endif // VECTORTILEHANDLER_H
