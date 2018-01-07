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

    @defgroup TestTimer_h Unit Tests for PAL Timer

    Unit Tests for PAL Timer

    This file contains all unit tests for the PAL timer component
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

#include "testtimer.h"
#include "main.h"
#include "palclock.h"
#include "paltimer.h"
#include "PlatformUtil.h"

typedef struct TimerTestResult
{
    void *expireEvent;

    int isExpired;
    nb_unixTime expireTime;
    int isExpireCallbackCalled;
    int isCancelCallbackCalled;
} TimerTestResult;

static void TimerCallbackFunc(PAL_Instance* pal, void *userData,
        PAL_TimerCBReason reason)
{
    TimerTestResult *pTestResult = (TimerTestResult *)userData;

    CU_ASSERT(pTestResult != NULL);
    CU_ASSERT((reason == PTCBR_TimerFired) || (reason == PTCBR_TimerCanceled));

    if (reason == PTCBR_TimerFired)
    {
        pTestResult->isExpired = TRUE;
        pTestResult->expireTime = PAL_ClockGetUnixTime();
        pTestResult->isExpireCallbackCalled = TRUE;
		SetCallbackCompletedEvent(pTestResult->expireEvent);
	}
    else
    {
        pTestResult->isCancelCallbackCalled = TRUE;
    }
}

static void TestTimerSet(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    const int interval = 1000; /* 1000 ms, or 1 seconds */
    TimerTestResult testResult;

    testResult.expireEvent = CreateCallbackCompletedEvent();
    testResult.isExpired = FALSE;
    testResult.isExpireCallbackCalled= FALSE;
    testResult.isCancelCallbackCalled= FALSE;

    CU_ASSERT(PAL_TimerSet(pal, interval, TimerCallbackFunc, &testResult) == PAL_Ok);

    /* check before timer expires */
    CU_ASSERT_FALSE(WaitForCallbackCompletedEvent(testResult.expireEvent, interval/2));
    CU_ASSERT_FALSE(testResult.isExpired);
    CU_ASSERT_FALSE(testResult.isExpireCallbackCalled);
    CU_ASSERT_FALSE(testResult.isCancelCallbackCalled);

    /* check after timer expires */
    CU_ASSERT(WaitForCallbackCompletedEvent(testResult.expireEvent, interval));
    CU_ASSERT(testResult.isExpired);
    CU_ASSERT(testResult.isExpireCallbackCalled);
    CU_ASSERT_FALSE(testResult.isCancelCallbackCalled);

    CU_ASSERT(PAL_TimerCancel(pal, TimerCallbackFunc, NULL) == PAL_Ok);
    DestroyCallbackCompletedEvent(testResult.expireEvent);
	PAL_Destroy(pal);
}

static void TestTimerCancel(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    const int interval = 1000; /* 1000 ms, or 1 seconds */
    TimerTestResult testResult;

    testResult.expireEvent = CreateCallbackCompletedEvent();
    testResult.isExpired = FALSE;
    testResult.isExpireCallbackCalled= FALSE;
    testResult.isCancelCallbackCalled= FALSE;

    CU_ASSERT(PAL_TimerSet(pal, interval, TimerCallbackFunc, &testResult) == PAL_Ok);

    /* check before timer expires */
    CU_ASSERT_FALSE(WaitForCallbackCompletedEvent(testResult.expireEvent, interval/2));
    CU_ASSERT_FALSE(testResult.isExpired);
    CU_ASSERT_FALSE(testResult.isExpireCallbackCalled);
    CU_ASSERT_FALSE(testResult.isCancelCallbackCalled);

    /* cancel timer */
    CU_ASSERT(PAL_TimerCancel(pal, TimerCallbackFunc, &testResult) == PAL_Ok);

    /* make sure timer is indeed cancelled */
    CU_ASSERT_FALSE(WaitForCallbackCompletedEvent(testResult.expireEvent, interval));
    CU_ASSERT_FALSE(testResult.isExpired);
    CU_ASSERT_FALSE(testResult.isExpireCallbackCalled);
    CU_ASSERT(testResult.isCancelCallbackCalled);

    DestroyCallbackCompletedEvent(testResult.expireEvent);
	PAL_Destroy(pal);
}

static void TimerCancelInCallbackFunc(PAL_Instance* pal, void *userData,
        PAL_TimerCBReason reason)
{
    TimerTestResult *pTestResult = (TimerTestResult *)userData;

    CU_ASSERT(pTestResult != NULL);
    CU_ASSERT((reason == PTCBR_TimerFired) || (reason == PTCBR_TimerCanceled));

    if (reason == PTCBR_TimerFired)
    {
        pTestResult->isExpired = TRUE;
        pTestResult->expireTime = PAL_ClockGetUnixTime();
        pTestResult->isExpireCallbackCalled = TRUE;

        // try cancel the current timer
        CU_ASSERT(PAL_TimerCancel(pal, TimerCancelInCallbackFunc, userData) == PAL_Ok);

		SetCallbackCompletedEvent(pTestResult->expireEvent);
	}
    else
    {
        pTestResult->isCancelCallbackCalled = TRUE;
    }
}
static void TestTimerCancelInCallback(void)
{
	PAL_Instance *pal = PAL_CreateInstance();
    const int interval = 1000; /* 1000 ms, or 1 seconds */
    TimerTestResult testResult;

    testResult.expireEvent = CreateCallbackCompletedEvent();
    testResult.isExpired = FALSE;
    testResult.isExpireCallbackCalled= FALSE;
    testResult.isCancelCallbackCalled= FALSE;

    CU_ASSERT(PAL_TimerSet(pal, interval, TimerCancelInCallbackFunc, &testResult) == PAL_Ok);

    /* check before timer expires */
    CU_ASSERT_FALSE(WaitForCallbackCompletedEvent(testResult.expireEvent, interval/2));
    CU_ASSERT_FALSE(testResult.isExpired);
    CU_ASSERT_FALSE(testResult.isExpireCallbackCalled);
    CU_ASSERT_FALSE(testResult.isCancelCallbackCalled);

    /* check after timer expires */
    CU_ASSERT(WaitForCallbackCompletedEvent(testResult.expireEvent, interval));
    CU_ASSERT(testResult.isExpired);
    CU_ASSERT(testResult.isExpireCallbackCalled);
    CU_ASSERT_FALSE(testResult.isCancelCallbackCalled);

    CU_ASSERT(PAL_TimerCancel(pal, TimerCallbackFunc, NULL) == PAL_Ok);
    DestroyCallbackCompletedEvent(testResult.expireEvent);
	PAL_Destroy(pal);
}

/*! Add all your test functions here

@return None
*/
void
TestTimer_AddAllTests( CU_pSuite pTestSuite )
{
    CU_add_test(pTestSuite, "TestTimerSet", &TestTimerSet);
    CU_add_test(pTestSuite, "TestTimerCancel", &TestTimerCancel);
    CU_add_test(pTestSuite, "TestTimerCancelInCallback", &TestTimerCancelInCallback);
};


/*! Add common initialization code here.

@return 0

@see TestTimer_SuiteCleanup
*/
int
TestTimer_SuiteSetup()
{    
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestTimer_SuiteSetup
*/
int
TestTimer_SuiteCleanup()
{
    return 0;
}

/*! @} */
