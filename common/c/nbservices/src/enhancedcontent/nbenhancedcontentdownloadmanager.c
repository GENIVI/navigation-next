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

    @file     nbenhancedcontentdownloadmanager.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbenhancedcontentdownloadmanager.h"
#include "palnet.h"
#include "palunzip.h"
#include "nbcontextprotected.h"
#include "vec.h"
#include "fileutil.h"
#include "cslcache.h"
#include "nbqalog.h"


/*! @{ */


// Constants .........................................................................................................

#define MAX_HOSTNAME_LENGTH             128
#define MAX_URL_LENGTH                  256
#define MAX_ADDITIONAL_HEADERS_LENGTH   256
#define MAX_FILE_PATH                   1024

#define MAX_RETRY_TIMES        3
#define PRIORITY_INVALID       0xFFFFFFFF


// Remove this from the base url. The connection will fail if it is present.
static const char BASE_URL_REMOVE[]                = "http://";
static const char HTTP_VERB[]                      = "GET";

// Constants for http additional headers
static const char CLIENT_GUID_HEADER[]             = "X-Navbuilder-Clientid: ";
static const char HEADER_SEPARATOR[]               = "\r\n";
//header for resume from break point
static const char HEADER_RANGE[]                   = "Range: bytes=";

// We use the cache mainly for the persistent cache functionality and not the memory cache functionality. So we only
// need to keep a few elements in the memory cache. This number affects how many entries we can keep locked. So this
// number is dependent on the number of signs/MJOs the UI holds on to at any given time.
const int MAX_MEMORY_CACHE_COUNT = 5;

// The obfuscate key is only used to obfuscate the index-file for the cache and not the actual entries.
static const byte OBFUSCATE_KEY[] = {0x01, 0x02, 0x03, 0xAA, 0xBB, 0xAF, 0xFA, 0x23, 0xA8, 0x99};


// Local types .......................................................................................................

/*! Entry for pending download queue

    A pending entry can either contain one 'NB_EnhancedContentIdentifier' item or multiple items
    in one dataset.

    @see NB_EnhancedContentIdentifier
*/
typedef struct
{
    NB_EnhancedContentDataType type;    /*!< See NB_EnhancedContentIdentifier */
    char*   fileVersion;                /*!< File version for MJOs and SARs */
    char*   dataset;                    /*!< See NB_EnhancedContentIdentifier */
    char**  itemIdArray;                /*!< Array of item-Ids
                                             If it's NULL the dataset is a single file. (USA-COMMON.zip, USA-MJO.zip) */
    int     itemCount;                  /*!< Number of item-Ids in the array */
    int     nextItemIndex;              /*!< This is the next item we need to request/download. Only after we download
                                             all items do we delete the PendingEntry structure which contains the array */
} PendingEntry;

/*! Temporary item used for HTTP request.

    The lifespan of this item is during a HTTP request. It gets created when starting the http request and gets
    destroyed once done.
*/
typedef struct
{
    /*! The item to download */
    NB_EnhancedContentIdentifier* item;

    /*!< File version for MJOs and SARs */
    char*   fileVersion;

    /*! Buffer to hold the (partially) downloaded data.
        Used to save cache. (MJO and SAR) */
    byte*   buffer;
    uint32  size;

    /*! Path to save item.
        Full path used to save file. (City model and textures) */
    char*   path;

    /*! If there's an error during this tile download then we set this flag and report the error once the
        download has finished (instead of reporting multiple errors). */
    NB_Error error;

    /* Total file size */
    uint32 fileSize;

    /*Downloaded file size */
    uint32 downloadedSize;
} RequestItem;

/*! Download queue of download manager. */
typedef struct _DownloadQueue
{
    NB_EnhancedContentDownloadLevel level;          /*!< Level of this queue. */
    nb_boolean                      enabled;        /*!< Flag to indicate whether this queue is enabled */
    CSL_Vector*                     pendingItems;   /*!< Pending request to be downloaded. */
} DownloadQueue;

/*! HTTP connection used for preload and on-demand connections */
typedef struct
{
    NB_EnhancedContentDownloadManager*  manager;        /*!< Pointer to parent structure */

    PAL_NetConnection*                  httpConnection; /*!< PAL connection of this Connection */
    DownloadQueue*                      pendingQueues;  /*!< Array of download queues, sorted by download level*/
    CSL_Vector*                         pendingItems;   /*!< Pointer to the list being
                                                             processed currently, added for
                                                             backward compability.*/
    RequestItem*                        requestItem;    /*!< Request item */
    nb_boolean                          inProgress;     /*!< Download in progress */
    int                                 retryTimes;     /*!< Times retried for current request.*/

} Connection;


/*! The download manager */
struct NB_EnhancedContentDownloadManager
{
    NB_Context*                                     context;
    NB_EnhancedContentDownloadManagerCallbackData   callbackData;

    /*! The data source is owned by the enhanced content manager */
    NB_EnhancedContentDataSource*                   dataSource;

    /*! Host name and port for the HTTP connection. This information gets retrieved from the data source */
    char                                            hostName[MAX_HOSTNAME_LENGTH];
    uint16                                          port;

    /*! HTTP connections */
    Connection                                      preloadConnection;
    Connection                                      onDemandConnection;

    /*! Cache for SAR (signs are real) and MJO (motor junction objects).

        SAR and MJOs are stored using the CSL cache. The cache ensures that the oldest objects get
        cleared and only the latest objects are kept. We use the cache in non-obfuscated mode so that
        the navigation can directly access the files.
    */
    CSL_Cache*                                      signsCache;
    CSL_Cache*                                      junctionsCache;

    CSL_Cache*                                      speedLimitsSignCache; /*!< Cache for speed limit images. */

    CSL_Cache*                                      specialRegionsCache;  /*!< Cache for special region images. */

    // @todo: Add content we need from NB_EnhancedContentManagerConfiguration

    char enhancedCityModelPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];       /*!< ECM file path */
    char motorwayJunctionObjectPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];  /*!< MJO file path */
    char texturesPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                /*!< textures path */
    char realisticSignsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];          /*!< SAR path */
    char speedLimitsSignsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];        /*!< Speed limit path */
    char specialRegionsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];          /*!< Special region path. */
    char regionsPath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];                 /*!< region data file path */

    char additionalHeaders[MAX_ADDITIONAL_HEADERS_LENGTH + 1];              /*!< Additional headers for http requests */

    nb_boolean waitingForMetadata; /*!< Flag to indicate that there are remained download
                                        requests, and we are waiting for the update of metadata.
                                        Download should be resumed after metadata is updated. */
    int        waitCounter;        /*!< Counter of how many times we have retried to update the
                                        metadata actively. */
};


// Local functions ...................................................................................................

static nb_boolean CreateConnection(NB_EnhancedContentDownloadManager* pThis, Connection* connection);
static void DestroyConnection(NB_EnhancedContentDownloadManager* pThis, Connection* connection);

static NB_Error CreateDownloadQueues(DownloadQueue **queue);
static void DestroyDownloadQueues(DownloadQueue *queue);

static NB_EnhancedContentDownloadLevel EnhancedContentTypeToDownloadLevel(NB_EnhancedContentDataType type);

static DownloadQueue* FindDownloadQueueByType(Connection* connection,
                                              NB_EnhancedContentDataType type);
static CSL_Vector* FindPendingItemsByType(Connection* connection,
                                          NB_EnhancedContentDataType type);
static nb_boolean PreparePendingItemsForDownload(Connection* connection);

static NB_Error CancelRequests(Connection* connection, NB_EnhancedContentDataType dataType);
static NB_Error CancelRequestData(Connection* connection, const NB_EnhancedContentIdentifier* itemId);

static NB_Error PauseDownload(Connection* connection);
static NB_Error ResumeDownload(Connection* connection);

static nb_boolean CheckDuplicated(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentDownloadType downloadType, const NB_EnhancedContentIdentifier* itemId);
static NB_Error TranslateHttpPalErrorToNeError(PAL_Error palError);

// Callback function for network call
static void StatusCallback(void* userData, PAL_NetConnectionStatus status);
static void ReceiveCallback(void* userData, void* requestData, PAL_Error errorCode, const byte* bytes, uint32 count);
static void ResponseStatusCallback(PAL_Error errorCode, void* userData,
                                   void* requestData,uint32 contentLength);

// Allocate/Free helper functions
static RequestItem* AllocateRequestItem(NB_EnhancedContentDataType type, const char* dataset, const char* itemId, const char* fileVersion);
static void FreeRequestItem(RequestItem* requestItem);
static void FreePendingEntry(PendingEntry* entry);
static int FreePendingEntryCallback(void* userData, void* entry);

static void ProcessPendingItem(Connection* connection);
static void StartRequest(Connection* connection);

static char* GetBasePath(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentDataType type);
static NB_Error BuildItemFilePath(NB_EnhancedContentDownloadManager* pThis, const NB_EnhancedContentIdentifier* item, nb_boolean zipped, nb_boolean created, char** path);
static void SaveData(NB_EnhancedContentDownloadManager* pThis, RequestItem* item, const byte* buffer, uint32 size);
static NB_Error SaveToCache(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentIdentifier* item, const char* fileVersion, byte* buffer, uint32 size);
static boolean SaveToFile(PAL_Instance* pal,
                            const char* filename,
                            uint8* data,                /*!< Data to write */
                            int size,                   /*!< Size of data to write */
                            uint32 pos);                /*!< append the file form the position */
static void CacheRemovedNotificationCallback(CSL_Cache* cache, const byte* name, size_t namelen, void* userData);

static void QaLogEnhancedContentDownload(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentDownloadType downloadType, const NB_EnhancedContentIdentifier* itemId, const char* url);
static void QaLogEnhancedContentDownloadResult(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentDownloadType downloadType, const NB_EnhancedContentIdentifier* itemId, NB_Error result);
static NB_QaLogEnhancedContentType GetQaLogEnhancedContentType(NB_EnhancedContentDataType type);
static NB_QaLogEnhancedContentDownloadType GetQaLogEnhancedContentDownloadType(NB_EnhancedContentDownloadType downloadType);

