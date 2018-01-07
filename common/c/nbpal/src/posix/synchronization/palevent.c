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

#include <sys/time.h>
#include "palerror.h"
#include "pthread.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "palevent.h"

#define TIMEOUT unsigned int
#define BOOL int

struct PAL_Event
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    BOOL flag;
    TIMEOUT wait_time;
};

PAL_DEC PAL_Error
PAL_EventCreate(PAL_Instance* pal, PAL_Event** palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    *palevent = nsl_malloc(sizeof(PAL_Event));

    if (*palevent == NULL)
    {
        return PAL_ErrNoMem;
    }

    nsl_memset(*palevent, 0, sizeof(PAL_Event));
    (*palevent)->flag = FALSE;
    (*palevent)->wait_time = 0;

    pthread_mutex_init(&(*palevent)->mutex, NULL);
    pthread_cond_init(&(*palevent)->cond, NULL);

    return PAL_Ok;
}


PAL_DEC PAL_Error
PAL_EventDestroy(PAL_Event* palevent)
{
    if (palevent == NULL)
    {
        return PAL_ErrBadParam;
    }

    pthread_mutex_destroy(&palevent->mutex);

    pthread_cond_destroy(&palevent->cond);

    free(palevent);

    return PAL_Ok;
}

static PAL_Error
TimedWait(PAL_Event* palevent)
{
    int    rc = 0;
    PAL_Error error = PAL_Ok;

    struct timespec ts;
    struct timeval tp;

    pthread_mutex_lock(&palevent->mutex);
    gettimeofday(&tp, NULL);
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000;
    ts.tv_sec += palevent->wait_time;

    if (palevent->flag == FALSE)
    {
        rc = pthread_cond_timedwait(&palevent->cond, &palevent->mutex, &ts);
    }
    error = (rc != 0 && rc != 110 /* Timeout */) ? PAL_Failed : error;

    palevent->flag = FALSE;
    pthread_mutex_unlock(&palevent->mutex);

    return error;
}

static PAL_Error
InfiniteWait(PAL_Event* palevent)
{
    int       rc    = 0;
    PAL_Error error = PAL_Ok;

    pthread_mutex_lock(&palevent->mutex);

    if (palevent->flag == FALSE)
    {
        rc = pthread_cond_wait(&palevent->cond, &palevent->mutex);
    }
    error = (rc != 0) ? PAL_Failed : error;

    palevent->flag = FALSE;
    pthread_mutex_unlock(&palevent->mutex);

    return error;
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
        return TimedWait(palevent);
    }
    else
    {
        return InfiniteWait(palevent);
    }
}

PAL_DEC PAL_Error
PAL_EventSet(PAL_Event* palevent)
{
    pthread_mutex_lock(&palevent->mutex);

    palevent->flag = TRUE;
    pthread_cond_broadcast(&palevent->cond);

    pthread_mutex_unlock(&palevent->mutex);

    return PAL_Ok;
}

