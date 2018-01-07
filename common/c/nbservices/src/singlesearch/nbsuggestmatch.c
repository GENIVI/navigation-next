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

    @file     nbsuggestmatch.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbsuggestmatch.h"
#include "nbcontextprotected.h"
#include "nbsuggestmatchprivate.h"
#include "nbpersistentdataprotected.h"
#include "data_suggest_match.h"
#include "data_pair.h"

/*! @{ */

// Constants ....................................................................................

// Maximum length of persistent key
#define PERSISTENT_DATA_MAX_LENGTH          16

// Suggest match strings
#define SUGGEST_MATCH_POI                   "poi"
#define SUGGEST_MATCH_ADDRESS               "address"
#define SUGGEST_MATCH_AIRPORT               "airport"
#define SUGGEST_MATCH_GAS                   "gas"
#define SUGGEST_MATCH_CATEGORY              "category"

// These keys are used to serialize with data.
#define PERSISTENT_KEY_LINE1                "line1"
#define PERSISTENT_KEY_LINE2                "line2"
#define PERSISTENT_KEY_LINE3                "line3"
#define PERSISTENT_KEY_MATCH_TYPE           "match-type"
#define PERSISTENT_KEY_SEARCH_FILTER        "search-filter"
#define PERSISTENT_KEY_KEY                  "key"
#define PERSISTENT_KEY_RESULT_STYLE         "result-style"
#define PERSISTENT_KEY_VECTOR_PAIR          "vector-pair"
#define PERSISTENT_KEY_TRANSIENT            "transient"
#define PERSISTENT_KEY_VALUE                "value"
#define PERSISTENT_KEY_SECTION              "section"
#define PERSISTENT_KEY_TYPE                 "type"
#define PERSISTENT_KEY_ICON_ID              "icon-id"

// Boolean value of persistent data
#define PERSISTENT_DATA_BOOLEAN_TRUE        "1"
#define PERSISTENT_DATA_BOOLEAN_FALSE       "0"

// For check sum, used by crc32.
#define CRC_VALUE 0xAEF0AEF0

// Serialization data version number
#define SERIALIZATION_DATA_VERSION_NUM_SIZE sizeof(int)


// Local types ..................................................................................

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

/*! Definition of 'NB_SuggestMatch' */
struct NB_SuggestMatch
{
    NB_Context* context;                /*!< Pointer to current context */
    data_suggest_match dataSuggestMatch;/*!< 'data_suggest_match' object */
};


// Declaration of local functions ...............................................................

static NB_Error CreateSuggestMatch(NB_Context* context, NB_SuggestMatch** suggestMatch);

static NB_Error SerializeFromSuggestMatch(NB_SuggestMatch* suggestMatch, uint8** data, nb_size* dataSize);
static NB_Error SerializeFromSuggestMatchLite(NB_SuggestMatch* suggestMatch, uint8** data, nb_size* dataSize);
static NB_Error DeserializeToSuggestMatch(NB_SuggestMatch* suggestMatch, const uint8* data, nb_size dataSize);
static NB_Error DeserializeToSuggestMatchLite(NB_SuggestMatch* suggestMatch, const uint8* data, nb_size dataSize);
static nb_size SerializeDataStringToDataStream(data_util_state* state, byte* dataStream, data_string* source);
static nb_size DeserializeDataStreamToDataString(data_util_state* state, const byte* dataStream, data_string* out);
static NB_Error SerializeFromDataSearchFilter(data_util_state* state, data_search_filter* dataSearchFilter, uint8** data, nb_size* dataSize);
static NB_Error DeserializeToDataSearchFilter(data_util_state* state, const uint8* data, nb_size dataSize, data_search_filter* dataSearchFilter);
static NB_Error SerializeFromVectorOfDataPair(data_util_state* state, CSL_Vector* vectorPair, uint8** data, nb_size* dataSize);
static NB_Error DeserializeToVectorOfDataPair(data_util_state* state, const uint8* data, nb_size dataSize, CSL_Vector** vectorPair);
static NB_Error SerializeFromDataPair(data_util_state* state, data_pair* dataPair, uint8** data, nb_size* dataSize);
static NB_Error DeserializeToDataPair(data_util_state* state, const uint8* data, nb_size dataSize, data_pair* dataPair);

static NB_Error GetStringFromPersistentData(NB_PersistentData* persistentData, const char* key, char** data);
static NB_Error SetStringToPersistentData(NB_PersistentData* persistentData, const char* key, const char* data);

static NB_Error ChecksumIsChecked(uint32 crcValue, const uint8* data, nb_size dataSize, nb_boolean* isOK);

// Public functions .............................................................................

