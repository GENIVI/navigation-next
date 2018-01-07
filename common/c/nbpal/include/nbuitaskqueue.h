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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*!--------------------------------------------------------------------------
 
 @file nbuitaskqueue.h
 
 C++ Interface for NB PAL task queue.
 
 The task queue provides a mechanism to call user callback in the context of particular
 thread. It can be UI, Event or Worker thread. Adding a task can be called from any thread.
 
 All UI function calls should be done in the context of UI thread. Such tasks should
 be added to UI task queue.
 
 All CCC API calls should be done in the context of CCC/PAL thread. Such tasks should
 be added to Event task queue.
 
 The internal CCC calls which require a lot of calculations or can be blocked should be
 done in the context of worker thread. Such tasks should be added to Worker task queue.
 
 The task queues implementation is highly platform dependent. Some platforms
 (e.g. iPhone) already have implementation of a task queue and should
 therefore use the platform specific implementation, while other platforms
 (e.g. Windows Mobile) need to implement the task queue using
 semaphores/critical sections in order to ensure proper synchronization.
 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NB_UITASK_QUEUE_H
#define	NB_UITASK_QUEUE_H

#include "smartpointer.h"
#include "paluitaskqueue.h"
#include "palerror.h"
#include "pal.h"
#include "commontaskqueue.h"


///////////////////////////////////////////////////
// UI task queue.
///////////////////////////////////////////////////
class UiTaskQueue : public TaskQueue
{
public:
    UiTaskQueue(PAL_Instance* pal) : TaskQueue(pal) {}
    virtual ~UiTaskQueue() {}
    
    virtual PAL_Error AddTask(Task* task);
    
    virtual PAL_Error RemoveTask(Task* task, bool cancelIfRunning);
    
    virtual void RemoveAllTasks(void);
};

#endif // NB_TASK_QUEUE_H

/*! @} */