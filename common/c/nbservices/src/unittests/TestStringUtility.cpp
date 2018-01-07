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
   @file        TestStringUtility.cpp
   @defgroup    test

   Description: Test StringUtility.

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
#include "TestStringUtility.h"
#include "StringUtility.h"
#include "main.h"

void
TestStringUtilityEscapeSpecialCharacter(void)
{
    string sourceStr("Test/String///+-Utility/Escape//Special//Character/=");
    nbcommon::StringUtility::EscapeSpecialCharactersForBaseName(sourceStr);
    size_t pos = sourceStr.find("/");
    if (pos != string::npos)
    {
        CU_FAIL("Failed to replace string.");
    }
}

void TestStringUtilityReplaceString(void)
{
    string sourceStr("http://dev14dts.navbuilder.nimlbs.net/&fmt=$fmt&vfmt=$vfmt&v=$v");
    string sourceStr2(sourceStr);

    // When matchWord is set, the $vfmt should not be replace when trying to replace $v.
    nbcommon::StringUtility::ReplaceString(sourceStr, "$v", "1.0", true);
    if (sourceStr.find("$vfmt") == string::npos)
    {
        CU_FAIL("Failed to replace string using 'matchWord'\n");
    }

    // If matchWord is set to false, all "$v" should be replaced.
    nbcommon::StringUtility::ReplaceString(sourceStr2, "$v", "1.0", false);
    if (sourceStr2.find("$vfmt") != string::npos)
    {
        CU_FAIL("Failed to replace string using 'matchWord'\n");
    }
}

/*! Add all your test functions here

  @return None
*/
void
TestStringUtility_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestStringUtilityEscapeSpecialCharacter",
                &TestStringUtilityEscapeSpecialCharacter);
    CU_add_test(pTestSuite, "TestStringUtilityReplaceString",
                &TestStringUtilityReplaceString);
};

/*! Add common initialization code here

  @return 0
*/
int
TestStringUtility_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

  @return 0
*/
int
TestStringUtility_SuiteCleanup()
{
    return 0;
}

/*! @} */


