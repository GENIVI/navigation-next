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

@file notificationthread.cpp
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

/*!
    @addtogroup abpalgpswinmobile
    @{
*/

#include "notificationthread.h"
#include "locationlistener.h"
#include <algorithm>
#include <functional>


#define MINIMUM_NOTIFY_TIMEOUT_INTERVAL     250


// Helper to compare LocationListener objects with find_if
struct LocationListenerCompare : public std::binary_function<LocationListener*, const LocationListener*, bool>
{
    bool operator()(LocationListener* rhs, const LocationListener* lhs) const
    {
        return rhs->IsSameListener(*lhs);
    }
};

NotificationThread::NotificationThread(GpsContext* gpsContext, ListenersChangedCallback* listenersChangedCallback) :
    GpsThread(gpsContext),
    m_listenersChangedCallback(listenersChangedCallback),
    m_listenersChangedEvent(0)
{
    InitializeCriticalSection(&m_listenersLock);
    InitializeCriticalSection(&m_locationLock);
}

NotificationThread::~NotificationThread()
{
    if (m_listenersChangedEvent)
    {
        CloseHandle(m_listenersChangedEvent);
    }

    std::for_each(m_listeners.begin(), m_listeners.end(), DeleteListener);

    DeleteCriticalSection(&m_listenersLock);
    DeleteCriticalSection(&m_locationLock);
}

PAL_Error NotificationThread::CreateInstance()
{
    if (!m_listenersChangedEvent)
    {
        m_listenersChangedEvent = CreateEvent(0, FALSE, FALSE, 0);
    }

    return (m_listenersChangedEvent != NULL) ? PAL_Ok : PAL_Failed;
}

void NotificationThread::SetLocation(const Location& location)
{
    LockLocation();
    m_location = location;
    UnlockLocation();
}

void NotificationThread::GetLocation(Location& location)
{
    LockLocation();
    location = m_location;
    UnlockLocation();
}

PAL_Error NotificationThread::AddListener(LocationListener* listener)
{
    PAL_Error err = PAL_Ok;

    LockListeners();

    std::vector<LocationListener*>::iterator i = std::find_if(m_listeners.begin(), m_listeners.end(), bind2nd(LocationListenerCompare(), listener));
    if (i == m_listeners.end())
    {
        m_listeners.push_back(listener);

        UpdateListenerFixCriteria();
        SetEvent(m_listenersChangedEvent);
    }
    else
    {
        err = PAL_Failed;
    }

    UnlockListeners();

    return err;
}

PAL_Error NotificationThread::RemoveListener(LocationListener* listener)
{
    PAL_Error err = PAL_Ok;

    LockListeners();

    std::vector<LocationListener*>::iterator i = std::find_if(m_listeners.begin(), m_listeners.end(), bind2nd(LocationListenerCompare(), listener));
    if (i != m_listeners.end())
    {
        LocationListener* listener = *i;

        m_listeners.erase(i);
        delete listener;

        UpdateListenerFixCriteria();
        SetEvent(m_listenersChangedEvent);
    }
    else
    {
        err = PAL_Failed;
    }

    UnlockListeners();

    return err;
}

PAL_Error NotificationThread::SuspendListener(LocationListener* listener)
{
    PAL_Error err = PAL_Ok;

    LockListeners();

    std::vector<LocationListener*>::iterator i = std::find_if(m_listeners.begin(), m_listeners.end(), bind2nd(LocationListenerCompare(), listener));
    if (i != m_listeners.end())
    {
        (*i)->Suspend();
        UpdateListenerFixCriteria();
    }
    else
    {
        err = PAL_Failed;
    }

    UnlockListeners();

    return err;
}

PAL_Error NotificationThread::ResumeListener(LocationListener* listener)
{
    PAL_Error err = PAL_Ok;

    LockListeners();

    std::vector<LocationListener*>::iterator i = std::find_if(m_listeners.begin(), m_listeners.end(), bind2nd(LocationListenerCompare(), listener));
    if (i != m_listeners.end())
    {
        (*i)->Resume();

        UpdateListenerFixCriteria();
        SetEvent(m_listenersChangedEvent);          // Set event to provide fix immediately
    }
    else
    {
        err = PAL_Failed;
    }

    UnlockListeners();

    return err;
}

