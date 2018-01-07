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

@file notificationthread.h
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

#pragma once

#include "windows.h"
#include "gpsthread.h"
#include "location.h"
#include <vector>

class LocationListener;

typedef void ListenersChangedCallback(GpsContext* gpsContext, bool listenersPresent, uint32 lowestCommonInterval, uint32 lowestCommonAccuracy);

/*! NotificationThread is used to notify the application when a new fix is available */
class NotificationThread : public GpsThread
{
public:   
    NotificationThread(GpsContext* gpsContext, ListenersChangedCallback* listenersChangedCallback);
    
    /*! Sets the latest location */
    void                            SetLocation(const Location& location);

    /*! Gets the latest location */
    void                            GetLocation(Location& location);

    /*! Add a new location listener */
    PAL_Error                       AddListener(LocationListener* listener);

    /*! Remove an existing location listener */
    PAL_Error                       RemoveListener(LocationListener* listener);

    /*! Suspend a location listener */
    PAL_Error                       SuspendListener(LocationListener* listener);

    /*! Resume a location listener */
    PAL_Error                       ResumeListener(LocationListener* listener);

    /*! Remove all one shot listeners */
    PAL_Error                       RemoveOneShotListeners();
    
protected:
    ~NotificationThread();
    virtual PAL_Error               CreateInstance();
    virtual PAL_Error               DestroyInstance() { return PAL_Ok; };
    virtual LPTHREAD_START_ROUTINE  GetThreadProc() const { return NotifyThreadProc; }

private:
    void                            LockListeners() { EnterCriticalSection(&m_listenersLock); };
    void                            UnlockListeners() { LeaveCriticalSection(&m_listenersLock); };
    static void                     DeleteListener(LocationListener* listener);

    void                            LockLocation() { EnterCriticalSection(&m_locationLock); };
    void                            UnlockLocation() { LeaveCriticalSection(&m_locationLock); };

    static DWORD _stdcall           NotifyThreadProc(LPVOID parameter);
    DWORD                           ThreadProc();
    uint32                          NotifyListeners();

    static bool                     IsOneShotListener(LocationListener* listener);

    void                            UpdateListenerFixCriteria();

    HANDLE                          m_listenersChangedEvent;
    ListenersChangedCallback*       m_listenersChangedCallback;
    Location                        m_location;
    CRITICAL_SECTION                m_listenersLock;
    CRITICAL_SECTION                m_locationLock;
    std::vector<LocationListener*>  m_listeners;
};


/*! @} */
