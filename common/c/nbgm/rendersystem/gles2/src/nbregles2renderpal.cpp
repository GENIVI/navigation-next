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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbregles2renderpal.h"
#include "nbregles2hardwareindexbuffer.h"
#include "nbregles2hardwarevertexbuffer.h"
#include "nbregles2renderwindow.h"
#include "nbregles2vertexdeclaration.h"
#include "nbregles2texture.h"
#include "nbregles2rendertexture.h"
#include "nbregles2gpuprogramfactory.h"
#include "nbreitextureimage.h"
#include "nbrematerial.h"
#include "nbretypeconvert.h"
#include "nbrelog.h"
#include "nbregles2common.h"
#include "palgl2.h"
#include "nbregles2ssaoprogram.h"

#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLenum
ToGLBlendFunction(NBRE_BlendFunction f)
{
    switch(f)
    {
    case NBRE_BF_SRC_ALPHA:
        return GL_SRC_ALPHA;

    case NBRE_BF_ONE_MINUS_SRC_ALPHA:
        return GL_ONE_MINUS_SRC_ALPHA;

    case NBRE_BF_ZERO:
        return GL_ZERO;

    case NBRE_BF_ONE:
        return GL_ONE;

    case NBRE_BF_SRC_COLOR:
        return GL_SRC_COLOR;

    case NBRE_BF_ONE_MINUS_SRC_COLOR:
        return GL_ONE_MINUS_SRC_COLOR;

    case NBRE_BF_DST_COLOR:
        return GL_DST_COLOR;

    case NBRE_BF_ONE_MINUS_DST_COLOR:
        return GL_ONE_MINUS_DST_COLOR;

    case NBRE_BF_DST_ALPHA:
        return GL_DST_ALPHA;

    case NBRE_BF_ONE_MINUS_DST_ALPHA:
        return GL_ONE_MINUS_DST_ALPHA;

    default:
        return GL_ONE;
    }
}

static GLenum
ToGLTextureAddressMode(NBRE_TextureAddressMode f)
{
    switch(f)
    {
    case NBRE_TAM_CLAMP:
        return GL_CLAMP_TO_EDGE;
    case NBRE_TAM_REPEAT:
        return GL_REPEAT;
    default:
        NBRE_DebugLog(PAL_LogSeverityInfo, "ToGLTextureAddressMode, Not Support wrap function!");
        return GL_CLAMP_TO_EDGE;
    }
}

static GLenum
ToGLTextureFilterType(NBRE_TextureFilterType f)
{
    switch(f)
    {
    case NBRE_TFT_POINT:
        return GL_NEAREST;
    case NBRE_TFT_LINEAR:
        return GL_LINEAR;
    case NBRE_TFT_MIPMAP_TRILINEAR:
        return GL_LINEAR_MIPMAP_LINEAR;
    case NBRE_TFT_MIPMAP_POINT:
        return GL_NEAREST_MIPMAP_NEAREST;
    case NBRE_TFT_MIPMAP_LINEAR:
        return GL_LINEAR_MIPMAP_NEAREST;
    default:
        return GL_REPEAT;
    }
}

static GLenum
ToGLCompareFunction(NBRE_CompareFunction f)
{
    switch(f)
    {
    case NBRE_CF_LESS_EQUAL:
        return GL_LEQUAL;

    case NBRE_CF_LESS:
        return GL_LESS;

    case NBRE_CF_NEVER:
        return GL_NEVER;

    case NBRE_CF_EQUAL:
        return GL_EQUAL;

    case NBRE_CF_GREATER:
        return GL_GREATER;

    case NBRE_CF_NOT_EQUAL:
        return GL_NOTEQUAL;

    case NBRE_CF_GREATER_EQUAL:
        return GL_GEQUAL;

    case NBRE_CF_ALWAYS:
        return GL_ALWAYS;

    default:
        return GL_ALWAYS;
    }
}

static GLenum
ToGLDataType(NBRE_VertexElement::VertexElementType type)
{
    switch(type)
    {
    case NBRE_VertexElement::VET_FLOAT4:
    case NBRE_VertexElement::VET_FLOAT3:
    case NBRE_VertexElement::VET_FLOAT2:
    case NBRE_VertexElement::VET_FLOAT1:
        return GL_FLOAT;

    case NBRE_VertexElement::VET_SHORT1:
    case NBRE_VertexElement::VET_SHORT2:
    case NBRE_VertexElement::VET_SHORT3:
    case NBRE_VertexElement::VET_SHORT4:
        return GL_SHORT;

    case NBRE_VertexElement::VET_COLOUR:
    case NBRE_VertexElement::VET_COLOUR_ABGR:
    case NBRE_VertexElement::VET_COLOUR_ARGB:
    case NBRE_VertexElement::VET_UBYTE4:
        return GL_UNSIGNED_BYTE;

    default:
        nbre_assert(0);
        break;
    };
    return GL_FLOAT;
}

static GLenum
ToGLPrimType(NBRE_PrimitiveType operationType)
{
    switch(operationType)
    {
    case NBRE_PMT_TRIANGLE_LIST:
        return GL_TRIANGLES;

    case NBRE_PMT_LINE_LIST:
        return GL_LINES;

    case NBRE_PMT_TRIANGLE_STRIP:
        return GL_TRIANGLE_STRIP;

    case NBRE_PMT_LINE_STRIP:
        return GL_LINE_STRIP;

    case NBRE_PMT_TRIANGLE_FAN:
        return GL_TRIANGLE_FAN;

    case NBRE_PMT_POINT_LIST:
        return GL_POINTS;

    default:
        nbre_assert(0);
        return GL_TRIANGLES;
    }
}

