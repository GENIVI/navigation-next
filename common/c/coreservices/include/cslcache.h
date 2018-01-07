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

@file     cslcache.h
@date     01/06/2005
@defgroup COREUTIL_API_GROUP         NIM Core Cache Util

This API provides an interface to the NIM cache util
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

/*! @page NET_OVERVIEW_PAGE     Cache API Overview
This API provides the interface to the cache interface.

<p>
\ref COREUTIL_API_GROUP
</p>
*/

/*! @file nimnetwork.h 
\defgroup   COREUTIL_API_GROUP      NIM Core Cache Util
*/
/*@{*/

#ifndef CSLCACHE_H
#define	CSLCACHE_H

#include "paltypes.h"
#include "pal.h"
#include "palstdlib.h"
#include "cslexp.h"
#include "nberror.h"

typedef struct CSL_Cache CSL_Cache;

/*! Clear option used to clear cache.

    @see CSL_CacheClearExtended
*/
typedef enum
{
    CSL_CF_All,                 /*!< Clear both the persistent and memory cache */
    CSL_CF_MemoryOnly,          /*!< Clear only the memory cache including any protected/locked entries. */
    CSL_CF_UnprotectedOnly      /*!< Clear only the memory cache; protected entries are kept and not deleted */

} CSL_ClearOption;

typedef void (*CSL_CacheUserFunction)(uint32 timestamp /* not used anymore */, byte* name, size_t nameLength, byte* data, size_t dataLength, void* userData);

/*! User provided callback functions to handle persistent cache content. 

    If no user functions are provided then the default function are used which use a file based persistent cache.
*/
typedef NB_Error (*CSL_CacheSaveIndexFunction)(CSL_Cache* cache, 
                                               const byte* data,    /*!< Buffer of index file to save */
                                               uint32 length        /*!< Length of index file buffer */
                                               );

typedef NB_Error (*CSL_CacheSaveFunction)(CSL_Cache* cache, 
                                          uint32 fileNumber,        /*!< File number. Only used internally */
                                          const byte* name,         /*!< Unique name for cache entry to save */
                                          size_t namelen,           /*!< Length of cache entry name */
                                          const byte* data,         /*!< Data buffer of cache entry to save */
                                          uint32 length             /*!< Length of data buffer of cache entry */
                                          );

typedef NB_Error (*CSL_CacheLoadIndexFunction)(CSL_Cache* cache,
                                               byte** data,         /*!< On return the buffer containing the index file data. Call nsl_free() to free the buffer. */
                                               uint32* length       /*!< On return the length of the index file data buffer */
                                               );

typedef NB_Error (*CSL_CacheLoadFunction)(CSL_Cache* cache, 
                                          uint32 fileNumber,        /*!< File number. Only used internally */
                                          const byte* name,         /*!< Unique name of cache entry to load */
                                          size_t namelen,           /*! Length of cache entry name. */
                                          byte** data,              /*!< On return the buffer for the cache entry. Call nsl_free() to free the buffer. */
                                          uint32* length            /*!< On return the length of the data buffer */
                                          );

typedef NB_Error (*CSL_CacheRemoveFunction)(CSL_Cache* cache, 
                                            uint32 fileNumber,        /*!< File number. Only used internally */
                                            const byte* name,         /*!< Unique name of cache entry to remove/delete */
                                            size_t namelen            /*!< Length of cache entry name. */
                                            );

typedef void (*CSL_CacheDumpFunction)(CSL_Cache* cache, const char* caption);

/*
    Notification functions. These functions are different then the callback functions above. They only serve as a notification and are only called if they are
    set using CSL_CacheSetNotificationFunctions.

    @see CSL_CacheSetNotificationFunctions
*/
/*! Notification function for entries removed from the persistent cache.

    Only entries removed from the persistent cache are notified. Not entries removed from the memory cache.
*/
typedef void (*CSL_CacheRemoveNotificationFunction)(CSL_Cache* cache, 
                                                    const byte* name,       /*!< Unique name of cache entry to remove */
                                                    size_t namelen,         /*!< Length of cache entry name. */
                                                    void* userData          /*!< User data passed in CSL_CacheSetNotificationFunctions */
                                                    );