/* See header file for description */
NB_DEF NB_Error
NB_SuggestMatchCreateBySerializedData(NB_Context* context,
                                      const uint8* data,
                                      nb_size dataSize,
                                      NB_SuggestMatch** suggestMatch)
{
    NB_Error result = NE_OK;
    NB_SuggestMatch* pThis = NULL;
    nb_boolean ok = FALSE;

    if (!context || !data || (dataSize <= 0) || !suggestMatch)
    {
        return NE_INVAL;
    }

    result = CreateSuggestMatch(context, &pThis);
    if (result != NE_OK)
    {
        return result;
    }

    result = ChecksumIsChecked(CRC_VALUE, data, dataSize, &ok);
    if (result != NE_OK)
    {
        return result;
    }

    if (ok)
    {
        result = DeserializeToSuggestMatchLite(pThis, data, dataSize);
    }
    else
    {
        result = DeserializeToSuggestMatch(pThis, data, dataSize);
    }

    if (result != NE_OK)
    {
        // Destroy the above created 'NB_SuggestMatch' object
        NB_SuggestMatchDestroy(pThis);
        pThis = NULL;

        return result;
    }

    *suggestMatch = pThis;

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SuggestMatchCreateByCopy(NB_SuggestMatch* sourceSuggestMatch,
                            NB_SuggestMatch** suggestMatch)
{
    if (!sourceSuggestMatch || !suggestMatch)
    {
        return NE_INVAL;
    }

    return NB_SuggestMatchCreateByDataSuggestMatch(sourceSuggestMatch->context,
                                                   &(sourceSuggestMatch->dataSuggestMatch),
                                                   suggestMatch);
}

/* See header file for description */
NB_DEF void
NB_SuggestMatchDestroy(NB_SuggestMatch* suggestMatch)
{
    data_util_state* state = NULL;

    if (!suggestMatch)
    {
        return;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    // Free the 'data_suggest_match' object.
    data_suggest_match_free(state, &(suggestMatch->dataSuggestMatch));

    nsl_free(suggestMatch);
}

/* See header file for description */
NB_DEF NB_Error
NB_SuggestMatchGetDataToDisplay(NB_SuggestMatch* suggestMatch,
                                NB_SuggestMatchData* dataToDisplay)
{
    const char* matchTypeString = NULL;
    data_util_state* state = NULL;
    data_suggest_match* dataSuggestMatch = NULL;
    int countIconID = 0;
    int i = 0;

    if (!suggestMatch || !dataToDisplay)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    dataSuggestMatch = &(suggestMatch->dataSuggestMatch);

    if (!dataSuggestMatch)
    {
        return NE_UNEXPECTED;
    }

    dataToDisplay->line1 = data_string_get(state, &(dataSuggestMatch->line1));
    dataToDisplay->line2 = data_string_get(state, &(dataSuggestMatch->line2));
    dataToDisplay->line3 = data_string_get(state, &(dataSuggestMatch->line3));
    dataToDisplay->distance = dataSuggestMatch->distance;

    matchTypeString = data_string_get(state, &(dataSuggestMatch->match_type));
    if (!matchTypeString)
    {
        dataToDisplay->matchType = NB_MT_None;
    }
    else if (nsl_strcmp(matchTypeString, SUGGEST_MATCH_POI) == 0)
    {
        dataToDisplay->matchType = NB_MT_POI;
    }
    else if (nsl_strcmp(matchTypeString, SUGGEST_MATCH_ADDRESS) == 0)
    {
        dataToDisplay->matchType = NB_MT_Address;
    }
    else if (nsl_strcmp(matchTypeString, SUGGEST_MATCH_AIRPORT) == 0)
    {
        dataToDisplay->matchType = NB_MT_Airport;
    }
    else if (nsl_strcmp(matchTypeString, SUGGEST_MATCH_GAS) == 0)
    {
        dataToDisplay->matchType = NB_MT_Gas;
    }
    else if (nsl_strcmp(matchTypeString, SUGGEST_MATCH_CATEGORY) == 0)
    {
        dataToDisplay->matchType = NB_MT_Category;
    }
    else
    {
        dataToDisplay->matchType = NB_MT_None;
    }

    countIconID = CSL_VectorGetLength(dataSuggestMatch->vec_pairs);
    for (i = 0; i < countIconID; ++i)
    {
        data_pair* pairIconID = (data_pair*) CSL_VectorGetPointer(dataSuggestMatch->vec_pairs, i);

        const char* pairKey = data_string_get(state, &pairIconID->key);
        if (nsl_strlen(pairKey) > 0 && nsl_strcmp(pairKey, PERSISTENT_KEY_ICON_ID) == 0)
            dataToDisplay->iconIDs[i] = data_string_get(state, &pairIconID->value);
    }

    dataToDisplay->countIconID = countIconID;

    return NE_OK;
}

/* See header file for description */
NB_DEF NB_Error
NB_SuggestMatchSerialize(NB_SuggestMatch* suggestMatch,
                         uint8** data,
                         nb_size* dataSize)
{
    if (!suggestMatch || !data || !dataSize)
    {
        return NE_INVAL;
    }

    return SerializeFromSuggestMatch(suggestMatch, data, dataSize);
}

/* See header file for description */
NB_DEF NB_Error
NB_SuggestMatchSerializeLite(NB_SuggestMatch* suggestMatch,
                             uint8** data,
                             nb_size* dataSize)
{
    if (!suggestMatch || !data || !dataSize)
    {
        return NE_INVAL;
    }

    return SerializeFromSuggestMatchLite(suggestMatch, data, dataSize);
}

/* See header file for description */
NB_DEF nb_boolean
NB_SuggestMatchIsEqual(NB_SuggestMatch* suggestMatch,
                       NB_SuggestMatch* anotherSuggestMatch)
{
    data_util_state* state = NULL;

    if (suggestMatch == anotherSuggestMatch)
    {
        // Both pointers are same address or NULL.
        return TRUE;
    }
    else if (!suggestMatch || !anotherSuggestMatch)
    {
        // One pointer is NULL.
        return FALSE;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    return (nb_boolean) data_suggest_match_equal(state,
                                                 &(suggestMatch->dataSuggestMatch),
                                                 &(anotherSuggestMatch->dataSuggestMatch));
}


// Private functions ............................................................................

/* See 'nbsuggestmatchprivate.h' for description */
NB_Error
NB_SuggestMatchCreateByDataSuggestMatch(NB_Context* context,
                                        data_suggest_match* dataSuggestMatch,
                                        NB_SuggestMatch** suggestMatch)
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SuggestMatch* pThis = NULL;

    if (!context || !dataSuggestMatch || !suggestMatch)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    result = CreateSuggestMatch(context, &pThis);
    if (result != NE_OK)
    {
        return result;
    }

    result = data_suggest_match_copy(state,
                                     &(pThis->dataSuggestMatch),
                                     dataSuggestMatch);
    if (result != NE_OK)
    {
        // Destroy the above created 'NB_SuggestMatch' object
        NB_SuggestMatchDestroy(pThis);
        pThis = NULL;

        return result;
    }

    *suggestMatch = pThis;

    return result;
}

/* See 'nbsuggestmatchprivate.h' for description */
NB_Error
NB_SuggestMatchCopySearchFilter(NB_SuggestMatch* suggestMatch,
                                data_search_filter* searchFilter)
{
    data_util_state* state = NULL;

    if (!suggestMatch || !searchFilter)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    return data_search_filter_copy(state,
                                   searchFilter,
                                   &(suggestMatch->dataSuggestMatch.search_filter));
}


// Definition of local functions .....................................................................................

/*! Create a new instance of a 'NB_SuggestMatch' object

    Use this function to create a new instance of a 'NB_SuggestMatch' object.
    Call NB_SuggestMatchDestroy() to destroy the object.

    @return NE_OK if success
    @see NB_SuggestMatchDestroy
*/
NB_Error
CreateSuggestMatch(
    NB_Context* context,                    /*!< Pointer to current context */
    NB_SuggestMatch** suggestMatch          /*!< On success, returns pointer to 'NB_SuggestMatch' object */
    )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SuggestMatch* pThis = NULL;

    if (!context || !suggestMatch)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    pThis = nsl_malloc(sizeof(NB_SuggestMatch));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    // Initialize the 'data_suggest_match' object.
    result = data_suggest_match_init(state, &(pThis->dataSuggestMatch));
    if (result != NE_OK)
    {
        NB_SuggestMatchDestroy(pThis);
        return result;
    }

    *suggestMatch = pThis;

    return result;
}

/*! Serialize the 'NB_SuggestMatch' object to data

    @return NE_OK if success
    @see DeserializeToSuggestMatch
*/
NB_Error
SerializeFromSuggestMatch(
    NB_SuggestMatch* suggestMatch,          /*!< 'NB_SuggestMatch' instance */
    uint8** data,                           /*!< On success, receives a pointer to a data buffer. The
                                             pointer must be freed with nsl_free */
    nb_size* dataSize                       /*!< On success, receives the size of the data buffer */
    )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    data_suggest_match* dataSuggestMatch = NULL;
    NB_PersistentData* persistentData = NULL;

    if (!suggestMatch || !data || !dataSize)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    dataSuggestMatch = &(suggestMatch->dataSuggestMatch);
    if (!dataSuggestMatch)
    {
        return NE_UNEXPECTED;
    }

    result = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    // Add 'line1', 'line2' and 'match-type' to persistent data.
    {
        const char* line1 = data_string_get(state, &(dataSuggestMatch->line1));
        const char* line2 = data_string_get(state, &(dataSuggestMatch->line2));
        const char* line3 = data_string_get(state, &(dataSuggestMatch->line3));
        const char* matchType = data_string_get(state, &(dataSuggestMatch->match_type));

        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_LINE1, line1);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_LINE2, line2);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_LINE3, line3);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_MATCH_TYPE, matchType);
        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }
    }

    // Add 'search-filter' to persistent data.
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;
        result = result ? result : SerializeFromDataSearchFilter(state,
                                                                 &(dataSuggestMatch->search_filter),
                                                                 &serializedData,
                                                                 &serializedDataSize);

        if (serializedData && (serializedDataSize > 0))
        {
            result = result ? result : NB_PersistentDataSet(persistentData,
                                                            PERSISTENT_KEY_SEARCH_FILTER,
                                                            serializedData,
                                                            serializedDataSize,
                                                            FALSE,
                                                            FALSE);
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;
        }

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }
    }

    // Add 'icon-id' to persistent data.
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;
        result = result ? result : SerializeFromVectorOfDataPair(state,
                                                                 dataSuggestMatch->vec_pairs,
                                                                 &serializedData,
                                                                 &serializedDataSize);

        if (serializedData && serializedDataSize > 0)
        {
            result = result ? result : NB_PersistentDataSet(persistentData,
                                                            PERSISTENT_KEY_ICON_ID,
                                                            serializedData,
                                                            serializedDataSize,
                                                            FALSE,
                                                            FALSE);
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;
        }
    }

    // Get the serialized data by the 'NB_PersistentData' object.
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        result = NB_PersistentDataSerialize(persistentData,
                                            (const uint8**) (&serializedData),
                                            &serializedDataSize);
        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }

        *data = serializedData;
        *dataSize = serializedDataSize;
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    return result;
}

