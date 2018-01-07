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

    @defgroup TestTaskQueue_h Unit Tests for PAL Task Queue

    Unit Tests for PAL Task Queue

    This file contains all unit tests for the PAL Task Queue component
*/
/*
    (C) Copyright 2011 by Telecommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to Telecommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Telecommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "testtaskqueue.h"
#include "main.h"
#include "paltaskqueue.h"
#include "PlatformUtil.h"
#include "palstdlib.h"
#include "palatomic.h"

static void TestUiQueueAddTask(void);
static void TestEventQueueAddTask(void);
static void TestWorkerQueueAddTask(void);

static void UiQueueTaskCallback(PAL_Instance* pal, void* userData);
static void EventQueueTaskCallback(PAL_Instance* pal, void* userData);
static void WorkerQueueTaskCallback(PAL_Instance* pal, void* userData);

#define QUEUE_NUMBER 10
#define WAIT_TIMEOUT 5000 // in milliseconds

typedef struct _Test
{
    void* completeEvent;
} Test;

typedef struct _TaskInfo
{
    PAL_Instance*    pal;
    TaskId           taskId;
    PAL_QueueHandle  handle;
    Test*            test;
    volatile int32*  cbCounter;
} TaskInfo;

void UiQueueTaskCallback(PAL_Instance* pal, void* userData)
{
    TaskInfo* info = NULL;
    int32 counter = 0;

    CU_ASSERT(pal != NULL && userData != NULL);
    if (pal == NULL || userData == NULL)
    {
        return;
    }

    info = (TaskInfo*)userData;

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("Task run, threadId %d\n", CurrentThreadId());
#endif
    counter = PAL_AtomicDecrement(info->cbCounter);
    if (counter == 0)
    {
        SetCallbackCompletedEvent(info->test->completeEvent);
    }
}

void EventQueueTaskCallback(PAL_Instance* pal, void* userData)
{
    TaskInfo* info = NULL;
    int32 counter = 0;

    CU_ASSERT(pal != NULL && userData != NULL);
    if (pal == NULL || userData == NULL)
    {
        return;
    }

    info = (TaskInfo*)userData;

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("TaskId %d, threadId %d\n", info->taskId, CurrentThreadId());
#endif
    counter = PAL_AtomicDecrement(info->cbCounter);
    if (counter == 0)
    {
        SetCallbackCompletedEvent(info->test->completeEvent);
    }
}

void WorkerQueueTaskCallback(PAL_Instance* pal, void* userData)
{
    TaskInfo* info = NULL;
    int32 counter = 0;

    CU_ASSERT(pal != NULL && userData != NULL);
    if (pal == NULL || userData == NULL)
    {
        return;
    }

    info = (TaskInfo*)userData;

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("TaskId %d, threadId %d\n", info->taskId, CurrentThreadId());
#endif
    counter = PAL_AtomicDecrement(info->cbCounter);
    if (counter == 0)
    {
        SetCallbackCompletedEvent(info->test->completeEvent);
    }

    PAL_WorkerTaskQueueDestroy(info->pal, info->handle);
}

void TestUiQueueAddTask(void)
{
    volatile int32* counter = NULL;
    int32 i = 0, actual = 0;
    PAL_Instance* pal = NULL;
    Test* test = NULL;
    TaskInfo* taskInfoArr[QUEUE_NUMBER] = {0};

    counter = nsl_malloc(sizeof(int));
    CU_ASSERT_FATAL(counter != NULL);
    *counter = QUEUE_NUMBER;

    test = nsl_malloc(sizeof(Test));
    CU_ASSERT_FATAL(test != NULL);
    nsl_memset(test, 0, sizeof(Test));
    test->completeEvent = CreateCallbackCompletedEvent();
    CU_ASSERT_FATAL(test->completeEvent != NULL);

    pal = PAL_CreateInstance();
    CU_ASSERT_FATAL(pal != NULL);

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("\nMain threadId %d\n", CurrentThreadId());
#endif
    for (i = 0, actual = 0; i < QUEUE_NUMBER; ++i)
    {
        TaskInfo* info = nsl_malloc(sizeof(TaskInfo));
        if (info != NULL)
        {
            nsl_memset(info, 0, sizeof(TaskInfo));
            info->cbCounter = counter;
            info->test = test;
            info->pal = pal;

            //PAL_UiTaskQueueAdd(pal, UiQueueTaskCallback, info);
            taskInfoArr[i] = info;
            ++actual;
        }
    }

    // Check how many tasks were actually scheduled.
    CU_ASSERT(actual == QUEUE_NUMBER);
    WaitForCallbackCompletedEvent(test->completeEvent, WAIT_TIMEOUT);

    CU_ASSERT(*counter == 0);

    for (i = 0; i < QUEUE_NUMBER; ++i)
    {
        if (taskInfoArr[i] != NULL)
        {
            nsl_free(taskInfoArr[i]);
        }
    }

    PAL_Destroy(pal);
    nsl_free((void*)counter);

    DestroyCallbackCompletedEvent(test->completeEvent);
    nsl_free(test);
}

void TestEventQueueAddTask(void)
{
    volatile int32* counter = NULL;
    int32 i = 0, actual = 0;
    PAL_Instance* pal = NULL;
    Test* test = NULL;
    TaskInfo* taskInfoArr[QUEUE_NUMBER] = {0};

    counter = nsl_malloc(sizeof(int));
    CU_ASSERT_FATAL(counter != NULL);
    *counter = QUEUE_NUMBER;

    test = nsl_malloc(sizeof(Test));
    CU_ASSERT_FATAL(test != NULL);
    nsl_memset(test, 0, sizeof(Test));
    test->completeEvent = CreateCallbackCompletedEvent();
    CU_ASSERT_FATAL(test->completeEvent != NULL);

    pal = PAL_CreateInstance();
    CU_ASSERT_FATAL(pal != NULL);

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("\nMain threadId %d\n", CurrentThreadId());
#endif
    for (i = 0, actual = 0; i < QUEUE_NUMBER; ++i)
    {
        TaskInfo* info = nsl_malloc(sizeof(TaskInfo));
        if (info != NULL)
        {
            TaskId tid = 0;

            nsl_memset(info, 0, sizeof(TaskInfo));
            info->cbCounter = counter;
            info->test = test;
            info->pal = pal;

            PAL_EventTaskQueueAdd(pal, EventQueueTaskCallback, info, &tid);
            info->taskId = tid;
            taskInfoArr[i] = info;
            ++actual;
        }
    }

    // Check how many tasks were actually scheduled.
    CU_ASSERT(actual == QUEUE_NUMBER);
    WaitForCallbackCompletedEvent(test->completeEvent, WAIT_TIMEOUT);

    CU_ASSERT(*counter == 0);

    for (i = 0; i < QUEUE_NUMBER; ++i)
    {
        if (taskInfoArr[i] != NULL)
        {
            nsl_free(taskInfoArr[i]);
        }
    }

    PAL_Destroy(pal);
    nsl_free((void*)counter);

    DestroyCallbackCompletedEvent(test->completeEvent);
    nsl_free(test);
}

void TestWorkerQueueAddTask(void)
{
    volatile int32* counter = NULL;
    int32 i = 0, actual = 0;
    PAL_Instance* pal = NULL;
    Test* test = NULL;
    PAL_Error err = PAL_Ok;
    TaskInfo* taskInfoArr[QUEUE_NUMBER] = {0};

    counter = nsl_malloc(sizeof(int));
    CU_ASSERT_FATAL(counter != NULL);
    *counter = QUEUE_NUMBER;

    test = nsl_malloc(sizeof(Test));
    CU_ASSERT_FATAL(test != NULL);
    nsl_memset(test, 0, sizeof(Test));
    test->completeEvent = CreateCallbackCompletedEvent();
    CU_ASSERT_FATAL(test->completeEvent != NULL);

    pal = PAL_CreateInstance();
    CU_ASSERT_FATAL(pal != NULL);

#ifdef UNIT_TEST_VERBOSE_HIGH
    PRINTF("\nMain threadId %d\n", CurrentThreadId());
#endif
    for (i = 0, actual = 0; i < QUEUE_NUMBER; ++i)
    {
        TaskInfo* info = nsl_malloc(sizeof(TaskInfo));
        if (info != NULL)
        {
            TaskId tid = 0;

            nsl_memset(info, 0, sizeof(TaskInfo));
            err = PAL_WorkerTaskQueueCreate(pal, NULL, &(info->handle));
            CU_ASSERT(err == PAL_Ok);

            info->cbCounter = counter;
            info->test = test;
            info->pal = pal;

            PAL_WorkerTaskQueueAdd(pal, info->handle, WorkerQueueTaskCallback, info, &tid);
            info->taskId = tid;
            taskInfoArr[i] = info;
            ++actual;
        }
    }

    // Check how many tasks were actually scheduled.
    CU_ASSERT(actual == QUEUE_NUMBER);
    WaitForCallbackCompletedEvent(test->completeEvent, WAIT_TIMEOUT);

    CU_ASSERT(*counter == 0);

    for (i = 0; i < QUEUE_NUMBER; ++i)
    {
        if (taskInfoArr[i] != NULL)
        {
            nsl_free(taskInfoArr[i]);
        }
    }

    PAL_Destroy(pal);
    nsl_free((void*)counter);

    DestroyCallbackCompletedEvent(test->completeEvent);
    nsl_free(test);
}

/*! Add all your test functions here

@return None
*/
void
TestTaskQueue_AddAllTests( CU_pSuite pTestSuite )
{
    CU_add_test(pTestSuite, "TestUiQueueAddTask", &TestUiQueueAddTask);
    CU_add_test(pTestSuite, "TestEventQueueAddTask", &TestEventQueueAddTask);
    CU_add_test(pTestSuite, "TestWorkerQueueAddTask", &TestWorkerQueueAddTask);
};


/*! Add common initialization code here.

@return 0

@see TestTaskQueue_SuiteCleanup
*/
int
TestTaskQueue_SuiteSetup()
{    
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestTaskQueue_SuiteSetup
*/
int
TestTaskQueue_SuiteCleanup()
{
    return 0;
}

/*! @} */
