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
#include "nbreglesrenderpal.h"
#include "nbregleshardwareindexbuffer.h"
#include "nbregleshardwarevertexbuffer.h"
#include "nbreglesrenderwindow.h"
#include "nbreitextureimage.h"
#include "nbrematerial.h"
#include "nbreglestexture.h"
#include "nbreglesrendertexture.h"
#include "nbretypeconvert.h"
#include "nbrelog.h"
#include "nbreglescommon.h"

#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLenum
ToGLBlendFunction(NBRE_BlendFunction f)
{
    GLenum result = GL_ONE;
    switch (f)
    {
    case NBRE_BF_ZERO:
        result = GL_ZERO;
        break;
    case NBRE_BF_ONE:
        result = GL_ONE;
        break;
    case NBRE_BF_SRC_COLOR:
        result = GL_SRC_COLOR;
        break;
    case NBRE_BF_ONE_MINUS_SRC_COLOR:
        result = GL_ONE_MINUS_SRC_COLOR;
        break;
    case NBRE_BF_SRC_ALPHA:
        result = GL_SRC_ALPHA;
        break;
    case NBRE_BF_ONE_MINUS_SRC_ALPHA:
        result = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case NBRE_BF_DST_COLOR:
        result = GL_DST_COLOR;
        break;
    case NBRE_BF_ONE_MINUS_DST_COLOR:
        result = GL_ONE_MINUS_DST_COLOR;
        break;
    case NBRE_BF_DST_ALPHA:
        result = GL_DST_ALPHA;
        break;
    case NBRE_BF_ONE_MINUS_DST_ALPHA:
        result = GL_ONE_MINUS_DST_ALPHA;
        break;
    }
    return result;
}

static GLenum 
ToGLTextureAddressMode(NBRE_TextureAddressMode f)
{
    GLenum result = GL_REPEAT;
    switch (f)
    {
    case NBRE_TAM_CLAMP:
        result = GL_CLAMP_TO_EDGE;
        break;

    case NBRE_TAM_REPEAT:
        result = GL_REPEAT;
        break;

    default:
        result = GL_CLAMP_TO_EDGE;
        NBRE_DebugLog(PAL_LogSeverityInfo, "ToGLTextureAddressMode, Not Support wrap function!");
        break;
    }
    return result;
}

static GLenum 
ToGLTextureFilterType(NBRE_TextureFilterType f)
{
    GLenum result = GL_REPEAT;
    switch(f)
    {
    case NBRE_TFT_LINEAR:
        result = GL_LINEAR;
        break;

    case NBRE_TFT_MIPMAP_LINEAR:
        result = GL_LINEAR_MIPMAP_NEAREST;
        break;

    case NBRE_TFT_MIPMAP_TRILINEAR:
        result = GL_LINEAR_MIPMAP_LINEAR;
        break;

    case NBRE_TFT_POINT:
        result = GL_NEAREST;
        break;

    case NBRE_TFT_MIPMAP_POINT:
        result = GL_NEAREST_MIPMAP_NEAREST;
        break;
    }
    return result;
}

static GLenum 
ToGLFogMode(NBRE_FogMode f)
{
    GLenum result = GL_LINEAR;
    switch(f)
    {
    case NBRE_FM_LINEAR:
        result = GL_LINEAR;
        break;
    case NBRE_FM_EXP:
        result = GL_EXP;
        break;
    case NBRE_FM_EXP2:
        result = GL_EXP2;
        break;
    }
    return result;
}

static GLenum 
ToGLCompareFunction(NBRE_CompareFunction f)
{
    GLenum result = GL_ALWAYS;
    switch(f)
    {
    case NBRE_CF_NEVER:
        result = GL_NEVER;
        break;
    case NBRE_CF_LESS:
        result = GL_LESS;
        break;
    case NBRE_CF_LESS_EQUAL:
        result = GL_LEQUAL;
        break;
    case NBRE_CF_EQUAL:
        result = GL_EQUAL;
        break;
    case NBRE_CF_GREATER:
        result = GL_GREATER;
        break;
    case NBRE_CF_NOT_EQUAL:
        result = GL_NOTEQUAL;
        break;
    case NBRE_CF_GREATER_EQUAL:
        result = GL_GEQUAL;
        break;
    case NBRE_CF_ALWAYS:
        result = GL_ALWAYS;
        break;
    }
    return result;
}

