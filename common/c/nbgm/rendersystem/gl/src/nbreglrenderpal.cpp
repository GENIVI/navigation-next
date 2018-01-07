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
#include "nbreglrenderpal.h"
#include "nbreglhardwareindexbuffer.h"
#include "nbreglhardwarevertexbuffer.h"
#include "palgl.h"
#include "nbreglrenderwindow.h"
#include "nbreitextureimage.h"
#include "nbrematerial.h"
#include "nbregltexture.h"
#include "nbreglrendertexture.h"
#include "nbretypeconvert.h"
#include "nbreglcommon.h"

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
    GLint value;
    glGetIntegerv(GL_MAX_LIGHTS, &value);
    cap.SetMaxLights(value);
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &value);
    cap.SetMaxTextureUnits(value);

    char* glExt = (char*) glGetString(GL_EXTENSIONS);
    if (glExt)
    {
        NBRE_String exts((const char*) glExt);
        int i = exts.find("GL_OES_texture_cube_map");
        cap.SetSupportCubeMap(i!=-1);

        i = exts.find("GL_ARB_vertex_buffer_object");
        cap.SetSupportVBO(i!=-1);

        i = exts.find("GL_EXT_texture_filter_anisotropic");
        cap.SetSupportAnisotropy(i!=-1);

        i = exts.find("GL_ARB_multisample");
        cap.SetSupportMultipleSample(i!=-1);
    }
    else
    {
        cap.SetSupportCubeMap(FALSE);
        cap.SetSupportVBO(FALSE);
        cap.SetSupportAnisotropy(FALSE);
        cap.SetSupportMultipleSample(FALSE);
    }
}

NBRE_GlRenderPal::NBRE_GlRenderPal(): mGlRenderWindow(NULL), mCurrentRenderTarget(NULL), mMaxAnisotropy(0)
{
    GetGLCapabilities(mCapabilities);

    if(mCapabilities.GetSupportAnisotropy())
    {
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mMaxAnisotropy);
    }

    if(mCapabilities.GetSupportMultipleSample())
    {
        glEnable(GL_MULTISAMPLE);
    }

    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);

    mTextureTypes = NBRE_NEW GLenum[mCapabilities.GetMaxTextureUnits()];
    mTextureCoordIndex = NBRE_NEW uint32[mCapabilities.GetMaxTextureUnits()];
	for (uint32 i = 0; i < mCapabilities.GetMaxTextureUnits(); ++i)
	{
		mTextureTypes[i] = 0;
		mTextureCoordIndex[i] = 99;
        mCurrentEnableTexcoordArray.push_back(FALSE);
        mLastEnableTexcoordArray.push_back(FALSE);
        glClientActiveTexture(GL_TEXTURE0 + i);
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}
    mDisabledTexUnitsFrom = 0;

    glEnableClientState( GL_VERTEX_ARRAY );
    mLastEnableNormalArray = FALSE;
    glDisableClientState( GL_NORMAL_ARRAY );
    mLastEnableColorArray = FALSE;
    glDisableClientState( GL_COLOR_ARRAY );

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

NBRE_GlRenderPal::~NBRE_GlRenderPal()
{
    NBRE_DELETE_ARRAY mTextureTypes;
    NBRE_DELETE_ARRAY mTextureCoordIndex;

    NBRE_DELETE mGlRenderWindow;
}

void NBRE_GlRenderPal::SetMaterial(const NBRE_Material& material)
{
    float ambient[] = {material.Ambient().r, material.Ambient().g, material.Ambient().b, material.Ambient().a};
    float diffuse[] = {material.Diffuse().r, material.Diffuse().g, material.Diffuse().b, material.Diffuse().a};
    float specular[] = {material.Specular().r, material.Specular().g, material.Specular().b, material.Specular().a};
    float emission[] = {material.Emissive().r, material.Emissive().g, material.Emissive().b, material.Emissive().a};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.Shiness());
}

void
NBRE_GlRenderPal::EnableNormalArray(nb_boolean enable)
{
    if (enable != mLastEnableNormalArray)
    {
        if (enable)
        {
            glEnableClientState(GL_NORMAL_ARRAY);
        }
        else
        {
            glDisableClientState(GL_NORMAL_ARRAY);
        }
        mLastEnableNormalArray = enable;
    }
}

