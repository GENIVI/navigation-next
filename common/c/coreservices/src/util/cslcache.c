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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
* cache.c: created 2005/01/14 by Mark Goddard.
*/

#include "palfile.h"
#include "palstdlib.h"
#include "palclock.h"

#include "cslcache.h"
#include "vec.h"
#include "cslutil.h"
#include "fileutil.h"
#include "dynbuf.h"
#include "cslhashtable.h"

#define FILENUM_NONE	0xFFFFFFFF

// Fileindex for master index-file. The cache entry files begin with index 1.
#define FILENUM_INDEX	0
#define FILENAME_LENGTH	256

/*
    The maximum number of filenumber we use is the size of the memory cache plus the size of the persistent cache.
    But when we add new entries then we can temporary exceed that number. We therefore add an extra buffer to the
    filenumber vector.
*/
static const int FILENUMBER_VECTOR_BUFFER = 100;

// Flags used for filenumber vector
static const nb_boolean FLAG_TRUE = TRUE;
static const nb_boolean FLAG_FALSE = FALSE;

// File format for filenames using file numbers and cache name
static const char FILENAME_FORMAT[] = "%8.8d";


/*
    Increment this version if 'PersistentEntry' or 'IndexFileHeader' changes.
    Start with 0xFFFF to not conflict with old implementations.

    Version 2: Add optional user data to index-file
    Version 3: Rework of entire cache to use hash table. Remove obsolete 'hash' field, removed timestamp field
               from PersistentEntry.
    Version 4: Increment CSL_MAXIMUM_CACHE_NAME_LENGTH to 128 to accomodate longer cache entry names for Vx
    Version 5: Increment CSL_MAXIMUM_CACHE_NAME_LENGTH to 256 to accomodate longer cache entry names for Phase2
    Version 6: 'PersistentEntry.key' became pointer instead of static array.
*/
#define CURRENT_INDEX_FILE_VERSION  0xFFFF0006

typedef struct CacheEntry CacheEntry;
typedef struct LinkedEntry LinkedEntry;

/*! Linked list used for priority lists.

    We use two priority linked lists which are sorted by last accessed time. They are cross-linked to the hash table.
    That allows us to change the priority (changing position in linked list) without having to perform a costly
    sort or linear search.

    The priority lists are used to determine which elements to remove from the cache when the cache is full.
    The oldest elements get removed from the cache.

    @see LinkedEntry
*/
typedef struct
{
    LinkedEntry*    first;      /*!< First element of linked list. Can be NULL if list is empty */
    LinkedEntry*    last;       /*!< Last element of linked list. Can be NULL if list is empty */

    uint32          count;      /*!< Number of entries in list */

} LinkedList;

/*! Linked entry used in LinkedList.

    @see LinkedList
*/
struct LinkedEntry
{
    LinkedEntry*    next;       /*!< Next element in linked list. Can be NULL if this is the last element. */
    LinkedEntry*    previous;   /*!< Previous element in linked list. Can be NULL if this is the first element */

    CacheEntry*     entry;      /*!< Pointer to the actual cache entry. We use the key in order to find the actual
                                     hash entry (used to remove elements from the hash table). */
};

/*!
    This is the CSL_Cache structure
*/
struct CSL_Cache
{
    PAL_Instance*                   pal;                        /*!< Pointer to nbipal instance */

    const char*	                    name;                       /*!< Name of cache. The name is used as the directory name for the cache. */

    int			                    maximumItemsMemory;         /*!< Maximum entries in memory cache */
    int			                    maximumItemsPersistent;     /*!< Maximum entries in persistent cache. */

    CSL_HashTable*                  hashTable;                  /*!< Hash table to store all cache items. Contains 'CacheEntry' items */

    LinkedList                      priorityListMemory;         /*!< Linked priority list of all memory entries. Sorted by last access time.
                                                                     See LinkedList for more details. */

    LinkedList                      priorityListPersistent;     /*!< Linked priority list of all persistent entries. Sorted by last access time. */

    const byte*	                    obfuscateKey;               /*!< Obfuscate key for file backed cache. If NULL use un-obfuscated file cache. */
    size_t		                    obfuscateKeyLen;            /*!< Obfuscate key length. If zero, use un-obfuscated file cache. */
    nb_boolean                      obfuscateEntries;           /*!< If set to TRUE files are obfuscated. Index file is always obfuscated */

    nb_boolean                      indexFileInvalid;           /*!< Flag to indicate that the index file is out-of-sync with the list in memory */

    byte*                           userData;                   /*!< Optional user data for the index-file. Set to NULL if not used. */
    size_t                          userDataSize;               /*!< Size of user data. */

    /*
        Filenumbers are not used for un-obfuscated file cache.
    */
    CSL_Vector*                     filenumbersInUse;           /*!< Vector of 'FilenumberUsage' structures. This vector is used to calculate the
                                                                     next available filenumber/name to use. */
    uint32                          nextAvailableFilenumber;    /*!< Next available filenumber to use. Get calculated based on the values in 'filenumbersInUse' */

    CSL_CacheSaveIndexFunction      saveIndexFunction;          /*!< Save index function */
    CSL_CacheSaveFunction           saveFunction;               /*!< Save function to save one entry */
    CSL_CacheLoadIndexFunction      loadIndexFunction;          /*!< Load index function */
    CSL_CacheLoadFunction           loadFunction;               /*!< Load function to load one entry */
    CSL_CacheRemoveFunction         removeFunction;             /*!< Function to remove one entry from the persistent cache */
    CSL_CacheDumpFunction           dumpFunction;               /*!< Dump function */

    /* Optional notification function(s). Notifications are only called if the function pointers are set. */
    CSL_CacheRemoveNotificationFunction removeNotificationFunction;

    void*                           userDataNotifications;      /*!< User data for notification callbacks. See CSL_CacheSetNotificationFunctions */
};

/*! Persistent cache entry information.

    It contains all the information needed for the persistent entries. This structure is stored in our
    index-file in order to save/load the cache index file (without having to load every single item)

    !Important: When changing this structure make sure to increment CURRENT_INDEX_FILE_VERSION!
*/
typedef struct
{
    byte*   key;                                    /*!< Unique key into hash table*/
    uint8   keyLength;                              /*!< Length of key in bytes */

    uint32  dataLength;                             /*!< Data length of cache element */
    uint32  filenumber;                             /*!< File number used for filename */

} PersistentEntry;

/*! Cache entry.

    Information used for each cache entry. This structure is stored in the hash table.
*/
struct CacheEntry
{
    PersistentEntry persistentEntry;            /*!< Data needed for persistent entries */
    byte*           data;                       /*!< The data to cache. The data length is stored in the 'persistentEntry' object.
                                                     'data can be set to NULL if the entry is only in the persistent cache */

    uint32          expireTime;                  /*!< Absolute time of entry's expiration -
                                                      timestamp then entry was added to cache plus period, after that entry will be expired*/

    nb_boolean      dirty;                      /*!< The dirty flag gets set for entries which got added to the memory cache calling CSL_CacheAdd(). */
    nb_boolean      accessed;                   /*!< The accessed flag gets set for entries for which CSL_CacheFind() has been called */
    nb_boolean      protect;                    /*!< Protected items are used by the user and must not be removed from the memory cache */

    LinkedEntry*    memoryLink;                 /*!< Pointer to the linked-list entry element in the memory priority list. This pointer
                                                     allows us to directly access/modify the priority list without having to perform
                                                     a search. If this is NULL then the entry is not in memory. */

    LinkedEntry*    persistentLink;             /*!< Pointer to the linked-entry element in the persistent priority list.
                                                     If this pointer is NULL then the element is not in the persistent cache. */
};

/*! Structure used for new file header for index file.

    Older cache objects did not include a header. Any old cache index files will be ignored/overwritten
    which ensures that it will not break existing code.
*/
typedef struct
{
    uint32 fileVersion;     /*!< This version has to start with 0xFFFF since older cache implementations stored
                                 the number of cache entries as first DWORD. Set to 'CURRENT_INDEX_FILE_VERSION' */

    uint32 valid;           /*!< Flag to indicate that the index file is valid or out of synch with the saved file.
                                 We want to avoid writting the entire index file each time we save cache entries to the
                                 persistent cache. That means the index file will be out of synch (invalid) until the cache
                                 gets shut down properly. This flag gets checked during loading. */

    uint32 entryCount;      /*!< Number of entries in the cache */

    size_t userDataSize;    /*!< Size of optional user data. If this size is not zero then the user data block follows
                                 immidiately after the header. */

} IndexFileHeader;

/*! Structure used in 'filenumbersInUse' vector.

    It indicates if a filenumber is used in a memory and/or persistent cache item. Only if it is not used in either
    or can we use it for a new item.
*/
typedef struct
{
    nb_boolean memory;      /*!< 'TRUE' to indicate that the filenumber is used in a memory entry */
    nb_boolean persistent;  /*!< 'TRUE' to indicate that the filenumber is used in a persistent entry */

} FilenumberUsage;

/*! structure used for TEST_ForEachCallback. */
typedef struct
{
    uint32 memoryCount;
    uint32 persistentCount;

} TEST_Counters;


// Local functions ...............................................................................

// Filenumber functions
static void         GetFilename(CSL_Cache* cache, uint32 filenumber, char* filename);
static char*        GetUnobfuscatedFilename(CSL_Cache* cache, const byte* name);
static NB_Error     ValidateFilesOnDisk(CSL_Cache* cache, CSL_Vector* filenumbersInPersistentCache);
static void         InitializeFilenumbersInUse(CSL_Cache* cache, CSL_Vector* filenumbersInPersistentCache);
static void         UpdateFilenumberFlags(CSL_Cache* cache, uint32 filenumber, const nb_boolean* memoryFlag, const nb_boolean* persistentFlag);
static uint32       GetNextFilenumber(CSL_Cache* cache);
static int          CompareFilenumber(const void* left, const void* right);
static void         Dump(CSL_Cache* cache, const char* caption);

// Save functions
static NB_Error     SaveEntryToPersistent(CSL_Cache* cache, CacheEntry* entry);
static NB_Error     SaveInternal(CSL_Cache* cache, int32 maximumEntries, int32* savedEntries, nb_boolean removeFromMemory, nb_boolean saveAccessed, nb_boolean saveDirty);
static NB_Error     SaveIndexFunctionDefault(CSL_Cache* cache, const byte* data, uint32 length);
static NB_Error     SaveFunctionDefault(CSL_Cache* cache, uint32 fileNumber, const byte* name, size_t nameLength, const byte* data, uint32 length);
static void         SetIndexFileToInvalid(CSL_Cache* cache);

