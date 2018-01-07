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

    @file     nbpersistentdata.c

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

#include "pal.h"
#include "nberror.h"
#include "nbexp.h"
#include "nbpersistentdataprotected.h"
#include "csdict.h"
#include "cslutil.h"
#include "vec.h"

/*! @{ */

// Constants .....................................................................................

static const byte persist_data_key[] = {  /*  same as qa key */
    152, 123, 70, 221, 139, 152, 40, 6,
    26, 154, 54, 227, 238, 207, 142, 52,
    71, 227, 138, 31, 12, 160, 113, 214,
    68, 215, 170, 210, 231, 210, 126, 193,
    183, 189, 56, 157, 146, 121, 3, 122,
    155, 60, 248, 220, 233, 106, 121, 141,
    242, 248, 29, 205, 98, 51, 66, 209,
    160, 175, 88, 83, 163, 64, 242, 121
};

#define PERSIST_DATA_KEY persist_data_key
#define PERSIST_DATA_KEY_SIZE sizeof(persist_data_key)

#define INITIAL_DICTIONARY_SIZE 8

static const uint32 PERSIST_DATA_MAGIC_NUMBER   = 0xFAFABCBC;
static const uint32 PERSIST_DATA_VERSION_NUMBER = 0x00000003;

/* These strings cannot be changed. Because they are keys for transform between the persistent
   data and the TPS element. And they are strange to not duplicate with names of TPS protocol.
*/
#define MAX_PERSIST_TPS_ELEMENT_KEY_LENGTH 128

static const char PERSIST_TPS_ELEMENT_NAME[]                  = "=TPS+ELEMENT+NAME=";
static const char PERSIST_TPS_ELEMENT_CHILD_FORMAT[]          = "=TPS+ELEMENT+CHILD+%d=";
static const char PERSIST_TPS_ELEMENT_CHILD_COMPARED_STRING[] = "=TPS+ELEMENT+CHILD+";


// Local Types ...................................................................................

struct NB_PersistentData
{
    struct CSL_Dictionary*              dataDictionary;
    nb_boolean                          dataDirty;
    NB_PersistentDataChangedCallback    dataChangeNotifyCallback;
};


// Private functions .............................................................................

static NB_Error DeserializeToDataDictionary(struct CSL_Dictionary* dataDictionary, const uint8* data, nb_size dataSize);
static NB_Error SerializeFromDataDictionary(struct CSL_Dictionary* dataDictionary, const uint8** data, nb_size* dataSize);

static int MasterClearDataDictionaryIterationFunction(void* userData, const char* itemName, const char *itemData, size_t itemDataLength);

static NB_Error GetStringFromPersistentData(NB_PersistentData* persistentData, const char* key, char** data);
static NB_Error SetStringToPersistentData(NB_PersistentData* persistentData, const char* key, const char* data);


// Public functions ..............................................................................

NB_DEF NB_Error
NB_PersistentDataCreate(const uint8* data, nb_size dataSize, NB_PersistentDataChangedCallback* callback, NB_PersistentData** persistentData)
{
    NB_Error err = NE_OK;
    NB_PersistentData*  pThis = 0;
    struct CSL_Dictionary* dataDictionary = 0;

    if (!persistentData)
    {
        return NE_INVAL;
    }
    *persistentData = 0;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    // create dictionary for data storage
    dataDictionary = CSL_DictionaryAlloc(INITIAL_DICTIONARY_SIZE);
    if (!dataDictionary)
    {
        err = NE_NOMEM;
    }
    pThis->dataDictionary = dataDictionary;

    // if serialized data provided, deserialize it to dictionary
    if (!err && data && dataSize)
    {
        err = DeserializeToDataDictionary(dataDictionary, data, dataSize);
    }

    // if error, destroy and return
    if (err)
    {
        if (dataDictionary)
        {
            (void)CSL_DictionaryDealloc(dataDictionary);
        }
        nsl_free(pThis);
        return err;
    }

    if (callback)
    {
        pThis->dataChangeNotifyCallback = *callback;
    }
    pThis->dataDirty = FALSE;

    *persistentData = pThis;

    return NE_OK;
}