PAL_Error NotificationThread::RemoveOneShotListeners()
{
    PAL_Error err = PAL_Ok;
    bool listenersChanged = false;

    LockListeners();

    std::vector<LocationListener*>::iterator i;
    while ((i = std::find_if(m_listeners.begin(), m_listeners.end(), IsOneShotListener)) != m_listeners.end())
    {
        LocationListener* listener = *i;

        m_listeners.erase(i);
        delete listener;

        listenersChanged = true;
    }

    if (listenersChanged)
    {
        UpdateListenerFixCriteria();
        SetEvent(m_listenersChangedEvent);
    }

    UnlockListeners();

    return err;
}

void NotificationThread::DeleteListener(LocationListener* listener)
{
    delete listener;
}

void NotificationThread::UpdateListenerFixCriteria()
{
    if (m_listenersChangedCallback)
    {
        bool listenersPresent = FALSE;
        uint32 lowestCommonInterval = INFINITE;
        uint32 lowestCommonAccuracy = INFINITE;

        LockListeners();

        std::vector<LocationListener*>::iterator i = m_listeners.begin();
        while (i != m_listeners.end())
        {
            LocationListener* listener = *i;
            if (!listener->IsSuspended())
            {
                listenersPresent = TRUE;
                lowestCommonInterval = min(listener->GetFixInterval(), lowestCommonInterval);
                lowestCommonAccuracy = min(listener->GetFixAccuracy(), lowestCommonAccuracy);
            }
            i++;
        }

        UnlockListeners();

        if (lowestCommonInterval == INFINITE)
        {
            lowestCommonInterval = 0;
        }
        if (lowestCommonAccuracy == INFINITE)
        {
            lowestCommonAccuracy = 0;
        }
        
        (m_listenersChangedCallback)(GetGpsContext(), listenersPresent, lowestCommonInterval, lowestCommonAccuracy);
    }
}

uint32 NotificationThread::NotifyListeners()
{
    Location currentLocation;
    GetLocation(currentLocation);
    uint32 timeout = INFINITE;
    bool listenersChanged = false;

    LockListeners();

    std::vector<LocationListener*>::iterator i = m_listeners.begin();
    while (i != m_listeners.end())
    {
        LocationListener* listener = *i;
        uint32 listenerTimeout = listener->Notify(currentLocation);
        if (listenerTimeout != INFINITE)
        {
            if (listenerTimeout < MINIMUM_NOTIFY_TIMEOUT_INTERVAL)
            {
                listenerTimeout = MINIMUM_NOTIFY_TIMEOUT_INTERVAL;
            }

            if (listenerTimeout <  timeout)
            {
                timeout = listenerTimeout;
            }
            i++;
        }
        else
        {
            i = m_listeners.erase(i);
            delete listener;

            listenersChanged = true;
        }
    }

    if (listenersChanged)
    {
        UpdateListenerFixCriteria();
    }

    UnlockListeners();

    return timeout;
}

DWORD NotificationThread::ThreadProc()
{
    HANDLE waitEvents[] = { m_listenersChangedEvent, GetStopEvent() };
    uint32 timeout = INFINITE;
    Location location;

    while (true)
    {
        DWORD result = WaitForMultipleObjects(2, waitEvents, FALSE, timeout);
        if (result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT)
        {
            // Notify the listeners of the latest fix
            timeout = NotifyListeners();
        }
        else if (result == WAIT_OBJECT_0 + 1)
        {
            // exit thread
            break;
        }
        else
        {
            // an error occurred, break out of the loop
            break;
        }
    }

    return 0;
}

DWORD NotificationThread::NotifyThreadProc(LPVOID parameter)
{
    NotificationThread* pThis = reinterpret_cast<NotificationThread*>(parameter);
    DWORD result = pThis->ThreadProc();
    pThis->Release();
    return result;
}

bool NotificationThread::IsOneShotListener(LocationListener* listener)
{
    return listener->IsOneShot();
}

/*! @} */