// Load functions
static NB_Error     LoadEntryFromPersistent(CSL_Cache* cache, CacheEntry* entry);
static NB_Error     LoadIndexFunctionDefault(CSL_Cache* cache, byte** data, uint32* length);
static NB_Error     LoadFunctionDefault(CSL_Cache* cache, uint32 fileNumber, const byte* name, size_t nameLength, byte** data, uint32* length);

// Clear functions
static void         ClearBothEntries(void* value, void* userData);
static void         ClearMemoryEntry(CSL_Cache* cache, CacheEntry* entry);
static NB_Error     RemoveLastElementFromMemory(CSL_Cache* cache);
static NB_Error     RemoveExcessEntries(CSL_Cache* cache);
static NB_Error     RemoveFunctionDefault(CSL_Cache* cache, uint32 fileNumber, const byte* name, size_t nameLength);
static void         ClearKey(PersistentEntry* entry);
static nb_boolean   ReplaceExistingIfNecessary(CSL_Cache* cache, byte* name, size_t nameLength, byte* data, size_t dataLength, nb_boolean protect, nb_boolean makeCopy, uint32 itemExpiration, NB_Error* result);

// Linked list functions
static void         MoveToTop(LinkedList* list, LinkedEntry* entry);
static nb_boolean   AddToTop(LinkedList* list, CacheEntry* entry, nb_boolean isMemoryList);
static void         RemoveFromList(LinkedList* list, LinkedEntry* entry);
static void         ClearList(LinkedList* list);

static void         ReadData(void* destination, byte** source, uint32 size);


// Disabled to eliminate compiler's warning.
#if 0
// For testing only!
static void         TEST_ValidateCacheConsistency(CSL_Cache* cache);
static void         TEST_ValidateFilenumbersConsistency(CSL_Cache* cache);
static void         TEST_ValidateLinkedListConsistency(CSL_Cache* cache);
static void         TEST_ValidateHashTableConsistency(CSL_Cache* cache);
static void         TEST_ForEachCallback(void* value, void* userData);
#endif

// Public functions ..............................................................................

/* See header for description */
CSL_DEF CSL_Cache*
CSL_CacheAlloc(PAL_Instance* pal,
               const char* name,
               int maximumItemsMemory,
               int maximumItemsPersistent,
               const byte* obfuscateKey,
               size_t obfuscateKeyLen,
               CSL_CacheConfig* config,
               nb_boolean obfuscateEntries)
{
    CSL_Cache* cache =  NULL;
    byte* pStart = NULL;

    if (!pal || !obfuscateKey || !name)
    {
        return NULL;
    }

    cache = nsl_malloc(sizeof(*cache));
    if (!cache)
    {
        return NULL;
    }
    nsl_memset(cache, 0, sizeof(*cache));

    cache->name = nsl_strdup(name);
    if (cache->name == NULL)
    {
        CSL_CacheDealloc(cache);
        return NULL;
    }

    // Create hash table. The hash table contains both memory and persistent entries. We use the default hash function.
    cache->hashTable = CSL_HashTableCreate(maximumItemsMemory + maximumItemsPersistent, NULL);
    if (! cache->hashTable)
    {
        CSL_CacheDealloc(cache);
        return NULL;
    }

    // Save parameters
    cache->pal                      = pal;
    cache->maximumItemsMemory       = maximumItemsMemory;
    cache->maximumItemsPersistent   = maximumItemsPersistent;
    cache->obfuscateKey             = obfuscateKey;
    cache->obfuscateKeyLen          = obfuscateKeyLen;
    cache->obfuscateEntries         = obfuscateEntries;

    if (!config)
    {
        // If the configuration is not set use the default file cache functions
        cache->saveIndexFunction    = SaveIndexFunctionDefault;
        cache->saveFunction         = SaveFunctionDefault;
        cache->loadIndexFunction    = LoadIndexFunctionDefault;
        cache->loadFunction         = LoadFunctionDefault;
        cache->removeFunction       = RemoveFunctionDefault;
        cache->dumpFunction         = Dump;
    }
    else
    {
        // Use user provided functions
        cache->saveIndexFunction    = config->saveIndexFunction;
        cache->saveFunction         = config->saveFunction;
        cache->loadIndexFunction    = config->loadIndexFunction;
        cache->loadFunction         = config->loadFunction;
        cache->removeFunction       = config->removeFunction;
        cache->dumpFunction         = config->dumpFunction;
    }

    // We only use the filenumber vector if we obfuscate files
    if (obfuscateEntries)
    {
        /* Vector for file numbers */
        cache->filenumbersInUse = CSL_VectorAlloc(sizeof(FilenumberUsage));
        if (! cache->filenumbersInUse)
        {
            CSL_CacheDealloc(cache);
            return NULL;
        }

        /* Initialize the filenumbers vector. See FILENUMBER_VECTOR_BUFFER for explanation about size. */
        if (! CSL_VectorSetSize(cache->filenumbersInUse, maximumItemsMemory + maximumItemsPersistent + FILENUMBER_VECTOR_BUFFER))
        {
            CSL_CacheDealloc(cache);
            return NULL;
        }
        // Set all flags to FALSE initially. The flags get set in CSL_CacheLoadIndex.
        pStart = CSL_VectorGetPointer(cache->filenumbersInUse, 0);
        nsl_memset(pStart, 0, CSL_VectorGetLength(cache->filenumbersInUse) * CSL_VectorGetItemSize(cache->filenumbersInUse));

        // Start first filenumber at one. Zero is the index file, see FILENUM_INDEX.
        cache->nextAvailableFilenumber = 1;
    }

    return cache;
}

