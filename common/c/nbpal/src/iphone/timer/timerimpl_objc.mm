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
#include "palstdlib.h"
#include "timerimpl.h"
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "paltaskqueue.h"
#include "palclock.h"
#include "palevent.h"
#include "smartpointer.h"
#include <map>
#include "palimpl.h"
#import <Foundation/Foundation.h>

using namespace std;

class Timer;
static void TimerDispatch(Timer* timer, PAL_Instance* palInstance);

@interface TimerDispatcher : NSObject
{
    Timer*          timer;
    PAL_Instance*   palInstance;
}
@property (nonatomic, readwrite) Timer*         timer;
@property (nonatomic, readwrite) PAL_Instance*  palInstance;
- (void)timerCallback:(NSTimer *)timer;
@end

@implementation TimerDispatcher
@synthesize timer;
@synthesize palInstance;
- (void)timerCallback:(NSTimer *)t
{
    TimerDispatch(timer, palInstance);
}
@end

struct TimerManager
{
public:
    TimerManager(PAL_Instance *pal)
        : m_pal(pal)
    {
    }
    ~TimerManager()
    {
        m_timerList.clear();
        m_executingList.clear();
    }

    PAL_Instance *m_pal;

    /* Data for save timers object */
    map<Timer*, shared_ptr<Timer> > m_timerList;

    /* Data for save current is running timers object */
    map<Timer*, shared_ptr<Timer> > m_executingList;

    pthread_mutex_t m_timerMutex;
};

class Timer
{
public:
Timer(uint32 interval, PAL_TimerCallback* callback, void* userData,
      TimerManager* timerManager, uint32 ownerID, PAL_QueueHandle handle, nb_threadId tid)
        : m_timerId(nil),
            m_timerDispatcher(nil),
            m_interval(interval),
            m_callback(callback),
            m_userData(userData),
            m_timerManager(timerManager),
            m_ownerID(ownerID),
            m_handle(handle)
            {
                nsl_memset(&m_tid, 0, sizeof(m_tid));
                PAL_EventCreate(timerManager->m_pal, &(m_timeEvent));
            }
    /* make a TimerKey for to find Timer Object*/
Timer(void* userData, PAL_TimerCallback* callback)
        : m_timerId(nil),
            m_timerDispatcher(nil),
            m_interval(0),
            m_callback(callback),
            m_userData(userData),
            m_timerManager(NULL),
            m_ownerID(0),
            m_handle(NULL),
            m_timeEvent(NULL)
            {
                nsl_memset(&m_tid, 0, sizeof(m_tid));
            }

    ~Timer()
    {
        if (m_timeEvent)
        {
            PAL_EventDestroy(m_timeEvent);
            m_timeEvent = NULL;
        }
        if (m_timerDispatcher)
        {
            [m_timerDispatcher release];
        }
    }

    NSTimer*            m_timerId;
    TimerDispatcher*    m_timerDispatcher;
    uint32              m_interval; /* ms */

    PAL_TimerCallback*  m_callback;
    void*               m_userData;

    TimerManager*       m_timerManager;
    uint32              m_ownerID;
    PAL_QueueHandle     m_handle;

    PAL_Event*          m_timeEvent;
    pthread_t           m_tid;
};

shared_ptr<Timer> FindTimer(const map<Timer*, shared_ptr<Timer> >& list, Timer* timerKey)
{
    map<Timer*, shared_ptr<Timer> >::const_iterator iterator = list.begin();
    map<Timer*, shared_ptr<Timer> >::const_iterator end = list.end();
    for (; iterator != end; ++iterator)
    {
        if ((*iterator).first->m_callback == timerKey->m_callback
            && (*iterator).first->m_userData == timerKey->m_userData)
        {
            return (*iterator).second;
        }
    }
    return shared_ptr<Timer>();
}

PAL_Error TimerCancel(shared_ptr<Timer> timer)
{
    PAL_Error palError = PAL_Ok;
    if (timer->m_timerId != nil)
    {
        [timer->m_timerId invalidate];
        timer->m_timerId = nil;

        /* timer is canceled */
        if (timer->m_callback && pthread_equal(timer->m_tid, (pthread_t)(0)))
        {
            PAL_Instance* pal = timer->m_timerManager->m_pal;
            timer->m_callback(pal, timer->m_userData, PTCBR_TimerCanceled);
        }
    }
    return palError;
}

