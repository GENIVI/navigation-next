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

@file gpsthread.h
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

#include "palerror.h"
#include "winbase.h"

#pragma once

class GpsContext;

/*! Base class for all GPS threads
*/
class GpsThread
{
public:
    GpsThread(GpsContext* context);

    /*! Creates and initializes the GPS object
    @return PAL_Error
    */
    PAL_Error                           Create();

    /*! Request that the object be destroyed
    Terminate the thread and free all resources
    @return PAL_Error
    */
    PAL_Error                           Destroy();

    /*! Request that a new thread be started
    @return PAL_Error
    */
    PAL_Error                           StartThread();

protected:
    virtual ~GpsThread();

    /*! Add a reference to this object
    Called by StartThread to add a reference for the new thread
    */
    void                                AddRef();

    /*! Release a reference to this object
    Must be called by a derived ThreadProc; should be the last operation before returning
    */
    void                                Release();

    /*! Request that the thread be stopped
    */
    void                                StopThread();

    /* Derived classes must implement to return threadproc
    */
    virtual LPTHREAD_START_ROUTINE      GetThreadProc() const = 0;

protected:
    GpsContext*                         GetGpsContext() const { return m_gpsContext; }
    HANDLE                              GetThreadHandle() const { return m_thread; }
    DWORD                               GetThreadId() const { return m_threadId; }
    HANDLE                              GetStopEvent() const { return m_stopEvent; }

    /*! Instance-specific creation function
    @return PAL_Error
    */
    virtual PAL_Error                   CreateInstance() = 0;

    /*! Instance-specific destroy function
    @return PAL_Error
    */
    virtual PAL_Error                   DestroyInstance() = 0;

private:
    GpsContext*                         m_gpsContext;
    int                                 m_refCount;
    HANDLE                              m_thread;
    DWORD                               m_threadId;
    HANDLE                              m_stopEvent;
};

/*! @} */