static GLenum
ToGLIndexType(NBRE_HardwareIndexBuffer::IndexType type)
{
    switch(type)
    {
    case NBRE_HardwareIndexBuffer::IT_16BIT:
        return GL_UNSIGNED_SHORT;

    case NBRE_HardwareIndexBuffer::IT_8BIT:
        return GL_UNSIGNED_BYTE;

    case NBRE_HardwareIndexBuffer::IT_32BIT:
    default:
        nbre_assert(0);
        return GL_UNSIGNED_SHORT;
    }
}

static GLint
ToSemanticIndex(NBRE_VertexElement::VertexElementSemantic sem, uint32 index)
{
    switch (sem)
    {
    case NBRE_VertexElement::VES_POSITION:
        return VS_ATTR_POSION_INDEX;

    case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
        {
            switch(index)
            {
            case 0:
                return VS_ATTR_TEXTRUE_COORD0_INDEX;
            case 1:
                return VS_ATTR_TEXTRUE_COORD1_INDEX;
            default:
                nbre_assert(0);
                return VS_ATTR_TEXTRUE_COORD0_INDEX;
            }
        }
        break;

    case NBRE_VertexElement::VES_NORMAL:
        return VS_ATTR_NORMAL_INDEX;

    case NBRE_VertexElement::VES_DIFFUSE:
        return VS_ATTR_COLOR_INDEX;

    case NBRE_VertexElement::VES_USER_DEFINE:
        return VS_ATTR_USER_DEFINE_INDEX;

    default:
        nbre_assert(0);
        return VS_ATTR_POSION_INDEX;
    }
}

static void
GetGLCapabilities(NBRE_RenderPalCapabilities& cap)
{
    GLint value = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    cap.SetMaxTextureSize(value);
    cap.SetMaxStreams(NBRE_GLES2_MINIMUM_SUPPORT_NUM);
    cap.SetMaxLights(NBRE_GLES2_CURRENT_SUPPORT_LIGHTS_NUM);
    cap.SetMaxTextureUnits(NBRE_GLES2_CURRENT_SUPPORT_TEXTURES_NUM);

    char* glExt = (char*) glGetString(GL_EXTENSIONS);
    if (glExt)
    {
        NBRE_String ext((const char*) glExt);
		int i = 0;
        i = ext.find("depth_texture");
        cap.SetSupportDepthTexture(i!=-1);

//         i = ext.find("GL_NV_path_rendering");
//         cap.SetSupportNVPathRendering(i!=-1);

//         if (!cap.GetSupportNVPathRendering())
//         {
//             char* glRenderer = (char*) glGetString(GL_RENDERER);
//             if (glRenderer)
//             {
//                 // This is required to run on Jetson Pro board
//                 // because the board doesn't provide GL_NV_path_rendering extension string
//                 // but actually support NVPR
//                 NBRE_String r((const char*) glRenderer);
//                 int i = r.find("NVIDIA");
//                 cap.SetSupportNVPathRendering(i!=-1);
//                 if (!cap.GetSupportDepthTexture() && i != -1)
//                 {
//                     cap.SetSupportDepthTexture(TRUE);
//                 }
//             }
//         }

        if (cap.GetSupportDepthTexture())
        {
            cap.SetSupportHBAO(TRUE);
        }

        if (!cap.GetSupportNVPathRendering())
        {
            const char* version = (const char*) glGetString(GL_VERSION);
            if (strstr(version, "OpenGL ES 1") /*||
                strstr(version, "OpenGL ES 2") ||
                strstr(version, "OpenGL ES 3.0")*/
                )
            {
                cap.SetSupportHBAO(FALSE);
            }
        }

        
        i = ext.find("GL_EXT_multisampled_render_to_texture");
        cap.SetSupporFBOAA(false);
        
        i = ext.find("GL_EXT_texture_filter_anisotropic");
        cap.SetSupportAnisotropy(i!=-1);
    }

    cap.SetSupportVAO(FALSE);
    cap.SetSupportCubeMap(FALSE);
}

NBRE_GLES2RenderPal::NBRE_GLES2RenderPal(PAL_Instance* pal):
    mPalInstance(pal),
    mGlRenderWindow(NULL),
    mCurrentRenderTarget(NULL),
    mVertexFormat(0xFFFFFFFF),
    mProgramFactory(NULL),
    mMaxAnisotropy(1),
    mTexCoordinateScaleX(1.f),
    mTexCoordinateScaleY(1.f),
    mEnableAntiAliasing(false),
    mEnableAnisotropicFiltering(false),
    mHBAOInitDone(FALSE),
    mHBAOInitOk(FALSE),
    mSSAOProgram(NULL)
{
    Initialize();
}

NBRE_GLES2RenderPal::~NBRE_GLES2RenderPal()
{
    delete mSSAOProgram;
    glDeleteTextures(1, &mDefaultTextureID);
    NBRE_DELETE_ARRAY mTextureTypes;
    NBRE_DELETE_ARRAY mTextureCoordIndex;
    NBRE_DELETE mGlRenderWindow;
    NBRE_DELETE mProgramFactory;
}

void
NBRE_GLES2RenderPal::Configure(const NBRE_RenderConfig& config)
{
    mEnableAntiAliasing = config.enableAntiAliasing;
    mEnableAnisotropicFiltering = config.enableAnisotropicFiltering;
}

