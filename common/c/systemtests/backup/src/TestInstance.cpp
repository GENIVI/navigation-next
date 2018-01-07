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
   @file        TestInstance.cpp
   @defgroup

   Description: Implementation of TestInstance.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

#include "CUnit.h"

extern "C"
{
#include "testnetwork.h"
#include "palclock.h"
#include "main.h"
#include "platformutil.h"
#include "palmath.h"
#include "paltaskqueue.h"
#include "nbhybridmanager.h"
#include "palfile.h"
}

#include "TestInstance.h"

TestInstance::TestInstance() :
        m_pal(NULL),
        m_context(NULL),
        m_initialized(false),
        m_completeEvent(NULL),
        m_functor(NULL)
{
    Initialize();
}

TestInstance::~TestInstance()
{
    if (m_completeEvent)
    {
        DestroyCallbackCompletedEvent(m_completeEvent);
        m_completeEvent = NULL;
    }

    if (m_pal)
    {
        PAL_Destroy(m_pal);
        m_pal = NULL;
    }

}

NB_Error TestInstance::Initialize()
{
    NB_Error error = NE_NOTINIT;
    do
    {
        m_initialized = (CreatePalAndContextMultiThread(&m_pal, &m_context) != 0);
        CU_ASSERT(m_initialized);
        if (!m_initialized)
        {
            break;
        }

        //enable onboard/offboard mode
        if(Test_OptionsGet()->onBoard == OnBoard)
        {
            NB_HybridManagerSetMapConfig(m_context, Test_OptionsGet()->mapConfig);
        }
        NB_HybridManagerEnableOnboard(m_context, (Test_OptionsGet()->onBoard == OnBoard));

        m_completeEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(m_completeEvent);
        if (!m_completeEvent)
        {
            break;
        }
        error = NE_OK;
    } while (0);
    return error;
}

bool TestInstance::WaitEvent(uint32 timeout)
{
    bool result = true;
    if (m_completeEvent && timeout > 0)
    {
        result = WaitForCallbackCompletedEvent(m_completeEvent, timeout) == TRUE ? true : false;
    }
    return result;
}

void TestInstance::ScheduleFinishTest()
{
    /* We need to schedule releasing test resources to avoid some potential scenarios. For
       example: network layer uses PAL event task queue to call user callbacks in the
       context of PAL thread. When network connection is destroyed from NB_ContextDestroy(),
       it removes all its tasks from the queue. But one of the tasks can be already popped
       from the queue and started. To cancel the task execution we need to terminate the PAL
       thread(which is unacceptable). So we schedule all resources releasing which means
       adding the new task to the queue end. When resources are released all related tasks
       should be already completed.
    */
    if (m_initialized)
    {
        TaskId tid = 0;
        m_initialized = false;
        if (m_functor)
        {
            delete m_functor;
            m_functor = NULL;
        }
        m_functor = new TestTask<TestInstance>(this, &TestInstance::FinishTest, NULL);
        PAL_EventTaskQueueAdd(m_pal, EventTaskCallback, this, &tid);
    }
}

void TestInstance::EventTaskCallback(PAL_Instance * pal, void * userData)
{
    TestInstance* pThis = (TestInstance*) userData;
    if (pThis && pThis->m_functor)
    {
        pThis->m_functor->ExecuteTask();
    }
}

void TestInstance::FinishTest()
{
    FinishTestPrivate();
    NB_Error err = NE_OK;

    if (m_context)
    {
        err = NB_ContextDestroy(m_context);
        m_context = NULL;
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    if (m_functor)
    {
        delete m_functor;
        m_functor = NULL;
    }
    SetCallbackCompletedEvent(m_completeEvent);
}

void TestInstance::CallTestFunction(void)
{
    if (!m_initialized)
    {
        CU_FAIL("Test is not initialized.");
        SetCallbackCompletedEvent(m_completeEvent);
        return;
    }

    if (PAL_IsPalThread(m_pal) == FALSE)
    {
        // Schedule test function call
        TaskId tid = 0;
        PAL_EventTaskQueueAdd(m_pal, EventTaskCallback, this, &tid);
    }
    else
    {
        if (m_functor)
        {
            m_functor->ExecuteCallback();
        }
    }
}

/*! @} */
