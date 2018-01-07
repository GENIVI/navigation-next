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
#include "nbregles2ssaoprogram.h"
#include "nbrelog.h"
#include "nbreiostream.h"
#include "nbregles2texture.h"
#include "nbregles2rendertexture.h"

static const char* vsSSAO =
    "attribute vec4 position;\n"
    "varying vec2 texcoordOut;\n"
    "void main() {\n"
    "   texcoordOut = position.xy * 0.5 + 0.5;\n"
    "   gl_Position = vec4(position.xy, 0.0, 1.0);\n"
    "}\n";

static const char* psSSAO =
    //"#define MAX_SAMPLES 8\n"//Should be added when create AO program
#ifndef WIN32
    "precision highp float;\n"
#endif
    "uniform sampler2D texDepth;\n"
    //"uniform sampler2D texRandDir;\n"
    "uniform mat4 projInv;\n"
    "uniform vec2 sizeInv;\n"
    "uniform vec2 sampleDirs[MAX_SAMPLES];\n"
    //"uniform vec2 randSampleScale;\n"//=randSize/screenSize
    "uniform float radius;\n"// in view space units
    "uniform float falloff;\n"// 1 to INF
    "uniform float bias;\n"//0 to 1
    "uniform float density;\n"//0 to INF
    "uniform mat4 projMat;\n"
    "varying vec2 texcoordOut;\n"
    "vec3 GetPosition(vec2 uv){\n"
    "   vec4 P = projInv * vec4(vec3(uv, texture2D(texDepth, uv).r) * 2.0 - 1.0, 1.0);\n"
    "   P.xyz /= P.w;\n"
    "   return P.xyz;"
    "}\n"
    "vec3 GetNormal(vec3 P, vec2 uv){\n"
    "   vec2 dx = vec2(sizeInv.x * 1.0, 0.0);\n"
    "   vec2 dy = vec2(0.0, sizeInv.y * 1.0);\n"
    "   vec3 PX = GetPosition(uv + dx) - P;\n"
    "   vec3 PY = GetPosition(uv + dy) - P;\n"
    "   vec3 N = cross(PX, PY);\n"
    "   return normalize(N);\n"
    "}\n"
    "\n"
    "float getAO(vec3 N, vec3 P, vec2 tc) {\n"
    "   vec3 pos = GetPosition(tc);\n"
    "   vec3 tp = pos.xyz - P.xyz;\n"
    "   float l = min(1.0, length(tp)/radius);\n"
    "   return max(0.0, dot(N,normalize(tp)) - bias) * max(0.0, 1.0 - pow(l, falloff));\n"
    "}\n"
    "\n"
    "vec2 radiusToUVOffset(float radius, float zEye){\n"
    "   return vec2(1.0 / projMat[0][0], 1.0 / projMat[1][1]) * (radius / -zEye * 0.5);\n"//z is negative in view frustum
    "}\n"
    "\n"
    "vec2 rand(vec2 co){\n"
    "   float n = 3.1415926 * 2.0 * fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);\n"
    "   return vec2(cos(n),sin(n));\n"
    "}\n"
    "void main() {\n"
    "   vec3 P = GetPosition(texcoordOut);\n"
    "   if (P.z < 0.0) {\n"//any geometry render at the pixel?
    "       vec3 N = GetNormal(P, texcoordOut);\n"
    //"       vec2 rn = normalize(texture2D(texRandDir, texcoordOut * randSampleScale).xy * 2.0 - 1.0);\n"
    "       vec2 rn = rand(texcoordOut);\n"
    "       float ao = 0.0;\n"
    "       vec2 baseOffset = radiusToUVOffset(radius, P.z);\n"
    "       for (int i = 0; i < MAX_SAMPLES; ++i)\n"
    "       {\n"
    "           vec2 uv = texcoordOut + (reflect(sampleDirs[i] * baseOffset, rn));\n"
    "           ao += getAO(N, P, uv);\n"
    "       }\n"
    "       ao = 1.0 - ao * density / float(MAX_SAMPLES);\n"
    "       gl_FragColor = vec4(ao, ao, ao, 1.0);\n"
    "   } else {\n"
    "       gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "   }\n"
    "}\n";

static const char* vsBlur =
    "attribute vec4 position;\n"
    "varying vec2 texcoordOut;\n"
    "void main() {\n"
    "   texcoordOut = position.xy * 0.5 + 0.5;\n"
    "   gl_Position = vec4(position.xy, 0.0, 1.0);\n"
    "}\n";
static const char* psBlurH =
#ifndef WIN32
    "precision highp float;\n"
#endif
    "uniform sampler2D texColor;\n"
    "uniform vec2 sizeInv;\n"
    "varying vec2 texcoordOut;\n"
    "void main() {\n"
    "   float r = 2.0;\n"
    "   gl_FragColor = texture2D(texColor, texcoordOut) * 0.4;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(sizeInv.x,0.0)*r) * 0.2;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(-sizeInv.x,0.0)*r) * 0.2;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(sizeInv.x*2.0,0.0)*r) * 0.1;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(-sizeInv.x*2.0,0.0)*r) * 0.1;\n"
    "}\n";
static const char* psBlurV =
#ifndef WIN32
    "precision highp float;\n"
