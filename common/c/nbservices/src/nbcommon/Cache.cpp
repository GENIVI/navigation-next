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

    @file       Cache.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

extern "C"
{
#include "palstdlib.h"
#include "palfile.h"
#include "nbcontextprotected.h"
}

#include "Cache.h"
#include "CachingEntryDatabaseProcessor.h"
#include "LeastRecentlyUsedEvictionStrategy.h"
#include <limits>
#include "CacheTasks.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "Lock.h"

/*! @{ */

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................

#define MAX_PATH_LENGTH 512

// 1 K bytes equals 1024 bytes.
//static const uint32 K_BYTES = 1024;

// Expiring time for closing file handles
//static const uint32 FILE_HANDLE_EXPIRING_TIME = 30000;  // milliseconds

// Maximum count of opening file handles
//static const uint32 MAX_OPENING_FILE_HANDLE_COUNT = 64;

// Caching index filename for saving caching entries.
static const char CACHING_INDEX_FILENAME[] = "___I_N_D_E_X___";

// Public functions ........................................................................

/*! Check if a string pointer is valid.

    Treat string as valid if:  Pointer is not NULL && string is not empty.

    @return true if it is valid.
*/

inline bool IsStringValid(const shared_ptr<string>& str)
{
    return (str && !str->empty());
}

/*! Build a identity given type and name.
    @return string
*/
static inline string BuildCacheIdentity(const shared_ptr <string>& type,
                                        const shared_ptr <string>& name)
{
    string identity;
    if (IsStringValid(type))
    {
        identity.assign(*type);
        if (IsStringValid(name))
        {
            identity.append("_SEP_");
            identity.append(*name);
        }
    }
    return identity;
}

/* See header file for description */
Cache::Cache() : m_initializeResult(NE_NOTINIT),
                 m_context(NULL),
                 m_maximumDataCount(0),
                 m_name(),
                 m_path(),
                 m_entryProcessor(),
                 m_evictionStrategy(),
                 m_validFiles(),
                 m_willexpiredFiles()
{
}

/* See header file for description */
Cache::Cache(NB_Context* context,
             uint32 maximumDataCount,
             shared_ptr<string> name,
             shared_ptr<string> path)
        : m_context(context),
          m_maximumDataCount(maximumDataCount),
          m_name(name),
          m_path(path)
{
    m_initializeResult = Initialize(context, maximumDataCount, name, path);
}

/* See header file for description */
Cache::~Cache()
{
    Reset();
    if (m_lock)
    {
        PAL_LockDestroy(m_lock);
    }
}