void
NBRE_GLES2RenderPal::Initialize()
{
    nsl_memset(&mCurrentShaderState, 0, sizeof(mCurrentShaderState));
    GetGLCapabilities(mCapabilities);

    if(mCapabilities.GetSupportAnisotropy())
    {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mMaxAnisotropy);
        GL_CHECK_ERROR;
    }

    mTextureTypes = NBRE_NEW GLenum[mCapabilities.GetMaxTextureUnits()];
    mTextureCoordIndex = NBRE_NEW uint32[mCapabilities.GetMaxTextureUnits()];
    for (uint32 i = 0; i < mCapabilities.GetMaxTextureUnits(); ++i)
    {
        mTextureTypes[i] = 0;
        mTextureCoordIndex[i] = 99;
        mLastEnableTexcoordArray.push_back(FALSE);
    }

    mDisabledTexUnitsFrom = 0;
    mLastEnableNormalArray = FALSE;
    mLastEnableColorArray = FALSE;
    static float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glVertexAttrib4fv(VS_ATTR_COLOR_INDEX, white);

    // precreate gpu programs
    mProgramFactory = NBRE_NEW NBRE_GLES2GPUPorgramFactory(mPalInstance);
    mProgramFactory->Initialize();
    GLuint programid = mProgramFactory->GetDefaultProgram()->GetProgramId();
    mCurrentShaderState.programId = programid;
    glUseProgram(programid);
    mCurrentShaderState.matrixModelViewId = glGetUniformLocation(programid, MATRIX_MODELVIEW);

    char buffer[256] = {0};
    for(uint32 i = 0; i < mCapabilities.GetMaxTextureUnits(); i++)
    {
        nsl_sprintf(buffer,MATRIX_TEXTURE"%d", i);
        mCurrentShaderState.matrixTextureId[i] = glGetUniformLocation(programid, buffer);
    }

    for(uint32 i = VS_ATTR_POSION_INDEX; i <= VS_ATTR_USER_DEFINE_INDEX; ++i)
    {
        glDisableVertexAttribArray(i);
    }

    static float texcoord[] = {0.5f, 0.5f};
    glVertexAttrib2fv(VS_ATTR_TEXTRUE_COORD0_INDEX, texcoord);
    glVertexAttrib2fv(VS_ATTR_TEXTRUE_COORD1_INDEX, texcoord);

    float mat[] = {1, 0, 1, 0, 1, 0, 1, 0 };
    nsl_memcpy(mLastTextureMatrix, mat, sizeof(mLastTextureMatrix));
    glUniform4fv(mCurrentShaderState.matrixTextureId[0], 1, mat);
    glUniform4fv(mCurrentShaderState.matrixTextureId[1], 1, mat);

    if (mCapabilities.GetSupportDepthTexture() && !mCapabilities.GetSupportNVPathRendering())
    {
#ifdef __QNX__
        mSSAOProgram = NBRE_NEW NBRE_GLES2SSAOProgram(this, 16, 64, 1.0f, 4.0f, 0.1f);
#else
        mSSAOProgram = NBRE_NEW NBRE_GLES2SSAOProgram(this, 4, 64, 1.0f, 4.0f, 0.1f);
#endif
    }

    //Init GL State
    mCullFace = FALSE;
    glDisable(GL_CULL_FACE);

    mRedColorWrite = TRUE;
    mGreenColorWrite = TRUE;
    mBlueColorWrite = TRUE;
    mAlphaColorWrite = TRUE;
    glColorMask(TRUE, TRUE, TRUE, TRUE);

    mDepthTest = TRUE;
    glEnable(GL_DEPTH_TEST);

    mDepthMask = TRUE;
    glDepthMask(TRUE);

    mBlend = TRUE;
    glEnable(GL_BLEND);

    mSrcBlendFunction = NBRE_BF_SRC_ALPHA;
    mDestBlendFunction = NBRE_BF_ONE_MINUS_SRC_ALPHA;
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    glClearColor(1.f, 1.f, 1.f, 1.f);
	// TEMP WORK-AROUND
	if (glClearDepthf != NULL)
	    glClearDepthf(1.f);
	else {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2RenderPal::Unable to set glClearDepthf, line=%d", __LINE__);
	}

    mDepthFunction = NBRE_CF_LESS_EQUAL;
    glDepthFunc(GL_LEQUAL);


    mCurrentShaderState.textureSampler1 = glGetUniformLocation(programid, TEXTURE_SAMPLE_1);
    mCurrentShaderState.textureSampler2 = glGetUniformLocation(programid, TEXTURE_SAMPLE_2);

    glUniform1i(mCurrentShaderState.textureSampler1, 0);
    glUniform1i(mCurrentShaderState.textureSampler2, 1);

    CreateDefaultTexture();
    SelectDefaultTexture(0);
    SelectDefaultTexture(1);
}

