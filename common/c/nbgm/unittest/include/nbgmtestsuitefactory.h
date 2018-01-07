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

    @file     nbgmtestsuitefactory.h
    @date     03/20/2012
    @defgroup Common Utility Functions

    Function declarations for creating Unit test frame used for NBGM test

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
#ifndef _NBRE_TEST_SUITE_FACTORY_H_
#define _NBRE_TEST_SUITE_FACTORY_H_
#include <vector>

class NBRE_TestSuite;
struct NBRE_TestCase;

class NBRE_TestSuiteFactory
{
public:
    typedef std::vector<NBRE_TestSuite*> TestSuiteList;

public:
    NBRE_TestSuiteFactory();
    ~NBRE_TestSuiteFactory();

public:
    NBRE_TestSuite* GetTestSuite(const char* suiteName);
    NBRE_TestCase* GetTestCase(const char* suiteName, const char* caseName);

private:
    void SetUp();

private:
    void AddTestSuite(NBRE_TestSuite* suite);
    void AddTestCase(NBRE_TestCase* suit);
private:
    TestSuiteList mTestSuiteList;
};

#endif

