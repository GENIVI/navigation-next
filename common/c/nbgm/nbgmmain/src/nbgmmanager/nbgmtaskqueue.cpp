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

    @file nbgmtaskqueue.cpp
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbgmtaskqueue.h"
#include "nbrecommon.h"

NBGM_Task::~NBGM_Task()
{
}

void
NBGM_Task::Execute()
{
    if(mTaskListener)
    {
        mTaskListener->OnTaskRemoved(this);
    }
    NBRE_DELETE this;
}

NBGM_TaskQueue::NBGM_TaskQueue(shared_ptr<TaskQueue> taskQueue, PAL_Instance* pal) : mNativeTaskQueue(taskQueue)
{
    PAL_LockCreate(pal, &mLock);
}

NBGM_TaskQueue::~NBGM_TaskQueue()
{
    PAL_LockDestroy(mLock);
}

PAL_Error 
NBGM_TaskQueue::AddTask(NBGM_Task* task)
{
    OnTaskAdded(task);
    task->SetTaskListener(this);
    PAL_Error err = mNativeTaskQueue->AddTask(task);
    if (err != PAL_Ok && task != NULL)
    {
        OnTaskRemoved(task);
        NBRE_DELETE task;
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_TaskQueue::AddTask, error = 0x%08x", err);
    }
    return err;
}

PAL_Error
NBGM_TaskQueue::RemoveTask(NBGM_Task* task, nb_boolean cancelifRunning)
{
    PAL_Error err = mNativeTaskQueue->RemoveTask(task, cancelifRunning ? true : false);
    if (err == PAL_Ok && task != NULL)
    {
        OnTaskRemoved(task);
        NBRE_DELETE task;
    }
    return err;
}

void
NBGM_TaskQueue::RemoveAllTask()
{
    PAL_Error err = PAL_Ok;
    PAL_LockLock(mLock);
    for(NBRE_Set<NBGM_Task*>::iterator iter = mTaskList.begin(); iter != mTaskList.end(); ++iter)
    {
        NBGM_Task* task = *iter;
        err = mNativeTaskQueue->RemoveTask(task, TRUE);
        if (err == PAL_Ok && task != NULL)
        {
            NBRE_DELETE task;
        }
    }
    mTaskList.clear();
    PAL_LockUnlock(mLock);
}

void
NBGM_TaskQueue::OnTaskAdded(NBGM_Task* task)
{
    PAL_LockLock(mLock);
    mTaskList.insert(task);
    PAL_LockUnlock(mLock);
}

void
NBGM_TaskQueue::OnTaskRemoved(NBGM_Task* task)
{
    PAL_LockLock(mLock);
    NBRE_Set<NBGM_Task*>::iterator iter = mTaskList.find(task);
    if(iter != mTaskList.end())
    {
        mTaskList.erase(iter);
    }
    PAL_LockUnlock(mLock);
}