static void 
GetGLCapabilities(NBRE_RenderPalCapabilities& cap)
{
    GLint value = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &value);
    GL_CHECK_ERROR;
    cap.SetMaxLights(value);

    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &value);
    GL_CHECK_ERROR;
    cap.SetMaxTextureUnits(value);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    GL_CHECK_ERROR;
    cap.SetMaxTextureSize(value);

    cap.SetSupportVBO(TRUE);

    const char* glExt = (const char*) glGetString(GL_EXTENSIONS);
    if (glExt)
    {
        NBRE_String exts((const char*) glExt);
        int i = exts.find("GL_OES_texture_cube_map");
        cap.SetSupportCubeMap(i!=-1);
        i = exts.find("GL_EXT_texture_filter_anisotropic");
        cap.SetSupportAnisotropy(i!=-1);
    }
    else
    {
        cap.SetSupportCubeMap(FALSE);
        cap.SetSupportAnisotropy(FALSE);
    }
}

NBRE_GLESRenderPal::NBRE_GLESRenderPal():
    mGlRenderWindow(NULL),
    mCurrentRenderTarget(NULL)
{
    GetGLCapabilities(mCapabilities);

    if(mCapabilities.GetSupportAnisotropy())
    {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mMaxAnisotropy);
        GL_CHECK_ERROR;
    }

    glClearDepthf(1.0f);
    GL_CHECK_ERROR;
    glDepthFunc(GL_LEQUAL);
    GL_CHECK_ERROR;
    mTextureTypes = NBRE_NEW GLenum[mCapabilities.GetMaxTextureUnits()];
    mTextureCoordIndex = NBRE_NEW uint32[mCapabilities.GetMaxTextureUnits()];
	for (uint32 i = 0; i < mCapabilities.GetMaxTextureUnits(); ++i)
	{
		mTextureTypes[i] = 0;
		mTextureCoordIndex[i] = 99;
        mCurrentEnableTexcoordArray.push_back(FALSE);
        mLastEnableTexcoordArray.push_back(FALSE);
        glClientActiveTexture(GL_TEXTURE0 + i);
        GL_CHECK_ERROR;
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        GL_CHECK_ERROR;
	}
    mDisabledTexUnitsFrom = 0;
    glEnableClientState( GL_VERTEX_ARRAY );
    GL_CHECK_ERROR;

    mLastEnableNormalArray = FALSE;
    glDisableClientState( GL_NORMAL_ARRAY );
    GL_CHECK_ERROR;

    mLastEnableColorArray = FALSE;
    glDisableClientState( GL_COLOR_ARRAY );
    GL_CHECK_ERROR;
    
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    GL_CHECK_ERROR;
    
    mClientActiveTexture = GL_TEXTURE0;
    glClientActiveTexture(GL_TEXTURE0);
    GL_CHECK_ERROR;
    mActiveTexture = GL_TEXTURE0;
    glActiveTexture(GL_TEXTURE0);
    GL_CHECK_ERROR;

    mMaxtrixMode = GL_MODELVIEW;
    glMatrixMode(GL_MODELVIEW);
    
    mDisabledLightsFrom = 0;

    mAlphaTest = FALSE;
    glDisable(GL_ALPHA_TEST);
    GL_CHECK_ERROR;
    
    mFog = FALSE;
    glDisable(GL_FOG);
    GL_CHECK_ERROR;
    
    mLighting = FALSE;
    glDisable(GL_LIGHTING);
    GL_CHECK_ERROR;
    
    mCullFace = FALSE;
    glDisable(GL_CULL_FACE);
    GL_CHECK_ERROR;

    mRedColorWrite = TRUE;
    mGreenColorWrite = TRUE;
    mBlueColorWrite = TRUE;
    mAlphaColorWrite = TRUE;
    glColorMask(TRUE, TRUE, TRUE, TRUE);
    GL_CHECK_ERROR;
    
    mDepthTest = TRUE;
    glEnable(GL_DEPTH_TEST);
    GL_CHECK_ERROR;

    mDepthMask = TRUE;
    glDepthMask(TRUE);
    GL_CHECK_ERROR;
    
    mBlend = TRUE;
    glEnable(GL_BLEND);
    GL_CHECK_ERROR;
    
    mSrcBlendFunction = NBRE_BF_SRC_ALPHA;
    mDestBlendFunction = NBRE_BF_ONE_MINUS_SRC_ALPHA;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL_CHECK_ERROR;
    
    glColor4f(1, 1, 1, 1);
    GL_CHECK_ERROR;

    mLeft = 0;
    mTop = 0;
    mWidth = 0;
    mHeight = 0;
}