NB_Error
SerializeFromSuggestMatchLite(
    NB_SuggestMatch* suggestMatch,
    uint8** data,
    nb_size* dataSize
    )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    data_suggest_match* dataSuggestMatch = NULL;
    int i = 0;
    int totalSize = SERIALIZATION_DATA_VERSION_NUM_SIZE;
    unsigned char* space = NULL;
    unsigned char* spaceBaseAddr = NULL;

    if (!suggestMatch || !data || !dataSize)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    dataSuggestMatch = &suggestMatch->dataSuggestMatch;
    if (!dataSuggestMatch)
    {
        return NE_UNEXPECTED;
    }

    /* Here is calculation of space where to store "data_suggest_match". */

    totalSize += sizeof(int); /* Size of CSL_Vector for looping when deserializing. (icon-ids) */
    for (i = 0; i < CSL_VectorGetLength(dataSuggestMatch->vec_pairs); ++i)
    {
        data_pair* pair = (data_pair*)CSL_VectorGetPointer(dataSuggestMatch->vec_pairs, i);
        nsl_assert(pair != NULL);

        totalSize += sizeof(boolean); /* transient */
        totalSize += data_string_get_tps_size(state, &pair->key) + 1;
        totalSize += data_string_get_tps_size(state, &pair->value) + 1;
        totalSize += data_string_get_tps_size(state, &pair->section) + 1;
        totalSize += data_string_get_tps_size(state, &pair->type) + 1;
    }

    totalSize += sizeof(int); /* Size of CSL_Vector for looping when deserializing. */
    for (i = 0; i < CSL_VectorGetLength(dataSuggestMatch->search_filter.vec_pairs); ++i)
    {
        data_pair* pair = (data_pair*)CSL_VectorGetPointer(dataSuggestMatch->search_filter.vec_pairs, i);
        nsl_assert(pair != NULL);

        totalSize += sizeof(boolean); /* transient */
        totalSize += data_string_get_tps_size(state, &pair->key) + 1;
        totalSize += data_string_get_tps_size(state, &pair->value) + 1;
        totalSize += data_string_get_tps_size(state, &pair->section) + 1;
        totalSize += data_string_get_tps_size(state, &pair->type) + 1;
    }

    totalSize += data_string_get_tps_size(state, &dataSuggestMatch->search_filter.result_style.key) + 1;

    nsl_assert(totalSize > 0); /* Make sure totalSize is never 0. */

    /* Here starts to serialize. */
    space = (unsigned char*)nsl_malloc(totalSize);
    spaceBaseAddr = space;
    nsl_memset(space, 0, totalSize);

    /* Version number. */
    *((int*)spaceBaseAddr) = 0; /* Later decided. "checksum" will be used. */
    /* icon-ids */
    *((int*)(spaceBaseAddr += sizeof(int))) = CSL_VectorGetLength(dataSuggestMatch->vec_pairs);
    spaceBaseAddr += sizeof(int);
    for (i = 0; i < CSL_VectorGetLength(dataSuggestMatch->vec_pairs); ++i)
    {
        data_pair* pair = (data_pair*)CSL_VectorGetPointer(dataSuggestMatch->vec_pairs, i);
        nsl_assert(pair != NULL);

        *((boolean*)spaceBaseAddr) = pair->transient;
        spaceBaseAddr += sizeof(boolean);

        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->key);
        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->value);
        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->section);
        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->type);
    }
    /* Size of No.1 part of search filter. */
    *((int*)spaceBaseAddr) = CSL_VectorGetLength(dataSuggestMatch->search_filter.vec_pairs);
    spaceBaseAddr += sizeof(int);
    /* Serialize an entire vector in a search filter. */
    for (i = 0; i < CSL_VectorGetLength(dataSuggestMatch->search_filter.vec_pairs); ++i)
    {
        data_pair* pair = (data_pair*)CSL_VectorGetPointer(dataSuggestMatch->search_filter.vec_pairs, i);
        nsl_assert(pair != NULL);

        *((boolean*)spaceBaseAddr) = pair->transient;
        spaceBaseAddr += sizeof(boolean);

        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->key);
        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->value);
        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->section);
        spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &pair->type);
    }

    spaceBaseAddr += SerializeDataStringToDataStream(state, spaceBaseAddr, &dataSuggestMatch->search_filter.result_style.key);


    /* The first 4 bytes indicate a version number of integer. */
    obfuscate_buffer((byte*)(space + SERIALIZATION_DATA_VERSION_NUM_SIZE),
                     totalSize - SERIALIZATION_DATA_VERSION_NUM_SIZE,
                     PERSIST_DATA_KEY,
                     PERSIST_DATA_KEY_SIZE);

    /* Now decide the check-sum, and make it be version number. */
    *((int*)space) = crc32(CRC_VALUE,
                           (const byte*)(space + SERIALIZATION_DATA_VERSION_NUM_SIZE),
                           totalSize - SERIALIZATION_DATA_VERSION_NUM_SIZE);

    *data = space;
    *dataSize = totalSize;

    return result;
}

