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

    @file     PlatformUtil.h
    @date     02/06/2009
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Function declarations for platform-specific utilities used for system test.

    This file contains the declaration for the platform-specific utilities
    needed for system testing.
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

/*! @{ */

#ifndef _PLATFORMUTIL_H_
#define _PLATFORMUTIL_H_

#include "paltypes.h"
#include "pal.h"

#ifdef __cplusplus
	extern "C"
		{
#endif

typedef nb_boolean boolean;

/*! Yields control to the platform OS to pump and process any pending messages.

    This function allows platform-independent test code to temporarily yield control
    to the OS to process any pending queued messages.  This function is called
    from within WaitForCallbackCompletedEvent to allow messages to be processed
    while waiting for a callback to be invoked, but should also be called when
    ending a test session so that any pending closing messages can be processed.
*/
void ProcessPendingMessages(void);

/*! Creates platform-specific event object.

    This function allows platform-independent test code to create an event object
    that can be used to signal that a callback has been invoked with a response
    from a submitted request.

    @return Generic pointer to an event object

    @see WaitForCallbackCompletedEvent
    @see SetCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
void* CreateCallbackCompletedEvent(void);

/*! Waits for the specified event object to be signalled, while allowing pending
    queued messages to be processed.

    This function waits for a given event object to be signalled, while periodically
    allowing queued messages to be processed.  Will return if the callback event
    has not been signalled within the given amount of time.

    @return TRUE indicating event was signalled, or FALSE indicating timeout or error

    @see CreateCallbackCompletedEvent
    @see SetCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
boolean WaitForCallbackCompletedEvent(
    void* callbackCompletedEvent,   /*!< event to wait on */
    uint32 timeoutMsec              /*!< maximum msec to wait for event to be signalled */
    );

/*! Signals given event object.

    This function allows platform-independent test code to signal the given event
    object.

    @see CreateCallbackCompletedEvent
    @see WaitForCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
void SetCallbackCompletedEvent(
    void* callbackCompletedEvent    /*!< event to signal */
    );

/*! Destroys the given event object.

    This function allows platform-independent test code to destroy the given event
    object.

    @see CreateCallbackCompletedEvent
*/
void DestroyCallbackCompletedEvent(
    void* callbackCompletedEvent    /*!< event to destroy */
    );

PAL_Instance* PAL_CreateInstance();

/*! Initialize heap check.

    @see TestHeapCheck
*/
void InitializeHeapCheck();

/*! Heap check test.

    @see InitializeHeapCheck
*/
void TestHeapCheck(void);

/*! Returns current thread ID.

*/
uint32 CurrentThreadId(void);

#ifdef __cplusplus
		}
#endif

#endif

/*! @} */
