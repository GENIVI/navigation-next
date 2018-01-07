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

    @defgroup  Unit Tests for PAL ImageBlender

    Unit Tests for PAL ImageBlender

    This file contains all unit tests for the PAL ImageBlender component
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testimageblender.h"
#include "palimageblender.h"
#include "PlatformUtil.h"
#include "palfile.h"

/*! Add all your test functions here
@return None
*/

#define BASE_IMAGE    "native/connect-pc-64.png"
#define OVERLAY_IMAGE "native/arrow-east-64.png"
#define TARGET_IMAGE  "native/logoff.png"

static void TestImageBlender(void)
{
    PAL_Instance* pal                 = NULL;
    PAL_File*     file                = NULL;
    PAL_Image*    baseImage           = NULL;
    PAL_Image*    overlayImage        = NULL;
    PAL_Image*    targetImage         = NULL;
    uint32        bytesWritten        = 0;
    PAL_Error     err                 = PAL_Failed;

    pal = PAL_CreateInstance();

    baseImage = (PAL_Image*)nsl_malloc(sizeof(PAL_Image));
    CU_ASSERT_FATAL(baseImage != NULL);
    nsl_memset(baseImage, 0, sizeof(PAL_Image));

    overlayImage = (PAL_Image*)nsl_malloc(sizeof(PAL_Image));
    CU_ASSERT_FATAL(overlayImage != NULL);
    nsl_memset(overlayImage, 0, sizeof(PAL_Image));

    CU_ASSERT(PAL_FileLoadFile(pal, BASE_IMAGE, &baseImage->imageBuffer, &baseImage->imageBufferSize) == PAL_Ok);
    baseImage->width  = 64;
    baseImage->height = 64;
    baseImage->format = PAL_IF_Unknown;

    CU_ASSERT(PAL_FileLoadFile(pal, OVERLAY_IMAGE, &overlayImage->imageBuffer, &overlayImage->imageBufferSize) == PAL_Ok);
    overlayImage->width  = 64;
    overlayImage->height = 64;
    overlayImage->format = PAL_IF_Unknown;

    err = PAL_ImageBlend(baseImage, overlayImage, 10, 10, PAL_IF_ARGB32, &targetImage);
    CU_ASSERT(err == PAL_Ok);
    CU_ASSERT_FATAL(targetImage != NULL);

    CU_ASSERT(PAL_FileOpen(pal, TARGET_IMAGE, PFM_Create, &file) == PAL_Ok);
    CU_ASSERT(PAL_FileWrite(file, targetImage->imageBuffer, targetImage->imageBufferSize, &bytesWritten) == PAL_Ok);
    PAL_FileClose(file);

    nsl_free(baseImage->imageBuffer);
    baseImage->imageBuffer = NULL;
    nsl_free(overlayImage->imageBuffer);
    overlayImage->imageBuffer = NULL;
    nsl_free(targetImage->imageBuffer);
    targetImage->imageBuffer = NULL;

    nsl_free(baseImage);
    baseImage = NULL;
    nsl_free(overlayImage);
    overlayImage = NULL;
    nsl_free(targetImage);
    targetImage = NULL;

    PAL_Destroy(pal);
}

void
TestImageBlender_AddAllTests( CU_pSuite pTestSuite )
{
    CU_add_test(pTestSuite, "TestImageBlender", &TestImageBlender);
};

/*! Add common initialization code here.

@return 0

@see TestClock_SuiteCleanup
*/
int
TestImageBlender_SuiteSetup()
{
    PAL_Instance *pal = PAL_CreateInstance();

    if (pal)
    {
        // Remove all test data
        PAL_FileRemove(pal, TARGET_IMAGE);
        PAL_Destroy(pal);
    }

    return 0;
}

/*! Add common cleanup code here.

@return 0

@see TestClock_SuiteSetup
*/
int
TestImageBlender_SuiteCleanup()
{
    PAL_Instance *pal = PAL_CreateInstance();

    if (pal)
    {
        // Remove all test data
        PAL_FileRemove(pal, TARGET_IMAGE);
        PAL_Destroy(pal);
    }

    return 0;
}

/*! @} */
