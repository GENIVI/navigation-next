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

    @file       Cache.h

    Class Cache is used to save and get the data as a data stream by a
    type and a name. User could use the type and name to append, get or
    remove the data by a Cache object. This class Cache is only used for
    persistent cache and there is no memory cache implemented in this
    class for now.
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

#ifndef CACHE_H
#define CACHE_H

/*!
    @addtogroup nbcommon
    @{
*/

extern "C"
{
#include "paltypes.h"
#include "palfile.h"
#include "paltimer.h"
#include "nbcontext.h"
#include "nberror.h"
#include "pallock.h"
}

#include "base.h"
#include "smartpointer.h"
#include "CachingEntryProcessor.h"
#include "EvictionStrategy.h"
#include "FileOperatingTaskManager.h"
#include "AsyncCallback.h"
#include "datastream.h"
#include "CacheOperationCallback.h"

namespace nbcommon
{

// Types ......................................................................................

#define CACHE_ADDITIONAL_KEY_SIZE      "cache-size"
#define CACHE_ADDITIONAL_KEY_PROTECTED "protected"



class CachingIndex;
class CacheTask;

/*! Cache for persistent data */
class Cache : public Base
{
public:
    // Public functions .........................................................................

    /*! Cache constructor */
    Cache();

    /*! Cache constructor with initialization

        This function combines default constructor and Initialize. Because constructor cannot
        return an error and C++ exception cannot be used. User could check if initialization
        succeeds by function IsInitialized. And user should not call function Initialize after
        calling this constructor.
    */
    Cache(NB_Context* context,          /*!< Pointer to current context */
          uint32 maximumDataCount,      /*!< Maximum count of the data. The data is
                                             specified by the type and name. */
          shared_ptr<string> name, /*!< A name of the cache */
          shared_ptr<string> path  /*!< A path for caching the data */
          );

    /*! Cache destructor */
    virtual ~Cache();

    /*! Initialize a Cache object

        This function should be called before user calls other public functions.

        @return NE_OK if success
    */
    NB_Error
    Initialize(NB_Context* context,         /*!< Pointer to current context */
               uint32 maximumDataCount,     /*!< Maximum count of the data. The data is
                                                 specified by the type and name. */
               shared_ptr<string> name,/*!< A name of the cache */
               shared_ptr<string> path /*!< A path for caching the data */
               );

    /*! Check if this object is initialized

        This function should be called in all public functions.

        @return true if this object is initialized, false otherwise.
    */
    bool
    IsInitialized() const;

    /*! Get data specified by type & name from cache.

        This is an asynchronous function, data will be returned to caller through callback.

        @return None.
    */
    void
    GetData(shared_ptr<string> type,         /*!< Type to get the data */
            shared_ptr<string> name,         /*!< Name to get the data */
            CacheOperationCallbackPtr callback /*!< Callback to notify.*/
            );

    /*! Save data to cached.

        @return None
    */
    void
    SaveData(shared_ptr<string> type,         /*!< Type to get the data */
             shared_ptr<string> name,         /*!< Name to get the data */
             DataStreamPtr data,              /*!< Data to be saved. */
             AdditionalDataPtr  addtionalData, /*!< Additional data of this Data.*/
             CacheOperationCallbackPtr callback = CacheOperationCallbackPtr() /*!< Callback to notify.*/
             );


    void
    RemoveData(shared_ptr<string> type, /*!< Type of data */
               shared_ptr<string> name, /*!< Name of data */
               CacheOperationCallbackPtr callback = CacheOperationCallbackPtr() /*!< Callback to notify.*/
               );

    /*! Check if the item exists in the cache

        @return true if the item exists in the cache, false otherwise.
    */
    bool
    IsItemExisting(shared_ptr<string> type,/*!< Type to check if the item exists */
                   shared_ptr<string> name /*!< Name to check if the item exists */
                   );

    /*! Check if the item is protected

        @return true if the item is protected, false otherwise.
    */
    bool
    IsItemProtected(shared_ptr<string> type,   /*!< Type to check if the item is protected */
                    shared_ptr<string> name    /*!< Name to check if the item is protected */
                    );


