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
    @file     NavTask.h
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */
#ifndef __NAVTASK_H__
#define __NAVTASK_H__

extern "C"
{
#include "pal.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"
}
#include "smartpointer.h"

/*! Template NavTask class use for NavKit internal.

   The class is used for adding task into event task queue, and call class member function in event
   task queue call back.
*/
template <class T, typename Y>
class NavTask
{
public:
    typedef void (T::*TaskFunction)(Y* data);
    /*! Construction.

        @param pal Pal instance.
        @param impl class of function want be used.
        @param data the user data of function, which is allocated by 'new' and would be deleted after function executed.
        @param func function want to be invoked.
        @param ifForceAdd true is to add into event queue forced no matter invoker thread.
        @param isAddedToUIThread is special this task will be excuted in UI Thread.
        @param ifOwnerData the data will be deleted if it is true.
    */
    NavTask(PAL_Instance* pal, T* impl, Y* data, TaskFunction func, shared_ptr<bool> isValid,
            bool ifForceAdd = false, bool isAddedToUIThread = false, bool ifOwnerData = true)
            : m_pal(pal), m_data(data), m_impl(impl), m_func(func), m_ifForceAdd(ifForceAdd),
              m_isAddToUIThread(isAddedToUIThread), m_ifOwnerData(ifOwnerData)
    {
        m_isValid = isValid;
    }

    void Execute()
    {
        if (!m_isAddToUIThread)
        {
            if (PAL_IsPalThread(m_pal) == FALSE || m_ifForceAdd)
            {
                uint32 tid = 0;
                // schedule task to Event thread
                PAL_EventTaskQueueAdd(m_pal, Static_Scheduled, this, &tid);
                return;
            }
            if (m_isValid && (*m_isValid))
            {
                (m_impl->*m_func)(m_data);
            }
            if (m_data && m_ifOwnerData)
            {
                delete m_data;
            }
            delete this;
        }
        else
        {
            PAL_UiTaskQueueAdd(m_pal, Static_Scheduled, this);
        }
    }

    void Scheduled()
    {
        if (m_isValid && (*m_isValid))
        {
            (m_impl->*m_func)(m_data);
        }
        if (m_data && m_ifOwnerData)
        {
            delete m_data;
        }
    }

private:
    static void Static_Scheduled(PAL_Instance* pal, void* userData)
    {
        if (userData)
        {
            NavTask* task = static_cast<NavTask*>(userData);
            task->Scheduled();
            delete task;
        }
    }

    PAL_Instance*    m_pal;
    Y*               m_data;
    T*               m_impl;
    TaskFunction     m_func;
    bool             m_ifForceAdd;
    bool             m_isAddToUIThread;
    shared_ptr<bool> m_isValid;
    bool             m_ifOwnerData;
};

#endif /* __NAVTASK_H__ */

/*! @} */