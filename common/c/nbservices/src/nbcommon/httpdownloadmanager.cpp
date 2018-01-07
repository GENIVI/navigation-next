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

    @file       httpdownloadmanager.cpp

    See header file for description.
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

extern "C"
{
#include "nbcontextprotected.h"
#include "paltaskqueue.h"
}

#include <algorithm>
#include <functional>
#include "httpdownloadmanager.h"
#include "downloadrequest.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"

/*! @{ */

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................

static const uint16 HTTP_DEFAULT_PORT = 80;
static const char HTTP_VERB_GET[] = "GET";
static const char URL_DELIMITER[] = "/";
static const char HOSTNAME_HTTP_IDENTIFIER[] = "http://";
static const char HOSTNAME_HTTPS_IDENTIFIER[] = "https://";
static const uint32 DOWNLOADMANAGER_INVALID_TASK_ID = (uint32) -1;

// #define LOG_PERFORMANCE

// HttpDownloadManager Public functions .........................................................

/* See header file for description */
HttpDownloadManager::HttpDownloadManager()
: m_context(NULL),
  m_downloadConnections(),
  m_downloadLevels(),
  m_downloadQueue(),
  m_scheduleTaskId(DOWNLOADMANAGER_INVALID_TASK_ID),
  m_taskScheduled(false)
{
    // Nothing to do here.
}

/* See header file for description */
HttpDownloadManager::HttpDownloadManager(NB_Context* context,
                                         uint32 downloadConnectionCount)
: m_context(NULL),
  m_downloadConnections(),
  m_downloadLevels(),
  m_downloadQueue(),
  m_scheduleTaskId(DOWNLOADMANAGER_INVALID_TASK_ID),
  m_taskScheduled(false){
    // Ignore returned error.
    Initialize(context, downloadConnectionCount);
}

/* See header file for description */
HttpDownloadManager::~HttpDownloadManager()
{
    if (m_taskScheduled)
    {
        PAL_EventTaskQueueRemove(NB_ContextGetPal(m_context),
                                 m_scheduleTaskId, TRUE);
    }

    m_context = NULL;
}

/* See header file for description */
NB_Error
HttpDownloadManager::Initialize(NB_Context* context,
                                uint32 downloadConnectionCount)
{
    NB_Error error = NE_OK;
    uint32 i = 0;

    // Check if the parameters are valid.
    if (!context || (downloadConnectionCount == 0))
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (IsInitialized())
    {
        return NE_EXIST;
    }

    // Set the context.
    m_context = context;

    // Initialize the vector of connections.
    for (i = 0; i < downloadConnectionCount; ++i)
    {
        DownloadConnection* connection = NULL;

        connection = new DownloadConnection(*this);
        if (!connection)
        {
            // Reset this 'HttpDownloadManager' object to be uninitialized.
            Reset();

            return NE_NOMEM;
        }

        error = connection->Initialize(context);
        if (error != NE_OK)
        {
            delete connection;
            connection = NULL;

            // Reset this 'HttpDownloadManager' object to be uninitialized.
            Reset();

            return error;
        }

        // Take ownership of 'DownloadConnection' object.
        DownloadConnectionPtr connectionPtr(connection);
        m_downloadConnections.push_back(connectionPtr);
    }

    return error;
}

/* See header file for description */
bool
HttpDownloadManager::IsInitialized() const
{
    if (m_context &&
        !(m_downloadConnections.empty()))
    {
        return true;
    }

    return false;
}

/* See header file for description */
NB_Context*
HttpDownloadManager::GetContext()
{
    return m_context;
}

/* See description in 'downloadmanager.h' */
NB_Error
HttpDownloadManager::RequestData(const vector<DownloadRequestPtr>& requests,
                                 uint32 levelId,
                                 vector<uint32>* duplicatedLevelIds)
{
    NB_Error error = NE_OK;
    int position = -1;

    if (requests.empty() || (levelId == INVALID_DOWNLOAD_LEVEL))
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    // Try to find the download level. If the download level does not exist, add a new one.
    error = FindLevel(levelId, &position);
    if (error == NE_NOENT)
    {
        error = CreateLevel(levelId, position);
    }

    if (error != NE_OK)
    {
        return error;
    }

    // Add requests to the download level and download queue.
    {
        DownloadLevelPtr targetLevel = m_downloadLevels.at(position);
        vector<DownloadLevelPtr> duplicatedLevels;

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!targetLevel)
        {
            return NE_UNEXPECTED;
        }

        // This function always removes duplicated requests in requested level.
        duplicatedLevels.push_back(targetLevel);

        // Get a vector of download levels by parameter 'duplicatedLevelIds'.
        if (duplicatedLevelIds)
        {
            vector<uint32>::const_iterator duplicatedIterator = duplicatedLevelIds->begin();
            vector<uint32>::const_iterator duplicatedEnd = duplicatedLevelIds->end();

            for (; duplicatedIterator != duplicatedEnd; ++duplicatedIterator)
            {
                int tempPosition = -1;
                NB_Error tempError = FindLevel(*duplicatedIterator, &tempPosition);

                // If the download level is found and it is not the requested level.
                if ((tempError == NE_OK) && (tempPosition != position))
                {
                    duplicatedLevels.push_back(m_downloadLevels.at(tempPosition));
                }
            }
        }

        // Add the requests.
        {
            vector<DownloadRequestPtr>::const_iterator requestIterator = requests.begin();
            vector<DownloadRequestPtr>::const_iterator requestEnd = requests.end();

            for (; requestIterator != requestEnd; ++requestIterator)
            {
                error = AddRequest(*requestIterator, targetLevel, duplicatedLevels);
                if (error != NE_OK)
                {
                    return error;
                }
            }
        }
    }

    // Try to trigger downloading.
    error = ProcessRequests();
    if (error != NE_OK)
    {
        return error;
    }

    return error;
}

/* See description in 'downloadmanager.h' */
NB_Error
HttpDownloadManager::CancelRequest(DownloadRequestPtr request)
{
    NB_Error error = NE_OK;

    if (!request)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    // Try to find and cancel the request in connection if it is downloading in progress.
    {
        vector<DownloadConnectionPtr>::iterator connectionIterator = m_downloadConnections.begin();
        vector<DownloadConnectionPtr>::const_iterator connectionEnd = m_downloadConnections.end();

        for (; connectionIterator != connectionEnd; ++connectionIterator)
        {
            DownloadConnectionPtr connection = *connectionIterator;

            // This pointer to 'DownloadConnection' object should be not 'NULL'.
            if (!connection)
            {
                return NE_UNEXPECTED;
            }

            uint32 levelId = INVALID_DOWNLOAD_LEVEL;
            error = connection->CancelRequest(request, &levelId);
            if (error == NE_OK)
            {
                // Remove this request from the download level.
                error = RemoveRequestFromLevel(levelId, request);

                /* Process pending requests. Because a request which is downloading in
                   progress is canceled.
                */
                error = error ? error : ProcessRequests();

                return error;
            }
        }
    }

    /* Try to find and cancel the request in download level. Because this request is not
       found in any connections. It is not downloading in progress.
    */
    {
        vector<DownloadLevelPtr>::iterator levelIterator = m_downloadLevels.begin();
        vector<DownloadLevelPtr>::const_iterator levelEnd = m_downloadLevels.end();

        for (; levelIterator != levelEnd; ++levelIterator)
        {
            int index = -1;
            DownloadLevelPtr level = *levelIterator;

            // This pointer to 'DownloadLevel' object should be not 'NULL'.
            if (!level)
            {
                return NE_UNEXPECTED;
            }

            error = level->FindEntry(request, &index);
            if (error == NE_OK)
            {
                return RemoveRequestByIndex(index, request, level);
            }
        }
    }

    return NE_NOENT;
}

