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

/*--------------------------------------------------------------------------

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
#include "palstdlib.h"
#include "palimpl.h"
#include "paltimer.h"
#include "timerimpl.h"


PAL_DEF PAL_Error
PAL_TimerSet(PAL_Instance *pal, int32 milliseconds,
        PAL_TimerCallback *callback, void *userData)
{
    return PAL_TimerSetWithQueueHandle(pal, NULL, milliseconds, callback, userData);
}

PAL_DEF PAL_Error PAL_TimerSetWithQueueHandle(PAL_Instance *pal,
                                              PAL_QueueHandle handle,
                                              int32 milliseconds,
                                              PAL_TimerCallback *callback,
                                              void *userData)
{
    if ((pal == NULL) || (callback == NULL))
    {
        return PAL_ErrBadParam;
    }

    // just in case the timer is already set, cancel it first
    PAL_TimerCancel(pal, callback, userData);

    return TimerManagerCreateTimer(pal->timerManager, handle,
                                   milliseconds,
                                   callback, userData);
}

PAL_DEF PAL_Error
PAL_TimerCancel(PAL_Instance *pal, PAL_TimerCallback *callback,
        void *userData)
{
    if ((pal == NULL) || (callback == NULL))
    {
        return PAL_ErrBadParam;
    }

    return TimerManagerDestroyTimer(pal->timerManager, callback, userData);
}

uint32 PAL_TimerExpirationTime(PAL_Instance *pal, PAL_TimerCallback *callback, void *userData)
{
    return 0;
}
/*! @} */
