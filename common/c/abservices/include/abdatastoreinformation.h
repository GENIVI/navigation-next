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

    @file     abdatastoreinformation.h
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

#ifndef ABDATASTOREINFO_H
#define ABDATASTOREINFO_H

#include "abexp.h"
#include "nberror.h"


/*!
    @addtogroup abdatastoreinformation
    @{
*/


/*! Data Store Operations */
typedef enum
{
    ADSO_Undefined = 0,
    ADSO_Store,
    ADSO_Retrieve,
    ADSO_Delete,
} AB_DataStoreOperation;


/*! @struct AB_DataStoreInformation
Information about the results of a data store request
*/
typedef struct AB_DataStoreInformation AB_DataStoreInformation;


/*! Get a data store operation count

This function returns a copy of the data store operation results: for store & delete, the ID of the data store is returned;
for retrieve, the data store content is returned.  The caller is responsible for freeing the returned data item.

@param information An AB_DataStoreInformation object
@param operationCount Pointer to buffer for returned operation result count
@param completionCode Pointer to buffer for returned completion code
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreInformationGetResultCount(
    AB_DataStoreInformation* information,
    uint32* operationCount,
    uint32* completionCode
);


/*! Get a data store operation result by index

This function returns a copy of the data store operation result by index: for store & delete, the ID of the data store is returned;
for retrieve, the data store content is returned.  The caller is responsible for freeing the returned data item.

@param information An AB_DataStoreInformation object
@param index Index of result
@param data Copy of data returned (data store ID if store or delete operation, the data retrieved for retrieve operation)
@param dataSize Size of data item
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreInformationGetResult(
    AB_DataStoreInformation* information,
    uint32 index,
    byte** data,
    nb_size* dataSize
);


/*! Destroy a previously created DataStoreInformation object

@param information An AB_DataStoreInformation object created with AB_DataStoreHandlerGetInformation()
@returns NB_Error
*/
AB_DEC NB_Error AB_DataStoreInformationDestroy(AB_DataStoreInformation* information);


#endif
