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

    @file     nbrastermaphandler.h
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

#ifndef NBRASTERMAPHANDLER_H
#define NBRASTERMAPHANDLER_H

#include "nbcontext.h"
#include "nbhandler.h"
#include "nbrastermapinformation.h"
#include "nbrastermapparameters.h"
#include "nbsearchinformation.h"


/*!
    @addtogroup nbrastermaphandler
    @{
*/

/*! @struct NB_RasterMapHandler
A RasterMapHandler is used to download raster map files
*/
typedef struct NB_RasterMapHandler NB_RasterMapHandler;


/*! Create and initialize a new RasterMapHandler object

@param context NB_Context
@param callback Request handler status update callback
@param parameters NB_RasterMapParameters object specifying the raster map to retrieve
@param handler On success, the newly created handler; NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_RasterMapHandler** handler);


/*! Destroy a previously created RasterMapHandler object

@param handler A NB_RasterMapHandler object created with NB_RasterMapHandlerCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapHandlerDestroy(NB_RasterMapHandler* handler);


/*! Start a network request to retrieve a raster map file

Initiate a network request that randers and retrieves a raster map file.  Only one request may be active at a given time

@param handler A NB_RasterMapHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapHandlerStartRequest(NB_RasterMapHandler* pThis, NB_RasterMapParameters* parameters);


/*! Cancel a previously started request

@param handler A NB_RasterMapHandler object
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapHandlerCancelRequest(NB_RasterMapHandler* handler);


/*! Check if a download request is in progress

@param handler A NB_RasterMapHandler object
@returns Non-zero if a request is in progress; zero otherwise
*/
NB_DEC nb_boolean NB_RasterMapHandlerIsRequestInProgress(NB_RasterMapHandler* handler);


/*! Retrieves a NB_RasterMapInformation object

@param handler A NB_RasterMapHandler object
@param map On success, a NB_RasterMapInformation object with the result of the last download; NULL otherwise.  An object returned from this function must be destroyed using NB_RasterMapInformationDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapHandlerGetMapInformation(NB_RasterMapHandler* handler, NB_RasterMapInformation** map);


/*! Retrieves a NB_SearchInformation object containing the place and traffic information from the last request

@param handler A NB_RouteHandler object
@param hasTrafficIncidents last request contains traffic incidents
@param route On success, a NB_SearchInformation object with the result of the last request; NULL otherwise.  An object returned from this function must be destroyed using NB_TrafficInformationDestroy().
@returns NB_Error
*/
NB_DEC NB_Error NB_RasterMapHandlerGetSearchInformation(NB_RasterMapHandler* handler, nb_boolean* hasTrafficIncidents, NB_SearchInformation** searchInformation);


/*! @} */

#endif
