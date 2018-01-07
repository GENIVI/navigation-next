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

    @defgroup TestClock_h Unit Tests for PAL Clock

    Unit Tests for PAL Clock

    This file contains all unit tests for the PAL clock component
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

#include "testclock.h"
#include <stdio.h>
#include "main.h"
#include "palclock.h"
#include "palstdlib.h"
#include "PlatformUtil.h"
#ifdef WINCE
#include "winbase.h"
#endif

/*! Add all your test functions here
@return None
*/

static void TestGetGPSTime(void)
{
    nb_unixTime gpsTime = 0;

    gpsTime = PAL_ClockGetGPSTime();

    CU_ASSERT(gpsTime);
}

static void TestGetUnixTime(void)
{
    nb_unixTime unixTime = 0;
    unixTime = PAL_ClockGetUnixTime();

    CU_ASSERT(unixTime);
}

static void TestGetDateTime(void)
{
    PAL_ClockDateTime currentTime;

    nsl_memset(&currentTime, 0, sizeof(currentTime));
    CU_ASSERT(PAL_ClockGetDateTime(&currentTime) == PAL_Ok);
    CU_ASSERT(currentTime.year >= 2009);
    CU_ASSERT(currentTime.month >= 1);
    CU_ASSERT(currentTime.month <= 12);
    CU_ASSERT(currentTime.day <= 31);
    CU_ASSERT(currentTime.dayOfWeek <= 6);
    CU_ASSERT(currentTime.hour <= 23);
    CU_ASSERT(currentTime.minute <= 59);
    CU_ASSERT(currentTime.second <= 59);
    CU_ASSERT(currentTime.milliseconds <= 999);

#ifdef UNIT_TEST_VERBOSE_HIGH
    {
        char buffer[128];

        nsl_snprintf(buffer, sizeof(buffer), " <Current Time: %d/%d/%d %d:%d:%d.%d> ",
                currentTime.year, currentTime.month, currentTime.day,
                currentTime.hour, currentTime.minute, currentTime.second,
                currentTime.milliseconds);
        PRINTF(buffer);
    }
#endif
}

void
TestClock_AddAllTests( CU_pSuite pTestSuite )
{
    CU_add_test(pTestSuite, "TestGetGPSTime",       &TestGetGPSTime);
    CU_add_test(pTestSuite, "TestGetUnixTime",      &TestGetUnixTime);
    CU_add_test(pTestSuite, "TestGetDateTime",      &TestGetDateTime);
};

/*! Add common initialization code here.

@return 0

@see TestClock_SuiteCleanup
*/
int
TestClock_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestClock_SuiteSetup
*/
int
TestClock_SuiteCleanup()
{
    return 0;
}

/*! @} */
