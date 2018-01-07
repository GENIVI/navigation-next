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
#include "nbregles2gpuprogram.h"
#include "nbrelog.h"
#include "nbreiostream.h"

static void
BindAttributes(GLuint programId)
{
    glBindAttribLocation(programId, VS_ATTR_POSION_INDEX, VS_ATTR_POSION);
    GL_CHECK_ERROR;
    glBindAttribLocation(programId, VS_ATTR_COLOR_INDEX, VS_ATTR_COLOR);
    GL_CHECK_ERROR;
    glBindAttribLocation(programId, VS_ATTR_TEXTRUE_COORD0_INDEX, VS_ATTR_TEXTRUE_COORD0);
    GL_CHECK_ERROR;
    glBindAttribLocation(programId, VS_ATTR_TEXTRUE_COORD1_INDEX, VS_ATTR_TEXTRUE_COORD1);
    GL_CHECK_ERROR;
    glBindAttribLocation(programId, VS_ATTR_NORMAL_INDEX, VS_ATTR_NORMAL);
    GL_CHECK_ERROR;
}

NBRE_GLES2GPUPorgram::NBRE_GLES2GPUPorgram(NBRE_IOStream& vertexShaderSrc, NBRE_IOStream& pixelShaderSrc):
    mLoaded(FALSE),
    mVertexShaderId(0),
    mPixelShaderId(0),
    mProgramId(0),
    mVertexShaderSrc(vertexShaderSrc),
    mPixelShaderSrc(pixelShaderSrc)
{

}

NBRE_GLES2GPUPorgram::~NBRE_GLES2GPUPorgram()
{
    Unload();
}

void
NBRE_GLES2GPUPorgram::Unload()
{
    if(mLoaded)
    {
        glDeleteShader(mVertexShaderId);
        GL_CHECK_ERROR;
        glDeleteShader(mPixelShaderId);
        GL_CHECK_ERROR;
        glDeleteProgram(mProgramId);
        GL_CHECK_ERROR;
        mLoaded = FALSE;
    }
}

PAL_Error
NBRE_GLES2GPUPorgram::Load()
{
    if(mLoaded)
    {
        return PAL_Ok;
    }
    PAL_Error err = CreateShaders();
    err = (err == PAL_Ok)?CreateProgram():err;
    if(err == PAL_Ok)
    {
        mLoaded = TRUE;
    }
    return err;
}

static PAL_Error
LoadFile(NBRE_IOStream &file, char** src)
{
    uint32 size = file.GetSize();

    *src = NBRE_NEW char[size];
    nsl_memset(*src, 0, size);

    uint32 readSize = 0;
    PAL_Error err = file.Read((uint8*)*src, size, &readSize);

    if(err != PAL_Ok)
    {
        NBRE_DELETE[] *src;
        *src = NULL;
    }
    return err;
}

static void
CheckShaderLog(GLuint id)
{
    GLint infoLen = 0;
    glGetShaderiv ( id, GL_INFO_LOG_LENGTH, &infoLen );
    if ( infoLen > 1 )
    {
        char* infoLog = NBRE_NEW char[infoLen];

        glGetShaderInfoLog ( id, infoLen, NULL, infoLog );
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2GPUPorgram::LoadShader: Error compiling shader: %s", infoLog);
        NBRE_DELETE_ARRAY infoLog;
    }
}

static PAL_Error
LoadShader(GLenum shaderType, NBRE_IOStream &shaderSrc, GLuint &id)
{
    // Create the shader object
    id = glCreateShader(shaderType);
    if (id == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_GLES2GPUPorgram::CreateShader Failed!");
        return PAL_Failed;
    }

    // Compile the shader
    const char* src[1];
    char *temp = NULL;
    PAL_Error err = LoadFile(shaderSrc, &temp);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2GPUPorgram::CreateShaders: Load shader file failed! fileError = %d", err);
        return err;
    }

    *src = temp;
    glShaderSource(id, 1, src, NULL);
    glCompileShader(id);

    NBRE_DELETE_ARRAY temp;

    GLint compiled = 0;
    glGetShaderiv ( id, GL_COMPILE_STATUS, &compiled );
    if ( !compiled )
    {
        CheckShaderLog(id);
        glDeleteShader ( id );
        return PAL_Failed;
    }

    return PAL_Ok;
}

PAL_Error
NBRE_GLES2GPUPorgram::CreateShaders()
{
    nbre_assert(!mLoaded);
    PAL_Error err = LoadShader(GL_VERTEX_SHADER, mVertexShaderSrc, mVertexShaderId);
    err = (err != PAL_Ok)?err:LoadShader(GL_FRAGMENT_SHADER, mPixelShaderSrc, mPixelShaderId);
    return err;
}

static void
CheckProgramLog(GLuint id)
{
    GLint infoLen = 0;

    glGetProgramiv ( id, GL_INFO_LOG_LENGTH, &infoLen );

    if ( infoLen > 1 )
    {
        char* infoLog = NBRE_NEW char[infoLen];

        glGetProgramInfoLog ( id, infoLen, NULL, infoLog );
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_GLES2GPUPorgram::CheckProgramLog: Error linking program: %s", infoLog);
        NBRE_DELETE_ARRAY infoLog;
    }
}

PAL_Error
NBRE_GLES2GPUPorgram::CreateProgram()
{
    nbre_assert(!mLoaded && mVertexShaderId != 0 && mPixelShaderId != 0);
    // Create the program object
    mProgramId = glCreateProgram();
    if (mProgramId == 0)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_GLES2GPUPorgram::CreateProgram Failed!");
        return PAL_Failed;
    }

    // Attach the shaders
    glAttachShader(mProgramId, mVertexShaderId);
    glAttachShader(mProgramId, mPixelShaderId);

    BindAttributes(mProgramId);

    // Link the program
    glLinkProgram(mProgramId);

    GLint linked = 0;
    glGetProgramiv(mProgramId, GL_LINK_STATUS, &linked);
    if ( !linked )
    {
        CheckProgramLog(mProgramId);
        glDeleteProgram ( mProgramId );
        return PAL_Failed;
    }

    glValidateProgram(mProgramId);
    GLint validation = 0;
    glGetProgramiv(mProgramId, GL_VALIDATE_STATUS, &validation);
    if ( !validation )
    {
        CheckProgramLog(mProgramId);
        glDeleteProgram ( mProgramId );
        mProgramId = 0;
        return PAL_Failed;
    }
    return PAL_Ok;
}