// Public functions ..................................................................................................

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerCreate(NB_Context* context,
                                        const NB_EnhancedContentManagerConfiguration* configuration,
                                        NB_EnhancedContentDataSource* dataSource,
                                        const NB_EnhancedContentDownloadManagerCallbackData* callbackData,
                                        NB_EnhancedContentDownloadManager** instance)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;
    NB_EnhancedContentDownloadManager* pThis = NULL;

    if (!context || ! configuration || !dataSource || ! callbackData || !instance)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(context);

    if (nsl_strlen(configuration->texturesPath) == 0)
    {
        return NE_INVAL;
    }

    if (configuration->enhancedCityModelEnabled)
    {
        if (nsl_strlen(configuration->enhancedCityModelPath) == 0)
        {
            return NE_INVAL;
        }
    }

    if (configuration->mapRegionsEnabled)
    {
        if (nsl_strlen(configuration->mapRegionsPath) == 0)
        {
            return NE_INVAL;
        }
    }

    if(configuration->motorwayJunctionObjectsEnabled)
    {
        if ((configuration->maxMotorwayJunctionsCacheSize == 0) ||
            (nsl_strlen(configuration->motorwayJunctionObjectPath) == 0))
        {
            return NE_INVAL;
        }
    }

    if(configuration->realisticSignsEnabled)
    {
        if ((configuration->maxRealisticSignsCacheSize == 0) ||
            (nsl_strlen(configuration->realisticSignsPath) == 0))
        {
            return NE_INVAL;
        }
    }

    if(configuration->speedLimitsSignEnabled)
    {
        if (configuration->maxSpeedLimitImagesCacheSize == 0 ||
            (nsl_strlen(configuration->speedLimitsSignsPath) == 0))
        {
            return NE_INVAL;
        }
    }

    if (configuration->specialRegionsEnabled)
    {
        if (configuration->maxSpecialRegionsCacheSize == 0 ||
            (nsl_strlen(configuration->specialRegionsPath) == 0))
        {
            return NE_INVAL;
        }
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    // Copy parameters
    pThis->context = context;
    pThis->callbackData = *callbackData;
    pThis->dataSource = dataSource;

    // Copy data we need from the configuration
    nsl_strlcpy(pThis->enhancedCityModelPath,        configuration->enhancedCityModelPath,      sizeof(pThis->enhancedCityModelPath));
    nsl_strlcpy(pThis->texturesPath,                 configuration->texturesPath,               sizeof(pThis->texturesPath));
    nsl_strlcpy(pThis->regionsPath,                  configuration->mapRegionsPath,             sizeof(pThis->regionsPath));

    // If client guid provided, prepare additional header for send requests
    if (configuration->clientGuid[0])
    {
        nsl_strlcpy(pThis->additionalHeaders, CLIENT_GUID_HEADER,        sizeof(pThis->additionalHeaders));
        nsl_strlcat(pThis->additionalHeaders, configuration->clientGuid, sizeof(pThis->additionalHeaders));
        nsl_strlcat(pThis->additionalHeaders, HEADER_SEPARATOR,          sizeof(pThis->additionalHeaders));
    }

    // Create all connections
    if ((! CreateConnection(pThis, &pThis->preloadConnection)) ||
        (! CreateConnection(pThis, &pThis->onDemandConnection)))
    {
        NB_EnhancedContentDownloadManagerDestroy(pThis);
        return NE_NOMEM;
    }

    /* Create cache for SAR and MJO objects. We use the path as the cache name and we use the
       non-obfuscated cache so that the cache entries get saved using the cache entry name as
       file name. */
    if (configuration->realisticSignsEnabled)
    {
        nsl_strlcpy(pThis->realisticSignsPath, configuration->realisticSignsPath,
                    sizeof(pThis->realisticSignsPath));
        pThis->signsCache = CSL_CacheAlloc(pal,
                                           configuration->realisticSignsPath,
                                           MAX_MEMORY_CACHE_COUNT,
                                           configuration->maxRealisticSignsCacheSize,
                                           OBFUSCATE_KEY,
                                           sizeof(OBFUSCATE_KEY),
                                           NULL,
                                           FALSE);
        if (!pThis->signsCache)
        {
            NB_EnhancedContentDownloadManagerDestroy(pThis);
            return NE_NOMEM;
        }
        else
        {
            CSL_CacheLoadIndex(pThis->signsCache);
            result = CSL_CacheSetNotificationFunctions(pThis->signsCache,
                                                       CacheRemovedNotificationCallback, pThis);
            if (result != NE_OK)
            {
                NB_EnhancedContentDownloadManagerDestroy(pThis);
                return result;
            }
        }
    }

    if (configuration->motorwayJunctionObjectsEnabled)
    {
        nsl_strlcpy(pThis->motorwayJunctionObjectPath,
                    configuration->motorwayJunctionObjectPath,
                    sizeof(pThis->motorwayJunctionObjectPath));
        pThis->junctionsCache = CSL_CacheAlloc(pal,
                                               configuration->motorwayJunctionObjectPath,
                                               MAX_MEMORY_CACHE_COUNT,
                                               configuration->maxMotorwayJunctionsCacheSize,
                                               OBFUSCATE_KEY,
                                               sizeof(OBFUSCATE_KEY),
                                               NULL,
                                               FALSE);

        if (!pThis->junctionsCache)
        {
            NB_EnhancedContentDownloadManagerDestroy(pThis);
            return NE_NOMEM;
        }
        else
        {
            CSL_CacheLoadIndex(pThis->junctionsCache);
            result = CSL_CacheSetNotificationFunctions(pThis->junctionsCache,
                                                       CacheRemovedNotificationCallback, pThis);
            if (result != NE_OK)
            {
                NB_EnhancedContentDownloadManagerDestroy(pThis);
                return result;
            }
        }
    }

    if (configuration->speedLimitsSignEnabled)
    {
        nsl_strlcpy(pThis->speedLimitsSignsPath, configuration->speedLimitsSignsPath,
                    sizeof(pThis->speedLimitsSignsPath));
        PAL_FileCreateDirectoryEx(pal, configuration->speedLimitsSignsPath);
        pThis->speedLimitsSignCache = CSL_CacheAlloc(pal,
                                                     configuration->speedLimitsSignsPath,
                                                     MAX_MEMORY_CACHE_COUNT,
                                                     configuration->maxSpeedLimitImagesCacheSize,
                                                     OBFUSCATE_KEY,
                                                     sizeof(OBFUSCATE_KEY),
                                                     NULL,
                                                     FALSE);
        if (!pThis->speedLimitsSignCache)
        {
            NB_EnhancedContentDownloadManagerDestroy(pThis);
            return NE_NOMEM;
        }
        else
        {
            CSL_CacheLoadIndex(pThis->speedLimitsSignCache);
            result = CSL_CacheSetNotificationFunctions(pThis->speedLimitsSignCache,
                                                       CacheRemovedNotificationCallback, pThis);
            if (result != NE_OK)
            {
                NB_EnhancedContentDownloadManagerDestroy(pThis);
                return result;
            }
        }
    }

    if (configuration->specialRegionsEnabled)
    {
        nsl_strlcpy(pThis->specialRegionsPath, configuration->specialRegionsPath, sizeof(pThis->specialRegionsPath));
        PAL_FileCreateDirectoryEx(pal, configuration->specialRegionsPath);
        pThis->specialRegionsCache = CSL_CacheAlloc(pal,
                                                    configuration->specialRegionsPath,
                                                    MAX_MEMORY_CACHE_COUNT,
                                                    configuration->maxSpecialRegionsCacheSize,
                                                    OBFUSCATE_KEY,
                                                    sizeof(OBFUSCATE_KEY),
                                                    NULL,
                                                    FALSE);
        if (!pThis->specialRegionsCache)
        {
            NB_EnhancedContentDownloadManagerDestroy(pThis);
            return NE_NOMEM;
        }
        else
        {
            CSL_CacheLoadIndex(pThis->specialRegionsCache);
            result = CSL_CacheSetNotificationFunctions(pThis->specialRegionsCache,
                                                       CacheRemovedNotificationCallback, pThis);
            if (result != NE_OK)
            {
                NB_EnhancedContentDownloadManagerDestroy(pThis);
                return result;
            }
        }
    }


    pThis->waitingForMetadata = FALSE;
    pThis->waitCounter = 0;

    *instance = pThis;

    return NE_OK;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerDestroy(NB_EnhancedContentDownloadManager* instance)
{
    if (!instance)
    {
        // Not sure if we should treat this as error or not
        return NE_OK;
    }

    DestroyConnection(instance, &instance->preloadConnection);
    DestroyConnection(instance, &instance->onDemandConnection);

    if (instance->signsCache)
    {
        CSL_CacheDealloc(instance->signsCache);
    }
    if (instance->junctionsCache)
    {
        CSL_CacheDealloc(instance->junctionsCache);
    }
    if (instance->speedLimitsSignCache)
    {
        CSL_CacheDealloc(instance->speedLimitsSignCache);
    }
    if (instance->specialRegionsCache)
    {
        CSL_CacheDealloc(instance->specialRegionsCache);
    }
    // Don't destroy the data source, it is owned by the enhanced content manager.

    nsl_free(instance);
    return NE_OK;
}

/* See header for description */
//@todo This function and CheckDuplicated() could be simplified.
NB_Error
NB_EnhancedContentDownloadManagerRequestData(NB_EnhancedContentDownloadManager* instance,
                                             const NB_EnhancedContentIdentifier* itemId,
                                             const char* fileVerion,
                                             NB_EnhancedContentDownloadType requestType)
{
    Connection*  connection    = NULL;
    CSL_Vector*  pendingItems  = NULL;
    PendingEntry newEntry      = {0};
    const char*  tmpFileVerion = fileVerion;

    if (! instance || ! itemId || !itemId->dataset)
    {
        return NE_INVAL;
    }

    switch (requestType)
    {
        case NB_ECD0_OnDemand:
            connection = &instance->onDemandConnection;
            break;

        case NB_ECD0_Preload:
            connection = &instance->preloadConnection;
            break;

        default:
            return NE_INVAL;
    }

    pendingItems = FindPendingItemsByType(connection, itemId->type);
    if (!pendingItems)
    {
        return NE_UNEXPECTED;
    }

    // Add the data item to pending list.
    if (itemId->fileVersion)
    {
        tmpFileVerion = itemId->fileVersion;
    }

    if (tmpFileVerion)
    {
        newEntry.fileVersion = nsl_strdup(tmpFileVerion);
        if (! newEntry.fileVersion)
        {
            FreePendingEntry(&newEntry);
            return NE_NOMEM;
        }
    }

    newEntry.type = itemId->type;
    newEntry.dataset = nsl_strdup(itemId->dataset);
    if (! newEntry.dataset)
    {
        FreePendingEntry(&newEntry);
        return NE_NOMEM;
    }

    if (itemId->itemId)
    {
        if (CheckDuplicated(instance, requestType, itemId))
        {
            FreePendingEntry(&newEntry);

            // Requested item is already in pending queue.
            return NE_OK;
        }

        // Allocate memory for single char pointer
        newEntry.itemIdArray = nsl_malloc(sizeof(char*));
        if (! newEntry.itemIdArray)
        {
            FreePendingEntry(&newEntry);
            return NE_NOMEM;
        }

        // Add a single file.
        *newEntry.itemIdArray = nsl_strdup(itemId->itemId);
        if (! (*newEntry.itemIdArray))
        {
            FreePendingEntry(&newEntry);
            return NE_NOMEM;
        }

        newEntry.itemCount = 1;
    }
    else
    {
        NB_Error result = NE_OK;
        int n = 0;
        int k = 0;
        int length = 0;
        NB_EnhancedContentIdentifier* identifier = NULL;
        CSL_Vector* vectorIdentifiers = NULL;

        vectorIdentifiers = CSL_VectorAlloc(sizeof(NB_EnhancedContentIdentifier));
        if (!vectorIdentifiers)
        {
            FreePendingEntry(&newEntry);
            return NE_NOMEM;
        }

        result = NB_EnhancedContentDataSourceGetDataItemIDs(instance->dataSource, TRUE, itemId->type, itemId->dataset, vectorIdentifiers);
        if (result != NE_OK)
        {
            FreePendingEntry(&newEntry);
            CSL_VectorDealloc(vectorIdentifiers);
            return result;
        }

        length = CSL_VectorGetLength(vectorIdentifiers);
        if (length <= 0)
        {
            FreePendingEntry(&newEntry);
            CSL_VectorDealloc(vectorIdentifiers);
            return NE_NOENT;
        }

        // Textures haven't itemId.
        if (length == 1)
        {
            identifier = (NB_EnhancedContentIdentifier*) CSL_VectorGetPointer(vectorIdentifiers, 0);
            if (! identifier->itemId)
            {
                if (CheckDuplicated(instance, requestType, identifier))
                {
                    // Free memory in NB_EnhancedContentIdentifier.
                    NB_EnhancedContentIdentifierSet(identifier, NB_ECDT_None, NULL, NULL, NULL);
                    CSL_VectorDealloc(vectorIdentifiers);
                    FreePendingEntry(&newEntry);

                    // Requested item is already in pending queue.
                    return NE_OK;
                }

                // Free memory in NB_EnhancedContentIdentifier.
                NB_EnhancedContentIdentifierSet(identifier, NB_ECDT_None, NULL, NULL, NULL);
                CSL_VectorDealloc(vectorIdentifiers);

                // Add new entry to pending list.
                result = CSL_VectorAppend(pendingItems, &newEntry) ? NE_OK : NE_NOMEM;
                if (result == NE_OK)
                {
                    ProcessPendingItem(connection);
                    return NE_OK;
                }
                else
                {
                    FreePendingEntry(&newEntry);
                    return result;
                }
            }
        }

        newEntry.itemIdArray = nsl_malloc(length * sizeof(char*));
        if (! newEntry.itemIdArray)
        {
            FreePendingEntry(&newEntry);
            CSL_VectorDealloc(vectorIdentifiers);
            return NE_NOMEM;
        }
        newEntry.itemCount = length;

        k = 0;
        for (n = 0; n < length; ++n)
        {
            identifier = (NB_EnhancedContentIdentifier*) CSL_VectorGetPointer(vectorIdentifiers, n);

            if (! identifier->itemId)
            {
                FreePendingEntry(&newEntry);
                CSL_VectorDealloc(vectorIdentifiers);
                return NE_UNEXPECTED;
            }

            if (CheckDuplicated(instance, requestType, identifier))
            {
                --newEntry.itemCount;

                continue;
            }

            // Add the item.
            newEntry.itemIdArray[k] = nsl_strdup(identifier->itemId);
            if (! (newEntry.itemIdArray[k]))
            {
                FreePendingEntry(&newEntry);
                CSL_VectorDealloc(vectorIdentifiers);
                return NE_NOMEM;
            }

            // Free memory in NB_EnhancedContentIdentifier.
            NB_EnhancedContentIdentifierSet(identifier, NB_ECDT_None, NULL, NULL, NULL);

            ++k;
        }
        CSL_VectorDealloc(vectorIdentifiers);

        if (newEntry.itemCount != length)
        {
            if (newEntry.itemCount > 0)
            {
                newEntry.itemIdArray = nsl_realloc(newEntry.itemIdArray, newEntry.itemCount * sizeof(char*));
                if (! newEntry.itemIdArray)
                {
                    FreePendingEntry(&newEntry);
                    return NE_NOMEM;
                }
            }
            else
            {
                nsl_free(newEntry.itemIdArray);
                newEntry.itemIdArray = NULL;
                FreePendingEntry(&newEntry);

                // Requested item is already in pending queue.
                return NE_OK;
            }
        }
    }

    // Add new entry to pending list
    if (! CSL_VectorAppend(pendingItems, &newEntry))
    {
        FreePendingEntry(&newEntry);
        return NE_NOMEM;
    }
    // Process pending list
    ProcessPendingItem(connection);

    return NE_OK;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerRemoveData(NB_EnhancedContentDownloadManager* instance,
                                            const NB_EnhancedContentIdentifier* itemId,
                                            nb_boolean zipped)
{
    NB_Error result = NE_OK;
    char* path = NULL;
    PAL_Instance* pal = NULL;

    if (! instance || ! itemId || ! itemId->dataset)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(instance->context);

    switch (itemId->type)
    {
        case NB_ECDT_CityModel:
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            // Return full path for city model and textures.
            result = BuildItemFilePath(instance, itemId, zipped, FALSE, &path);
            if (result != NE_OK)
            {
                return result;
            }

            if (PAL_FileExists(pal, path) == PAL_Ok)
            {
                if (PAL_FileIsDirectory(pal, path))
                {
                    result = PAL_FileRemoveDirectory(pal, path, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
                }
                else
                {
                    result = PAL_FileRemove(pal, path) == PAL_Ok ? NE_OK : NE_FSYS;
                }
            }
            else
            {
                result = NE_NOENT;
            }

            nsl_free(path);

            break;
        }
        case NB_ECDT_MotorwayJunctionObject:
        {
            if (!instance->junctionsCache)
            {
                return NE_INVAL;
            }
            if (itemId->itemId)
            {
                // Return subpath for MJO.
                result = BuildItemFilePath(instance, itemId, zipped, FALSE, &path);
                if (result != NE_OK)
                {
                    return result;
                }

                result = CSL_CacheClearOne(instance->junctionsCache, (byte*)path, nsl_strlen(path));

                nsl_free(path);
            }
            else
            {
                result = CSL_CacheClear(instance->junctionsCache);
            }

            break;
        }
        case NB_ECDT_RealisticSign:
        {
            if (!instance->signsCache)
            {
                return NE_INVAL;
            }
            if (itemId->itemId)
            {
                // Return subpath for SAR.
                result = BuildItemFilePath(instance, itemId, zipped, FALSE, &path);
                if (result != NE_OK)
                {
                    return result;
                }

                result = CSL_CacheClearOne(instance->signsCache, (byte*)path, nsl_strlen(path));

                nsl_free(path);
            }
            else
            {
                result = CSL_CacheClear(instance->signsCache);
            }

            break;
        }
        case NB_ECDT_SpeedLimitsSign:
        {
            if (!instance->speedLimitsSignCache)
            {
                return NE_INVAL;
            }
            if (itemId->itemId)
            {
                // Return subpath for Speed Limits Images.
                result = BuildItemFilePath(instance, itemId, zipped, FALSE, &path);
                if (result != NE_OK)
                {
                    return result;
                }

                result = CSL_CacheClearOne(instance->speedLimitsSignCache,
                                           (byte*)path, nsl_strlen(path));
                if (result == NE_NOENT) // In this case, need to manually remove manifest entry.
                {
                    NB_EnhancedContentDataSourceRemoveDataItem(instance->dataSource,
                                                               itemId->type,
                                                               itemId->dataset,
                                                               itemId->itemId);
                }
                nsl_free(path);
            }
            else
            {
                result = CSL_CacheClear(instance->speedLimitsSignCache);
            }

            break;
        }
        case NB_ECDT_SpecialRegions:
        {
            if (!instance->specialRegionsCache)
            {
                return NE_INVAL;
            }
            if (itemId->itemId)
            {
                // Return subpath for spcial region images.
                result = BuildItemFilePath(instance, itemId, zipped, FALSE, &path);
                if (result != NE_OK)
                {
                    return result;
                }

                result = CSL_CacheClearOne(instance->specialRegionsCache, (byte*)path, nsl_strlen(path));
                if (result == NE_NOENT) // In this case, need to manually remove manifest entry.
                {
                    NB_EnhancedContentDataSourceRemoveDataItem(instance->dataSource, itemId->type, itemId->dataset, itemId->itemId);
                }
                nsl_free(path);
            }
            else
            {
                result = CSL_CacheClear(instance->specialRegionsCache);
            }
        }
        default:
        {
            break;
        }
    }

    return result;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerCancelRequests(NB_EnhancedContentDownloadManager* instance,
                                                NB_EnhancedContentDownloadType requestType,
                                                NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;

    if (! instance)
    {
        return NE_INVAL;
    }

    switch (requestType)
    {
        case NB_ECDO_None:
        {
            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = CancelRequests(&instance->onDemandConnection, dataType);
            break;
        }
        case NB_ECD0_Preload:
        {
            result = CancelRequests(&instance->preloadConnection, dataType);
            break;
        }
        case NB_ECDO_All:
        {
            result = result ? result : CancelRequests(&instance->onDemandConnection, dataType);
            result = result ? result : CancelRequests(&instance->preloadConnection, dataType);
            break;
        }
    }

    return result;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerCancelRequestData(NB_EnhancedContentDownloadManager* instance,
                                                   NB_EnhancedContentDownloadType requestType,
                                                   const NB_EnhancedContentIdentifier* itemId)
{
    NB_Error result = NE_OK;

    if (! instance || ! itemId || ! itemId->dataset)
    {
        return NE_INVAL;
    }

    switch (requestType)
    {
        case NB_ECDO_None:
        {
            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = CancelRequestData(&instance->onDemandConnection, itemId);
            break;
        }
        case NB_ECD0_Preload:
        {
            result = CancelRequestData(&instance->preloadConnection, itemId);
            break;
        }
        case NB_ECDO_All:
        {
            result = result ? result : CancelRequestData(&instance->onDemandConnection, itemId);
            result = result ? result : CancelRequestData(&instance->preloadConnection, itemId);
            break;
        }
    }

    return result;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerPauseDownload(NB_EnhancedContentDownloadManager* instance,
                                               NB_EnhancedContentDownloadType requestType)
{
    NB_Error result = NE_OK;

    if (! instance)
    {
        return NE_INVAL;
    }

    switch (requestType)
    {
        case NB_ECDO_None:
        {
            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = PauseDownload(&instance->onDemandConnection);
            break;
        }
        case NB_ECD0_Preload:
        {
            result = PauseDownload(&instance->preloadConnection);
            break;
        }
        case NB_ECDO_All:
        {
            result = result ? result : PauseDownload(&instance->onDemandConnection);
            result = result ? result : PauseDownload(&instance->preloadConnection);
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerResumeDownload(NB_EnhancedContentDownloadManager* instance,
                                                NB_EnhancedContentDownloadType requestType)
{
    NB_Error result = NE_OK;

    if (! instance)
    {
        return NE_INVAL;
    }

    switch (requestType)
    {
        case NB_ECDO_None:
        {
            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = ResumeDownload(&instance->onDemandConnection);
            break;
        }
        case NB_ECD0_Preload:
        {
            result = ResumeDownload(&instance->preloadConnection);
            break;
        }
        case NB_ECDO_All:
        {
            result = result ? result : ResumeDownload(&instance->onDemandConnection);
            result = result ? result : ResumeDownload(&instance->preloadConnection);
            break;
        }
        default:
        {
            break;
        }
    }

    return NE_OK;
}

/* See header for description */
nb_boolean
NB_EnhancedContentDownloadManagerIsPending(NB_EnhancedContentDownloadManager* instance,
                                           NB_EnhancedContentDownloadType requestType)
{
    nb_boolean result = FALSE;

    if (!instance)
    {
        return FALSE;
    }

    switch (requestType)
    {
        case NB_ECDO_None:
        {
            result = FALSE;
            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = (nb_boolean) (CSL_VectorGetLength(instance->onDemandConnection.pendingItems) > 0);
            break;
        }
        case NB_ECD0_Preload:
        {
            result = (nb_boolean) (CSL_VectorGetLength(instance->preloadConnection.pendingItems) > 0);
            break;
        }
        case NB_ECDO_All:
        {
            result = (nb_boolean) (CSL_VectorGetLength(instance->onDemandConnection.pendingItems) > 0 &&
                                   CSL_VectorGetLength(instance->preloadConnection.pendingItems) > 0);
        }
        default:
        {
            break;
        }
    }

    return result;
}

/* See header for description */
nb_boolean
NB_EnhancedContentDownloadManagerIsPaused(NB_EnhancedContentDownloadManager* instance,
                                          NB_EnhancedContentDownloadType requestType)
{
    nb_boolean result = FALSE;

    if (!instance)
    {
        return FALSE;
    }

    switch (requestType)
    {
        case NB_ECDO_None:
        {
            result = FALSE;
            break;
        }
        case NB_ECD0_OnDemand:
        {
            result = (nb_boolean) (! instance->onDemandConnection.httpConnection);
            break;
        }
        case NB_ECD0_Preload:
        {
            result = (nb_boolean) (! instance->preloadConnection.httpConnection);
            break;
        }
        case NB_ECDO_All:
        {
            result = (nb_boolean) (! instance->onDemandConnection.httpConnection &&
                                   ! instance->preloadConnection.httpConnection);
        }
        default:
        {
            break;
        }
    }

    return result;
}

/*! Reset status of download queue.

    @return NB_OK if succeeded
*/
NB_Error
ToggleDownloadQueueByLevel(Connection* connection,                  /*!< Connection instance */
                           NB_EnhancedContentDownloadLevel level,   /*!< Level of queue to reset */
                           nb_boolean enabled)                      /*!< Target status */
{
    NB_Error result = NE_OK;
    if (connection && connection->pendingQueues)
    {
        DownloadQueue* queue = NULL;
        switch (level)
        {
            case NB_ECDL_Low:
            case NB_ECDL_Middle:
            case NB_ECDL_Heigh:
            {
                // Pause download if item being processed belongs to this leve.
                if (!enabled && connection->inProgress && connection->requestItem)
                {
                    if (EnhancedContentTypeToDownloadLevel(connection->requestItem->item->type)
                        == level)
                    {
                        result = PauseDownload(connection);
                        result = result ? result : ResumeDownload(connection);
                    }
                }
                if (result != NE_OK)
                {
                    return result;
                }
                queue = connection->pendingQueues + (uint32)level;
                if (queue)
                {
                    queue->enabled = enabled;
                    result = NE_OK;
                }
                break;
            }
            case NB_ECDL_All: // XXX: we can also do this through for();
            {
                result  = ToggleDownloadQueueByLevel(connection, NB_ECDL_Low, enabled);
                result |= ToggleDownloadQueueByLevel(connection, NB_ECDL_Middle, enabled);
                result |= ToggleDownloadQueueByLevel(connection, NB_ECDL_Heigh, enabled);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return result;
}

/* Refer to header file for description*/
NB_Error
DisableDownloadQueueByLevel(Connection* connection,
                            NB_EnhancedContentDownloadLevel level)
{
    return ToggleDownloadQueueByLevel(connection, level, FALSE);
}

/* Refer to header file for description*/
NB_Error
EnableDownloadQueueByLevel(Connection* connection,
                            NB_EnhancedContentDownloadLevel level)
{
    return ToggleDownloadQueueByLevel(connection, level, TRUE);
}

/* Refer to header file for description*/
NB_Error
NB_EnhancedContentDownloadManagerDisableLevel(NB_EnhancedContentDownloadManager* instance,
                                              NB_EnhancedContentDownloadType requestType,
                                              NB_EnhancedContentDownloadLevel    level)
{
    NB_Error result = NE_INVAL;
    if (instance)
    {
        switch (requestType)
        {
            case NB_ECDO_None:
            {
                result = FALSE;
                break;
            }
            case NB_ECD0_OnDemand:
            {
                result = DisableDownloadQueueByLevel(&(instance->onDemandConnection), level);
                break;
            }
            case NB_ECD0_Preload:
            {
                result = DisableDownloadQueueByLevel(&(instance->preloadConnection), level);
                break;
            }
            case NB_ECDO_All:
            {
                result = DisableDownloadQueueByLevel(&(instance->onDemandConnection), level);
                result = result ? result :
                         DisableDownloadQueueByLevel(&(instance->preloadConnection), level);
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return result;
}

/* Refer to header file for description*/
NB_Error
NB_EnhancedContentDownloadManagerEnableLevel(NB_EnhancedContentDownloadManager* instance,
                                              NB_EnhancedContentDownloadType requestType,
                                              NB_EnhancedContentDownloadLevel    level)
{
    NB_Error result = NE_INVAL;
    if (instance)
    {
        switch (requestType)
        {
            case NB_ECDO_None:
            {
                result = FALSE;
                break;
            }
            case NB_ECD0_OnDemand:
            {
                result = EnableDownloadQueueByLevel(&(instance->onDemandConnection), level);
                break;
            }
            case NB_ECD0_Preload:
            {
                result = EnableDownloadQueueByLevel(&(instance->preloadConnection), level);
                break;
            }
            case NB_ECDO_All:
            {
                result = EnableDownloadQueueByLevel(&(instance->onDemandConnection), level);
                result = result ? result :
                         EnableDownloadQueueByLevel(&(instance->preloadConnection), level);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return result;
}

/*! Helper function to check if a queue in connection is enabled.

    @return TRUE if enabled.
*/
nb_boolean
IsQueueLevelOfConnectionEnabled(Connection* connection,               /*!< Connection instance */
                                NB_EnhancedContentDownloadLevel level /*!< level of download quue */
                                )
{
    nb_boolean result = FALSE;
    if (connection && connection->pendingQueues &&
        /*level >= 0 && */level < NB_ECDL_All)
    {
        DownloadQueue* queue = connection->pendingQueues + level;
        if (queue)
        {
            result = queue->enabled;
        }
    }
    return result;
}

/* Refer to header file for description*/
nb_boolean
NB_EnhancedContentDownloadManagerIsLevelEnabled(NB_EnhancedContentDownloadManager* instance,
                                                NB_EnhancedContentDownloadType requestType,
                                                NB_EnhancedContentDownloadLevel    level)
{
    nb_boolean result = FALSE;
    if (instance)
    {
        switch (requestType)
        {
            case NB_ECDO_None:
            {
                result = FALSE;
                break;
            }
            case NB_ECD0_OnDemand:
            {
                result = IsQueueLevelOfConnectionEnabled(&instance->onDemandConnection, level);
                break;
            }
            case NB_ECD0_Preload:
            {
                result = IsQueueLevelOfConnectionEnabled(&instance->preloadConnection, level);
                break;
            }
            case NB_ECDO_All:
            {
                result = IsQueueLevelOfConnectionEnabled(&instance->preloadConnection, level) &&
                         IsQueueLevelOfConnectionEnabled(&instance->onDemandConnection, level);
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return result;
}

/*! Finds corresponding CSL_Cache based on data type.

    @return The found cache, or NULL if unsupported.
*/
CSL_Cache*
FindCacheByDataType(NB_EnhancedContentDownloadManager* instance,    /*! Download Manager Instance*/
                    NB_EnhancedContentDataType type                 /*!< Data type */
                    )
{
    switch (type)
    {
        case NB_ECDT_RealisticSign:
        {
            return instance->signsCache;
        }
        case NB_ECDT_MotorwayJunctionObject:
        {
            return instance->junctionsCache;
        }
        case NB_ECDT_SpeedLimitsSign:
        {
            return instance->speedLimitsSignCache;
        }
        case NB_ECDT_SpecialRegions:
        {
            return instance->specialRegionsCache;
        }
        default:
        {
            return NULL;
        }
    }
}

/* See header for description */
nb_boolean
NB_EnhancedContentDownloadManagerIsDataItemAvailable(NB_EnhancedContentDownloadManager* instance,
                                                     const NB_EnhancedContentIdentifier* itemId)
{
    nb_boolean result = FALSE;
    NB_Error error = NE_OK;
    char* path = NULL;
    PAL_Instance* pal = NULL;

    if (! instance || ! itemId || ! itemId->dataset)
    {
        return FALSE;
    }

    pal = NB_ContextGetPal(instance->context);

    // Return full path for city model and textures or subpath for MJO and SAR.
    error = BuildItemFilePath(instance, itemId, FALSE, FALSE, &path);
    if (error != NE_OK)
    {
        return FALSE;
    }

    switch (itemId->type)
    {
        case NB_ECDT_CityModel:
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            result = (nb_boolean) (PAL_FileExists(pal, path) == PAL_Ok);

            break;
        }
        case NB_ECDT_RealisticSign:
        case NB_ECDT_MotorwayJunctionObject:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        {
            CSL_Cache* cache = FindCacheByDataType(instance, itemId->type);
            if (cache)
            {
                result = CSL_CacheIsItemInCache(cache, (byte*)path, nsl_strlen(path));
            }
            break;
        }
        default:
        {
            break;
        }
    }

    nsl_free(path);

    return result;
}

/* See header for description */
// @todo: Priorities of download requests: priority differs between SAR and SpeedLimitsSigns,
//        but it is not supported by the current download manager.
NB_Error
NB_EnhancedContentDownloadManagerGetDataItem(NB_EnhancedContentDownloadManager* instance,
                                             const NB_EnhancedContentIdentifier* itemId,
                                             uint32* dataSize,
                                             uint8** data)
{
    NB_Error result = NE_INVAL;
    char* path = NULL;
    PAL_Instance* pal = NULL;

    if (! instance || ! itemId || ! itemId->dataset || ! itemId->itemId || ! dataSize || ! data)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(instance->context);

    // Return full path for city model and textures or subpath for MJO and SAR.
    result = BuildItemFilePath(instance, itemId, FALSE, FALSE, &path);
    if (result != NE_OK)
    {
        return result;
    }

    switch (itemId->type)
    {
        case NB_ECDT_CityModel:
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            if (PAL_FileExists(pal, path) == PAL_Ok && !PAL_FileIsDirectory(pal, path))
            {
                result = PAL_FileLoadFile(pal, path, data, dataSize) == PAL_Ok ? NE_OK : NE_FSYS;
            }
            else
            {
                result = NE_NOENT;
            }

            break;
        }
        case NB_ECDT_RealisticSign:
        case NB_ECDT_MotorwayJunctionObject:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        {
            CSL_Cache* cache = FindCacheByDataType(instance, itemId->type);
            size_t tDataSize = (size_t)(*dataSize);
            if (cache)
            {
                result = CSL_CacheFind(cache, (byte*)path, nsl_strlen(path), data,
                                       &tDataSize, FALSE, TRUE, FALSE);
                *dataSize = (uint32)tDataSize;
            }
            break;
        }
        default:
        {
            result = NE_INVAL;
            break;
        }
    }

    nsl_free(path);

    return result;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerDoStorageConsistencyCheck(NB_EnhancedContentDownloadManager* instance)
{
    return NE_NOSUPPORT;
}

/* See header for description */
NB_Error
NB_EnhancedContentDownloadManagerMasterClear(NB_EnhancedContentDownloadManager* instance)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;

    if (! instance)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(instance->context);

    result = result ? result : NB_EnhancedContentDownloadManagerCancelRequests(instance, NB_ECDO_All, NB_ECDT_All);

    if (instance->signsCache != NULL)
    {
        result = result ? result : CSL_CacheClear(instance->signsCache);
    }

    if (instance->junctionsCache != NULL)
    {
        result = result ? result : CSL_CacheClear(instance->junctionsCache);
    }

    if (instance->speedLimitsSignCache != NULL)
    {
        result = result ? result : CSL_CacheClear(instance->speedLimitsSignCache);
    }

    if (instance->specialRegionsCache != NULL)
    {
        result = result ? result : CSL_CacheClear(instance->specialRegionsCache);
    }

    if (result != NE_OK)
    {
        return result;
    }

    if ((result == NE_OK) &&
        (nsl_strlen(instance->enhancedCityModelPath) > 0) &&
        (PAL_FileExists(pal, instance->enhancedCityModelPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->enhancedCityModelPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }
    if ((result == NE_OK) &&
        (nsl_strlen(instance->regionsPath) > 0) &&
        (PAL_FileExists(pal, instance->regionsPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->regionsPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }
    if ((result == NE_OK) &&
        (nsl_strlen(instance->motorwayJunctionObjectPath) > 0) &&
        (PAL_FileExists(pal, instance->motorwayJunctionObjectPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->motorwayJunctionObjectPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }
    if ((result == NE_OK) &&
        (nsl_strlen(instance->texturesPath) > 0) &&
        (PAL_FileExists(pal, instance->texturesPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->texturesPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }
    if ((result == NE_OK) &&
        (nsl_strlen(instance->realisticSignsPath) > 0) &&
        (PAL_FileExists(pal, instance->realisticSignsPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->realisticSignsPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }
    if ((result == NE_OK) &&
        (nsl_strlen(instance->speedLimitsSignsPath) > 0) &&
        (PAL_FileExists(pal, instance->speedLimitsSignsPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->speedLimitsSignsPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }
    if ((result == NE_OK) &&
        (nsl_strlen(instance->specialRegionsPath) > 0) &&
        (PAL_FileExists(pal, instance->specialRegionsPath) == PAL_Ok))
    {
        result = PAL_FileRemoveDirectory(pal, instance->specialRegionsPath, TRUE) == PAL_Ok ? NE_OK : NE_FSYS;
    }

    return result;
}

nb_boolean
NB_EnhancedContentDownloadManagerIsWaitingMetadata(NB_EnhancedContentDownloadManager* instance)
{
    if (!instance)
    {
        return FALSE;
    }

    return (instance->waitingForMetadata &&
            NB_EnhancedContentDownloadManagerIsPaused(instance, NB_ECD0_OnDemand));
}



NB_Error
NB_EnhancedContentDownloadManagerSetWaitingMetadata(NB_EnhancedContentDownloadManager* manager,
                                                    nb_boolean waiting)
{
    if (!manager)
    {
        return NE_INVAL;
    }
    if (waiting)
    {
        NB_EnhancedContentDownloadManagerPauseDownload(manager, NB_ECD0_OnDemand);
        manager->waitingForMetadata = TRUE;
    }
    else
    {
        NB_EnhancedContentDownloadManagerResumeDownload(manager, NB_ECD0_OnDemand);
        manager->waitingForMetadata = FALSE;
    }
    return NE_OK;
}

// Local functions ...................................................................................................

/*! Create a connection object including its pending queue.

    @return TRUE on success, FALSE otherwise.
*/
nb_boolean
CreateConnection(NB_EnhancedContentDownloadManager* pThis, Connection* connection)
{
    PAL_Error     palResult = PAL_Ok;
    NB_Error      error     = NE_OK;
    PAL_Instance* pal       = NULL;

    if (! pThis || ! connection)
    {
        return FALSE;
    }

    pal = NB_ContextGetPal(pThis->context);

    // Create HTTP network connection
    palResult = PAL_NetCreateConnection(pal, PNCP_HTTP, &(connection->httpConnection));
    if (palResult != PAL_Ok)
    {
        return FALSE;
    }

    // Create download queue of different levels.

    error = CreateDownloadQueues(&(connection->pendingQueues));
    if (error)
    {
        return FALSE;
    }

    connection->pendingItems = connection->pendingQueues->pendingItems;
    connection->requestItem = NULL;
    connection->inProgress = FALSE;
    connection->retryTimes = 0;

    connection->manager = pThis;

    return TRUE;
}

/*! Destroy a connection object, including its pending queue.

    @return None
*/
void
DestroyConnection(NB_EnhancedContentDownloadManager* pThis, Connection* connection)
{
    if (! connection)
    {
        return;
    }

    connection->retryTimes = 0;

    if (connection->httpConnection)
    {
        PAL_NetConnection* httpConnection = connection->httpConnection;

        connection->httpConnection = NULL;

        // Destroy HTTP network connection
        PAL_NetDestroyConnection(httpConnection);
    }

    if (connection->inProgress)
    {
        connection->inProgress = FALSE;

        // Remove downloading file in progress.
        if (CSL_VectorGetLength(connection->pendingItems) > 0)
        {
            char* itemId = NULL;
            NB_EnhancedContentIdentifier* identifier = NULL;
            PendingEntry* entry = CSL_VectorGetPointer(connection->pendingItems, 0);

            if (entry->itemIdArray)
            {
                nsl_assert(entry->nextItemIndex < entry->itemCount);

                itemId = entry->itemIdArray[entry->nextItemIndex];
            }

            identifier = NB_EnhancedContentIdentifierCreate(entry->type, entry->dataset,
                                                            itemId, entry->fileVersion);
            if (! identifier)
            {
                // Ignore error.
                return;
            }

            // Ignore error.
            NB_EnhancedContentDownloadManagerRemoveData(connection->manager, identifier, TRUE);
            NB_EnhancedContentIdentifierDestroy(identifier);
        }
    }

    connection->pendingItems = NULL;

    if (connection->pendingQueues)
    {
        DestroyDownloadQueues(connection->pendingQueues);
    }

    if (connection->requestItem)
    {
        FreeRequestItem(connection->requestItem);

        connection->requestItem = NULL;
    }
}

/*! Creates a new download queue.

    @return NB_OK if succeeded.
*/
NB_Error
CreateDownloadQueues(DownloadQueue **queues /*!< download queue instance */
                     )
{
    DownloadQueue* pThis   = NULL;
    DownloadQueue* pointer = NULL;
    uint32 i    = 0;
    uint32 size = ((uint32) NB_ECDL_All) * sizeof(DownloadQueue);

    if (!queues)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(size);
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, size);


    // Assign this before allocate other variable to make vectors can always be checked.
    *queues = pThis;

    pointer = pThis;
    size = sizeof(PendingEntry);
    for (i = 0; i < (int)NB_ECDL_All; i++)
    {
        pointer->pendingItems = CSL_VectorAlloc(size);
        if (!pThis->pendingItems)
        {
            return NE_NOMEM;;
        }

        pointer->level   = (NB_EnhancedContentDownloadLevel)i;
        pointer->enabled = TRUE;

        pointer ++;
    }
    return NE_OK;
}

/*! Destroy a download queue.

    @return none.
*/
void
DestroyDownloadQueues(DownloadQueue *queues	/*!<  queue to be destroyed. */
                      )
{
    if (queues)
    {
        uint32 i;
        DownloadQueue* pointer = queues;
        for (i = 0; i < (int)NB_ECDL_All; i++)
        {
            if (pointer->pendingItems)
            {
                // We have to go through the vector and delete all item arrays.
                CSL_VectorForEach(pointer->pendingItems, &FreePendingEntryCallback, NULL);

                // Free vector itself
                CSL_VectorDealloc(pointer->pendingItems);

                pointer->pendingItems = NULL;
            }
            pointer ++;
        }
        nsl_free(queues);
    }
}

NB_EnhancedContentDownloadLevel
EnhancedContentTypeToDownloadLevel(NB_EnhancedContentDataType type)
{
    NB_EnhancedContentDownloadLevel level = NB_ECDL_Low;
    switch (type)
    {
        case NB_ECDT_SpeedLimitsSign:
        {
            level = NB_ECDL_Heigh;
            break;
        }
        case NB_ECDT_RealisticSign:
        {
            level = NB_ECDL_Middle;
            break;
        }
        default:
        {
            level = NB_ECDL_Low;
            break;
        }
    }
    return level;
}

//XXX: type should not be: NB_ECDT_ALL!!!
DownloadQueue*
FindDownloadQueueByType(Connection* connection,
                        NB_EnhancedContentDataType type)
{
    DownloadQueue* queue = NULL;
    if (connection && connection->pendingQueues)
    {
        NB_EnhancedContentDownloadLevel level = EnhancedContentTypeToDownloadLevel(type);
        queue = connection->pendingQueues + ((uint32) level);
    }

    return queue;
}

//XXX: type should not be: NB_ECDT_ALL!!!
CSL_Vector*
FindPendingItemsByType(Connection* connection,
                       NB_EnhancedContentDataType type)
{
    CSL_Vector* pendingItems = NULL;
    DownloadQueue* queue = FindDownloadQueueByType(connection, type);
    if (queue)
    {
        pendingItems = queue->pendingItems;
    }

    return pendingItems;
}





/*! Cancel requests of data type.

    @return NB_Error
*/
NB_Error
CancelRequests(Connection* connection, NB_EnhancedContentDataType dataType)
{
    NB_Error result = NE_OK;
    nb_boolean inProgress = FALSE;

    if (! connection)
    {
        return NE_INVAL;
    }

    switch (dataType)
    {
        case NB_ECDT_CityModel:
        case NB_ECDT_MotorwayJunctionObject:
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_RealisticSign:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        case NB_ECDT_MapRegionsCommon:
        case NB_ECDT_MapRegions:
        {
            int length = 0;
            int n = 0;

            PendingEntry*  entry        = NULL;

            CSL_Vector*    pendingItems = NULL;
            pendingItems = FindPendingItemsByType(connection, dataType);
            if (!pendingItems) //XXX: Should not happen, find a way to handle this.
            {
                return NE_UNEXPECTED;
            }

            length = CSL_VectorGetLength(pendingItems);
            if (length <= 0)
            {
                break;
            }

            for (n = length - 1; n > 0; --n)
            {
                entry = (PendingEntry*) CSL_VectorGetPointer(pendingItems, n);
                if (entry->type == dataType)
                {
                    FreePendingEntry(entry);
                    CSL_VectorRemove(pendingItems, n);
                }
            }

            entry = (PendingEntry*) CSL_VectorGetPointer(pendingItems, 0);
            if (entry->type == dataType)
            {
                inProgress = connection->inProgress;
                if (inProgress)
                {
                    result = result ? result : PauseDownload(connection);
                }

                FreePendingEntry(entry);
                CSL_VectorRemove(pendingItems, 0);

                if (inProgress)
                {
                    result = result ? result : ResumeDownload(connection);
                }
            }
            break;
        }
        case NB_ECDT_All:
        {
            DownloadQueue* queue = connection->pendingQueues;
            uint32 i = 0;

            if (!queue)
            {
                return NE_UNEXPECTED;
            }

            inProgress = connection->inProgress;
            if (inProgress)
            {
                result = result ? result : PauseDownload(connection);
            }

            for (i = 0; i < (uint32)NB_ECDL_All; i++) {
                CSL_VectorForEach(queue->pendingItems, &FreePendingEntryCallback, NULL);
                CSL_VectorRemoveAll(queue->pendingItems);

                queue ++;
            }

            // We have to go through the vector and delete all item arrays.
            if (inProgress)
            {
                result = result ? result : ResumeDownload(connection);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

/*! Cancel request of item data.

    @return NB_Error
*/
NB_Error
CancelRequestData(Connection* connection, const NB_EnhancedContentIdentifier* itemId)
{
    NB_Error result = NE_OK;
    nb_boolean inProgress = FALSE;
    int length = 0;
    int n = 0;
    int i = 0;
    PendingEntry* entry = NULL;
    CSL_Vector* pendingItems = NULL;

    if (! connection || ! itemId || ! itemId->dataset)
    {
        return NE_INVAL;
    }

    pendingItems = FindPendingItemsByType(connection, itemId->type);
    if (!pendingItems) //XXX: fatal error, handle this!
    {
        return NE_UNEXPECTED;
    }

    length = CSL_VectorGetLength(pendingItems);
    if (length <= 0)
    {
        return NE_OK;
    }

    for (n = length - 1; n > 0; --n)
    {
        entry = (PendingEntry*) CSL_VectorGetPointer(pendingItems, n);
        if (entry->type != itemId->type || nsl_strcmp(entry->dataset, itemId->dataset) != 0)
        {
            continue;
        }

        if (itemId->itemId)
        {
            for (i = entry->itemCount - 1; i >= entry->nextItemIndex; --i)
            {
                // Remove request entry from pending list when itemId is identical and the
                // fileVersion meets one of the following condition:
                //     1). fileVersion of Input entry is NULL, or
                //     2). both fileVersion of Input entry and pending entry are not NULL, and
                //         they are identical.
                if ((nsl_strcmp(itemId->itemId, entry->itemIdArray[i]) == 0) &&
                    ((itemId->fileVersion == NULL) ||
                     (entry->fileVersion != NULL && itemId->fileVersion != NULL &&
                      nsl_strcmp(itemId->fileVersion, entry->fileVersion)) == 0))
                {
                    nsl_free(entry->itemIdArray[i]);
                    nsl_memmove(entry->itemIdArray + i, entry->itemIdArray + i + 1, (entry->itemCount - i - 1) * sizeof(char*));
                    --(entry->itemCount);
                    if (entry->itemCount > 0)
                    {
                        entry->itemIdArray = nsl_realloc(entry->itemIdArray, entry->itemCount * sizeof(char*));
                    }
                    else
                    {
                        FreePendingEntry(entry);
                        CSL_VectorRemove(pendingItems, n);
                    }

                    return NE_OK;
                }
            }
        }
        else
        {
            FreePendingEntry(entry);
            CSL_VectorRemove(pendingItems, n);
        }
    }

    entry = (PendingEntry*) CSL_VectorGetPointer(pendingItems, 0);
    if (entry->type == itemId->type && nsl_strcmp(entry->dataset, itemId->dataset) == 0)
    {
        // Local variable inProgress indicates whether the last entry in this pending queue is
        // being processed now. It is set as TRUE when:
        //    1. Current connection is busy (ie, there is some request under going).
        //    2. Current connection's pendingItems points to the list where the last entry lays.

        inProgress = (nb_boolean)(connection->inProgress && pendingItems == connection->pendingItems);

        if (itemId->itemId)
        {
            for (i = entry->itemCount - 1; i >= entry->nextItemIndex; --i)
            {
                if (nsl_strcmp(itemId->itemId, entry->itemIdArray[i]) == 0)
                {
                    inProgress = (nb_boolean) (inProgress && i == entry->nextItemIndex);

                    if (inProgress)
                    {
                        result = result ? result : PauseDownload(connection);
                    }

                    nsl_free(entry->itemIdArray[i]);
                    nsl_memmove(entry->itemIdArray + i, entry->itemIdArray + i + 1, (entry->itemCount - i - 1) * sizeof(char*));
                    --(entry->itemCount);
                    if (entry->itemCount > 0)
                    {
                        entry->itemIdArray = nsl_realloc(entry->itemIdArray, entry->itemCount * sizeof(char*));
                    }
                    else
                    {
                        FreePendingEntry(entry);
                        CSL_VectorRemove(connection->pendingItems, n);
                    }

                    if (inProgress)
                    {
                        result = result ? result : ResumeDownload(connection);
                    }

                    return result;
                }
            }
        }
        else
        {
            if (inProgress)
            {
                result = result ? result : PauseDownload(connection);
            }

            FreePendingEntry(entry);
            CSL_VectorRemove(connection->pendingItems, 0);

            if (inProgress)
            {
                result = result ? result : ResumeDownload(connection);
            }
        }
    }

    return result;
}

/*! Pause download of the connection.

    @todo: Destroy HTTP connection for pausing currently. It's more clear and safe than PAL_NetCloseConnection.

    @return NB_Error
*/
NB_Error
PauseDownload(Connection* connection)
{
    NB_Error result = NE_OK;

    if (! connection)
    {
        return NE_INVAL;
    }

    connection->retryTimes = 0;

    if (connection->httpConnection)
    {
        PAL_NetConnection* httpConnection = connection->httpConnection;

        connection->httpConnection = NULL;

        // Destroy HTTP network connection
        PAL_NetDestroyConnection(httpConnection);
    }

    if (connection->inProgress)
    {
        connection->inProgress = FALSE;

        // Remove downloading file in progress.
        if (CSL_VectorGetLength(connection->pendingItems) > 0)
        {
            char* itemId = NULL;
            NB_EnhancedContentIdentifier* identifier = NULL;
            PendingEntry* entry = CSL_VectorGetPointer(connection->pendingItems, 0);

            if (entry->itemIdArray)
            {
                nsl_assert(entry->nextItemIndex < entry->itemCount);

                itemId = entry->itemIdArray[entry->nextItemIndex];
            }

            identifier = NB_EnhancedContentIdentifierCreate(entry->type, entry->dataset, itemId,
                                                            entry->fileVersion);
            if (! identifier)
            {
                return NE_NOMEM;
            }

            result = NB_EnhancedContentDownloadManagerRemoveData(connection->manager, identifier, TRUE);
            // Ignore NE_NOENT error.
            result = result == NE_NOENT ? NE_OK : result;

            NB_EnhancedContentIdentifierDestroy(identifier);
        }
    }

    if (connection->requestItem)
    {
        FreeRequestItem(connection->requestItem);

        connection->requestItem = NULL;
    }

    return result;
}

/*! Clean up download queues and choose next queue to download.

    @return TRUE if everything is OK, the caller can start the download;
            FALSE if exception occurred, caller should not begin the download.
*/
nb_boolean PreparePendingItemsForDownload(Connection* connection)
{
    nb_boolean result = FALSE;
    if (connection && connection->pendingQueues)
    {
        uint32 i = 0;
        DownloadQueue* queue = connection->pendingQueues;
        for (i = 0; i < (uint32)NB_ECDL_All; i++)
        {
            if (queue->enabled && queue->pendingItems &&
                CSL_VectorGetLength(queue->pendingItems) > 0)
            {
                connection->pendingItems = queue->pendingItems;
                result = TRUE;
                break;
            }
            queue ++;
        }
    }

    return result;
}

/*! Resume download of the connection.

    @return NB_Error
*/
NB_Error
ResumeDownload(Connection* connection)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;

    if (! connection)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(connection->manager->context);

    if (! connection->httpConnection)
    {
        // Create HTTP network connection
        result = PAL_NetCreateConnection(pal, PNCP_HTTP, &(connection->httpConnection)) == PAL_Ok ? NE_OK : NE_NET;
        if (result != NE_OK)
        {
            return result;
        }

        // Only proceed if we have any pending items
        if (CSL_VectorGetLength(connection->pendingItems) > 0)
        {
            ProcessPendingItem(connection);
        }
    }

    return result;
}

/*! Check whether the fileVersion of the two input argument is identical.

    @return TRUE if they can be treated as identical. When FALSE is returned, the caller may
            choose to remove old entry, and append a new one with different fileVersion.
 */
static nb_boolean
TreatFileversionAsIdentical(PendingEntry* entry,
                            const NB_EnhancedContentIdentifier* itemId
                            )
{
    nb_boolean result = TRUE;
    if (!itemId || !entry) // Invalid arguments, return FALSE
    {
        result = FALSE;
    }
    else
    {
        // FileVersion should be only checked when:
        //  1. There's only one signid in this pending entry, and
        //  2. The fileVersion attribute of enhanced content identifier is not NULL.
        if ((entry->itemCount == 1) && itemId->fileVersion != NULL)
        {
            // Treat as different when:
            //  1. fileVersion of pending entry is NULL, or
            //  2. fileVersion of pending entry is not NULL, but not the same as expected.
            if (entry->fileVersion == NULL ||
                (entry->fileVersion &&
                 (nsl_strcmp(entry->fileVersion, itemId->fileVersion) != 0)))
            {
                result = FALSE;
            }
        }
    }
    return result;
}

/*! Check if data item request duplicated.

    If request type is "on demand":
        1. If it's duplicated in "on demand" queue, this function returns TRUE.
        2. If it's duplicated with the "in progress" item of "preload", this function returns TRUE.
        3. If it's duplicated in "preload" queue, deleted the item in "preload" queue and this function returns FALSE.
    If request type is "preload":
        1. If it's duplicated in "on demand" queue, this function returns TRUE.
        2. If it's duplicated in "preload" queue, this function returns TRUE.

    @return Is duplicated
*/
nb_boolean
CheckDuplicated(NB_EnhancedContentDownloadManager* pThis,
                NB_EnhancedContentDownloadType downloadType,
                const NB_EnhancedContentIdentifier* itemId)
{
    int i = 0;
    int n = 0;
    int length = 0;

    DownloadQueue* queue                = NULL;
    CSL_Vector*    onDemandPendingItems = NULL;
    CSL_Vector*    preLoadPendingItems  = NULL;
    PendingEntry*  entry                = NULL;

    if (! pThis || ! itemId || ! itemId->dataset)
    {
        return FALSE;
    }

    queue = FindDownloadQueueByType(&pThis->onDemandConnection, itemId->type);
    if (queue)
    {
        onDemandPendingItems = queue->pendingItems;
    }
    queue = FindDownloadQueueByType(&pThis->preloadConnection, itemId->type);
    if (queue)
    {
        preLoadPendingItems = queue->pendingItems;
    }

    if (!preLoadPendingItems || !onDemandPendingItems)
    {
        return FALSE; //XXX: should never happen, how to hanle this if it happened?
    }

    // Request type is "on demand".
    if (downloadType == NB_ECD0_OnDemand)
    {
        // 1. If it's duplicated in "on demand" queue, this function returns TRUE.
        length = CSL_VectorGetLength(onDemandPendingItems);
        for (n = 0; n < length; ++n)
        {
            entry = (PendingEntry*) CSL_VectorGetPointer(onDemandPendingItems, n);
            if (itemId->type != entry->type || nsl_strcmp(itemId->dataset, entry->dataset) != 0)
            {
                continue;
            }

            // Textures haven't itemId.
            if (! itemId->itemId || ! entry->itemIdArray)
            {
                return TRUE;
            }

            for (i = entry->nextItemIndex; i < entry->itemCount; ++i)
            {
                if (nsl_strcmp(itemId->itemId, entry->itemIdArray[i]) == 0)
                {
                    if (TreatFileversionAsIdentical(entry, itemId))
                    {
                        return TRUE;
                    }
                    else // Should not be treat as identical, remove old one and return FALSE
                    {
                        FreePendingEntry(entry);
                        CSL_VectorRemove(onDemandPendingItems, n);
                        return FALSE;
                    }
                }
            }
        }

        // 2. If it's duplicated with the "in progress" item of "preload", this function returns TRUE.
        if (pThis->preloadConnection.inProgress && CSL_VectorGetLength(preLoadPendingItems) > 0)
        {
            entry = (PendingEntry*) CSL_VectorGetPointer(preLoadPendingItems, 0);
            if (itemId->type == entry->type && nsl_strcmp(itemId->dataset, entry->dataset) == 0)
            {
                // Textures haven't itemId.
                if (! itemId->itemId || ! entry->itemIdArray)
                {
                    return TRUE;
                }

                if (nsl_strcmp(itemId->itemId, entry->itemIdArray[entry->nextItemIndex]) == 0)
                {
                    if (TreatFileversionAsIdentical(entry, itemId))
                    {
                        return TRUE;
                    }
                    else // Not identical, but it is under process, do not clear, but return FALSE
                    {
                        return FALSE;
                    }
                }
            }

            // 3. If it's duplicated in "preload" queue, deleted the item in "preload" queue and
            //    this function returns FALSE. Do not care about fileVersion, just use new ones.
            length = CSL_VectorGetLength(preLoadPendingItems);
            for (n = length - 1; n >= 0; --n)
            {
                entry = (PendingEntry*) CSL_VectorGetPointer(preLoadPendingItems, n);
                if (itemId->type != entry->type || nsl_strcmp(itemId->dataset, entry->dataset) != 0)
                {
                    continue;
                }

                // Textures haven't itemId.
                if (! itemId->itemId || ! entry->itemIdArray)
                {
                    FreePendingEntry(entry);
                    CSL_VectorRemove(preLoadPendingItems, n);
                    return FALSE;
                }

                for (i = entry->itemCount - 1; i >= entry->nextItemIndex; --i)
                {
                    if (nsl_strcmp(itemId->itemId, entry->itemIdArray[i]) == 0)
                    {
                        if (TreatFileversionAsIdentical(entry, itemId) == FALSE ||
                            (entry->itemCount - 1 == entry->nextItemIndex))
                        {
                            FreePendingEntry(entry);
                            CSL_VectorRemove(preLoadPendingItems, n);
                            return FALSE;
                        }

                        nsl_free(entry->itemIdArray[i]);
                        nsl_memmove(entry->itemIdArray + i, entry->itemIdArray + i + 1,
                                    (entry->itemCount - i - 1) * sizeof(char*));
                        --entry->itemCount;
                        entry->itemIdArray = nsl_realloc(entry->itemIdArray,
                                                         entry->itemCount * sizeof(char*));
                        return FALSE;
                    }
                }
            }
        }
    }
    // Request type is "preload".
    else if (downloadType == NB_ECD0_Preload)
    {
        // 1. If it's duplicated in "on demand" queue, this function returns TRUE.
        length = CSL_VectorGetLength(onDemandPendingItems);
        for (n = 0; n < length; ++n)
        {
            entry = (PendingEntry*) CSL_VectorGetPointer(onDemandPendingItems, n);
            if (itemId->type != entry->type || nsl_strcmp(itemId->dataset, entry->dataset) != 0)
            {
                continue;
            }

            // Textures haven't itemId.
            if (! itemId->itemId || ! entry->itemIdArray)
            {
                return TRUE;
            }

            for (i = entry->nextItemIndex; i < entry->itemCount; ++i)
            {
                if (nsl_strcmp(itemId->itemId, entry->itemIdArray[i]) == 0)
                {
                    if (TreatFileversionAsIdentical(entry, itemId))
                    {
                        return TRUE;
                    }
                    else // @todo No need to remove old entry by present.
                    {
                        return FALSE;
                    }
                }
            }
        }

        // 2. If it's duplicated in "preload" queue, this function returns TRUE.
        length = CSL_VectorGetLength(preLoadPendingItems);
        for (n = 0; n < length; ++n)
        {
            entry = (PendingEntry*) CSL_VectorGetPointer(preLoadPendingItems, n);
            if (itemId->type != entry->type || nsl_strcmp(itemId->dataset, entry->dataset) != 0)
            {
                continue;
            }

            // Textures haven't itemId.
            if (! itemId->itemId || ! entry->itemIdArray)
            {
                return TRUE;
            }

            for (i = entry->nextItemIndex; i < entry->itemCount; ++i)
            {
                if (nsl_strcmp(itemId->itemId, entry->itemIdArray[i]) == 0)
                {
                    if (TreatFileversionAsIdentical(entry, itemId))
                    {
                        return FALSE;
                    }
                    else
                    {
                        return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

/*! Status callback for HTTP connection.

    @return None
    @see PAL_NetConnectionStatusCallback
*/
void
StatusCallback(void* userData, PAL_NetConnectionStatus status)
{
    // The user data points to a connection
    Connection* connection = (Connection*) userData;

    switch (status)
    {
        case PNCS_Failed:
        case PNCS_Error:
        case PNCS_Connected:
        case PNCS_Closed:

            // All handling (including error handling) is done in ProcessPendingItem()
            ProcessPendingItem(connection);
            break;

        case PNCS_Created:
        case PNCS_Resolving:
        case PNCS_Connecting:
        case PNCS_Closing:
        case PNCS_ProxyAuthRequired:
        case PNCS_Undefined:
        default:
            // Don't handle these
            break;
    }
}

/*! Translate a PAL error of HTTP to a NB error

  This function returns NE_OK if the parameter is PAL_Ok. Otherwises this function returns NE_NET
  by default. Other translated NB errors are all final errors for downloading. The request should
  not be retried when downloading returns these errors.

  @return A NB error translated from a PAL error of HTTP
*/
NB_Error
TranslateHttpPalErrorToNeError(PAL_Error palError)
{
    NB_Error nbError = NE_OK;

    switch (palError)
    {
        case PAL_Ok:
            {
                nbError = NE_OK;
                break;
            }
        case PAL_ErrHttpNoContent:
            {
                nbError = NE_HTTP_NO_CONTENT;
                break;
            }
        case PAL_ErrHttpNotModified:
            {
                nbError = NE_HTTP_NOT_MODIFIED;
                break;
            }
        case PAL_ErrHttpBadRequest:
            {
                nbError = NE_HTTP_BAD_REQUEST;
                break;
            }
        case PAL_ErrHttpResourceNotFound:
            {
                nbError = NE_HTTP_RESOURCE_NOT_FOUND;
                break;
            }
        case PAL_ErrHttpInternalServerError:
            {
                nbError = NE_HTTP_INTERNAL_SERVER_ERROR;
                break;
            }
        case PAL_ErrHttpBadGateway:
            {
                nbError = NE_HTTP_BAD_GATEWAY;
                break;
            }
        default:
            {
                // Set the error to NE_NET by default.
                nbError = NE_NET;
                break;
            }
    }

    return nbError;
}

/*! Response status callback for HTTP download.

    @return None.
*/
void
ResponseStatusCallback(PAL_Error errorCode, /*!< Error code returned by PAL. */
                       void* userData,      /*!< Empty user Data  */
                       void* requestData,   /*!< Empty request Data  */
                       uint32 contentLength /*!< */
                       )
{
    NB_Error nbError = NE_OK;

    // The user data points to a connection.
    Connection* connection = (Connection*) userData;

    if ((!connection) || (!(connection->httpConnection)))
    {
        // These pointers cannot be 'NULL' here.
        return;
    }

    //get the file size from the http reply.
    if (connection->requestItem &&  contentLength != 0xffffffff)
    {
        connection->requestItem->fileSize = contentLength + connection->requestItem->downloadedSize;
    }

    if (errorCode == PAL_Ok)
    {
        return;
    }

    // Translate the returned PAL error of HTTP to a NB error.
    nbError = TranslateHttpPalErrorToNeError(errorCode);

    if (nbError == NE_OK)
    {
        return;
    }
    else if (nbError == NE_HTTP_RESOURCE_NOT_FOUND)
    {
        // 404 received, start new metadata-source-query and retry download.
        NB_EnhancedContentDownloadManager* manager    = connection->manager;
        NB_EnhancedContentDataSource*      dataSource = NULL;
        if (!manager)
        {
            return;
        }
        dataSource = manager->dataSource;
        if (!dataSource)
        {
            return;
        }
        //@todo: Add QALog.
        if (++(manager->waitCounter) < MAX_RETRY_TIMES)
        {
            if (NB_EnhancedContentDataSourceIsMetadataSynchronizeInProgress(dataSource) ||
                NB_EnhancedContentDataSourceIsManifestSynchronizeInProgress(dataSource))
            {
                // Synchronizing, skip ...
                return;
            }
            else
            {
                nbError = NB_EnhancedContentDownloadManagerSetWaitingMetadata(manager, TRUE);
                nbError = nbError ? nbError :
                          NB_EnhancedContentDataSourceStartMetadataSynchronize(dataSource);
            }
        }
        else
        {
            connection->retryTimes = MAX_RETRY_TIMES;
            ReceiveCallback(userData, requestData, errorCode, NULL, 0);
        }
    }
    else if (nbError == NE_NET)
    {
        // Retry to MAX_RETRY_TIMES times,  retry times will be controlled by ReceiveCallback.
        ReceiveCallback(userData, requestData, errorCode, NULL, 0);
    }
    else
    {
        // Other errors, skip?
    }
}

/*! Data receive callback for HTTP download.

    @return None
    @see PAL_NetHttpDataReceivedCallback
*/

void
ReceiveCallback(void* userData,         /*!< See PAL_NetHttpDataReceivedCallback */
                void* requestData,
                PAL_Error errorCode,
                const byte* bytes,
                uint32 count)
{
    /*
        Compare function to DataReceivedCallback() in "nbrastertilemanager.c".
    */

    Connection* connection = (Connection*) userData;
    RequestItem* item = connection->requestItem;
    NB_EnhancedContentDownloadManager* pThis = connection->manager;
    PAL_Instance* pal = NB_ContextGetPal(pThis->context);
    PendingEntry* entry = NULL;

    // HTTP connection has been destroyed.
    if (!connection->httpConnection || !item)
    {
        return;
    }

    // New data received from network, reset waitCounter.
    connection->manager->waitCounter = 0;

    // Do we have any data
    if (errorCode == PAL_Ok && bytes && (count > 0))
    {
        NB_Error saveResult = NE_OK;
        SaveData(pThis, item, bytes, count);

        /*if savedata failed, close the connection.
          callback the error and store the downloaded size to database. */
        saveResult = item->error;
        if (saveResult != NE_OK)
        {
            //store the downloaded size to database
            NB_EnhancedContentDataSourceUpdateDataItemDownloadStatus(pThis->dataSource, item->item,
                                                                     NB_ECUS_UpdateStatusNew,
                                                                     NB_ECDS_DownloadPaused,
                                                                     item->downloadedSize);
            //close connection
            PAL_NetCloseConnection(connection->httpConnection);
        }
        else
        {
            item->downloadedSize += count;
        }
        if (item->fileSize != 0)
        {
            uint32 itemDownloadedPrecentage = (uint32)(((double)(item->downloadedSize)/item->fileSize)*100);
            /*the callback when download complete will called on line, means after deal with the downloaded file,
              it is available now */
            if (itemDownloadedPrecentage < 100 )
            {
                pThis->callbackData.callback(pThis, saveResult, itemDownloadedPrecentage,
                                             item->item, pThis->callbackData.userData);
            }
        }
    }
    // This must be the end of the receiving data
    else
    {
        connection->requestItem = NULL;
        connection->inProgress = FALSE;

        // Reallocation error OR Network error for this specific request
        if (item->error != NE_OK || errorCode != PAL_Ok)
        {
            if (connection->retryTimes >= MAX_RETRY_TIMES)
            {
                NB_Error result = (errorCode != PAL_Ok) ? NE_NETSVC : item->error;

                connection->retryTimes = 0;

                // If this was the last (or only) entry in our pending-entry array then we remove the item from
                // the pending list
                if (CSL_VectorGetLength(connection->pendingItems) > 0)
                {
                    entry = CSL_VectorGetPointer(connection->pendingItems, 0);
                    if (! entry->itemIdArray || ++(entry->nextItemIndex) >= entry->itemCount)
                    {
                        FreePendingEntry(entry);
                        CSL_VectorRemove(connection->pendingItems, 0);
                    }
                }

                // Log download result to QA log.
                QaLogEnhancedContentDownloadResult(pThis, connection == &pThis->onDemandConnection ? NB_ECD0_OnDemand : NB_ECD0_Preload, item->item, result);

                //store the downloaded size to database
                NB_EnhancedContentDataSourceUpdateDataItemDownloadStatus(pThis->dataSource, item->item,
                                                                         NB_ECUS_UpdateStatusNew,
                                                                         NB_ECDS_DownloadPaused,
                                                                         item->downloadedSize);
                // Inform the user that the download has failed
                pThis->callbackData.callback(pThis, result,
                                             (uint32)(((double)(item->downloadedSize)/item->fileSize)*100),
                                             item->item, pThis->callbackData.userData);
            }
            else
            {
                ++(connection->retryTimes);
            }
        }
        // We successfully downloaded the tile.
        else
        {
            NB_Error result = NE_OK;

            connection->retryTimes = 0;

            // If this was the last (or only) entry in our pending-entry array then we remove the item from
            // the pending list
            if (CSL_VectorGetLength(connection->pendingItems) > 0)
            {
                entry = CSL_VectorGetPointer(connection->pendingItems, 0);
                if (! entry->itemIdArray || ++(entry->nextItemIndex) >= entry->itemCount)
                {
                    FreePendingEntry(entry);
                    CSL_VectorRemove(connection->pendingItems, 0);
                }
            }

            switch (item->item->type)
            {
                case NB_ECDT_RealisticSign:
                case NB_ECDT_MotorwayJunctionObject:
                case NB_ECDT_SpeedLimitsSign:
                case NB_ECDT_SpecialRegions:
                {
                    PAL_Error palErr = PAL_Ok;
                    PAL_SetLastError(pal, PAL_Ok); // reset PAL last error and call SaveToCache then
                    // Take ownership of the buffer.
                    result = SaveToCache(pThis, item->item, item->fileVersion, item->buffer, item->size);

                    palErr = PAL_GetLastError(pal);

                    if (result == NE_FSYS && palErr && (
                            palErr == PAL_ErrFileFailed ||
                            palErr == PAL_ErrFileNotExist ||
                            palErr == PAL_ErrFileExist ||
                            palErr == PAL_ErrFileNoPermission ||
                            palErr == PAL_ErrFileNotOpen))
                    {
                        char buf[128] = {0};
                        nsl_snprintf(buf, sizeof(buf), "nbenhancedcontentdownloadmanager.c::ReceiveCallback(), item->size=%d, palErr=%d", item->size, palErr);
                        buf[sizeof(buf) - 1] = '\0';
                        NB_AnalyticsAddAppFileSystemErrorEvent(pThis->context, NE_FSYS_WRITE_ERROR, "NE_FSYS_WRITE_ERROR",
                                buf, NULL, item->path);
                    }

                    item->buffer = NULL;
                    item->size = 0;

                    break;
                }
                case NB_ECDT_CityModel:
                {
                    // Nothing to do there. The textures and city tiles get saved directly to disk. See SaveData().
                    // test.
                    if (connection == &pThis->onDemandConnection)
                    {
                        TEST_LOGGING(" - On demand downloaded city tile: %s", item->path);
                    }
                    else
                    {
                        TEST_LOGGING(" - Preload downloaded city tile: %s", item->path);
                    }
                    break;
                }
                case NB_ECDT_CommonTexture:
                case NB_ECDT_SpecificTexture:
                case NB_ECDT_MapRegions:
                case NB_ECDT_MapRegionsCommon:
                {
                    PAL_Error palErr = PAL_Ok;
                    char pathWithoutFile[MAX_FILE_PATH] = {0};

                    nsl_memset(pathWithoutFile, 0, sizeof(pathWithoutFile));
                    nsl_strlcpy(pathWithoutFile, item->path, MAX_FILE_PATH);

                    palErr = palErr ? palErr : PAL_FileRemovePath(pal, pathWithoutFile);
                    palErr = palErr ? palErr : PAL_FileUnzip(pal, item->path, pathWithoutFile);
                    palErr = palErr ? palErr : PAL_FileRemove(pal, item->path);

                    if (palErr && (
                            palErr == PAL_ErrFileFailed ||
                            palErr == PAL_ErrFileNotExist ||
                            palErr == PAL_ErrFileExist ||
                            palErr == PAL_ErrFileNoPermission ||
                            palErr == PAL_ErrFileNotOpen))
                    {
                        char buf[128] = {0};
                        nsl_snprintf(buf, sizeof(buf), "nbenhancedcontentdownloadmanager.c::ReceiveCallback(), item->size=%d, palErr=%d", item->size, palErr);
                        buf[sizeof(buf) - 1] = '\0';
                        NB_AnalyticsAddAppFileSystemErrorEvent(pThis->context, NE_FSYS_UNZIP_READ_ERROR, "NE_FSYS_UNZIP_READ_ERROR",
                                buf, NULL, item->path);
                    }

                    TEST_LOGGING(" - Downloaded texture: %s", item->path);

                    result = (palErr != PAL_Ok) ? NE_FSYS : NE_OK;

                    break;
                }
                default:
                {
                    result = NE_INVAL;
                    break;
                }
            }

            // Log download result to QA log.
            QaLogEnhancedContentDownloadResult(pThis, connection == &pThis->onDemandConnection ? NB_ECD0_OnDemand : NB_ECD0_Preload, item->item, result);

            // Indicate result to user (success or failure)
            pThis->callbackData.callback(pThis, result, (result == NE_OK) ? 100 : 0, item->item, pThis->callbackData.userData);
        }

        /* request life cycle complete. Free it */
        FreeRequestItem(item);

        ProcessPendingItem(connection);
    }
}

/*! Allocate a new request item

    @return request item or NULL on failure
*/
RequestItem*
AllocateRequestItem(NB_EnhancedContentDataType type, const char* dataset, const char* itemId, const char* fileVersion)
{
    RequestItem* item = nsl_malloc(sizeof(RequestItem));
    if (! item)
    {
        return NULL;
    }
    nsl_memset(item, 0, sizeof(RequestItem));

    item->item = NB_EnhancedContentIdentifierCreate(type, dataset, itemId, fileVersion);
    if (! item->item)
    {
        FreeRequestItem(item);
        return NULL;
    }

    if (fileVersion)
    {
        item->fileVersion = nsl_strdup(fileVersion);
        if (! item->fileVersion)
        {
            FreeRequestItem(item);
            return NULL;
        }
    }

    item->buffer = NULL;
    item->size = 0;
    item->path = NULL;
    item->error = NE_OK;

    return item;
}

/*! Free a request item.

    @return None
*/
void
FreeRequestItem(RequestItem* item)
{
    if (! item)
    {
        return;
    }
    if (item->item)
    {
        NB_EnhancedContentIdentifierDestroy(item->item);
    }
    if (item->fileVersion)
    {
        nsl_free(item->fileVersion);
    }
    if (item->buffer)
    {
        nsl_free(item->buffer);
    }
    if (item->path)
    {
        nsl_free(item->path);
    }
    nsl_free(item);
}

/*! Free a pending entry.

    This only frees the data associated with the pending entry, not the structure itself.

    @return None
*/
void
FreePendingEntry(PendingEntry* entry)
{
    if (entry)
    {
        if (entry->fileVersion)
        {
            nsl_free(entry->fileVersion);
        }

        if (entry->dataset)
        {
            nsl_free(entry->dataset);
        }

        if (entry->itemIdArray)
        {
            int i = 0;
            for (; i < entry->itemCount; ++i)
            {
                // Free the item
                nsl_free(entry->itemIdArray[i]);
            }

            // Free the char* array itself
            nsl_free(entry->itemIdArray);
        }
    }
}

/*! Callback used for CSL_VectorForEach() to free all pending entries.

    @return 1 to continue.
*/
int
FreePendingEntryCallback(void* userData, void* entry)
{
    FreePendingEntry((PendingEntry*)entry);
    return 1;
}

/*! Process next pending item.

    If the connection is disconnected then a new connection is established. If the connection is idle then
    the next pending item is requested. If the connection is currently busy then this function does nothing.

    @return None
*/
void
ProcessPendingItem(Connection* connection)
{
    PAL_NetConnectionStatus status = PNCS_Undefined;
    NB_EnhancedContentDownloadManager* pThis = connection->manager;

    // Download is paused.
    if (! connection->httpConnection)
    {
        return;
    }

    status = PAL_NetGetStatus(connection->httpConnection);
    switch (status)
    {
        case PNCS_Closed:
        case PNCS_Initialized:
        {
            PAL_Error palResult = PAL_Ok;

            // Network configuration. We only need the status and receive callbacks.
            PAL_NetConnectionConfig configuration = {0};
            configuration.netStatusCallback             = &StatusCallback;
            configuration.netHttpDataReceivedCallback   = &ReceiveCallback;
            configuration.netHttpResponseStatusCallback = &ResponseStatusCallback;
            configuration.userData                      = connection;

            // Get the host name for the first request. Don't get the host name during create since the
            // data source would not be ready yet.
            if (nsl_strempty(pThis->hostName))
            {
                char tempHostname[MAX_HOSTNAME_LENGTH] = {0};
                char* removePosition = NULL;

                // Get host name and port from data source
                // @todo: Change a way to get base url: base URLs for SAR, MOJ, or
                //        SpeedLimitsSign may be different in feature.
                NB_Error result = NB_EnhancedContentDataSourceGetHostname(pThis->dataSource, MAX_HOSTNAME_LENGTH, pThis->hostName, &pThis->port);
                if (result != NE_OK)
                {
                    pThis->hostName[0] = '\0';

                    // Inform user of failure.
                    // @todo: I think we should remove the item from the queue?
                    pThis->callbackData.callback(pThis, result, 0, NULL, pThis->callbackData.userData);
                    return;
                }

                // Remove "http://" if present. Our connect will fail if it is present.
                nsl_strlcpy(tempHostname, pThis->hostName, sizeof(tempHostname));
                removePosition = nsl_stristr(tempHostname, BASE_URL_REMOVE);
                if (removePosition)
                {
                    nsl_strlcpy(pThis->hostName, removePosition + nsl_strlen(BASE_URL_REMOVE), MAX_HOSTNAME_LENGTH);
                }
            }

            // Open the http connection. ProcessPendingItem() gets called again from the status callback once
            // the connection has been established (or has failed).
            palResult = PAL_NetOpenConnection(connection->httpConnection, &configuration, pThis->hostName, pThis->port);
            if (palResult != PAL_Ok)
            {
                // Inform user of failure.
                // @todo: I think we should remove the item from the queue?
                pThis->callbackData.callback(pThis, NE_NET, 0, NULL, pThis->callbackData.userData);
                return;
            }

            break;
        }

        // The connection to the host has failed
        case PNCS_Failed:
        {
            // Forward the network connection failure to the API users callback
//            pThis->callbackData.callback(pThis, NE_NET, 0, NULL, pThis->callbackData.userData);

            break;
        }

        case PNCS_Error:
        {
            // Close the connection. We should get called with the state 'PNCS_Closed' again to
            // reconnect (if necessary)
            PAL_NetCloseConnection(connection->httpConnection);

            break;
        }

        case PNCS_Connected:
        {
            // Request the next item (if any is pending)
            StartRequest(connection);

            break;
        }

        case PNCS_Undefined:
        case PNCS_Created:
        case PNCS_Resolving:
        case PNCS_Connecting:
        case PNCS_Closing:
        case PNCS_ProxyAuthRequired:
        default:

            // Ignore those states
            break;
    }
}

/*! Send out the next HTTP request.

    Subfunction of ProcessPendingItem().

    This function should only be called in the connected state. If there are no pending items for the given
    connection then the call is ignored.

    @return None
    @see ProcessPendingItem
*/
void
StartRequest(Connection* connection /*!< Connection for which to process the next request */
             )
{
    NB_EnhancedContentDownloadManager* pThis = connection->manager;

    if (connection->inProgress)
    {
        return;
    }

    if (!PreparePendingItemsForDownload(connection))
    {
        return;
    }

    // Only proceed if we have any pending items
    if (CSL_VectorGetLength(connection->pendingItems) <= 0)
    {
        // Request "preload" items if there are no "on demand" items pending
        if (connection == &pThis->onDemandConnection)
        {
            ProcessPendingItem(&pThis->preloadConnection);
        }

        return;
    }

    // Cannot stop current "preload" downloading just stop queued items sending new "preload" requests
    // Don't request "preload" items if "on demand" isn't paused and there are "on demand" items pending
    if (connection == &pThis->preloadConnection &&                          // This is "preload"
        pThis->onDemandConnection.httpConnection &&                         // "On demand" isn't paused
        CSL_VectorGetLength(pThis->onDemandConnection.pendingItems) > 0)    // there are "on demand" items pending
    {
        return;
    }

    {
        NB_Error result = NE_OK;
        char* itemId = NULL;
        PendingEntry* entry = NULL;
        RequestItem* requestItem = NULL;
        char url[MAX_URL_LENGTH] = {0};

        /*
            Since we use FIFO (first in, first out) we have to remove the items from the front of the vector. This
            is not very efficient since the vector has to do a realloc and copy. Ideally we would want to use a queue
            (linked list), but it is probably not a big deal.
        */
        entry = CSL_VectorGetPointer(connection->pendingItems, 0);

        if (entry->itemIdArray)
        {
            // We remove the pending-entry once we're done with the last item in the array so we should never have an invalid
            // index here.
            nsl_assert(entry->nextItemIndex < entry->itemCount);

            itemId = entry->itemIdArray[entry->nextItemIndex];
        }

        // Create structure for http request
        requestItem = AllocateRequestItem(entry->type, entry->dataset, itemId, entry->fileVersion);
        if (requestItem)
        {
            /* set the file size and downloaded size. the file size used for
               caculate the downloaded percentage. */
            NB_EnhancedContentDataSourceGetDataItemSizeInfo(pThis->dataSource, requestItem->item,
                                                            &requestItem->fileSize, &requestItem->downloadedSize);

            result = NB_EnhancedContentDataSourceBuildItemUrl(pThis->dataSource, requestItem->item, MAX_URL_LENGTH, url);
            if (result == NE_OK)
            {
                PAL_Error palResult = PAL_Ok;
                char tempAdditionalHeader[MAX_URL_LENGTH] = {0};
                if (pThis->additionalHeaders[0])
                {
                    char headerRange[MAX_URL_LENGTH] = {0};
                    nsl_strlcpy(tempAdditionalHeader, pThis->additionalHeaders, sizeof(tempAdditionalHeader));
                    nsl_strlcat(tempAdditionalHeader, HEADER_RANGE, sizeof(tempAdditionalHeader));
                    sprintf(headerRange, "%d-", requestItem->downloadedSize);
                    nsl_strlcat(tempAdditionalHeader, headerRange, sizeof(tempAdditionalHeader));
                }
                // Make the HTTP request.
                palResult = PAL_NetHttpSend(connection->httpConnection, NULL, 0, HTTP_VERB, url, NULL,
                                            tempAdditionalHeader[0] ? tempAdditionalHeader : NULL,
                                            NULL);
                result = palResult != PAL_Ok ? NE_NETSVC : result;
                if (result == NE_OK)
                {
                    // Log download request to QA log.
                    QaLogEnhancedContentDownload(pThis, connection == &pThis->onDemandConnection ?
                                                                        NB_ECD0_OnDemand : NB_ECD0_Preload,
                                                requestItem->item, url);
                }
            }
        }
        else
        {
            result = NE_NOMEM;
        }

        if (result != NE_OK)
        {
            // Inform user of failure
            pThis->callbackData.callback(pThis, result, 0, requestItem ? requestItem->item : NULL, pThis->callbackData.userData);

            FreeRequestItem(requestItem);

            // If this was the last (or only) entry in our pending-entry array then we remove the item from
            // the pending list
            if (! entry->itemIdArray || ++(entry->nextItemIndex) >= entry->itemCount)
            {
                FreePendingEntry(entry);
                CSL_VectorRemove(connection->pendingItems, 0);
            }
        }
        else
        {
            FreeRequestItem(connection->requestItem);
            connection->requestItem = requestItem;
            connection->inProgress = TRUE;
        }
    }
}

/*! Get the base path based on the type.

    The returned path is valid as long as pThis is valid. It does not need to be freed.

    @return Pointer to base path on success, NULL on failure.
*/
char*
GetBasePath(NB_EnhancedContentDownloadManager* pThis,
            NB_EnhancedContentDataType type
            )
{
    switch (type)
    {
        case NB_ECDT_CityModel:
            return pThis->enhancedCityModelPath;

        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
            // Use the same base path for all textures
            return pThis->texturesPath;

        case NB_ECDT_RealisticSign:
            return pThis->realisticSignsPath;

        case NB_ECDT_MotorwayJunctionObject:
            return pThis->motorwayJunctionObjectPath;

        case NB_ECDT_SpeedLimitsSign:
            return pThis->speedLimitsSignsPath;

        case NB_ECDT_SpecialRegions:
            return pThis->specialRegionsPath;
        case NB_ECDT_MapRegionsCommon:
        case NB_ECDT_MapRegions:
            return pThis->regionsPath;
        default:
            return NULL;
    }
}

/*! Build item file path.

    Build file path of item. And ensure that the path exists if parameter "created" is TRUE.
    Return full path for city model and textures or subpath for MJO and SAR.

    @return NB_Error
*/
NB_Error
BuildItemFilePath(NB_EnhancedContentDownloadManager* pThis,
                  const NB_EnhancedContentIdentifier* item, /*!< Item to build file path for */
                  nb_boolean zipped,                        /*!< If TRUE build zipped file path or unzipped folder path
                                                                 Only available for textures */
                  nb_boolean created,                       /*!< Flag to indicate to create path */
                  char** path                               /*!< On return full path for city model and textures or
                                                                 subpath for MJO and SAR.
                                                                 Has to be freed using nsl_free() when no longer used. */
                  )
{
    NB_Error  result   = NE_OK;
    PAL_Error palErr   = PAL_Ok;
    char*     basePath = NULL;
    char subPath[MAX_FILE_PATH]  = {0};
    char fullPath[MAX_FILE_PATH] = {0};
    PAL_Instance* pal = NULL;

    if (! pThis || ! item || ! path)
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(pThis->context);

    basePath = GetBasePath(pThis, item->type);
    if (! basePath)
    {
        return NE_INVAL;
    }

    // Get sub-path for item
    nsl_memset(subPath, 0, sizeof(subPath));
    result = NB_EnhancedContentDataSourceBuildItemFilePath(pThis->dataSource, item, zipped, MAX_FILE_PATH, subPath);
    if (result != NE_OK)
    {
        return result;
    }

    // Get full path for the file
    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, MAX_FILE_PATH);

    palErr = PAL_FileAppendPath(pal, fullPath, MAX_FILE_PATH, subPath);
    if (palErr != PAL_Ok)
    {
        return NE_FSYS;
    }

    if (created)
    {
        // Remove the filename from the path
        char pathWithoutFile[MAX_FILE_PATH] = {0};

        nsl_memset(pathWithoutFile, 0, MAX_FILE_PATH);
        nsl_strcpy(pathWithoutFile, fullPath);

        palErr = palErr ? palErr : PAL_FileRemovePath(pal, pathWithoutFile);

        // Ensure that the path exists.
        /*
         @todo: We should not do this for every single file. We should only do this once for every file structure.
         Maybe move to data source?
         */
        palErr = palErr ? palErr : PAL_FileCreateDirectoryEx(pal, pathWithoutFile);

        if (palErr != PAL_Ok)
        {
            return NE_FSYS;
        }
    }

    // Return full path for city model and textures or subpath for MJO and SAR.
    switch (item->type)
    {
        case NB_ECDT_CityModel:
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            *path = nsl_strdup(fullPath);
            if (! (*path))
            {
                result = NE_NOMEM;
            }

            break;
        }
        case NB_ECDT_MotorwayJunctionObject:
        case NB_ECDT_RealisticSign:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        {
            *path = nsl_strdup(subPath);
            if (! (*path))
            {
                result = NE_NOMEM;
            }

            break;
        }
        default:
        {
            result = NE_INVAL;
            break;
        }
    }

    return result;
}

/*! Save the given data.

    Save/append file for city model and textures. Copy to memory for MJO and SAR.

    @return None
*/
void
SaveData(NB_EnhancedContentDownloadManager* pThis,
         RequestItem* item,     /*!< Item to save */
         const byte* buffer,    /*!< Buffer of item to save */
         uint32 size            /*!< Size of buffer */
         )
{
    PAL_Instance* pal = NULL;
    PAL_Error palErr = PAL_Ok;

    if (! pThis || ! item || ! item->item || (item->error != NE_OK) || ! buffer || (size == 0))
    {
        return;
    }

    pal = NB_ContextGetPal(pThis->context);

    switch (item->item->type)
    {
        case NB_ECDT_CityModel:
        case NB_ECDT_CommonTexture:
        case NB_ECDT_SpecificTexture:
        case NB_ECDT_MapRegions:
        case NB_ECDT_MapRegionsCommon:
        {
            if (!item->path)
            {
                item->error = BuildItemFilePath(pThis, item->item, TRUE, TRUE, &item->path);
                if (item->error != NE_OK)
                {
                    return;
                }
                if (PAL_FileExists(pal, item->path) != PAL_Ok)
                {
                    PAL_Error result = PAL_Ok;
                    PAL_File* file = NULL;
                    //pre-allocate file
                    result = PAL_FileOpen(pal, item->path, PFM_Create, &file);
                    result = result == PAL_Ok ? PAL_FileTruncate(file, item->fileSize):result;
                    PAL_FileClose(file);
                }
            }
            item->error = SaveToFile(pal, item->path, (uint8*) buffer, size, item->downloadedSize) ? NE_OK : NE_RES;

            // check the error code after savefile() called
            palErr = PAL_GetLastError(pal);
            if (palErr && (
                    palErr == PAL_ErrFileFailed ||
                    palErr == PAL_ErrFileNotExist ||
                    palErr == PAL_ErrFileExist ||
                    palErr == PAL_ErrFileNoPermission ||
                    palErr == PAL_ErrFileNotOpen))
            {
                char buf[128] = {0};
                nsl_snprintf(buf, sizeof(buf), "nbenhancedcontentdownloadmanager.c::SaveData(), size=%d, palErr=%d", size, palErr);
                buf[sizeof(buf) - 1] = '\0';
                NB_AnalyticsAddAppFileSystemErrorEvent(pThis->context, NE_FSYS_WRITE_ERROR, "NE_FSYS_WRITE_ERROR",
                        buf, NULL, item->path);
            }

            break;
        }
        case NB_ECDT_RealisticSign:
        case NB_ECDT_MotorwayJunctionObject:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        {
            // Is it the first chunck of data
            if ((item->buffer == NULL) && (item->size == 0))
            {
                // Save data to newly allocated block
                item->buffer = nsl_malloc(size);
                if (! item->buffer)
                {
                    item->error = NE_NOMEM;
                    return;
                }

                item->size = size;
                nsl_memcpy(item->buffer, buffer, size);
            }
            // This is an additional chunck of data
            else
            {
                // Reallocate buffer
                item->buffer = nsl_realloc(item->buffer, item->size + size);
                if (! item->buffer)
                {
                    item->error = NE_NOMEM;
                    return;
                }

                // Copy new data to end of buffer
                nsl_memcpy(item->buffer + item->size, buffer, size);
                item->size += size;
            }

            break;
        }
        default:
        {
            item->error = NE_RES;
            break;
        }
    }
}

/*! Save item data to cache.

    Take ownership of parameter "buffer".

    @return NB_Error
*/
NB_Error
SaveToCache(NB_EnhancedContentDownloadManager* pThis,
            NB_EnhancedContentIdentifier* item,     /*!< Item to save */
            const char* fileVersion,                /*!< File version to save */
            byte* buffer,                           /*!< Buffer of item to save */
            uint32 size                             /*!< Size of buffer */
            )
{
    NB_Error result = NE_OK;
    char* subPath = NULL;

    if (! buffer)
    {
        return NE_INVAL;
    }

    if (! pThis || ! item || ! fileVersion || (size == 0))
    {
        nsl_free(buffer);
        return NE_INVAL;
    }

    result = BuildItemFilePath(pThis, item, TRUE, TRUE, &subPath);
    if (result != NE_OK)
    {
        nsl_free(buffer);
        return result;
    }

    switch (item->type)
    {
        case NB_ECDT_RealisticSign:
        case NB_ECDT_MotorwayJunctionObject:
        case NB_ECDT_SpeedLimitsSign:
        case NB_ECDT_SpecialRegions:
        {
            CSL_Cache* cache = FindCacheByDataType(pThis, item->type);
            // Add the item to the cache. The cache takes ownership of the buffer (if successful)
            // Use the file path as the cache entry
            if (cache)
            {
                result = CSL_CacheAdd(cache, (byte*)subPath, nsl_strlen(subPath) , buffer, size,
                                      FALSE, FALSE, 0);

                TEST_LOGGING(" - Cache item: %s", subPath);

                if (result != NE_OK)
                {
                    nsl_free(buffer);
                    nsl_free(subPath);
                    return result;
                }

                // Save the new entry to the persistent cache immediately and remove it from the
                // memory cache. We only really care for the persistent cache (saving to
                // elements to file)
                result = result ? result : CSL_CacheSaveDirty(cache, -1, NULL, TRUE);
                result = result ? result :
                         NB_EnhancedContentDataSourceAddManifestEntry(pThis->dataSource,
                                                                      item->type, item->dataset,
                                                                      item->itemId, fileVersion);
                if (result == NE_EXIST) // There are old manifest entry, delete and retry.
                {
                    NB_EnhancedContentDataSourceRemoveDataItem(pThis->dataSource, item->type,
                                                               item->dataset, item->itemId);
                    result = NB_EnhancedContentDataSourceAddManifestEntry(pThis->dataSource,
                                                                          item->type,
                                                                          item->dataset,
                                                                          item->itemId,
                                                                          fileVersion);
                }

                // It would be better if we would save the index file only when we get pushed to
                // the background and not for every entry. For simplicity we do it here. We
                // don't have that many signs/junction objects so the performance hit should be
                // negligeable.
                CSL_CacheSaveIndex(cache);
            }
            else
            {
                result = NE_INVAL;
            }
            break;
        }
        default:
        {
            nsl_free(buffer);
            result = NE_INVAL;
            break;
        }
    }

    nsl_free(subPath);

    return result;
}

/*! Notification callback for cache removed.

    @see CSL_CacheRemoveNotificationFunction

    @return None
*/
void
CacheRemovedNotificationCallback(CSL_Cache* cache,
                                 const byte* name,
                                 size_t namelen,
                                 void* userData)
{
    NB_Error result = NE_OK;
    size_t length = 0;
    NB_EnhancedContentDataType type = NB_ECDT_None;
    char* path = NULL;
    char* itemId = NULL;
    const char* datasetId = NULL;
    NB_EnhancedContentDownloadManager* pThis = (NB_EnhancedContentDownloadManager*)userData;

    if (! cache || ! name || (namelen == 0) || ! userData)
    {
        return;
    }

    // Get the data type and dataset id.
    if (cache == pThis->signsCache)
    {
        type = NB_ECDT_RealisticSign;
        datasetId = NB_EnhancedContentDataSourceGetRealisticSignsDatasetId(pThis->dataSource);
    }
    else if (cache == pThis->junctionsCache)
    {
        type = NB_ECDT_MotorwayJunctionObject;
        datasetId = NB_EnhancedContentDataSourceGetMotorwayJunctionObjectsDatasetId(pThis->dataSource);
    }
    else if (cache == pThis->speedLimitsSignCache)
    {
        type = NB_ECDT_SpeedLimitsSign;
        datasetId = NB_EnhancedContentDataSourceGetSpeedLimitsSignDatasetId(pThis->dataSource);
    }
    else if (cache == pThis->specialRegionsCache)
    {
        type = NB_ECDT_SpecialRegions;
        datasetId = NB_EnhancedContentDataSourceGetSpecialRegionsDatasetId(pThis->dataSource);
    }

    length = namelen * sizeof(byte) + sizeof(char);
    path = nsl_malloc(length);
    if (!path)
    {
        return;
    }
    nsl_memset(path, 0, length);
    nsl_memcpy(path, name, length - sizeof(char));

    result = result ? result : NB_EnhancedContentDataSourceGetItemIdFromFilePath(pThis->dataSource, path, &itemId);
    nsl_free(path);
    result = result ? result : NB_EnhancedContentDataSourceRemoveDataItem(pThis->dataSource, type, datasetId, itemId);
    if (itemId)
    {
        nsl_free(itemId);
    }
}

/*! Log download request to QA log.

    @return None
*/
void
QaLogEnhancedContentDownload(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentDownloadType downloadType, const NB_EnhancedContentIdentifier* itemId, const char* url)
{
    NB_Error error = NE_OK;
    uint32 dataSize = 0;
    NB_QaLogEnhancedContentType qaLogContentType = NB_QLECT_Undefined;
    NB_QaLogEnhancedContentDownloadType qaLogDownloadType = NB_QLECDT_Undefined;
    char manifestVersion[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];

    if (! pThis || ! NB_ContextGetQaLog(pThis->context) || ! itemId || ! itemId->dataset || ! url)
    {
        return;
    }

    // Get manifest version
    nsl_memset(manifestVersion, 0, sizeof(manifestVersion));
    error = NB_EnhancedContentDataSourceGetManifestVersion(pThis->dataSource, itemId, MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1, manifestVersion);
    if (error != NE_OK)
    {
        return;
    }

    // Get data size
    error = NB_EnhancedContentDataSourceGetDataItemSize(pThis->dataSource, itemId, &dataSize);
    if (error != NE_OK)
    {
        return;
    }

    qaLogContentType = GetQaLogEnhancedContentType(itemId->type);
    qaLogDownloadType = GetQaLogEnhancedContentDownloadType(downloadType);

    NB_QaLogEnhancedContentDownload(pThis->context,
                                    itemId->itemId ? itemId->itemId : itemId->dataset,
                                    qaLogContentType,
                                    qaLogDownloadType,
                                    manifestVersion,
                                    dataSize,
                                    url);
}

/*! Log download result to QA log.

    @return None
*/
void
QaLogEnhancedContentDownloadResult(NB_EnhancedContentDownloadManager* pThis, NB_EnhancedContentDownloadType downloadType, const NB_EnhancedContentIdentifier* itemId, NB_Error result)
{
    NB_Error error = NE_OK;
    uint32 dataSize = 0;
    NB_QaLogEnhancedContentType qaLogContentType = NB_QLECT_Undefined;
    NB_QaLogEnhancedContentDownloadType qaLogDownloadType = NB_QLECDT_Undefined;
    NB_QaLogDownloadResult qaLogDownloadResult = NB_QLDR_Undefined;
    char manifestVersion[MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1];

    if (! pThis || ! NB_ContextGetQaLog(pThis->context) || ! itemId || ! itemId->dataset)
    {
        return;
    }

    // Get manifest version
    nsl_memset(manifestVersion, 0, sizeof(manifestVersion));
    error = NB_EnhancedContentDataSourceGetManifestVersion(pThis->dataSource, itemId, MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1, manifestVersion);
    if (error != NE_OK)
    {
        return;
    }

    // Get data size
    error = NB_EnhancedContentDataSourceGetDataItemSize(pThis->dataSource, itemId, &dataSize);
    if (error != NE_OK)
    {
        return;
    }

    qaLogContentType = GetQaLogEnhancedContentType(itemId->type);
    qaLogDownloadType = GetQaLogEnhancedContentDownloadType(downloadType);
    qaLogDownloadResult = result == NE_OK ? NB_QLDR_Success : NB_QLDR_Failure;

    NB_QaLogEnhancedContentDownloadResult(pThis->context,
                                          itemId->itemId ? itemId->itemId : itemId->dataset,
                                          qaLogContentType,
                                          qaLogDownloadType,
                                          manifestVersion,
                                          dataSize,
                                          qaLogDownloadResult);
}

/*! Convert from NB_EnhancedContentDataType to NB_QaLogEnhancedContentType.

    @return NB_QaLogEnhancedContentType
*/
NB_QaLogEnhancedContentType
GetQaLogEnhancedContentType(NB_EnhancedContentDataType type)
{
    NB_QaLogEnhancedContentType qaLogContentType = NB_QLECT_Undefined;

    switch (type)
    {
        case NB_ECDT_CityModel:
        {
            qaLogContentType = NB_QLECT_CityModels;
            break;
        }
        case NB_ECDT_MotorwayJunctionObject:
        {
            qaLogContentType = NB_QLECT_Junctions;
            break;
        }
        case NB_ECDT_CommonTexture:
        {
            qaLogContentType = NB_QLECT_CommonTextures;
            break;
        }
        case NB_ECDT_SpecificTexture:
        {
            qaLogContentType = NB_QLECT_SpecificTextures;
            break;
        }
        case NB_ECDT_RealisticSign:
        {
            qaLogContentType = NB_QLECT_RealisticSigns;
            break;
        }
        case NB_ECDT_SpeedLimitsSign:
        {
            qaLogContentType = NB_QLECT_SpeedLimit;
            break;
        }
        case NB_ECDT_SpecialRegions:
        {
            qaLogContentType = NB_QLECT_SpecialRegion;
            break;
        }
        default:
        {
            break;
        }
    }

    return qaLogContentType;
}

/*! Convert from NB_EnhancedContentDownloadType to NB_QaLogEnhancedContentDownloadType.

    @return NB_QaLogEnhancedContentDownloadType
*/
NB_QaLogEnhancedContentDownloadType
GetQaLogEnhancedContentDownloadType(NB_EnhancedContentDownloadType downloadType)
{
    NB_QaLogEnhancedContentDownloadType qaLogDownloadType = NB_QLECDT_Undefined;

    switch (downloadType)
    {
        case NB_ECD0_OnDemand:
        {
            qaLogDownloadType = NB_QLECDT_OnDemand;
            break;
        }
        case NB_ECD0_Preload:
        {
            qaLogDownloadType = NB_QLECDT_Preload;
            break;
        }
        default:
        {
            break;
        }
    }

    return qaLogDownloadType;
}



NB_Error
NB_EnhancedContentDownloadManagerPauseItemDownload(NB_EnhancedContentDownloadManager* instance,
                                                    const NB_EnhancedContentIdentifier* item,
                                                    NB_EnhancedContentDownloadType downloadType)
{
    NB_Error result = NE_NOENT;
    Connection*  connection = NULL;
    RequestItem* requestItem = NULL;
    PendingEntry* entry = NULL;
    int index = 0;
    switch (downloadType)
    {
        case NB_ECD0_OnDemand:
            connection = &instance->onDemandConnection;
            break;

        case NB_ECD0_Preload:
            connection = &instance->preloadConnection;
            break;

        default:
            return NE_INVAL;
    }
    if (!connection)
    {
        return result;
    }
    //remove form the pandingItem queue.
    while (index < CSL_VectorGetLength(connection->pendingItems))
    {
        entry = CSL_VectorGetPointer(connection->pendingItems, index);
        if (nsl_strcmp(item->dataset, entry->dataset) == 0)
        {
            CSL_VectorRemove(connection->pendingItems, index);
            break;
        }
        ++index;
    }
    //check if the item is current request item. if it is , current connection should be disconnected.
    requestItem = connection->requestItem;
    if (requestItem && nsl_strcmp(requestItem->item->dataset, item->dataset) == 0)
    {
        result = NB_EnhancedContentDataSourceUpdateDataItemDownloadStatus(instance->dataSource,
                        requestItem->item,
                        NB_ECUS_UpdateStatusNew,
                        NB_ECDS_DownloadPaused,
                        requestItem->downloadedSize);
        //close connection
        PAL_NetCloseConnection(connection->httpConnection);
    }
    else
    {
        result = NB_EnhancedContentDataSourceUpdateDataSetDownloadStatus(instance->dataSource,
                                                                    item,
                                                                    NB_ECDS_DownloadPaused);
    }
    return result;
}

NB_Error
NB_EnhancedContentDownloadManagerResumeItemDownload(NB_EnhancedContentDownloadManager* instance,
                                                    const NB_EnhancedContentIdentifier* item,
                                                    NB_EnhancedContentDownloadType downloadType)
{
    NB_Error result = NE_OK;
    Connection*  connection = NULL;
    switch (downloadType)
    {
        case NB_ECD0_OnDemand:
            connection = &instance->onDemandConnection;
            break;

        case NB_ECD0_Preload:
            connection = &instance->preloadConnection;
            break;

        default:
            return NE_INVAL;
    }
    //update download status in manifest
    result = NB_EnhancedContentDataSourceUpdateDataSetDownloadStatus(instance->dataSource,
                                                                     item,
                                                                     NB_ECDS_DownloadNotStarted);
    if (CSL_VectorGetLength(connection->pendingItems) == 0)
    {
        uint32 fileSize = 0;
        /* get the file size which will be used to check if the disk free space is enough
           and downloaded size for break transmission */
        NB_EnhancedContentDataSourceGetDataItemSize(instance->dataSource, item, &fileSize);
        /*Check if disk free space is enough for the region, if no ,return the error.
            As the file size is the original size, so 2*file Size is larger than
            original size + zipped size.*/
        if (PAL_FileGetFreeSpace(NB_ContextGetPal(instance->context)) > 2 * fileSize)
        {
            result = NB_EnhancedContentDownloadManagerRequestData(instance, item,
                                                                  item->fileVersion,
                                                                  downloadType);
        }
        else if (instance->callbackData.callback)
        {
            instance->callbackData.callback(instance, NE_FSYS_NOSPACE, 0, item,
                                            instance->callbackData.userData);
        }
    }

    return result;
}

boolean
SaveToFile(PAL_Instance* pal, const char* fullfilepath, uint8* data, int size, uint32 pos)
{
    PAL_Error result = PAL_Ok;
    PAL_File* file = NULL;
    uint32 written = 0;
    PAL_SetLastError(pal, PAL_Ok);

    result = PAL_FileOpen(pal, fullfilepath, PFM_Append, &file);
    if (result == PAL_ErrFileNotExist)
    {
        return FALSE;
    }

    result = PAL_FileSetPosition(file, PFSO_Start, pos);
    result = result == PAL_Ok ? PAL_FileWrite(file, (uint8*) data, size, &written) : result;

    if (result != PAL_Ok)
    {
        PAL_FileClose(file);
        return FALSE;
    }

    PAL_FileClose(file);
    return (boolean)(written == (uint32)size);
}

/*! @} */