void
NBRE_GLES2RenderPal::CreateDefaultTexture()
{
    uint8 pixels[4];
    pixels[0] = 255;
    pixels[1] = 255;
    pixels[2] = 255;
    pixels[3] = 255;
    glGenTextures(1, &mDefaultTextureID);
    glBindTexture(GL_TEXTURE_2D, mDefaultTextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}
void
NBRE_GLES2RenderPal::SelectDefaultTexture(uint32 stage)
{
    glActiveTexture(GL_TEXTURE0 + stage);
    glBindTexture(GL_TEXTURE_2D, mDefaultTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void
NBRE_GLES2RenderPal::UpdateProgram(uint32 vertexFormat)
{
   EnableColorArray(vertexFormat & NBRE_VSIF_COLOR ? TRUE : FALSE);
   EnableTexcoordArray(vertexFormat & NBRE_VSIF_TEXCORD0 ? TRUE : FALSE, 0);
   EnableTexcoordArray(vertexFormat & NBRE_VSIF_TEXCORD1 ? TRUE : FALSE, 1);
   EnableNormalArray(vertexFormat & NBRE_VSIF_NORMAL ? TRUE : FALSE);
}

void
NBRE_GLES2RenderPal::EnableNormalArray(nb_boolean enable)
{
    if (enable != mLastEnableNormalArray)
    {
        if (enable)
        {
            glEnableVertexAttribArray(VS_ATTR_NORMAL_INDEX);
        }
        else
        {
            glDisableVertexAttribArray(VS_ATTR_NORMAL_INDEX);
        }
        mLastEnableNormalArray = enable;
    }
}

void
NBRE_GLES2RenderPal::EnableColorArray(nb_boolean enable)
{
    if (enable != mLastEnableColorArray)
    {
        if (enable)
        {
            glEnableVertexAttribArray(VS_ATTR_COLOR_INDEX);
        }
        else
        {
            glDisableVertexAttribArray(VS_ATTR_COLOR_INDEX);
            static float white[] = {1.0f, 1.0f, 1.0f, 1.0f};
            glVertexAttrib4fv(VS_ATTR_COLOR_INDEX, white);
        }
        mLastEnableColorArray = enable;
    }
}

void
NBRE_GLES2RenderPal::EnableTexcoordArray(nb_boolean enable, uint32 index)
{
    nbre_assert(index < mCapabilities.GetMaxTextureUnits());
    if (enable != mLastEnableTexcoordArray[index])
    {
        GLuint attrIndex = 0;
        switch(index)
        {
        case 0:
            attrIndex = VS_ATTR_TEXTRUE_COORD0_INDEX;
            break;
        case 1:
            attrIndex = VS_ATTR_TEXTRUE_COORD1_INDEX;
            break;
        default:
            nbre_assert(0);
            break;
        }
        if (enable)
        {
            glEnableVertexAttribArray(attrIndex);
        }
        else
        {
            static float texcoord[] = {0.5f, 0.5f};
            glDisableVertexAttribArray(attrIndex);
            glVertexAttrib2fv(attrIndex, texcoord);
        }


        mLastEnableTexcoordArray[index] = enable;
    }
}

void
NBRE_GLES2RenderPal::SetVertexBuffer( NBRE_VertexData* vertexData )
{
    nbre_assert(vertexData);

    GL_CHECK_ERROR;

    if(mCapabilities.GetSupportVAO())
    {
        glBindVertexArray(0);
    }

    NBRE_GLES2VertexDeclaration *del = static_cast<NBRE_GLES2VertexDeclaration*>(vertexData->GetVertexDeclaration());
    uint32 vertexFormat = del->GetVertexInputFormat();

    if(mVertexFormat != vertexFormat)
    {
        UpdateProgram(vertexFormat);
        mVertexFormat = vertexFormat;
    }

    if(mCapabilities.GetSupportVAO())
    {
        GLuint id = del->GetVAOId();
        const NBRE_VertexDeclaration::VertexElementList& decl = del->GetVertexElementList();
        NBRE_VertexDeclaration::VertexElementList::const_iterator elemEnd = decl.end();

        if(id != 0)
        {
            for (NBRE_VertexDeclaration::VertexElementList::const_iterator elem = decl.begin(); elem != elemEnd; ++elem)
            {
                const NBRE_VertexElement* element = *elem;
                if(element->Semantic() == NBRE_VertexElement::VES_USER_DEFINE)
                    continue;
                NBRE_GLES2HardwareVertexBuffer* buffer = static_cast<NBRE_GLES2HardwareVertexBuffer*>(vertexData->GetBuffer(element->Source()));
                if(buffer->IsModified())
                {
                    glDeleteVertexArrays(1, &id);
                    id = 0;
                    break;
                }
            }
        }

        if(id == 0)
        {
            glGenVertexArrays(1, &id);
            glBindVertexArray(id);
            for (NBRE_VertexDeclaration::VertexElementList::const_iterator elem = decl.begin(); elem != elemEnd; ++elem)
            {
                const NBRE_VertexElement* element = *elem;
                if(element->Semantic() == NBRE_VertexElement::VES_USER_DEFINE)
                    continue;
                NBRE_GLES2HardwareVertexBuffer* buffer = static_cast<NBRE_GLES2HardwareVertexBuffer*>(vertexData->GetBuffer(element->Source()));
                glBindBuffer(GL_ARRAY_BUFFER, buffer->GetBufferId());
                GLint index = ToSemanticIndex(element->Semantic(), element->Index());
                GLint offset = element->Offset();
                glEnableVertexAttribArray(index);
                glVertexAttribPointer(index,                                               // index
                    static_cast<GLint>(NBRE_VertexElement::GetTypeCount(element->Type())), // size
                    ToGLDataType(element->Type()),                                         // type
                    GL_FALSE,                                                              // normalized
                    static_cast<GLsizei>(buffer->GetVertexSize()),                         // stride
                    VBO_BUFFER_OFFSET(offset));
            }
            del->SetVAOId(id);
        }
        else
        {
            glBindVertexArray(id);
        }
    }
    else
    {
        const NBRE_VertexDeclaration::VertexElementList& decl = del->GetVertexElementList();
        NBRE_VertexDeclaration::VertexElementList::const_iterator elemEnd = decl.end();
        NBRE_VertexDeclaration::VertexElementList::const_iterator elem = decl.begin();
        for (; elem != elemEnd; ++elem)
        {
            const NBRE_VertexElement* element = *elem;
            if(element->Semantic() == NBRE_VertexElement::VES_USER_DEFINE)
            {
                continue;
            }

            if(element->Semantic() == NBRE_VertexElement::VES_TEXTURE_COORDINATES)
            {
//            	if(element->Index() == 0)
                {
                    static float texcoord[] = {0.5f, 0.5f};
                    glVertexAttrib2fv(VS_ATTR_TEXTRUE_COORD0_INDEX, texcoord);

                }
            }

            NBRE_GLES2HardwareVertexBuffer* buffer = static_cast<NBRE_GLES2HardwareVertexBuffer*>(vertexData->GetBuffer(element->Source()));
            glBindBuffer(GL_ARRAY_BUFFER, buffer->GetBufferId());
            GLint index = ToSemanticIndex(element->Semantic(), element->Index());
            GLint offset = element->Offset();
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index,                                               // index
                static_cast<GLint>(NBRE_VertexElement::GetTypeCount(element->Type())), // size
                ToGLDataType(element->Type()),                                         // type
                GL_FALSE,                                                              // normalized
                static_cast<GLsizei>(buffer->GetVertexSize()),                         // stride
                VBO_BUFFER_OFFSET(offset));
        }
    }
}

void
NBRE_GLES2RenderPal::DrawIndexedPrimitive( NBRE_PrimitiveType operationType, const NBRE_IndexData* indexData )
{
    nbre_assert(indexData);
    GL_CHECK_ERROR

    NBRE_GLES2HardwareIndexBuffer* indexBuffer = static_cast<NBRE_GLES2HardwareIndexBuffer*>(indexData->IndexBuffer());
    if(indexBuffer != NULL)
    {
        if (indexBuffer->GetBufferId() != 0)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->GetBufferId());
            glDrawElements(ToGLPrimType(operationType), indexData->IndexCount(), ToGLIndexType(indexBuffer->GetType()), NULL);
        }
    }
    else
    {
        glDrawArrays(ToGLPrimType(operationType), indexData->IndexStart(), indexData->IndexCount());
    }
}