/* See header file for description */
//@todo: Move file operations to FileOperationThread!
NB_Error
Cache::Initialize(NB_Context* context,
                  uint32 maximumDataCount,
                  shared_ptr<string> name,
                  shared_ptr<string> path)
{
    PAL_Error palError = PAL_Ok;
    NB_Error nbError = NE_OK;
    PAL_Instance* pal = NULL;
    char pathChar[MAX_PATH_LENGTH] = {0};

    if ((!context) ||
        (maximumDataCount == 0) ||
        (!name) || (name->empty()) ||
        (!path) || (path->empty()))
    {
        return NE_INVAL;
    }

    pal = NB_ContextGetPal(context);
    if (!pal)
    {
        return NE_INVAL;
    }

    // Append the name to the path and save this path as the caching path.
    nsl_memset(pathChar, 0, MAX_PATH_LENGTH);
    nsl_strlcpy(pathChar, path->c_str(), MAX_PATH_LENGTH);

    palError = palError ? palError : PAL_FileAppendPath(pal, pathChar, MAX_PATH_LENGTH, name->c_str());
    palError = palError ? palError : PAL_FileCreateDirectoryEx(pal, pathChar);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    // Create the caching path.
    shared_ptr<string> cachingPath(new string(pathChar));
    if (!cachingPath)
    {
        return NE_NOMEM;
    }

    // Create the database full path for saving the caching entries.
    palError = PAL_FileAppendPath(pal, pathChar, MAX_PATH_LENGTH, CACHING_INDEX_FILENAME);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }
    shared_ptr<string> databaseFullPath(new string(pathChar));
    if (!databaseFullPath)
    {
        return NE_NOMEM;
    }

    // Create a processor for the caching entries.
    CachingEntryDatabaseProcessor* databaseEntryProcessor = new CachingEntryDatabaseProcessor();
    if (!databaseEntryProcessor)
    {
        return NE_NOMEM;
    }

    m_fileOperationManager = FileOperatingTaskManager::GetInstance(context);
    if (!m_fileOperationManager)
    {
        return NE_UNEXPECTED;
    }

    PAL_Instance* palInstance = NB_ContextGetPal(context);
    if (!palInstance || (PAL_LockCreate(palInstance, &m_lock) != PAL_Ok))
    {
        return NE_NOTINIT;
    }

    // Take ownership of the processor object.
    CachingEntryProcessorPtr entryProcessor(databaseEntryProcessor);

    // Initialize the processor object.
    nbError = databaseEntryProcessor->Initialize(pal, databaseFullPath);
    if (nbError != NE_OK)
    {
        /* This error may be caused by the change of database structure. The caching path
           should be cleared and initialize the entry processor again.
        */
        RemovePath(cachingPath->c_str());
        palError = PAL_FileCreateDirectoryEx(pal, cachingPath->c_str());
        if (palError != PAL_Ok)
        {
            return NE_FSYS;
        }

        // Initialize the processor object again.
        nbError = databaseEntryProcessor->Initialize(pal, databaseFullPath);
        if (nbError != NE_OK)
        {
            return nbError;
        }
    }
    databaseEntryProcessor = NULL;

    // Create an eviction strategy.
    shared_ptr<EvictionStrategy<CachingIndex> > evictionStrategy(new LeastRecentlyUsedEvictionStrategy<CachingIndex>());
    if (!evictionStrategy)
    {
        return NE_NOMEM;
    }

    Reset();

    // Initialize the members of this object.
    m_context = context;
    m_maximumDataCount = maximumDataCount;
    m_name = name;
    m_path = cachingPath;
    m_entryProcessor = entryProcessor;
    m_evictionStrategy = evictionStrategy;

    // Initialize the eviction strategy.
    InitializeEvictionStrategy();

    m_isValid.reset(new bool(true));

    // Check if the cache is full.
    shared_ptr<string> emptyPointer;
    nbError = ProcessCacheFull(emptyPointer, emptyPointer, 0);
    if (nbError != NE_OK)
    {
        // Remove all items of this cache.
        RemoveDataInternal(emptyPointer, emptyPointer);

        // Check if the cache is full again.
        nbError = ProcessCacheFull(emptyPointer, emptyPointer, 0);
        if (nbError != NE_OK)
        {
            Reset();
            return nbError;
        }
    }

    m_initializeResult = NE_OK;
    return NE_OK;
}

/* See header file for description */
bool
Cache::IsInitialized() const
{
    if (m_initializeResult != NE_OK)
    {
        return false;
    }

    if (m_context && NB_ContextGetPal(m_context) &&
        (m_maximumDataCount > 0) &&
        m_name && (!(m_name->empty())) &&
        m_path && (!(m_path->empty())) &&
        m_entryProcessor &&
        m_evictionStrategy)
    {
        return true;
    }

    return false;
}

/* See header file for description */
void
Cache::RemoveDataInternal(shared_ptr<string> type,
                          shared_ptr<string> name)
{
    // Check if this object is initialized.
    if ((!IsInitialized() || !m_context) || (!m_path) || (m_path->empty()))
    {
        return;
    }

    PAL_Instance* pal = NB_ContextGetPal(m_context);
    if (!pal)
    {
        return;
    }

    if (m_entryProcessor)
    {
        m_entryProcessor->RemoveEntry(type, name);
    }

    shared_ptr<string> path;

    if(!type)           // remove all items of this cache.
    {
        path = m_path;
    }
    else
    {
        path = BuildCachingPath(type, name);
    }

    if (path)
    {
        (void)FileOperatingTaskManager::DeleteFile(m_fileOperationManager, path);
    }

    if (type && name)
    {
        // Remove one caching entry.
        if (m_evictionStrategy)
        {
            m_evictionStrategy->IndexOperated(CachingIndex(type, name), REMOVING_INDEX);
        }
    }
    else
    {
        // :TRICKY: we don't need to delete cached files here, it will be deleted when
        //          necessary.
        if (m_evictionStrategy)
        {
            m_evictionStrategy->Clear();
            InitializeEvictionStrategy();
        }
    }
}

/* See header file for description */
bool
Cache::IsItemExisting(shared_ptr<string> type,
                      shared_ptr<string> name)
{
    bool result = false;

    if (m_entryProcessor)
    {
        result = m_entryProcessor->IsEntryExisting(type, name);
    }

    // Notify the caching entry is used.
    if (result && m_evictionStrategy)
    {
        m_evictionStrategy->IndexOperated(CachingIndex(type, name), USING_INDEX);
    }

    return result;
}



