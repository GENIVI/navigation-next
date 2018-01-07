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

@file nbred3d11gpuprogramfactory.h
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
#ifndef _NBRE_D3D11_GPU_PROGRAM_FACTORY_H_
#define _NBRE_D3D11_GPU_PROGRAM_FACTORY_H_
#include "nbresigleton.h"
#include "nbred3d11gpuprogram.h"
#include "nbrecache.h"

class NBRE_D3D11RenderPal;

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_D3D11RenderPal
*  @{
*/

class NBRE_D3D11GPUPorgramFactory
{
public:
    NBRE_D3D11GPUPorgramFactory(NBRE_D3D11RenderPal *pal);
    virtual ~NBRE_D3D11GPUPorgramFactory();

public:
    /// Create common shaders
    void Initialize();

    /// Get default shaders
    NBRE_D3D11GpuProgramPtr GetDefaultVS(uint32 vertexInputFormat);
    NBRE_D3D11GpuProgramPtr GetDefaultPS(uint32 vertexInputFormat);

private:
    void CreateAllDefaultShader();
    void CreateDefaultVSByBinFile(const char* filePath, uint32 key);
    void CreateDefaultPSByBinFile(const char* filePath, uint32 key);
    PAL_Error LoadShaderFile();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_D3D11GPUPorgramFactory);

private:
    typedef NBRE_Map<uint32, NBRE_D3D11GpuProgramPtr> ShaderCache;
    ShaderCache mVertexShaders;
    ShaderCache mPixelShaders;
    NBRE_D3D11RenderPal *mPal;
};

/*! @} */
/*! @} */
#endif