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

    @file     nbpersistentdata.h

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

#ifndef NBPERSISTENTDATA_H
#define NBPERSISTENTDATA_H

#include "pal.h"
#include "paltypes.h"
#include "nberror.h"
#include "nbexp.h"

/*!
    @addtogroup nbpersistentdata
    @{
*/

/*! @struct NB_PersistentData
An object that allows an application to persist NAVBuilder implementation data
*/
typedef struct NB_PersistentData NB_PersistentData;


/*! Data changed callback function

This callback function will be invoked whenever the data changes.  An application
must call NB_PersistentDataSerialize to get the data and it then must save the
data to persistent storage.  On subsequent application sessions, the persisted data
must be used to create a new instance of the object

@param persistentData The object that has changed
@param userData The opaque data used to create the callback function
@return Non-zero if the data was saved successfully; zero otherwise
*/
typedef nb_boolean (*NB_PersistentDataChangedCallbackFunction)(NB_PersistentData* persistentData, void* userData);


/*! @struct NB_PersistentDataChangedCallback
Defines the data changed callback and opaque user data to be provided to the function
*/
typedef struct
{
    NB_PersistentDataChangedCallbackFunction    callback;
    void*                                       callbackData;
} NB_PersistentDataChangedCallback;


/*! Create persistent data object

@param data A data buffer containing the results of a previous NB_PersistentDataSerialize call; NULL if no data has been persisted
@param dataSize The size of the data buffer
@param callback Callback to receive notification that data has changed and must be saved
@param persistentData On success, a newly created NB_PersistentData object; NULL otherwise.  A valid object must be passed to NB_ContextCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_PersistentDataCreate(const uint8* data, nb_size dataSize, NB_PersistentDataChangedCallback* callback, NB_PersistentData** persistentData);


/*! Destroy a persistent data object

  Destroys a persistent data object.  If there are unsaved changes (NB_PersistentDataSet was last called with notifyApplication = FALSE, or the
  application returned FALSE from the NB_PersistentDataChangedCallback, the callback will be fired before the object is destroyed

  @param persistentData The persistent data object to be destroyed
  @return NB_Error
*/
NB_DEC NB_Error NB_PersistentDataDestroy(NB_PersistentData* persistentData);


/*! Serialize the persistent data object

@param persistentData The object to be serialized
@param data On success, receives a pointer to a data buffer to be saved in persistent storage.  Once saved, the pointer must be freed with nsl_free
@param dataSize On success, receives the size of the data buffer
@return NB_Error
*/
NB_DEC NB_Error NB_PersistentDataSerialize(NB_PersistentData* persistentData, const uint8** data, nb_size* dataSize);


/*! Master clear all non-permanent data items in the persistent data object

@param persistentData The object to be cleared
@return NB_Error
*/
NB_DEC NB_Error NB_PersistentDataMasterClear(NB_PersistentData* persistentData);


/*! Get a data item

  @param persistentData The persistent data object
  @param name The name of the data item to get
  @param data On success, receives a pointer to a data buffer containing the data.  A valid object must be freed with nsl_free
  @param dataSize On success, receives the size of the data buffer
  @return NB_Error
*/
NB_DEC NB_Error NB_PersistentDataGet(NB_PersistentData* persistentData, const char* name, uint8** data, nb_size* dataSize);


/*! Set a data item

  @param persistentData The persistent data object
  @param name The name of the data item to set
  @param data A data buffer containing the data
  @param dataSize The size of the data buffer
  @param permanent If non-zero, indicates this data item should survive master clears
  @param notifyApplication If non-zero, fire a notification callback that the data store needs to be saved by the application; zero otherwise
  @return NB_Error
*/
NB_DEC NB_Error NB_PersistentDataSet(NB_PersistentData* persistentData, const char* name, const uint8* data, nb_size dataSize, nb_boolean permanent, nb_boolean notifyApplication);



/*! @} */

#endif