NB_DEF NB_Error
NB_PersistentDataSerialize(NB_PersistentData* persistentData, const uint8** data, nb_size* dataSize)
{
    if (!persistentData || !data || !dataSize)
    {
        return NE_INVAL;
    }

    return SerializeFromDataDictionary(persistentData->dataDictionary, data, dataSize);
}


NB_DEF NB_Error
NB_PersistentDataMasterClear(NB_PersistentData* persistentData)
{
    int result = 0;

    if (!persistentData)
    {
        return NE_INVAL;
    }

    // iterate through dictionary, removing items not marked to survive master clears (non-zero value in first byte of item value)
    result = CSL_DictionaryIteration(persistentData->dataDictionary, MasterClearDataDictionaryIterationFunction, persistentData);

    return result ? NE_OK : NE_INVAL;
}


// Protected functions ...........................................................................

NB_DEF NB_Error
NB_PersistentDataGet(NB_PersistentData* persistentData, const char* name, uint8** data, nb_size* dataSize)
{
    char* itemData = 0;
    size_t itemDataSize = 0;

    if (!persistentData || !name || !data || !dataSize)
    {
        return NE_INVAL;
    }

    *data = 0;
    *dataSize = 0;

    itemData = CSL_DictionaryGet(persistentData->dataDictionary, name, &itemDataSize);
    if (!itemData)
    {
        return NE_NOENT;
    }

    // compensate for permanent flag in first byte
    itemDataSize--;
    *dataSize = itemDataSize;

    if (itemDataSize)
    {
        *data = nsl_malloc(itemDataSize);
        if (*data)
        {
            // copy data after permanent flag byte to return
            nsl_memcpy(*data, itemData + 1, itemDataSize);
        }
        else
        {
            return NE_NOMEM;
        }
    }
    else
    {
        *data = NULL;
    }

    return NE_OK;
}


NB_DEF NB_Error
NB_PersistentDataSet(NB_PersistentData* persistentData, const char* name, const uint8* data, nb_size dataSize, nb_boolean permanent, nb_boolean notifyApplication)
{
    NB_Error err = NE_OK;

    char* itemData = 0;
    nb_size itemDataSize = 0;

    if (!persistentData || !name || !data)
    {
        return NE_INVAL;
    }

    // save data with permanent flag in leading byte
    itemDataSize = dataSize + 1;
    itemData = nsl_malloc(itemDataSize);
    if (!itemData)
    {
        return NE_NOMEM;
    }

    *itemData = permanent;
    nsl_memcpy(itemData + 1, data, dataSize);

    if (!CSL_DictionarySet(persistentData->dataDictionary, name, itemData, itemDataSize))
    {
        err = NE_NOMEM;
    }
    nsl_free(itemData);

    if (!err)
    {
        persistentData->dataDirty = TRUE;

        // if notify requested, and callback supplied, invoke callback
        if (notifyApplication &&  persistentData->dataChangeNotifyCallback.callback)
        {
            if ((persistentData->dataChangeNotifyCallback.callback)(persistentData, persistentData->dataChangeNotifyCallback.callbackData))
            {
                persistentData->dataDirty = FALSE;
            }
        }
    }

    return err;
}


