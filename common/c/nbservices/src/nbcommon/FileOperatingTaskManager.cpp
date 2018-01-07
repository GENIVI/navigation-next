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
   @file         FileOperatingTask.cpp
   @defgroup     nbcommon
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "FileOperatingTaskManager.h"
#include "contextbasedsingleton.h"
#include "workerqueuemanager.h"
#include "commoncomponentconfiguration.h"
#include "datastreamimplementation.h"
#include <algorithm>
extern "C"
{
#include "palfile.h"
}

//#define DFILE
#ifdef DFILE
#include <QDebug>
#include <pthread.h>
#endif

#define CCC_NEW new (std::nothrow)


using namespace nbcommon;

// Retrieve this variable from PAGE_SIZE should boost the performance.
static const uint32 BUFFER_SIZE                   = 4096;

// Expiring time for closing file handles
static const uint32 FILE_HANDLE_EXPIRING_TIME     = 30000; // milliseconds

// Maximum count of opening file handles
static const uint32 MAX_OPENING_FILE_HANDLE_COUNT = 64;

static const uint32 HIGHEST_PRIORITY_TASK = 0;

// Implementation of FileOperatingTaskManager.
shared_ptr <FileOperatingTaskManager>
FileOperatingTaskManager::GetInstance(NB_Context* context)
{
    return ContextBasedSingleton<FileOperatingTaskManager>::getInstance(context);
}


/* See description in header file. */
FileOperatingTaskManager::FileOperatingTaskManager(NB_Context* context)
        : m_pContext(context)
{
    shared_ptr<WorkerQueueManager> threadManager =
            CommonComponentConfiguration::GetWorkerQueueManagerComponent(context);
    if (threadManager)
    {
        /* get worker task queue for nbgm rendering tiles to nbgm in separate threads */
        m_taskQueue = threadManager->RetrieveFileOperationTaskQueue();
    }
}

/* See description in header file. */
FileOperatingTaskManager::~FileOperatingTaskManager()
{
}


/* See description in header file. */
NB_Context* FileOperatingTaskManager::GetContext()
{
    return m_pContext;
}


/* See description in header file. */
NB_Error FileOperatingTaskManager::ScheduleGetFile(StringPtr path,
                                                   FileOperatingCallbackPtr callback)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::ScheduleGetFile(" << path->c_str() << ")";
#endif
    if (!path || path->empty() || !callback)
    {
        return NE_INVAL;
    }

    FileOperatingTask* task = CCC_NEW FileOperatingTask(FOT_Get, path, callback, *this);
    if (!task)
    {
        return NE_NOMEM;
    }
    return AddCustomizedTask(task, true);
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::ScheduleRemoveFile(StringPtr path,
                                                      FileOperatingCallbackPtr callback )
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::ScheduleRemoveFile(" << path->c_str() << ")";
#endif
    if (!path || path->empty() || !callback)
    {
        return NE_INVAL;
    }

    FileOperatingTask* task = CCC_NEW FileOperatingTask(FOT_Delete, path, callback, *this);
    if (!task)
    {
        return NE_NOMEM;
    }
    return AddCustomizedTask(task, true);
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::SchedulePutFile(StringPtr path, DataStreamPtr data,
                                                   FileOperatingCallbackPtr callback)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::SchedulePutFile(" << path->c_str() << ")";
#endif
    if (!path || path->empty() || !data)
    {
        return NE_INVAL;
    }

    FileOperatingTask* task = CCC_NEW FileOperatingTask(FOT_Put, path, data, callback, *this);
    if (!task)
    {
        return NE_NOMEM;
    }
    return AddCustomizedTask(task);
}

/* See description in header file. */
FileOperatingTaskManager::FileOperatingTask::FileOperatingTask(FileOperationType        type,
                                                               StringPtr                path,
                                                               FileOperatingCallbackPtr callback,
                                                               FileOperatingTaskManager& manager)
        : m_type(type),
          m_path(path),
          m_callback(callback),
          m_manager(manager)
{
}