/* See header file for description */
bool
Cache::IsItemProtected(shared_ptr<string> type,
                       shared_ptr<string> name)
{
    if (m_entryProcessor)
    {
        CachingEntryPtr entry = m_entryProcessor->GetEntry(type, name);
        if (entry)
        {
            return entry->GetProtected();
        }
    }

    return false;
}

/* See header file for description */
AdditionalDataPtr
Cache::GetAdditionalData(shared_ptr<string> type,
                         shared_ptr<string> name)
{
    if (!m_entryProcessor)
    {
        return shared_ptr<map<string, shared_ptr<string> > >();
    }

    // Get the caching entry by the type and name.
    CachingEntryPtr entry = m_entryProcessor->GetEntry(type, name);
    if (!entry)
    {
        return shared_ptr<map<string, shared_ptr<string> > >();
    }

    return entry->GetAdditionalData();
}

/* See header file for description */
void
Cache::SetAdditionalData(shared_ptr<string> type,
                         shared_ptr<string> name,
                         shared_ptr<map<string, shared_ptr<string> > > additionalData)
{
    NB_Error error = NE_OK;

    if (!m_entryProcessor)
    {
        return;
    }

    // Get the caching entry by the type and name.
    CachingEntryPtr entry = m_entryProcessor->GetEntry(type, name);
    if (!entry)
    {
        return;
    }

    entry->SetAdditionalData(additionalData);

    // Update the caching entry.
    error = m_entryProcessor->SetEntry(entry);
    if (error != NE_OK)
    {
        return;
    }
}

/* See header file for description */
shared_ptr<string>
Cache::BuildCachingPath(shared_ptr<string> type,
                        shared_ptr<string> name)
{
    PAL_Error palError = PAL_Ok;
    PAL_Instance* pal = NULL;
    char pathChar[MAX_PATH_LENGTH] = {0};

    if ((!type) || (type->empty()) ||
        (!name) || (name->empty()) ||
        (!m_path) || (m_path->empty()) ||
        (!m_context))
    {
        return shared_ptr<string>();
    }

    pal = NB_ContextGetPal(m_context);
    if (!pal)
    {
        return shared_ptr<string>();
    }

    nsl_memset(pathChar, 0, MAX_PATH_LENGTH);
    nsl_strlcpy(pathChar, m_path->c_str(), MAX_PATH_LENGTH);

    // Append the type as a subpath to the path.
    palError = palError ? palError : PAL_FileAppendPath(pal, pathChar, MAX_PATH_LENGTH, type->c_str());

    // Append the name as a filename to the path.
    palError = palError ? palError : PAL_FileAppendPath(pal, pathChar, MAX_PATH_LENGTH, name->c_str());

    if (palError != PAL_Ok)
    {
        return shared_ptr<string>();
    }

    return shared_ptr<string>(new string(pathChar));
}

/* See header file for description */
NB_Context*
Cache::GetContext()
{
    return m_context;
}


// Private functions ............................................................................

/*! Reset this object to be uninitialized

    Clear all members in this object.

    @return None
*/
void
Cache::Reset()
{
    // Clear other members in this object.
    m_evictionStrategy.reset();
    m_entryProcessor.reset();
    m_path.reset();
    m_name.reset();
    m_maximumDataCount = 0;
    m_context = NULL;
    if (m_isValid)
    {
        *m_isValid = false;
        m_isValid.reset();
    }
}

/*! Remove the path

    If parameter path specifies a file, this file is removed. If it specifies a directory,
    this directory is removed.

    @return None
*/
void
Cache::RemovePath(const char* path)
{
    PAL_Instance* pal = NULL;

    // Check if parameter path is valid.
    if ((!path) || (nsl_strlen(path) == 0) || (!m_context))
    {
        return;
    }

    pal = NB_ContextGetPal(m_context);
    if (!pal)
    {
        return;
    }

    // Check if the path exists.
    if (PAL_FileExists(pal, path) == PAL_Ok)
    {
        // Remove the file or directory.
        if (PAL_FileIsDirectory(pal, path))
        {
            // Ignore returned error.
            PAL_FileRemoveDirectory(pal, path, TRUE);
        }
        else
        {
            // Ignore returned error.
            PAL_FileRemove(pal, path);
        }
    }
}

