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

@file     TheadHandler.cpp
@date     04/13/2009
@defgroup THREAD_HANDLER
*/
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "paltaskqueuethreadhandler.h"
#include "paltaskqueuelock.h"

// Local Constants ...........................................................

// Maximum number in request queue. Used for semaphores
const LONG MAXIMUM_REQUESTS = 1000;

/*! Timeout value for message pump. See MainThread() for more details. */
const DWORD MESSAGE_QUEUE_TIMEOUT_IN_MS = 500;


PALTaskQueueThreadHandler::PALTaskQueueThreadHandler(void)
    :   m_requestSemaphore(NULL),
        m_cancelRequestSemaphore(NULL),
        m_shutdownEvent(NULL),
        m_mainThread(NULL),
        m_initStartEvent(NULL),
        m_initFinishEvent(NULL),
        m_bInitialized(false)
{
}

PALTaskQueueThreadHandler::~PALTaskQueueThreadHandler(void)
{
    if (m_mainThread && m_shutdownEvent)
    {
        // Trigger shutdown of thread and wait for it to finish
        SetEvent(m_shutdownEvent);
        WaitForSingleObject(m_mainThread, INFINITE);
        CloseHandle(m_mainThread);
    }

    if (m_requestSemaphore && m_cancelRequestSemaphore && m_shutdownEvent)
    {
        // Delete all synchronization objects
        CloseHandle(m_cancelRequestSemaphore);
        CloseHandle(m_requestSemaphore);
        CloseHandle(m_shutdownEvent);
        if(m_initStartEvent)
        {
            CloseHandle(m_initStartEvent);
            m_initStartEvent = NULL;
        }

        if (m_initFinishEvent)
        {
            CloseHandle(m_initFinishEvent);
            m_initFinishEvent = NULL;
        }

        DeleteCriticalSection(&m_queueAccess);
    }
}


// Public Functions ..............................................................................

// See description in "ServiceConnectorBase.h"
PAL_Error
PALTaskQueueThreadHandler::AddRequest(void* pThis     /*!< Pointer to the request handler class. The pure virtual function
                                                           HandleRequest() will be called to handle the request. */
                                                           )
{
    // Make sure the main thread is running
    if (!StartThreadIfNecessary())
    {
        return PAL_ErrNoInit;
    }
    if(pThis == NULL)
    {
        return PAL_Ok;
    }
    // Add request to queue. The requests will be handled in the main thread
    PALTaskQueue_Lock lock(m_queueAccess);
    m_requests.push(pThis);

    // Trigger main thread
    BOOL result = ReleaseSemaphore(m_requestSemaphore, 1, NULL);
    return (result ? PAL_Ok : PAL_ErrNoInit);
}

// See description in "ServiceConnectorBase.h"
PAL_Error
PALTaskQueueThreadHandler::AddCancelRequest(void* pThis)
{
    // Make sure the main thread is running
    if (!StartThreadIfNecessary())
    {
        return PAL_ErrNoInit;
    }

    {
        // Add request to cancel queue. The requests will be handled in the main thread
        PALTaskQueue_Lock lock(m_queueAccess);
        m_cancelRequests.push(pThis);
    }

    // Trigger main thread
    BOOL result = ReleaseSemaphore(m_cancelRequestSemaphore, 1, NULL);
    return (result ? PAL_Ok : PAL_ErrNoInit);
}

PAL_Error
PALTaskQueueThreadHandler::Initialize(PAL_Instance* pal)
{
    // Make sure the main thread is running
    m_initStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_initFinishEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(m_initStartEvent == NULL || m_initFinishEvent == NULL)
    {
        return PAL_ErrNoInit;
    }

    if (!StartThreadIfNecessary())
    {
        return PAL_ErrNoInit;
    }

    m_ppPal = pal;

    SetEvent(m_initStartEvent); // Run initialize case in MainThread
    WaitForSingleObject(m_initFinishEvent, INFINITE); //Wait until initialize event signal to continue
    return m_bInitialized == true ? PAL_Ok : PAL_ErrNoInit;
}