/*! Cache configuration.

    The user can pass in user-defined callback function to modify the persistent cache behavior.
    If the user doesn't provide any functions then the default behavior will use a file-based
    persistent cache.
*/
typedef struct CSL_CacheConfig 
{
    CSL_CacheSaveIndexFunction      saveIndexFunction;      /*!< Save index function */
    CSL_CacheSaveFunction           saveFunction;           /*!< Save function to save one entry */
    CSL_CacheLoadIndexFunction      loadIndexFunction;      /*!< Load index function */
    CSL_CacheLoadFunction           loadFunction;           /*!< Load function to load one entry */
    CSL_CacheRemoveFunction         removeFunction;         /*!< Function to remove one entry from the persistent cache */
    CSL_CacheDumpFunction           dumpFunction;           /*!< Dump function */

} CSL_CacheConfig;

/*! Construct a CSL_Cache instance.

    The cache now supports un-obfuscated file-backed functionality. This is needed for the Enhanced Content Download Manager.
    In this mode, the key/name of the cache entry will be used as the file name. As always, the cache name is used to
    determine the cache directory (can contain subdirectories).

@param pal                      Pointer to the nbipal instance
@param name                     Name of cache, this is currently used as the directory name of the cache. Can contain subdirectories.
@param maximumItemsMemory       Maximum number of entries in memory cache
@param maximumItemsPersistent   Maximum number of entries in persistent cache. 
@param obfuscateKey             Obfuscate key for file backed cache. The pointer (and content) is assumed to be a static 
                                value which has to be persistent during the existent of the cache.
@param obfuscateKeyLen          Obfuscate key length
@param config                   Optional cache configuration. Pass in NULL if not used. If set to NULL file cache will be used.
@param obfuscateEntries         Set to 'TRUE' to obfuscate cache entries and cache names. If set to 'FALSE' then the names of the
                                cache entries are used as the file names for the persistent cache and the file content is not
                                obfuscated. The index-file will always be obfuscated.

@return An instance of CSL_Cache is returned or NULL if something fails.
*/
CSL_DEC CSL_Cache*
CSL_CacheAlloc(PAL_Instance* pal, 
               const char* name, 
               int maximumItemsMemory, 
               int maximumItemsPersistent, 
               const byte* obfuscateKey, 
               size_t obfuscateKeyLen, 
               CSL_CacheConfig* config,
               nb_boolean obfuscateEntries);

/*! Destroy a CSL_Cache instance.

@param cache Pointer to CSL_Cache instance
@return void
*/
CSL_DEC void
CSL_CacheDealloc(CSL_Cache* cache);

/*! Get the maximum size of the cache.

@param cache                    Pointer to CSL_Cache instance
@param maximumItemsMemory       New maximum number of entries in memory cache
@param maximumItemsPersistent   New maximum number of entries in persistent cache.

@return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheSetSize(CSL_Cache* cache, int maximumItemsMemory, int maximumItemsPersistent);

/*! Gets Max Items, the number of dirty items, vector length, number of protected items, and number of deleted items in an existing CSL_Cache.

@param cache                    Pointer to CSL_Cache instance
@param maximumItemsMemory       Returns maximum number of entries in memory cache. Optional, set to NULL if not needed.
@param maximumItemsPersistent   Returns maximum number of entries in persistent cache. Optional, set to NULL if not needed.
@param dirtyItems               Returns dirty items. Optional, set to NULL if not needed.
@param cachedMemoryItems        Returns number of entries in the memory cache. Optional, set to NULL if not needed.
@param cachedPersistentItems    Returns number of entries in the persistent cache. Optional, set to NULL if not needed.
@param protectedItems           Returns protected items. Optional, set to NULL if not needed.

@return void
*/
CSL_DEC void
CSL_CacheGetStats(CSL_Cache* cache, 
                  int* maximumItemsMemory, 
                  int* maximumItemsPersistent, 
                  int* dirtyItems, 
                  int* cachedMemoryItems, 
                  int* cachedPersistentItems, 
                  int* protectedItems);