NBRE_Texture*
NBRE_GLES2RenderPal::CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    return NBRE_NEW NBRE_GLES2Texture(this, &info, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_GLES2RenderPal::CreateTexture( NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name )
{
    nbre_assert(image != NULL && numMipmaps != 0);
    return NBRE_NEW NBRE_GLES2Texture(this, image, faceCount, numMipmaps, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_GLES2RenderPal::CreateTexture( NBRE_ITextureImage* textureLoader, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
   nbre_assert(textureLoader != NULL);
   return NBRE_NEW NBRE_GLES2Texture(this, textureLoader, texType, isUsingMipmap, name);
}

NBRE_HardwareIndexBuffer*
NBRE_GLES2RenderPal::CreateIndexBuffer( NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage)
{
    return NBRE_NEW NBRE_GLES2HardwareIndexBuffer(idxType, numIndexes, useShadowBuffer, usage);
}

NBRE_RenderTarget*
NBRE_GLES2RenderPal::GetDefaultRenderTarget()
{
    if(mGlRenderWindow == NULL)
    {
        mGlRenderWindow = NBRE_NEW NBRE_GLES2RenderWindow;
    }
    return mGlRenderWindow;
}

void
NBRE_GLES2RenderPal::LoadGpuProgram(const NBRE_String& name, const NBRE_String& vertShaderSrc, const NBRE_String& fragShaderSrc)
{
    mProgramFactory->CreateProgram(name, vertShaderSrc, fragShaderSrc);
}

void
NBRE_GLES2RenderPal::EnableDepthTest( nb_boolean bEnable )
{
    if(mDepthTest == bEnable)
    {
        return;
    }

    mDepthTest = bEnable;
    if (bEnable)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void
NBRE_GLES2RenderPal::EnableStencilTest( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_STENCIL_TEST);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
}

void
NBRE_GLES2RenderPal::EnableTexture( uint32 stage, nb_boolean bEnable )
{
    if(stage >= mCapabilities.GetMaxTextureUnits())
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2RenderPal::EnableTexture failed! Please check the max support texture units , line=%d", __LINE__);
        return;
    }
    nbre_assert(stage < mCapabilities.GetMaxTextureUnits());
    if(!bEnable)
    {
        SelectDefaultTexture(stage);
    }
    mTextureCoordIndex[stage] = bEnable;
}

void
NBRE_GLES2RenderPal::DisableTextureUnitsFrom( uint32 stage )
{
    if(stage >= mCapabilities.GetMaxTextureUnits())
    {
        return;
    }
    for (uint32 i = stage; i < 2; ++i)
    {
        EnableTexture(i, FALSE);
    }
    mDisabledTexUnitsFrom = stage;
}

void
NBRE_GLES2RenderPal::SelectTexture(uint32 stage, NBRE_Texture* texture)
{
    if(stage >= mCapabilities.GetMaxTextureUnits())
    {
        return;
    }

    if(texture)
    {
        NBRE_GLES2Texture *glTexture = static_cast<NBRE_GLES2Texture*>(texture);
        GLenum textureType = glTexture->GetTextureTarget();
        mTextureTypes[stage] = textureType;
        EnableTexture(stage, TRUE);
        glActiveTexture(GL_TEXTURE0 + stage);
        glTexture->Load();
        glBindTexture(textureType, glTexture->GetID());
        mTexCoordinateScaleX = glTexture->GetCoordinateScaleX();
        mTexCoordinateScaleY = glTexture->GetCoordinateScaleY();
    }
    else
    {
        EnableTexture(stage, FALSE);
    }
}

void
NBRE_GLES2RenderPal::EnableBlend( nb_boolean bEnable )
{
    if (mBlend == bEnable)
    {
        return;
    }
    mBlend = bEnable;

    if (bEnable)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

NBRE_HardwareVertexBuffer*
NBRE_GLES2RenderPal::CreateVertexBuffer( uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    NBRE_GLES2HardwareVertexBuffer* buffer = NBRE_NEW NBRE_GLES2HardwareVertexBuffer(vertexSize, numVertices, useShadowBuffer, usage);
    if(buffer!= NULL)
    {
        buffer->Allocate();
    }
    return buffer;
}

void
NBRE_GLES2RenderPal::SetViewTransform( const NBRE_Matrix4x4d& mtrx )
{
    mViewMatrix = mtrx;
    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix);
    m = mProjMatrix*m;
    m.Transpose();
    glUniformMatrix4fv(mCurrentShaderState.matrixModelViewId, 1, GL_FALSE, m[0]);
}

void
NBRE_GLES2RenderPal::SetWorldTransform( const NBRE_Matrix4x4d& mtrx )
{
    mWorldMatrix = mtrx;
    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix);
    m = mProjMatrix*m;
    m.Transpose();
    glUniformMatrix4fv(mCurrentShaderState.matrixModelViewId, 1, GL_FALSE, m[0]);
}

void
NBRE_GLES2RenderPal::SetProjectionTransform( const NBRE_Matrix4x4f& mtrx )
{
    mProjMatrix =  mtrx;
    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix);
    m = mProjMatrix*m;
    m.Transpose();
    glUniformMatrix4fv(mCurrentShaderState.matrixModelViewId, 1, GL_FALSE, m[0]);
}