    /*! Get the additional data of key and value

        @return Additional data of key and value
        @see SetAdditionalData
        @see AppendData
    */
    AdditionalDataPtr
    GetAdditionalData(shared_ptr<string> type, /*!< Type to get the additional data */
                      shared_ptr<string> name  /*!< Name to get the additional data */
                      );

    /*! Set the additional data of key and value

        This function sets or updates the additional data of the caching entry specified
        the type and name. Nothing to do if there is no caching entry found by the type
        and name.

        @return None
        @see GetAdditionalData
        @see AppendData
    */
    void
    SetAdditionalData(shared_ptr<string> type, /*!< Type to set the additional data */
                      shared_ptr<string> name, /*!< Name to set the additional data */
                      shared_ptr<map<string, shared_ptr<string> > > additionalData   /*!< Additional data of key and value to set */
                      );

    /*! Build the full path of the caching data by the type and name

        This function just builds the caching path by the type and name and does not ensure
        the caching entry existing.

        @return Full path of the caching data
    */
    shared_ptr<string>
    BuildCachingPath(shared_ptr<string> type,  /*!< Type to build the caching path */
                     shared_ptr<string> name   /*!< Name to build the caching path */
                     );

    /*! Get a pointer to current context

        Class ContextBasedSingleton calls this function. Cannot compile successfully without
        this function.

        @return A pointer to current context
    */
    NB_Context*
    GetContext();

    shared_ptr<bool> IsValid();
private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    Cache(const Cache& cache);
    Cache& operator=(const Cache& cache);

    /* See source file for description */

    void Reset();
    void RemovePath(const char* path);
    void InitializeEvictionStrategy();
    /*! Remove the data

      Nothing to do if there is no caching entry specified by the type and name. If the
      parameter 'type' is NULL, all items of this cache are removed. If the parameter
      'name' is NULL, all items of the specified type is removed.

      @return None
    */
    void
    RemoveDataInternal(shared_ptr<string> type,/*!< Type to remove the data. If it is NULL, all items
                                                 of this cache are removed. */
                       shared_ptr<string> name /*!< Name of data to be removed. If it is NULL, all items
                                                 will be removed. */
                       );

    NB_Error ProcessCacheFull(shared_ptr<string> type,
                              shared_ptr<string> name,
                              uint32 entryCountToAdd);

    // Functions used by tasks.
    void FileOperationThread_InitializeCacheFiles(const CacheTask* pTask);
    void FileOperationThread_ProcessCachedFull(const CacheTask* pTask);
    void FileOperationThread_GetData(const CacheTask* pTask);
    void FileOperationThread_SaveData(const CacheTask* pTask);
    void FileOperationThread_RemoveData(const CacheTask* pTask);

    // Private members ..........................................................................

    NB_Error m_initializeResult;                /*!< Result from Initialize() call, which is called in the constructor */

    NB_Context* m_context;                      /*!< Pointer to current context */
    uint32 m_maximumDataCount;                  /*!< Maximum count of the data. The data is
                                                     specified by the type and name. */
    shared_ptr<string> m_name;             /*!< A name of the cache */
    shared_ptr<string> m_path;             /*!< A path for caching the data */
    CachingEntryProcessorPtr m_entryProcessor;  /*!< A processor for the caching entries */
    shared_ptr<EvictionStrategy<CachingIndex> > m_evictionStrategy; /*!< Eviction strategy for removing a caching
                                                                         entry if the cache is full */
    map<CachingIndex, PAL_File*> m_validFiles;                 /*!< Valid file handles. These file handles
                                                                         will not be closed at next expired time. */
    map<CachingIndex, PAL_File*> m_willexpiredFiles;           /*!< File handles will be closed at next
                                                                         expired time. */
    nbcommon::FileOperatingTaskManagerPtr m_fileOperationManager;
    shared_ptr<bool> m_isValid;

    PAL_Lock* m_lock; /*! Lock for thread synchronization. */
    typedef map<string, pair<AdditionalDataPtr, DataStreamPtr> >   PATH_DATA_MAP;
    PATH_DATA_MAP m_writingList; /*! List of files needs to be written*/
};

typedef shared_ptr<Cache> CachePtr;

};  // namespace nbcommon

/*! @} */

#endif  // CACHE_H