/*! Adds data to an existing CSL_Cache. 

    If the entry already exists then the cache entry gets replaced with the new data. If the cache is full the last 
    (non protected) entry will be discarded. If all entries are protected then the function will fail.

    If the parameter 'makeCopy' is set then a copy of the passed in data is made. If 'makeCopy' is not set, then
    the CSL_Cache takes ownership of the passed in pointer and releases it once it is no longer in the cache. Passed
    in pointers with 'makeCopy' not set have to be dynamically allocated using nsl_malloc().

    // @todo: Add functionality of returning error when cache is full.

    @param cache Pointer to CSL_Cache instance
    @param name Name of cache entry
    @param namelen Length of name of cache entry.
    @param data Data to be stored in the cache entry
    @param datalen Size of data to be stored
    @param protect Specifies whether data is protected or not
    @param makeCopy See description above
    @param itemExpiration   Optional expiration of the cache item in seconds. The expiration time will be checked each time
                            the item gets accessed and automatically discarded if the time expired. Only unproteced items
                            get discarded. If set to zero then the item never expires.

    @return NE_OK on success, NE_RES if all entries are protected.
*/
CSL_DEC NB_Error
CSL_CacheAdd(CSL_Cache* cache, byte* name, size_t namelen, byte* data, size_t datalen, nb_boolean protect, nb_boolean makeCopy, uint32 itemExpiration);

/*! Finds data in in an existing CSL_Cache.

    This function returns either a copy of the cache data or a pointer to the cache data itself. See parameter
    'makeCopy' for more details. The user has to call nsl_free() for the returned data if 'makeCopy' was set.

    This function can automatically protect the returned cache item. This will ensure that retuned data doesn't
    get deleted. This especially useful if 'makeCopy' is set to 'FALSE'.

    Calling this function will set the internal 'accessed' flag. Use CSL_CacheSaveAccessed() to save
    all accessed entries to the persistent/file cache.

    If the entry is not in the memory cache but in the persistent/file cache then the function will load the entry
    into the memory cache.

    If an expired item (see CSL_CacheAdd() for more details) is requested then the item will be discarded and NE_NOENT
    will be returned.

    @param cache                Pointer to CSL_Cache instance
    @param name                 Name of cache entry
    @param namelen              Length of name of cache entry.
    @param pdata                On return a pointer to the copy of the cache entry. Call nsl_free() to free data when no longer needed.
    @param datalen              On return size of the returned cache entry.
    @param protect              If set then this call will automatically set the protected flag for the returned item

    @param makeCopy             If set then this call will make a copy of cache entry. Otherwise it will return a pointer to the 
                                internal data. Setting this flag to 'FALSE' should only be used if the protected flag is set. Otherwise
                                the user has no garantee on how long the data stays valid.

    @param checkMemoryCacheOnly Setting this flag will result in lookup of the memory cache only and not in the persistent cache. If
                                the entry was not found in the memory cache, this function will fail.

    @return NE_OK if the entry was found, NE_NOENT if the entry was not found.

    @see CSL_CacheSaveAccessed
    @see CSL_CacheClearProtectOne
    @see CSL_CacheClearProtect
*/
CSL_DEC NB_Error
CSL_CacheFind(CSL_Cache* cache, byte* name, size_t namelen, byte** pdata, 
              size_t* pdatalen, nb_boolean protect, nb_boolean makeCopy, nb_boolean checkMemoryCacheOnly);

/*! Empties an existing CSL_Cache.

    This removes all memory cache entries and removes all entries from the persistent cache.
    This function does NOT keep any protected entries. Use CSL_CacheClearExtended() if you need to keep
    protected entries.

    @param cache Pointer to CSL_Cache instance

    @see CSL_CacheClearExtended
    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheClear(CSL_Cache* cache);

/*! Empties an existing CSL_Cache with options.

    @param cache                Pointer to CSL_Cache instance
    @param clearOption          See CSL_ClearOption for description of options.

    @see CSL_CacheClear
    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheClearExtended(CSL_Cache* cache, CSL_ClearOption clearOption);

/*! Removes one entry from the cache.

    @param cache Pointer to CSL_Cache instance
    @param name Name of cache entry to remove
    @param namelen Length of name of cache entry.

    @return NE_OK on success, NE_NOENT if the entry doesn't exist
*/
CSL_DEC NB_Error
CSL_CacheClearOne(CSL_Cache* cache, byte* name, size_t namelen);