/*! Initialize the eviction strategy

    This function gets all caching entries from 'm_entryProcessor' and adds them to
    'm_evictionStrategy'.

    @return None
*/
void
Cache::InitializeEvictionStrategy()
{
    if ((!m_entryProcessor) || (!m_evictionStrategy))
    {
        return;
    }

    // Get all caching entries from the entry processor.
    vector<CachingEntryPtr> entries;
    m_entryProcessor->GetAllEntries(entries);

    // Add all unprotected caching entries to the eviction strategy.
    vector<CachingEntryPtr>::const_iterator iterator = entries.begin();
    vector<CachingEntryPtr>::const_iterator end = entries.end();
    for (; iterator != end; ++iterator)
    {
        CachingEntryPtr entry = *iterator;

        if (entry)
        {
            shared_ptr<string> type = entry->GetType();
            shared_ptr<string> name = entry->GetName();

            // The protected caching entry canot be added.
            if ((!(entry->GetProtected())) && type && name)
            {
                m_evictionStrategy->IndexOperated(CachingIndex(type, name), ADDING_INDEX);
            }
        }
    }
}

/*! Process caching entries if the cache is full

    This function uses the eviction strategy to remove caching entries if the
    cache is full.

    @return NE_OK if success
*/
NB_Error
Cache::ProcessCacheFull(shared_ptr<string> type,/*!< Type of current used caching entry */
                        shared_ptr<string> name,/*!< Name of current used caching entry */
                        uint32 entryCountToAdd  /*!< Count of caching entry to add */
                        )
{
    uint32 entryCount = 0;
    PAL_Instance* pal = NULL;

    if ((!m_context) || (!m_path) || (m_path->empty()) || (!m_entryProcessor))
    {
        return NE_UNEXPECTED;
    }

    pal = NB_ContextGetPal(m_context);
    if (!pal)
    {
        return NE_UNEXPECTED;
    }

    if (!m_evictionStrategy)
    {
        // There is no method to reduce the cache.
        return NE_RES;
    }

    entryCount = m_entryProcessor->GetEntryCount() + entryCountToAdd;
    CachingIndex currentUsedIndex(type, name);
    while (entryCount > m_maximumDataCount)
    {
        CachingIndex index = m_evictionStrategy->GetFirstToRemove();

        if ((!index.m_type) || (!index.m_name))
        {
            // There is no method to reduce the cache.
            return NE_RES;
        }

        if (currentUsedIndex == index)
        {
            // Use this index and try to get another index.
            m_evictionStrategy->IndexOperated(index, USING_INDEX);
            index = m_evictionStrategy->GetFirstToRemove();

            if (currentUsedIndex == index)
            {
                // There is no method to reduce the cache.
                return NE_RES;
            }
        }

        // Remove the item from the eviction strategy.
        RemoveDataInternal(index.m_type, index.m_name);

        --entryCount;
    }

    return NE_OK;
}


/* See description in header file. */
shared_ptr <bool> Cache::IsValid()
{
    return  m_isValid;
}

/* See description in header file. */
void Cache::FileOperationThread_ProcessCachedFull(const CacheTask* pTask)
{
    const CacheTaskProcessCacheFull* task = static_cast<const CacheTaskProcessCacheFull*>(pTask);
    if (task)
    {
        ProcessCacheFull(task->m_type, task->m_name, task->m_entryCountToAdd);
    }
}

/* See description in header file. */
void Cache::FileOperationThread_InitializeCacheFiles(const CacheTask* pTask)
{
}

/* See description in header file. */
void Cache::GetData(shared_ptr <string> type,
                    shared_ptr <string> name,
                    CacheOperationCallbackPtr callback)
{
    if (callback)
    {
        if (!m_fileOperationManager)
        {
            callback->Error(NE_UNEXPECTED);
        }
        else
        {
            m_fileOperationManager->AddCustomizedTask(
                new CacheTaskGetData(this,
                                     &Cache::FileOperationThread_GetData,
                                     type, name, callback), true);
        }
    }
}