NBRE_GLESRenderPal::~NBRE_GLESRenderPal()
{
    NBRE_DELETE_ARRAY mTextureTypes;
    NBRE_DELETE_ARRAY mTextureCoordIndex;

    NBRE_DELETE mGlRenderWindow;
}

void NBRE_GLESRenderPal::SetMaterial(const NBRE_Material& material)
{
    float ambient[] = {material.Ambient().r, material.Ambient().g, material.Ambient().b, material.Ambient().a};
    float diffuse[] = {material.Diffuse().r, material.Diffuse().g, material.Diffuse().b, material.Diffuse().a};
    float specular[] = {material.Specular().r, material.Specular().g, material.Specular().b, material.Specular().a};
    float emission[] = {material.Emissive().r, material.Emissive().g, material.Emissive().b, material.Emissive().a};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    GL_CHECK_ERROR;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    GL_CHECK_ERROR;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    GL_CHECK_ERROR;
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    GL_CHECK_ERROR;
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.Shiness());
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::EnableNormalArray(nb_boolean enable)
{
    if (enable != mLastEnableNormalArray)
    {
        if (enable)
        {
            glEnableClientState(GL_NORMAL_ARRAY);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisableClientState( GL_NORMAL_ARRAY );
            GL_CHECK_ERROR;
        }
        mLastEnableNormalArray = enable;
    }
}
void 
NBRE_GLESRenderPal::EnableColorArray(nb_boolean enable)
{   
    if (enable != mLastEnableColorArray)
    {
        if (enable)
        {
            glEnableClientState(GL_COLOR_ARRAY);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisableClientState( GL_COLOR_ARRAY );
            GL_CHECK_ERROR;
        }
        mLastEnableColorArray = enable;
    }
}
void 
NBRE_GLESRenderPal::EnableTexcoordArray(const NBRE_Vector<nb_boolean>& enables)
{
    for(uint32 i = 0; i < enables.size(); ++i)
    {
        if (enables[i] != mLastEnableTexcoordArray[i])
        {
            ClientActiveTexture(GL_TEXTURE0 + i);
            if (enables[i])
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                GL_CHECK_ERROR;
            }
            else
            {
                glDisableClientState( GL_TEXTURE_COORD_ARRAY );
                GL_CHECK_ERROR;
            }
            mLastEnableTexcoordArray[i] = enables[i];
        }
    }
}

static GLenum
GetVBOUsage(const NBRE_HardwareBuffer* buffer)
{
    switch(buffer->GetUsage())
    {
    case NBRE_HardwareBuffer::HBU_STATIC:
        return GL_STATIC_DRAW;

    case NBRE_HardwareBuffer::HBU_DYNAMIC:
        return GL_DYNAMIC_DRAW;

    case NBRE_HardwareBuffer::HBU_STREAM:
        return GL_STREAM_DRAW;

    default:
        return GL_STATIC_DRAW;
    };
}