/*! Iterates through an existing CSL_Cache and calls the supplied CSL_CacheFunction for each cache entry

    ! This does only iterate through the memory cache entries. (otherwise we would have to load all
    persistent entries which then in itself would invalidate some of the memory cache entries.)

    @param cache        Pointer to CSL_Cache instance
    @param callback     Function to be called on each cache entry
    @param userData     Callback data for user-supplied function
    @param getData      NOT USED

    @return void
*/
CSL_DEC void
CSL_CacheForEach(CSL_Cache* cache, CSL_CacheUserFunction callback, void* userData, nb_boolean getData);

/*! Protects an entry in an existing CSL_Cache.

    @param cache Pointer to CSL_Cache instance
    @param name Name of cache entry
    @param namelen Length of name of cache entry.

    @return NE_OK on success, NE_NOENT if the entry doesn't exist
*/
CSL_DEC NB_Error
CSL_CacheProtect(CSL_Cache* cache, byte* name, size_t namelen);

/*! Save the index file to disk or uses user supplied function.

    !Important:
    If the persistent cache is used then this function has to be called before destroying the cache object.
    Otherwise the persistent cache items will not be accessable after the next restart.

    Calls a user specified save index function.  If no save function was supplied, then this function
    will save the index to a file.

    @param cache Pointer to CSL_Cache instance
    @return NB_Error
    @see CSL_CacheLoadIndex
*/
CSL_DEC NB_Error
CSL_CacheSaveIndex(CSL_Cache* cache);

/*! Save all accessed entries to persistent cache.

    Calls a users supplied function to save accessed cache entries.  If no function has bee supplied by the user,
    then this function, by default, will save the accessed cache entries to files.

    Accessed entries are entries for which CSL_CacheFind() was called.

    @param cache Pointer to CSL_Cache instance
    @param maximumEntries   Maximum entries to save to the persistent cache. Set to '-1' to save all entries.
    @param savedEntries     Number of entries saved by this call. Can be less than the maximum requested entries. Set to NULL if not needed.
    @param removeFromMemory If set then the saved entries will be removed from the memory cache. Protected items will not be removed from memory.

    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheSaveAccessed(CSL_Cache* cache, int32 maximumEntries, int32* savedEntries, nb_boolean removeFromMemory);

/*! Save dirty entries to the persistent cache.

    This function will only save up to 'maximumEntries' dirty entries to the persistent cache. This
    allows to not overwhelm the system by saving too many entries at once.

    Calls a users supplied function to save dirty cache entries.  If no function has been supplied by the user,
    then this function, by default, will save the dirty cache entries to files.

    Dirty entries are entries which have been added to the cache.

    @param cache            Pointer to CSL_Cache instance
    @param maximumEntries   Maximum entries to save to the persistent cache. Set to '-1' to save all entries.
    @param savedEntries     On return the number of entries saved by this call. Can be less than the maximum requested entries. Set to NULL if not needed.
    @param removeFromMemory If set then the saved entries will be removed from the memory cache. Protected items will not be removed from memory.

    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheSaveDirty(CSL_Cache* cache, int32 maximumEntries, int32* savedEntries, nb_boolean removeFromMemory);

/*! Load the index-file from disk.

    !Important:
    If the persistent cache is used then this function has to be called after creating the cache object.
    Otherwise the last saved persistent cache items will not be accessable.

    This does not load any cached entries to the memory cache. The entries get loaded once they get accessed.
    If a user defined load function was provided, then it gets called.  By default this will use a file-backed function.

    @param cache Pointer to CSL_Cache instance
    @return NB_Error
    @see CSL_CacheSaveIndex
*/
CSL_DEC NB_Error
CSL_CacheLoadIndex(CSL_Cache* cache);

/*! Dumps the content of the CSL_Cache.

    @param cache Pointer to CSL_Cache instance
    @param caption Caption to display when dumping cache
    @return NB_Error
*/
CSL_DEC void
CSL_CacheDump(CSL_Cache* cache, const char* caption);

/*! Clears cache accessed flags from all cache entries.

    @param cache Pointer to CSL_Cache instance
    @return void
*/
CSL_DEC void
CSL_CacheClearAccessed(CSL_Cache* cache);

