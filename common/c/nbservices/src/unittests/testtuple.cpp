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

    @file       testtuple.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
}
#include "CUnit.h"
#include "testtuple.h"
#include "Tuple.h"
#include "smartpointer.h"
#include <string>

using namespace nbcommon;
using namespace std;

// Local Constants ..............................................................................

static const int TEST_INT_VALUE[] = {0, 1, 2, 3, 4, 5};

static const double TEST_DOUBLE_VALUE[] = {0.0, 1.1, 2.2, 3.3, 4.4, 5.5};

static const char* TEST_STRING_VALUE[] =
{
    "string0",
    "string1",
    "string2",
    "string3",
    "string4",
    "string5"
};


// Local Types ..................................................................................


// Test Functions ...............................................................................

static void TestTupleWithInteger(void);
static void TestTupleWithDouble(void);
static void TestTupleWithString(void);
static void TestTupleWithSharedPointer(void);
static void TestTupleWithDifferentType(void);


// Public Functions .............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestTuple_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestTupleWithInteger", &TestTupleWithInteger);
    CU_add_test(pTestSuite, "TestTupleWithDouble", &TestTupleWithDouble);
    CU_add_test(pTestSuite, "TestTupleWithString", &TestTupleWithString);
    CU_add_test(pTestSuite, "TestTupleWithSharedPointer", &TestTupleWithSharedPointer);
    CU_add_test(pTestSuite, "TestTupleWithDifferentType", &TestTupleWithDifferentType);
};

/*! Add common initialization code here

    @return 0
*/
int
TestTuple_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

    @return 0
*/
int
TestTuple_SuiteCleanup()
{
    return 0;
}


// Test Functions ...............................................................................

/*! Test initializing with integers

    @return None. CUnit Asserts get called on failures.
*/
void
TestTupleWithInteger(void)
{
    Tuple<TYPELIST_6(int, int, int, int, int, int)> tupleObject = MakeTuple(TEST_INT_VALUE[0],
                                                                            TEST_INT_VALUE[1],
                                                                            TEST_INT_VALUE[2],
                                                                            TEST_INT_VALUE[3],
                                                                            TEST_INT_VALUE[4],
                                                                            TEST_INT_VALUE[5]);
    CU_ASSERT(GetValue<0>(tupleObject) == TEST_INT_VALUE[0]);
    CU_ASSERT(GetValue<1>(tupleObject) == TEST_INT_VALUE[1]);
    CU_ASSERT(GetValue<2>(tupleObject) == TEST_INT_VALUE[2]);
    CU_ASSERT(GetValue<3>(tupleObject) == TEST_INT_VALUE[3]);
    CU_ASSERT(GetValue<4>(tupleObject) == TEST_INT_VALUE[4]);
    CU_ASSERT(GetValue<5>(tupleObject) == TEST_INT_VALUE[5]);
}

/*! Test initializing with doubles

    @return None. CUnit Asserts get called on failures.
*/
void
TestTupleWithDouble(void)
{
    Tuple<TYPELIST_6(double, double, double, double, double, double)> tupleObject = MakeTuple(TEST_DOUBLE_VALUE[0],
                                                                                              TEST_DOUBLE_VALUE[1],
                                                                                              TEST_DOUBLE_VALUE[2],
                                                                                              TEST_DOUBLE_VALUE[3],
                                                                                              TEST_DOUBLE_VALUE[4],
                                                                                              TEST_DOUBLE_VALUE[5]);
    CU_ASSERT(GetValue<0>(tupleObject) == TEST_DOUBLE_VALUE[0]);
    CU_ASSERT(GetValue<1>(tupleObject) == TEST_DOUBLE_VALUE[1]);
    CU_ASSERT(GetValue<2>(tupleObject) == TEST_DOUBLE_VALUE[2]);
    CU_ASSERT(GetValue<3>(tupleObject) == TEST_DOUBLE_VALUE[3]);
    CU_ASSERT(GetValue<4>(tupleObject) == TEST_DOUBLE_VALUE[4]);
    CU_ASSERT(GetValue<5>(tupleObject) == TEST_DOUBLE_VALUE[5]);
}