/* See description in 'downloadmanager.h' */
NB_Error
HttpDownloadManager::ClearRequests(uint32 levelId,
                                   bool inProgress)
{
    NB_Error error = NE_OK;
    int position = -1;

    if (levelId == INVALID_DOWNLOAD_LEVEL)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    error = FindLevel(levelId, &position);
    if (error != NE_OK)
    {
        return error;
    }

    // Clear requests in the found download level.
    {
        DownloadLevelPtr level = m_downloadLevels.at(position);

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!level)
        {
            return NE_UNEXPECTED;
        }

        // Remove requests which are not downloading in progress.
        {
            vector<shared_ptr<ListNode<DownloadEntryPtr> > > entriesNotInProgress = level->RemoveEntriesNotInProgress();
            vector<shared_ptr<ListNode<DownloadEntryPtr> > >::const_iterator notInProgressIterator = entriesNotInProgress.begin();
            vector<shared_ptr<ListNode<DownloadEntryPtr> > >::const_iterator notInProgressEnd = entriesNotInProgress.end();
            for (; notInProgressIterator != notInProgressEnd; ++notInProgressIterator)
            {
                shared_ptr<ListNode<DownloadEntryPtr> > listNode = *notInProgressIterator;

                // Remove this request from the download queue.
                if (listNode)
                {
                    m_downloadQueue.Erase(listNode);

                    DownloadEntryPtr entry = listNode->m_data;
                    if (entry)
                    {
                        DownloadRequestPtr request = entry->GetRequest();
                        if (request)
                        {
                            // Notify the request is removed from the download queue.
                            request->RemovedFromDownloadQueue(request);
                        }
                    }
                }
            }
        }

        // Clear requests which are downloading in progress.
        if (inProgress)
        {
            // Get requests which are downloading in progress.
            vector<shared_ptr<ListNode<DownloadEntryPtr> > > entriesInProgress = level->GetEntriesInProgress();

            vector<shared_ptr<ListNode<DownloadEntryPtr> > >::const_iterator inProgressIterator = entriesInProgress.begin();
            vector<shared_ptr<ListNode<DownloadEntryPtr> > >::const_iterator inProgressEnd = entriesInProgress.end();
            for (; inProgressIterator != inProgressEnd; ++inProgressIterator)
            {
                shared_ptr<ListNode<DownloadEntryPtr> > listNode = *inProgressIterator;
                if (!listNode)
                {
                    continue;
                }

                // Get the entry.
                DownloadEntryPtr entry = listNode->m_data;
                if (!entry)
                {
                    continue;
                }

                // Get the request.
                DownloadRequestPtr request = entry->GetRequest();
                if (!request)
                {
                    continue;
                }

                // Cancel this request.
                error = CancelRequest(request);
                if (error != NE_OK)
                {
                    return error;
                }
            }

            /* Process pending requests. Because a request which is downloading in
               progress is canceled.
            */
            if (entriesInProgress.size() > 0)
            {
                error = ProcessRequests();
                if (error != NE_OK)
                {
                    return error;
                }
            }
        }
    }

    return error;
}

/* See description in 'downloadmanager.h' */
NB_Error
HttpDownloadManager::RemoveLevel(uint32 levelId)
{
    NB_Error error = NE_OK;
    int position = -1;

    if (levelId == INVALID_DOWNLOAD_LEVEL)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    error = FindLevel(levelId, &position);
    if (error != NE_OK)
    {
        return error;
    }

    // Clear pending requests of download level.
    error = ClearRequests(levelId, true);
    if (error != NE_OK)
    {
        return error;
    }

    // Remove this download level from the vector.
    m_downloadLevels.erase(m_downloadLevels.begin() + position);

    return error;
}

/* See description in 'downloadmanager.h' */
NB_Error
HttpDownloadManager::PauseDownloading(uint32 levelId,
                                      bool inProgress)
{
    NB_Error error = NE_OK;
    int position = -1;

    if (levelId == INVALID_DOWNLOAD_LEVEL)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    error = FindLevel(levelId, &position);
    if (error != NE_OK)
    {
        return error;
    }

    // Pause downloading of the found download level.
    {
        DownloadLevelPtr level = m_downloadLevels.at(position);

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!level)
        {
            return NE_UNEXPECTED;
        }

        // Check if the download level is already paused.
        if (level->IsPaused())
        {
            return NE_OK;
        }

        level->SetPaused(true);
    }

    // Cancel requests of this download level which are downloading in progress.
    if (inProgress)
    {
        vector<DownloadConnectionPtr>::iterator iterator = m_downloadConnections.begin();
        vector<DownloadConnectionPtr>::const_iterator end = m_downloadConnections.end();

        for (; iterator != end; ++iterator)
        {
            DownloadConnectionPtr connection = *iterator;

            // This pointer to 'DownloadConnection' object should be not 'NULL'.
            if (!connection)
            {
                return NE_UNEXPECTED;
            }

            /* Ignore the returned error. Because function 'CancelRequestByLevel' returns
               'NE_NOENT' if there is no request of the download level in the connection.
               This function only cancels the request, but it does not remove the request
               from the download queue.
            */
            connection->CancelRequestByLevel(levelId);
        }

        /* Process pending requests. Because a request which is downloading in
           progress is canceled.
        */
        error = ProcessRequests();
        if (error != NE_OK)
        {
            return error;
        }
    }

    return error;
}

/* See description in 'downloadmanager.h' */
NB_Error
HttpDownloadManager::ResumeDownloading(uint32 levelId)
{
    NB_Error error = NE_OK;
    int position = -1;

    if (levelId == INVALID_DOWNLOAD_LEVEL)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    error = FindLevel(levelId, &position);
    if (error != NE_OK)
    {
        return error;
    }

    // Resume downloading of the found download level.
    {
        DownloadLevelPtr level = m_downloadLevels.at(position);

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!level)
        {
            return NE_UNEXPECTED;
        }

        // Check if the download level is already resumed.
        if (!(level->IsPaused()))
        {
            return NE_OK;
        }

        level->SetPaused(false);

        // Process pending requests. Because a download level is resumed.
        if (level->GetEntryCount() > 0)
        {
            error = ProcessRequests();
            if (error != NE_OK)
            {
                return error;
            }
        }
    }

    return error;
}

/* See description in 'downloadmanager.h' */
bool
HttpDownloadManager::IsEmpty(uint32 levelId)
{
    NB_Error error = NE_OK;
    int position = -1;

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return true;
    }

    error = FindLevel(levelId, &position);
    if (error != NE_OK)
    {
        return true;
    }

    // Check if the found level is empty.
    {
        DownloadLevelPtr level = m_downloadLevels.at(position);

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!level)
        {
            return true;
        }

        return (level->GetEntryCount() == 0);
    }
}

/* See description in 'downloadmanager.h' */
bool
HttpDownloadManager::IsPaused(uint32 levelId)
{
    NB_Error error = NE_OK;
    int position = -1;

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return false;
    }

    error = FindLevel(levelId, &position);
    if (error != NE_OK)
    {
        return false;
    }

    // Check if the found level is paused.
    {
        DownloadLevelPtr level = m_downloadLevels.at(position);

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!level)
        {
            return false;
        }

        return (level->IsPaused());
    }
}


// HttpDownloadManager Private functions ........................................................

/*! Reset a 'HttpDownloadManager' object to be uninitialized

    Clear all members in this 'HttpDownloadManager' object.

    @return None
*/
void
HttpDownloadManager::Reset()
{
    // Clear all members in this 'HttpDownloadManager' object.
    m_downloadLevels.clear();
    m_downloadConnections.clear();
    m_downloadQueue.Clear();
    m_context = NULL;
}

/*! Find the position of specified download level in the vector of download levels

    The parameter 'position' (not 'NULL') is set to the found position of the vector when the
    download level is found. It is set to inserted position if the download level is not found.

    @return NE_OK if the download level is found, NE_NOENT otherwise.
*/
NB_Error
HttpDownloadManager::FindLevel(uint32 levelId,  /*!< A download level to find */
                               int* position    /*!< It is ignored if it is 'NULL'. Otherwise it
                                                     is set to the found or inserted position. */
                               )
{
    int low = 0;
    int high = m_downloadLevels.size() - 1;

    // Use the method 'Binary Insertion' to find the position of specified download level.
    while (low <= high)
    {
        int middle = (low + high) >> 1;
        uint32 middleLevelId = INVALID_DOWNLOAD_LEVEL;

        // Value of variable 'middle' should always be less than size of the vector.
        DownloadLevelPtr level = m_downloadLevels.at(middle);

        // This pointer to 'DownloadLevel' object should be not 'NULL'.
        if (!level)
        {
            return NE_UNEXPECTED;
        }

        middleLevelId = level->GetLevelId();
        if (levelId > middleLevelId)
        {
            low = middle + 1;
        }
        else if (levelId < middleLevelId)
        {
            high = middle - 1;
        }
        // The position of specified download level is found because 'level == middleLevel'.
        else
        {
            if (position)
            {
                *position = middle;
                return NE_OK;
            }
        }
    }

    if (position)
    {
        *position = low;
    }

    return NE_NOENT;
}

