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

    @file     nbstorerequestinformation.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef NBSTOREREQUESTINFORMATION_H
#define NBSTOREREQUESTINFORMATION_H

#include "nbexp.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbmobilecoupons.h"

/*!
    @addtogroup nbstorerequestinformation
    @{
*/


/*! @struct NB_StoreRequestInformation
Information about the results of store coupon request
*/
typedef struct NB_StoreRequestInformation NB_StoreRequestInformation;


/*! Get stores

    The store data is allocated by this function and NB_StoreRequestInformationFreeStoreData()
    has to be called to free the data.

@param information A NB_StoreRequestInformation object
@param index Result index
@param store Store

@return NB_Error
*/
NB_DEC NB_Error NB_StoreRequestInformationGetStore(NB_StoreRequestInformation* information, int32 index, NB_Store** store);


/*! Free store object allocated in NB_StoreRequestInformationGetStore().

@param store A store object
@return NB_Error
*/
NB_DEC NB_Error NB_StoreRequestInformationFreeStoreData(NB_Store* store);


/*! Get number of results in the server response

@param information A NB_StoreRequestInformation object
@param count Number of results in the server response

@return NB_Error
*/
NB_DEC NB_Error NB_StoreRequestInformationGetResultCount(NB_StoreRequestInformation* information, int32* count);


/*! This function returns whether or not more results are available on the server

@param information A NB_StoreRequestInformation object
@param moreResults TRUE if more results are available on the server, FALSE otherwise

@return NB_Error
*/
NB_DEC NB_Error NB_StoreRequestInformationHasMoreResults(NB_StoreRequestInformation* information, nb_boolean* moreResults);


/*! Destroy a previously created StoreRequestInformation object

@param information A NB_StoreRequestHandler object created with NB_StoreRequestHandlerGetInformation()
@return NB_Error
*/
NB_DEC NB_Error NB_StoreRequestInformationDestroy(NB_StoreRequestInformation* information);


/*! @} */

#endif
