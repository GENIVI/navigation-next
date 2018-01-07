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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "png_test.h"
#include "nbrepngcodec.h"
#include "nbreimage.h"
#include <Windows.h>
#include <string.h>
#include "utility.h"

static int 
Init(void)
{
    return 0;
}

static int 
CleanUp(void)
{
    return 0;
}

static void 
TestLoad(const char* file, uint32 width, uint32 height, NBRE_PixelFormat format)
{
    char currentPath[MAX_PATH + 1];
    GetCurrentDirectoryA(MAX_PATH + 1, currentPath);
    char path[MAX_PATH + 1];
    sprintf_s(path, MAX_PATH + 1, "%s\\%s", currentPath, file);

    PAL_Config pc = {0};
    PAL_Instance* pal = PAL_Create(&pc);

    NBRE_PngCodec loader(*pal);
    loader.SetSource(path, 0);
    NBRE_Image* image = loader.Image();
    CU_ASSERT(image != NULL);
    if(image != NULL)
    {
        CU_ASSERT(image->GetImageData() != NULL);
        CU_ASSERT(image->Width() == width);
        CU_ASSERT(image->Height() == height);
        CU_ASSERT(image->Format() == format);
    }
    NBRE_DELETE image;
    PAL_Destroy(pal);
}

void 
TestPngCodec(void)
{   
    TestLoad("greyscale.png", 256, 256, NBRE_PF_A8);
    TestLoad("rgb.png", 32, 32, NBRE_PF_R8G8B8);
    TestLoad("rgba.png", 32, 32, NBRE_PF_R8G8B8A8);
}

PngTest::PngTest():NBRE_TestSuite("png_test", Init, CleanUp)
{
    AddTestCase(new NBRE_TestCase("Test_PngCodec", TestPngCodec));    
}