/*! Create a download level and add this level to specified position

    @return NE_OK if success
    @see HttpDownloadManager::FindLevel
*/
NB_Error
HttpDownloadManager::CreateLevel(uint32 levelId,    /*!< A download level to create */
                                 int position       /*!< A position added the created download queue to */
                                 )
{
    int size = m_downloadLevels.size();

    if ((levelId == INVALID_DOWNLOAD_LEVEL) ||
        (position < 0) ||
        // 'position == m_downloadLevels.size()' if the position is at the end of the vector.
        (position > size))
    {
        return NE_INVAL;
    }

    // Create a download level.
    DownloadLevelPtr level(new DownloadLevel(levelId));
    if (!level)
    {
        return NE_NOMEM;
    }

    // Initialize the download level.
    NB_Error error = level->Initialize();
    if (error != NE_OK)
    {
        return error;
    }

    // Insert the created download level to the position of the vector.
    m_downloadLevels.insert(m_downloadLevels.begin() + position, level);

    return NE_OK;
}

/*! Add a 'DownloadRequest' object to the download queue

    There are some rules for adding request in this function:
    1. If the request is downloading in progress, the new request is ignored no matter
       the download level and priority of this request.
    2. If the request is found in high (small value) download priority or same download
       priority with high download level, this request is ignored.
    3. If the request is found in low (large value) download priority or same download
       priority with low download level and it is not downloading in progress, the found
       request is removed and new request is added.
    4. The request is added directly if this request does not exist in duplicated levels.

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::AddRequest(DownloadRequestPtr request,                 /*!< A request to add */
                                DownloadLevelPtr targetLevel,               /*!< A level added request to */
                                vector<DownloadLevelPtr>& duplicatedLevels  /*!< A vector of levels contained a duplicated
                                                                                  request with parameter 'request' */
                                )
{
    NB_Error error = NE_OK;

    if ((!request) || (!targetLevel))
    {
        return NE_INVAL;
    }

    // Try to find a duplicated request in duplicated levels.
    {
        vector<DownloadLevelPtr>::iterator levelIterator = duplicatedLevels.begin();
        vector<DownloadLevelPtr>::const_iterator levelEnd = duplicatedLevels.end();

        for (; levelIterator != levelEnd; ++levelIterator)
        {
            int index = -1;
            DownloadLevelPtr level = *levelIterator;

            // This pointer to 'DownloadLevel' object should be not 'NULL'.
            if (!level)
            {
                return NE_UNEXPECTED;
            }

            error = level->FindEntry(request, &index);

            // Find the duplicated request in this level.
            if (error == NE_OK)
            {
                // Get the entry.
                shared_ptr<ListNode<DownloadEntryPtr> > listNode = level->GetEntry(index);
                if (!listNode)
                {
                    return NE_UNEXPECTED;
                }

                DownloadEntryPtr entry = listNode->m_data;
                if (!entry)
                {
                    return NE_UNEXPECTED;
                }

                // Get the duplicated request.
                DownloadRequestPtr duplicatedRequest = entry->GetRequest();
                if (!duplicatedRequest)
                {
                    return NE_UNEXPECTED;
                }

                /* 1. If the request is downloading in progress, the new request is ignored no matter
                      the download level and priority of this request.
                */
                if (duplicatedRequest->GetInProgressFlag())
                {
                    // Notify the new request will be ignored.
                    duplicatedRequest->DuplicatedRequestWillBeIgnored(request);
                    return NE_OK;
                }

                /* 2. If the request is found in high (small value) download priority or same download
                      priority with high download level, this request is ignored.
                */
                uint32 priority = request->GetPriority();
                uint32 duplicatedPriority = duplicatedRequest->GetPriority();
                if ((priority > duplicatedPriority) ||
                    ((priority == duplicatedPriority) && (targetLevel->GetLevelId() >= level->GetLevelId())))
                {
                    // Notify the new request will be ignored.
                    duplicatedRequest->DuplicatedRequestWillBeIgnored(request);
                    return NE_OK;
                }

                /* 3. If the request is found in low (large value) download priority or same download
                      priority with low download level and it is not downloading in progress, the found
                      request is removed and new request is added.
                */
                else
                {
                    error = RemoveRequestByIndex(index, duplicatedRequest, level);
                    error = error ? error : AddRequestToLevel(request, targetLevel);

                    if (error == NE_OK)
                    {
                        // Notify the old request will be ignored.
                        request->DuplicatedRequestWillBeIgnored(duplicatedRequest);
                    }

                    return error;
                }

                break;
            }
        }
    }

    // 4. The request is added directly if this request does not exist in duplicated levels.
    return AddRequestToLevel(request, targetLevel);
}

/*! Add a request to the download level and download queue

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::AddRequestToLevel(DownloadRequestPtr request,      /*!< A request to add */
                                       DownloadLevelPtr level           /*!< A level added request to */
                                       )
{
    if ((!request) || (!level))
    {
        return NE_INVAL;
    }

    // Create a new entry.
    uint32 levelId = level->GetLevelId();
    DownloadEntryPtr entry(new DownloadEntry(levelId, level->GetPausedFlag(), request));
    if (!entry)
    {
        return NE_NOMEM;
    }

    shared_ptr<ListNode<DownloadEntryPtr> > front = m_downloadQueue.Front();
    shared_ptr<ListNode<DownloadEntryPtr> > listNode = front;
    if (!front)
    {
        // Add the new entry directly if the download queue is empty.
        listNode = m_downloadQueue.PushFront(entry);
    }
    else
    {
        // Traverse the download queue to find the appropriate position to add the new entry.
        uint32 priority = request->GetPriority();
        do
        {
            // Check if the list node is valid.
            if (!listNode)
            {
                return NE_UNEXPECTED;
            }

            // Get the entry.
            DownloadEntryPtr tempEntry = listNode->m_data;
            if (!tempEntry)
            {
                return NE_UNEXPECTED;
            }

            // Get the request.
            DownloadRequestPtr tempRequest = tempEntry->GetRequest();
            if (!tempRequest)
            {
                return NE_UNEXPECTED;
            }

            /* Check if the entry in download queue has lower (large value) download priority or
               same download priority with lower download level than new entry.
            */
            uint32 tempPriority = tempRequest->GetPriority();
            if ((tempPriority > priority) ||
                ((tempPriority == priority) && (tempEntry->GetLevelId() > levelId)))
            {
                // Add the new entry to the found position of download queue.
                listNode = m_downloadQueue.Insert(listNode, entry);
                break;
            }
        }
        while ((listNode = listNode->m_next) != front);

        /* Check if the new entry has been added to download queue by comparing current list node
           with previous front node. If they are equal, add the new entry to the end of download
           queue.
        */
        if (listNode == front)
        {
            listNode = m_downloadQueue.PushBack(entry);
        }
    }

    // Check if the new entry is added successfully.
    if (!listNode)
    {
        return NE_UNEXPECTED;
    }

    // Check if the entry already exists in download level.
    int index = -1;
    NB_Error error = level->FindEntry(request, &index);
    error = (error == NE_OK) ? NE_EXIST : error;
    if (error != NE_NOENT)
    {
        // Remove the entry from download queue and return the error.
        m_downloadQueue.Erase(listNode);
        return error;
    }

    // Add the entry to download level.
    error = level->AddEntry(index, listNode);
    if (error != NE_OK)
    {
        // Remove the entry from download queue and return the error.
        m_downloadQueue.Erase(listNode);
        return error;
    }

    // Notify the request is added to the download queue.
    request->AddedToDownloadQueue(request);

    return NE_OK;
}

/*! Remove a request from the download level and download queue

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::RemoveRequestFromLevel(uint32 levelId,                 /*!< A level removed request from */
                                            DownloadRequestPtr request      /*!< A request to remove */
                                            )
{
    NB_Error error = NE_OK;

    if ((levelId == INVALID_DOWNLOAD_LEVEL) || (!request))
    {
        return NE_INVAL;
    }

    // Find the download level.
    int levelPosition = -1;
    error = FindLevel(levelId, &levelPosition);
    if (error != NE_OK)
    {
        return error;
    }
    DownloadLevelPtr level = m_downloadLevels.at(levelPosition);

    // This pointer to 'DownloadLevel' object should be not 'NULL'.
    if (!level)
    {
        return NE_UNEXPECTED;
    }

    // Find the request in download level.
    int entryIndex = -1;
    error = level->FindEntry(request, &entryIndex);
    if (error != NE_OK)
    {
        return error;
    }

    return RemoveRequestByIndex(entryIndex, request, level);
}

