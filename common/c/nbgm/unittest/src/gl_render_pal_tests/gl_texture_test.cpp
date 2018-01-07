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

#include "gl_texture_test.h"
#include "nbretexture.h"
#include "nbrepngtextureimage.h"
#include "winutility.h"
#include "windows.h"
#include "private\nbregltexture.h"
#include "nbrecontext.h"

int
Init(void)
{
    return 0;
}

int
DeInit(void)
{
    return 0;
}

static NBRE_Context* CreateContent()
{
    NBRE_Context* context = NBRE_NEW NBRE_Context;
    nsl_memset(&context, 0, sizeof(NBRE_Context));

    PAL_Config pc = {0};
    PAL_Instance* pal = PAL_Create(&pc);
    context->mPalInstance = pal;

    return context;
}

static void DestroyContent(NBRE_Context* context)
{
    if (context != NULL)
    {
        PAL_Destroy(context->mPalInstance);
        NBRE_DELETE context;
    }
}

void
CreateGeneralTextureFromFile(const char* filename)
{
    char path[MAX_PATH + 1];
    char *folder = GetExePath();
    sprintf_s(path, MAX_PATH + 1, "%s\\%s", folder, filename);

    NBRE_Context* context = CreateContent();

    NBRE_PngTextureImage *png = NBRE_NEW NBRE_PngTextureImage(*context->mPalInstance, path, 0);
    NBRE_GLTexture *texture = NBRE_NEW NBRE_GLTexture(NULL, png, NBRE_Texture::TT_2D, FALSE, filename);
    CU_ASSERT(texture->GetName() == filename);
    CU_ASSERT(texture->GetState() == NBRE_Texture::TS_UNLOADED);
    CU_ASSERT(texture->GetType() == NBRE_Texture::TT_2D);
    CU_ASSERT(texture->IsUsingMipmap() == FALSE);

    PAL_Error err = texture->Load();
    CU_ASSERT(err = PAL_Failed); // GL context unprepared
    CU_ASSERT(texture->GetID() == 0);
    CU_ASSERT(texture->GetTextureInfo().mFormat == NBRE_PF_R8G8B8A8);
    CU_ASSERT(texture->GetTextureInfo().mHeight == 32);
    CU_ASSERT(texture->GetTextureInfo().mWidth == 32);
    CU_ASSERT(texture->GetWidth() == 32);
    CU_ASSERT(texture->GetHeight() == 32);
    CU_ASSERT(texture->GetSrcPixelFormat() == GL_RGBA);
    CU_ASSERT(texture->GetTexPixelFormat() == GL_RGBA8);
    CU_ASSERT(texture->GetPixelType() == GL_UNSIGNED_BYTE);
    CU_ASSERT(texture->GetTextureTarget() == GL_TEXTURE_2D);
    NBRE_DELETE texture;
    delete[] folder;
    DestroyContent(context); 
}

void
CreateGeneralTextureFromImages()
{
    // Create Images;
    uint8 *buffer = NBRE_NEW uint8[64*64*3];
    NBRE_Image *image = NBRE_NEW NBRE_Image(64, 64, NBRE_PF_R8G8B8, buffer);
    NBRE_Image **images = NBRE_NEW NBRE_Image*[1];
    images[0] = image;

    NBRE_Context* context = CreateContent();

    NBRE_GLTexture *texture = NBRE_NEW NBRE_GLTexture(NULL, images, 1,  1, NBRE_Texture::TT_2D, TRUE, "TestTexture");
    CU_ASSERT(texture->GetName() == "TestTexture");
    CU_ASSERT(texture->GetID() == 0);  // GL context unprepared
    CU_ASSERT(texture->GetState() == NBRE_Texture::TS_UNLOADED);
    CU_ASSERT(texture->GetType() == NBRE_Texture::TT_2D);
    CU_ASSERT(texture->IsUsingMipmap() == TRUE);
    CU_ASSERT(texture->GetTextureInfo().mFormat == NBRE_PF_R8G8B8);
    CU_ASSERT(texture->GetTextureInfo().mHeight == 64);
    CU_ASSERT(texture->GetTextureInfo().mWidth == 64);
    CU_ASSERT(texture->GetWidth() == 64);
    CU_ASSERT(texture->GetHeight() == 64);
    CU_ASSERT(texture->GetSrcPixelFormat() == GL_RGB);
    CU_ASSERT(texture->GetTexPixelFormat() == GL_RGB8);
    CU_ASSERT(texture->GetPixelType() == GL_UNSIGNED_BYTE);
    CU_ASSERT(texture->GetTextureTarget() == GL_TEXTURE_2D);
    NBRE_DELETE texture;
    DestroyContent(context); 
}

void
CreateRenderTexture()
{
    NBRE_ImageInfo targetInfo = { 1024, 768, NBRE_PF_R8G8B8A8 };

    NBRE_Context* context = CreateContent();

    NBRE_GLTexture *texture = NBRE_NEW NBRE_GLTexture(NULL, &targetInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE, "RenderTexture");
    CU_ASSERT(texture->GetName() == "RenderTexture");
    CU_ASSERT(texture->GetID() == 0); // GL context unprepared
    CU_ASSERT(texture->GetState() == NBRE_Texture::TS_UNLOADED);
    CU_ASSERT(texture->GetType() == NBRE_Texture::TT_2D);
    CU_ASSERT(texture->IsUsingMipmap() == FALSE);
    CU_ASSERT(texture->GetTextureInfo().mFormat == NBRE_PF_R8G8B8A8);
    CU_ASSERT(texture->GetTextureInfo().mWidth == 1024);
    CU_ASSERT(texture->GetTextureInfo().mHeight == 768);
    CU_ASSERT(texture->GetWidth() == 1024);
    CU_ASSERT(texture->GetHeight() == 768);
    CU_ASSERT(texture->GetSrcPixelFormat() == GL_RGBA);
    CU_ASSERT(texture->GetTexPixelFormat() == GL_RGBA8);
    CU_ASSERT(texture->GetPixelType() == GL_UNSIGNED_BYTE);
    CU_ASSERT(texture->GetTextureTarget() == GL_TEXTURE_2D);
    NBRE_DELETE texture;
    DestroyContent(context); 
}

void
CreateTexture()
{
    CreateGeneralTextureFromFile("rgba.png");
    CreateGeneralTextureFromImages();
    CreateRenderTexture();
}

GLTextureTest::GLTextureTest():NBRE_TestSuite("gl_texture_test", Init, DeInit)
{
    AddTestCase(new NBRE_TestCase("Test_GL_Texture_Create", CreateTexture));
}
