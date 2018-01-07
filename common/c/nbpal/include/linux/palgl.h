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

    @file     palgl.h
    @date     2/03/2012
    @defgroup PALGL_H PALGL

    @brief    palgl PAL OpenGL Library

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

#ifndef _PALGL_H_
#define _PALGL_H_

//#define GL_GLEXT_PROTOTYPES 1

#include <GLES/gl.h>
#include <GLES/glext.h>
#include "palexp.h"

#define GL_UNSIGNED_INT 0x1405

PAL_DEC void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);

PAL_DEC void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
                       GLfloat centerx, GLfloat centery, GLfloat centerz,
                       GLfloat upx, GLfloat upy, GLfloat upz);

PAL_DEC void InitGLFunctions();

extern PFNGLISRENDERBUFFEROESPROC glIsRenderbufferOES1;
extern PFNGLBINDRENDERBUFFEROESPROC glBindRenderbufferOES1;
extern PFNGLDELETERENDERBUFFERSOESPROC glDeleteRenderbuffersOES1;
extern PFNGLGENRENDERBUFFERSOESPROC glGenRenderbuffersOES1;
extern PFNGLRENDERBUFFERSTORAGEOESPROC glRenderbufferStorageOES1;
extern PFNGLGETRENDERBUFFERPARAMETERIVOESPROC glGetRenderbufferParameterivOES1;
extern PFNGLISFRAMEBUFFEROESPROC glIsFramebufferOES1;
extern PFNGLBINDFRAMEBUFFEROESPROC glBindFramebufferOES1;
extern PFNGLDELETEFRAMEBUFFERSOESPROC glDeleteFramebuffersOES1;
extern PFNGLGENFRAMEBUFFERSOESPROC glGenFramebuffersOES1;
extern PFNGLCHECKFRAMEBUFFERSTATUSOESPROC glCheckFramebufferStatusOES1;
extern PFNGLFRAMEBUFFERRENDERBUFFEROESPROC glFramebufferRenderbufferOES1;
extern PFNGLFRAMEBUFFERTEXTURE2DOESPROC glFramebufferTexture2DOES1;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC glGetFramebufferAttachmentParameterivOES1;
extern PFNGLGENERATEMIPMAPOESPROC glGenerateMipmapOES1;
extern PFNGLBLENDEQUATIONOESPROC glBlendEquationOES1;
extern PFNGLBLENDFUNCSEPARATEOESPROC glBlendFuncSeparateOES1;
extern PFNGLBLENDEQUATIONSEPARATEOESPROC glBlendEquationSeparateOES1;

#define glIsRenderbuffer glIsRenderbufferOES1
#define glBindRenderbuffer  glBindRenderbufferOES1
#define glDeleteRenderbuffers  glDeleteRenderbuffersOES1
#define glGenRenderbuffers  glGenRenderbuffersOES1
#define glRenderbufferStorage  glRenderbufferStorageOES1
#define glGetRenderbufferParameteriv  glGetRenderbufferParameterivOES1
#define glIsFramebuffer  glIsFramebufferOES1
#define glBindFramebuffer  glBindFramebufferOES1
#define glDeleteFramebuffers  glDeleteFramebuffersOES1
#define glGenFramebuffers  glGenFramebuffersOES1
#define glCheckFramebufferStatus  glCheckFramebufferStatusOES1
#define glFramebufferRenderbuffer  glFramebufferRenderbufferOES1
#define glFramebufferTexture2D  glFramebufferTexture2DOES1
#define glGetFramebufferAttachmentParameteriv  glGetFramebufferAttachmentParameterivOES1
#define glGenerateMipmap  glGenerateMipmapOES1
#define glBlendEquation  glBlendEquationOES1
#define glBlendFuncSeparate  glBlendFuncSeparateOES1
#define glBlendEquationSeparate  glBlendEquationSeparateOES1


#define GL_FRAMEBUFFER              GL_FRAMEBUFFER_OES
#define GL_RENDERBUFFER             GL_RENDERBUFFER_OES
#define GL_DEPTH_COMPONENT16        GL_DEPTH_COMPONENT16_OES
#define GL_DEPTH_ATTACHMENT         GL_DEPTH_ATTACHMENT_OES
#define GL_COLOR_ATTACHMENT0        GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_COMPLETE     GL_FRAMEBUFFER_COMPLETE_OES
#define GL_FRAMEBUFFER              GL_FRAMEBUFFER_OES
#define GL_BLEND_DST_ALPHA          GL_BLEND_DST_ALPHA_OES


#define GL_NORMAL_MAP                     GL_NORMAL_MAP_OES                 
#define GL_REFLECTION_MAP                 GL_REFLECTION_MAP_OES            
#define GL_TEXTURE_CUBE_MAP               GL_TEXTURE_CUBE_MAP_OES           
#define GL_TEXTURE_BINDING_CUBE_MAP       GL_TEXTURE_BINDING_CUBE_MAP_OES   
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      GL_MAX_CUBE_MAP_TEXTURE_SIZE_OES
#define GL_TEXTURE_GEN_MODE               GL_TEXTURE_GEN_MODE_OES
#define GL_TEXTURE_GEN_STR                GL_TEXTURE_GEN_STR_OES

#endif

/*! @} */
