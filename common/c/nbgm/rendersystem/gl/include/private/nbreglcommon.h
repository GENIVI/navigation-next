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

    @file nbreglcommon.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_GL_COMMON_H_
#define _NBRE_GL_COMMON_H_
#include "palgl.h"
#include "nbrelog.h"

#define ENABLE_GL_CHECK 1

#if ENABLE_GL_CHECK
#define GL_CHECK_ERROR \
    { \
    int e = glGetError(); \
    while (e != 0) \
        { \
        const char * errorString = ""; \
        switch(e) \
            { \
        case GL_INVALID_ENUM:       errorString = "GL_INVALID_ENUM";        break; \
        case GL_INVALID_VALUE:      errorString = "GL_INVALID_VALUE";       break; \
        case GL_INVALID_OPERATION:  errorString = "GL_INVALID_OPERATION";   break; \
        case GL_OUT_OF_MEMORY:      errorString = "GL_OUT_OF_MEMORY";       break; \
        default:                                                            break; \
            } \
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM: OpenGL error 0x%04X %s in %s at line %i\n", e, errorString, __FILE__, __LINE__); \
        } \
        e = glGetError(); \
    }
#else
#define GL_CHECK_ERROR {}
#endif

#endif
