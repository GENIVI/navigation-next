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

    @file     ServiceConnectorBase.cpp
    @date     04/13/2009
    @defgroup SERVICE_CONNECTOR Service Connector Dll
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

#include "paltaskqueueimpl.h"
#include "paltaskqueuethreadhandler.h"
#include <palconfig.h>
#include <palfile.h>
#include <palnet.h>

// Local variables ...............................................................................

// We have one PAL and NB_Context instance.
static PAL_Instance*    g_pal                       = NULL;

// One and only instance of thread handler
static PALTaskQueueThreadHandler*           g_threadHandler     = NULL;


// API functions .................................................................................

// See header for description
PAL_Error
PALTaskQueue_Initialize()
{
    // This function should only be called once
    if (g_threadHandler)
    {
        return PAL_ErrNoInit;
    }

    // Create thread handler
    g_threadHandler = new PALTaskQueueThreadHandler;
    if (!g_threadHandler)
    {
        return PAL_ErrNoMem;
    }

    // Create PAL, NBContext and Network instance
    bool result = g_threadHandler->Initialize(g_pal) == PAL_Ok ? true : false;

    if (!result)
    {
        delete g_threadHandler;
        g_threadHandler = NULL;
        return PAL_ErrNoMem;
    }

    return PAL_Ok;
}

// See header for description
void
PALTaskQueue_Cleanup()
{
    if (g_threadHandler)
    {
        // Delete the thread handler object. This will do a graceful shutdown of the main thread.
        delete g_threadHandler;
        g_threadHandler = NULL;
    }
}

// See header for description
PAL_Error
PALTaskQueue_AddRequest(void* requestHandler)
{
    if (!g_threadHandler)
    {
        return PAL_ErrNoInit;
    }

    // Forward call to PALTaskQueueThreadHandler
    return g_threadHandler->AddRequest(requestHandler);
}

// See header for description
PAL_Error
PALTaskQueue_AddCancelRequest(void* requestHandler)
{
    if (!g_threadHandler)
    {
        return PAL_ErrNoInit;
    }

    // Forward call to PALTaskQueueThreadHandler
    return g_threadHandler->AddCancelRequest(requestHandler);
}

/*! @} */