void
NBRE_GLES2RenderPal::SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx )
{
   if(stage >= mCapabilities.GetMaxTextureUnits())
   {
       NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2RenderPal::SetTextureTransform failed! Please check the max support texture units , line=%d", __LINE__);
       return;
   }
   float mat[] = { mtrx[0][0]*mTexCoordinateScaleX, mtrx[0][3]*mTexCoordinateScaleX,mtrx[1][1]*mTexCoordinateScaleY, mtrx[1][3]*mTexCoordinateScaleY};
   if(nsl_memcmp(mLastTextureMatrix + stage*4, mat, sizeof(mat)))
   {
       glUniform4fv(mCurrentShaderState.matrixTextureId[stage], 1, mat);
       nsl_memcpy(mLastTextureMatrix+ stage*4, mat, sizeof(mat));
   }
}

void
NBRE_GLES2RenderPal::SetTextureState(uint32 stage, const NBRE_TextureState& state)
{
    if(stage >= mCapabilities.GetMaxTextureUnits())
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2RenderPal::SetTextureState failed! Please check the max support texture units , line=%d", __LINE__);
        return;
    }
    glActiveTexture(GL_TEXTURE0 + stage);
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_MIN_FILTER, ToGLTextureFilterType(state.GetMinFilter()));
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_MAG_FILTER, ToGLTextureFilterType(state.GetMagFilter()));
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_S, ToGLTextureAddressMode(state.GetWrapS()));
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_T, ToGLTextureAddressMode(state.GetWrapT()));
    if(mEnableAnisotropicFiltering && mCapabilities.GetSupportAnisotropy() && state.UsingAnisotropic())
    {
        glTexParameterf(mTextureTypes[stage], GL_TEXTURE_MAX_ANISOTROPY_EXT, mMaxAnisotropy);
    }
}

void
NBRE_GLES2RenderPal::SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void
NBRE_GLES2RenderPal::SetClearDepth(float value)
{
    glClearDepthf(value);
}

void
NBRE_GLES2RenderPal::SetClearStencil(int32 value)
{
    glClearStencil(value);
}

void
NBRE_GLES2RenderPal::Clear(uint32 flags)
{
    GLbitfield mask = 0;

    if (flags & NBRE_CLF_CLEAR_COLOR)
    {
        EnableColorWrite(TRUE,TRUE,TRUE,TRUE);
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if (flags & NBRE_CLF_CLEAR_DEPTH)
    {
        EnableDepthWrite(TRUE);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (flags & NBRE_CLF_CLEAR_STENCIL)
    {
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    if(mask!=0)
    {
        glClear(mask);
    }
}

void
NBRE_GLES2RenderPal::EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha )
{
    if ((mRedColorWrite == enableRed) && (mGreenColorWrite == enableGreen) && (mBlueColorWrite == enableBlue) && (mAlphaColorWrite == enableAlpha))
    {
        return;
    }

    mRedColorWrite = enableRed;
    mGreenColorWrite = enableGreen;
    mBlueColorWrite = enableBlue;
    mAlphaColorWrite = enableAlpha;
    glColorMask(enableRed, enableGreen, enableBlue, enableAlpha);
}

void
NBRE_GLES2RenderPal::EnableDepthWrite( nb_boolean bEnable )
{
    if (mDepthMask == bEnable)
    {
        return;
    }
    mDepthMask = bEnable;
    glDepthMask(bEnable);
}

void
NBRE_GLES2RenderPal::EnableStencilWrite( uint32 maskBits )
{
    glStencilMask(maskBits);
}

void
NBRE_GLES2RenderPal::EnableCullFace( nb_boolean bEnable )
{
    if (mCullFace == bEnable)
    {
        return;
    }

    mCullFace = bEnable;
    if (bEnable)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void
NBRE_GLES2RenderPal::SetDepthFunc(NBRE_CompareFunction f)
{
    if(mDepthFunction == f)
    {
        return;
    }
    mDepthFunction = f;
    glDepthFunc(ToGLCompareFunction(f));
}

void
NBRE_GLES2RenderPal::SelectRenderTarget( NBRE_RenderTarget* rt )
{
    mCurrentRenderTarget = rt;
}

NBRE_RenderTarget*
NBRE_GLES2RenderPal::CreateRenderTargetFromTexture(NBRE_Texture* texture,
                                                   int32 face,
                                                   nb_boolean useDepthTexture)
{
    nb_boolean supportFBOAA = mEnableAntiAliasing ? mCapabilities.GetSupportFBOAA() : FALSE;
    nb_boolean supportDepthTexture = useDepthTexture ? mCapabilities.GetSupportDepthTexture() : FALSE;
    nb_boolean supportNVPathRendering = mCapabilities.GetSupportNVPathRendering();

    return NBRE_NEW NBRE_GLES2RenderTexture(static_cast<NBRE_GLES2Texture*>(texture),
                                            face,
                                            supportFBOAA,
                                            supportNVPathRendering,
                                            supportDepthTexture);
}

void
NBRE_GLES2RenderPal::SetViewPort( int32 left, int32 top, uint32 width , uint32 height )
{
    mLastViewPort[0] = left;
    mLastViewPort[1] = top;
    mLastViewPort[2] = width;
    mLastViewPort[3] = height;
    glViewport(left, top, width, height);
}

void
NBRE_GLES2RenderPal::SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor)
{
    if ((mSrcBlendFunction == srcFactor) && (mDestBlendFunction == dstFactor))
    {
        return;
    }
    mSrcBlendFunction = srcFactor;
    mDestBlendFunction = dstFactor;
    glBlendFuncSeparate(ToGLBlendFunction(srcFactor), ToGLBlendFunction(dstFactor), GL_ONE, GL_ONE);
}

nb_boolean
NBRE_GLES2RenderPal::BeginScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->BeginRender();
    }
    return TRUE;
}