/* See description in header file. */
void Cache::SaveData(shared_ptr <string> type,
                     shared_ptr <string> name,
                     DataStreamPtr data,
                     AdditionalDataPtr additionalData,
                     CacheOperationCallbackPtr callback)
{
    NB_Error error = NE_OK;
    do
    {
        if (!m_fileOperationManager)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Make a copy of AdditionalData and add extra metadata for this cache entry.
        AdditionalDataPtr copy(additionalData ?
                               new AdditionalData(*additionalData) : new AdditionalData());
        if (!copy)
        {
            error = NE_NOMEM;
            break;
        }

        (*copy)[CACHE_ADDITIONAL_KEY_SIZE] = StringPtr(new string(StringUtility::NumberToString(data ? data->GetDataSize() : 0)));

        // @note: sanity check of type & name are differed to FileOperatingThread, this will
        //        ensure callbacks to be called synchronously. Mixed use of Synchronous and
        //        Asynchronous callback makes logic complex.
        m_fileOperationManager->AddCustomizedTask(
            new CacheTaskSaveData(this,
                                  &Cache::FileOperationThread_SaveData,
                                  type, name, data, copy, callback));

    } while (0);

    if (error != NE_OK && callback)
    {
        callback->Error(error);
    }
}

/* See description in header file. */
void Cache::RemoveData(shared_ptr <string> type,
                       shared_ptr <string> name,
                       CacheOperationCallbackPtr callback)
{
    if (m_fileOperationManager)
    {
        m_fileOperationManager->AddCustomizedTask(
            new CacheTaskRemoveData(this,
                                    &Cache::FileOperationThread_RemoveData,
                                    type, name, callback), true);
    }
    else if (callback)
    {
        callback->Error(NE_UNEXPECTED);
    }
}


/* See description in header file. */
void Cache::FileOperationThread_GetData(const CacheTask* pTask)
{
    const CacheTaskGetData* task = static_cast<const CacheTaskGetData*>(pTask);
    if (!task || !task->m_callback)
    {
        return;
    }

    const shared_ptr<string>&        type     = task->m_type;
    const shared_ptr<string>&        name     = task->m_name;
    const CacheOperationCallbackPtr& callback = task->m_callback;

    NB_Error error = NE_OK;
    CacheOperationEntityPtr result;
    do
    {
        if (!IsInitialized())
        {
            error = NE_NOTINIT;
            break;
        }

        if (!IsStringValid(type) || !IsStringValid(name))
        {
            error = NE_INVAL;
            break;
        }

        if (!IsItemExisting(type, name))
        {
            error = NE_NOENT;
            break;
        }

        result.reset(new CacheOperationEntity);
        if (!result)
        {
            error = NE_NOMEM;
            break;
        }

        result->m_type           = type;
        result->m_name           = name;
        result->m_cachingPath    = BuildCachingPath(type, name);
        if (!result->m_cachingPath)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Try to get data from m_writingList first.
        string identity = BuildCacheIdentity(type, name);
        if (!identity.empty())
        {
            nb::Lock lock(m_lock);
            PATH_DATA_MAP::iterator iter = m_writingList.find(identity);
            if (iter != m_writingList.end())
            {
                result->m_additionalData = iter->second.first;
                result->m_data           = iter->second.second;
                break;
            }
        }
        // Then try to get it from file system.
        if (!IsItemExisting(type, name))
        {
            error = NE_NOENT;
            break;
        }

        result->m_additionalData = GetAdditionalData(type, name);
        result->m_data.reset(new DataStreamImplementation);
        if (!result->m_data || !result->m_additionalData)
        {
            error = NE_UNEXPECTED;
            break;
        }

        error = FileOperatingTaskManager::GetFile(m_fileOperationManager,
                                                  result->m_cachingPath, result->m_data);
        if (error == NE_OK)
        {
            // Check if data was read correctly by checking real data size with the one
            // recorded in addtionalData.
            StringPtr sizeString = (*(result->m_additionalData))[CACHE_ADDITIONAL_KEY_SIZE];
            if (sizeString && !sizeString->empty())
            {
                uint32 size = 0;
                StringUtility::StringToNumber(*sizeString, size);
                if (size != result->m_data->GetDataSize())
                {
                    error = NE_UNEXPECTED;
                    result->m_data.reset();
                    break;
                }
            }
            else
            {
                // If there is no cache size in metadata(additionData), this cache should be
                // a saved by old client, skip size checking in this case.
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo,
                            "Cache::FileOperationThread_GetData() -- size of cache is unknown, type: %s, name: %s\n",
                            type->c_str(), name->c_str());
            }
        }
    } while (0);


    if (result)
    {
        result->m_error = error;
        callback->Success(result);
    }
    else
    {
        callback->Error(error);
    }
}