/* See header for description */
CSL_DEF void
CSL_CacheDealloc(CSL_Cache* cache)
{
    LinkedEntry* current = NULL;

    if (cache == NULL)
    {
        return;
    }

    if (cache->name)
    {
        nsl_free((char*)cache->name);
    }

    // Clear all data from the memory cache
    for (current = cache->priorityListMemory.first; current; current = current->next)
    {
        // This is the memory list so the data has to be valid. Delete it.
        nsl_assert(current->entry->data);
        nsl_free(current->entry->data);
        ClearKey(&current->entry->persistentEntry);
    }

    // Clear all keys from the persistnet cache
    for (current = cache->priorityListPersistent.first; current; current = current->next)
    {
        ClearKey(&current->entry->persistentEntry);
    }

    // Clear the linked lists
    ClearList(&cache->priorityListMemory);
    ClearList(&cache->priorityListPersistent);

    if (cache->filenumbersInUse)
    {
        CSL_VectorDealloc(cache->filenumbersInUse);
    }

    if (cache->hashTable)
    {
        // Destroy hash table. Also destroy all CacheEntry elements still in the hash table.
        CSL_HashTableDestroy(cache->hashTable, TRUE);
    }

    // Free any user data
    if (cache->userData)
    {
        nsl_free(cache->userData);
        cache->userData = NULL;
        cache->userDataSize = 0;
    }

    nsl_free(cache);
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSetSize(CSL_Cache* cache, int maximumItemsMemory, int maximumItemsPersistent)
{
    // We currently don't support resizing the cache dynamically.
    return NE_NOSUPPORT;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSaveIndex(CSL_Cache* cache)
{
    NB_Error result = NE_OK;
    struct dynbuf buffer = {0};
    int cacheSize = 0;
    byte* data = NULL;
    size_t size = 0;

    // The save-index-function pointer has to be valid
    if (!cache->saveIndexFunction)
    {
        return NE_NOTINIT;
    }

    cacheSize = cache->priorityListPersistent.count;

    // Always save in the index-file, even if we don't have any entries, yet.

    // Allocate roughly the size we need. It doesn't really matter since the dynmic buffer resizes if needed.
    result = dbufnew(&buffer, sizeof(PersistentEntry) * ((cacheSize > 0) ? cacheSize : 1));
    if (result == NE_OK)
    {
        LinkedEntry* current = NULL;
        PersistentEntry* info = NULL;

        IndexFileHeader header = {0};
        header.fileVersion  = CURRENT_INDEX_FILE_VERSION;
        header.valid        = TRUE;
        header.entryCount   = cacheSize;
        header.userDataSize = cache->userDataSize;

        // Save index file header
        dbufcat(&buffer, (const byte*)&header, sizeof(IndexFileHeader));

        // If we have any user data then we save it immidiately after the header.
        if (cache->userData && (cache->userDataSize > 0))
        {
            dbufcat(&buffer, cache->userData, cache->userDataSize);
        }

        // Walk the persistent priority list. Walk the list backwards so that we can load them in CSL_CacheLoadIndex()
        // and use the AddToTop() function. Otherwise the order would be reversed.
        for (current = cache->priorityListPersistent.last; current; current = current->previous)
        {
            info = &current->entry->persistentEntry;

            // Save persistent entry to the index file
            dbufcat(&buffer, (const byte*) &info->keyLength, sizeof(info->keyLength));
            dbufcat(&buffer, (const byte*) info->key, info->keyLength);
            dbufcat(&buffer, (const byte*) &info->dataLength, sizeof(info->dataLength));
            dbufcat(&buffer, (const byte*) &info->filenumber, sizeof(info->filenumber));
        }

        data = (byte*) dbufget(&buffer);
        size = dbuflen(&buffer);

        if (data)
        {
            // Call user supplied save-index-function. This will save the index to a file by default.
            result = cache->saveIndexFunction(cache, data, (uint32)size);

            // Reset the invalid-flag in memory
            if (result == NE_OK)
            {
                cache->indexFileInvalid = FALSE;
            }
        }
        else
        {
            result = NE_NOMEM;
        }

        dbufdel(&buffer);
    }

    return result;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSaveAccessed(CSL_Cache* cache, int32 maximumEntries, int32* savedEntries, nb_boolean removeFromMemory)
{
    // Save all accessed entries
    return SaveInternal(cache, maximumEntries, savedEntries, removeFromMemory, TRUE, FALSE);
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSaveDirty(CSL_Cache* cache, int32 maximumEntries, int32* savedEntries, nb_boolean removeFromMemory)
{
    // Save all dirty entries
    return SaveInternal(cache, maximumEntries, savedEntries, removeFromMemory, FALSE, TRUE);
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheLoadIndex(CSL_Cache* cache)
{
    NB_Error result = NE_OK;
    byte* data = NULL;
    uint32 size = 0;
    byte* pos = NULL;
    uint32 i = 0;
    uint32 maximumFilenumber = 0;
    CSL_Vector* filenumbersInCache = NULL;

    if (cache->obfuscateEntries)
    {
        maximumFilenumber = CSL_VectorGetLength(cache->filenumbersInUse);

        // Vector of all the filenumbers used in the cache. This is used to initialized the vector for filenumbers used.
        filenumbersInCache = CSL_VectorAlloc(sizeof(uint32));
        if (!filenumbersInCache)
        {
            return NE_NOMEM;
        }
    }

    // Call the load-index-function. This can be a user supplied function. The default function loads the index from file.
    result = cache->loadIndexFunction(cache, &data, &size);
    if (result == NE_OK)
    {
        pos = data;

        // This should only be called once during startup. The persistent linked-list should still be empty
        nsl_assert(cache->priorityListPersistent.count == 0);

        /*
            The new cache implementation uses a index-file header with a version. We only load the index
            file if it has the header and matches the current version. We ignore old cache entries
            (they all get deleted in ValidateFilesOnDisk(), if the index file is not loaded).
        */
        if (size >= sizeof(IndexFileHeader))
        {
            IndexFileHeader header = {0};
            ReadData(&header, &pos, sizeof(IndexFileHeader));

            // Only proceed if the index file is valid. It will not be valid if the application/cache was
            // not shut down correctly and the index file was not saved at the end (e.g. application crashed or run in debugger)
            if ((header.fileVersion == CURRENT_INDEX_FILE_VERSION) && header.valid)
            {
                // If we have any user data then load it
                if (header.userDataSize > 0)
                {
                    cache->userData = nsl_malloc(header.userDataSize);
                    if (!cache->userData)
                    {
                        return NE_NOMEM;
                    }
                    ReadData(cache->userData, &pos, (uint32)header.userDataSize);
                    cache->userDataSize = header.userDataSize;
                }

                // Preallocate data to avoid re-allocations
                if (filenumbersInCache)
                {
                    CSL_VectorSetAlloc(filenumbersInCache, header.entryCount);
                }

                /*
                    Read the index-data in the exact same order as we wrote it to the file. See CSL_CacheSaveIndex().
                */
                for (i = 0; i < header.entryCount && pos < (data + size); i++)
                {
                    PersistentEntry* info = NULL;

                    // Create new cache entry
                    CacheEntry* entry = nsl_malloc(sizeof(CacheEntry));
                    if (! entry)
                    {
                        result = NE_NOMEM;
                        break;
                    }
                    nsl_memset(entry, 0, sizeof(CacheEntry));

                    info = &entry->persistentEntry;

                    // The ReadData does not check if we exceed the total size. Since we have a
                    // file header we assume that the file is not corrupt.
                    ReadData(&info->keyLength,   &pos, sizeof(info->keyLength));
                    info->key = (byte*)nsl_malloc(info->keyLength +1);
                    if(!info->key)
                    {
                        nsl_free(entry);
                        result = NE_NOMEM;
                        break;
                    }
                    ReadData(info->key,         &pos, info->keyLength);
                    info->key[info->keyLength] = '\0';
                    ReadData(&info->dataLength,  &pos, sizeof(info->dataLength));
                    ReadData(&info->filenumber,  &pos, sizeof(info->filenumber));
                    /*
                        If the reach the maximum number in the persistent count then we abort loading entries.
                        This can only happen if the cache size was reduced between runs.
                    */
                    if (cache->priorityListPersistent.count >= (unsigned)cache->maximumItemsPersistent)
                    {
                        // Abort for-loop, but don't set an error. This is not an error case.
                        ClearKey(info);
                        nsl_free(entry);
                        break;
                    }

                    /*
                        If the filenumber is larger then our maximum allowed filenumber then we ignore the entry.
                        That is possible if an older version was using a larger cache size. The orphaned file
                        will be removed in ValidateFilesOnDisk().
                        Don't do check if we don't obfuscate entries.
                    */
                    if ((cache->obfuscateEntries && (info->filenumber < maximumFilenumber)) ||
                        (! cache->obfuscateEntries))
                    {
                        if (! CSL_HashTableInsert(cache->hashTable, info->key, info->keyLength, entry))
                        {
                            result = NE_NOMEM;
                            ClearKey(info);
                            nsl_free(entry);
                            break;
                        }

                        // Add entry to persistent linked list
                        if (! AddToTop(&cache->priorityListPersistent, entry, FALSE))
                        {
                            result = NE_NOMEM;
                            ClearKey(info);
                            nsl_free(entry);
                            break;
                        }
                        // Add filenumber to vector
                        if (filenumbersInCache)
                        {
                            CSL_VectorAppend(filenumbersInCache, &info->filenumber);
                        }
                    }
                    else
                    {
                        ClearKey(info);
                        nsl_free(entry);
                    }
                } // for
            }
        }

        nsl_free(data);
    }

    // We only use the filenumber vector if we obfuscate files
    if (cache->obfuscateEntries && filenumbersInCache)
    {
        // Sort the filenumber vector, the two functions below expect a sorted vector.
        CSL_VectorSort(filenumbersInCache, &CompareFilenumber);

        // Set all the persistent flags in our internal vector
        InitializeFilenumbersInUse(cache, filenumbersInCache);

        CSL_VectorDealloc(filenumbersInCache);
    }
    return result;
}

/* See header for description */
CSL_DEF void
CSL_CacheGetStats(CSL_Cache* cache,
                  int* maximumItemsMemory,
                  int* maximumItemsPersistent,
                  int* dirtyItems,
                  int* cachedMemoryItems,
                  int* cachedPersistentItems,
                  int* protectedItems)
{
    LinkedEntry* current = NULL;

    if (maximumItemsMemory)
    {
        *maximumItemsMemory = cache->maximumItemsMemory;
    }

    if (maximumItemsPersistent)
    {
        *maximumItemsPersistent = cache->maximumItemsPersistent;
    }

    if (cachedPersistentItems)
    {
        *cachedPersistentItems = cache->priorityListPersistent.count;
    }

    if (dirtyItems)
    {
        *dirtyItems = 0;
    }

    if (protectedItems)
    {
        *protectedItems = 0;
    }

    // Walk memory priority linked list
    for (current = cache->priorityListMemory.first; current; current = current->next)
    {
        if (current->entry->dirty)
        {
            if (dirtyItems)
            {
                (*dirtyItems)++;
            }
        }
        if (current->entry->protect)
        {
            if (protectedItems)
            {
                (*protectedItems)++;
            }
        }
    }

    if (cachedMemoryItems)
    {
        *cachedMemoryItems = cache->priorityListMemory.count;
    }
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheAdd(CSL_Cache* cache,
             byte* name,
             size_t nameLength,
             byte* data,
             size_t dataLength,
             nb_boolean protect,
             nb_boolean makeCopy,
             uint32 itemExpiration)
{
    NB_Error result = NE_OK;
    CacheEntry* entry = NULL;

    // TEST
    // TEST_ValidateCacheConsistency(cache);

    if ((name == NULL) || (nameLength == 0) || (data == NULL) || (dataLength == 0))
    {
        return NE_INVAL;
    }

    // Check if the entry exists. If so replace it and return.
    if (ReplaceExistingIfNecessary(cache, name, nameLength, data, dataLength, protect, makeCopy, itemExpiration, &result))
    {
        return result;
    }
    if (result != NE_OK)
    {
        return result;
    }

    // Create new entry
    entry = nsl_malloc(sizeof(CacheEntry));
    if (! entry)
    {
        return NE_NOMEM;
    }
    nsl_memset(entry, 0, sizeof(CacheEntry));

    if (makeCopy)
    {
        // Make copy of passed in data
        entry->data = nsl_malloc(dataLength);
        if (entry->data == NULL)
        {
            nsl_free(entry);
            return NE_NOMEM;
        }
        nsl_memcpy(entry->data, data, dataLength);
    }
    else
    {
        // Just copy the pointer to the passed in data. The data is now owned by the cache object.
        entry->data = data;
    }

    entry->persistentEntry.key = (byte*)nsl_malloc(nameLength+1);
    if (!entry->persistentEntry.key)
    {
        if(makeCopy && entry->data)
        {
            nsl_free(entry->data);
            entry->data = NULL;
        }
        nsl_free(entry);
        entry = NULL;
        return NE_NOMEM;
    }

    entry->persistentEntry.filenumber = GetNextFilenumber(cache);
    entry->persistentEntry.dataLength = (uint32)dataLength;
    nsl_memcpy(entry->persistentEntry.key, name, nameLength);
    entry->persistentEntry.key[nameLength] = '\0';
    entry->persistentEntry.keyLength = (uint8)nameLength;

    entry->dirty = TRUE;
    entry->protect = protect;

    if(itemExpiration)
    {
        entry->expireTime = itemExpiration + PAL_ClockGetGPSTime();
    }

    // Update the memory flag for this filenumber
    UpdateFilenumberFlags(cache, entry->persistentEntry.filenumber, &FLAG_TRUE, NULL);

    // Add entry to memory priority linked list
    result = NE_NOMEM;
    if (AddToTop(&cache->priorityListMemory, entry, TRUE))
    {
        // Add entry to hash table
        if (CSL_HashTableInsert(cache->hashTable, name, (uint32)nameLength, entry))
        {
            // Remove entry from memory cache if we exceed the maximum count
            result = RemoveLastElementFromMemory(cache);
        }
    }

    if (result != NE_OK)
    {
        // If something went wrong then we try to remove the element again.
        CSL_CacheClearOne(cache, name, nameLength);
    }

    // TEST
    // TEST_ValidateCacheConsistency(cache);

    return result;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheFind(CSL_Cache* cache,
              byte* name,
              size_t nameLength,
              byte** pData,
              size_t* pDataLength,
              nb_boolean protect,
              nb_boolean makeCopy,
              nb_boolean checkMemoryCacheOnly)
{
    NB_Error result = NE_OK;
    CacheEntry* entry = NULL;

    if (!pData || (!pDataLength))
    {
        return NE_INVAL;
    }
    *pData = NULL;
    *pDataLength = 0;

    // TEST
    // TEST_ValidateCacheConsistency(cache);

    // Check if the entry is in the hash table
    entry = CSL_HashTableFind(cache->hashTable, name, (uint32)nameLength);
    if (!entry)
    {
        // Entry does not exist
        return NE_NOENT;
    }

    //Check - is entry expired
    if (entry->expireTime > 0 && (PAL_ClockGetGPSTime() >= entry->expireTime))
    {
        //Remove entry and return NOENT
        CSL_CacheClearOne(cache, name, nameLength);
        return NE_NOENT;
    }

    // If the memory-only flag is set and the entry is only in the persistent cache then we return
    if (checkMemoryCacheOnly && (! entry->memoryLink))
    {
        nsl_assert(entry->persistentLink);
        return NE_NOENT;
    }

    // Is the entry in memory
    if (entry->memoryLink)
    {
        // Move to top of priority linked list
        MoveToTop(&(cache->priorityListMemory), entry->memoryLink);

        // If the entry is also in the persistent cache then we have to move that entry to the top of the persistent linked list as well
        if (entry->persistentLink)
        {
            MoveToTop(&(cache->priorityListPersistent), entry->persistentLink);
        }
    }
    // Entry is in persistent cache only
    else
    {
        // The entry has to be in the persistent cache
        nsl_assert(entry->persistentLink);

        result = LoadEntryFromPersistent(cache, entry);
        if (result != NE_OK)
        {
            return result;
        }
    }
    entry->accessed = TRUE;

    if (makeCopy)
    {
        *pData = nsl_malloc(entry->persistentEntry.dataLength);
        if (*pData == NULL)
        {
            return NE_NOMEM;
        }
        nsl_memcpy(*pData, entry->data, entry->persistentEntry.dataLength);
    }
    else
    {
        // Return pointer to existing data
        *pData = entry->data;
    }
    *pDataLength = entry->persistentEntry.dataLength;

    // Set the protected flag if parameter is set. If the entry is already proteced don't reset internal flag.
    if (protect)
    {
        entry->protect = TRUE;
    }

    return NE_OK;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheClear(CSL_Cache* cache)
{
    return CSL_CacheClearExtended(cache, CSL_CF_All);
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheClearExtended(CSL_Cache* cache, CSL_ClearOption clearOption)
{
    switch (clearOption)
    {
        case CSL_CF_All:
        {
            // First delete all data associated with all the hash table entries. This also deletes all persistent data
            // (files on disk by default)
            CSL_HashTableForEach(cache->hashTable, &ClearBothEntries, cache);

            // Now delete all the elements in the hash table
            CSL_HashTableDeleteAll(cache->hashTable);

            // Clear both linked lists
            ClearList(&cache->priorityListMemory);
            ClearList(&cache->priorityListPersistent);

            if (cache->filenumbersInUse)
            {
                // Clear filenumber vector and reset the next available filenumber to one (zero is the index file)
                void* filenumberData = CSL_VectorGetPointer(cache->filenumbersInUse, 0);
                nsl_memset(filenumberData, 0, CSL_VectorGetItemSize(cache->filenumbersInUse) * CSL_VectorGetLength(cache->filenumbersInUse));
                cache->nextAvailableFilenumber = 1;
            }
            break;
        }
        case CSL_CF_MemoryOnly:
        {
            /*
                Walk the memory linked list and free all memory data. The hash table entries only get
                removed if the entries don't have any associated persistent data.
            */

            LinkedEntry* current = cache->priorityListMemory.first;
            for (; current; current = current->next)
            {
                ClearMemoryEntry(cache, current->entry);
            }

            // Now clear the entire memory priority linked list.
            ClearList(&cache->priorityListMemory);
            break;
        }
        case CSL_CF_UnprotectedOnly:
        {
            LinkedEntry* current = cache->priorityListMemory.first;
            LinkedEntry* temp = NULL;

            /* Walk the memory priority linked list and remove only the non-protected entries. */
            while (current)
            {
                // If the entry is protected then just move on to the next
                if (current->entry->protect)
                {
                    current = current->next;
                }
                else
                {
                    ClearMemoryEntry(cache, current->entry);

                    // Advance to the next element and then delete the current entry. This works since removing
                    // the current entry will not alter the following entry in the linked list.
                    temp = current;
                    current = current->next;
                    RemoveFromList(&cache->priorityListMemory, temp);
                }
            }
            break;
        }
    }

    // TEST
    // TEST_ValidateCacheConsistency(cache);

    return NE_OK;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheClearOne(CSL_Cache* cache, byte* name, size_t nameLength)
{
    // Check if the entry exists
    CacheEntry* entry = CSL_HashTableFind(cache->hashTable, name, (uint32)nameLength);
    if (! entry)
    {
        return NE_NOENT;
    }

    if (entry->protect)
    {
        // This cache entry is locked by UI and cannot be removed from cache right now.
        return NE_DECLINED;
    }

    // Clear both flags from the filenumber vector
    UpdateFilenumberFlags(cache, entry->persistentEntry.filenumber, &FLAG_FALSE, &FLAG_FALSE);

    // Remove entry from both priority linked lists (if set).
    RemoveFromList(&cache->priorityListMemory, entry->memoryLink);
    RemoveFromList(&cache->priorityListPersistent, entry->persistentLink);

    // Clear memory and persistent data. This also deletes the entry itself.
    ClearBothEntries(entry, cache);

    // Delete the entry from the hash table
    if (! CSL_HashTableDelete(cache->hashTable, name, (uint32)nameLength))
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

/* See header for description */
CSL_DEF void
CSL_CacheForEach(CSL_Cache* cache, CSL_CacheUserFunction callback, void* userData, nb_boolean getData)
{
    /*
        ! This does only iterate through the memory cache entries. (otherwise we would have to load all
        persistent entries which then in itself would invalidate some of the memory cache entries.)
    */

    LinkedEntry* current = cache->priorityListMemory.first;
    for (; current; current = current->next)
    {
        CacheEntry* entry = current->entry;

        callback(0 /* We don't use timestamps anymore */,
                 entry->persistentEntry.key,
                 entry->persistentEntry.keyLength,
                 entry->data,
                 entry->persistentEntry.dataLength,
                 userData);
    }
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheProtect(CSL_Cache* cache, byte* name, size_t nameLength)
{
    CacheEntry* entry = CSL_HashTableFind(cache->hashTable, name, (uint32)nameLength);
    if (entry == NULL)
    {
        return NE_NOENT;
    }

    entry->protect = TRUE;
    return NE_OK;
}

/* See header for description */
CSL_DEF void
CSL_CacheDump(CSL_Cache* cache, const char* caption)
{
    if (cache->dumpFunction)
    {
        cache->dumpFunction(cache, caption);
    }
}

/* See header for description */
CSL_DEF void
CSL_CacheClearAccessed(CSL_Cache* cache)
{
    // Go through all memory cache entries and clear the accessd flag
    LinkedEntry* current = cache->priorityListMemory.first;
    for (; current; current = current->next)
    {
        current->entry->accessed = FALSE;
    }
}

/* See header for description */
CSL_DEF void
CSL_CacheClearProtect(CSL_Cache* cache)
{
    // Go through all memory cache entries and clear the protected flag
    LinkedEntry* current = cache->priorityListMemory.first;
    for (; current; current = current->next)
    {
        current->entry->protect = FALSE;
    }
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheClearProtectOne(CSL_Cache* cache, byte* name, size_t nameLength)
{
    CacheEntry* entry = CSL_HashTableFind(cache->hashTable, name, (uint32)nameLength);
    if (entry == NULL)
    {
        return NE_NOENT;
    }

    entry->protect = FALSE;
    return NE_OK;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSetFunctions(CSL_Cache* cache,
                      CSL_CacheSaveIndexFunction saveIndexFunction,
                      CSL_CacheSaveFunction saveFunction,
                      CSL_CacheLoadIndexFunction loadIndexFunction,
                      CSL_CacheLoadFunction loadFunction,
                      CSL_CacheRemoveFunction removeFunction,
                      CSL_CacheDumpFunction dumpFunction)
{
    if (!cache)
    {
        return NE_INVAL;
    }

    if (saveIndexFunction)
    {
        cache->saveIndexFunction = saveIndexFunction;
    }

    if (saveFunction)
    {
        cache->saveFunction = saveFunction;
    }

    if (loadIndexFunction)
    {
        cache->loadIndexFunction = loadIndexFunction;
    }

    if (loadFunction)
    {
        cache->loadFunction = loadFunction;
    }

    if (removeFunction)
    {
        cache->removeFunction = removeFunction;
    }

    if (dumpFunction)
    {
        cache->dumpFunction = dumpFunction;
    }

    return NE_OK;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSetNotificationFunctions(CSL_Cache* cache,
                                  CSL_CacheRemoveNotificationFunction removeNotificationFunction,
                                  void* userData)
{
    // Update notification function. Don't check for NULL, the pointer can be NULL to reset it.
    cache->removeNotificationFunction = removeNotificationFunction;
    cache->userDataNotifications = userData;
    return NE_OK;
}

/* See header for description */
CSL_DEF nb_boolean
CSL_CacheIsItemInCache(CSL_Cache* cache, byte* name, size_t nameLength)
{
    void* entry = CSL_HashTableFind(cache->hashTable, name, (uint32)nameLength);
    if (entry)
    {
        return TRUE;
    }
    return FALSE;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheSetUserData(CSL_Cache* cache, byte* data, size_t size)
{
    // Delete any old data
    if (cache->userData)
    {
        nsl_free(cache->userData);
        cache->userData = NULL;
        cache->userDataSize = 0;
    }

    if (data && (size > 0))
    {
        // Make a copy of the data. It gets saved to the index-file the next time the index-file gets saved.
        cache->userData = nsl_malloc(size);
        if (!cache->userData)
        {
            return NE_NOMEM;
        }
        nsl_memcpy(cache->userData, data, size);
        cache->userDataSize = size;
    }

    return NE_OK;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheGetUserData(CSL_Cache* cache, byte** data, size_t* size)
{
    // Return pointer to the internal data. The caller of this function has to make a copy of the data.
    *data = cache->userData;
    *size = cache->userDataSize;
    return NE_OK;
}

/* See header for description */
CSL_DEF NB_Error
CSL_CacheValidateFilesOnDisk(CSL_Cache* cache)
{
    NB_Error err = NE_OK;
    CSL_Vector* filenumbersInCache = NULL;
    LinkedEntry* current = NULL;

    // This function only gets rid of any orphaned numbered files. It has no effect for non-obfuscated cache entries
    if ((cache == NULL) || (cache->obfuscateKey == NULL))
    {
        return NE_NOTINIT;
    }

    filenumbersInCache = CSL_VectorAlloc(sizeof(uint32));
    if (!filenumbersInCache)
    {
        return NE_NOMEM;
    }

    // Get all entries in the persistent cache and add them to our temporary vector
    current = cache->priorityListPersistent.first;
    while (current)
    {
        if (!CSL_VectorAppend(filenumbersInCache, &current->entry->persistentEntry.filenumber))
        {
            err = NE_NOMEM;
            goto cleanup;
        }
        current = current->next;
    }

    // Sort the filenumber vector, ValidateFilesOnDisk() expect a sorted vector.
    CSL_VectorSort(filenumbersInCache, &CompareFilenumber);

    err = ValidateFilesOnDisk(cache, filenumbersInCache);

cleanup:
    CSL_VectorDealloc(filenumbersInCache);
    return err;
}


// Local functions ...............................................................................

/*! Return formatted filename.

    @return None
*/
void
GetFilename(CSL_Cache* cache,
             uint32 filenumber,     /*!< file-number of file-name to generate */
             char* filename         /*!< Buffer for filename. Has to be of length FILENAME_LENGTH */
             )
{
    // First format the file name
    char tempFilename[FILENAME_LENGTH] = {0};
    nsl_snprintf(tempFilename, FILENAME_LENGTH, FILENAME_FORMAT, filenumber);

    // Now add the cache name and the file name together. That's what we use to save the file
    nsl_strcpy(filename, cache->name);
    PAL_FileAppendPath(cache->pal, filename, FILENAME_LENGTH, tempFilename);
}

/*! Return formatted filename for cache entries using un-obfuscated file names.

    The returned string has to be freed using nsl_free().

    @return filename or NULL on failure.
*/

char*
GetUnobfuscatedFilename(CSL_Cache* cache,
                        const byte* name        /*!< Name of cache entry */
                        )
{
    // Get size of both strings. Add some buffer for null termination and delimiter
    size_t size = nsl_strlen(cache->name) + nsl_strlen((const char*)name) + 50;
    char* filename = nsl_malloc(size);
    if (! filename)
    {
        return NULL;
    }
    filename[0] = '\0';

    // We use the cache name and the cache-entry name as our path
    nsl_strcpy(filename, cache->name);

    // Now add the cache entry to the path
    PAL_FileAppendPath(cache->pal, filename, (uint32)size, (const char*)name);
    return filename;
}

/*! Validate that there are no orphaned cache files on disk.

    We check that we don't have any orphaned cache entries in the file system.
    This could happen if the cache was not shut down correctly and/or CSL_CacheSaveIndex() was
    not called before destroying the cache.

    @see CSL_CacheLoadIndex
*/
NB_Error
ValidateFilesOnDisk(CSL_Cache* cache,
                    CSL_Vector* filenumbersInPersistentCache  /*!< Filenumbers currently in use by the cache. This vector has to be
                                                                   sorted before calling this function! Contains uint32 entries. */
                    )
{
    /*
        We create a vector of all filenumbers used in the filesystem (in the cache folder).
        We already have a vector with all filenumbers used in the cache (see parameter).
        We create a third vector which is the difference between the two vectors to get all orphaned files.
        Both input vectors have to be sorted. This approach is a lot faster then searching through the
        entire cache for every single file on disk.
    */
    CSL_Vector* filenumbersOnDisk = NULL;
    CSL_Vector* orphanedFilenumbers = NULL;
    PAL_FileEnum* enumerator = NULL;
    PAL_FileEnumInfo info = {0};
    NB_Error err = NE_OK;

    // Only check if no user functions were provided and we're using the default file-cache.
    if (cache->loadIndexFunction != &LoadIndexFunctionDefault)
    {
        return NE_INVAL;
    }

    if (!PAL_FileIsDirectory(cache->pal, cache->name))
    {
        return NE_NOENT;
    }

    filenumbersOnDisk = CSL_VectorAlloc(sizeof(uint32));
    if (!filenumbersOnDisk)
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    orphanedFilenumbers = CSL_VectorAlloc(sizeof(uint32));
    if (!orphanedFilenumbers)
    {
        err = NE_NOMEM;
        goto cleanup;
    }

    // Go through all the cache files on disk and save the filenumber (which is the name of the file) to our temporary vector.
    if (PAL_FileEnumerateCreate(cache->pal, cache->name, FALSE, &enumerator) == PAL_Ok)
    {
        // For all files in the cache folder
        while (PAL_FileEnumerateNext(enumerator, &info) == NE_OK)
        {
            // The filename is just the number of the cache entry
            int fileNumber = nsl_atoi(info.filename);
            if (fileNumber != FILENUM_INDEX)
            {
                if (!CSL_VectorAppend(filenumbersOnDisk, &fileNumber))
                {
                    err = NE_NOMEM;
                    goto cleanup;
                }
            }
        }

        PAL_FileEnumerateDestroy(enumerator);
        enumerator = NULL;
    }

    // Sort the filenumbers so that we can call CSL_VectorDifference().
    CSL_VectorSort(filenumbersOnDisk, &CompareFilenumber);

    // Get the vector of all filenumbers which are on disk but not in the cache. These are our orphaned files.
    if (CSL_VectorDifference(filenumbersOnDisk, filenumbersInPersistentCache, orphanedFilenumbers, &CompareFilenumber))
    {
        int size = CSL_VectorGetLength(orphanedFilenumbers);
        if (size > 0)
        {
            int i = 0;
            uint32* fileNumber = CSL_VectorGetPointer(orphanedFilenumbers, 0);

            // Delete all orphaned files
            for (i = 0; i < size; ++i)
            {
                char filename[FILENAME_LENGTH] = {0};
                GetFilename(cache, fileNumber[i], filename);

                if (PAL_FileRemove(cache->pal, filename) != PAL_Ok)
                {
                    err = NE_FSYS;
                    goto cleanup;
                }
            }
        }
    }

cleanup:
    if (enumerator)
    {
        PAL_FileEnumerateDestroy(enumerator);
        enumerator = NULL;
    }
    if (filenumbersOnDisk)
    {
        CSL_VectorDealloc(filenumbersOnDisk);
    }
    if (orphanedFilenumbers)
    {
        CSL_VectorDealloc(orphanedFilenumbers);
    }
    return err;
}

/*! Initialize/Update all the persistent filenumber flags based on the given vector.

    This function has to be called when loading persistent cache entries during startup.

    @see CSL_CacheLoadIndex
*/
void
InitializeFilenumbersInUse(CSL_Cache* cache,
                           CSL_Vector* filenumbersInPersistentCache     /*!< Filenumbers currently in use by the cache. Contains uint32 entries. */
                           )
{
    int i = 0;
    int count = CSL_VectorGetLength(filenumbersInPersistentCache);
    if (count > 0)
    {
        uint32* pValue = CSL_VectorGetPointer(filenumbersInPersistentCache, 0);

        // Walk the input vector and set the persistent flag for every filenumber in use
        for (i = 0; i < count; ++i)
        {
            // Don't set flag directly. Use UpdateFilenumberFlags() function.
            UpdateFilenumberFlags(cache, *pValue++, NULL, &FLAG_TRUE);
        }
    }
}

/*! Update memory and/or persistent filenumber flags.

    This function has to be called each time a cache entry is added/removed from either the
    memory or persistent cache. The filenumbers vector is used to calculate the next available
    filenumber.

    Never adjust a flag in the 'filenumbersInUse' vector directly!
*/
void
UpdateFilenumberFlags(CSL_Cache* cache,
                      uint32 filenumber,                    /*!< Filenumber for which to update the flags */
                      const nb_boolean* memoryFlag,         /*!< New memory flag. Set to NULL if flag should not be modified */
                      const nb_boolean* persistentFlag      /*!< New persistent flag. Set to NULL if flag should not be modified */
                      )
{
    FilenumberUsage* pValue = NULL;
    uint32 totalCount = 0;

    // We only use the filenumber vector if we obfuscate files
    if (! cache->obfuscateEntries)
    {
        return;
    }

    totalCount = CSL_VectorGetLength(cache->filenumbersInUse);

    // Double check input
    if (filenumber >= totalCount)
    {
        // This should never happen
        nsl_assert(FALSE);
        return;
    }

    pValue = CSL_VectorGetPointer(cache->filenumbersInUse, filenumber);

    // Update flags
    if (memoryFlag)
    {
        pValue->memory = *memoryFlag;
    }
    if (persistentFlag)
    {
        pValue->persistent = *persistentFlag;
    }

    // Update the next-available filenumber flag if necessary.

    if (filenumber < cache->nextAvailableFilenumber)
    {
        // If both flags are cleared then we use it as the next available filenumber
        if ((pValue->memory == FALSE) && (pValue->persistent == FALSE))
        {
            cache->nextAvailableFilenumber = filenumber;
        }
    }
    else if (filenumber == cache->nextAvailableFilenumber)
    {
        // If any of the flags got set then we need to search for a new available filenumber
        if (pValue->memory || pValue->persistent)
        {
            uint32 i = 0;
            for (i = filenumber + 1; i < totalCount; ++i)
            {
                // If both flags are cleared then we use it
                pValue = CSL_VectorGetPointer(cache->filenumbersInUse, i);
                if ((pValue->memory == FALSE) && (pValue->persistent == FALSE))
                {
                    cache->nextAvailableFilenumber = i;
                    break;
                }
            }
            // We should never reach the end of the vector
            nsl_assert(i < totalCount);
        }
    }
}

/*! Get the next available filenumber.

    @return filenumber.
*/
uint32
GetNextFilenumber(CSL_Cache* cache)
{
    // We only use the filenumber vector if we obfuscate files
    if (cache->obfuscateEntries)
    {
        uint32 result = cache->nextAvailableFilenumber;

        int count = CSL_VectorGetLength(cache->filenumbersInUse);
        FilenumberUsage* pValue = CSL_VectorGetPointer(cache->filenumbersInUse, result + 1);
        int i = 0;

        // Walk the vector to find the next entry which has both flags (memory and persistent) cleared.
        for (i = result + 1; i < count; i++)
        {
            if (! (pValue->memory || pValue->persistent))
            {
                // Update the next available filenumber and return our initial next-available filenumber
                cache->nextAvailableFilenumber = i;
                return result;
            }
            pValue++;
        }

        // We should never reach the end
        nsl_assert(FALSE);
        return 1;
    }
    else
    {
        // Not used.
        return 1;
    }
}

/*! Compare function for filenumbers.

    We want the filenumbers to be sorted in ascending order.

    @see CSL_VectorCompareFunction
*/
int
CompareFilenumber(const void* left, const void* right)
{
    if (*((int*)left) < *((int*)right))
    {
        return -1;
    }
    if (*((int*)left) > *((int*)right))
    {
        return 1;
    }
    return 0;
}

/*! Dump/Output cache content.

    Not used.
*/
void
Dump(CSL_Cache* cache, const char* caption)
{
    // Printout cache content. Currently not used!
}

/*! Save entry to persistent cache.

    This calls the save-function. By default it will save the entry to file.

    This will also add an entry to the index-vector.

    @return NB_Error
*/
NB_Error
SaveEntryToPersistent(CSL_Cache* cache,
                      CacheEntry* entry        /*!< Entry to save */
                      )
{
    NB_Error result = NE_OK;
    PersistentEntry* info = &entry->persistentEntry;

    // Call save function. This can be a user provided function or by default will save the entry to a file
    result = cache->saveFunction(cache, info->filenumber, info->key, info->keyLength, entry->data, info->dataLength);
    if (result == NE_OK)
    {
        // If the entry already exists then we just move the entry to the top of the priority list
        if (entry->persistentLink)
        {
            MoveToTop(&cache->priorityListPersistent, entry->persistentLink);
        }
        else
        {
            // Set persistent filenumber flag
            UpdateFilenumberFlags(cache, info->filenumber, NULL, &FLAG_TRUE);

            // Add entry to priority linked list
            if (! AddToTop(&cache->priorityListPersistent, entry, FALSE))
            {
                result = NE_NOMEM;
            }
        }
    }

    return result;
}

/*! Save accessed or dirty entries to persistent cache.

    This function gets called from CSL_CacheSaveAccessed() and CSL_CacheSaveDirty(). It goes through
    all entries and saves the accessed and/or dirty items to the persistent cache. It only saves up to
    'maximumEntries' items.

    @return NB_Error
    @see CSL_CacheSaveAccessed
    @see CSL_CacheSaveDirty
*/
NB_Error
SaveInternal(CSL_Cache* cache,
             int32 maximumEntries,          /*!< Maximum number of entries to save to the persistent cache. Set to '-1' to save all entries. */
             int32* savedEntries,           /*!< On return the number of saved entries. Set to NULL if not needed. */
             nb_boolean removeFromMemory,   /*!< If set to 'TRUE' the entries will be removed from the memory cache. Protected items will not be removed from memory. */
             nb_boolean saveAccessed,       /*!< If set to 'TRUE' it will save any accessed items */
             nb_boolean saveDirty           /*!< If set to 'TRUE' it will save any dirty items */
             )
{
    LinkedEntry* current = NULL;
    LinkedEntry* temp = NULL;
    int savedEntriesInternal = 0;

    // Check if we should go through all entries
    if (maximumEntries == -1)
    {
        maximumEntries = cache->priorityListMemory.count;
    }

    // Walk memory linked list.
    current = cache->priorityListMemory.first;
    while (current && (savedEntriesInternal < maximumEntries))
    {
        nb_boolean itemRemoved = FALSE;
        CacheEntry* entry = current->entry;

        if ((saveAccessed && entry->accessed) || (saveDirty && entry->dirty))
        {
            // Only save entry if it is not already in the persistent cache
            if (! entry->persistentLink)
            {
                // Save the entry to the persistent cache
                if (SaveEntryToPersistent(cache, entry) == NE_OK)
                {
                    // No need to update filenumber flag, it is done in SaveEntryToPersistent().

                    savedEntriesInternal++;
                }
                else
                {
                    return NE_FSYS;
                }
            }

            // Reset flags
            entry->accessed = FALSE;
            entry->dirty = FALSE;

            // Check if we should remove the item from memory. Don't remove protected entries.
            if (removeFromMemory && !entry->protect)
            {
                ClearMemoryEntry(cache, entry);

                // Advance to the next element and then delete the current entry. This works since removing
                // the current entry will not alter the following entry in the linked list.
                temp = current;
                current = current->next;
                RemoveFromList(&cache->priorityListMemory, temp);

                itemRemoved = TRUE;
            }
        }

        if (! itemRemoved)
        {
            current = current->next;
        }
    }

    if (savedEntriesInternal > 0)
    {
        NB_Error result = NE_OK;

        // Set the index-file to invalid to indicate that is now out-of-synch with our memory representation
        SetIndexFileToInvalid(cache);

        // Check if the maximum count for the persistent cache was exceeded and remove the oldest
        // element from the persistent cache.
        result = RemoveExcessEntries(cache);
        if (result != NE_OK)
        {
            return result;
        }
    }

    if (savedEntries)
    {
        *savedEntries = savedEntriesInternal;
    }

    return NE_OK;
}

/*! Default save-index-function.

    See CSL_CacheSaveIndexFunction() for more details.

    @return NB_Error
    @see CSL_CacheSaveIndexFunction
*/
NB_Error
SaveIndexFunctionDefault(CSL_Cache* cache, const byte* data, uint32 length)
{
    char filename[FILENAME_LENGTH] = {0};
    GetFilename(cache, FILENUM_INDEX, filename);

    // Create the directory if it doesn't already exist
    if (!PAL_FileIsDirectory(cache->pal, cache->name))
    {
        if (PAL_FileCreateDirectory(cache->pal, cache->name) != PAL_Ok)
        {
            return NE_FSYS;
        }
    }

    // Remove any old index file if old index exists.
    if ((PAL_FileExists(cache->pal, filename) == PAL_Ok) &&
        (PAL_FileRemove(cache->pal, filename) != PAL_Ok))
    {
        return NE_FSYS;
    }

    // Save the index file.
    if (!savefile_obfuscate(cache->pal, PFM_Create, filename, (uint8*) data, length, cache->obfuscateKey,
                            (uint32)cache->obfuscateKeyLen))
    {
        return NE_FSYS;
    }

    return NE_OK;
}

/*! Default save-function.

    See CSL_CacheSaveFunction() for more details.

    @return NB_Error
    @see CSL_CacheSaveFunction
*/
NB_Error
SaveFunctionDefault(CSL_Cache* cache, uint32 fileNumber, const byte* name, size_t nameLength, const byte* data, uint32 length)
{
    NB_Error result = NE_OK;

    // Create the directory if it doesn't already exist
    if (!PAL_FileIsDirectory(cache->pal, cache->name))
    {
        if (PAL_FileCreateDirectory(cache->pal, cache->name) != PAL_Ok)
        {
            return NE_FSYS;
        }
    }

    // Should we obfuscate file names
    if (cache->obfuscateEntries)
    {
        char filename[FILENAME_LENGTH] = {0};
        GetFilename(cache, fileNumber, filename);

        /* Save entry to the file system. */
        if (!savefile_obfuscate(cache->pal, PFM_Create, filename, (uint8*)data, length, cache->obfuscateKey,
                                (uint32)cache->obfuscateKeyLen))
        {
            result = NE_FSYS;
        }
    }
    else
    {
        char* filename = GetUnobfuscatedFilename(cache, name);
        if (filename)
        {
            // Use the key/name of the cache entry as file name and don't obfuscate the content
            if (!savefile(cache->pal, PFM_Create, filename, (uint8*)data, length))
            {
                result = NE_FSYS;
            }
            nsl_free(filename);
        }
        else
        {
            result = NE_NOMEM;
        }
    }

    return result;
}

/*! Set the invalid flag to the index file.

    The index file will be out-of-sync during normal operation. That is because we don't want to
    save the entire index-file each time we add a new entry to the persistent cache. We instead
    just set the index-file to invalid the first time we save a new entry. Once the cache gets
    shut down properly the index file gets saved correctly.

    Should the application shut down improperly then we know the next time we load the index
    file that it is invalid and ignore all entries.

    @return None
*/
void
SetIndexFileToInvalid(CSL_Cache* cache)
{
    // Only save the flag once
    if (!cache->indexFileInvalid)
    {
        byte* data = NULL;
        uint32 size = 0;

        cache->indexFileInvalid = TRUE;

        // Load the index-file and check the file header version. If we have an old index-file then this will fail.
        if (cache->loadIndexFunction && cache->loadIndexFunction(cache, &data, &size) == NE_OK)
        {
            if (size >= sizeof(IndexFileHeader))
            {
                IndexFileHeader* header = (IndexFileHeader*)data;
                if (header->fileVersion == CURRENT_INDEX_FILE_VERSION)
                {
                    header->valid = FALSE;

                    // Write the data back to the index file.
                    if (cache->saveIndexFunction)
                    {
                        cache->saveIndexFunction(cache, data, size);
                    }
                }
            }

            nsl_free(data);
        }
    }
}

/*! Load an entry from the persistent cache to the memory cache.

    @return NE_OK if the entry was loaded; NE_NOENT if the item was not found in the persistent cache
*/
NB_Error
LoadEntryFromPersistent(CSL_Cache* cache,
                        CacheEntry* entry       /*!< The entry to load from persistent cache. The entry has to be valid. */
                        )
{
    NB_Error result = NE_OK;

    // If the entry is not in the persistent cache then the return.
    if (! entry->persistentLink)
    {
        nsl_assert(FALSE);
        return NE_NOENT;
    }

    // It is assumed that the entry is not yet in the memory cache
    nsl_assert(! entry->memoryLink);

    // We currently assign the file number when the hash-table entry is created, even if the entry is only
    // stored in memory. We therefore should always have a valid filenumber.
    nsl_assert(entry->persistentEntry.filenumber > 0);

    // Load the entry from the persistent cache. By default this will load it from the file system
    result = cache->loadFunction(cache,
                                 entry->persistentEntry.filenumber,
                                 entry->persistentEntry.key,
                                 entry->persistentEntry.keyLength,
                                 &entry->data,
                                 &entry->persistentEntry.dataLength);
    if (result != NE_OK)
    {
        return result;
    }

    // Reset dirty flag, the memory and persistent cache are now in-sync.
    entry->dirty = FALSE;

    // We access the persistent entry, move it to the top of the linked list
    MoveToTop(&cache->priorityListPersistent, entry->persistentLink);

    // Add new linked-list entry to the memory priority list
    if (! AddToTop(&cache->priorityListMemory, entry, TRUE))
    {
        return NE_NOMEM;
    }

    // Set memory flag for filenumbers
    UpdateFilenumberFlags(cache, entry->persistentEntry.filenumber, &FLAG_TRUE, NULL);

    // Remove element from memory cache if it is full
    return RemoveLastElementFromMemory(cache);
}

/*! Default load-index-function.

    See CSL_CacheLoadIndexFunction() for more details.

    @return NB_Error
    @see CSL_CacheLoadIndexFunction
*/
NB_Error
LoadIndexFunctionDefault(CSL_Cache* cache, byte** data, uint32* length)
{
    nb_size readResult = 0;
    char filename[FILENAME_LENGTH] = {0};
    GetFilename(cache, FILENUM_INDEX, filename);

    // Read the index-file and return the result
    readResult = readfile_obfuscate(cache->pal, filename, (void**)data, cache->obfuscateKey,
                                            (uint32)cache->obfuscateKeyLen);
    if (readResult > 0)
    {
        *length = readResult;
        return NE_OK;
    }

    return NE_FSYS;
}

/*! Default load-function.

    See CSL_CacheLoadFunction() for more details.

    @return NB_Error
    @see CSL_CacheLoadFunction
*/
NB_Error
LoadFunctionDefault(CSL_Cache* cache, uint32 fileNumber, const byte* name, size_t nameLength, byte** data, uint32* length)
{
    if (cache->obfuscateEntries)
    {
        char filename[FILENAME_LENGTH] = {0};
        GetFilename(cache, fileNumber, filename);

        // Check if the file exists
        if (PAL_FileExists(cache->pal, filename) != PAL_Ok)
        {
            return NE_NOENT;
        }

        // Read the data from file. The function returns '-1' on error.
        *length = (uint32)readfile_obfuscate(cache->pal, filename, (void**)data, cache->obfuscateKey,
                                             (uint32)cache->obfuscateKeyLen);
    }
    else
    {
        // Use the cache name as filename. Don't obfuscate the content
        char* filename = GetUnobfuscatedFilename(cache, name);
        if (filename)
        {
            // Check if the file exists
            if (PAL_FileExists(cache->pal, filename) == PAL_Ok)
            {
                // Read the data from file. The function returns '-1' on error.
                *length = (uint32)readfile(cache->pal, filename, (void**)data);
            }
            else
            {
                nsl_free(filename);
                return NE_NOENT;
            }
            nsl_free(filename);
        }
    }

    return ((*length > 0) ? NE_OK : NE_FSYS);
}

/*! Delete the memory and persistent data associated with the cache entry.

    This function is used as a callback in CSL_CacheClearExtended() but can also be called directly
    passing in the entry and the cache pointer.

    This function does not remove the entry from the hash table and does not update the filenumber vector.

    @see CSL_CacheClearExtended
    @return None
*/
void
ClearBothEntries(void* value,           /*!< The hash table entry */
                 void* userData         /*!< Pointer to CSL_Cache */
                 )
{
    CacheEntry* entry = (CacheEntry*)value;
    CSL_Cache* cache = (CSL_Cache*)userData;

    // If we have memory data, remove it
    if (entry->memoryLink)
    {
        nsl_assert(entry->data);
        nsl_free(entry->data);
    }

    // If the entry has a persistent entry then remove it
    if (entry->persistentLink)
    {
        // Remove the entry from the persistent cache.
        NB_Error result = cache->removeFunction(cache, entry->persistentEntry.filenumber, entry->persistentEntry.key, entry->persistentEntry.keyLength);
        if (result == NE_OK)
        {
            // Call notification function, but only if it is set
            if (cache->removeNotificationFunction)
            {
                cache->removeNotificationFunction(cache, entry->persistentEntry.key, entry->persistentEntry.keyLength, cache->userDataNotifications);
            }
        }
    }
    ClearKey(&entry->persistentEntry);
    nsl_free(entry);
}

/*! Clear the memory link from the given cache element and remove it from the hash table if empty.

    With 'empty' we mean having no memory and no persistent entry anymore. This also resets the filenumber vector.

    @return None
*/
void
ClearMemoryEntry(CSL_Cache* cache, CacheEntry* entry)
{
    // This should only be called if we actually have a memory entry
    nsl_assert(entry->data);
    nsl_assert(entry->memoryLink);

    // Clear memory flag in filenumbers vector
    UpdateFilenumberFlags(cache, entry->persistentEntry.filenumber, &FLAG_FALSE, NULL);

    // Remove link to priority list
    entry->memoryLink = NULL;

    // Free the memory data
    nsl_free(entry->data);
    entry->data = NULL;
    // Don't reset the data length field. It is shared with the persistent entry

    // If the persistent link entry is empty then we can remove the entry from the hash table.
    if (! entry->persistentLink)
    {
        CSL_HashTableDelete(cache->hashTable, entry->persistentEntry.key, entry->persistentEntry.keyLength);
        ClearKey(&entry->persistentEntry);
        nsl_free(entry);
    }
}

/*! Check if the memory cache is full and if so remove the last non-protected element.

    This function is called after we add a new element to the memory cache. It validates
    that we don't exceed the maximum count.

    @return NB_Error
*/
NB_Error
RemoveLastElementFromMemory(CSL_Cache* cache)
{
    LinkedEntry* current = NULL;

    // If we didn't reach the maximum then we don't need to do anything
    if (cache->priorityListMemory.count <= (unsigned)cache->maximumItemsMemory)
    {
        return NE_OK;
    }

    // there might more than one excess item if all entries are protected
    //
    // We should only have one excess item
    //nsl_assert(cache->priorityListMemory.count == (unsigned)(cache->maximumItemsMemory + 1));

    /*
        The linked list is sorted by last accessed time. That means that the oldest items are at the
        end of the list. Search the list backwards and remove the first non-protected item.
        Don't check the first element, that's the one we just added.
    */

    current = cache->priorityListMemory.last;
    while (current != cache->priorityListMemory.first)
    {
        CacheEntry* entry = current->entry;

        // Is the item not protected
        if (! entry->protect)
        {
            // The current pointer always has to match the cross-link between the hash table entry and the linked list
            nsl_assert(entry->memoryLink == current);

            ClearMemoryEntry(cache, entry);

            // Remove from the memory linked list.
            RemoveFromList(&(cache->priorityListMemory), current);
            return NE_OK;
        }

        current = current->previous;
    }

    // If we reach this then all items are protected. That should never happen (except in unit tests)
    return NE_RES;
}

/*! Remove any entries in the persistent cache which exceed the maximum count.

    @return NB_Error
*/
NB_Error
RemoveExcessEntries(CSL_Cache* cache)
{
    NB_Error result = NE_OK;
    LinkedEntry* current = NULL;
    LinkedEntry* temp = NULL;

    // Only proceed if we exceeded the maximum count
    if (cache->priorityListPersistent.count <= (unsigned)cache->maximumItemsPersistent)
    {
        return NE_OK;
    }

    /*
        The priority lists are sorted by last accessed time. That means the oldest elements are at the
        end of the list. Walk the persistent priority list backwards to remove items.
    */
    current = cache->priorityListPersistent.last;
    while (current && (cache->priorityListPersistent.count > (unsigned)cache->maximumItemsPersistent))
    {
        CacheEntry* entry = current->entry;
        PersistentEntry* info = &entry->persistentEntry;

        // Remove the entry from the persistent cache. By default this will removed the file.
        NB_Error removeResult = cache->removeFunction(cache, info->filenumber, info->key, info->keyLength);
        if (removeResult == NE_OK)
        {
            // Call notification function, but only if it is set
            if (cache->removeNotificationFunction)
            {
                cache->removeNotificationFunction(cache, info->key, info->keyLength, cache->userDataNotifications);
            }
        }
        else
        {
            // Don't abort if one file failes. Delete all the other files.
            result = removeResult;
        }

        // Clear persistent filenumber flag
        UpdateFilenumberFlags(cache, info->filenumber, NULL, &FLAG_FALSE);

        entry->persistentLink = NULL;

        // If this element is not in the memory cache then we can remove the hash table entry
        if (! entry->memoryLink)
        {
            if (CSL_HashTableDelete(cache->hashTable, info->key, info->keyLength))
            {
                ClearKey(info);
                nsl_free(entry);
            }
            else
            {
                result = NE_NOMEM;
            }
        }

        // Move to previous linked element and remove the current one from the linked list
        temp = current;
        current = current->previous;
        RemoveFromList(&cache->priorityListPersistent, temp);
    }

    return result;
}

/*! Default remove/delete function.

    This will delete one entry from the file system.

    @return NB_Error
    @see CSL_CacheRemoveFunction
*/
NB_Error
RemoveFunctionDefault(CSL_Cache* cache,     // See parameter description for CSL_CacheRemoveFunction().
                      uint32 fileNumber,
                      const byte* name,
                      size_t nameLength)
{
    NB_Error result = NE_OK;

    if (cache->obfuscateEntries)
    {
        char filename[FILENAME_LENGTH] = {0};
        GetFilename(cache, fileNumber, filename);

        // Remove file
        if (PAL_FileRemove(cache->pal, filename) != PAL_Ok)
        {
            result = NE_FSYS;
        }
    }
    else
    {
        char* filename = GetUnobfuscatedFilename(cache, name);
        if (filename)
        {
            if (PAL_FileRemove(cache->pal, filename) != PAL_Ok)
            {
                result = NE_FSYS;
            }
            nsl_free(filename);
        }
        else
        {
            result = NE_NOMEM;
        }
    }

    return result;
}

static void
ClearKey(PersistentEntry* entry)
{
    if (entry && entry->key)
    {
        nsl_free(entry->key);
        entry->key = NULL;
        entry->keyLength = 0;
    }
}

/*! Check if the entry exists and if so replace it with the new data.

    @return 'TRUE' if the entry was an existing entry, 'FALSE' otherwise.
*/
nb_boolean
ReplaceExistingIfNecessary(CSL_Cache* cache,
                           byte* name,           /*!< Name of entry to check */
                           size_t nameLength,    /*!< Length of entry name */
                           byte* data,           /*!< Data to be stored in the cache entry */
                           size_t dataLength,    /*!< Size of data to be stored */
                           nb_boolean protect,   /*!< Set to 'TRUE' if the entry should be protected */
                           nb_boolean makeCopy,  /*!< Set to 'TRUE' if a copy of the passed in data should be made.
                                                      'FALSE' if the passed in pointers should be used. */
                           uint32 itemExpiration, /*!< Period of entry validity */
                           NB_Error* result      /*!< On return error code */
                           )
{
    CacheEntry* entry = NULL;
    *result = NE_OK;

    // Check if the entry exists in the cache
    entry = CSL_HashTableFind(cache->hashTable, name, (uint32)nameLength);
    if (! entry)
    {
        // Entry does not exist
        return FALSE;
    }

    // If the protected flag is set for this item then we fail
    if (entry->protect)
    {
        // Entry exists but we fail
        *result = NE_RES;
        return TRUE;
    }

    // It is possible the the item is only in the persistent cache but not in the memory cache.
    // We have to first load it into memory in that case.
    if (! entry->memoryLink)
    {
        *result = LoadEntryFromPersistent(cache, entry);
        if (*result != NE_OK)
        {
            // Indicate the the item exists, but the function fails.
            return TRUE;
        }
    }

    if (makeCopy)
    {
        // Reallocate a buffer for new data. Buffer could be larger or smaller than previous cache item
        entry->data = nsl_realloc(entry->data, dataLength);
        if (!entry->data)
        {
            *result = NE_NOMEM;
            return TRUE;
        }
        // Copy passed in data to new buffer.
        nsl_memcpy(entry->data, data, dataLength);
    }
    else
    {
        // Free old data and copy new data pointer
        nsl_free(entry->data);
        entry->data = data;
    }

    entry->persistentEntry.dataLength = (uint32)dataLength;
    entry->dirty = TRUE;

    if(itemExpiration)
    {
        entry->expireTime = itemExpiration + PAL_ClockGetGPSTime();
    }
    else
    {
        entry->expireTime = 0;
    }

    // If the protect flag is set, update it. But don't reset an existing flag if parameter 'protect' is set to FALSE.
    if (protect)
    {
        entry->protect = protect;
    }

    // We accessed the item, move it to the top of the list
    nsl_assert(entry->memoryLink);
    MoveToTop(&cache->priorityListMemory, entry->memoryLink);

    /*
        If the data is also in the persistent cache then we need to update the persistent entry, too.
    */
    if (entry->persistentLink)
    {
        *result = SaveEntryToPersistent(cache, entry);
    }

    return TRUE;
}

/*! Move the given linked list entry to the top of the linked list.

    @return None
*/
void
MoveToTop(LinkedList* list,     /*!< Linked list to modify, can be either the memory or persistent list */
          LinkedEntry* entry    /*!< Entry to move to the top of the list */
          )
{
    // If the entry is already on top, do nothing
    if (list->first == entry)
    {
        return;
    }
    nsl_assert(list->count > 1);

    // Is it the last element
    if (list->last == entry)
    {
        nsl_assert(entry->previous);

        // Move last pointer to previous element
        list->last = list->last->previous;
        list->last->next = NULL;
    }
    // Element is not at the edge of the list
    else
    {
        // Remove element (close gap)
        entry->previous->next = entry->next;
        entry->next->previous = entry->previous;
    }

    // Move entry to top of list
    entry->previous = NULL;
    entry->next = list->first;
    list->first = entry;
    entry->next->previous = entry;
}

/*! Add the given cache entry to the top of the linked list.

    This creates a new linked list entry.

    @return 'TRUE' on success, 'FALSE' on failure.
*/
nb_boolean
AddToTop(LinkedList* list,              /*!< Linked list to modify, can be either the memory or persistent list */
         CacheEntry* entry,             /*!< Entry to add, has to be valid */
         nb_boolean isMemoryList        /*!< 'TRUE' if it is the memory linked list, 'FALSE' if it is the persistent linked list */
         )
{
    // Create new linked entry
    LinkedEntry* listEntry = nsl_malloc(sizeof(LinkedEntry));
    if (! listEntry)
    {
        return FALSE;
    }
    nsl_memset(listEntry, 0, sizeof(LinkedEntry));
    listEntry->entry = entry;

    // Is it the first element
    if (list->count == 0)
    {
        list->last  = listEntry;
    }
    else
    {
        // Add to top of list
        listEntry->next = list->first;
        list->first->previous = listEntry;
    }
    list->first = listEntry;

    // Cross-link the element to the linked entry
    if (isMemoryList)
    {
        entry->memoryLink = listEntry;
    }
    else
    {
        entry->persistentLink = listEntry;
    }

    list->count++;
    return TRUE;
}

/*! Remove entry from a linked list.

    @return None
*/
void
RemoveFromList(LinkedList* list,
               LinkedEntry* entry   /*!< Entry to remove. Has to be valid/exist. */
               )
{
    if (! entry)
    {
        return;
    }

    nsl_assert(list->count > 0);

    // Is it the last element
    if (list->count == 1)
    {
        nsl_assert(list->first == list->last);

        list->first = NULL;
        list->last = NULL;
        list->count = 0;
        nsl_free(entry);
        return;
    }

    // Is it the first element
    if (list->first == entry)
    {
        nsl_assert(entry->next);

        // Move first pointer to next element
        list->first = entry->next;
        list->first->previous = NULL;
    }
    // Is it the last element
    else if (list->last == entry)
    {
        nsl_assert(entry->previous);

        // Move last pointer to previous element
        list->last = list->last->previous;
        list->last->next = NULL;
    }
    // Element is not at the edge of the list
    else
    {
        // Remove element (close gap)
        entry->previous->next = entry->next;
        entry->next->previous = entry->previous;
    }

    nsl_free(entry);
    list->count--;
}

/*! Delete all elements from the given linked list.

    @return None
*/
void
ClearList(LinkedList* list)
{
    LinkedEntry* current = list->first;
    LinkedEntry* temp = NULL;

    while (current)
    {
        temp = current;
        current = current->next;
        nsl_free(temp);
    }

    list->first = NULL;
    list->last = NULL;
    list->count = 0;
}

/*! Simple helper function to read data. */
void
ReadData(void* destination, byte** source, uint32 size)
{
    nsl_memcpy(destination, *source, size);
    *source += size;
}

/* Disabled to eliminate compiler's warning. */
#if 0
/*! TEST ONLY - Validate consistency of cache.

 Validate that the filenumber vector is consistent with the cache content.
 Also validate the priority linked lists and the hash table

 !Important:
 This function is only used for testing and should NOT be used during production code!
 (It is way too slow to be run during production)
 */
void
TEST_ValidateCacheConsistency(CSL_Cache* cache)
{
    printf("\r\n");

    TEST_ValidateFilenumbersConsistency(cache);
    TEST_ValidateLinkedListConsistency(cache);
    TEST_ValidateHashTableConsistency(cache);
}

/*! TEST ONLY - Validate consistency filenumbers vector.

    !Important:
    This function is only used for testing and should NOT be used during production code!
    (It is way too slow to be run during production)
*/
void
TEST_ValidateFilenumbersConsistency(CSL_Cache* cache)
{
    /*
        Count all the memory and persistent flags and compare it with our memory/persistent entry count!
        It should ALWAYS match, otherwise something's wrong with our logic!
    */

    uint32 memoryCount = 0;
    uint32 persistentCount = 0;
    uint32 nextFilenumber = FILENUM_NONE;

    int count = CSL_VectorGetLength(cache->filenumbersInUse);
    int i = 0;

    // Go through entire filenumber vector
    for (i = 0; i < count; ++i)
    {
        FilenumberUsage* pUsage = CSL_VectorGetPointer(cache->filenumbersInUse, i);
        if (pUsage->memory)
        {
            ++memoryCount;
        }
        if (pUsage->persistent)
        {
            ++persistentCount;
        }
        // Double check the next available filenumber. Ignore index filenumber.
        if (i != FILENUM_INDEX)
        {
            if ((nextFilenumber == FILENUM_NONE) && (pUsage->memory == FALSE) && (pUsage->persistent == FALSE))
            {
                nextFilenumber = i;
            }
        }
    }

    assert(memoryCount == cache->priorityListMemory.count);
    assert(persistentCount == cache->priorityListPersistent.count);
    assert(nextFilenumber == cache->nextAvailableFilenumber);

    printf("Validate Filenumbers. Memory: %d, Persistent: %d, Next: %d\r\n", memoryCount, persistentCount, nextFilenumber);
}

/*! TEST ONLY - Validate consistency of linked lists.

    !Important:
    This function is only used for testing and should NOT be used during production code!
    (It is way too slow to be run during production)
*/
void
TEST_ValidateLinkedListConsistency(CSL_Cache* cache)
{
    LinkedEntry* current = cache->priorityListMemory.first;
    uint32 memoryCount = 0;
    uint32 persistentCount = 0;

    // Check memory linked list
    for (; current; current = current->next)
    {
        // Validate that the cross-link is correct
        nsl_assert(current->entry->memoryLink == current);
        memoryCount++;
    }

    // Check persistent linked list
    current = cache->priorityListPersistent.first;
    for (; current; current = current->next)
    {
        // Validate that the cross-link is correct
        nsl_assert(current->entry->persistentLink == current);
        persistentCount++;
    }

    // Double check that the counts match
    assert(memoryCount == cache->priorityListMemory.count);
    assert(persistentCount == cache->priorityListPersistent.count);

    /*
        Walk lists backwards to check previous pointers
    */

    memoryCount = 0;
    persistentCount = 0;

    // Check memory linked list
    current = cache->priorityListMemory.last;
    for (; current; current = current->previous)
    {
        // Validate that the cross-link is correct
        nsl_assert(current->entry->memoryLink == current);
        memoryCount++;
    }

    // Check persistent linked list
    current = cache->priorityListPersistent.last;
    for (; current; current = current->previous)
    {
        // Validate that the cross-link is correct
        nsl_assert(current->entry->persistentLink == current);
        persistentCount++;
    }

    // Double check that the counts match
    assert(memoryCount == cache->priorityListMemory.count);
    assert(persistentCount == cache->priorityListPersistent.count);

    printf("Validate Linked lists. Memory: %d, Persistent: %d\r\n", memoryCount, persistentCount);
}

/*! TEST ONLY - Validate consistency of hash table.

    !Important:
    This function is only used for testing and should NOT be used during production code!
    (It is way too slow to be run during production)
*/
void
TEST_ValidateHashTableConsistency(CSL_Cache* cache)
{
    // Go through hash table and add all memory and persistent entries
    TEST_Counters counters = {0};
    CSL_HashTableForEach(cache->hashTable, &TEST_ForEachCallback, &counters);

    // Double check that the counts match
    assert(counters.memoryCount == cache->priorityListMemory.count);
    assert(counters.persistentCount == cache->priorityListPersistent.count);

    printf("Validate Hash Table. Memory: %d, Persistent: %d\r\n", counters.memoryCount, counters.persistentCount);

    {
        uint32 totalTableSize = 0;
        uint32 entryCount = 0;
        uint32 collisionCount = 0;
        double averageLookupCount = 0.0;
        double collisionPercentage = 0.0;

        CSL_HashTableGetStats(cache->hashTable,
                              &totalTableSize,
                              &entryCount,
                              &collisionCount,
                              &averageLookupCount);

        if (entryCount != 0)
        {
            collisionPercentage = (double)collisionCount / (double)entryCount * 100.0;
        }

        // Check collision percentage and average lookup count. They should not be too high
        // Only check if we have a couple of entries in the table
        if (entryCount > 20)
        {
            nsl_assert(collisionPercentage <= 40.0);
            nsl_assert(averageLookupCount <= 2.5);
        }

        printf("Hash Stats: Total Size: %d, Entry Count: %d, Collision Count: %d, Collision Percentage: %f, Average Lookup: %f\r\n",
               totalTableSize,
               entryCount,
               collisionCount,
               collisionPercentage,
               averageLookupCount);
    }
}

/*! Callback function used for TEST_ValidateHashTableConsistency().

    @see TEST_ValidateHashTableConsistency
*/
void
TEST_ForEachCallback(void* value, void* userData)
{
    CacheEntry* entry = (CacheEntry*)value;
    TEST_Counters* counters = (TEST_Counters*)userData;

    if (entry->memoryLink)
    {
        counters->memoryCount++;
    }
    if (entry->persistentLink)
    {
        counters->persistentCount++;
    }
}

#endif