void
NBRE_GLESRenderPal::SetVertexBuffer(NBRE_VertexData* vertexData )
{
    nbre_assert(vertexData);
    nb_boolean enableNormal = FALSE;
    nb_boolean enableColor = FALSE;
	for (uint32 i = 0; i < mCapabilities.GetMaxTextureUnits(); ++i)
	{
        mCurrentEnableTexcoordArray[i] = FALSE;
	}

    const NBRE_VertexDeclaration::VertexElementList& decl = vertexData->GetVertexDeclaration()->GetVertexElementList();
    NBRE_VertexDeclaration::VertexElementList::const_iterator elemEnd = decl.end();

    for (NBRE_VertexDeclaration::VertexElementList::const_iterator elem = decl.begin(); elem != elemEnd; ++elem)
    {
        const NBRE_VertexElement* element = *elem;
        NBRE_GLHardwareVertexBuffer* buffer = static_cast<NBRE_GLHardwareVertexBuffer*>(vertexData->GetBuffer(element->Source()));

        uint8* pBufferData = static_cast<uint8*>(buffer->Lock(element->Offset(), NBRE_HardwareBuffer::HBL_READ_ONLY));
        uint16 typeCount = NBRE_VertexElement::GetTypeCount(element->Type());
        GLsizei stride = static_cast<GLsizei>(buffer->GetVertexSize());

        GLenum glType = GL_FLOAT;
        switch(element->Type())
        {
        case NBRE_VertexElement::VET_FLOAT1:
        case NBRE_VertexElement::VET_FLOAT2:
        case NBRE_VertexElement::VET_FLOAT3:
        case NBRE_VertexElement::VET_FLOAT4:
            glType = GL_FLOAT;
            break;
        case NBRE_VertexElement::VET_SHORT1:
        case NBRE_VertexElement::VET_SHORT2:
        case NBRE_VertexElement::VET_SHORT3:
        case NBRE_VertexElement::VET_SHORT4:
            glType = GL_SHORT;
            break;
        case NBRE_VertexElement::VET_COLOUR:
        case NBRE_VertexElement::VET_COLOUR_ABGR:
        case NBRE_VertexElement::VET_COLOUR_ARGB:
        case NBRE_VertexElement::VET_UBYTE4:
            glType = GL_UNSIGNED_BYTE;
            break;
        default:
            nbre_assert(FALSE);
        };

        GLenum usage = GetVBOUsage(buffer);

        switch(element->Semantic())
        {
        case NBRE_VertexElement::VES_POSITION:
            if (buffer->UseShadowBuffer())
            {
                GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                if (bufferID == 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glVertexPointer(typeCount, glType, stride, pBufferData);
                    GL_CHECK_ERROR;
                }
                else
                {
                    buffer->UpdateFromShadow();
                    glVertexPointer(typeCount, glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                    GL_CHECK_ERROR;
                }
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glVertexPointer(typeCount, glType, stride, pBufferData);
                GL_CHECK_ERROR;
            }
            break;

        case NBRE_VertexElement::VES_NORMAL:
            if (buffer->UseShadowBuffer())
            {
                GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                if (bufferID == 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glNormalPointer(glType, stride, pBufferData);
                    GL_CHECK_ERROR;
                }
                else
                {
                    buffer->UpdateFromShadow();
                    glNormalPointer(glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                    GL_CHECK_ERROR;
                }
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glNormalPointer(glType, stride, pBufferData);
                GL_CHECK_ERROR;
            }
            enableNormal = TRUE;
            break;

        case NBRE_VertexElement::VES_DIFFUSE:
            if (buffer->UseShadowBuffer())
            {
                GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                if (bufferID == 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glColorPointer(4, glType, stride, pBufferData);
                    GL_CHECK_ERROR;
                }
                else
                {
                    buffer->UpdateFromShadow();
                    glColorPointer(4, glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                    GL_CHECK_ERROR;
                }
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glColorPointer(4, glType, stride, pBufferData);
                GL_CHECK_ERROR;
            }
            enableColor = TRUE;
            break;

        case NBRE_VertexElement::VES_SPECULAR:
            //NOTE: not supported now
            break;
        case NBRE_VertexElement::VES_TEXTURE_COORDINATES:
            // fixed function matching to units based on tex_coord_set
            for (uint32 i = 0; i < mDisabledTexUnitsFrom; ++i)
            {
                // Only set this texture unit's texcoord pointer if it
                // is supposed to be using this element's index
                if (mTextureCoordIndex[i] == element->Index() )
                {
                    ClientActiveTexture(GL_TEXTURE0 + i);
                    if (buffer->UseShadowBuffer())
                    {
                        GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                        if (bufferID == 0)
                        {
                            glBindBuffer(GL_ARRAY_BUFFER, 0);
                            glTexCoordPointer(typeCount, glType, stride, pBufferData);
                            GL_CHECK_ERROR;
                        }
                        else
                        {
                            buffer->UpdateFromShadow();
                            glTexCoordPointer(typeCount, glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                            GL_CHECK_ERROR;
                        }
                    }
                    else
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glTexCoordPointer(typeCount, glType, stride, pBufferData);
                        GL_CHECK_ERROR;
                    }
                    mCurrentEnableTexcoordArray[i] = TRUE;
                }
            }
            break;

        default:
            break;
        }
    }
    EnableNormalArray(enableNormal);
    EnableColorArray(enableColor);
    EnableTexcoordArray(mCurrentEnableTexcoordArray);
}

void
NBRE_GLESRenderPal::DrawIndexedPrimitive( NBRE_PrimitiveType operationType, const NBRE_IndexData* indexData )
{
    nbre_assert(indexData);

    GLenum primType = GL_POINTS;
    switch (operationType)
    {
    case NBRE_PMT_TRIANGLE_LIST:
        primType = GL_TRIANGLES;
        break;

    case NBRE_PMT_LINE_LIST:
        primType = GL_LINES;
        break;

    case NBRE_PMT_TRIANGLE_STRIP:
        primType = GL_TRIANGLE_STRIP;
        break;

    case NBRE_PMT_TRIANGLE_FAN:
        primType = GL_TRIANGLE_FAN;
        break;

    case NBRE_PMT_POINT_LIST:
        primType = GL_POINTS;
        break;

    case NBRE_PMT_LINE_STRIP:
        primType = GL_LINE_STRIP;
        break;

    default:
        nbre_assert(FALSE);
    }

    NBRE_GLHardwareIndexBuffer* indexBuffer = static_cast<NBRE_GLHardwareIndexBuffer*>(indexData->IndexBuffer());

    if(indexBuffer != NULL)
    {
        void* pBufferData = indexBuffer->Lock(indexData->IndexStart() * indexBuffer->GetIndexSize(), NBRE_HardwareBuffer::HBL_READ_ONLY);
        GLenum indexType = GL_UNSIGNED_SHORT;
        switch(indexBuffer->GetType())
        {
        case NBRE_HardwareIndexBuffer::IT_16BIT:
            indexType = GL_UNSIGNED_SHORT;
            break;

        case NBRE_HardwareIndexBuffer::IT_8BIT:
            indexType = GL_UNSIGNED_BYTE;
            break;

        default:
            nbre_assert(FALSE);
            break;
        }

        if (indexBuffer->UseShadowBuffer())
        {
            GLenum usage = GetVBOUsage(indexBuffer);
            GLuint bufferID = indexBuffer->GenerateVBOBuffer(usage);
            if (bufferID == 0)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glDrawElements(primType, indexData->IndexCount(), indexType, pBufferData);
                GL_CHECK_ERROR;
            }
            else
            {
                indexBuffer->UpdateFromShadow();
                glDrawElements(primType, indexData->IndexCount(), indexType, VBO_BUFFER_OFFSET(indexData->IndexStart() * indexBuffer->GetIndexSize()));
                GL_CHECK_ERROR;
            }
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDrawElements(primType, indexData->IndexCount(), indexType, pBufferData);
            GL_CHECK_ERROR;
        }
    }
    else
    {
        glDrawArrays(primType, indexData->IndexStart(), indexData->IndexCount());
        GL_CHECK_ERROR;
    }
}

NBRE_Texture*
NBRE_GLESRenderPal::CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    return NBRE_NEW NBRE_GLTexture(this, &info, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_GLESRenderPal::CreateTexture( NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name )
{
    nbre_assert(image != NULL && numMipmaps != 0);
    return NBRE_NEW NBRE_GLTexture(this, image, faceCount, numMipmaps, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_GLESRenderPal::CreateTexture( NBRE_ITextureImage* textureLoader, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
   nbre_assert(textureLoader != NULL);
   return NBRE_NEW NBRE_GLTexture(this, textureLoader, texType, isUsingMipmap, name);
}

NBRE_HardwareIndexBuffer*
NBRE_GLESRenderPal::CreateIndexBuffer( NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    return NBRE_NEW NBRE_GLHardwareIndexBuffer(idxType, numIndexes, useShadowBuffer, usage);
}

NBRE_RenderTarget*
NBRE_GLESRenderPal::GetDefaultRenderTarget()
{
    if(mGlRenderWindow == NULL)
    {
        mGlRenderWindow = NBRE_NEW NBRE_GlRenderWindow;
    }
    return mGlRenderWindow;
}

void
NBRE_GLESRenderPal::SetProjectionTransform( const NBRE_Matrix4x4f& mtrx )
{
    float buffer[16];
    mtrx.CopyTo(buffer, FALSE);
    SetMatrixMode(GL_PROJECTION);
    glLoadMatrixf(buffer);
    GL_CHECK_ERROR;
}

NBRE_GpuProgram*
NBRE_GLESRenderPal::LoadGpuProgram( const char* vertShaderSrc, const char* fragShaderSrc )
{
    return NULL;
}

void
NBRE_GLESRenderPal::EnableDepthTest( nb_boolean bEnable )
{
    if (mDepthTest == bEnable) 
    {
        return;
    }
    mDepthTest = bEnable;
    
    if (bEnable)
    {
        glEnable(GL_DEPTH_TEST);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
        GL_CHECK_ERROR;
    }
}

void 
NBRE_GLESRenderPal::EnableAlphaTest( nb_boolean bEnable )
{
    if (mAlphaTest == bEnable) 
    {
        return;
    }
    mAlphaTest = bEnable;

    if (bEnable)
    {
        glEnable(GL_ALPHA_TEST);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
        GL_CHECK_ERROR;
    }
}

void 
NBRE_GLESRenderPal::EnableStencilTest( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_STENCIL_TEST);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
        GL_CHECK_ERROR;
    }
}

void
NBRE_GLESRenderPal::EnableLighting( nb_boolean bEnable )
{
    if (mLighting == bEnable) 
    {
        return;
    }
    mLighting = bEnable;

    if (bEnable)
    {
        glEnable(GL_LIGHTING);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_LIGHTING);
        GL_CHECK_ERROR;
    }
}

void
NBRE_GLESRenderPal::EnableTexture( uint32 stage, nb_boolean bEnable )
{
    nbre_assert(FALSE);
}

void
NBRE_GLESRenderPal::DisableTextureUnitsFrom( uint32 stage )
{
    for (uint32 i = stage; i < mDisabledTexUnitsFrom; ++i)
    {
        if (mTextureTypes[i] != 0) 
        {
            ActiveTexture(GL_TEXTURE0 + i);
            glDisable( mTextureTypes[i] );
            GL_CHECK_ERROR;
        }
        mTextureTypes[i] = 0;
    }
    mDisabledTexUnitsFrom = stage;
}

void
NBRE_GLESRenderPal::SelectTexture(uint32 stage, NBRE_Texture* texture)
{
    nbre_assert(texture);
    ActiveTexture(GL_TEXTURE0 + stage);
    NBRE_GLTexture *glTexture = static_cast<NBRE_GLTexture*>(texture);
    PAL_Error err = glTexture->Load();
    if(err != PAL_Ok && err != PAL_ErrAlreadyExist)
    {
        return;
    }
	GLenum lastTextureType = mTextureTypes[stage];
    GLenum textureType = glTexture->GetTextureTarget();
    mTextureTypes[stage] = textureType;
	if(lastTextureType != textureType)
	{
        if (lastTextureType != 0) 
        {
            glDisable( lastTextureType );
            GL_CHECK_ERROR;
        }
        glEnable(textureType);
        GL_CHECK_ERROR;
	}
    glBindTexture(textureType, glTexture->GetID());
    GL_CHECK_ERROR;
}
void 
NBRE_GLESRenderPal::SetTextureCoordSet(uint32 stage, uint32 index)
{
	mTextureCoordIndex[stage] = index;
}

void
NBRE_GLESRenderPal::EnableBlend( nb_boolean bEnable )
{
    if (mBlend == bEnable) 
    {
        return;
    }
    mBlend = bEnable;

    if (bEnable)
    {
        glEnable(GL_BLEND);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_BLEND);
        GL_CHECK_ERROR;
    }
}

NBRE_HardwareVertexBuffer*
NBRE_GLESRenderPal::CreateVertexBuffer( uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage )
{
    return NBRE_NEW NBRE_GLHardwareVertexBuffer(vertexSize, numVertices, useShadowBuffer, usage);
}

void
NBRE_GLESRenderPal::SetViewTransform( const NBRE_Matrix4x4d& mtrx )
{
    float buffer[16];
    mViewMatrix = mtrx;
    mWorldViewMatrix = NBRE_TypeConvertf::Convert(mViewMatrix * mWorldMatrix);
    mWorldViewMatrix.CopyTo(buffer, FALSE);

    SetMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(buffer);
    GL_CHECK_ERROR;
}
void
NBRE_GLESRenderPal::SetWorldTransform( const NBRE_Matrix4x4d& mtrx )
{
    float buffer[16];
    mWorldMatrix = mtrx;
    mWorldViewMatrix = NBRE_TypeConvertf::Convert(mViewMatrix * mWorldMatrix);
    mWorldViewMatrix.CopyTo(buffer, FALSE);
    SetMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(buffer);
    GL_CHECK_ERROR;
}

void
NBRE_GLESRenderPal::SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx )
{
    ActiveTexture(GL_TEXTURE0 + stage);
    float buffer[16];
    mtrx.CopyTo(buffer, FALSE);
    SetMatrixMode(GL_TEXTURE);
    glLoadMatrixf(buffer);
    GL_CHECK_ERROR;
}


