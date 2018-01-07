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

    @file absynchronizationinfo.h
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

#ifndef ABSYNCHRONIZATIONINFORMATION_H
#define ABSYNCHRONIZATIONINFORMATION_H

#include "abexp.h"
#include "absynchronizationparameters.h"


/*!
    @addtogroup absyncinformation
    @{
*/

/*! @struct AB_SynchronizationInformation
Information about the results of a geocode request
*/
typedef struct AB_SynchronizationInformation AB_SynchronizationInformation;


/*! Get server operation count

This function returns the count of the given synchronization operation resulting from the last
synchronization request.

@param information An AB_SynchronizationInformation object
@param dbName Name of the database
@param opType Synchronization operation type
@param operationCount Pointer to buffer for count of the the specified synchronization operation
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationInformationGetServerOperationCount(
    AB_SynchronizationInformation* information,
    const char* databaseName,
    AB_SynchronizationOperationType opType,
    uint32* operationCount
);


/*! Get server operation

This function retrieves the synchronization operation specified by index from the last requested
synchronization.

@param information An AB_SynchronizationInformation object
@param databaseName Name of the database
@param opType Synchronization operation type
@param index Index of operation to retrieve
@param operation Pointer to buffer for retrieved synchronization operation
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationInformationGetServerOperation(
    AB_SynchronizationInformation* information,
    const char* databaseName,
    AB_SynchronizationOperationType opType,
    uint32 index,
    AB_SynchronizationOperation* operation
);



/*! Get the generation and database ID from the last synchronization of the database.

@param parameters An AB_SynchronizationParameters object
@param databaseName Name of the database
@param generationId Generation ID of the database from the server
@param databaseId Database ID from the server
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationInformationGetDatabaseData(
    AB_SynchronizationInformation* information,
    const char* databaseName,
    uint32* generationId,
    uint32* databaseId
);


/*! Gets the total number of places within the specified database after synchronization.

This function returns the total number of places within the specified database after synchronization.

@param information An AB_SynchronizationInformation object
@param databaseName Name of the database
@param serverCount Pointer to buffer for total number of places within the specified database
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationInformationGetServerCount(
    AB_SynchronizationInformation* information,
    const char* databaseName,
    uint32* serverCount
);


/*! Get sync error code and value of last sync.

This function returns the error code and value returned from the last synchronization.

@param information An AB_SynchronizationInformation object
@param databaseName Database name 
@param errorValid Pointer to buffer to buffer indicating if an error occurred
@param errorCode Pointer to AB_SynchronizationError buffer, valid if errorValid is TRUE
@param errorValue Pointer to error value, valid if errorValid is TRUE
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationInformationGetServerError(
    AB_SynchronizationInformation* information,
    const char* databaseName,
    nb_boolean* errorValid,
    AB_SynchronizationError* errorCode,
    uint32* errorValue
);    

/*! Destroy a previously created SynchronizationInformation object

@param information An NB_GeocodeHandler object created with AB_SynchronizationHandlerGetInformation()
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationInformationDestroy(
    AB_SynchronizationInformation* information
);


#endif
