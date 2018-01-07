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
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef PALGL_H_INCLUDED
#define PALGL_H_INCLUDED

#include <GLES/gl.h>
#include <GLES/glext.h>
#include "pal.h"

#define GL_UNSIGNED_INT 0x1405

PAL_DEC void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);

PAL_DEC void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
	              GLfloat centerx, GLfloat centery, GLfloat centerz,
	              GLfloat upx, GLfloat upy, GLfloat upz);

#define glGenFramebuffers           glGenFramebuffersOES
#define glBindFramebuffer           glBindFramebufferOES
#define glGenRenderbuffers          glGenRenderbuffersOES
#define glBindRenderbuffer          glBindRenderbufferOES
#define glRenderbufferStorage       glRenderbufferStorageOES
#define glFramebufferRenderbuffer   glFramebufferRenderbufferOES
#define glFramebufferTexture2D      glFramebufferTexture2DOES
#define glCheckFramebufferStatus    glCheckFramebufferStatusOES
#define glDeleteFramebuffers        glDeleteFramebuffersOES
#define glDeleteRenderbuffers       glDeleteRenderbuffersOES
#define glGenerateMipmap            glGenerateMipmapOES 

#define GL_FRAMEBUFFER              GL_FRAMEBUFFER_OES
#define GL_RENDERBUFFER             GL_RENDERBUFFER_OES
#define GL_DEPTH_COMPONENT16        GL_DEPTH_COMPONENT16_OES
#define GL_DEPTH_ATTACHMENT         GL_DEPTH_ATTACHMENT_OES
#define GL_COLOR_ATTACHMENT0        GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_COMPLETE     GL_FRAMEBUFFER_COMPLETE_OES
#define GL_BLEND_DST_ALPHA          GL_DST_ALPHA
#define GL_TEXTURE_CUBE_MAP              GL_TEXTURE_CUBE_MAP_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X   GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES

#endif