/* See description in header file. */
void Cache::FileOperationThread_SaveData(const CacheTask* pTask)
{
    const CacheTaskSaveData* task = static_cast<const CacheTaskSaveData*>(pTask);
    if (!task)
    {
        return;
    }

    const shared_ptr<string>&        type           = task->m_type;
    const shared_ptr<string>&        name           = task->m_name;
    const DataStreamPtr&             data           = task->m_data;
    const AdditionalDataPtr&         additionalData = task->m_additionalData;
    const CacheOperationCallbackPtr& callback       = task->m_callback;

    NB_Error error = NE_OK;
    CacheOperationEntityPtr entity;
    do
    {
        if (!IsInitialized())
        {
            error = NE_NOTINIT;
            break;
        }

        if (!IsStringValid(type) || !IsStringValid(name))
        {
            error = NE_INVAL;
            break;
        }

        shared_ptr<string> cachingPath = BuildCachingPath(type, name);
        if (!cachingPath)
        {
            error = NE_UNEXPECTED;
            break;
        }

        if (callback)
        {
            entity.reset(new CacheOperationEntity);
            if (!entity)
            {
                error = NE_NOMEM;
                break;
            }

            entity->m_type           = type;
            entity->m_name           = name;
            entity->m_additionalData = additionalData;
            entity->m_cachingPath    = cachingPath;
        }

        // Check if the caching entry exists.
        bool isEntryExisting = m_entryProcessor->IsEntryExisting(type, name);
        if (!isEntryExisting)
        {
            error = ProcessCacheFull(type, name, 1);
            if (error != NE_OK)
            {
                break;
            }

            // Create a new caching entry and add it to the entry processor.
            CachingEntryPtr entry(new CachingEntry(type, name, additionalData));
            if (!entry)
            {
                error = NE_NOMEM;
                break;
            }

            error = m_entryProcessor->SetEntry(entry);
            if (error != NE_OK)
            {
                break;
            }

            if (additionalData->find(CACHE_ADDITIONAL_KEY_PROTECTED) != additionalData->end())
            {
                m_entryProcessor->SetEntryProtected(type, name, true);
            }
        }

        if (data) // This function may choose to store additionalData only.
        {
            error = FileOperatingTaskManager::PutFile(m_fileOperationManager,
                                                      cachingPath, data);
            if (error != NE_OK)
            {
                RemoveDataInternal(type, name);
                break;
            }
        }

        // Notify the caching entry is used.
        if (m_evictionStrategy)
        {
            m_evictionStrategy->IndexOperated(CachingIndex(type, name),
                                              isEntryExisting ? USING_INDEX : ADDING_INDEX);
        }

    } while (0);

    // remove this item from m_writingList.
    string identity = BuildCacheIdentity(type, name);
    if (!identity.empty())
	{
        nb::Lock lock(m_lock);
        m_writingList.erase(identity);
    }

    // Invoke callback (if exists).
    if (callback)
    {
        if (entity)
        {
            entity->m_error = error;
            callback->Success(entity);
        }
        else
        {
            callback->Error(error);
        }
    }
}

/* See description in header file. */
void Cache::FileOperationThread_RemoveData(const CacheTask* pTask)
{
    const CacheTaskRemoveData* task = static_cast<const CacheTaskRemoveData*>(pTask);
    if (!task)
    {
        return;
    }

    const shared_ptr<string>&        type     = task->m_type;
    const shared_ptr<string>&        name     = task->m_name;
    const CacheOperationCallbackPtr& callback = task->m_callback;

    NB_Error error = NE_OK;
    CacheOperationEntityPtr entity;
    do
    {
        if (!IsInitialized())
        {
            error = NE_NOTINIT;
            break;
        }

        // Unlike adding operation, we don't need to check type and name when deleting
        // cache -- it is allowed to pass empty strings here to clear all tiles of specified
        // type.
        if (callback)
        {
            entity.reset(new CacheOperationEntity);
            if (!entity)
            {
                error = NE_NOMEM;
                break;
            }

            entity->m_type           = type;
            entity->m_name           = name;
            entity->m_cachingPath    = BuildCachingPath(type, name);
            if (!entity->m_cachingPath)
            {
                error = NE_UNEXPECTED;
                break;
            }
        }

        RemoveDataInternal(type, name);
    } while (0);

    if (callback)
    {
        if (entity)
        {
            entity->m_error = error;
            callback->Success(entity);
        }
        else
        {
            callback->Error(error);
        }
    }
}

/*! @} */
