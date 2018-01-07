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

 @file     timerimpl.c
 @date     03/09/2009
 @defgroup Timer Implemenation Source File
 */
/*
 See file description in header file.

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

#include <errno.h>
#include <time.h>
#include "palstdlib.h"
#include "timerimpl.h"
#include <pthread.h>
#include <unistd.h>
#include "paltaskqueue.h"
#include "palclock.h"
#include "paltaskqueue.h"
#include <signal.h>

typedef struct Timer Timer;

struct TimerNode
{
    Timer *timer;
    struct TimerNode* next;
};

struct TimerManager
{
    PAL_Instance *pal;

    /* Linked list of timers */
    struct TimerNode* timerList;

    pthread_mutex_t timerMutex;
};

struct Timer
{
    uint32 interval; /* ms */
    uint32 posixTimerInterval; /* ms */
    uint32 actualCallbackInterval; /* ms */
    PAL_TimerCallback* callback;
    void* userData;
    timer_t timerId;
    int isExpired;
    unsigned int isCancelFail;

    /* Event struct for timer events */
    struct sigevent event;

    TimerManager* timerManager;
    uint32 ownerID;
    PAL_QueueHandle handle;
};

pthread_mutex_t g_timer_mutex_t;
int g_is_mutex_t_initialized = 0;

void
TaskQueueCallback(PAL_Instance* pal, void* pParam)
{
    Timer *timer = (Timer *)pParam;
    if (timer)
    {
        uint32 interval = timer->interval;
        uint32 posixTimerInterval = timer->posixTimerInterval;
        uint32 actualCallbackInterval = PAL_ClockGetTimeMs() - timer->actualCallbackInterval;
        // timer callback could cancel the timer, so save these values first
        PAL_TimerCallback* callback = timer->callback;
        void* userData = timer->userData;
        TimerManager* timerManager = timer->timerManager;
        //nsl_assert(timerManager != NULL);
        if (timerManager == NULL)
        {
            return;
        }
        timer->isExpired = true;
        // remove the expired timer first
        TimerManagerDestroyTimer(timerManager, callback, userData);
        if (callback)
        {
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "PAL_Timer Fired callback=%p interval=%d qnxTimer=%d actual=%d",
                        callback, interval, posixTimerInterval, actualCallbackInterval);
            callback(timerManager->pal, userData, PTCBR_TimerFired);
        }
    }
}

static void TimerCallback(Timer* timer)
{
    if(timer->isCancelFail)
    {
        nsl_free(timer);
        timer = NULL;
        return;
    }

    if (timer && timer->timerManager)
    {
        // Event task queue could be overload (> 300 tasks in queue).
        // In that case Timer task is executed after all event tasks are executed.
        // It is not right because timer assumes callback should be invoked immediately
        // when timer is expired. Therefore the timer task is scheduled in Event task queue
        // with the highest priority
        timer->posixTimerInterval = PAL_ClockGetTimeMs() - timer->posixTimerInterval;
        if (timer->handle)
        {
            (void)PAL_WorkerTaskQueueAddWithPriority(timer->timerManager->pal,
                                                     timer->handle,
                                                     TaskQueueCallback,
                                                     timer,
                                                     &timer->ownerID,
                                                     MAX_USER_TASK_PRIORITY);
        }
        else
        {
            (void)PAL_EventTaskQueueAddWithPriority(timer->timerManager->pal,
                                                    TaskQueueCallback,
                                                    timer,
                                                    &timer->ownerID,
                                                    MAX_USER_TASK_PRIORITY);
        }
    }
}

static void timerHandler(union sigval val)
{
    pthread_mutex_lock(&g_timer_mutex_t);
    Timer* timer = (Timer*) val.sival_ptr;
    TimerCallback(timer);
    pthread_mutex_unlock(&g_timer_mutex_t);
}

static Timer* TimerAllocate(TimerManager* timerManager,
                            PAL_QueueHandle handle,
                            uint32 interval,
                            PAL_TimerCallback* callback,
                            void* userData)
{
    if ((timerManager == NULL) || (callback == NULL))
    {
        return NULL;
    }

    Timer* timer = (Timer *) nsl_malloc(sizeof(Timer));
    if (timer)
    {
        timer->interval = interval;
        timer->callback = callback;
        timer->userData = userData;
        //    timer->timerId = -1;
        timer->timerManager = timerManager;
     
        timer->isExpired = FALSE;
        timer->isCancelFail = FALSE;
        timer->handle = handle;
        timer->posixTimerInterval = PAL_ClockGetTimeMs();
        timer->actualCallbackInterval = PAL_ClockGetTimeMs();

        memset (&timer->event, 0, sizeof (struct sigevent));
        timer->event.sigev_notify = SIGEV_THREAD;
        timer->event.sigev_notify_function = timerHandler;
        timer->event.sigev_value.sival_ptr = timer;
        timer->event.sigev_notify_attributes = NULL;

        if (timer_create(CLOCK_REALTIME, &timer->event, &timer->timerId) == -1)
        {
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "timer_create() error: %s\n", strerror(errno));
            timer->timerId = NULL;
            return NULL;
        }
    }

    return timer;
}