/*! Deserialize the data to the 'NB_SuggestMatch' object

    @return NE_OK if success
    @see SerializeFromSuggestMatch
*/
NB_Error
DeserializeToSuggestMatch(
    NB_SuggestMatch* suggestMatch,          /*!< 'NB_SuggestMatch' instance */
    const uint8* data,                      /*!< A data buffer containing the results of a previous
                                                 'SerializeFromSuggestMatch' call */
    nb_size dataSize                        /*!< The size of the data buffer */
    )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_PersistentData* persistentData = NULL;
    data_suggest_match dataSuggestMatch;
    nsl_memset(&dataSuggestMatch, 0, sizeof(dataSuggestMatch));

    if (!suggestMatch || !data || (dataSize <= 0))
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    result = NB_PersistentDataCreate(data, dataSize, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    result = data_suggest_match_init(state, &dataSuggestMatch);
    if (result != NE_OK)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;

        return result;
    }

    // Get 'line1', 'line2' and 'match-type' from persistent data.
    {
        char* line1 = NULL;
        char* line2 = NULL;
        char* line3 = NULL;
        char* matchType = NULL;

        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_LINE1, &line1);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_LINE2, &line2);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_LINE3, &line3);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_MATCH_TYPE, &matchType);

        if (line1)
        {
            result = result ? result : data_string_set(state, &(dataSuggestMatch.line1), line1);

            nsl_free(line1);
            line1 = NULL;
        }
        if (line2)
        {
            result = result ? result : data_string_set(state, &(dataSuggestMatch.line2), line2);

            nsl_free(line2);
            line2 = NULL;
        }
        if (line3)
        {
            result = result ? result : data_string_set(state, &(dataSuggestMatch.line3), line3);

            nsl_free(line3);
            line3 = NULL;
        }
        if (matchType)
        {
            result = result ? result : data_string_set(state, &(dataSuggestMatch.match_type), matchType);

            nsl_free(matchType);
            matchType = NULL;
        }

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            data_suggest_match_free(state, &dataSuggestMatch);

            return result;
        }
    }

    // Get 'search-filter' from persistent data.
    {
        uint8* dataOfSearchFilter = NULL;
        nb_size dataSizeOfSearchFilter = 0;

        result = NB_PersistentDataGet(persistentData,
                                      PERSISTENT_KEY_SEARCH_FILTER,
                                      &dataOfSearchFilter,
                                      &dataSizeOfSearchFilter);
        if (result == NE_OK)
        {
            result = DeserializeToDataSearchFilter(state,
                                                   dataOfSearchFilter,
                                                   dataSizeOfSearchFilter,
                                                   &(dataSuggestMatch.search_filter));
            nsl_free(dataOfSearchFilter);
            dataOfSearchFilter = NULL;
            dataSizeOfSearchFilter = 0;
        }
        else if (result == NE_NOENT)
        {
            // Ignore the 'NE_NOENT' error.
            result = NE_OK;
        }
        else
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            data_suggest_match_free(state, &dataSuggestMatch);

            return result;
        }
    }

    // Get 'icon-id' from persistent data.
    {
        uint8* dataOfIconID = NULL;
        nb_size dataSizeOfIconID = 0;

        result = NB_PersistentDataGet(persistentData,
                                      PERSISTENT_KEY_ICON_ID,
                                      &dataOfIconID,
                                      &dataSizeOfIconID);
        if (result == NE_OK)
        {
            result = DeserializeToVectorOfDataPair(state,
                                                   dataOfIconID,
                                                   dataSizeOfIconID,
                                                   &(dataSuggestMatch.vec_pairs));
            nsl_free(dataOfIconID);
            dataOfIconID = NULL;
            dataSizeOfIconID = 0;
        }
        else if (result == NE_NOENT)
        {
            // Ignore the 'NE_NOENT' error.
            result = NE_OK;
        }
        else
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            data_suggest_match_free(state, &dataSuggestMatch);

            return result;
        }
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    result = data_suggest_match_copy(state,
                                     &(suggestMatch->dataSuggestMatch),
                                     &dataSuggestMatch);
    data_suggest_match_free(state, &dataSuggestMatch);
    if (result != NE_OK)
    {
        return result;
    }

    return result;
}

