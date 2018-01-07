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

@file     finishfunctionmanager.cpp
*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "finishfunctionmanager.h"
#include "palstdlib.h"

namespace
{
    /*! Helper class to manage finish function lock
    */
    class LockHelper
    {
    public:
        /*! Acquire the lock
        */
        LockHelper(PAL_Lock *lock) :
            m_lock(lock)
        {
            PAL_LockLock(m_lock);
        }

        /*! Releases the lock
        */
        ~LockHelper()
        {
            PAL_LockUnlock(m_lock);
        }

    private:
        PAL_Lock *m_lock;
    };
};

static void TaskQueueCallback(PAL_Instance* pal, void* userData);

FinishFunctionManager::FinishFunctionManager(PAL_Instance* pal) :
    m_pal(pal),
    m_ownerId(0)
{
    PAL_LockCreate(pal, &m_lock);
}

FinishFunctionManager::~FinishFunctionManager()
{
    DeleteAllListEntries();
    PAL_LockDestroy(m_lock);
}

uint32 FinishFunctionManager::RegisterOwner()
{
    LockHelper lock(m_lock);
    return ++m_ownerId;
}

PAL_Error FinishFunctionManager::Schedule(uint32 owner, FinishFunctionBase* function)
{
    ListEntry* entry = new ListEntry;
    if (!entry)
    {
        return PAL_ErrNoMem;
    }

    entry->ownerId = owner;
    entry->function = function;
    entry->manager = this;

    {
        LockHelper lock(m_lock);
//        try
//        {
            m_list.push_back(entry);
//        }
//        catch (...)
//        {
//            delete entry;
//            return PAL_ErrNoMem;
//        }
    }

    TaskId tid = 0;
    PAL_Error err = PAL_EventTaskQueueAdd(m_pal, TaskQueueCallback, entry, &tid);

    LockHelper lock(m_lock);
    if (err == PAL_Ok)
    {
        // We should check - is entry still in the m_list. It might be already deleted from other thread.
        std::list<ListEntry*>::iterator i = m_list.begin();
        while (i != m_list.end())
        {
            if (*(i) == entry)
            {
                entry->taskId = tid;
                break;
            }
            else
            {
                ++i;
            }
        }
    }
    else
    {
        m_list.remove(entry);
        delete entry;
    }

    return err;
}

PAL_Error FinishFunctionManager::DeleteScheduledFunctions(uint32 ownerId)
{
    LockHelper lock(m_lock);

    std::list<ListEntry*>::iterator i = m_list.begin();
    while (i != m_list.end())
    {
        if ((*i)->ownerId == ownerId)
        {
            PAL_EventTaskQueueRemove(m_pal, (*i)->taskId, TRUE);
            delete (*i)->function;
            delete (*i);
            i = m_list.erase(i);
        }
        else
        {
            ++i;
        }
    }

    return PAL_Ok;
}

void FinishFunctionManager::DeleteAllListEntries()
{
    LockHelper lock(m_lock);

    std::list<ListEntry*>::iterator i = m_list.begin();
    while (i != m_list.end())
    {
        PAL_EventTaskQueueRemove(m_pal, (*i)->taskId, TRUE);
        delete(*i)->function;
        delete(*i);
        i = m_list.erase(i);
    }
}

void FinishFunctionManager::ProcessCallback(ListEntry* entry)
{
    bool found = false;

    {
        // remove entry from the list
        LockHelper lock(m_lock);
        std::list<ListEntry*>::iterator i = m_list.begin();
        while (i != m_list.end())
        {
            if (*(i) == entry)
            {
                m_list.erase(i);
                found = true;
                break;
            }
            else
            {
                ++i;
            }
        }
    }

    if (found && entry->function != NULL)
    {
        entry->function->Invoke();
    }
    delete entry->function;
    delete entry;
}

void TaskQueueCallback(PAL_Instance* /*pal*/, void* userData)
{
    FinishFunctionManager::ListEntry* entry = static_cast<FinishFunctionManager::ListEntry*>(userData);
    if (entry != NULL && entry->manager != NULL)
    {
        entry->manager->ProcessCallback(entry);
    }
}
