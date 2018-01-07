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
   @file         FileOperatingTask.h
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

#ifndef _FILEOPERATINGTASKMANAGER_H_
#define _FILEOPERATINGTASKMANAGER_H_

#include "AsyncCallback.h"
#include "datastream.h"
#include "nbcontext.h"
#include "nbtaskqueue.h"
#include <map>

extern "C"
{
#include "paltimer.h"
}

struct PAL_File;

namespace nbcommon
{
typedef shared_ptr<string> StringPtr;
typedef shared_ptr<nbmap::AsyncCallback<DataStreamPtr> >   FileOperatingCallbackPtr;

typedef enum _FileOperationType
{
    FOT_Invalid = 0,
    FOT_Get,
    FOT_Put,
    FOT_Delete,
    FOT_CheckHandler,
} FileOperationType;


/*! Task manager for file operaions.*/
class FileOperatingTaskManager
{
public:
    /*! Get a conext-based singleton of FileOperatingManager.

        @return shared pointer of manager.
    */
    static shared_ptr<FileOperatingTaskManager> GetInstance(NB_Context* context);

    FileOperatingTaskManager(NB_Context* context);
    virtual ~FileOperatingTaskManager();
    NB_Context* GetContext();

    /*! Schedule a task to get content from a file.

       @return NE_OK if task added successfully.
    */
    NB_Error ScheduleGetFile(StringPtr path,   /*!<  path to read.  */
                             FileOperatingCallbackPtr callback /*!< callback through which
                                                                    data will be retruned. */
                             );

    /*! Schedule a task to remove a file specified by `path`

        @return NE_OK if task added successfully.
    */
    NB_Error ScheduleRemoveFile(StringPtr path,
                                FileOperatingCallbackPtr callback=FileOperatingCallbackPtr()
                                );

    /*! Schedule a task to write a file.

        @return NE_OK if task added successfully.
    */
    NB_Error SchedulePutFile(StringPtr path,     /*!< path where file will be stored. */
                             DataStreamPtr data, /*!< data to written. */
                             FileOperatingCallbackPtr callback=FileOperatingCallbackPtr()
                             );

    /*! Add customized task to be executed in FileOPerationThread.

        @return NE_OK if added successfully.
    */
    NB_Error AddCustomizedTask(Task* task, bool highPriority=false);

    // Utility functions which can be called directly to get/put/delete files.
    static NB_Error GetFile(shared_ptr<FileOperatingTaskManager> manager,
                            const StringPtr& path, DataStreamPtr& data);
    static NB_Error PutFile(shared_ptr<FileOperatingTaskManager> manager,
                            const StringPtr& path, const DataStreamPtr& data);
    static NB_Error DeleteFile(shared_ptr<FileOperatingTaskManager> manager,
                               const StringPtr& path);

private:
    // Refer to source file for description.
    NB_Error DoGetFile(const StringPtr& path, DataStreamPtr& data);
    NB_Error DoPutFile(const StringPtr& path, const DataStreamPtr& data);
    NB_Error DoDeleteFile(const StringPtr& path);
    NB_Error DoCheckFileHandlers();

    PAL_File* GetFileHandle(const StringPtr& path, bool autoCreate=false);
    void RemoveFileHandle(const StringPtr& path);
    void CloseAllFileHandles();
    void RemovePath(const char* path);

    static void CloseFileHandle(pair<string, PAL_File*> element);
    static void FileHandleExpiringTimerCallback(PAL_Instance *pal,
                                                void *userData,
                                                PAL_TimerCBReason reason);


    NB_Context*                  m_pContext;  /*!< Pointer of NBContext. */
    shared_ptr <WorkerTaskQueue> m_taskQueue; /*!< Task queue where file tasks will be add to.*/

    typedef map<string, PAL_File*>   HANDLE_MAP;
    HANDLE_MAP  m_validFiles;           /*!< Cached file handlers */
    HANDLE_MAP m_expiringFiles;         /*!< Cached file handlers which are going to expired. */

    class FileOperatingTask : public Task
    {
    public:
        FileOperatingTask(FileOperationType type,
                          StringPtr         path,
                          FileOperatingCallbackPtr callback,
                          FileOperatingTaskManager&  manager);
        FileOperatingTask(FileOperationType type,
                          StringPtr         path,
                          DataStreamPtr     data,
                          FileOperatingCallbackPtr callback,
                          FileOperatingTaskManager&  manager);

        virtual ~FileOperatingTask();
        virtual void Execute();

    private:

        FileOperationType  m_type;
        shared_ptr<string> m_path;
        DataStreamPtr      m_data;
        shared_ptr<nbmap::AsyncCallback<DataStreamPtr> > m_callback;
        FileOperatingTaskManager& m_manager;
		FileOperatingTask& operator= (const FileOperatingTask& param);
    };

};

typedef shared_ptr<FileOperatingTaskManager> FileOperatingTaskManagerPtr;

}

#endif /* _FILEOPERATINGTASKMANAGER_H_ */
/*! @} */
