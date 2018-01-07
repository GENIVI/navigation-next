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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "palerror.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "palevent.h"
#include <QMutex>
#include <QWaitCondition>

#define TIMEOUT unsigned int
#define BOOL int

struct PAL_Event
{
    QMutex* mutex;
    QWaitCondition* cond;
    BOOL flag;
    TIMEOUT wait_time;
};

PAL_DEC PAL_Error
PAL_EventCreate(PAL_Instance* /*pal*/, PAL_Event** palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    *palevent = (PAL_Event*)nsl_malloc(sizeof(PAL_Event));

    if (*palevent == NULL)
    {
        return PAL_ErrNoMem;
    }

    nsl_memset(*palevent, 0, sizeof(PAL_Event));
    (*palevent)->flag = FALSE;
    (*palevent)->wait_time = 0;

    (*palevent)->mutex = new QMutex();
    (*palevent)->cond = new QWaitCondition();
    return PAL_Ok;
}


PAL_DEC PAL_Error
PAL_EventDestroy(PAL_Event* palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    delete palevent->cond;
    delete palevent->mutex;

    free(palevent);

    return PAL_Ok;
}

PAL_DEC PAL_Error
PAL_EventWaitForEvent(PAL_Event* palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (palevent->wait_time > 0)
    {
        return palevent->cond->wait(palevent->mutex, palevent->wait_time)?PAL_Ok:PAL_Failed;
    }
    else
    {
        return palevent->cond->wait(palevent->mutex)?PAL_Ok:PAL_Failed;
    }
}

PAL_DEC PAL_Error
PAL_EventSet(PAL_Event* palevent)
{
    palevent->mutex->lock();

    palevent->flag = TRUE;
    palevent->cond->wakeAll();

    palevent->mutex->unlock();

    return PAL_Ok;
}