NB_Error DeserializeToSuggestMatchLite(NB_SuggestMatch* suggestMatch, const uint8* data, nb_size dataSize)
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    data_suggest_match dataSuggestMatch;
    int i = 0;
    int checksum = 0;
    int vec_len = 0;
    const uint8* spaceBaseAddr = NULL;

    nsl_memset(&dataSuggestMatch, 0, sizeof(dataSuggestMatch));

    dataSuggestMatch.vec_pairs = CSL_VectorAlloc(sizeof(data_pair));
    dataSuggestMatch.search_filter.vec_pairs = CSL_VectorAlloc(sizeof(data_pair));

    if (!suggestMatch || !data || (dataSize <= 0))
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(suggestMatch->context);

    obfuscate_buffer((byte*)(data + SERIALIZATION_DATA_VERSION_NUM_SIZE),
                     dataSize - SERIALIZATION_DATA_VERSION_NUM_SIZE,
                     PERSIST_DATA_KEY,
                     PERSIST_DATA_KEY_SIZE);

    spaceBaseAddr = data;

    checksum = *((int*)spaceBaseAddr);
    (void)checksum; /* Unused in this function. */

    /* icon-ids */
    vec_len = *((int*)(spaceBaseAddr += sizeof(int)));
    spaceBaseAddr += sizeof(int);
    for (i = 0; i < vec_len; ++i)
    {
        data_pair* dp = (data_pair*)nsl_malloc(sizeof(data_pair));
        if (dp == NULL)
        {
            return NE_NOMEM;
        }

        data_pair_init(state, dp);

        dp->transient = *((boolean*)spaceBaseAddr);
        spaceBaseAddr += sizeof(boolean);

        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->key);
        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->value);
        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->section);
        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->type);

        CSL_VectorAppend(dataSuggestMatch.vec_pairs, dp);

        if(dp)
        {
            nsl_free(dp);
            dp = NULL;
        }
    }

    /* Search filter */
    vec_len = *((int*)spaceBaseAddr);
    spaceBaseAddr += sizeof(int);
    for (i = 0; i < vec_len; ++i)
    {
        data_pair* dp = (data_pair*)nsl_malloc(sizeof(data_pair));
        if (dp == NULL)
        {
            return NE_NOMEM;
        }

        data_pair_init(state, dp);

        dp->transient = *((boolean*)spaceBaseAddr);
        spaceBaseAddr += sizeof(boolean);

        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->key);
        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->value);
        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->section);
        spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dp->type);

        CSL_VectorAppend(dataSuggestMatch.search_filter.vec_pairs, dp);

        if(dp)
        {
            nsl_free(dp);
            dp = NULL;
        }
    }

    spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dataSuggestMatch.search_filter.result_style.key);

    spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dataSuggestMatch.line1);
    spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dataSuggestMatch.line2);
    spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dataSuggestMatch.line3);
    spaceBaseAddr += DeserializeDataStreamToDataString(state, spaceBaseAddr, &dataSuggestMatch.match_type);

    data_suggest_match_copy(state, &suggestMatch->dataSuggestMatch, &dataSuggestMatch);

    DATA_VEC_FREE( state, dataSuggestMatch.search_filter.vec_pairs, data_pair );
    nsl_free( dataSuggestMatch.search_filter.vec_pairs );


    data_suggest_match_free(state, &dataSuggestMatch);

    return result;
}

/*! Serialize a 'data_string' object to a data stream.

    @return the length of serialized data_string.
*/
nb_size
SerializeDataStringToDataStream(
    data_util_state* pds,
    byte* dataStream,
    data_string* source
    )
{
    nb_size len = 1;

    if (pds == NULL || dataStream == NULL || source == NULL || *source == NULL)
    {
        return len;
    }

    len = data_string_get_tps_size(pds, source) + 1;
    if (len > 1)
    {
        nsl_memcpy(dataStream, *source, len);
    }

    return len;
}

/*! Deserialize a data stream to a data string.

    @return the length of deserialized data_string.
*/
nb_size
DeserializeDataStreamToDataString(
    data_util_state* pds,
    const byte* dataStream,
    data_string* out
    )
{
    nb_size len = 1;

    if (pds == NULL || dataStream == NULL || out == NULL)
    {
        return len;
    }

    len = data_string_get_tps_size(pds, (data_string*)&dataStream) + 1;
    if (len > 1)
    {
        *out = nsl_strdup((const char*)dataStream);
    }

    return len;
}