/*! Remove a request by the index of download level

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::RemoveRequestByIndex(int index,                    /*!< The index of the request in download level to remove */
                                          DownloadRequestPtr request,   /*!< The request to remove */
                                          DownloadLevelPtr level        /*!< A level removed request from */
                                          )
{
    if ((!request) || (!level))
    {
        return NE_INVAL;
    }

    // Remove the entry from download level.
    shared_ptr<ListNode<DownloadEntryPtr> > listNode = level->RemoveEntry(index);
    if (!listNode)
    {
        return NE_UNEXPECTED;
    }

    // Remove the entry from download queue.
    listNode = m_downloadQueue.Erase(listNode);
    if (!listNode)
    {
        return NE_UNEXPECTED;
    }

    // Notify the request is removed from the download queue.
    request->RemovedFromDownloadQueue(request);

    return NE_OK;
}

/*! Process pending download requests

    This function is used to trigger downloading.

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::ProcessRequests()
{
    NB_Error error = NE_OK;

    // Check if the download queue is empty.
    if (m_downloadQueue.Empty())
    {
        return NE_OK;
    }

#ifdef LOG_PERFORMANCE
    uint32 nr = m_downloadQueue.Size();
    if (nr % 20 == 0 || nr  <= 5)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelDebug,
                    "HttpDownloadManager: Remaining requests: %u\n",nr);
    }
#endif

    // Get requests in download queue by sequence.
    shared_ptr<ListNode<DownloadEntryPtr> > front = m_downloadQueue.Front();
    shared_ptr<ListNode<DownloadEntryPtr> > listNode = front;
    do
    {
        // Check if the list node is valid.
        if (!listNode)
        {
            return NE_UNEXPECTED;
        }

        // Get the entry.
        DownloadEntryPtr entry = listNode->m_data;
        if (!entry)
        {
            return NE_UNEXPECTED;
        }

        // Do not process requests of download level if the download level is paused.
        if (entry->IsPaused())
        {
            continue;
        }

        // Get the request.
        DownloadRequestPtr request = entry->GetRequest();
        if (!request)
        {
            return NE_UNEXPECTED;
        }

        // Check if this request is downloading in progress.
        if (request->GetInProgressFlag())
        {
            continue;
        }

        // Try to get an idle connection and request data with this connection.
        uint32 levelId = entry->GetLevelId();
        vector<DownloadConnectionPtr>::iterator connectionIterator = m_downloadConnections.begin();
        vector<DownloadConnectionPtr>::const_iterator connectionEnd = m_downloadConnections.end();
        for (; connectionIterator != connectionEnd; ++connectionIterator)
        {
            DownloadConnectionPtr connection = *connectionIterator;

            // This pointer to 'DownloadConnection' object should be not 'NULL'.
            if (!connection)
            {
                return NE_UNEXPECTED;
            }

            // Check if there is a request downloading in progress.
            if (connection->IsRequestInProgress())
            {
                continue;
            }

            // Request downloading a data by this connection.
            error = connection->RequestData(request, levelId, 0);
            if (error != NE_OK)
            {
                return error;
            }

            break;
        }

        if (connectionIterator == connectionEnd)
        {
            // No connection is idle.
            return NE_OK;
        }
    }
    while ((listNode = listNode->m_next) != front);

    // All requests are processed.
    return NE_OK;
}

/*! Compare two requests

    @return 0 if two requests are equal,
            negative sign if 'request' is less than 'anotherRequest',
            positive sign if 'request' is greater than 'anotherRequest'.
*/
int
HttpDownloadManager::CompareRequest(DownloadRequestPtr request,             /*!< A 'DownloadRequest' object to compare */
                                    DownloadRequestPtr anotherRequest       /*!< Another 'DownloadRequest' object to compare */
                                    )
{
    // Check if two pointers point to the same object.
    if (request == anotherRequest)
    {
        return 0;
    }

    // Check if one of pointers is 'NULL'.
    if (!request)
    {
        return -1;
    }
    else if (!anotherRequest)
    {
        return 1;
    }

    // Compare the URLs.
    int comparedResult = 0;
    comparedResult = StringUtility::CompareStdString(request->GetUrl(),
                                                  anotherRequest->GetUrl());
    if (comparedResult != 0)
    {
        return comparedResult;
    }

    // Compare the hostnames.
    comparedResult = StringUtility::CompareStdString(request->GetHostname(),
                                                  anotherRequest->GetHostname());
    if (comparedResult != 0)
    {
        return comparedResult;
    }

    // Compare the request headers.
    comparedResult = StringUtility::CompareStdString(request->GetRequestHeaders(),
                                                  anotherRequest->GetRequestHeaders());
    if (comparedResult != 0)
    {
        return comparedResult;
    }

    // Compare the ports.
    uint16 port = request->GetPort();
    uint16 anotherPort = anotherRequest->GetPort();
    if (port < anotherPort)
    {
        return -1;
    }
    else if (port > anotherPort)
    {
        return 1;
    }

    // Compare the protocols.
    DownloadProtocol protocol = request->GetProtocol();
    DownloadProtocol anotherProtocol = anotherRequest->GetProtocol();
    if (protocol < anotherProtocol)
    {
        return -1;
    }
    else if (protocol > anotherProtocol)
    {
        return 1;
    }

    // Two requests are equal.
    return 0;
}



/*! Schedule process remaining tasks. */
void HttpDownloadManager::ScheduleProcessTasks()
{
    if (!m_taskScheduled)
    {
        (void)PAL_EventTaskQueueAdd(NB_ContextGetPal(m_context),
                                    &HttpDownloadManager::ScheduledTasksCallback,
                                    this, &m_scheduleTaskId);
        m_taskScheduled = true;
    }
}

/*! Callback from Event Queue

    It will ask HttpDownloadManager to process remaining tasks if possible.
 */
void HttpDownloadManager::ScheduledTasksCallback(PAL_Instance* pal, void* userData)
{
    HttpDownloadManager* downloadManager = static_cast<HttpDownloadManager*>(userData);
    if (downloadManager)
    {
        downloadManager->m_taskScheduled = false;
        downloadManager->ProcessRequests();
    }
}

// DownloadEntry functions ......................................................................

/*! DownloadEntry constructor */
HttpDownloadManager::DownloadEntry::DownloadEntry(uint32 levelId,               /*!< Download level of the request */
                                                  shared_ptr<bool> isPaused,    /*!< Is the download level paused? */
                                                  DownloadRequestPtr request    /*!< A download request */
                                                  )
: m_levelId(levelId),
  m_isPaused(isPaused),
  m_request(request)
{
    // Nothing to do here.
}

/*! DownloadEntry destructor */
HttpDownloadManager::DownloadEntry::~DownloadEntry()
{
    // Nothing to do here.
}

/*! Get download level of the request

    @return Download level of the request
*/
uint32
HttpDownloadManager::DownloadEntry::GetLevelId() const
{
    return m_levelId;
}

/*! Check if the request is paused

    @return Flag if the request is paused
*/
bool
HttpDownloadManager::DownloadEntry::IsPaused() const
{
    if (m_isPaused)
    {
        return (*m_isPaused);
    }

    return false;
}

/*! Get the request

    @return Shared pointer to the request
*/
DownloadRequestPtr
HttpDownloadManager::DownloadEntry::GetRequest()
{
    return m_request;
}


// DownloadLevel functions ......................................................................

/*! DownloadLevel constructor */
HttpDownloadManager::DownloadLevel::DownloadLevel(uint32 levelId    /*!< The ID of the download level */
                                                  )
: m_levelId(levelId),
  m_isPaused(),
  m_entries()
{
    // Nothing to do here.
}

/*! DownloadLevel destructor */
HttpDownloadManager::DownloadLevel::~DownloadLevel()
{
    // Nothing to do here.
}

/*! Initialize a 'DownloadLevel' object

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::DownloadLevel::Initialize()
{
    // Check if this object is initialized.
    if (m_isPaused)
    {
        return NE_EXIST;
    }

    // Create the paused flag.
    m_isPaused.reset(new bool(false));
    if (!m_isPaused)
    {
        return NE_NOMEM;
    }

    return NE_OK;
}

/*! Get the ID of the download level

    @return The ID of the download level
*/
uint32
HttpDownloadManager::DownloadLevel::GetLevelId() const
{
    return m_levelId;
}

/*! Set paused flag of the download level

    @return None
*/
void
HttpDownloadManager::DownloadLevel::SetPaused(bool isPaused     /*!< Is the download level paused? */
                                              )
{
    if (m_isPaused)
    {
        (*m_isPaused) = isPaused;
    }
}

/*! Check if the download level is paused

    @return Flag if the download level is paused
*/
bool
HttpDownloadManager::DownloadLevel::IsPaused() const
{
    if (m_isPaused)
    {
        return (*m_isPaused);
    }

    return false;
}