void
NBRE_GLES2RenderPal::EndScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->EndRender();
    }
}

NBRE_VertexDeclaration*
NBRE_GLES2RenderPal::CreateVertexDeclaration(void)
{
    return NBRE_NEW NBRE_GLES2VertexDeclaration();
}

void
NBRE_GLES2RenderPal::SetTextureCoordSet(uint32 /*stage*/, uint32 /*index*/)
{
}

void
NBRE_GLES2RenderPal::SetColor(float /*r*/, float /*g*/, float /*b*/, float /*a*/)
{
//    float white[] = {r, g, b, a};
//    glVertexAttrib4fv(VS_ATTR_COLOR_INDEX, white);
}

void
NBRE_GLES2RenderPal::SetAlphaFunc(NBRE_CompareFunction /*f*/, float /*refValue*/)
{
    // Only support less-than for now
    //glUniform1f(mCurrentShaderState.alphaValue, refValue);
}

void
NBRE_GLES2RenderPal::SetWireframeMode(nb_boolean /*bLine*/)
{
}

void
NBRE_GLES2RenderPal::EnableLighting( nb_boolean /*bEnable*/ )
{
}

void
NBRE_GLES2RenderPal::EnableFog( nb_boolean /*bEnable*/ )
{
}

void
NBRE_GLES2RenderPal::SetFog(NBRE_FogMode /*mode*/, float /*density*/, float /*fogStart*/, float /*fogEnd*/, float /*red*/, float /*green*/, float /*blue*/, float /*alpha*/)
{
}

void
NBRE_GLES2RenderPal::UseLights( const NBRE_LightList& /*lights*/ )
{
}

void
NBRE_GLES2RenderPal::SetActiveLights( const NBRE_LightIndexList& /*indices*/ )
{
}

void
NBRE_GLES2RenderPal::SetMaterial(const NBRE_Material& /*material*/)
{
    //float ambient[] = {material.Ambient().r, material.Ambient().g, material.Ambient().b, material.Ambient().a};
    //float diffuse[] = {material.Diffuse().r, material.Diffuse().g, material.Diffuse().b, material.Diffuse().a};
    //float specular[] = {material.Specular().r, material.Specular().g, material.Specular().b, material.Specular().a};
    //float emission[] = {material.Emissive().r, material.Emissive().g, material.Emissive().b, material.Emissive().a};
    // TODO...
}
void
NBRE_GLES2RenderPal::ReadPixels(int x, int y, uint32 width, uint32 height, uint8 *buffer)const
{
    glReadPixels((GLint)x, (GLint)y, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buffer);
    GL_CHECK_ERROR;

    uint32 rowSize = 4*width;
    uint8* rowData = NBRE_NEW uint8[rowSize];
    #define ROW(line) (uint8*)(buffer + (line)*rowSize)
    for (uint32 i = 0; i < height/2; ++i)
    {
        nsl_memcpy(rowData,            ROW(i),             rowSize);
        nsl_memcpy(ROW(i),             ROW(height - i -1), rowSize);
        nsl_memcpy(ROW(height - i -1), rowData,            rowSize);
        for(uint32 j = 3; j < rowSize; j+=4)
        {
            *(ROW(i)+j) = 255;
            *(ROW(height - i -1)+j) = 255;
        }
    }
    #undef ROW
    NBRE_DELETE[] rowData;
}

void
NBRE_GLES2RenderPal::SetLineWidth(float width)
{
    glLineWidth(width);
}

void
NBRE_GLES2RenderPal::SetGpuProgram(const NBRE_String& name)
{
    GL_CHECK_ERROR

    if (mGpuProgramName != name)
    {
        mGpuProgramName = name;
    }

    NBRE_GLES2GpuProgramPtr p = mProgramFactory->GetByName(name);
    if (p)
    {
        glUseProgram(p->GetProgramId());
    }
    else
    {
        glUseProgram(mProgramFactory->GetDefaultProgram()->GetProgramId());
    }
    GL_CHECK_ERROR
}