/*! Serialize the 'data_search_filter' object to data

    @return NE_OK if success
    @see DeserializeToDataSearchFilter
*/
NB_Error
SerializeFromDataSearchFilter(
    data_util_state* state,                 /*!< 'data_util_state' instance */
    data_search_filter* dataSearchFilter,   /*!< 'data_search_filter' instance */
    uint8** data,                           /*!< On success, receives a pointer to a data buffer. The
                                                 pointer must be freed with nsl_free */
    nb_size* dataSize                       /*!< On success, receives the size of the data buffer */
    )
{
    NB_Error result = NE_OK;
    NB_PersistentData* persistentData = NULL;

    if (!state || !dataSearchFilter || !data || !dataSize)
    {
        return NE_INVAL;
    }

    result = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    /*  Serialize from the 'data_result_style' object. And then clear the 'NB_PersistentData'
        object and Serialize from the 'data_search_filter' object.
    */

    // Add 'key' of 'result-style' to persistent data.
    {
        const char* key = data_string_get(state, &(dataSearchFilter->result_style.key));
        result = SetStringToPersistentData(persistentData, PERSISTENT_KEY_KEY, key);
        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }
    }

    /*  Serialize from the 'data_result_style' object. Then clear the 'NB_PersistentData'
        object and add 'result-style' to persistent data.
    */
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        result = result ? result : NB_PersistentDataSerialize(persistentData,
                                                              (const uint8**) (&serializedData),
                                                              &serializedDataSize);
        result = result ? result : NB_PersistentDataMasterClear(persistentData);

        if (serializedData && (serializedDataSize > 0))
        {
            result = result ? result : NB_PersistentDataSet(persistentData,
                                                            PERSISTENT_KEY_RESULT_STYLE,
                                                            serializedData,
                                                            serializedDataSize,
                                                            FALSE,
                                                            FALSE);
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;
        }

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }
    }

    // Add vector of 'pair' to persistent data.
    if (dataSearchFilter->vec_pairs)
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        result = result ? result : SerializeFromVectorOfDataPair(state,
                                                                 dataSearchFilter->vec_pairs,
                                                                 &serializedData,
                                                                 &serializedDataSize);
        if (serializedData && (serializedDataSize > 0))
        {
            result = result ? result : NB_PersistentDataSet(persistentData,
                                                            PERSISTENT_KEY_VECTOR_PAIR,
                                                            serializedData,
                                                            serializedDataSize,
                                                            FALSE,
                                                            FALSE);
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;
        }

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }
    }

    // Get the serialized data by the 'NB_PersistentData' object.
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        result = NB_PersistentDataSerialize(persistentData,
                                            (const uint8**) (&serializedData),
                                            &serializedDataSize);
        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }

        *data = serializedData;
        *dataSize = serializedDataSize;
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    return result;
}

/*! Deserialize the data to the 'data_search_filter' object

    @return NE_OK if success
    @see SerializeFromDataSearchFilter
*/
NB_Error
DeserializeToDataSearchFilter(
    data_util_state* state,                 /*!< 'data_util_state' instance */
    const uint8* data,                      /*!< A data buffer containing the results of a previous
                                                 'SerializeFromDataSearchFilter' call */
    nb_size dataSize,                       /*!< The size of the data buffer */
    data_search_filter* dataSearchFilter    /*!< On success, this 'data_search_filter' object is filled */
    )
{
    NB_Error result = NE_OK;
    NB_PersistentData* persistentData = NULL;
    data_search_filter tempDataSearchFilter = {0};

    if (!state || !data || (dataSize <= 0) || !dataSearchFilter)
    {
        return NE_INVAL;
    }

    result = NB_PersistentDataCreate(data, dataSize, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    result = data_search_filter_init(state, &tempDataSearchFilter);
    if (result != NE_OK)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;

        return result;
    }

    // Get vector of 'pair' from persistent data.
    {
        uint8* dataOfvectorPair = NULL;
        nb_size dataSizeOfVectorPair = 0;

        result = NB_PersistentDataGet(persistentData,
                                      PERSISTENT_KEY_VECTOR_PAIR,
                                      &dataOfvectorPair,
                                      &dataSizeOfVectorPair);
        if (result == NE_OK)
        {
            result = DeserializeToVectorOfDataPair(state,
                                                   dataOfvectorPair,
                                                   dataSizeOfVectorPair,
                                                   &(tempDataSearchFilter.vec_pairs));
            nsl_free(dataOfvectorPair);
            dataOfvectorPair = NULL;
            dataSizeOfVectorPair = 0;
        }
        else if (result == NE_NOENT)
        {
            // Ignore the 'NE_NOENT' error.
            result = NE_OK;
        }
        else
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            data_search_filter_free(state, &tempDataSearchFilter);

            return result;
        }
    }

    // Get 'result-style' from persistent data.
    {
        uint8* dataOfResultStyle = NULL;
        nb_size dataSizeOfResultStyle = 0;

        result = NB_PersistentDataGet(persistentData,
                                      PERSISTENT_KEY_RESULT_STYLE,
                                      &dataOfResultStyle,
                                      &dataSizeOfResultStyle);

        /*  Destroy the 'NB_PersistentData' object and ignore the error returned. Then
            create new 'NB_PersistentData' object to deserialize the data to the
            'data_result_style' object.
        */
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
        if (result == NE_OK)
        {
            result = NB_PersistentDataCreate(dataOfResultStyle,
                                             dataSizeOfResultStyle,
                                             NULL,
                                             &persistentData);

            nsl_free(dataOfResultStyle);
            dataOfResultStyle = NULL;
            dataSizeOfResultStyle = 0;

            if (result != NE_OK)
            {
                data_search_filter_free(state, &tempDataSearchFilter);

                return result;
            }

            // Get 'key' of 'result-style' from persistent data.
            {
                char* key = NULL;

                result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_KEY, &key);

                if (key)
                {
                    result = result ? result : data_string_set(state,
                                                               &(tempDataSearchFilter.result_style.key),
                                                               key);

                    nsl_free(key);
                    key = NULL;
                }

                if (result != NE_OK)
                {
                    // Ignore the error returned.
                    NB_PersistentDataDestroy(persistentData);
                    persistentData = NULL;

                    data_search_filter_free(state, &tempDataSearchFilter);

                    return result;
                }
            }

            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;
        }
        else if (result == NE_NOENT)
        {
            // Ignore the 'NE_NOENT' error.
            result = NE_OK;
        }
        else
        {
            data_search_filter_free(state, &tempDataSearchFilter);
            return result;
        }
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    result = data_search_filter_copy(state,
                                     dataSearchFilter,
                                     &tempDataSearchFilter);
    data_search_filter_free(state, &tempDataSearchFilter);
    if (result != NE_OK)
    {
        return result;
    }

    return result;
}

