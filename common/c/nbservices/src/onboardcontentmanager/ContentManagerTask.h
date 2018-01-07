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
@file         ContentManagerTask.h
@defgroup     onboardcontentmanager
*/
/*
(C) Copyright 2014 by TeleCommunications Systems, Inc.

The information contained herein is confidential, proprietary to
TeleCommunication Systems, Inc., and considered a trade secret as defined
in section 499C of the penal code of the State of California. Use of this
information by anyone other than authorized employees of TeleCommunication
Systems is granted only under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#ifndef __NBSERVICES__CONTENTMANAGERTASK_H__
#define __NBSERVICES__CONTENTMANAGERTASK_H__

#include "smartpointer.h"
extern "C"
{
#include "pal.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"
}

namespace nbcommon
{
/*! Template ContentManagerTask class used for OnboardContentManagerImpl internal.
 *  The class is used to add task into event or ui queue, and call class member
 *    function in task queue call back.
 */
template <class T, typename Y>
class ContentManagerTask
{
public:
    typedef void (T::*TaskFunction)(Y* data);
    /*! Constructor.
     *
     *  @param pal      Pal instance.
     *  @param impl     class of function want to used.
     *  @param data     userdata of function.
     *  @param func     function want to be invoked.
     *  @param isValid  the instance is valid or not.
     *  @param isAddedToUIThread   add the task to UIThread.
     */
    ContentManagerTask(PAL_Instance* pal,
                       T* impl,
                       Y* data,
                       TaskFunction func,
                       shared_ptr<bool> isValid,
                       bool isAddedToUIThread = false)
        :m_pal(pal),
         m_impl(impl),
         m_data(data),
         m_func(func),
         m_isValid(isValid),
         m_isAddToUIThread(isAddedToUIThread)
    {}
    void Execute()
    {
        if (!m_isAddToUIThread)
        {
            if(PAL_IsPalThread(m_pal) == false)
            {
                uint32 tid = 0;
                PAL_EventTaskQueueAdd(m_pal, Static_Scheduled, this, &tid);
                return;
            }
            if (m_isValid &&(*m_isValid))
            {
                (m_impl->*m_func)(m_data);
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
    }
private:
    static void Static_Scheduled(PAL_Instance* pal, void* userData)
    {
        if (userData)
        {
            ContentManagerTask* task = static_cast<ContentManagerTask*>(userData);
            task->Scheduled();
            delete task;
        }
    }

    PAL_Instance*  m_pal;
    Y*             m_data;
    T*             m_impl;
    TaskFunction   m_func;
    bool           m_isAddToUIThread;
    shared_ptr<bool> m_isValid;
};
}
#endif
/*! @} */