/*! Get a shared pointer to the paused flag of the download level

    @return A shared pointer to the paused flag of the download level
*/
shared_ptr<bool>
HttpDownloadManager::DownloadLevel::GetPausedFlag()
{
    return m_isPaused;
}

/*! Get the count of entries in the download level

    @return The count of entries in the download level
*/
int
HttpDownloadManager::DownloadLevel::GetEntryCount() const
{
    return static_cast<int>(m_entries.size());
}

/*! Add an entry to the download level

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::DownloadLevel::AddEntry(int index,                                     /*!< An index added the entry to */
                                             shared_ptr<ListNode<DownloadEntryPtr> > entry  /*!< An entry to add */
                                             )
{
    int size = m_entries.size();

    if ((index < 0) ||
        // 'index == m_entries.size()' if the index is at the end of the vector.
        (index > size) ||
        (!entry))
    {
        return NE_INVAL;
    }

    // Insert the entry to the index of the vector.
    m_entries.insert(m_entries.begin() + index, entry);

    return NE_OK;
}

/*! Find the index of specified request in the vector of entries

    The parameter 'index' (not 'NULL') is set to the found index of the vector when the
    entry is found. It is set to inserted position if the entry is not found.

    @return NE_OK if the entry is found, NE_NOENT otherwise.
*/
NB_Error
HttpDownloadManager::DownloadLevel::FindEntry(DownloadRequestPtr request,   /*!< A request to find */
                                              int* index                    /*!< It is ignored if it is 'NULL'. Otherwise it
                                                                                 is set to the found or inserted position. */
                                              )
{
    int low = 0;
    int high = m_entries.size() - 1;

    // Use the method 'Binary Insertion' to find the position.
    while (low <= high)
    {
        int middle = (low + high) >> 1;

        // Value of variable 'middle' should always be less than size of the vector.
        shared_ptr<ListNode<DownloadEntryPtr> > listNode = m_entries.at(middle);
        if (!listNode)
        {
            return NE_UNEXPECTED;
        }

        // Get the entry.
        DownloadEntryPtr entry = listNode->m_data;
        if (!entry)
        {
            return NE_UNEXPECTED;
        }

        // Get the request.
        DownloadRequestPtr middleRequest = entry->GetRequest();
        if (!middleRequest)
        {
            return NE_UNEXPECTED;
        }

        int comparedResult = HttpDownloadManager::CompareRequest(request, middleRequest);
        if (comparedResult > 0)
        {
            low = middle + 1;
        }
        else if (comparedResult < 0)
        {
            high = middle - 1;
        }
        // The index is found.
        else
        {
            if (index)
            {
                *index = middle;
                return NE_OK;
            }
        }
    }

    if (index)
    {
        *index = low;
    }

    return NE_NOENT;
}

/*! Get an entry of the download level by index

    This function returns 'NULL' if the index is out of range.

    @return Shared pointer to the list node of entry
*/
shared_ptr<ListNode<HttpDownloadManager::DownloadEntryPtr> >
HttpDownloadManager::DownloadLevel::GetEntry(int index      /*!< Index to get the entry */
                                             )
{
    int entryCount = m_entries.size();
    if ((index >= 0) && (index < entryCount))
    {
        return m_entries.at(index);
    }

    // Return 'NULL' if the index is out of range.
    return shared_ptr<ListNode<DownloadEntryPtr> >();
}

/*! Remove an entry from the download level by index

    @return NE_OK if success
*/
shared_ptr<ListNode<HttpDownloadManager::DownloadEntryPtr> >
HttpDownloadManager::DownloadLevel::RemoveEntry(int index   /*!< Index of the entry to remove */
                                                )
{
    int size = m_entries.size();

    if ((index >= 0) && (index < size))
    {
        // The vector of entries should not be empty, because (0 <= index < size).
        shared_ptr<ListNode<DownloadEntryPtr> > entryToRemove = m_entries.at(index);
        m_entries.erase(m_entries.begin() + index);
        return entryToRemove;
    }

    return shared_ptr<ListNode<DownloadEntryPtr> >();
}

/*! Get the entries which are downloading in progress

    @return A vector contained the list nodes of entries which are downloading in progress
*/
vector<shared_ptr<ListNode<HttpDownloadManager::DownloadEntryPtr> > >
HttpDownloadManager::DownloadLevel::GetEntriesInProgress()
{
    vector<shared_ptr<ListNode<DownloadEntryPtr> > > entriesInProgress;
    vector<shared_ptr<ListNode<DownloadEntryPtr> > >::const_iterator entryIterator = m_entries.begin();
    vector<shared_ptr<ListNode<DownloadEntryPtr> > >::const_iterator entryEnd = m_entries.end();
    for (; entryIterator != entryEnd; ++entryIterator)
    {
        shared_ptr<ListNode<DownloadEntryPtr> > listNode = *entryIterator;
        if (!listNode)
        {
            continue;
        }

        // Get the entry.
        DownloadEntryPtr entry = listNode->m_data;
        if (!entry)
        {
            continue;
        }

        // Get the request.
        DownloadRequestPtr request = entry->GetRequest();
        if (!request)
        {
            continue;
        }

        /* Cannot exit the loop when the request is not downloading in progress. Because
           the requests have download priority. The request of high download priority may
           be inserted when the request of low download priority is downloading in progress.
        */
        if (request->GetInProgressFlag())
        {
            entriesInProgress.push_back(listNode);
        }
    }

    return entriesInProgress;
}

/*! Remove the entries which are not downloading in progress

    @return A vector contained the removed list nodes of entries which are not downloading
            in progress
*/
vector<shared_ptr<ListNode<HttpDownloadManager::DownloadEntryPtr> > >
HttpDownloadManager::DownloadLevel::RemoveEntriesNotInProgress()
{
    int i = m_entries.size() - 1;
    vector<shared_ptr<ListNode<DownloadEntryPtr> > >::iterator begin = m_entries.begin();
    vector<shared_ptr<ListNode<DownloadEntryPtr> > > entriesNotInProgress;

    /* Remove the request which is not downloading in progress. Requests have download
       priority. The request of high download priority may be inserted when the request
       of low download priority is downloading in progress.
    */
    for (; i >= 0; --i)
    {
        shared_ptr<ListNode<DownloadEntryPtr> > listNode = m_entries.at(i);
        if (listNode)
        {
            // Get the entry.
            DownloadEntryPtr entry = listNode->m_data;
            if (entry)
            {
                // Get the request.
                DownloadRequestPtr request = entry->GetRequest();
                if (request)
                {
                    // Check if the request is downloading in progress.
                    if (request->GetInProgressFlag())
                    {
                        continue;
                    }
                    else
                    {
                        // Add the list node to the vector to return.
                        entriesNotInProgress.push_back(listNode);
                    }
                }
            }
        }

        // Erase the list node which is empty or not downloading in progress.
        m_entries.erase(begin + i);
    }

    return entriesNotInProgress;
}


// DownloadConnection public functions ..........................................................

/*! DownloadConnection constructor */
HttpDownloadManager::DownloadConnection::DownloadConnection(HttpDownloadManager& httpDownloadManager    /*!< 'HttpDownloadManager' object */
                                                            )
: m_currentPort(HTTP_DEFAULT_PORT),
  m_currentRetryTimes(0),
  m_levelIdInProgress(INVALID_DOWNLOAD_LEVEL),
  m_protocol(DOWNLOAD_PROTOCOL_UNDEFINED),
  m_httpDownloadManager(httpDownloadManager),
  m_context(NULL),
  m_httpConnection(NULL),
  m_currentHostname(),
  m_requestInProgress()
{
    // Nothing to do here.
}

/*! DownloadConnection destructor */
HttpDownloadManager::DownloadConnection::~DownloadConnection()
{
    /* Cancel the downloading request but do not remove this 'DownloadRequest' object
       from download queue. */
    if (m_requestInProgress)
    {
        CancelRequest(m_requestInProgress, NULL);
    }

    /* Destroy the PAL connection. Cancel the current request above. So there is no
       item to request in this function. */
    DestroyPalConnectionToRequest();

    // Set context NULL.
    m_context = NULL;
}

/*! Initialize a 'DownloadConnection' object

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::DownloadConnection::Initialize(NB_Context* context     /*!< Pointer to current context */
                                                    )
{
    if (!context)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (IsInitialized())
    {
        return NE_EXIST;
    }

    // Set context.
    m_context = context;

    return NE_OK;
}

/*! Check if there is a request downloading in progress

    @return true if there is a request downloading in progress, false otherwise
*/
bool
HttpDownloadManager::DownloadConnection::IsRequestInProgress() const
{
    return (m_requestInProgress != NULL);
}

