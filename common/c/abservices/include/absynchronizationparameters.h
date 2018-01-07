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

    @file absynchronizationparameters.h
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

#ifndef ABSYNCHRONIZATIONPARAMETERS_H
#define ABSYNCHRONIZATIONPARAMETERS_H

#include "abexp.h"
#include "nbcontext.h"
#include "nbplace.h"
#include "nbextappcontent.h"
#include "absynchronizationtypes.h"


/*!
    @addtogroup absyncparameters
    @{
*/


/*! @struct AB_SynchronizationParameters
    Opaque data structure that defines the parameters of a synchronization operation
*/
typedef struct AB_SynchronizationParameters AB_SynchronizationParameters;

#define AB_DATABASE_NAME_LENGTH 64


/*! Synchronization operation types. */
typedef enum
{
    ASOT_All = -1,
	ASOT_None = 0,
	ASOT_AssignId = 1,
	ASOT_Delete,
	ASOT_Modify,
	ASOT_ModifyTimeStamp,
	ASOT_Add
} AB_SynchronizationOperationType;


/*! Synchronization errors. */
typedef enum
{
    ASE_None = 0,              /*!< No error */
    ASE_Full,                  /*!< Server full */
    ASE_InconsistentDatabase   /*!< Server/client mismatch */
} AB_SynchronizationError;


/*! Synchronization operation. */
typedef struct
{
    char databaseName[AB_DATABASE_NAME_LENGTH+1];          /*!< Database name (ex. "recents", "favorites", "placeinbox", "placeoutbox") */
    AB_SynchronizationOperationType	operationType;         /*!< Synchronization operation */

    AB_PlaceID					    localId;               /*!< Local ID previously assigned to this place */
    AB_PlaceID					    serverId;              /*!< Server ID assigned to this place, if < 0 this is a newly added item on client*/
    nb_unixTime					    modifiedTime;          /*!< Modification time for this place */
	
    NB_Place					    place;                 /*!< Place to operate on */
    nb_boolean					    placeValid;            /*!< Indicates if place is valid */

    AB_PlaceMessage			        message;               /*!< Place message to operate on */
    nb_boolean					    messageValid;          /*!< Indicates if place message is valid */

    AB_PlaceMessageFlags            placeMessageFlags;     /*< Place message flag to include in modify operation */

    NB_ExtAppContent                extAppContent;         /*< Ext app content */

} AB_SynchronizationOperation;


/*! Create parameters for a set of synchronization operations

@param synchronizationParameters On success, a newly created AB_SynchronizationParameters object; NULL otherwise.  A valid object must be destroyed using AB_SynchronizationParametersDestroy()
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationParametersCreate(NB_Context* context, AB_SynchronizationParameters** synchronizationParameters);


/*! Specify the generation and database ID from the last synchronization of the database.

@param parameters An AB_SynchronizationParameters object
@param databaseName Name of the database
@param generationId Generation ID of the database that was returned at last synchronization (zero if no sync has occurred)
@param databaseId Database ID  that was returned at last synchronization (zero if no sync has occurred)
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationParamatersSetDatabaseData(AB_SynchronizationParameters* parameters, const char* databaseName, uint32 generationId, uint32 databaseId);


/*! Add a local synchronize operation for the next synchronization request.

@param parameters An AB_SynchronizationParameters object
@param operation Local synchronization operation to add
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationParametersAddLocalOperation(AB_SynchronizationParameters* parameters, AB_SynchronizationOperation* operation);

/*! Get the total number of local operations for the next synchronization request of the database

@param parameters An AB_SynchronizationParameters object
@param databaseName Name of the database
@param opType Synchronization operation type
@param localOperationCount Pointer to buffer for the total number of the next synchronization request of the database
@returns NB_Error
 */
AB_DEC NB_Error AB_SynchronizationParametersGetLocalOperationCount(AB_SynchronizationParameters* parameters,
        const char* databaseName, AB_SynchronizationOperationType opType, uint32* localOperationCount);

/*! Destroy a previously created SynchronizationParameters object

@param parameters An AB_SynchronizationParameters object created with AB_SynchronizationParametersCreate()
@returns NB_Error
*/
AB_DEC NB_Error AB_SynchronizationParametersDestroy(AB_SynchronizationParameters* parameters);


/*! @} */

#endif
