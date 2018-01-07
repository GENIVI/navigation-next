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

    @file     abdatastoreparameters.h
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

#ifndef ABDATASTOREPARAMETERS_H
#define ABDATASTOREPARAMETERS_H


#include "abexp.h"


/*!
    @addtogroup abdatastoreparameters
    @{
*/


/*! @struct AB_DataStoreParameters
    Opaque data structure that defines the parameters of a data store operation
*/
typedef struct AB_DataStoreParameters AB_DataStoreParameters;


/*! Create parameters for a data store operations

@param context context instance
@param expireTimeMinutes minutes before data stored on server may be deleted
@param datastoreParameters On success, a newly created AB_DataStoreParameters object; NULL otherwise.  A valid object must be destroyed using AB_DataStoreParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreParametersCreate(NB_Context* context, /* uint32 expireTimeMinutes, */ AB_DataStoreParameters** datastoreParameters);


/*! Add a data store store operation

@param parameters An AB_DataStoreParameters object 
@param buffer data buffer to be stored
@param size size of data buffer to be stored
@param expireTimeMinutes minutes before data stored on server may be deleted
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreParametersAddStore(AB_DataStoreParameters* datastoreParameters, byte* buffer, nb_size size);


/*! Add a data store retrieve operation

@param parameters An AB_DataStoreParameters object 
@param retrieveId ID string of previously stored data store item to retrieve
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreParametersAddRetrieve(AB_DataStoreParameters* datastoreParameters, char* retrieveId);


/*! Add a data store delete operation

@param parameters An AB_DataStoreParameters object 
@param deleteId ID string of previously stored data store item to delete
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreParametersAddDelete(AB_DataStoreParameters* datastoreParameters, char* deleteId);


/*! Destroy a previously created DataStoreParameters object

@param parameters An AB_DataStoreParameters object created with AB_DataStoreParametersCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreParametersDestroy(AB_DataStoreParameters* parameters);


/*! @} */

#endif
