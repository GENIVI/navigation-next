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

    @file nbreglesrendertexture.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_GLES_RENDER_TEXTURE_H_
#define _NBRE_GLES_RENDER_TEXTURE_H_

#include "nbrerendertarget.h"
#include "palgl.h"
#include "nbretypes.h"

class NBRE_GLTexture;
/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_GLRenderPal
*  @{
*/
/** This class represents a RenderTarget that renders to a Texture. There is no 1 on 1
    relation between Textures and RenderTextures, as there can be multiple
    RenderTargets rendering to different mipmaps, faces (for cubemaps) or slices (for 3D textures)
    of the same Texture.
*/
class NBRE_GLRenderTexture:public NBRE_RenderTarget
{

public:
    NBRE_GLRenderTexture(NBRE_GLTexture* texture, int32 face);
    virtual ~NBRE_GLRenderTexture();

public:
    virtual void BeginRender();
    virtual void EndRender();

private:
    void CreateFrameBuffer(NBRE_GLTexture* texture, int32 face);

private:
    GLint mDefaultFramebufferID;
    GLuint mFramebuffer;
    GLuint mDepthbuffer;
};


#endif
