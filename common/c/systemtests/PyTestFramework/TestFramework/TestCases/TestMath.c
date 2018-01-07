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

#include "TestMath.h"

#define ADD_TEST(pTestSuite, methodbname)                       \
    do {                                                        \
        CU_add_test(pTestSuite, #methodbname, &methodbname);    \
    } while (0)

static void TestAddition();
static void Testsubtraction();


void TestMath_AddTests(CU_pSuite pTestSuite)
{
    ADD_TEST(pTestSuite, TestAddition);
    ADD_TEST(pTestSuite, Testsubtraction);
}

// Cleanup for suite
int TestMath_SuiteSetup(void)
{
    return 0;
}

int TestMath_SuiteCleanup(void)
{
    return 0;
}

int add(int a, int b)
{
    return a + b;
}

void TestAddition()
{
    CU_ASSERT_EQUAL(2, add(1, 1));
    CU_ASSERT_NOT_EQUAL(3, add(1, 1));
}

int sub(int a, int b)
{
    return a-b;
}

void Testsubtraction()
{
    CU_ASSERT_EQUAL(0, sub(1,1));
    CU_ASSERT_NOT_EQUAL(1, sub(1,1));
}