/*! Request downloading a data by a 'DownloadRequest' object

    @return NE_OK if success, NE_BUSY if there is another request downloading in progress.
*/
NB_Error
HttpDownloadManager::DownloadConnection::RequestData(DownloadRequestPtr request,                    /*!< A 'DownloadRequest' object to request */
                                                     uint32 levelId,                                /*!< Download level of this request */
                                                     uint32 currentRetryTimes                       /*!< Current retry times. It is only used to
                                                                                                         retry the request. Each new request should
                                                                                                         set this parameter to 0. */
                                                     )
{
    if (!request)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    // Check if there is a request downloading in progress.
    if (IsRequestInProgress())
    {
        return NE_BUSY;
    }

    m_requestInProgress = request;
    m_requestInProgress->SetInProgressFlag(true);
    m_requestInProgress->SetDataStream(DataStreamPtr());
    m_levelIdInProgress = levelId;
    m_currentRetryTimes = currentRetryTimes;

    // Process this request.
    ProcessRequest();

    return NE_OK;
}

/*! Cancel download request of the specified 'DownloadRequest' object

    @return NE_OK if success,
            NE_NOENT if this request is not downloading in this connection.
*/
NB_Error
HttpDownloadManager::DownloadConnection::CancelRequest(DownloadRequestPtr request,                  /*!< A 'DownloadRequest' object to cancel request */
                                                       uint32* levelId                              /*!< Return download level of cancelled request when
                                                                                                             this pointer is not 'NULL' and the function
                                                                                                             returns 'NE_OK'. */
                                                       )
{
    NB_Error error = NE_OK;

    if (!request)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    // Check if they are the same requests.
    if (HttpDownloadManager::CompareRequest(m_requestInProgress, request) != 0)
    {
        return NE_NOENT;
    }

    /* Destroy the HTTP network connection to cancel the request which is downloading
       in progress.

       TRICKY: Do not call the function PAL_NetCloseConnection when the request is
               downloading in progress. Because the function PAL_NetCloseConnection
               may not cancel the current request if the request is not sent.
               I check the code of the HTTP connection of iphone and bb10, there is
               a queue saved the pending requests. The request is removed from the
               queue when it is sent. But if the request has not been removed from
               the queue when user calls the function PAL_NetCloseConnection, the
               current request is remained in the queue. I could not know about this
               operation outside of PAL, so I decide to destroy this connection.
    */
    if (m_httpConnection)
    {
        PAL_NetDestroyConnection(m_httpConnection);
        m_httpConnection = NULL;

        m_currentPort = HTTP_DEFAULT_PORT;
        m_currentHostname.reset();
    }

    // Clear the item which is downloading in progress.
    m_requestInProgress->SetInProgressFlag(false);
    m_requestInProgress->SetDataStream(DataStreamPtr());
    m_requestInProgress = DownloadRequestPtr();

    if (levelId)
    {
        *levelId = m_levelIdInProgress;
    }
    m_levelIdInProgress = INVALID_DOWNLOAD_LEVEL;

    return error;
}

/*! Cancel download request by a specified download level

    This function only cancels the request, but it does not remove the request from
    the download queue.

    @return NE_OK if success,
            NE_NOENT if there is no request of the download level in this connection.
*/
NB_Error
HttpDownloadManager::DownloadConnection::CancelRequestByLevel(uint32 levelId    /*!< Download level of the request to cancel */
                                                              )
{
    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    if ((!m_requestInProgress) || (m_levelIdInProgress != levelId))
    {
        return NE_NOENT;
    }

    return CancelRequest(m_requestInProgress, NULL);
}


// DownloadConnection private functions .........................................................

/*! Check if this 'DownloadConnection' object is initialized

    This function should be called in all public functions of class 'DownloadConnection'.

    @return Flag if this 'DownloadConnection' object is initialized
*/
bool
HttpDownloadManager::DownloadConnection::IsInitialized() const
{
    if (m_context)
    {
        return true;
    }

    return false;
}

/*! Create the 'PAL_NetConnection' instance if it is 'NULL'

    @return NE_OK if success
*/
NB_Error
HttpDownloadManager::DownloadConnection::CreatePalConnectionIfNotExist(DownloadRequestPtr request   /*!< Current request to download which need to
                                                                                                         create a connection */
                                                                       )
{
    PAL_Error palError = PAL_Ok;
    PAL_NetConnectionProtocol palProtocol = PNCP_Undefined;
    DownloadProtocol downloadProtocol = DOWNLOAD_PROTOCOL_UNDEFINED;
    PAL_Instance* pal = NULL;

    if (!request)
    {
        return NE_INVAL;
    }

    // Check if this object is initialized.
    if (!IsInitialized())
    {
        return NE_NOTINIT;
    }

    // Check if HTTP connection has already been created.
    if (m_httpConnection)
    {
        return NE_OK;
    }

    // Get the PAL instance from the context.
    pal = NB_ContextGetPal(m_context);
    if (!pal)
    {
        return NE_UNEXPECTED;
    }

    // Get the download protocol from request.
    downloadProtocol = request->GetProtocol();

    // Check if the download protocol is undefined.
    if (downloadProtocol == DOWNLOAD_PROTOCOL_UNDEFINED)
    {
        // Try to parse the download protocol from request.
        shared_ptr<string> standardizedHostname = StandardizeHostname(request,
                                                                      downloadProtocol);
        if ((!standardizedHostname) ||
            (downloadProtocol == DOWNLOAD_PROTOCOL_UNDEFINED))
        {
            return NE_UNEXPECTED;
        }
    }

    // Convert the download protocol to a connection protocol of PAL.
    switch (downloadProtocol)
    {
        case DOWNLOAD_PROTOCOL_HTTP:
        {
            palProtocol = PNCP_HTTP;
            break;
        }
        case DOWNLOAD_PROTOCOL_HTTPS:
        {
            palProtocol = PNCP_HTTPS;
            break;
        }
        default:
        {
            break;
        }
    }

    // Check if the protocol is supported by this download manager.
    if (palProtocol == PNCP_Undefined)
    {
        return NE_NOSUPPORT;
    }

    // Create HTTP network connection.
    palError = PAL_NetCreateConnection(pal, palProtocol, &m_httpConnection);
    if (palError != PAL_Ok)
    {
        return NE_NET;
    }

    // Save the protocol of current connection.
    m_protocol = downloadProtocol;
    return NE_OK;
}

/*! Destroy the 'PAL_NetConnection' instance and request current item

    This function destroys the PAL connection. And if the current item is not empty,
    request this item.

    @return None
*/
void
HttpDownloadManager::DownloadConnection::DestroyPalConnectionToRequest()
{
    // Destroy HTTP network connection.
    if (m_httpConnection)
    {
        PAL_NetConnection* httpConnection = m_httpConnection;

        m_httpConnection = NULL;
        PAL_NetDestroyConnection(httpConnection);

        m_currentPort = HTTP_DEFAULT_PORT;
        m_currentHostname.reset();
        m_protocol = DOWNLOAD_PROTOCOL_UNDEFINED;
    }

    // If the current item is not empty, request this item.
    if (m_requestInProgress)
    {
        // Clear the received data and process this request.
        m_requestInProgress->SetDataStream(DataStreamPtr());
        ProcessRequest();
    }
}

