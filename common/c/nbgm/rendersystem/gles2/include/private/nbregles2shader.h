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

    @file nbregles2shader.h
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
#ifndef _NBRE_GLES2_SHADER_H_
#define _NBRE_GLES2_SHADER_H_

static const char DefualtVertexShader[] = 


"uniform  mat4 u_mvMatrix; \n"
"uniform  mat4 u_pMatrix; \n"
"uniform  vec4 u_texMatrix0; \n"
"uniform  vec4 u_texMatrix1; \n"
"attribute vec4 a_position; \n"
"attribute vec3 a_normal; \n"
"attribute vec2 a_tex0; \n"
"attribute vec2 a_tex1; \n"
"attribute vec4 a_color; \n"

"varying vec2 v_texCoord0; \n"
"varying vec2 v_texCoord1; \n"
"varying vec4 v_color; \n"

"void main()\n"
"{ \n"
"    gl_Position = u_mvMatrix*a_position; \n"
"    v_color = a_color; \n"
"    { v_texCoord0.x = a_tex0.x*u_texMatrix0.x + u_texMatrix0.y;  v_texCoord0.y = a_tex0.y*u_texMatrix0.z + u_texMatrix0.w;}\n"
"    { v_texCoord1.x = a_tex1.x;  v_texCoord1.y = a_tex1.y;}\n"
" \n"
"} \n"
;

static const char DefualtPixelShader[] = 
"#ifndef GL_ES\n"
    "#define mediump \n"
    "#define lowp \n"
    "#define highp \n"
"#endif\n"

"uniform sampler2D u_texSampler_1; \n"
"uniform sampler2D u_texSampler_2; \n"

"varying mediump vec2 v_texCoord0; \n"
"varying lowp vec2 v_texCoord1; \n"
"varying lowp vec4 v_color; \n"

"void main() \n"
"{ \n"
"  gl_FragColor = v_color; \n"
"  gl_FragColor *= texture2D(u_texSampler_1, v_texCoord0); \n"
//"  gl_FragColor *= texture2D(u_texSampler_2, v_texCoord1); \n"
"  gl_FragColor.a *= texture2D(u_texSampler_2, v_texCoord1).a; \n"
//"  if(u_alphaTest && gl_FragColor.a < u_alphaVaule) discard; \n"
"} \n"
;

static const char AOVertexShader[] = 
"#ifndef GL_ES\n"
    "#define mediump \n"
    "#define lowp \n"
    "#define highp \n"
"#endif\n"
    "attribute vec4 a_position; \n"
    "attribute vec2 a_tex0; \n"
    "attribute vec4 a_color; \n"
    "uniform highp mat4 u_texMatrix0; \n"
    "varying highp vec2 v_texCoord0; \n"
    "varying lowp vec4 v_color; \n"
    
    "void main()\n"
    "{ \n"
    "    gl_Position = vec4(a_position.xy, 0.0, 1.0); \n"
    "    v_texCoord0 = (u_texMatrix0 * vec4(a_tex0, 0.0, 1.0)).xy;\n"
    "    v_color = a_color; \n"
    "} \n"
    ;

static const char AOPixelShader[] = 

"#ifndef GL_ES\n"
    "#define mediump \n"
    "#define lowp \n"
    "#define highp \n"
"#endif\n"

    "uniform sampler2D u_texSampler_1; \n"
    "uniform sampler2D u_texSampler_2; \n"
    "varying highp vec2 v_texCoord0; \n"
    "varying lowp vec4 v_color; \n"

    "void main() \n"
    "{ \n"
    "  highp float ao = texture2D(u_texSampler_2, v_texCoord0).r;\n"
//    "  highp vec4 src = vec4(0.0, 0.0, 0.0, 1.0 - ao);\n"
//    "  highp vec4 dst = texture2D(u_texSampler_1, v_texCoord0);\n"
//    "  highp float outA = src.a + dst.a * (1.0 - src.a);\n"
//    "  gl_FragColor = vec4((src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a)) / outA, outA)*v_color;\n"
    "   gl_FragColor = vec4(0.0, 0.0, 0.0, 0.02 + (1.0 - ao) * 0.98);\n"
    //"   gl_FragColor = texture2D(u_texSampler_2, v_texCoord0);\n"
    "} \n"
    ;