NB_DEF NB_Error
NB_PersistentDataDestroy(NB_PersistentData* persistentData)
{
    if (!persistentData)
    {
        return NE_INVAL;
    }

    // if data currently dirty, and callback supplied, invoke callback before destroying
    if (persistentData->dataDirty && persistentData->dataChangeNotifyCallback.callback)
    {
        (void)(persistentData->dataChangeNotifyCallback.callback)(persistentData, persistentData->dataChangeNotifyCallback.callbackData);
    }

    if (persistentData->dataDictionary)
    {
        (void)CSL_DictionaryDealloc(persistentData->dataDictionary);
    }

    nsl_free(persistentData);

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_PersistentDataNext(NB_PersistentData* persistentData,
                      int* iterator,
                      const char** key,
                      uint8** data,
                      nb_size* dataSize)
{
    int result = 0;
    char* itemData = 0;
    size_t itemDataSize = 0;

    if ((!persistentData) ||
        (!(persistentData->dataDictionary)) ||
        (!iterator) ||
        (!key) ||
        (!data) ||
        (!dataSize))
    {
        return NE_INVAL;
    }

    // Get the next element.
    result = CSL_DictionaryNext(persistentData->dataDictionary,
                                iterator,
                                key,
                                &itemData,
                                &itemDataSize);
    if (!result || !itemData)
    {
        return NE_NOENT;
    }

    --itemDataSize;
    *dataSize = itemDataSize;
    // Compensate for permanent flag in first byte.
    if (itemDataSize > 0)
    {
        *data = nsl_malloc(itemDataSize + 1);
        if (*data)
        {
            // Copy data after permanent flag byte to return.
            nsl_memset(*data, 0, itemDataSize + 1);
            nsl_memcpy(*data, itemData + 1, itemDataSize);
        }
        else
        {
            return NE_NOMEM;
        }
    }
    else
    {
        *data = NULL;
    }

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_PersistentDataCreateByTpsElement(tpselt tpsElement,
                                    NB_PersistentData** persistentData)
{
    NB_Error error = NE_OK;
    int i = 0;
    int childIterator = 0;
    const char* name = NULL;
    tpselt childTpsElement = NULL;
    CSL_Vector* attributes = NULL;
    NB_PersistentData* newPersistentData = NULL;

    if ((!persistentData) || (!tpsElement))
    {
        return NE_INVAL;
    }

    // Create a persistent data instance.
    error = NB_PersistentDataCreate(NULL, 0, NULL, &newPersistentData);
    if (error != NE_OK)
    {
        return error;
    }

    // Set the name of the TPS element to the persistent data.
    name = te_getname(tpsElement);
    if (!name)
    {
        NB_PersistentDataDestroy(newPersistentData);
        newPersistentData = NULL;
        return NE_INVAL;
    }

    error = SetStringToPersistentData(newPersistentData, PERSIST_TPS_ELEMENT_NAME, name);
    if (error != NE_OK)
    {
        NB_PersistentDataDestroy(newPersistentData);
        newPersistentData = NULL;
        return error;
    }

    // Set the attributes to the persistent data.
    attributes = te_getattrs(tpsElement);
    if (attributes)
    {
        int length = CSL_VectorGetLength(attributes);
        for (i = 0; i < length; ++i)
        {
            char*  attributeData     = NULL;
            size_t attributeDataSize = 0;

            const char** attributeName = (const char**) CSL_VectorGetPointer(attributes, i);

            if ((!attributeName) ||
                (!(*attributeName)) ||
                (nsl_strlen(*attributeName) == 0) ||
                (te_getattr(tpsElement, *attributeName, &attributeData, &attributeDataSize) == 0))
            {
                error = NE_UNEXPECTED;
            }

            // Some of attribute is optional or deprecated, and is meaning full only when it
            // returned from server. They are set to NULL by default. So if they are NULL,
            // forget it because it is not used at all.
            if (!attributeData) // Skip optional attributes.
            {
                continue;
            }

            error = error ? error :  NB_PersistentDataSet(newPersistentData,
                                                          *attributeName,
                                                          (const uint8*)attributeData,
                                                          attributeDataSize,
                                                          FALSE,
                                                          FALSE);
            if (error != NE_OK) // Error occurred while dumping attributes, stop and return.
            {
                break;
            }
        }

        CSL_VectorDealloc(attributes);
        attributes = NULL;

        if (error != NE_OK)
        {
            NB_PersistentDataDestroy(newPersistentData);
            newPersistentData = NULL;
            return error;
        }
    }

    // Set the children to the persistent data.
    childIterator = 0;
    for (i = 0; (childTpsElement = te_nextchild(tpsElement, &childIterator)) != NULL; ++i)
    {
        uint8*             childSerializedData     = NULL;
        nb_size            childSerializedDataSize = 0;
        NB_PersistentData* childPersistent         = NULL;
        char childPersistentKey[MAX_PERSIST_TPS_ELEMENT_KEY_LENGTH] = {0};

        // Call this function recursively to generate the data for the child TPS element.
        error = NB_PersistentDataCreateByTpsElement(childTpsElement, &childPersistent);
        if (error != NE_OK)
        {
            break;
        }

        error = NB_PersistentDataSerialize(childPersistent,
                                           (const uint8**) (&childSerializedData),
                                           &childSerializedDataSize);
        NB_PersistentDataDestroy(childPersistent);
        if (error != NE_OK)
        {
            break;
        }

        if ((!childSerializedData) || (childSerializedDataSize == 0))
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Format a string as the persistent key.
        nsl_memset(childPersistentKey, 0, MAX_PERSIST_TPS_ELEMENT_KEY_LENGTH);
        nsl_sprintf(childPersistentKey, PERSIST_TPS_ELEMENT_CHILD_FORMAT, i);

        error = NB_PersistentDataSet(newPersistentData,
                                     childPersistentKey,
                                     childSerializedData,
                                     childSerializedDataSize,
                                     FALSE,
                                     FALSE);
        nsl_free(childSerializedData);
        childSerializedData     = NULL;
        childSerializedDataSize = 0;
        if (error != NE_OK)
        {
            break;
        }
    }

    if (error != NE_OK)
    {
        NB_PersistentDataDestroy(newPersistentData);
        newPersistentData = NULL;
        return error;
    }

    *persistentData = newPersistentData;
    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_PersistentDataGetToTpsElement(NB_PersistentData* persistentData,
                                 tpselt* tpsElement)
{
    NB_Error error = NE_OK;
    int i = 0;
    char* name = NULL;
    tpselt newTpsElement = NULL;

    if ((!persistentData) || (!tpsElement))
    {
        return NE_INVAL;
    }

    // Get the name of the TPS element from the persistent data.
    error = GetStringFromPersistentData(persistentData,
                                        PERSIST_TPS_ELEMENT_NAME,
                                        &name);
    if (error != NE_OK)
    {
        return error;
    }

    if (!name)
    {
        return NE_UNEXPECTED;
    }

    // Create a TPS element with the name.
    newTpsElement = te_new(name);
    nsl_free(name);
    name = NULL;
    if (!newTpsElement)
    {
        return NE_NOMEM;
    }

    // Get the children from the persistent data.
    for (i = 0; error == NE_OK; ++i)
    {
        uint8* childSerializedData = NULL;
        nb_size childSerializedDataSize = 0;
        tpselt childTpsElement = NULL;
        NB_PersistentData* childPersistent = NULL;
        char childPersistentKey[MAX_PERSIST_TPS_ELEMENT_KEY_LENGTH] = {0};

        // Format a string as the persistent key.
        nsl_memset(childPersistentKey, 0, MAX_PERSIST_TPS_ELEMENT_KEY_LENGTH);
        nsl_sprintf(childPersistentKey, PERSIST_TPS_ELEMENT_CHILD_FORMAT, i);

        // Get the serialized data of the child TPS element.
        error = NB_PersistentDataGet(persistentData,
                                     childPersistentKey,
                                     &childSerializedData,
                                     &childSerializedDataSize);
        if (error != NE_OK)
        {
            break;
        }

        if ((!childSerializedData) || (childSerializedDataSize == 0))
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Create a persistent data instance by the serialized data.
        error = NB_PersistentDataCreate(childSerializedData,
                                        childSerializedDataSize,
                                        NULL,
                                        &childPersistent);
        nsl_free(childSerializedData);
        childSerializedDataSize = 0;
        if (error != NE_OK)
        {
            break;
        }

        // Call this function recursively to generate a child TPS element.
        error = NB_PersistentDataGetToTpsElement(childPersistent, &childTpsElement);
        NB_PersistentDataDestroy(childPersistent);
        childPersistent = NULL;
        if (error != NE_OK)
        {
            break;
        }

        if (!childTpsElement)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Attach the child TPS element as a child.
        if (te_attach(newTpsElement, childTpsElement) == 0)
        {
            te_dealloc(childTpsElement);
            childTpsElement = NULL;
            error = NE_UNEXPECTED;
            break;
        }
    }

    // Ignore the error NE_NOENT.
    error = (error == NE_NOENT) ? NE_OK : error;

    if (error != NE_OK)
    {
        te_dealloc(newTpsElement);
        newTpsElement =NULL;
        return error;
    }

    /* @todo: Both the attributes and children can be set in one loop by the function
              'NB_PersistentDataNext'. And I use this function to set the attributes.
              But I get and attach the children in the above loop. Because I want to
              keep the order of the children by getting the child by the formatted key.
              See the above loop for details. If the order is useless the code should
              be fixed to one loop.
    */

    // Get the attributes from the persistent data.
    i = 0;
    while (error == NE_OK)
    {
        const char* attributeKey      = NULL;
        uint8*      attributeData     = NULL;
        nb_size     attributeDataSize = 0;

        // Get the next persistent element.
        error = NB_PersistentDataNext(persistentData,
                                      &i,
                                      &attributeKey,
                                      &attributeData,
                                      &attributeDataSize);
        if (error != NE_OK)
        {
            break;
        }

        // Treat as error when key is empty, or data is empty, but dataSize is not zero.
        if ((!attributeKey) || ((!attributeData) && attributeDataSize))
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Check if it is an attribute (not the name of the TPS element or an child element).
        if ((nsl_strcmp(attributeKey, PERSIST_TPS_ELEMENT_NAME) == 0) ||
            (nsl_strcmp(attributeKey, PERSIST_TPS_ELEMENT_CHILD_COMPARED_STRING) == 0))
        {
            nsl_free(attributeData);
            attributeData = NULL;
            attributeDataSize = 0;
            continue;
        }

        // Set the attribute to the TPS element.
        /*
          :WARNING:
           te_setattr() expects the dataSize to be exactly the size of data, excluding any
           trailing NULLs.

           Please pay attention when modifying this code, or it may cause data corruption.
         */
        if (te_setattr(newTpsElement, attributeKey, (const char*)attributeData, attributeDataSize) == 0)
        {
            nsl_free(attributeData);
            attributeData = NULL;
            attributeDataSize = 0;
            error = NE_UNEXPECTED;
            break;
        }

        if (attributeData)
        {
            nsl_free(attributeData);
            attributeData = NULL;
        }
        attributeDataSize = 0;
    }
    // Ignore the error NE_NOENT.
    error = (error == NE_NOENT) ? NE_OK : error;

    if (error != NE_OK)
    {
        te_dealloc(newTpsElement);
        newTpsElement =NULL;
        return error;
    }

    *tpsElement = newTpsElement;
    return NE_OK;
}


// Private functions .............................................................................

#define AdjustToMultipleOfFour(n) ((n) + ((4 - ((n) % 4)) % 4))

// NOTE: Below is the serialization format as of version 0x00000002.
//       All items start on 4-byte boundary.
//
//      magic number        (uint32)    0xFAFABCBC
//      version number      (uint32)    0x00000003
//      totalDataSize       (nb_size)
//      totalDataItemCount  (uint32)
//      for each dataItem:
//          itemName        (char[], null-terminated)
//          dataSize        (nb_size)
//          data            (uint8[dataSize]) -- non-zero first byte indicates item survives clear

NB_Error
SerializeFromDataDictionary(struct CSL_Dictionary* dataDictionary, const uint8** data, nb_size* dataSize)
{
    nb_size totalDataSize = 0;
    uint32 totalDataItemCount = 0;

    int index = 0;
    const char* itemName = 0;
    uint32 itemDataSize = 0; // we need to make sure itemDataSize is 32bits long so it can be parsed easier.
    char* itemData = 0;

    uint8* serializedData = 0;
    uint8* p = 0;
    size_t tmpSize = 0;

    if (!dataDictionary || !data || !dataSize)
    {
        return NE_INVAL;
    }

    *data = 0;
    *dataSize = 0;

    // determine total size needed for serialization, starting with the header
    totalDataSize =
        AdjustToMultipleOfFour(sizeof(uint32))  +   // magic number
        AdjustToMultipleOfFour(sizeof(uint32))  +   // version number
        AdjustToMultipleOfFour(sizeof(nb_size)) +   // totalDataSize
        AdjustToMultipleOfFour(sizeof(uint32));     // dataItemCount

    // data items:
    index = 0;
    while (CSL_DictionaryNext(dataDictionary, &index, &itemName, &itemData, &tmpSize))
    {
        itemDataSize   = (uint32)tmpSize;                                   // 32 bits should be large enough.
        totalDataSize += AdjustToMultipleOfFour(nsl_strlen(itemName) + 1);  // size of itemName and null-terminator, increased for 4 byte boundary
        totalDataSize += AdjustToMultipleOfFour(sizeof(itemDataSize));      // size of itemDataSize
        totalDataSize += AdjustToMultipleOfFour(itemDataSize);              // size of itemData, increased for 4 byte boundary
        totalDataItemCount++;
    }

    // allocate buffer for serialization
    serializedData = nsl_malloc(totalDataSize);

    if (!serializedData)
    {
        return NE_NOMEM;
    }
    nsl_memset(serializedData, 0, totalDataSize);

    // serialize header
    p = serializedData;

    *(uint32*)p = PERSIST_DATA_MAGIC_NUMBER;
    p += AdjustToMultipleOfFour(sizeof(PERSIST_DATA_MAGIC_NUMBER));

    *(uint32*)p = PERSIST_DATA_VERSION_NUMBER;
    p += AdjustToMultipleOfFour(sizeof(PERSIST_DATA_VERSION_NUMBER));

    *(nb_size*)p = totalDataSize;
    p += AdjustToMultipleOfFour(sizeof(totalDataSize));

    *(uint32*)p = totalDataItemCount;
    p += AdjustToMultipleOfFour(sizeof(totalDataItemCount));

    // serialize data items
    index = 0;
    while (CSL_DictionaryNext(dataDictionary, &index, &itemName, &itemData, &tmpSize))
    {
        itemDataSize = (uint32)tmpSize;
        nsl_strcpy((char*)p, itemName);
        p += AdjustToMultipleOfFour(nsl_strlen(itemName) + 1);

        *(nb_size*)p = itemDataSize;
        p += AdjustToMultipleOfFour(sizeof(itemDataSize));

        nsl_memcpy(p, itemData, itemDataSize);
        p += AdjustToMultipleOfFour(itemDataSize);
    }

    obfuscate_buffer(serializedData, (uint32)totalDataSize, PERSIST_DATA_KEY, PERSIST_DATA_KEY_SIZE);

    *data = serializedData;
    *dataSize = totalDataSize;

    return NE_OK;
}

NB_Error
DeserializeToDataDictionary(struct CSL_Dictionary* dataDictionary, const uint8* data, nb_size dataSize)
{
    NB_Error err = NE_BADDATA;
    uint8* serializedData = 0;
    uint8* p = 0;

    uint32 magicNumber = 0;
    uint32 versionNumber = 0;
    nb_size totalDataSize = 0;
    uint32 totalDataItemCount = 0;

    uint32 n = 0;

    if (!dataDictionary || !data || !dataSize)
    {
        return NE_INVAL;
    }

    // copy and deobfuscate data buffer provided
    serializedData = nsl_malloc(dataSize);
    if (!serializedData)
    {
        return NE_NOMEM;
    }
    nsl_memcpy(serializedData, data, dataSize);
    obfuscate_buffer(serializedData, (uint32)dataSize, PERSIST_DATA_KEY, PERSIST_DATA_KEY_SIZE);

    // extract header information
    p = serializedData;

    magicNumber = *(uint32*)p;
    p += AdjustToMultipleOfFour(sizeof(magicNumber));
    if (magicNumber != PERSIST_DATA_MAGIC_NUMBER)
    {
        goto error;
    }

    versionNumber = *(uint32*)p;
    p += AdjustToMultipleOfFour(sizeof(versionNumber));
    if (versionNumber != PERSIST_DATA_VERSION_NUMBER)
    {
        // ignore internally-generated version 1
        if (versionNumber == 1)
        {
            err = NE_OK;
        }
        goto error;
    }

    totalDataSize = *(nb_size*)p;
    p += AdjustToMultipleOfFour(sizeof(totalDataSize));
    if (totalDataSize != dataSize)
    {
        goto error;
    }

    totalDataItemCount = *(uint32*)p;
    p += AdjustToMultipleOfFour(sizeof(totalDataItemCount));

    // extract and store data items
    for (n = 0; n < totalDataItemCount; n++)
    {
        char* itemName = 0;
        uint32 itemDataSize = 0;
        char* itemData = 0;

        itemName = (char*)p;
        p += AdjustToMultipleOfFour(nsl_strlen(itemName) + 1);

        itemDataSize = *(uint32*)p;
        p += AdjustToMultipleOfFour(sizeof(itemDataSize));

        itemData = (char*)p;
        p += AdjustToMultipleOfFour(itemDataSize);

        if (!CSL_DictionarySet(dataDictionary, itemName, itemData, itemDataSize))
        {
            err = NE_NOMEM;
            goto error;
        }
    }

    err = NE_OK;

error:
    nsl_free(serializedData);
    return err;
}

int
MasterClearDataDictionaryIterationFunction(void* userData, const char* itemName, const char *itemData, size_t itemDataLength)
{
    // non-zero in first byte of data item indicates whether item should survive a master clear
    // returning zero from this function will delete this item; non-zero will not
    return itemData && itemDataLength ? *itemData : 1;
}

/*! Get a string from the persistent data instance

    This function returns a pointer to a string if success. This pointer
    must be freed with 'nsl_free' when it is useless.

    @return NE_OK if success
*/
NB_Error
GetStringFromPersistentData(NB_PersistentData* persistentData,  /*!< A persistent data instance */
                            const char* key,                    /*!< A persistent key */
                            char** data                         /*!< On success, receives a pointer to a string. The
                                                                     pointer must be freed with 'nsl_free' */
                            )
{
    NB_Error error = NE_OK;
    char* tempData = NULL;
    nb_size dataSize = 0;

    if (!persistentData || !key || !data)
    {
        return NE_INVAL;
    }

    /*  Get the data to a 'char*'. Because Storing length of string plus 1. This is same
        as below. See the serialized functions for details.
    */
    error = NB_PersistentDataGet(persistentData,
                                 key,
                                 (uint8**) (&tempData),
                                 &dataSize);
    if (error == NE_OK)
    {
        *data = tempData;
        tempData = NULL;
    }
    else if (error == NE_NOENT)
    {
        // Returns 'NE_OK' to caller if can not find a string by key.
        error = NE_OK;
        *data = NULL;
    }

    return error;
}

/*! Set a string to the persistent data instance

    @return NE_OK if success
*/
NB_Error
SetStringToPersistentData(NB_PersistentData* persistentData,    /*!< A persistent data instance */
                          const char* key,                      /*!< A persistent key */
                          const char* data                      /*!< A persistent data of type string */
                          )
{
    nb_size dataSize = 0;

    if (!persistentData || !key)
    {
        return NE_INVAL;
    }

    // There is no need to add if parameter 'data' is NULL.
    if (!data)
    {
        // Just returns 'NE_OK' to caller.
        return NE_OK;
    }

    /* Add the data even if the parameter 'data' is "". Because we should save the key
       with empty data.
    */
    dataSize = (nb_size) (nsl_strlen(data) + 1);

    /*  Storing length of string plus 1 is simple to deserialize. It could be directly
        converted to 'char*'. This is same as below.
    */
    return NB_PersistentDataSet(persistentData,
                                key,
                                (const uint8*) data,
                                dataSize,
                                FALSE,
                                FALSE);
}

/*! @} */