FileOperatingTaskManager::FileOperatingTask::FileOperatingTask(FileOperationType         type,
                                                               StringPtr                 path,
                                                               DataStreamPtr             /*data*/,
                                                               FileOperatingCallbackPtr  callback,
                                                               FileOperatingTaskManager& manager)
        : m_type(type),
          m_path(path),
          m_callback(callback),
          m_manager(manager)

{
}

/* See description in header file. */
FileOperatingTaskManager::FileOperatingTask::~FileOperatingTask()
{
}

//FileOperatingTask& FileOperatingTask::FileOperatingTask::opertor= (const FileOperatinTask& param)
//{
//	return NULL;
//}

/* See description in header file. */
void FileOperatingTaskManager::FileOperatingTask::Execute()
{
    NB_Error error = NE_INVAL;

    switch (m_type)
    {
        case FOT_Get:
        {
            error = m_manager.DoGetFile(m_path, m_data);
            break;
        }
        case FOT_Put:
        {
            error = m_manager.DoPutFile(m_path, m_data);
            break;
        }
        case FOT_Delete:
        {
            error = m_manager.DoDeleteFile(m_path);
            break;
        }
        case FOT_CheckHandler:
        {
            error = m_manager.DoCheckFileHandlers();
            break;
        }
        default:
        {
            break;
        }
    }

    if (m_callback)
    {
        if (error != NE_OK)
        {
            m_callback->Error(error);
        }
        else
        {
            m_callback->Success(m_data);
        }
    }
    delete this;
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::DoGetFile(const StringPtr& path, DataStreamPtr& data)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::DoGetFile(" << path->c_str() << ")";
#endif
    if (!path || path->empty() || !data)
    {
        return NE_INVAL;
    }

    PAL_File* file = GetFileHandle(path);
    if (!file)
    {
        return NE_FSYS;
    }

    PAL_Instance* pal      = NB_ContextGetPal(m_pContext);
    uint32        fileSize = 0;
    PAL_Error     palError = PAL_FileGetSize(pal, path->c_str(), &fileSize);
    palError               = (palError == PAL_Ok) ? \
                             PAL_FileSetPosition(file, PFSO_Start, 0) : palError;

    if (palError == PAL_Ok && fileSize)
    {
        uint8 buffer[BUFFER_SIZE] = {0};
        uint32   bytesRead        = 0;
        NB_Error nbError          = NE_OK;
        while (palError == PAL_Ok && fileSize > 0)
        {
            palError = PAL_FileRead(file, buffer, BUFFER_SIZE, &bytesRead);
            if (palError == PAL_Ok)
            {
                fileSize -= bytesRead;
                nbError = data->AppendData(buffer, bytesRead);
                if (nbError != NE_OK)
                {
                    return nbError;
                }
            }
        }
    }

    if (palError == PAL_Ok)
    {
        return NE_OK;
    }

    return NE_FSYS;
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::DoPutFile(const StringPtr& path,
                                             const DataStreamPtr& data)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::DoPutFile(" << path->c_str() << ")";
#endif
    // TODO: Ensure file system is clean if failed to write.
    if (!path || path->empty() || !data)
    {
        return NE_INVAL;
    }

    PAL_File* file = GetFileHandle(path, true);
    if (!file)
    {
        return NE_FSYS;
    }
    // Let DataStreamImpl to write file to avoid memcpy.
    return data->DumpToPALFile(file);
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::DoDeleteFile(const StringPtr& path)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::DoDeleteFile(" << path->c_str() << ")";
#endif
    if (!path || path->empty())
    {
        return NE_INVAL;
    }

    RemoveFileHandle(path);
    RemovePath(path->c_str());
    return  NE_OK;
}

