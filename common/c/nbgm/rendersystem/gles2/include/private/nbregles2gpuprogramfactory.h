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

    @file nbregles2gpuprogramfactory.h
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
#ifndef _NBRE_GLES2_GPU_PROGRAM_FACTORY_H_
#define _NBRE_GLES2_GPU_PROGRAM_FACTORY_H_

#include "nbresigleton.h"
#include "nbregles2gpuprogram.h"
#include "nbrecache.h"
#include "nbrecontext.h"

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_GLES2RenderPal
*  @{
*/

class NBRE_GLES2GPUPorgramFactory
{
public:
    NBRE_GLES2GPUPorgramFactory(PAL_Instance* pal);
    virtual ~NBRE_GLES2GPUPorgramFactory();

public:
    /// Create common shaders
    void Initialize();
    /// Create a shader by file name, if failed return nullptr
    NBRE_GLES2GpuProgramPtr CreateProgramFromFiles(const NBRE_String& vsFileName, const NBRE_String& psFileName);
    /// Create a shader by string, if failed return nullptr
    NBRE_GLES2GpuProgramPtr CreateProgram(const NBRE_String& name, const NBRE_String& vs, const NBRE_String& ps);
    /// Get a program by name, if not exist return nullptr
    NBRE_GLES2GpuProgramPtr GetByName(const NBRE_String &name);
    /// Get a program by the format of vertex data
    NBRE_GLES2GpuProgramPtr GetDefaultProgram();
    /// Lock the shader in order to avoid be released when refreshing
    PAL_Error Lock(const NBRE_String &name);
    /// Unlock the shader
    PAL_Error Unlock(const NBRE_String &name);
    /// Remove all shaders
    void RemoveAll();
    /// Remove all unused shaders (not include locked textures)
    void Refresh();

private:
    void CreateAllDefaultPrograms();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_GLES2GPUPorgramFactory);

private:
    typedef NBRE_Cache<NBRE_String, NBRE_GLES2GpuProgramPtr> ShaderCache;

    PAL_Instance* mPalInstance;
    ShaderCache mShaders;
};

/*! @} */
/*! @} */
#endif