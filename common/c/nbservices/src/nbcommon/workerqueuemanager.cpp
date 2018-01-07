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

    @file     workerqueuemanager.cpp
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "workerqueuemanager.h"
#include "nbcontextprotected.h"

WorkerQueueManager::WorkerQueueManager(NB_Context* context)
 : m_context(context)
{

}

WorkerQueueManager::~WorkerQueueManager()
{
}

shared_ptr<WorkerTaskQueue>
WorkerQueueManager::RetrieveNavWorkerTaskQueue()
{
    if(!m_navThread)
    {
        m_navThread = shared_ptr<WorkerTaskQueue>(new WorkerTaskQueue(NB_ContextGetPal(m_context), CHAR_PTR_TO_UTF_STRING_PTR("navigation")));
    }

    return m_navThread;
}

shared_ptr<WorkerTaskQueue>
WorkerQueueManager::RetrieveCommonWorkerTaskQueue()
{
    if(!m_commonThread)
    {
        m_commonThread = shared_ptr<WorkerTaskQueue>(new WorkerTaskQueue(NB_ContextGetPal(m_context), CHAR_PTR_TO_UTF_STRING_PTR("common")));
    }

    return m_commonThread;
}

shared_ptr<WorkerTaskQueue>
WorkerQueueManager::RetrieveLoadTilesWorkerTaskQueue()
{
    if(!m_loadTilesThread)
    {
        m_loadTilesThread = shared_ptr<WorkerTaskQueue>(new WorkerTaskQueue(NB_ContextGetPal(m_context), CHAR_PTR_TO_UTF_STRING_PTR("loadtiles")));
    }

    return m_loadTilesThread;
}

shared_ptr<WorkerTaskQueue>
WorkerQueueManager::RetrieveNBGMRenderingTaskQueue()
{
    if(!m_nbgmRenderingThread)
    {
        m_nbgmRenderingThread = shared_ptr<WorkerTaskQueue>(new WorkerTaskQueue(NB_ContextGetPal(m_context), CHAR_PTR_TO_UTF_STRING_PTR("nbgmrendering")));
    }

    return m_nbgmRenderingThread;
}

NB_Context*
WorkerQueueManager::GetContext()
{
    return m_context;
}

/* See description in header file. */
shared_ptr <WorkerTaskQueue> WorkerQueueManager::RetrieveFileOperationTaskQueue()
{
    if (!m_fileOperationThread)
    {
        m_fileOperationThread = shared_ptr<WorkerTaskQueue>(
            new WorkerTaskQueue(NB_ContextGetPal(m_context),
                                CHAR_PTR_TO_UTF_STRING_PTR("FileOperating")));
    }
    return m_fileOperationThread;
}

/* See description in header file. */
vector <shared_ptr <WorkerTaskQueue> > WorkerQueueManager::RetrieveAllTaskQueues() const
{
    vector<shared_ptr<WorkerTaskQueue> > queues;
    queues.push_back(m_navThread);
    queues.push_back(m_commonThread);
    queues.push_back(m_loadTilesThread);
    queues.push_back(m_nbgmRenderingThread);
    queues.push_back(m_fileOperationThread);
    return queues;
}