void
NBRE_GLESRenderPal::SetTextureState(uint32 stage, const NBRE_TextureState& state)
{
    ActiveTexture(GL_TEXTURE0 + stage);
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_MIN_FILTER, ToGLTextureFilterType(state.GetMinFilter()));
    GL_CHECK_ERROR;
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_MAG_FILTER, ToGLTextureFilterType(state.GetMagFilter()));
    GL_CHECK_ERROR;
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_S, ToGLTextureAddressMode(state.GetWrapS()));
    GL_CHECK_ERROR;
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_T, ToGLTextureAddressMode(state.GetWrapT()));
    GL_CHECK_ERROR;
    if(mCapabilities.GetSupportAnisotropy())
    {
        glTexParameterf(mTextureTypes[stage], GL_TEXTURE_MAX_ANISOTROPY_EXT, mMaxAnisotropy);
        GL_CHECK_ERROR;
    }
}

void 
NBRE_GLESRenderPal::SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::SetClearDepth(float value)
{
    glClearDepthf(value);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::SetClearStencil(int32 value)
{
    glClearStencil(value);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::Clear(uint32 flags)
{
    GLbitfield mask = 0;
    if (flags & NBRE_CLF_CLEAR_COLOR)
    {
        EnableColorWrite(TRUE, TRUE, TRUE, TRUE);
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
    if(mask != 0)
    {
        glClear(mask);
        GL_CHECK_ERROR;
    }
}

void
NBRE_GLESRenderPal::UseLights( const NBRE_LightList& lights )
{
    NBRE_Matrix4x4d identity;
    SetWorldTransform(identity);

    uint32 lightNum = 0;
    for (NBRE_LightList::const_iterator i = lights.begin(); i != lights.end(); ++i)
    {
        GLenum glLight = GL_LIGHT0 + lightNum;
        const NBRE_Light& light = **i;
        
        float ambient[] = {light.GetAmbient().r, light.GetAmbient().g, light.GetAmbient().b, light.GetAmbient().a};
        float diffuse[] = {light.GetDiffuse().r, light.GetDiffuse().g, light.GetDiffuse().b, light.GetDiffuse().a};
        float specular[] = {light.GetSpecular().r, light.GetSpecular().g, light.GetSpecular().b, light.GetSpecular().a};

        glLightfv(glLight, GL_AMBIENT, ambient);
        GL_CHECK_ERROR;
        glLightfv(glLight, GL_DIFFUSE, diffuse);
        GL_CHECK_ERROR;
        glLightfv(glLight, GL_SPECULAR, specular);
        GL_CHECK_ERROR;

        float position[] = {light.GetPosition().x, light.GetPosition().y, light.GetPosition().z, 1.0f};
        if (light.GetType() == NBRE_LT_DIRECTIONAL)
        {
            position[0] = light.GetDirection().x;
            position[1] = light.GetDirection().y;
            position[2] = light.GetDirection().z;
            position[3] = 0.0f;
        }
        glLightfv(glLight, GL_POSITION, position);
        GL_CHECK_ERROR;

        if (light.GetType() == NBRE_LT_SPOTLIGHT)
        {
            float direction[] = {light.GetDirection().x, light.GetDirection().y, light.GetDirection().z};
            glLightfv(glLight, GL_SPOT_DIRECTION, direction);
            glLightf(glLight, GL_SPOT_EXPONENT, light.GetExponent());
            glLightf(glLight, GL_SPOT_CUTOFF, light.GetCutoff());
        }
        else
        {
            float direction[] = {0, 0, 1};
            glLightfv(glLight, GL_SPOT_DIRECTION, direction);
            glLightf(glLight, GL_SPOT_EXPONENT, 0);
            glLightf(glLight, GL_SPOT_CUTOFF, 180);
        }

        glEnable(glLight);
        GL_CHECK_ERROR;
        ++lightNum;
    }

    while (lightNum < mDisabledLightsFrom)
    {
        glDisable(GL_LIGHT0 + lightNum);
        GL_CHECK_ERROR;
        ++lightNum;
    }
    mDisabledLightsFrom = lights.size();
}

void
NBRE_GLESRenderPal::SetActiveLights( const NBRE_LightIndexList& indices )
{
    if (mDisabledLightsFrom == 0) 
    {
        return;
    }
    nb_boolean* lights = NBRE_NEW nb_boolean[mDisabledLightsFrom];
    nsl_memset(lights, 0, sizeof(nb_boolean) * mDisabledLightsFrom);

    for (NBRE_LightIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        nbre_assert(*it < mDisabledLightsFrom);
        lights[*it] = TRUE;
    }

    for (uint32 i = 0; i < mDisabledLightsFrom; ++i)
    {
        if (lights[i])
        {
            glEnable(GL_LIGHT0 + i);
            GL_CHECK_ERROR;
        }
        else
        {
            glDisable(GL_LIGHT0 + i);
            GL_CHECK_ERROR;
        }
    }
    NBRE_DELETE_ARRAY lights;
}

void
NBRE_GLESRenderPal::EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha )
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
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::EnableDepthWrite( nb_boolean bEnable )
{
    if (mDepthMask == bEnable) 
    {
        return;
    }
    mDepthMask = bEnable;
    glDepthMask(bEnable);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::EnableStencilWrite( uint32 maskBits )
{
    glStencilMask(maskBits);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::EnableFog( nb_boolean bEnable )
{
    if (mFog == bEnable) 
    {
        return;
    }
    mFog = bEnable;
    if (bEnable)
    {
        glEnable(GL_FOG);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_FOG);
        GL_CHECK_ERROR;
    }
}

void 
NBRE_GLESRenderPal::EnableCullFace( nb_boolean bEnable )
{
    if (mCullFace == bEnable) 
    {
        return;
    }
    mCullFace = bEnable;
    if (bEnable)
    {
        glEnable(GL_CULL_FACE);
        GL_CHECK_ERROR;
    }
    else
    {
        glDisable(GL_CULL_FACE);
        GL_CHECK_ERROR;
    }
}

void 
NBRE_GLESRenderPal::SetDepthFunc(NBRE_CompareFunction f)
{
    glDepthFunc(ToGLCompareFunction(f));
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::SetAlphaFunc(NBRE_CompareFunction f, float refValue)
{
    glAlphaFunc(ToGLCompareFunction(f), refValue);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::SetFog(NBRE_FogMode mode, float density, float fogStart, float fogEnd, float red, float green, float blue, float alpha)
{
    float color[] = {red, green, blue, alpha};
    glFogf(GL_FOG_MODE, static_cast<GLfloat>(ToGLFogMode(mode)));
    GL_CHECK_ERROR;
    glFogf(GL_FOG_DENSITY, density);
    GL_CHECK_ERROR;
    glFogf(GL_FOG_START, fogStart);
    GL_CHECK_ERROR;
    glFogf(GL_FOG_END, fogEnd);
    GL_CHECK_ERROR;
    glFogfv(GL_FOG_COLOR, color);
    GL_CHECK_ERROR;
}

void
NBRE_GLESRenderPal::SelectRenderTarget( NBRE_RenderTarget* rt )
{
    mCurrentRenderTarget = rt;
}

NBRE_RenderTarget*
NBRE_GLESRenderPal::CreateRenderTargetFromTexture( NBRE_Texture* texture, int32 face )
{
    return NBRE_NEW NBRE_GLRenderTexture(static_cast<NBRE_GLTexture*>(texture), face);
}

void
NBRE_GLESRenderPal::SetViewPort( int32 left, int32 top, uint32 width , uint32 height )
{
    if ((mLeft == left) && (mTop == top) && (mWidth == width) && (mHeight == height)) 
    {
        return;
    }
    mLeft = left;
    mTop = top;
    mWidth = width;
    mHeight = height;
    
    glViewport(left, top, width, height);
    GL_CHECK_ERROR;
}

void
NBRE_GLESRenderPal::SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor)
{
    if ((mSrcBlendFunction == srcFactor) && (mDestBlendFunction == dstFactor)) 
    {
        return;
    }
    mSrcBlendFunction = srcFactor;
    mDestBlendFunction = dstFactor;
    glBlendFunc(ToGLBlendFunction(srcFactor), ToGLBlendFunction(dstFactor));
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::SetColor(float r, float g, float b, float a)
{
}

nb_boolean 
NBRE_GLESRenderPal::BeginScene() 
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->BeginRender();
    }
    return TRUE;
}

void 
NBRE_GLESRenderPal::EndScene() 
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->EndRender();
    }
}

NBRE_VertexDeclaration*
NBRE_GLESRenderPal::CreateVertexDeclaration(void)
{
    return NBRE_NEW NBRE_VertexDeclaration();
}

void
NBRE_GLESRenderPal::SetWireframeMode(nb_boolean /*bLine*/)
{
    //Not supported in GLES
}

void
NBRE_GLESRenderPal::SetMatrixMode(uint32 mode)
{
    if(mMaxtrixMode == mode)
    {
        return;
    }
    mMaxtrixMode = mode;
    glMatrixMode(mode);
    GL_CHECK_ERROR;
}

void 
NBRE_GLESRenderPal::ActiveTexture(uint32 stage)
{
    if(mActiveTexture == stage)
    {
        return;
    }
    mActiveTexture = stage;
    glActiveTexture(stage);
    GL_CHECK_ERROR;
    
}

void
NBRE_GLESRenderPal::ClientActiveTexture(uint32 stage)
{
    if(mClientActiveTexture == stage)
    {
        return;
    }
    mClientActiveTexture = stage;
    glClientActiveTexture(stage);
    GL_CHECK_ERROR;
    
}

void
NBRE_GLESRenderPal::SetLineWidth(float width)
{
    glLineWidth(width);
}