/*! Serialize vector of 'data_pair' objects to data

    @return NE_OK if success
    @see DeserializeToVectorOfDataPair
*/
NB_Error
SerializeFromVectorOfDataPair(
    data_util_state* state,                 /*!< 'data_util_state' instance */
    CSL_Vector* vectorPair,                 /*!< Vector of 'data_pair' objects */
    uint8** data,                           /*!< On success, receives a pointer to a data buffer. The
                                                 pointer must be freed with nsl_free */
    nb_size* dataSize                       /*!< On success, receives the size of the data buffer */
    )
{
    NB_Error result = NE_OK;
    int n = 0;
    int length = 0;
    NB_PersistentData* persistentData = NULL;
    char persistentKey[PERSISTENT_DATA_MAX_LENGTH] = {0};

    if (!state || !vectorPair || !data || !dataSize)
    {
        return NE_INVAL;
    }

    result = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    length = CSL_VectorGetLength(vectorPair);
    for (n = 0; n < length; ++n)
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        data_pair* dataPair = (data_pair*) CSL_VectorGetPointer(vectorPair, n);
        if (!dataPair)
        {
            continue;
        }

        // Convert an integer to a string as the persistent key.
        nsl_memset(persistentKey, 0, PERSISTENT_DATA_MAX_LENGTH);
        nsl_sprintf(persistentKey, "%d", n);

        result = result ? result : SerializeFromDataPair(state,
                                                         dataPair,
                                                         &serializedData,
                                                         &serializedDataSize);

        if (serializedData && (serializedDataSize > 0))
        {
            result = result ? result : NB_PersistentDataSet(persistentData,
                                                            persistentKey,
                                                            serializedData,
                                                            serializedDataSize,
                                                            FALSE,
                                                            FALSE);
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;
        }

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            break;
        }
    }

    // Get the serialized data by the 'NB_PersistentData' object.
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        result = NB_PersistentDataSerialize(persistentData,
                                            (const uint8**) (&serializedData),
                                            &serializedDataSize);
        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }

        *data = serializedData;
        *dataSize = serializedDataSize;
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    return result;
}

/*! Deserialize the data to vector of 'data_pair' objects

    @return NE_OK if success
    @see SerializeFromVectorOfDataPair
*/
NB_Error
DeserializeToVectorOfDataPair(
    data_util_state* state,                 /*!< 'data_util_state' instance */
    const uint8* data,                      /*!< A data buffer containing the results of a previous
                                                 'SerializeFromVectorOfDataPair' call */
    nb_size dataSize,                       /*!< The size of the data buffer */
    CSL_Vector** vectorPair                 /*!< On success, returns a new created 'CSL_Vector'
                                                 object. It should be destroyed by calling the
                                                 function 'CSL_VectorDealloc' when it is useless. */
    )
{
    NB_Error result = NE_OK;
    int n = 0;
    CSL_Vector* tempVectorPair = NULL;
    NB_PersistentData* persistentData = NULL;
    char persistentKey[PERSISTENT_DATA_MAX_LENGTH] = {0};

    if (!state || !data || (dataSize <= 0) || !vectorPair)
    {
        return NE_INVAL;
    }

    tempVectorPair = CSL_VectorAlloc(sizeof(data_pair));
    if (!tempVectorPair)
    {
        return NE_NOMEM;
    }

    result = NB_PersistentDataCreate(data, dataSize, NULL, &persistentData);
    if (result != NE_OK)
    {
        CSL_VectorDealloc(tempVectorPair);
        tempVectorPair = NULL;

        return result;
    }

    n = 0;
    while (result == NE_OK)
    {
        uint8* dataOfDataPair = NULL;
        nb_size dataSizeOfDataPair = 0;
        data_pair dataPair = {0};

        // Convert an integer to a string as the persistent key.
        nsl_memset(persistentKey, 0, PERSISTENT_DATA_MAX_LENGTH);
        nsl_sprintf(persistentKey, "%d", n);

        result = NB_PersistentDataGet(persistentData,
                                      persistentKey,
                                      &dataOfDataPair,
                                      &dataSizeOfDataPair);
        if (result != NE_OK)
        {
            // Check the error and release resources after this loop.
            break;
        }

        result = data_pair_init(state, &dataPair);
        if (result != NE_OK)
        {
            nsl_free(dataOfDataPair);
            dataOfDataPair = NULL;
            dataSizeOfDataPair = 0;

            // Check the error and release resources after this loop.
            break;
        }

        result = DeserializeToDataPair(state,
                                       dataOfDataPair,
                                       dataSizeOfDataPair,
                                       &dataPair);
        nsl_free(dataOfDataPair);
        dataOfDataPair = NULL;
        dataSizeOfDataPair = 0;
        if (result != NE_OK)
        {
            data_pair_free(state, &dataPair);

            // Check the error and release resources after this loop.
            break;
        }

        result = result ? result : (CSL_VectorAppend(tempVectorPair, &dataPair) ? NE_OK : NE_NOMEM);
        if (result != NE_OK)
        {
            data_pair_free(state, &dataPair);

            // Check the error and release resources after this loop.
            break;
        }

        ++n;
    }
    result = (result == NE_NOENT) ? NE_OK : result;

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    if (result == NE_OK)
    {
        *vectorPair = tempVectorPair;
        tempVectorPair = NULL;
    }
    else
    {
        if (tempVectorPair)
        {
            CSL_VectorDealloc(tempVectorPair);
            tempVectorPair = NULL;
        }
    }

    return result;
}

/*! Serialize the 'data_pair' object to data

    @return NE_OK if success
    @see DeserializeToDataPair
*/
NB_Error
SerializeFromDataPair(
    data_util_state* state,                 /*!< 'data_util_state' instance */
    data_pair* dataPair,                    /*!< 'data_pair' instance */
    uint8** data,                           /*!< On success, receives a pointer to a data buffer. The
                                                 pointer must be freed with nsl_free */
    nb_size* dataSize                       /*!< On success, receives the size of the data buffer */
    )
{
    NB_Error result = NE_OK;
    NB_PersistentData* persistentData = NULL;

    if (!state || !dataPair || !data || !dataSize)
    {
        return NE_INVAL;
    }

    result = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    // Add 'transient', 'key', 'value', 'section' and 'type' to persistent data.
    {
        const char* transient = (dataPair->transient) ? PERSISTENT_DATA_BOOLEAN_TRUE : PERSISTENT_DATA_BOOLEAN_FALSE;
        const char* key = data_string_get(state, &(dataPair->key));
        const char* value = data_string_get(state, &(dataPair->value));
        const char* section = data_string_get(state, &(dataPair->section));
        const char* type = data_string_get(state, &(dataPair->type));

        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_TRANSIENT, transient);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_KEY, key);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_VALUE, value);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_SECTION, section);
        result = result ? result : SetStringToPersistentData(persistentData, PERSISTENT_KEY_TYPE, type);

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }
    }

    // Get the serialized data by the 'NB_PersistentData' object.
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;

        result = NB_PersistentDataSerialize(persistentData,
                                            (const uint8**) (&serializedData),
                                            &serializedDataSize);
        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            return result;
        }

        *data = serializedData;
        *dataSize = serializedDataSize;
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    return result;
}