void TaskQueueCallback(PAL_Instance* pal, void* pParam)
{
    Timer *timer = (Timer *) pParam;
    if (timer)
    {
        TimerManager* timerManager = pal->timerManager;
        nsl_assert(timerManager != NULL);

        shared_ptr<Timer> back_timer;

        pthread_mutex_lock(&timerManager->m_timerMutex);
        /* Find timer object*/
        map<Timer*, shared_ptr<Timer> >::iterator iterator = timerManager->m_timerList.find(timer);
        if (iterator != timerManager->m_timerList.end() && (back_timer = (*iterator).second))
        {
            back_timer->m_tid = pthread_self();

            /* Remove timer object from timerList and this time object will insert executing list. */
            timerManager->m_timerList.erase(timer);
            timerManager->m_executingList.insert(make_pair(back_timer.get(), back_timer));

            pthread_mutex_unlock(&timerManager->m_timerMutex);

            if (back_timer->m_callback)
            {
                back_timer->m_callback(timerManager->m_pal, back_timer->m_userData, PTCBR_TimerFired);
            }
            PAL_EventSet(back_timer->m_timeEvent);

            (void) TimerCancel(back_timer);
            return;
        }
        pthread_mutex_unlock(&timerManager->m_timerMutex);
    }
}

static void TimerDispatch(Timer* timer, PAL_Instance* palInstance)
{
    TimerManager* timerManager = palInstance->timerManager;
    if (timer)
    {
        timer->m_timerId = nil;
        if (timerManager)
        {
            shared_ptr<Timer> back_timer;

            /*  First to judge whether existed this timer object in the m_timerList, if not this timer
             *  object has been canceled.
             */
            pthread_mutex_lock(&timerManager->m_timerMutex);
            map<Timer*, shared_ptr<Timer> >::iterator iterator = timerManager->m_timerList.find(timer);
            if (iterator != timerManager->m_timerList.end())
            {
                back_timer = (*iterator).second;
            }
            pthread_mutex_unlock(&timerManager->m_timerMutex);

            if (back_timer)
            {
                if (back_timer->m_handle)
                {
                    (void) PAL_WorkerTaskQueueAddWithPriority(palInstance, back_timer->m_handle, TaskQueueCallback,
                                                              (void*) back_timer.get(), &(back_timer->m_ownerID),
                                                              MAX_USER_TASK_PRIORITY);
                }
                else
                {
                    (void) PAL_EventTaskQueueAddWithPriority(palInstance, TaskQueueCallback, (void*) back_timer.get(),
                                                             &(back_timer->m_ownerID), MAX_USER_TASK_PRIORITY);
                }
            }
        }
    }
}

shared_ptr<Timer> TimerAllocate(TimerManager* timerManager, PAL_QueueHandle handle, uint32 interval,
                                PAL_TimerCallback* callback, void* userData)
{
    if ((timerManager == NULL) || (callback == NULL))
    {
        return shared_ptr<Timer>();
    }

    shared_ptr<Timer> timer(new (std::nothrow) Timer(interval, callback, userData, timerManager, 0, handle, 0));
    if (timer)
    {
        timer->m_timerDispatcher = [[TimerDispatcher alloc] init];
        if (!timer->m_timerDispatcher)
        {
            return shared_ptr<Timer>();
        }
        timer->m_timerDispatcher.timer = timer.get();
        timer->m_timerDispatcher.palInstance = timerManager->m_pal;
    }
    return timer;
}

