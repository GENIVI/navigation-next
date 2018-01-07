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

@file     callbackmanager.cpp
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

#include "callbackmanager.h"
#include "palstdlib.h"
#include <algorithm> 

namespace nimpal
{
    namespace network
    {
        CallbackManager::CallbackManager(PAL_Instance* pal) :
            m_pal(pal),
            m_callbackQueue(),
            m_isInCallback(false),
            m_bStopCallback(false)
        {
			int r = pthread_mutex_init(&m_callbackQueueMutex, NULL);
			assert(r == 0);
        }

        CallbackManager::~CallbackManager()
        {
            StopAllCallbacks();

			int r = pthread_mutex_destroy(&m_callbackQueueMutex);
			assert(r == 0);
        }

        PAL_Error CallbackManager::Schedule(Callback* function)
        {
            if (m_bStopCallback)
            {
                // can not schedule this callback
                return PAL_Failed;
            }

            CallbackEntry* entry = (CallbackEntry*)nsl_malloc(sizeof(*entry));
            if (!entry)
            {
                return PAL_ErrNoMem;
            }

			pthread_mutex_lock(&m_callbackQueueMutex);
			m_callbackQueue.push_back(entry);

            entry->callback = function;
            entry->tid = 0;

            TaskId tid = 0;
            PAL_EventTaskQueueAdd(m_pal, EventTaskCallback, this, &tid);
            std::deque<CallbackEntry*>::iterator iter = find(m_callbackQueue.begin(),m_callbackQueue.end(),entry);
            if (iter != m_callbackQueue.end())
            {
                // We need to check the "entry" first because it might be already deleted in the EventTaskCallback()
                entry->tid = tid;
            }

			pthread_mutex_unlock(&m_callbackQueueMutex);

            return PAL_Ok;
        }

        CallbackManager::CallbackEntry* CallbackManager::GetCallbackEntry()
        {
            CallbackEntry* entry = NULL;

			pthread_mutex_lock(&m_callbackQueueMutex);
			if (!m_callbackQueue.empty()) {
				entry = m_callbackQueue.at(0);
				m_callbackQueue.pop_front();
			}
			pthread_mutex_unlock(&m_callbackQueueMutex);

            return entry;
        }

        void CallbackManager::EventTaskCallback(PAL_Instance* pal, void* user_data)
        {
            if (user_data)
            {
                CallbackManager* mgr = (CallbackManager*)user_data;
                CallbackEntry* entry = mgr->GetCallbackEntry();

                if (entry)
                {
                    if (entry->callback)
                    {
                        if (!mgr->m_bStopCallback)
                        {
                            mgr->m_isInCallback = true;
                            entry->callback->Invoke();
                            mgr->m_isInCallback = false;
                        }

                        delete entry->callback;
                    }

                    nsl_free(entry);
                }
            }
        }

        void CallbackManager::StopAllCallbacks()
        {
            CallbackEntry* entry = NULL;

            m_bStopCallback = true;

            if (m_isInCallback)
            {
                // current in callback, callback list is being used
                return;
            }

            while ((entry = GetCallbackEntry()) != NULL)
            {
                PAL_EventTaskQueueRemove(m_pal, entry->tid, FALSE);
                if (entry->callback)
                {
                    delete entry->callback;
					entry->callback = NULL;
                }

                nsl_free(entry);
            }
        }
    }
}

