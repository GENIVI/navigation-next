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

    @defgroup TestUnZip_h Unit Tests for UnZip Utility

    Unit Tests for UnZip Utility

    This file contains all unit tests for the UnZip Utility
    */
    /*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testunzip.h"
#include "main.h"
#include "PlatformUtil.h"
#include "palunzip.h"
#include "palfile.h"

#ifdef WINCE
#include "winbase.h"
#endif

#if WINCE || WIN32
#define DIR_SEPARATOR            "\\"
#else
#define DIR_SEPARATOR            "/"
#endif

#if defined(ANDROID_NDK)
#define TEST_FOLDER              "USA-BOS"
#define TEST_FILE                "./mnt/sdcard/test.zip"
#define TEST_DESTINATION         "./mnt/sdcard/dest"
#else
#define TEST_FOLDER              "USA-BOS"
#define TEST_FILE                "test.zip"
#define TEST_DESTINATION         "dest"
#endif

#define EXTRACTED_TEST_PATH      TEST_DESTINATION DIR_SEPARATOR TEST_FOLDER DIR_SEPARATOR
#define EXTRACTED_TEST_FILE1     EXTRACTED_TEST_PATH "001_FOD.PNG"
#define EXTRACTED_TEST_FILE2     EXTRACTED_TEST_PATH "001_FRA.PNG"

// Local Functions ...............................................................................

static void TestUnZipExtractFiles(void)
{
    PAL_Instance *pal = PAL_CreateInstance();

    CU_ASSERT(pal != NULL);

    CU_ASSERT(PAL_FileUnzip(pal, TEST_FILE, TEST_DESTINATION) == PAL_Ok);

    CU_ASSERT(PAL_FileExists(pal, EXTRACTED_TEST_FILE1) == PAL_Ok)

    CU_ASSERT(PAL_FileExists(pal, EXTRACTED_TEST_FILE2) == PAL_Ok)

    PAL_Destroy(pal);
}


/*! Add all your test functions here

@return None
*/
void
TestUnZip_AddAllTests( CU_pSuite pTestSuite )
{
    CU_add_test(pTestSuite, "TestUnZipExtractFiles", &TestUnZipExtractFiles);
};


/*! Add common initialization code here.

@return 0

@see TestUnZip_SuiteCleanup
*/
int
TestUnZip_SuiteSetup()
{
    PAL_Instance *pal = PAL_CreateInstance();

    if (pal)
    {
        // Remove all test data
        PAL_FileRemoveDirectory(pal, TEST_DESTINATION, TRUE);
        PAL_Destroy(pal);
    }

    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestUnZip_SuiteSetup
*/
int
TestUnZip_SuiteCleanup()
{
    PAL_Instance *pal = PAL_CreateInstance();

    if (pal)
    {
        // Remove all test data
        PAL_FileRemoveDirectory(pal, TEST_DESTINATION, TRUE);
        PAL_Destroy(pal);
    }

    return 0;
}

/*! @} */
