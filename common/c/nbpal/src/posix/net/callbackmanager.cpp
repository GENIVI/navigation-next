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
    @date     03/02/2012
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for storing a PAL HTTP network request.

    Qt QNX implementation for storing a PAL HTTP network request.
*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

//#include <atomic.h>
#include <unistd.h>
#include "callbackmanager.h"
#include "palstdlib.h"
#include "palatomic.h"

namespace nimpal
{
    namespace network
    {
        CallbackManager::CallbackManager(PAL_Instance* pal) :
            m_pal(pal),
            m_callbackQueue(),
            m_isInCallback(0),
            m_bStopCallback(0)
        {
            int r = pthread_mutex_init(&m_callbackQueueMutex, NULL);
            assert(r == 0);

            pthread_mutexattr_t attr;
            int err = pthread_mutexattr_init(&attr);
            assert(err == 0);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            err = pthread_mutex_init( &m_callbackStopMutex, &attr);
            pthread_mutexattr_destroy(&attr);
            assert(err == 0);
        }


        CallbackManager::~CallbackManager()
        {
            StopAllCallbacks();

            int r = pthread_mutex_destroy(&m_callbackQueueMutex);
            assert(r == 0);
            r = pthread_mutex_destroy(&m_callbackStopMutex);
            assert(r == 0);
        }

        PAL_Error CallbackManager::Schedule(Callback* function)
        {
            size_t size = 0;
            CallbackEntry* entry = NULL;

            if ( m_bStopCallback )
            {
                // can not schedule this callback
                return PAL_Failed;
            }

            entry = (CallbackEntry*)nsl_malloc(sizeof( CallbackEntry ));
            if ( !entry )
            {
                return PAL_ErrNoMem;
            }

            pthread_mutex_lock( &m_callbackQueueMutex );
            m_callbackQueue.push(entry);
            entry->callback = function;
            entry->tid = 0;

            PAL_EventTaskQueueAdd(m_pal, EventTaskCallback, this, &(entry->tid));

            pthread_mutex_unlock(&m_callbackQueueMutex);

            return PAL_Ok;
        }

        CallbackManager::CallbackEntry* CallbackManager::GetCallbackEntry()
        {
            CallbackEntry* entry = NULL;

            pthread_mutex_lock(&m_callbackQueueMutex);
            if (!m_callbackQueue.empty())
            {
                entry = m_callbackQueue.at(0);
                m_callbackQueue.pop();
            }
            pthread_mutex_unlock(&m_callbackQueueMutex);

            return entry;
        }

        void CallbackManager::EventTaskCallback(PAL_Instance* pal, void* user_data)
        {
            if (user_data)
            {
                CallbackManager* mgr = (CallbackManager*)user_data;

                if (pthread_mutex_trylock(&mgr->m_callbackStopMutex) != 0)
                {
                    return;
                }

                CallbackEntry* entry = mgr->GetCallbackEntry();

                if (entry)
                {
                    if (entry->callback)
                    {
                        entry->callback->Invoke();

                        delete entry->callback;

                    }

                    nsl_free(entry);
                }
                pthread_mutex_unlock(&mgr->m_callbackStopMutex);
            }
        }

        void CallbackManager::StopAllCallbacks()
        {
            pthread_mutex_lock( &m_callbackStopMutex );
            
            CallbackEntry* entry = NULL;

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

            pthread_mutex_unlock(&m_callbackStopMutex);
        }
    }
}

/*! @} */
