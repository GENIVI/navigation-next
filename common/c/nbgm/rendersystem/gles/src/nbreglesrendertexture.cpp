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
#include "nbreglesrendertexture.h"
#include "nbreglestexture.h"
#include "nbreglescommon.h"

NBRE_GLESRenderTexture::NBRE_GLESRenderTexture(NBRE_GLESTexture* texture, int32 face)
{
    CreateFrameBuffer(texture, face);
}

NBRE_GLESRenderTexture::~NBRE_GLESRenderTexture()
{
    glDeleteRenderbuffers( 1, &mDepthbuffer);
    GL_CHECK_ERROR;

    glDeleteFramebuffers( 1, &mFramebuffer);
    GL_CHECK_ERROR;
}

void NBRE_GLESRenderTexture::CreateFrameBuffer(NBRE_GLESTexture* texture, int32 face) 
{
    glGenFramebuffers(1, &mFramebuffer);
    GL_CHECK_ERROR;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    GL_CHECK_ERROR;

    glGenRenderbuffers(1, &mDepthbuffer);
    GL_CHECK_ERROR;
    
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthbuffer);
    GL_CHECK_ERROR;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texture->GetTextureInfo().mWidth, texture->GetTextureInfo().mHeight);
    GL_CHECK_ERROR;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthbuffer);
    GL_CHECK_ERROR;

    switch(texture->GetTextureTarget())
    {
    case GL_TEXTURE_2D:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->GetID(), 0);
        GL_CHECK_ERROR;
        break;
    case GL_TEXTURE_CUBE_MAP:
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, texture->GetID(), 0);
        GL_CHECK_ERROR;
        break;
    default:
        nbre_assert(FALSE);
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    {
        nbre_assert( FALSE );  
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERROR;
}

void NBRE_GLESRenderTexture::BeginRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    GL_CHECK_ERROR;
}

void NBRE_GLESRenderTexture::EndRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GL_CHECK_ERROR;
}