/*! Process pending download request in the 'DownloadConnection' object

    This function is used to trigger downloading.

    TRICKY: If there is an error occurred in this function, the request should not be
            retried. There are 2 reasons:
            1. The error is not caused by a network issue.
            2. The retry logic causes the recursive call for this function ProcessRequest.

    @return None
*/
void
HttpDownloadManager::DownloadConnection::ProcessRequest()
{
    NB_Error error = NE_OK;
    PAL_Error palError = PAL_Ok;
    PAL_NetConnectionStatus status = PNCS_Undefined;

    if (!m_requestInProgress)
    {
        // Nothing to do if there is no request.
        return;
    }

    error = CreatePalConnectionIfNotExist(m_requestInProgress);
    if (error != NE_OK)
    {
        NotifyRequestError(error);
        return;
    }

    status = PAL_NetGetStatus(m_httpConnection);
    switch (status)
    {
        case PNCS_Initialized:
        case PNCS_Closed:
        {
            uint16 port = m_requestInProgress->GetPort();
            DownloadProtocol protocol = DOWNLOAD_PROTOCOL_UNDEFINED;
            shared_ptr<string> standardizedHostname = StandardizeHostname(m_requestInProgress,
                                                                          protocol);
            PAL_NetConnectionConfig configuration = {0};

            // Check if protocol and hostname of this request are valid.
            if ((protocol == DOWNLOAD_PROTOCOL_UNDEFINED) ||
                (!standardizedHostname) || standardizedHostname->empty())
            {
                NotifyRequestError(NE_UNEXPECTED);
                return;
            }

            /* If current protocol of PAL connection is not same with this request, destroy the
               PAL connection and request this item. */
            if (protocol != m_protocol)
            {
                DestroyPalConnectionToRequest();
                return;
            }

            // Set network configuration.
            configuration.netStatusCallback = &StatusCallback;
            configuration.netHttpResponseStatusCallback = &ResponseStatusCallback;
            configuration.netHttpDataReceivedCallback = &DataReceivedCallback;
            configuration.netHttpResponseHeadersCallback = &ResponseHeadersCallback;
            configuration.userData = this;

            /* Open the HTTP connection. This function gets called again from the status callback
               function once the connection is connected. */
            palError = PAL_NetOpenConnection(m_httpConnection,
                                             &configuration,
                                             standardizedHostname->c_str(),
                                             port);
            if (palError != PAL_Ok)
            {
                NotifyRequestError(NE_NET);
                return;
            }

            /* Save the port and hostname of this HTTP connection. They are used to check if the
               HTTP connection should be reconnected for each request. Because requests may have
               different port or hostname of HTTP.
            */
            m_currentPort = port;
            m_currentHostname = standardizedHostname;

            break;
        }

        case PNCS_Connected:
        {
            uint16 port = m_requestInProgress->GetPort();
            const char* requestHeadersString = NULL;
            DownloadProtocol protocol = DOWNLOAD_PROTOCOL_UNDEFINED;
            shared_ptr<string> standardizedHostname = StandardizeHostname(m_requestInProgress,
                                                                          protocol);
            shared_ptr<string> requestHeaders = m_requestInProgress->GetRequestHeaders();

            if (requestHeaders)
            {
                requestHeadersString = requestHeaders->c_str();
            }

            // Check if protocol and hostname of this request are valid.
            if ((protocol == DOWNLOAD_PROTOCOL_UNDEFINED) ||
                (!standardizedHostname) || standardizedHostname->empty())
            {
                NotifyRequestError(NE_UNEXPECTED);
                return;
            }

            /* If current protocol of PAL connection is not same with this request, destroy the
             PAL connection and request this item. */
            if (protocol != m_protocol)
            {
                DestroyPalConnectionToRequest();
                return;
            }

            /* Close this HTTP connection if this request has different port or hostname of HTTP
               with current connection. */
            if ((m_currentPort != port) ||
                (!StringUtility::IsStringEqual(m_currentHostname, standardizedHostname)))
            {
                PAL_NetCloseConnection(m_httpConnection);
                return;
            }

            // Standardize the URL and send the request.
            {
                shared_ptr<string> standardizedUrl = StandardizeUrl(m_requestInProgress);

                // URL of this request cannot be 'NULL'.
                if ((!standardizedUrl) || standardizedUrl->empty())
                {
                    NotifyRequestError(NE_UNEXPECTED);
                    return;
                }

                palError = PAL_NetHttpSend(m_httpConnection,
                                           NULL,
                                           0,
                                           HTTP_VERB_GET,
                                           standardizedUrl->c_str(),
                                           NULL,
                                           requestHeadersString,
                                           m_requestInProgress.get());
                if (palError != PAL_Ok)
                {
                    NotifyRequestError(NE_NET);
                    return;
                }
            }

            break;
        }

        case PNCS_Failed:
        case PNCS_Error:
        {
            /* Close this HTTP connection if it returns failed or error. The function 'ProcessRequest'
               is called again when the HTTP connection is closed.
            */
            PAL_NetCloseConnection(m_httpConnection);

            break;
        }

        case PNCS_Undefined:
        case PNCS_Created:
        case PNCS_Resolving:
        case PNCS_Connecting:
        case PNCS_Closing:
        case PNCS_ProxyAuthRequired:
        case PNCS_PppStateChanged:
        default:
        {
            // Ignore these states.
            break;
        }
    }
}

/*! Notify the error code for the current request in the connection

    @return None
*/
void
HttpDownloadManager::DownloadConnection::NotifyRequestError(NB_Error errorCode      /*!< Error code to notify */
                                                            )
{
    uint32 tempLevelId = m_levelIdInProgress;
    DownloadRequestPtr tempRequest = m_requestInProgress;

    if (!tempRequest)
    {
        // Nothing to do if there is no request.
        return;
    }

    // Clear the item which is downloading in progress.
    tempRequest->SetInProgressFlag(false);
    m_requestInProgress = DownloadRequestPtr();
    m_levelIdInProgress = INVALID_DOWNLOAD_LEVEL;

    // Remove this request from the download level of HTTP download manager.
    m_httpDownloadManager.RemoveRequestFromLevel(tempLevelId, tempRequest);

    // Add another task to process pending requests in HTTP download manager.
    m_httpDownloadManager.ScheduleProcessTasks();

    // Report downloading this request with a 'NB_Error'.
    tempRequest->DataError(errorCode);
}

/*! Standardize the hostname of request for opening connection

    Remove the 'http://' or 'https://' at the beginning and the '/' at the end.

    @return Standardized hostname
*/
shared_ptr<string>
HttpDownloadManager::DownloadConnection::StandardizeHostname(DownloadRequestPtr request,    /*!< Request to standardize hostname */
                                                             DownloadProtocol& protocol     /*!< On return protocol of the request if
                                                                                                 success. If the protocol cannot be got
                                                                                                 from request, return DOWNLOAD_PROTOCOL_HTTP
                                                                                                 as default value. */
                                                             )
{
    bool isHttpIdentifier = false;
    bool isHttpsIdentifier = false;
    bool isUrlDelimiter = false;
    string::size_type hostnameLength = 0;
    string::size_type httpIdentifierLength = (string::size_type) nsl_strlen(HOSTNAME_HTTP_IDENTIFIER);
    string::size_type httpsIdentifierLength = (string::size_type) nsl_strlen(HOSTNAME_HTTPS_IDENTIFIER);
    string::size_type urlDelimiterLength = (string::size_type) nsl_strlen(URL_DELIMITER);
    string* standardizedHostname = NULL;

    // Check if the request is valid.
    if (!request)
    {
        return shared_ptr<string>();
    }

    // Get the hostname from request.
    shared_ptr<string> hostname = request->GetHostname();
    if ((!hostname) || hostname->empty())
    {
        return shared_ptr<string>();
    }

    hostnameLength = hostname->size();

    const char* hostnameChar = hostname->c_str();
    if ((!hostnameChar) || (nsl_strlen(hostnameChar) == 0))
    {
        return shared_ptr<string>();
    }

    // Get the flag if there is a 'http://' at the beginning.
    isHttpIdentifier = (nsl_strnicmp(hostnameChar,
                                     HOSTNAME_HTTP_IDENTIFIER,
                                     httpIdentifierLength) == 0);

    // Get the flag if there is a 'https://' at the beginning.
    if (!isHttpIdentifier)
    {
        isHttpsIdentifier = (nsl_strnicmp(hostnameChar,
                                          HOSTNAME_HTTPS_IDENTIFIER,
                                          httpsIdentifierLength) == 0);
    }

    // Get the flag if there is a '/' at the end.
    if (hostnameLength >= urlDelimiterLength)
    {
        isUrlDelimiter = (hostname->compare(hostnameLength - urlDelimiterLength,
                                            urlDelimiterLength,
                                            URL_DELIMITER) == 0);
    }

    if (!(isHttpIdentifier || isHttpsIdentifier || isUrlDelimiter))
    {
        // Cannot get protocol from hostname. Try to get protocol from request.
        protocol = request->GetProtocol();

        // If the protocol is undefined, return DOWNLOAD_PROTOCOL_HTTP as default value.
        if (protocol == DOWNLOAD_PROTOCOL_UNDEFINED)
        {
            protocol = DOWNLOAD_PROTOCOL_HTTP;
        }

        return hostname;
    }

    // Create a string for standardized hostname.
    standardizedHostname = new string(*hostname);
    if (!standardizedHostname)
    {
        // Do not return protocol if failed.
        return shared_ptr<string>();
    }

    /* Remove the '/' at the end first. Because the position is invalid after removing the
       'http://' at the beginning. */
    if (isUrlDelimiter)
    {
        standardizedHostname->erase(hostnameLength - urlDelimiterLength, urlDelimiterLength);
    }

    // Remove the 'http://' or 'https://' at the beginning.
    if (isHttpIdentifier)
    {
        standardizedHostname->erase(0, httpIdentifierLength);
        protocol = DOWNLOAD_PROTOCOL_HTTP;
    }
    else if (isHttpsIdentifier)
    {
        standardizedHostname->erase(0, httpsIdentifierLength);
        protocol = DOWNLOAD_PROTOCOL_HTTPS;
    }
    else
    {
        // Cannot get protocol from hostname. Try to get protocol from request.
        protocol = request->GetProtocol();

        // If the protocol is undefined, return DOWNLOAD_PROTOCOL_HTTP as default value.
        if (protocol == DOWNLOAD_PROTOCOL_UNDEFINED)
        {
            protocol = DOWNLOAD_PROTOCOL_HTTP;
        }
    }

    return shared_ptr<string>(standardizedHostname);
}

