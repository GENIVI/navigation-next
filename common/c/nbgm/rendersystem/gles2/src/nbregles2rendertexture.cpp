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
#include "nbregles2rendertexture.h"
#include "nbregles2texture.h"
#include "palgl2.h"

NBRE_GLES2RenderTexture::NBRE_GLES2RenderTexture(NBRE_GLES2Texture* texture,
                                                 int32 face,
                                                 nb_boolean enableFBOAA,
                                                 nb_boolean useStencilBuffer,
                                                 nb_boolean useDepthTexture):
    mTexture(texture),
    mFramebufferID(0),
    mDepthBufferID(0),
    mDepthTextureID(0),
    mDefaultFramebufferID(0),
    mEnableFBOAA(useDepthTexture ? FALSE : enableFBOAA),
    mUseDepthTexture(useDepthTexture),
    mUseStencilBuffer(useStencilBuffer)
{
    CreateFrameBuffer(texture, face);
}

NBRE_GLES2RenderTexture::~NBRE_GLES2RenderTexture()
{
    if (mUseDepthTexture && !mEnableFBOAA)
    {
        glDeleteRenderbuffers( 1, &mDepthBufferID);
    }
    else
    {
        glDeleteTextures(1, &mDepthTextureID);
    }

    glDeleteFramebuffers( 1, &mFramebufferID);
}

void NBRE_GLES2RenderTexture::CreateFrameBuffer(NBRE_GLES2Texture* texture, int32 face)
{
    GLint samples = 0;
    if(mEnableFBOAA)
    {
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
        if(samples <= 0 || texture->GetTextureTarget() != GL_TEXTURE_2D)
        {
            mEnableFBOAA = FALSE;
        }
    }

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFramebufferID);
    glGenFramebuffers(1, &mFramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);

    uint32 texWidth = texture->GetTextureInfo().mWidth;
    uint32 texHeight = texture->GetTextureInfo().mHeight;

    if (mUseDepthTexture)
    {
        if(mEnableFBOAA)
        {
            glGenRenderbuffers(1, &mDepthBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferID);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT16, texWidth, texHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBufferID);
        }
        else
        {
            glGenTextures(1, &mDepthTextureID);
            glBindTexture(GL_TEXTURE_2D, mDepthTextureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTextureID, 0);

            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
    else
    {
        if(mUseStencilBuffer)
        {
            glGenRenderbuffers(1, &mDepthBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferID);
            if(mEnableFBOAA)
            {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, texWidth, texHeight);
            }
            else
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, texWidth, texHeight);
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBufferID);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthBufferID);
        }
        else
        {
            glGenRenderbuffers(1, &mDepthBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferID);
            if(mEnableFBOAA)
            {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT16, texWidth, texHeight);
            }
            else
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texWidth, texHeight);
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBufferID);
        }
    }

    switch(texture->GetTextureTarget())
    {
    case GL_TEXTURE_2D:
        if(mEnableFBOAA)
        {
            glFramebufferTexture2DMultisample(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->GetID(), 0, samples);
        }
        else
        {
             glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->GetID(), 0);
        }
        break;
    case GL_TEXTURE_CUBE_MAP:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, texture->GetID(), 0);
        break;
    default:
        nbre_assert(FALSE);
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        nbre_assert(FALSE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFramebufferID);
}

void NBRE_GLES2RenderTexture::BeginRender()
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
}

void NBRE_GLES2RenderTexture::EndRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFramebufferID);
}

GLuint NBRE_GLES2RenderTexture::GetFramebufferId()
{
    return mFramebufferID;
}

GLuint NBRE_GLES2RenderTexture::GetDepthTextureId()
{
    return mDepthTextureID;
}

GLuint NBRE_GLES2RenderTexture::GetColorTextureId()
{
    return mTexture->GetID();
}

GLuint NBRE_GLES2RenderTexture::Width()
{
    return mTexture->GetWidth();
}

GLuint NBRE_GLES2RenderTexture::Height()
{
    return mTexture->GetHeight();
}
