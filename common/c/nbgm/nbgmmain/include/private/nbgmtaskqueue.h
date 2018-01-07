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

    @file nbgmtaskqueue.h
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

#ifndef _NBGM_TASK_QUEUE_H_
#define _NBGM_TASK_QUEUE_H_

#include "nbretypes.h"
#include "pallock.h"
#include "nbresharedptr.h"
#include "nbtaskqueue.h"

class NBGM_Task;

/** Listen to the task operation
*/
class NBGM_ITaskListener
{
public:
    virtual void OnTaskAdded(NBGM_Task* task) = 0;
    virtual void OnTaskRemoved(NBGM_Task* task) = 0;
};

/**NBGM Task
*/
class NBGM_Task : public Task
{
public:
    virtual ~NBGM_Task();
public:
    void SetTaskListener(NBGM_ITaskListener* taskListener) { mTaskListener = taskListener; }
    virtual void Execute();
protected:
    NBGM_Task() : mTaskListener(NULL) {}

protected:
    NBGM_ITaskListener* mTaskListener;
};

/** Task queue used for NBGM
*/
class NBGM_TaskQueue : public NBGM_ITaskListener
{
public:
    NBGM_TaskQueue(shared_ptr<TaskQueue> taskQueue, PAL_Instance* pal);
    virtual ~NBGM_TaskQueue();

public:
    PAL_Error AddTask(NBGM_Task* task);
    PAL_Error RemoveTask(NBGM_Task* task, nb_boolean cancelifRunning);
    void RemoveAllTask();

public:
    virtual void OnTaskAdded(NBGM_Task* task);
    virtual void OnTaskRemoved(NBGM_Task* task);

private:
    shared_ptr<TaskQueue> mNativeTaskQueue;
    NBRE_Set<NBGM_Task*> mTaskList;
    PAL_Lock* mLock;
};

#endif //_NBGM_TASK_QUEUE_H_