void
NBRE_GlRenderPal::EnableColorArray(nb_boolean enable)
{
    if (enable != mLastEnableColorArray)
    {
        if (enable)
        {
            glEnableClientState(GL_COLOR_ARRAY);
        }
        else
        {
            glDisableClientState(GL_COLOR_ARRAY);
        }
        mLastEnableColorArray = enable;
    }
}

void
NBRE_GlRenderPal::EnableTexcoordArray(const NBRE_Vector<nb_boolean>& enables)
{
    for(uint32 i = 0; i < enables.size(); ++i)
    {
        if (enables[i] != mLastEnableTexcoordArray[i])
        {
            glClientActiveTexture(GL_TEXTURE0 + i);
            if (enables[i])
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            }
            else
            {
                glDisableClientState( GL_TEXTURE_COORD_ARRAY );
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
NBRE_GlRenderPal::SetVertexBuffer(NBRE_VertexData* vertexData )
{
    nbre_assert(vertexData);
    nb_boolean enableNormal = FALSE;
    nb_boolean enableColor = FALSE;
    GL_CHECK_ERROR;

	for (uint32 i = 0; i < mCapabilities.GetMaxTextureUnits(); ++i)
	{
        mCurrentEnableTexcoordArray[i] = FALSE;
	}

    const NBRE_VertexDeclaration::VertexElementList& decl = vertexData->GetVertexDeclaration()->GetVertexElementList();
    NBRE_VertexDeclaration::VertexElementList::const_iterator elemEnd;
    elemEnd = decl.end();
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
            if (buffer->UseShadowBuffer() && mCapabilities.GetSupportVBO())
            {
                GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                if (bufferID == 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glVertexPointer(typeCount, glType, stride, pBufferData);
                }
                else
                {
                    buffer->UpdateFromShadow();
                    glVertexPointer(typeCount, glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                }
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glVertexPointer(typeCount, glType, stride, pBufferData);
            }
            break;

        case NBRE_VertexElement::VES_NORMAL:
            if (buffer->UseShadowBuffer() && mCapabilities.GetSupportVBO())
            {
                GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                if (bufferID == 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glNormalPointer(glType, stride, pBufferData);
                }
                else
                {
                    buffer->UpdateFromShadow();
                    glNormalPointer(glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                }
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glNormalPointer(glType, stride, pBufferData);
            }
            enableNormal = TRUE;
            break;

        case NBRE_VertexElement::VES_DIFFUSE:
            if (buffer->UseShadowBuffer() && mCapabilities.GetSupportVBO())
            {
                GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                if (bufferID == 0)
                {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glColorPointer(4, glType, stride, pBufferData);
                }
                else
                {
                    buffer->UpdateFromShadow();
                    glColorPointer(4, glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                }
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glColorPointer(4, glType, stride, pBufferData);
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
                    glClientActiveTextureARB(GL_TEXTURE0 + i);
                    if (buffer->UseShadowBuffer() && mCapabilities.GetSupportVBO())
                    {
                        GLuint bufferID = buffer->GenerateVBOBuffer(usage);
                        if (bufferID == 0)
                        {
                            glBindBuffer(GL_ARRAY_BUFFER, 0);
                            glTexCoordPointer(typeCount, glType, stride, pBufferData);
                        }
                        else
                        {
                            buffer->UpdateFromShadow();
                            glTexCoordPointer(typeCount, glType, stride, VBO_BUFFER_OFFSET(element->Offset()));
                        }
                    }
                    else
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                        glTexCoordPointer(typeCount, glType, stride, pBufferData);
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
NBRE_GlRenderPal::DrawIndexedPrimitive( NBRE_PrimitiveType operationType, const NBRE_IndexData* indexData )
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

    case NBRE_PMT_LINE_STRIP:
        primType = GL_LINE_STRIP;
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

        case NBRE_HardwareIndexBuffer::IT_32BIT:
            indexType = GL_UNSIGNED_INT;
            break;

        default:
            nbre_assert(FALSE);
            break;
        }

        if (indexBuffer->UseShadowBuffer() && mCapabilities.GetSupportVBO())
        {
            GLenum usage = GetVBOUsage(indexBuffer);
            GLuint bufferID = indexBuffer->GenerateVBOBuffer(usage);
            if (bufferID == 0)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glDrawElements(primType, indexData->IndexCount(), indexType, pBufferData);
            }
            else
            {
                indexBuffer->UpdateFromShadow();
                glDrawElements(primType, indexData->IndexCount(), indexType, VBO_BUFFER_OFFSET(indexData->IndexStart() * indexBuffer->GetIndexSize()));
            }
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDrawElements(primType, indexData->IndexCount(), indexType, pBufferData);
        }
    }
    else
    {
        glDrawArrays(primType, indexData->IndexStart(), indexData->IndexCount());
    }
}

NBRE_Texture*
NBRE_GlRenderPal::CreateTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
    return NBRE_NEW NBRE_GLTexture(this, &info, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_GlRenderPal::CreateTexture( NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType, const NBRE_String& name )
{
    nbre_assert(image != NULL && numMipmaps != 0);
    return NBRE_NEW NBRE_GLTexture(this, image, faceCount, numMipmaps, texType, isUsingMipmap, name);
}

NBRE_Texture*
NBRE_GlRenderPal::CreateTexture( NBRE_ITextureImage* textureLoader, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap, const NBRE_String& name)
{
   nbre_assert(textureLoader != NULL);
   return NBRE_NEW NBRE_GLTexture(this, textureLoader, texType, isUsingMipmap, name);
}


NBRE_HardwareIndexBuffer*
NBRE_GlRenderPal::CreateIndexBuffer( NBRE_HardwareIndexBuffer::IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage)
{
    return NBRE_NEW NBRE_GLHardwareIndexBuffer(idxType, numIndexes, useShadowBuffer, usage);
}

NBRE_RenderTarget*
NBRE_GlRenderPal::GetDefaultRenderTarget()
{
    if(mGlRenderWindow == NULL)
    {
        mGlRenderWindow = NBRE_NEW NBRE_GlRenderWindow;
    }
    return mGlRenderWindow;
}

void
NBRE_GlRenderPal::SetProjectionTransform( const NBRE_Matrix4x4f& mtrx )
{
    float buffer[16];
    mtrx.CopyTo(buffer, FALSE);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(buffer);
    glMatrixMode(GL_MODELVIEW);

}

NBRE_GpuProgram*
NBRE_GlRenderPal::LoadGpuProgram( const char* vertShaderSrc, const char* fragShaderSrc )
{
    return NULL;
}

void
NBRE_GlRenderPal::EnableDepthTest( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::EnableAlphaTest( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_ALPHA_TEST);
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::EnableStencilTest( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_STENCIL_TEST);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::EnableLighting( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_LIGHTING);
    }
    else
    {
        glDisable(GL_LIGHTING);
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::EnableTexture( uint32 stage, nb_boolean bEnable )
{
    glActiveTexture(GL_TEXTURE0 + stage);
    if (bEnable)
    {
        glEnable(GL_TEXTURE_2D);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::DisableTextureUnitsFrom( uint32 stage )
{
    for (uint32 i = stage; i < mDisabledTexUnitsFrom; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
    }
    mDisabledTexUnitsFrom = stage;
}

void
NBRE_GlRenderPal::SelectTexture(uint32 stage, NBRE_Texture* texture)
{
	GLenum lastTextureType = mTextureTypes[stage];
    glActiveTexture(GL_TEXTURE0 + stage);
    if(texture)
    {
        NBRE_GLTexture* glTexture = static_cast<NBRE_GLTexture*>(texture);
        GLenum textureType = glTexture->GetTextureTarget();

        glTexture->Load();

        mTextureTypes[stage] = textureType;
	    if(lastTextureType != textureType && lastTextureType != 0)
	    {
		    glDisable( lastTextureType );
	    }

	    glEnable(textureType);
        glBindTexture(textureType, glTexture->GetID());
    }
    else
    {
        if (lastTextureType != 0)
        {
            glDisable( lastTextureType );
        }
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::SetTextureCoordSet(uint32 stage, uint32 index)
{
	mTextureCoordIndex[stage] = index;
}

void
NBRE_GlRenderPal::EnableBlend( nb_boolean bEnable )
{
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
NBRE_GlRenderPal::CreateVertexBuffer( uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, NBRE_HardwareBuffer::Usage usage)
{
    return NBRE_NEW NBRE_GLHardwareVertexBuffer(vertexSize, numVertices, useShadowBuffer, usage);
}

void
NBRE_GlRenderPal::SetViewTransform( const NBRE_Matrix4x4d& mtrx )
{
    float buffer[16];
    mViewMatrix = mtrx;
    mWorldViewMatrix = NBRE_TypeConvertf::Convert(mViewMatrix * mWorldMatrix);
    mWorldViewMatrix.CopyTo(buffer, FALSE);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(buffer);

}

void
NBRE_GlRenderPal::SetWorldTransform( const NBRE_Matrix4x4d& mtrx )
{
    float buffer[16];
    mWorldMatrix = mtrx;
    mWorldViewMatrix = NBRE_TypeConvertf::Convert(mViewMatrix * mWorldMatrix);
    mWorldViewMatrix.CopyTo(buffer, FALSE);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(buffer);

    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::SetTextureTransform( uint32 stage, const NBRE_Matrix4x4f& mtrx )
{
    glActiveTexture(GL_TEXTURE0 + stage);
    float buffer[16];
    mtrx.CopyTo(buffer, FALSE);
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(buffer);
    glMatrixMode(GL_MODELVIEW);
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::SetTextureState(uint32 stage, const NBRE_TextureState& state)
{
    glActiveTexture(GL_TEXTURE0 + stage);
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_MIN_FILTER, ToGLTextureFilterType(state.GetMinFilter()));
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_MAG_FILTER, ToGLTextureFilterType(state.GetMagFilter()));
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_S, ToGLTextureAddressMode(state.GetWrapS()));
    glTexParameteri(mTextureTypes[stage], GL_TEXTURE_WRAP_T, ToGLTextureAddressMode(state.GetWrapT()));

    if(mCapabilities.GetSupportAnisotropy() && state.UsingAnisotropic())
    {
        glTexParameterf(mTextureTypes[stage], GL_TEXTURE_MAX_ANISOTROPY_EXT, mMaxAnisotropy);
    }
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::SetClearDepth(float value)
{
    glClearDepth(value);
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::SetClearStencil(int32 value)
{
    glClearStencil(value);
}

void
NBRE_GlRenderPal::Clear(uint32 flags)
{
    GLbitfield mask = 0;
    if (flags & NBRE_CLF_CLEAR_COLOR)
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (flags & NBRE_CLF_CLEAR_DEPTH)
    {
        glDepthMask(GL_TRUE);
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (flags & NBRE_CLF_CLEAR_STENCIL)
    {
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
    GL_CHECK_ERROR;
}

void
NBRE_GlRenderPal::UseLights( const NBRE_LightList& lights )
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
        glLightfv(glLight, GL_DIFFUSE, diffuse);
        glLightfv(glLight, GL_SPECULAR, specular);

        float position[] = {light.GetPosition().x, light.GetPosition().y, light.GetPosition().z, 1.0f};
        if (light.GetType() == NBRE_LT_DIRECTIONAL)
        {
            position[0] = light.GetDirection().x;
            position[1] = light.GetDirection().y;
            position[2] = light.GetDirection().z;
            position[3] = 0.0f;
        }
        glLightfv(glLight, GL_POSITION, position);

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
        ++lightNum;
    }

    while (lightNum < mCapabilities.GetMaxLights())
    {
        glDisable(GL_LIGHT0 + lightNum);
        ++lightNum;
    }
}

void
NBRE_GlRenderPal::SetActiveLights( const NBRE_LightIndexList& indices )
{
    nb_boolean* lights = NBRE_NEW nb_boolean[mCapabilities.GetMaxLights()];
    nsl_memset(lights, 0, sizeof(nb_boolean) * mCapabilities.GetMaxLights());

    for (NBRE_LightIndexList::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        nbre_assert(*it < mCapabilities.GetMaxLights());
        lights[*it] = TRUE;
    }

    for (uint32 i = 0; i < mCapabilities.GetMaxLights(); ++i)
    {
        if (lights[i])
        {
            glEnable(GL_LIGHT0 + i);
        }
        else
        {
            glDisable(GL_LIGHT0 + i);
        }
    }
    NBRE_DELETE_ARRAY lights;
}

void
NBRE_GlRenderPal::EnableColorWrite( nb_boolean enableRed, nb_boolean enableGreen, nb_boolean enableBlue, nb_boolean enableAlpha )
{
    glColorMask(enableRed, enableGreen, enableBlue, enableAlpha);
}

void
NBRE_GlRenderPal::EnableDepthWrite( nb_boolean bEnable )
{
    glDepthMask(bEnable ? GL_TRUE : GL_FALSE);
}

void
NBRE_GlRenderPal::EnableStencilWrite( uint32 maskBits )
{
    glStencilMask(maskBits);
}

void
NBRE_GlRenderPal::EnableFog( nb_boolean bEnable )
{
    if (bEnable)
    {
        glEnable(GL_FOG);
    }
    else
    {
        glDisable(GL_FOG);
    }
}

void
NBRE_GlRenderPal::EnableCullFace( nb_boolean bEnable )
{
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
NBRE_GlRenderPal::SetDepthFunc(NBRE_CompareFunction f)
{
    glDepthFunc(ToGLCompareFunction(f));
}

void
NBRE_GlRenderPal::SetAlphaFunc(NBRE_CompareFunction f, float refValue)
{
    glAlphaFunc(ToGLCompareFunction(f), refValue);
}

void
NBRE_GlRenderPal::SetFog(NBRE_FogMode mode, float density, float fogStart, float fogEnd, float red, float green, float blue, float alpha)
{
    float color[] = {red, green, blue, alpha};
    glFogf(GL_FOG_MODE, static_cast<GLfloat>(ToGLFogMode(mode)));
    glFogf(GL_FOG_DENSITY, density);
    glFogf(GL_FOG_START, fogStart);
    glFogf(GL_FOG_END, fogEnd);
    glFogfv(GL_FOG_COLOR, color);
}

void
NBRE_GlRenderPal::SelectRenderTarget( NBRE_RenderTarget* rt )
{
    mCurrentRenderTarget = rt;
}

NBRE_RenderTarget*
NBRE_GlRenderPal::CreateRenderTargetFromTexture( NBRE_Texture* texture, int32 face )
{
    return NBRE_NEW NBRE_GLRenderTexture(static_cast<NBRE_GLTexture*>(texture), face);
}

void
NBRE_GlRenderPal::SetViewPort( int32 left, int32 top, uint32 width , uint32 height )
{
    glViewport(left, top, width, height);
}

void
NBRE_GlRenderPal::SetBlendFunc(NBRE_BlendFunction srcFactor, NBRE_BlendFunction dstFactor)
{
    glBlendFuncSeparate(ToGLBlendFunction(srcFactor), ToGLBlendFunction(dstFactor), GL_ONE, GL_ONE);
}

void
NBRE_GlRenderPal::SetColor(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
}

nb_boolean
NBRE_GlRenderPal::BeginScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->BeginRender();
    }
    return TRUE;
}

void
NBRE_GlRenderPal::EndScene()
{
    if (mCurrentRenderTarget)
    {
        mCurrentRenderTarget->EndRender();
    }
}

NBRE_VertexDeclaration*
NBRE_GlRenderPal::CreateVertexDeclaration(void)
{
    return NBRE_NEW NBRE_VertexDeclaration();
}

void
NBRE_GlRenderPal::SetWireframeMode(nb_boolean bLine)
{
    if (bLine)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}
void
NBRE_GlRenderPal::ReadPixels(int x, int y, uint32 width, uint32 height, uint8 *buffer)  const
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
NBRE_GlRenderPal::SetLineWidth(float width)
{
    glLineWidth(width);
    GL_CHECK_ERROR;
}