/*! Deserialize the data to the 'data_pair' object

    @return NE_OK if success
    @see SerializeFromDataPair
*/
NB_Error
DeserializeToDataPair(
    data_util_state* state,                 /*!< 'data_util_state' instance */
    const uint8* data,                      /*!< A data buffer containing the results of a previous
                                                 'SerializeFromDataPair' call */
    nb_size dataSize,                       /*!< The size of the data buffer */
    data_pair* dataPair                     /*!< On success, this 'data_pair' object is filled */
    )
{
    NB_Error result = NE_OK;
    NB_PersistentData* persistentData = NULL;
    data_pair tempDataPair = {0};

    if (!state || !data || (dataSize <= 0) || !dataPair)
    {
        return NE_INVAL;
    }

    result = NB_PersistentDataCreate(data, dataSize, NULL, &persistentData);
    if (result != NE_OK)
    {
        return result;
    }

    result = data_pair_init(state, &tempDataPair);
    if (result != NE_OK)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;

        return result;
    }

    // Get 'transient', 'key', 'value', 'section' and 'type' from persistent data.
    {
        char* transient = NULL;
        char* key = NULL;
        char* value = NULL;
        char* section = NULL;
        char* type = NULL;

        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_TRANSIENT, &transient);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_KEY, &key);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_VALUE, &value);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_SECTION, &section);
        result = result ? result : GetStringFromPersistentData(persistentData, PERSISTENT_KEY_TYPE, &type);

        if (transient)
        {
            if ((nsl_strcmp(transient, PERSISTENT_DATA_BOOLEAN_TRUE) == 0))
            {
                tempDataPair.transient = TRUE;
            }
            else
            {
                tempDataPair.transient = FALSE;
            }

            nsl_free(transient);
            transient = NULL;
        }
        if (key)
        {
            result = result ? result : data_string_set(state, &(tempDataPair.key), key);

            nsl_free(key);
            key = NULL;
        }
        if (value)
        {
            result = result ? result : data_string_set(state, &(tempDataPair.value), value);

            nsl_free(value);
            value = NULL;
        }
        if (section)
        {
            result = result ? result : data_string_set(state, &(tempDataPair.section), section);

            nsl_free(section);
            section = NULL;
        }
        if (type)
        {
            result = result ? result : data_string_set(state, &(tempDataPair.type), type);

            nsl_free(type);
            type = NULL;
        }

        if (result != NE_OK)
        {
            // Ignore the error returned.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;

            data_pair_free(state, &tempDataPair);

            return result;
        }
    }

    if (persistentData)
    {
        // Ignore the error returned.
        NB_PersistentDataDestroy(persistentData);
        persistentData = NULL;
    }

    result = data_pair_copy(state,
                            dataPair,
                            &tempDataPair);
    data_pair_free(state, &tempDataPair);
    if (result != NE_OK)
    {
        return result;
    }

    return result;
}

/*! Get a string from the 'NB_PersistentData' object

    This function returns a pointer to a string if success. This pointer
    must be freed with 'nsl_free' when it is useless.

    @return NE_OK if success
*/
NB_Error
GetStringFromPersistentData(
    NB_PersistentData* persistentData,      /*!< 'NB_PersistentData' instance */
    const char* key,                        /*!< Persistent key */
    char** data                             /*!< On success, receives a pointer to a string. The
                                                 pointer must be freed with 'nsl_free' */
    )
{
    NB_Error result = NE_OK;
    char* tempData = NULL;
    nb_size dataSize = 0;

    if (!persistentData || !key || !data)
    {
        return NE_INVAL;
    }

    /*  Get the data to a 'char*'. Because Storing length of string plus 1. This is same
        as below. See the serialized functions for details.
    */
    result = NB_PersistentDataGet(persistentData,
                                  key,
                                  (uint8**) (&tempData),
                                  &dataSize);

    if (result == NE_OK)
    {
        *data = tempData;
        tempData = NULL;
    }
    else if (result == NE_NOENT)
    {
        // Returns 'NE_OK' to caller if can not find a string by key.
        result = NE_OK;
        *data = NULL;
    }

    return result;
}

/*! Set a string to the 'NB_PersistentData' object

    @return NE_OK if success
*/
NB_Error
SetStringToPersistentData(
    NB_PersistentData* persistentData,      /*!< 'NB_PersistentData' instance */
    const char* key,                        /*!< Persistent key */
    const char* data                        /*!< Persistent data of type string */
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

    // There is no need to add if parameter 'data' is "".
    dataSize = (nb_size) (nsl_strlen(data) + 1);
    if (dataSize <= 1)
    {
        // Just returns 'NE_OK' to caller.
        return NE_OK;
    }

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

NB_Error
ChecksumIsChecked(
    uint32 crcValue,   /*!< CRC Value. */
    const uint8* data, /*!< The data which has check-sum at first 4 bytes. */
    nb_size dataSize,  /*!< The size of data block above. */
    nb_boolean* isOK   /*!< True if the check-sum at first 4 bytes is equal to the calculated one from the rest data. Otherwise, FALSE. */
    )
{
    NB_Error err = NE_OK;
    uint32 checksum = 0, originalChecksum = 0;

    if (data == NULL || isOK == NULL)
    {
        return NE_INVAL;
    }

    checksum = crc32(crcValue, data + sizeof(uint32), dataSize - sizeof(uint32));
    originalChecksum = *((uint32*)data);

    *isOK = (checksum == originalChecksum ? TRUE : FALSE);

    return err;
}

/*! @} */