/*! Clears protect flags from all cache entries.

    The protected flag can be set by CSL_CacheProtect(), CSL_CacheFind() or CSL_CacheAdd().

    @param cache Pointer to CSL_Cache instance
    @return void

    @see CSL_CacheProtect
    @see CSL_CacheFind
    @see CSL_CacheAdd
*/
CSL_DEC void
CSL_CacheClearProtect(CSL_Cache* cache);

/*! Clears protect flags from one cache entries.

    This function has to be called to clear the protected flag if the protected flag was set by either calling 
    CSL_CacheProtect(), CSL_CacheFind() or CSL_CacheAdd().

    @param cache Pointer to CSL_Cache instance
    @param name Name of entry for which to clear the protect flag
    @param namelen Length of name of entry.
    @return NE_OK on success, NE_NOENT if the entry doesn't exist

    @see CSL_CacheClearProtect
    @see CSL_CacheProtect
    @see CSL_CacheFind
    @see CSL_CacheAdd
*/
CSL_DEC NB_Error
CSL_CacheClearProtectOne(CSL_Cache* cache, byte* name, size_t namelen);

/*! Sets cache callback functions.

    These functions differ to the notification functions.

    @param cache Pointer to CSL_Cache instance
    @return NB_Error

    @see CSL_CacheSetNotificationFunctions
*/
CSL_DEC NB_Error
CSL_CacheSetFunctions(CSL_Cache* cache,	    
                      CSL_CacheSaveIndexFunction saveIndexFunction,
                      CSL_CacheSaveFunction saveFunction,
                      CSL_CacheLoadIndexFunction loadIndexFunction,
                      CSL_CacheLoadFunction loadFunction,
                      CSL_CacheRemoveFunction removeFunction,
                      CSL_CacheDumpFunction dumpFunction);

/*! Set notification functions.

    The notification functions are only called if this function is called.

    @param removeNotificationFunction Notification function to call. Set to NULL if you want to disable notifications.
    @param userData user data for callbacks.
    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheSetNotificationFunctions(CSL_Cache* cache,	    
                                  CSL_CacheRemoveNotificationFunction removeNotificationFunction,
                                  void* userData);

/*! Finds an item in the cache by name, returns TRUE if found, FALSE if not found

    @param cache Pointer to CSL_Cache instance
    @param name name of cache item
    @param namelen name length of cache item.
    @return nb_boolean
*/
CSL_DEC nb_boolean
CSL_CacheIsItemInCache(CSL_Cache* cache, byte* name, size_t namelen);

/*! Set optional user data which will be saved in the index/header of the cache.

    The user data can be set at any time and will be saved to the index-file the next time CSL_CacheSaveIndex()
    is called. The cache creates a copy of the data.

    @param cache    Pointer to CSL_Cache instance
    @param data     User data to save. Set to NULL to clear user data.
    @param size     Size of user data to save

    @see CSL_CacheGetUserData
    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheSetUserData(CSL_Cache* cache, byte* data, size_t size);

/*! Get user data.

    The user data is the data which got loaded from the index-file (or the currently set user data).
    The returned data is just a pointer to the internal data. The caller of this function has to make a copy of the data
	if it wants to keep the data.

    @param cache    Pointer to CSL_Cache instance
    @param data     On return the user data. Can be NULL if no user data is set.
    @param size     On return the size of the user data.

    @see CSL_CacheSetUserData
    @return NB_Error
*/
CSL_DEC NB_Error
CSL_CacheGetUserData(CSL_Cache* cache, byte** data, size_t* size);

/*! Validate that there are no orphaned cache files on disk.

    We check that we don't have any orphaned cache entries in the file system.
    This could happen if the cache was not shut down correctly and/or CSL_CacheSaveIndex() was 
    not called before destroying the cache.

    This function is very processor intense (it can take multiple seconds to complete) and should only be called 
    very rarely. The function should NOT be called during startup of the application.

    CSL_CacheLoadIndex() has to be called before calling this function, otherwise all persistent entries are deleted.

    @return NB_Error

    @see CSL_CacheLoadIndex
*/
CSL_DEC NB_Error
CSL_CacheValidateFilesOnDisk(CSL_Cache* cache);


#endif
/*! @} */


