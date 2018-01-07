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

    @file nbred3d9hardwarevertexbuffer.h
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
#ifndef _NBRE_D3D9_HARDWARE_VERTEX_BUFFER_H_
#define _NBRE_D3D9_HARDWARE_VERTEX_BUFFER_H_
#include "nbrehardwarevertexbuffer.h"
#include "nbred3d9renderpal.h"
#include <d3d9.h>

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_D3D9RenderPal
*  @{
*/
/** Specialization of HardwareVertexBuffer for Direct3D 9.0 */
class NBRE_D3D9HardwareVertexBuffer:public NBRE_HardwareVertexBuffer
{
friend class NBRE_D3D9RenderPal;

public:
    NBRE_D3D9HardwareVertexBuffer(uint32 vertexSize, uint32 numVertices, nb_boolean needRead, NBRE_D3D9RenderPal *pal);
    ~NBRE_D3D9HardwareVertexBuffer();

public:
    virtual void Resize(uint32 length);
    virtual void WriteData(uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer);
    virtual void ReadData(uint32 offset, uint32 length, void* pDest) const;

    virtual void* Lock(uint32 offset, LockOptions lockOptions);
    virtual void* Lock(uint32 offset, uint32 length, LockOptions lockOptions);
    virtual void UnLock()const;

    virtual void UpdateFromShadow(){}

    /// Get the D3D-specific vertext buffer
    IDirect3DVertexBuffer9* GetD3DVertextBuffer()const;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_D3D9HardwareVertexBuffer);

private:
    mutable IDirect3DVertexBuffer9 *mD3D9Buffer;
    NBRE_D3D9RenderPal *mPal;
    nb_boolean mNeedRead;
    uint8* mBufferData;
    mutable nb_boolean mModified;
};
/*! @} */
/*! @} */
#endif
