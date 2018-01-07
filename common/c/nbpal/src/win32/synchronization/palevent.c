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

    @file palevent.c
    @date 03/13/2012
    @defgroup palevent.c in nbpal
*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#include "palevent.h"
#include "palstdlib.h"

#include <windows.h>

struct PAL_Event
{
    HANDLE event;
};

/* See header file for description. */
PAL_DEF PAL_Error
PAL_EventCreate(PAL_Instance* pal, PAL_Event** palevent)
{
    if (pal == NULL || palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    *palevent = nsl_malloc(sizeof(PAL_Event));
    if (*palevent == NULL)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(*palevent, 0, sizeof(PAL_Event));

    if (((*palevent)->event = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
      PAL_EventDestroy(*palevent);
      return PAL_Failed;
    }

    return PAL_Ok;
}

/* See header file for description. */
PAL_DEF PAL_Error
PAL_EventDestroy(PAL_Event* palevent)
{
    if (palevent)
    {
        if (palevent->event)
        {
            CloseHandle(palevent->event);
        }
        nsl_free(palevent);
    }

    return PAL_Ok;
}

/* See header file for description. */
PAL_DEF PAL_Error
PAL_EventWaitForEvent(PAL_Event* palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (!palevent->event)
    {
        return PAL_ErrNoInit;
    }

    WaitForSingleObject(palevent->event, INFINITE);
    return PAL_Ok;
}

/* See header file for description. */
PAL_DEF PAL_Error
PAL_EventSet(PAL_Event* palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (!palevent->event)
    {
        return PAL_ErrNoInit;
    }

    return SetEvent(palevent->event) ? PAL_Ok : PAL_Failed;
}

/*! @} */