void
NBRE_GLES2RenderPal::SetGpuProgramParam( const NBRE_String& name, const NBRE_GpuProgramParam& param)
{
    GL_CHECK_ERROR

    NBRE_GLES2GpuProgramPtr p = mProgramFactory->GetByName(mGpuProgramName);
    if (!p || p == mProgramFactory->GetDefaultProgram())
    {
        return;
    }

    GLint loc = glGetUniformLocation(p->GetProgramId(), name.c_str());
    if (loc == -1)
    {
        return;
    }
    switch (param.GetType())
    {
    case NBRE_GpuProgramParam::VT_Float:
        {
            glUniform1f(loc, param.GetValue().f1);
        }
        break;
    case NBRE_GpuProgramParam::VT_Float2:
        {
            glUniform2fv(loc, 1, param.GetValue().f2);
        }
        break;
    case NBRE_GpuProgramParam::VT_Float3:
        {
            glUniform3fv(loc, 1, param.GetValue().f3);
        }
        break;
    case NBRE_GpuProgramParam::VT_Float4:
        {
            glUniform4fv(loc, 1, param.GetValue().f4);
        }
        break;
    case NBRE_GpuProgramParam::VT_Int:
        {
            glUniform1i(loc, param.GetValue().i1);
        }
        break;
    case NBRE_GpuProgramParam::VT_Matrix4:
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, param.GetValue().mat4);
        }
        break;
    case NBRE_GpuProgramParam::VT_Predefined:
        {
            switch (param.GetValue().pre)
            {
            case NBRE_GpuProgramParam::PV_ModelViewProjectionMatrix:
                {
                    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mViewMatrix*mWorldMatrix);
                    m = mProjMatrix*m;
                    m.Transpose();
                    glUniformMatrix4fv(loc, 1, GL_FALSE, m[0]);
                }
                break;
            case NBRE_GpuProgramParam::PV_NormalMatrix:
                {
                    // normal matrix = transpose(inverse(model_view))
                    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert((mViewMatrix*mWorldMatrix).Inverse());
                    glUniformMatrix4fv(loc, 1, GL_FALSE, m[0]);
                }
                break;
            case NBRE_GpuProgramParam::PV_ModelMatrix:
                {
                    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mWorldMatrix);
                    m.Transpose();
                    glUniformMatrix4fv(loc, 1, GL_FALSE, m[0]);
                }
                break;
            case NBRE_GpuProgramParam::PV_ViewMatrix:
                {
                    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mViewMatrix);
                    m.Transpose();
                    glUniformMatrix4fv(loc, 1, GL_FALSE, m[0]);
                }
                break;
            case NBRE_GpuProgramParam::PV_ProjectionMatrix:
                {
                    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mProjMatrix);
                    m.Transpose();
                    glUniformMatrix4fv(loc, 1, GL_FALSE, m[0]);
                }
                break;
            case NBRE_GpuProgramParam::PV_ViewInverseTransposeMatrix:
                {
                    NBRE_Matrix4x4f m =  NBRE_TypeConvertf::Convert(mViewMatrix.Inverse());
                    glUniformMatrix4fv(loc, 1, GL_FALSE, m[0]);
                }
                break;
            default:
                break;
            }
        }
        break;
    case NBRE_GpuProgramParam::VT_Float3Array:
        {
            glUniform3fv(loc, param.GetCount(), &(*param.GetArray()).front());
        }
        break;
    default:
        break;
    }

    GL_CHECK_ERROR
}

nb_boolean
NBRE_GLES2RenderPal::RenderAO(NBRE_RenderTarget* out, NBRE_RenderTarget* depth, NBRE_RenderTarget* blur, NBRE_RenderTarget* aoBuffer, const float* projMatrix, float metersToViewSpaceUnits, float radius, float bias, float powerExponent
                                ,float detailAO, float coarseAO, nb_boolean enableBlur, int blurRadius, float blurSharpness)
{
    GL_CHECK_ERROR

    if (!out || !depth)
    {
        return FALSE;
    }

    if (!mCapabilities.GetSupportHBAO())
    {
        return FALSE;
    }

//     if (mCapabilities.GetSupportNVPathRendering())
//     {
//         if (!mHBAOInitDone)
//         {
//             mHBAOInitOk = PAL_HBAOInit();
//             mHBAOInitDone = TRUE;
//         }
// 
//         if (!mHBAOInitOk)
//         {
//             return FALSE;
//         }
//     }
//     else
    {
        if (!mSSAOProgram)
        {
            return FALSE;
        }
    }

    nb_boolean result = FALSE;
    GLint mDefaultFramebufferID = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFramebufferID);


    NBRE_GLES2RenderTexture* ot = (NBRE_GLES2RenderTexture*)out;
    NBRE_GLES2RenderTexture* t = (NBRE_GLES2RenderTexture*)depth;
    NBRE_GLES2RenderTexture* br = (NBRE_GLES2RenderTexture*)blur;
    NBRE_GLES2RenderTexture* ao = (NBRE_GLES2RenderTexture*)aoBuffer;

//     if (mCapabilities.GetSupportNVPathRendering())
//     {
//         result = PAL_HBAORender(ot->GetFramebufferId(), t->GetDepthTextureId(), projMatrix, metersToViewSpaceUnits, radius, bias, powerExponent
//             , detailAO, coarseAO, enableBlur, blurRadius, blurSharpness);
//     }
//     else
    {
        NBRE_Matrix4x4f projInv = NBRE_Matrix4x4f(projMatrix, FALSE).Inverse();
        float pi[16];
        projInv.CopyTo(pi, FALSE);

#ifdef __QNX__
        mSSAOProgram->Render(ot, t, br, ao, projMatrix, pi, radius * metersToViewSpaceUnits * 400, 3.0f, 0.6f, 8.0f);
#else
        mSSAOProgram->Render(ot, t, br, ao, projMatrix, pi, 280 * metersToViewSpaceUnits, 6.0f, 0.2f, 2.0f);
#endif  

        // Restore states
        if (mBlend)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }

        if (mDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        glActiveTexture(GL_TEXTURE0);
        glUseProgram(mProgramFactory->GetDefaultProgram()->GetProgramId());
        glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFramebufferID);
        glViewport(mLastViewPort[0], mLastViewPort[1], mLastViewPort[2], mLastViewPort[3]);
    }

    GL_CHECK_ERROR

    return result;
}

NBRE_GLES2GpuProgramPtr
NBRE_GLES2RenderPal::GetProgram(const NBRE_String& name)
{
    return mProgramFactory->GetByName(name);
}