static const char glowVertexShader[] = 

"#ifndef GL_ES\n"
    "#define mediump \n"
    "#define lowp \n"
    "#define highp \n"
"#endif\n"
    "attribute vec4 a_position; \n"
    "attribute vec2 a_tex0; \n"
    "attribute vec4 a_color; \n"
    "uniform highp mat4 u_texMatrix0; \n"
    "varying highp vec2 v_texCoord0; \n"
    "varying lowp vec4 v_color; \n"

    "void main()\n"
    "{ \n"
    "    gl_Position = vec4(a_position.xy, 0.0, 1.0); \n"
    "    v_texCoord0 = (u_texMatrix0 * vec4(a_tex0, 0.0, 1.0)).xy;\n"
    "    v_color = a_color; \n"
    "} \n"
    ;

static const char glowPixelShader[] = 

"#ifndef GL_ES\n"
    "#define mediump \n"
    "#define lowp \n"
    "#define highp \n"
"#endif\n"
    "#define MAX_SAMPLES 4\n"
    "uniform sampler2D u_texSampler_1; \n"
    "uniform sampler2D u_texSampler_2; \n"
    "uniform highp vec2 pixelSize; \n"
    "uniform highp vec4 scaleColor;\n"
    "uniform highp vec3 samples[MAX_SAMPLES*MAX_SAMPLES];\n"//=(xOffset,yOffset,weight)
    "uniform int numSamples;\n"
    "uniform highp float radius;\n"
    "varying highp vec2 v_texCoord0; \n"
    "varying lowp vec4 v_color; \n"

    "void main() \n"
    "{ \n"
    "  highp vec4 dst = vec4(0.0,0.0,0.0,0.0);\n"
    "  for(int i = 0; i < numSamples; ++i) {\n"
    "    highp vec3 s = samples[i];\n"
    "    highp vec2 uv = v_texCoord0 + (s.xy * radius * pixelSize);\n"
    "    dst += texture2D(u_texSampler_1, uv) * s.z;\n"
    "  }\n"
    "  gl_FragColor = dst * v_color * scaleColor;\n"
    "} \n"
    ;



static const char light3dVertexShader[] =
"uniform  mat4 mvp; \n"
"uniform  mat4 normalMat; \n"
"attribute vec4 a_position; \n"
"attribute vec3 a_normal; \n"
"attribute vec2 a_tex0; \n"
"attribute vec4 a_color; \n"
"varying vec2 v_texCoord0; \n"
"varying vec3 v_normal; \n"

"void main()\n"
"{ \n"
"    gl_Position = mvp*a_position; \n"
"    v_texCoord0 = a_tex0;\n"
"    vec4 N = vec4(a_normal,1.0);\n"
"    N = normalMat * N;\n"
"    v_normal = normalize(N.xyz);\n"
"} \n"
;

static const char light3dPixelShader[] =
"#ifdef GL_ES\n"
    "precision highp float; \n"
"#endif\n"

"uniform sampler2D u_texSampler_1; \n"
"uniform mat4 normalMat; \n"
"varying mediump vec2 v_texCoord0; \n"
"varying vec3 v_normal; \n"
"void main() \n"
"{ \n"
"  gl_FragColor = texture2D(u_texSampler_1, v_texCoord0); \n"
"   vec3 lightDir = vec3(0.5, 0.3, -2.0);\n"
"   vec4 skyColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"   vec4 groundColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"   vec4 D = texture2D(u_texSampler_1, v_texCoord0);\n"
"   vec3 N = normalize(v_normal);\n"
"   lightDir = (normalMat * vec4(normalize(lightDir),1.0)).xyz;\n"
"   vec3 L = normalize(lightDir);\n"
"   float NL = dot(N,L);\n"
"   float a = 0.5 + 0.5 * dot(N,L);\n"
"   gl_FragColor = mix(skyColor,groundColor,a) * D;\n"
"} \n"
;

#endif
