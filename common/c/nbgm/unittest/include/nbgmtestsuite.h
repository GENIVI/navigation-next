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

    @file     nbgmtestsuite.h
    */
    /*
    (C) Copyright 2012 by TCS, Inc.                

    The information contained herein is confidential, proprietary 
    to TCS, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#ifndef _NBRE_TEST_SUITE_
#define _NBRE_TEST_SUITE_
#include "CUnit.h"
#include "Automated.h"
#include "Basic.h"
#include "TestRun.h"
#include "palstdlib.h"
#include <vector>

typedef void (*SuiteTestFunc)();

struct NBRE_TestCase
{
    NBRE_TestCase(const char* name, SuiteTestFunc adderFunc)
    {
        nsl_strcpy(mCaseName, name);
        mAdderFunc = adderFunc;
    }
    char mCaseName[255];
    SuiteTestFunc mAdderFunc;
};

class NBRE_TestSuite
{
public:
    typedef std::vector<NBRE_TestCase*> TestCaseList;
public:
    NBRE_TestSuite(const char* groupName, CU_InitializeFunc setupFunc, CU_CleanupFunc cleanupFunc)
        :mSetupFunc(setupFunc), mCleanupFunc(cleanupFunc)
    {
        nsl_strcpy(mSuiteName, groupName);
    }
    virtual ~NBRE_TestSuite()
    {
        for(size_t i=0; i<mCaseList.size(); ++i)
        {
            delete mCaseList[i];
        }
    }

public:
    void AddTestCase(NBRE_TestCase* testCase)
    {
        mCaseList.push_back(testCase);
    }
    const TestCaseList& GetTestCaseList() const
    {
        return mCaseList;
    }

    void AddToCUint()
    {
        CU_pSuite suite = CU_add_suite(GetName(), GetSetupFunc(), GetCleanupFunc());
        for(size_t i=0; i<mCaseList.size(); ++i)
        {
            NBRE_TestCase* testcase = mCaseList[i];
            CU_add_test(suite, testcase->mCaseName, testcase->mAdderFunc);
        }
    }

    void Merge(const NBRE_TestSuite& suite)
    {
        nbre_assert( nsl_strcmp(mSuiteName, suite.mSuiteName) == 0);
        nbre_assert( mSetupFunc == suite.mSetupFunc);
        nbre_assert( mCleanupFunc == suite.mCleanupFunc);
        mCaseList.insert(mCaseList.end(), suite.mCaseList.begin(), suite.mCaseList.end());
    }
    const char* GetName() const {return mSuiteName;}
    CU_InitializeFunc GetSetupFunc() const { return mSetupFunc;}
    CU_CleanupFunc GetCleanupFunc() const {return mCleanupFunc;}

private:
    char mSuiteName[255];
    CU_InitializeFunc mSetupFunc;
    CU_CleanupFunc mCleanupFunc;

    TestCaseList mCaseList;
};
#endif