#endif
    "uniform sampler2D texColor;\n"
    "uniform vec2 sizeInv;\n"
    "varying vec2 texcoordOut;\n"
    "void main() {\n"
    "   float r = 4.0;\n"
    "   gl_FragColor = texture2D(texColor, texcoordOut) * 0.4;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(0.0,sizeInv.y)*r) * 0.2;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(0.0,-sizeInv.y)*r) * 0.2;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(0.0,sizeInv.y*2.0)*r) * 0.1;\n"
    "   gl_FragColor += texture2D(texColor, texcoordOut + vec2(0.0,-sizeInv.y*2.0)*r) * 0.1;\n"
    "}\n";

NBRE_GLES2SSAOProgram::NBRE_GLES2SSAOProgram(NBRE_GLES2RenderPal* renderPal, int maxSamples, int randTextureSize, float radius, float falloff, float bias)
    :mRenderPal(renderPal)
    ,mMaxSamples(maxSamples)
    ,mRandTextureSize(randTextureSize)
    ,mRadius(radius)
    ,mFalloff(falloff)
    ,mBias(bias)
{
    Build();
}

NBRE_GLES2SSAOProgram::~NBRE_GLES2SSAOProgram()
{
}

void NBRE_GLES2SSAOProgram::Build()
{
    mRandDirs.clear();

    char header[50];
    sprintf(header, "#define MAX_SAMPLES %d\n", mMaxSamples);
    std::string buf(header);
    buf += psSSAO;
    mRenderPal->LoadGpuProgram("ssao", vsSSAO, buf);
    mAOProgram = mRenderPal->GetProgram("ssao");

    mRenderPal->LoadGpuProgram("blurH", vsBlur, psBlurH);
    mBlurHProgram = mRenderPal->GetProgram("blurH");

    mRenderPal->LoadGpuProgram("blurV", vsBlur, psBlurV);
    mBlurVProgram = mRenderPal->GetProgram("blurV");


    int samplesPerRing = 2;
    int rings = mMaxSamples / samplesPerRing;
    for (int j = 0; j < rings; ++j)
    {
        float r = (j + 1) / (float)rings;
        float offset = j % 2 ? (0.5f / samplesPerRing) : 0.0f;
        for (int i = 0; i < samplesPerRing; ++i)
        {
            float a = 3.1415926f * 2.0f * (i / (float)samplesPerRing + offset);
            float x = r * (float)cos(a);
            float y = r * (float)sin(a);
            mRandDirs.push_back(x);
            mRandDirs.push_back(y);
        }
    }

}

void NBRE_GLES2SSAOProgram::SetSamples(int maxSample)
{
    if (maxSample != mMaxSamples)
    {
        mMaxSamples = maxSample;
        Build();
    }
}

void NBRE_GLES2SSAOProgram::Render(NBRE_GLES2RenderTexture* out, NBRE_GLES2RenderTexture* depthBuffer, NBRE_GLES2RenderTexture* blurBuffer, NBRE_GLES2RenderTexture* aoBuffer, const float* projMatrix, const float* projMatrixInv, float radius, float falloff, float bias, float density)
{
    if (!mAOProgram)
    {
        return;
    }

    GLuint pid = mAOProgram->GetProgramId();
    glUseProgram(pid);

    glBindFramebuffer(GL_FRAMEBUFFER, aoBuffer->GetFramebufferId());
    glViewport(0, 0, aoBuffer->Width(), aoBuffer->Height());

    static const float vertices[] =
    {
        -1, -1, 0,  0,
        1, -1, 1,  0,
        -1,  1, 0,  1,
        1,  1, 1,  1,
    };

    glUniform2fv(glGetUniformLocation(pid, "sampleDirs"), mMaxSamples, &mRandDirs[0]);
    glUniform1f(glGetUniformLocation(pid, "radius"), radius);
    glUniform1f(glGetUniformLocation(pid, "falloff"), falloff);
    glUniform1f(glGetUniformLocation(pid, "bias"), bias);
    glUniform1f(glGetUniformLocation(pid, "density"), density);
    glUniform2f(glGetUniformLocation(pid, "sizeInv"),
        1.0f / aoBuffer->Width(),
        1.0f / aoBuffer->Height());
    glUniformMatrix4fv(glGetUniformLocation(pid, "projMat"), 1, GL_FALSE, projMatrix);
    glUniformMatrix4fv(glGetUniformLocation(pid, "projInv"), 1, GL_FALSE, projMatrixInv);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthBuffer->GetDepthTextureId());
    glUniform1i(glGetUniformLocation(pid, "texDepth"), 2);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(VS_ATTR_POSION_INDEX);
    glVertexAttribPointer(VS_ATTR_POSION_INDEX, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glViewport(0, 0, out->Width(), out->Height());

    // blur h pass
    glUseProgram(mBlurHProgram->GetProgramId());
    glBindFramebuffer(GL_FRAMEBUFFER, blurBuffer->GetFramebufferId());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, aoBuffer->GetColorTextureId());
    glUniform1i(glGetUniformLocation(mBlurHProgram->GetProgramId(), "texColor"), 2);
    glUniform2f(glGetUniformLocation(mBlurHProgram->GetProgramId(), "sizeInv"),
        1.0f / aoBuffer->Width(),
        1.0f / aoBuffer->Height());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // blur v pass
    glUseProgram(mBlurVProgram->GetProgramId());
    glBindFramebuffer(GL_FRAMEBUFFER, out->GetFramebufferId());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, blurBuffer->GetColorTextureId());
    glUniform1i(glGetUniformLocation(mBlurVProgram->GetProgramId(), "texColor"), 2);
    glUniform2f(glGetUniformLocation(mBlurVProgram->GetProgramId(), "sizeInv"),
        1.0f / out->Width(),
        1.0f / out->Height());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    GL_CHECK_ERROR
}