static PAL_Error TimerSchedule(shared_ptr<Timer> timer)
{
    timer->m_timerId = [NSTimer timerWithTimeInterval:((double)(timer->m_interval) / 1000.0)
                                               target:timer->m_timerDispatcher
                                             selector:@selector(timerCallback:)
                                             userInfo:nil
                                              repeats:FALSE];
    if (timer->m_timerId == nil)
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "Failed to start a NSTimer.\n");
        return PAL_Failed;
    }
    [[NSRunLoop currentRunLoop] addTimer:timer->m_timerId forMode:NSRunLoopCommonModes];

    return PAL_Ok;
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

    pthread_mutex_lock(&timerManager->m_timerMutex);

    shared_ptr<Timer> timer = TimerAllocate(timerManager, handle, milliseconds, callback, userData);
    if (!timer)
    {
        pthread_mutex_unlock(&timerManager->m_timerMutex);
        return PAL_Failed;
    }
    timerManager->m_timerList.insert(make_pair(timer.get(), timer));

    if (TimerSchedule(timer) != PAL_Ok)
    {
        /* failed */
        timerManager->m_timerList.erase(timer.get());
        (void)TimerCancel(timer);
        pthread_mutex_unlock(&timerManager->m_timerMutex);
        return PAL_Failed;
    }

    pthread_mutex_unlock(&timerManager->m_timerMutex);

    return PAL_Ok;
}

PAL_Error TimerManagerDestroyTimer(TimerManager* timerManager, PAL_TimerCallback* callback, void* userData)
{
    PAL_Error palError = PAL_Ok;

    if ((timerManager == NULL) || (callback == NULL))
    {
        return PAL_Failed;
    }
    Timer timerKey(userData, callback);
    shared_ptr<Timer> timer;

    do
    {
        pthread_mutex_lock(&timerManager->m_timerMutex);
        timer = FindTimer(timerManager->m_timerList, &timerKey);
        if (timer)
        {
            timerManager->m_timerList.erase(timer.get());
            pthread_mutex_unlock(&timerManager->m_timerMutex);
            break;
        }
        else
        {
            timer = FindTimer(timerManager->m_executingList, &timerKey);
            if (timer)
            {
                timerManager->m_executingList.erase(timer.get());
                if (!pthread_equal(timer->m_tid, pthread_self()))
                {
                    pthread_mutex_unlock(&timerManager->m_timerMutex);
                    PAL_EventWaitForEvent(timer->m_timeEvent);
                    continue;
                }
            }
        }
        pthread_mutex_unlock(&timerManager->m_timerMutex);

        /* TODO: Here to continuing the loop find list(m_timerList and m_executingList) for
         * avoid in the callback of timer object internal set new timer object.
         */
    }while (timer);

    if (timer)
    {
        palError = TimerCancel(timer);
    }

    return palError;
}

TimerManager* TimerManagerCreate(PAL_Instance* pal)
{
    do
    {
        if (pal == NULL)
        {
            break;
        }
        TimerManager* timerManager = new(std::nothrow) TimerManager(pal);
        if (!timerManager)
        {
            break;
        }

        pthread_mutex_init(&timerManager->m_timerMutex, NULL);
        return timerManager;
    }while (FALSE);

    return NULL;
}

void TimerManagerDestroy(TimerManager *timerManager)
{
    if (timerManager)
    {
        do
        {
            pthread_mutex_lock(&timerManager->m_timerMutex);
            if (timerManager->m_timerList.empty() && timerManager->m_executingList.empty())
            {
                pthread_mutex_unlock(&timerManager->m_timerMutex);
                break;
            }
            if (!timerManager->m_timerList.empty())
            {
                /* free all timers */
                map<Timer*, shared_ptr<Timer> >::iterator iterator = timerManager->m_timerList.begin();
                map<Timer*, shared_ptr<Timer> >::iterator end = timerManager->m_timerList.end();
                for (; iterator != end; ++iterator)
                {
                    if (shared_ptr<Timer> timer = (*iterator).second)
                    {
                        TimerCancel(timer);
                    }
                }
                timerManager->m_timerList.clear();
            }
            if (!timerManager->m_executingList.empty())
            {
                shared_ptr<Timer> timer = (*timerManager->m_executingList.begin()).second;
                if (timer)
                {
                    timerManager->m_executingList.erase(timer.get());
                    if (!pthread_equal(timer->m_tid, pthread_self()))
                    {
                        pthread_mutex_unlock(&timerManager->m_timerMutex);
                        PAL_EventWaitForEvent(timer->m_timeEvent);
                    }
                    TimerCancel(timer);
                }
            }
            pthread_mutex_unlock(&timerManager->m_timerMutex);
        }while (true);

        pthread_mutex_unlock(&timerManager->m_timerMutex);
        pthread_mutex_destroy(&timerManager->m_timerMutex);
        delete timerManager;
    }

    return;
}

/*! @} */