/*! Standardize the URL of request for sending the request

    If there is not a '/' at the beginning of the URL add the '/'.

    @return Standardized URL
*/
shared_ptr<string>
HttpDownloadManager::DownloadConnection::StandardizeUrl(DownloadRequestPtr request      /*!< Request to standardize URL */
                                                        )
{
    string* standardizedUrl = NULL;

    // Check if the request is valid.
    if (!request)
    {
        return shared_ptr<string>();
    }

    // Get the URL from request.
    shared_ptr<string> url = request->GetUrl();
    if ((!url) || url->empty())
    {
        return shared_ptr<string>();
    }

    // Check if there is a '/' at the beginning of the URL.
    if (url->compare(0, nsl_strlen(URL_DELIMITER), URL_DELIMITER) == 0)
    {
        return shared_ptr<string>(url);
    }

    // Create a string for standardized URL and add the '/' at the beginning.
    standardizedUrl = new string(URL_DELIMITER);
    if (!standardizedUrl)
    {
        return shared_ptr<string>();
    }

    standardizedUrl->append(*url);

    return shared_ptr<string>(standardizedUrl);
}

/*! Translate a PAL error of HTTP to a NB error

    This function returns NE_OK if the parameter is PAL_Ok. Otherwises this function returns NE_NET
    by default. Other translated NB errors are all final errors for downloading. The request should
    not be retried when downloading returns these errors.

    @return A NB error translated from a PAL error of HTTP
*/
NB_Error
HttpDownloadManager::DownloadConnection::TranslateHttpPalErrorToNeError(PAL_Error palError  /*!< A PAL error to translate */
                                                                        )
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

/*! Callback function for PAL network connection status change notification

    @return None
*/
void
HttpDownloadManager::DownloadConnection::StatusCallback(void* userData,
                                                        PAL_NetConnectionStatus status)
{
    // The user data points to a connection.
    DownloadConnection* connection = (DownloadConnection*) userData;

    if ((!connection) || (!(connection->m_httpConnection)))
    {
        // The pointers to 'DownloadConnection' object and HTTP connection cannot be 'NULL' here.
        return;
    }

    switch (status)
    {
        case PNCS_Closed:
        case PNCS_Connected:
        {
            // Try to trigger downloading.
            connection->ProcessRequest();
            break;
        }

        case PNCS_Failed:
        case PNCS_Error:
        {
            // Retry this request.
            connection->NotifyRequestError(NE_NET);
            break;
        }

        case PNCS_Undefined:
        case PNCS_Created:
        case PNCS_Initialized:
        case PNCS_Resolving:
        case PNCS_Connecting:
        case PNCS_Closing:
        case PNCS_ProxyAuthRequired:
        case PNCS_PppStateChanged:
        default:
        {
            // Ignore these states.
            break;
        }
    }
}

/*! Callback function for HTTP response status code notification

    @return None
*/
void
HttpDownloadManager::DownloadConnection::ResponseStatusCallback(PAL_Error errorCode,
                                                                void* userData,
                                                                void* requestData,
                                                                uint32 contentLength)
{
    NB_Error nbError = NE_OK;

    // The user data points to a connection.
    DownloadConnection* connection = (DownloadConnection*) userData;

    if ((!connection) ||
        (!(connection->m_httpConnection)) ||
        (!(connection->m_requestInProgress)) ||
        (connection->m_requestInProgress.get() != requestData))
    {
        return;
    }

    // Translate the returned PAL error of HTTP to a NB error.
    nbError = TranslateHttpPalErrorToNeError(errorCode);
    connection->m_requestInProgress->SetResponseError(nbError);

    if (errorCode == PAL_Ok || nbError == NE_OK)
    {
        // Nothing to do if the error is NE_OK.
        return;
    }
    else if (nbError == NE_NET)
    {
        // If the error is a general network error (NE_NET) this request should be retried.
        connection->NotifyRequestError(NE_NET);
        return;
    }
    // Todo - This is just temp code. Currently server has some issues for this case. Will be removed later.
    // Based on SDS, we should NOT retry for HTTP 502. Per Daniel's request, this retry is added for 2012-04-12 build.
    else if (nbError == NE_HTTP_BAD_GATEWAY)
    {
        connection->NotifyRequestError(NE_HTTP_BAD_GATEWAY);
        return;
    }

    // Report this HTTP error directly.
    connection->NotifyRequestError(nbError);
}

/*! Callback function for PAL network connection data received notification

    @return None
*/
void
HttpDownloadManager::DownloadConnection::DataReceivedCallback(void* userData,
                                                              void* requestData,
                                                              PAL_Error errorCode,
                                                              const byte* bytes,
                                                              uint32 count)
{
    NB_Error error = NE_OK;

    // The user data points to a connection.
    DownloadConnection* connection = (DownloadConnection*) userData;

    if ((!connection) ||
        (!(connection->m_httpConnection)) ||
        (!(connection->m_requestInProgress)) ||
        (connection->m_requestInProgress.get() != requestData))
    {
        return;
    }

    // Check if any error occur when downloading.
    if (errorCode != PAL_Ok)
    {
        // Clear the data stream in this request.
        connection->m_requestInProgress->SetDataStream(DataStreamPtr());

        connection->NotifyRequestError(NE_NET);
        return;
    }

    // Check if this connection returns any data.
    if (bytes && (count > 0))
    {
        DataStreamPtr stream = connection->m_requestInProgress->GetDataStream();

        // Create and set a new 'DataStream' object for this request if it is 'NULL'.
        if (!stream)
        {
            DataStream* newDataStream = new DataStreamImplementation();

            if (!newDataStream)
            {
                // @todo: Should we retry this request when there is no memory?
                connection->NotifyRequestError(NE_NOMEM);
                return;
            }

            // Set this new 'DataStream' object for this request and get it again.
            connection->m_requestInProgress->SetDataStream(DataStreamPtr(newDataStream));
            stream = connection->m_requestInProgress->GetDataStream();
        }

        // Append this data to the 'DataStream' object of request.
        error = stream->AppendData((const uint8*) bytes, count);
        if (error != NE_OK)
        {
            // Clear the data stream in this request.
            connection->m_requestInProgress->SetDataStream(DataStreamPtr());

            connection->NotifyRequestError(error);
            return;
        }

        // Report data available.
        connection->m_requestInProgress->DataAvailable(count);

        return;
    }

    // Download this request successfully.
    {
        uint32 tempLevelId = connection->m_levelIdInProgress;
        DownloadRequestPtr tempRequest = connection->m_requestInProgress;

        if (!tempRequest)
        {
            // Nothing to do if there is no request.
            return;
        }

        // Clear the item which is downloading in progress.
        tempRequest->SetInProgressFlag(false);
        connection->m_requestInProgress = DownloadRequestPtr();
        connection->m_levelIdInProgress = INVALID_DOWNLOAD_LEVEL;
        connection->m_currentRetryTimes = 0;

        // Remove this request from the download level of HTTP download manager.
        connection->m_httpDownloadManager.RemoveRequestFromLevel(tempLevelId, tempRequest);

        // Add another task to process pending requests in HTTP download manager.
        connection->m_httpDownloadManager.ScheduleProcessTasks();

        // Report downloading this request successfully.
        tempRequest->DataAvailable(0);
    }
}

/*! Callback function for HTTP response headers notification

    @return None
*/
void
HttpDownloadManager::DownloadConnection::ResponseHeadersCallback(void* userData,
                                                                 void* requestData,
                                                                 const char* responseHeaders)
{
    // The user data points to a connection.
    DownloadConnection* connection = (DownloadConnection*) userData;

    if ((!connection) ||
        (!(connection->m_httpConnection)) ||
        (!(connection->m_requestInProgress)) ||
        (connection->m_requestInProgress.get() != requestData))
    {
        return;
    }

    connection->m_requestInProgress->SetResponseHeaders(shared_ptr<string>(new string(responseHeaders)));
}

/*! @} */