/* See description in header file. */
PAL_File* FileOperatingTaskManager::GetFileHandle(const StringPtr& path, bool autoCreate)
{
    if (!path || path->empty())
    {
        return NULL;
    }

    // 1. Find in expiring files.
    HANDLE_MAP::iterator iter = m_expiringFiles.find(*path);
    if (iter != m_expiringFiles.end())
    {
        pair<HANDLE_MAP::iterator, bool> result = m_validFiles.insert(*iter);
        if (result.first != m_validFiles.end())
        {
            m_expiringFiles.erase(iter);
            return result.first->second;
        }
    }

    // 2. Try to find in m_validFiles.
    iter = m_validFiles.find(*path);
    if (iter != m_validFiles.end())
    {
        return iter->second;
    }

    // 3. Open new handle otherwise.
    PAL_Instance* pal = NB_ContextGetPal(m_pContext);
    if (!pal)
    {
        return NULL;
    }

    if (PAL_FileIsDirectory(pal, path->c_str()))
    {
        return NULL;
    }

    // @todo: close part of file handles !
    if ((m_validFiles.size() + m_expiringFiles.size()) > MAX_OPENING_FILE_HANDLE_COUNT)
    {
        CloseAllFileHandles();
    }

    PAL_FileMode fileMode = PFM_ReadWrite;
    PAL_Error    palError = PAL_FileExists(pal, path->c_str());
    if (palError != PAL_Ok)
    {
        if (!autoCreate) // If file not exist, and Create flag is not set, return NULL.
        {
            return NULL;
        }

        fileMode = PFM_Create;
    }

    // Ensure directory name of path exists.
    uint32 size = path->size() + 1;
    char* buffer = (char*)nsl_malloc(size);
    memset(buffer, 0, size);
    (void)nsl_strncpy(buffer, path->c_str(), size);
    palError = PAL_FileRemovePath(pal, buffer);
    palError = palError == PAL_Ok ? PAL_FileCreateDirectoryEx(pal, buffer) : palError;
    if (palError != PAL_Ok)
    {
        nsl_free(buffer);
        return NULL;
    }
    nsl_free(buffer);

    // Now open file handler.
    PAL_File* palFile = NULL;
    palError = PAL_FileOpen(pal, path->c_str(), fileMode, &palFile);
    if (palError != PAL_Ok)
    {
        // @todo: Should check if errno is ENFILE before closing file handlers, but this is
        //        not supported in pal.
        // Close all opening file handles and try to open this file again.
        CloseAllFileHandles();

        palError = PAL_FileOpen(pal, path->c_str(), fileMode, &palFile);
        if (palError != PAL_Ok)
        {
            return NULL;
        }
    }

    if (!palFile)
    {
        return NULL;
    }

    // Add this file handle to 'm_validFiles'.
    pair<HANDLE_MAP::iterator, bool> result = m_validFiles.insert(make_pair(*path, palFile));
    if (result.second)
    {
        // Try to start the timer for file handle expiring and ignore the returned error.
        // @todo: Switch to PAL_TimerSetWithHandle!
        PAL_TimerSet(pal, FILE_HANDLE_EXPIRING_TIME, FileHandleExpiringTimerCallback, this);

        return palFile;
    }

    // Insertion fails.
    PAL_FileClose(palFile);
    palFile = NULL;
    if (fileMode == PFM_Create)
    {
        RemovePath(path->c_str());
    }
    return NULL;
}

/* See description in header file. */
void FileOperatingTaskManager::RemoveFileHandle(const StringPtr& path)
{
    if (path)
    {
        // Check from m_expiringFiles first, if not found, try to find in m_validFiles.
        HANDLE_MAP::iterator iter = m_expiringFiles.find(*path);
        if (iter != m_expiringFiles.end())
        {
            CloseFileHandle(*iter);
            m_expiringFiles.erase(iter);
            return;
        }

        iter = m_validFiles.find(*path);
        if (iter != m_validFiles.end())
        {
            CloseFileHandle(*iter);
            m_validFiles.erase(iter);
        }
    }
}


/* See description in header file. */
void FileOperatingTaskManager::CloseFileHandle(pair <string, PAL_File* > element)
{
    if (element.second)
    {
        // This element must be erased after calling this function.
        PAL_FileClose(element.second);
    }
}