/*! Test initializing with strings

    @return None. CUnit Asserts get called on failures.
*/
void
TestTupleWithString(void)
{
    Tuple<TYPELIST_6(string, string, string, string, string, string)> tupleObject = MakeTuple(string(TEST_STRING_VALUE[0]),
                                                                                              string(TEST_STRING_VALUE[1]),
                                                                                              string(TEST_STRING_VALUE[2]),
                                                                                              string(TEST_STRING_VALUE[3]),
                                                                                              string(TEST_STRING_VALUE[4]),
                                                                                              string(TEST_STRING_VALUE[5]));
    CU_ASSERT(GetValue<0>(tupleObject) == TEST_STRING_VALUE[0]);
    CU_ASSERT(GetValue<1>(tupleObject) == TEST_STRING_VALUE[1]);
    CU_ASSERT(GetValue<2>(tupleObject) == TEST_STRING_VALUE[2]);
    CU_ASSERT(GetValue<3>(tupleObject) == TEST_STRING_VALUE[3]);
    CU_ASSERT(GetValue<4>(tupleObject) == TEST_STRING_VALUE[4]);
    CU_ASSERT(GetValue<5>(tupleObject) == TEST_STRING_VALUE[5]);
}

/*! Test initializing with shared pointers of strings

    @return None. CUnit Asserts get called on failures.
*/
void
TestTupleWithSharedPointer(void)
{
    shared_ptr<string> stringPointer0(new string(TEST_STRING_VALUE[0]));
    if (!stringPointer0)
    {
        CU_FAIL("Out of memory when allocated string 0");
        return;
    }

    shared_ptr<string> stringPointer1(new string(TEST_STRING_VALUE[1]));
    if (!stringPointer1)
    {
        CU_FAIL("Out of memory when allocated string 1");
        return;
    }

    shared_ptr<string> stringPointer2(new string(TEST_STRING_VALUE[2]));
    if (!stringPointer2)
    {
        CU_FAIL("Out of memory when allocated string 2");
        return;
    }

    Tuple<TYPELIST_3(shared_ptr<string>, shared_ptr<string>, shared_ptr<string>)> tupleObject = MakeTuple(stringPointer0,
                                                                                                          stringPointer1,
                                                                                                          stringPointer2);
    CU_ASSERT(GetValue<0>(tupleObject) == stringPointer0);
    CU_ASSERT(GetValue<1>(tupleObject) == stringPointer1);
    CU_ASSERT(GetValue<2>(tupleObject) == stringPointer2);
}

/*! Test initializing with different types

    @return None. CUnit Asserts get called on failures.
*/
void
TestTupleWithDifferentType(void)
{
    shared_ptr<string> stringPointer0(new string(TEST_STRING_VALUE[0]));
    if (!stringPointer0)
    {
        CU_FAIL("Out of memory when allocated string 0");
        return;
    }
    string string1(TEST_STRING_VALUE[1]);
    Tuple<TYPELIST_6(shared_ptr<string>, string*, int, double, const int*, const double*)> tupleObject = MakeTuple(stringPointer0,
                                                                                                                   &string1,
                                                                                                                   TEST_INT_VALUE[2],
                                                                                                                   TEST_DOUBLE_VALUE[3],
                                                                                                                   &(TEST_INT_VALUE[4]),
                                                                                                                   &(TEST_DOUBLE_VALUE[5]));
    CU_ASSERT(GetValue<0>(tupleObject) == stringPointer0);
    CU_ASSERT(GetValue<1>(tupleObject) == &string1);
    CU_ASSERT(GetValue<2>(tupleObject) == TEST_INT_VALUE[2]);
    CU_ASSERT(GetValue<3>(tupleObject) == TEST_DOUBLE_VALUE[3]);
    CU_ASSERT(GetValue<4>(tupleObject) == &(TEST_INT_VALUE[4]));
    CU_ASSERT(GetValue<5>(tupleObject) == &(TEST_DOUBLE_VALUE[5]));
}

/*! @} */