static PAL_Error TimerCancel(Timer* timer)
{
    PAL_Error palError = PAL_ErrNotFound;

    if (timer == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (timer->timerId != NULL)
    {
        /* send a zeroed out structure to cancel the timer */
        if(timer_delete(timer->timerId) != 0)
        {
            timer->isCancelFail = TRUE;
        }

        if (!timer->isExpired) /* timer has not expired */
        {
            if (timer->handle)
            {
                palError = PAL_WorkerTaskQueueRemove(timer->timerManager->pal, timer->handle, timer->ownerID, FALSE);
            }
            else
            {
                palError = PAL_EventTaskQueueRemove(timer->timerManager->pal, timer->ownerID, FALSE);
            }

            /* timer is canceled */
            if (palError == PAL_Ok && timer->callback != NULL)
            {
                PAL_Instance* pal = timer->timerManager->pal;
                timer->callback(pal, timer->userData, PTCBR_TimerCanceled);
            }

            if (palError != PAL_Ok || timer->isCancelFail)
            {

                palError = PAL_ErrNotFound;
            }
        }
    }

    return palError;
}

static PAL_Error TimerSchedule(Timer* timer)
{
    int expireMS = timer->interval;
    struct itimerspec its;

    if ((timer == NULL) || (timer->timerId == NULL))
    {
        return PAL_Failed;
    }

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (expireMS >= 1000)
    {
        its.it_value.tv_sec = expireMS / 1000;
        its.it_value.tv_nsec = (expireMS % 1000) * 1000000;
    }
    else
    {
        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = expireMS * 1000000;
    }

    // Timer with it_value.tv_sec == 0 and it_value.tv_nsec == 0 will NEVER get fired in QNX.
    // We set it_value.tv_nsec to 1 nanosecond in this case as a workaround.
    if (expireMS == 0)
    {
        its.it_value.tv_nsec = 1;
    }

    if (timer_settime(timer->timerId, 0, &its, NULL) == -1)
    {
        return PAL_Failed;
    }

    return PAL_Ok;
}

PAL_Error TimerFree(Timer* timer)
{
    PAL_Error palError = PAL_ErrNotFound;
    if (timer != NULL)
    {
        /* stop the timer */
        if( TimerCancel(timer) == PAL_Ok )
        {
            nsl_free(timer);
            timer = NULL;
        }
    }

    return palError;
}

static int TimerCompare(const Timer* t1, const Timer* t2)
{
    if ((t1 == NULL) || (t2 == NULL))
    {
        return 1;
    }

    if (t1->callback < t2->callback)
    {
        return -1;
    }
    else if (t1->callback > t2->callback)
    {
        return 1;
    }
    else if (t1->userData < t2->userData)
    {
        return -1;
    }
    else if (t1->userData > t2->userData)
    {
        return 1;
    }

    return 0;
}

PAL_Error TimerManagerAddTimerToList(TimerManager* timerManager, Timer* newTimer)
{
    if ((timerManager == NULL) || (newTimer == NULL))
    {
        return PAL_Failed;
    }

    struct TimerNode* newnode = (struct TimerNode*)nsl_malloc(sizeof(struct TimerNode));
    if (newnode == NULL)
    {
        return PAL_Failed;
    }
    newnode->timer = newTimer;
    newnode->next = NULL;

    if (timerManager->timerList == NULL)
    {
        timerManager->timerList = newnode;
    }
    else
    {
        struct TimerNode *iterator = timerManager->timerList;
        while (iterator->next != NULL)
        {
            iterator = iterator->next;
        }
        iterator->next = newnode;
    }

    return PAL_Ok;
}

Timer* FindTimerByID(TimerManager* timerManager, int tid)
{
    if (timerManager == NULL)
    {
        return NULL;
    }

    struct TimerNode *iterator = timerManager->timerList;

    while (iterator != NULL)
    {
        if (*(int*)(iterator->timer->timerId) == tid)
        {
            break;
        }
        iterator = iterator->next;
    }

    if (iterator)
    {
        return iterator->timer;
    }
    return NULL;
}

Timer* FindTimer(TimerManager* timerManager, Timer* timerKey)
{
    if ((timerManager == NULL) || (timerKey == NULL))
    {
        return NULL;
    }

    struct TimerNode *iterator = timerManager->timerList;

    while (iterator != NULL)
    {
        if (TimerCompare(timerKey, iterator->timer) == 0)
        {
            break;
        }
        iterator = iterator->next;
    }

    if (iterator)
    {
        return iterator->timer;
    }
    return NULL;
}

/* Remove timer based on pointer compare */
PAL_Error TimerManagerRemoveTimer(TimerManager* timerManager, Timer *timer)
{
    if ((timerManager == NULL) || (timer == NULL))
    {
        return PAL_Failed;
    }
    if (timerManager->timerList == NULL)
    {
        return PAL_Failed;
    }

    struct TimerNode *iterator = timerManager->timerList;
    if (timerManager->timerList->timer->timerId == timer->timerId)
    {
        /* Remove from list, free node */
        timerManager->timerList = iterator->next;
        //nsl_free(iterator);
        return PAL_Ok;
    }
    while (iterator->next != NULL)
    {
        if (iterator->next->timer == timer)
        {
            /* Remove from list, free node */
            struct TimerNode* tmp = iterator->next->next;
            //nsl_free(iterator->next);
            iterator->next = tmp;
            return PAL_Ok;
        }
        else {
            iterator = iterator->next;
        }
    }

    return PAL_Failed;
}

PAL_Error TimerManagerCreateTimer(TimerManager* timerManager,
                                  PAL_QueueHandle handle,
                                  int32 milliseconds,
                                  PAL_TimerCallback* callback,
                                  void* userData)
{
    /* create a timer and add to the list */
    if ((timerManager == NULL) || (callback == NULL))
    {
        return PAL_Failed;
    }

    if (g_is_mutex_t_initialized == 0)
    {
        return PAL_Failed;
    }

    //pthread_mutex_lock(&timerManager->timerMutex);
    pthread_mutex_lock(&g_timer_mutex_t);

    Timer* timer = TimerAllocate(timerManager, handle, milliseconds, callback, userData);
    if (timer == NULL)
    {
        //pthread_mutex_unlock(&timerManager->timerMutex);
        pthread_mutex_unlock(&g_timer_mutex_t);
        return PAL_Failed;
    }

    if (TimerManagerAddTimerToList(timerManager, timer) != PAL_Ok)
    {
        (void)TimerFree(timer);
        //pthread_mutex_unlock(&timerManager->timerMutex);
        pthread_mutex_unlock(&g_timer_mutex_t);
        return PAL_Failed;
    }

    if (TimerSchedule(timer) != PAL_Ok)
    {
        /* failed */
        TimerManagerRemoveTimer(timerManager, timer);
        (void)TimerFree(timer);
        //pthread_mutex_unlock(&timerManager->timerMutex);
        pthread_mutex_unlock(&g_timer_mutex_t);
        return PAL_Failed;
    }
    //pthread_mutex_unlock(&timerManager->timerMutex);
    pthread_mutex_unlock(&g_timer_mutex_t);

    return PAL_Ok;
}

PAL_Error TimerManagerDestroyTimer(TimerManager* timerManager,
                                   PAL_TimerCallback* callback, void* userData)
{
    if ((timerManager == NULL) || (callback == NULL) || (userData == NULL))
    {
        return PAL_Failed;
    }

    if (g_is_mutex_t_initialized == 0)
    {
        return PAL_Failed;
    }

    Timer timerKey;
    timerKey.callback = callback;
    timerKey.userData = userData;

    //pthread_mutex_lock(&timerManager->timerMutex);
    pthread_mutex_lock(&g_timer_mutex_t);

    /* find timer */
    Timer *timer = FindTimer(timerManager, &timerKey);
    if (timer == NULL)
    {
        /* timer not found, maybe already expired */
        //pthread_mutex_unlock(&timerManager->timerMutex);
        pthread_mutex_unlock(&g_timer_mutex_t);
        return PAL_ErrNotFound;
    }

    /* remove from list */
    TimerManagerRemoveTimer(timerManager, timer);
    /* destroy timer */
    PAL_Error palError = TimerFree(timer);
    //pthread_mutex_unlock(&timerManager->timerMutex);
    pthread_mutex_unlock(&g_timer_mutex_t);

    return palError;
}

TimerManager* TimerManagerCreate(PAL_Instance* pal)
{
    if (pal == NULL)
    {
        return NULL;
    }

    TimerManager* timerManager = (TimerManager*)nsl_malloc(sizeof(TimerManager));
    if (timerManager)
    {
        timerManager->pal = pal;
        timerManager->timerList = NULL;
    }
    else
    {
        return NULL;
    }

    //pthread_mutex_init(&timerManager->timerMutex, NULL);
    pthread_mutex_init(&g_timer_mutex_t, NULL);
    g_is_mutex_t_initialized = 1;

    return timerManager;
}

void TimerManagerDestroy(TimerManager *timerManager)
{
    if (timerManager)
    {
        pthread_mutex_lock(&g_timer_mutex_t);
        if (timerManager->timerList)
        {
            /* free all timers */
            while (timerManager->timerList != NULL)
            {
                Timer* timer = timerManager->timerList->timer;
                TimerManagerRemoveTimer(timerManager,
                                        timerManager->timerList->timer);
                TimerFree(timer);
            }
        }
        pthread_mutex_unlock(&g_timer_mutex_t);

        //pthread_mutex_destroy(&timerManager->timerMutex);
        pthread_mutex_destroy(&g_timer_mutex_t);
        g_is_mutex_t_initialized = 0;
        nsl_free(timerManager);
        timerManager = NULL;
    }

    return;
}

/*! @} */