/* See description in header file. */
void FileOperatingTaskManager::FileHandleExpiringTimerCallback(PAL_Instance* /*pal*/,
                                                               void* userData,
                                                               PAL_TimerCBReason reason)
{
    if (reason != PTCBR_TimerFired)
    {
        return;
    }

    FileOperatingTaskManager* manager = static_cast<FileOperatingTaskManager*>(userData);
    if (!manager)
    {
        return;
    }

    //@todo: If this callback can be called in FileOperationThread directly, we can avoid
    //       adding extra task here.
    FileOperatingTask* task = CCC_NEW FileOperatingTask(FOT_CheckHandler,
                                                        StringPtr(),
                                                        FileOperatingCallbackPtr(),
                                                        *manager);
    if (task)
    {
        (void)manager->AddCustomizedTask(task);
    }
}

/* See description in header file. */
void FileOperatingTaskManager::CloseAllFileHandles()
{
    // Close all opening file handles.
    for_each(m_expiringFiles.begin(), m_expiringFiles.end(), CloseFileHandle);
    m_expiringFiles.clear();

    for_each(m_validFiles.begin(), m_validFiles.end(), CloseFileHandle);
    m_validFiles.clear();

    // Cacel the timer for file handle expiring.
    if (m_pContext)
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (pal)
        {
            PAL_TimerCancel(pal, FileHandleExpiringTimerCallback, this);
        }
    }
}

/* See description in header file. */
void FileOperatingTaskManager::RemovePath(const char* path)
{
    PAL_Instance* pal = NULL;
    if ((!path) || (nsl_strlen(path) == 0) || (!m_pContext) || !(pal=NB_ContextGetPal(m_pContext)))
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

/* See description in header file. */
NB_Error FileOperatingTaskManager::AddCustomizedTask(Task* task, bool highPriority)
{
    if (highPriority)
    {
        return m_taskQueue->AddTask(task, HIGHEST_PRIORITY_TASK) == PAL_Ok ? \
                NE_OK : NE_UNEXPECTED;
    }
    return m_taskQueue->AddTask(task) == PAL_Ok ? NE_OK : NE_UNEXPECTED;
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::GetFile(shared_ptr <FileOperatingTaskManager> manager,
                                           const StringPtr& path,
                                           DataStreamPtr& data)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::GetFile(" << path->c_str() << ")";
#endif
    nsl_assert(manager.get() != NULL && manager->m_taskQueue->IsRuningThread());
    return manager ? manager->DoGetFile(path, data) : NE_INVAL;
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::PutFile(shared_ptr <FileOperatingTaskManager> manager,
                                           const StringPtr& path, const DataStreamPtr& data)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::PutFile(" << path->c_str() << ")";
#endif
    nsl_assert(manager.get() != NULL && manager->m_taskQueue->IsRuningThread());
    return manager ? manager->DoPutFile(path, data) : NE_INVAL;
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::DeleteFile(shared_ptr <FileOperatingTaskManager> manager,
                                              const StringPtr& path)
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::DeleteFile(" << path->c_str() << ")";
#endif
    nsl_assert(manager.get() != NULL && manager->m_taskQueue->IsRuningThread());
    return manager ? manager->DoDeleteFile(path) : NE_INVAL;
}

/* See description in header file. */
NB_Error FileOperatingTaskManager::DoCheckFileHandlers()
{
#ifdef DFILE
    qWarning() << "DFILE: " << pthread_self() << " FileOperatingTaskManager::DoCheckFileHandlers()";
#endif
    if (!m_expiringFiles.empty() || !m_validFiles.empty())
    {
        // Close all expired file handles.
        for_each(m_expiringFiles.begin(), m_expiringFiles.end(), CloseFileHandle);
        m_expiringFiles.clear();

        // Move all file handles from 'm_validFiles' to 'm_expiringFiles'.
        m_validFiles.swap(m_expiringFiles);

        if (m_pContext && m_expiringFiles.size() > 0)
        {
            PAL_Instance* pal = NB_ContextGetPal(m_pContext);
            if (pal)
            {
                // Try to start the timer for file handle expiring and ignore the returned error.
                PAL_TimerSet(pal, FILE_HANDLE_EXPIRING_TIME, FileHandleExpiringTimerCallback, this);
            }
        }
    }
    return NE_OK;
}


/*! @} */