// Private functions .............................................................................

/*! Main thread.

This thread handles the synchronization to the NavBuilder (bluebox). All asynchronous
function are triaged through this thread.

@return 0

@see AddRequest
@see AddCancelRequest
*/
DWORD WINAPI
PALTaskQueueThreadHandler::MainThread(LPVOID parameter)
{
    // The parameter is the this pointer of this class
    PALTaskQueueThreadHandler* pThis = (PALTaskQueueThreadHandler*)parameter;

    const DWORD HANDLE_COUNT = 4;
    HANDLE handles[HANDLE_COUNT] =
    {
        pThis->m_shutdownEvent,
        pThis->m_requestSemaphore,
        pThis->m_cancelRequestSemaphore,
        pThis->m_initStartEvent
    };

    // Loop until we get the shutdown event
    while (true)
    {
        /*
            The function MsgWaitForMultipleObjects() sometimes doesn't return even if there are new messages in the queue.
            We think this happens when messages are posted to the thread while semaphores are processed. In order to avoid
            having the message queue being stuck we add a timeout. If the timeout expires then we also check for
            messages. Don't make the timeout value too small to not overwhelm the system.
        */
        DWORD waitResult = MsgWaitForMultipleObjects(HANDLE_COUNT, handles, FALSE, MESSAGE_QUEUE_TIMEOUT_IN_MS, QS_ALLINPUT);

        switch (waitResult)
        {
            // Shutdown event was triggered
            case WAIT_OBJECT_0:
                break;

            // Request semaphore was triggered
            case WAIT_OBJECT_0 + 1:
            {
                void* pRequestHandler = NULL;
                {
                    // Get first element from queue and remove it
                    PALTaskQueue_Lock lock(pThis->m_queueAccess);
                    pRequestHandler = pThis->m_requests.front();
                    pThis->m_requests.pop();
                }
                if(pRequestHandler)
                {
                    // Forward the request to the request handler
                    //pRequestHandler->HandleRequest();
                }
                break;
            }

            // Cancel request semaphore was triggered
            case WAIT_OBJECT_0 + 2:
            {
                void* pRequestHandler = NULL;
                {
                    // Get first element from queue and remove it
                    PALTaskQueue_Lock lock(pThis->m_queueAccess);
                    pRequestHandler = pThis->m_cancelRequests.front();
                    pThis->m_cancelRequests.pop();
                }

                // Forward the cancel request to the request handler
                //pRequestHandler->HandleCancelRequest();
                break;
            }
            // Initialize or Reset event was triggered
            case WAIT_OBJECT_0 + 3:
            {
                // Trigger blocked UI thread to continue
                SetEvent(pThis->m_initFinishEvent);
                break;
            }
            // Any message in the message queue (see MSDN for MsgWaitForMultipleObjects()) or if the timeout occured.
            case WAIT_TIMEOUT:
            case WAIT_OBJECT_0 + 4:
            {
                MSG msg;
                //Pump windows message for bluebox
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                break;
            }

            case WAIT_FAILED:
                break;
        }
    }

    return 0;
}

/*! Starts the main thread if it is not already running.

@return 'true' for success.
*/
bool
PALTaskQueueThreadHandler::StartThreadIfNecessary()
{
    // Check if we are called for the first time
    if (!m_mainThread)
    {
        // Initialize all synchronization objects
        InitializeCriticalSection(&m_queueAccess);
        m_shutdownEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_requestSemaphore = CreateSemaphore(NULL, 0, MAXIMUM_REQUESTS, NULL);
        m_cancelRequestSemaphore = CreateSemaphore(NULL, 0, MAXIMUM_REQUESTS, NULL);

        if (!m_shutdownEvent || !m_requestSemaphore || !m_cancelRequestSemaphore)
        {
            return false;
        }

        // Start main thread
        m_mainThread = CreateThread(NULL, 0, &PALTaskQueueThreadHandler::MainThread, this, 0, NULL);
        if (!m_mainThread)
        {
            return false;
        }
    }

    return true;
}

/*! @} */
