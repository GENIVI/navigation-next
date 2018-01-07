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

    @file nbregles2gpuprogram.h
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
#ifndef _NBRE_GLES2_GPU_PROGRAM_H_
#define _NBRE_GLES2_GPU_PROGRAM_H_

#include "nbregles2common.h"
#include "nbresharedptr.h"

class NBRE_IOStream;
/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_GLES2RenderPal
*  @{
*/

class NBRE_GLES2GPUPorgram
{
public:
    NBRE_GLES2GPUPorgram(NBRE_IOStream& vertexShaderBuf, NBRE_IOStream& pixelShaderBuf);
    virtual ~NBRE_GLES2GPUPorgram();

public:
    PAL_Error Load();
    void Unload();

    nb_boolean IsLoaded()const { return mLoaded; }

    GLuint GetVertexShaderId(void) { nbre_assert(mLoaded); return mVertexShaderId; }
    GLuint GetPixelShaderId(void)  { nbre_assert(mLoaded); return mPixelShaderId; }
    GLuint GetProgramId(void) { nbre_assert(mLoaded); return mProgramId; }

private:
    PAL_Error CreateShaders();
    PAL_Error CreateProgram();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_GLES2GPUPorgram);

private:
    nb_boolean mLoaded;
    GLuint  mVertexShaderId;
    GLuint  mPixelShaderId;
    GLuint  mProgramId;
    NBRE_IOStream& mVertexShaderSrc;
    NBRE_IOStream& mPixelShaderSrc;
};

typedef shared_ptr<NBRE_GLES2GPUPorgram> NBRE_GLES2GpuProgramPtr;

/*! @} */
/*! @} */
#endif