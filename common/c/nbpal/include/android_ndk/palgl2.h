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
 
 
 @file palgl2.h
 @defgroup _PALGL2_H_ PALGL
 
 @brief Provides an interface to PAL OpenGL ES 2.0 functions
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
#ifndef _PALGL2_H_
#define _PALGL2_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include "pal.h"

#ifndef GL_MAX_SAMPLES_EXT
#define GL_MAX_SAMPLES_EXT 0x8D57
#endif

#define NEED_QUERY_FBOAA
#define QUERY_FBOAA_STRING "GL_EXT_multisampled_render_to_texture"

#ifndef glRenderbufferStorageMultisampleEXT
typedef void (*PFNGLRENDERBUFFERSTORAGEMULTISAMPLE)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) ;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLE glRenderbufferStorageMultisample;
#endif

#ifndef glFramebufferTexture2DMultisampleEXT
typedef void (*PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLE)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLE glFramebufferTexture2DMultisample;
#endif

// TODO: temp solution
// Currently there is a link error.These codes will be deleted if the error is resolved.
#ifndef ANDROID_NDK_SUPPORT_VAO
    #define ANDROID_NDK_SUPPORT_VAO
    inline void glGenVertexArrays(GLsizei n, GLuint *arrays){}
    inline void glDeleteVertexArrays(GLsizei n, const GLuint *arrays){}
    inline void glBindVertexArray(GLuint array){}
#endif

PAL_DEC void InitializeGLES2();

#define GL_DEPTH24_STENCIL8    GL_DEPTH24_STENCIL8_OES

#endif
