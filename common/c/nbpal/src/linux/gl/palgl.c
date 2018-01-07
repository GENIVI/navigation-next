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

    @file     palgl.c
    @date     2/03/2012
    @defgroup PAL_GL PAL GL Functions

    Platform-independent GL functions.

    Common implementation for Pal GL functions.
*/
/*
    See file description in header file.

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

#include "palgl.h"
#include "palmath.h"

PFNGLISRENDERBUFFEROESPROC glIsRenderbufferOES1 = 0;
PFNGLBINDRENDERBUFFEROESPROC glBindRenderbufferOES1 = 0;
PFNGLDELETERENDERBUFFERSOESPROC glDeleteRenderbuffersOES1 = 0;
PFNGLGENRENDERBUFFERSOESPROC glGenRenderbuffersOES1 = 0;
PFNGLRENDERBUFFERSTORAGEOESPROC glRenderbufferStorageOES1 = 0;
PFNGLGETRENDERBUFFERPARAMETERIVOESPROC glGetRenderbufferParameterivOES1 = 0;
PFNGLISFRAMEBUFFEROESPROC glIsFramebufferOES1 = 0;
PFNGLBINDFRAMEBUFFEROESPROC glBindFramebufferOES1 = 0;
PFNGLDELETEFRAMEBUFFERSOESPROC glDeleteFramebuffersOES1 = 0;
PFNGLGENFRAMEBUFFERSOESPROC glGenFramebuffersOES1 = 0;
PFNGLCHECKFRAMEBUFFERSTATUSOESPROC glCheckFramebufferStatusOES1 = 0;
PFNGLFRAMEBUFFERRENDERBUFFEROESPROC glFramebufferRenderbufferOES1 = 0;
PFNGLFRAMEBUFFERTEXTURE2DOESPROC glFramebufferTexture2DOES1 = 0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC glGetFramebufferAttachmentParameterivOES1 = 0;
PFNGLGENERATEMIPMAPOESPROC glGenerateMipmapOES1 = 0;
PFNGLBLENDEQUATIONOESPROC glBlendEquationOES1 = 0;
PFNGLBLENDFUNCSEPARATEOESPROC glBlendFuncSeparateOES1 = 0;
PFNGLBLENDEQUATIONSEPARATEOESPROC glBlendEquationSeparateOES1 = 0;

PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES1 = 0;
PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES1 = 0;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES1 = 0;


PAL_DEF void
InitGLFunctions()
{
 	glIsRenderbufferOES1 = (PFNGLISRENDERBUFFEROESPROC)eglGetProcAddress("glIsRenderbufferOES");
	glBindRenderbufferOES1 = (PFNGLBINDRENDERBUFFEROESPROC)eglGetProcAddress("glBindRenderbufferOES");
	glDeleteRenderbuffersOES1 = (PFNGLDELETERENDERBUFFERSOESPROC)eglGetProcAddress("glDeleteRenderbuffersOES");
	glGenRenderbuffersOES1 = (PFNGLGENRENDERBUFFERSOESPROC)eglGetProcAddress("glGenRenderbuffersOES");
	glRenderbufferStorageOES1 = (PFNGLRENDERBUFFERSTORAGEOESPROC)eglGetProcAddress("glRenderbufferStorageOES");
	glGetRenderbufferParameterivOES1 = (PFNGLGETRENDERBUFFERPARAMETERIVOESPROC)eglGetProcAddress("glGetRenderbufferParameterivOES");
	glIsFramebufferOES1 = (PFNGLISFRAMEBUFFEROESPROC)eglGetProcAddress("glIsFramebufferOES");
	glBindFramebufferOES1 = (PFNGLBINDFRAMEBUFFEROESPROC)eglGetProcAddress("glBindFramebufferOES");
	glDeleteFramebuffersOES1 = (PFNGLDELETEFRAMEBUFFERSOESPROC)eglGetProcAddress("glDeleteFramebuffersOES");
	glGenFramebuffersOES1 = (PFNGLGENFRAMEBUFFERSOESPROC)eglGetProcAddress("glGenFramebuffersOES");
	glCheckFramebufferStatusOES1 = (PFNGLCHECKFRAMEBUFFERSTATUSOESPROC)eglGetProcAddress("glCheckFramebufferStatusOES");
	glFramebufferRenderbufferOES1 = (PFNGLFRAMEBUFFERRENDERBUFFEROESPROC)eglGetProcAddress("glFramebufferRenderbufferOES");
	glFramebufferTexture2DOES1 = (PFNGLFRAMEBUFFERTEXTURE2DOESPROC)eglGetProcAddress("glFramebufferTexture2DOES");
	glGetFramebufferAttachmentParameterivOES1 = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOESPROC)eglGetProcAddress("glGetFramebufferAttachmentParameterivOES");
	glGenerateMipmapOES1 = (PFNGLGENERATEMIPMAPOESPROC)eglGetProcAddress("glGenerateMipmapOES");
	glBlendEquationOES1 = (PFNGLBLENDEQUATIONOESPROC)eglGetProcAddress("glBlendEquationOES");
	glBlendFuncSeparateOES1 = (PFNGLBLENDFUNCSEPARATEOESPROC)eglGetProcAddress("glBlendFuncSeparateOES");
	glBlendEquationSeparateOES1 = (PFNGLBLENDEQUATIONSEPARATEOESPROC)eglGetProcAddress("glBlendEquationSeparateOES");

	glBindVertexArrayOES1 = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	glGenVertexArraysOES1 = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	glDeleteVertexArraysOES1 = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");
}

#if GL_CAPABILITY_TEST
PAL_DEF void
gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
   GLfloat xmin, xmax, ymin, ymax;

    ymax = zNear * (GLfloat)tan(fovy * PI / 360);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustumx((GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
               (GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
               (GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536));

}

PAL_DEF void
gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
              GLfloat centerx, GLfloat centery, GLfloat centerz,
              GLfloat upx, GLfloat upy, GLfloat upz)
{
    GLfloat m[16];
    GLfloat x[3], y[3], z[3];
    GLfloat mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = (float)sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {/* mpichler, 19950515 */
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    /* X vector = Y cross Z */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */

    mag = (float)sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = (float)sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

#define M(row,col)  m[col*4+row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
#undef M
    {
        int a;
        GLfixed fixedM[16];
        for (a = 0; a < 16; ++a)
            fixedM[a] = (GLfixed)(m[a] * 65536);
        glMultMatrixx(fixedM);
    }

    /* Translate Eye to Origin */
    glTranslatex((GLfixed)(-eyex * 65536),
                 (GLfixed)(-eyey * 65536),
                 (GLfixed)(-eyez * 65536));
}
#endif
/*! @} */
