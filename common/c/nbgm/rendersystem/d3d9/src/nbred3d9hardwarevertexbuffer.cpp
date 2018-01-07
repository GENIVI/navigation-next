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
#include "nbred3d9hardwarevertexbuffer.h"
#include "nbrecommon.h"
#include "nbred3d9renderpal.h"

NBRE_D3D9HardwareVertexBuffer::NBRE_D3D9HardwareVertexBuffer(uint32 vertexSize, uint32 numVertices, nb_boolean needRead, NBRE_D3D9RenderPal *pal)
:NBRE_HardwareVertexBuffer(vertexSize, numVertices, FALSE, NBRE_HardwareBuffer::HBU_STATIC),mNeedRead(needRead),mD3D9Buffer(NULL),mPal(pal),mBufferData(NULL),mModified(TRUE)
{
    mBufferData = NBRE_NEW uint8[mSizeInBytes];
    nsl_memset(mBufferData, 0, mSizeInBytes);
}

NBRE_D3D9HardwareVertexBuffer::~NBRE_D3D9HardwareVertexBuffer()
{
    NBRE_DELETE_ARRAY mBufferData;
    if (mD3D9Buffer)
    {
        mD3D9Buffer->Release();
    }
}

IDirect3DVertexBuffer9*
NBRE_D3D9HardwareVertexBuffer::GetD3DVertextBuffer()const
{
    if (!mD3D9Buffer || mModified)
    {
        if(mD3D9Buffer == NULL)
        {
            IDirect3DDevice9 *device = mPal->GetActiveD3D9Device();
            nbre_assert(device);
            D3DPOOL pool = D3DPOOL_MANAGED;
            device->CreateVertexBuffer(static_cast<UINT>(mSizeInBytes), 0, 0, pool, &mD3D9Buffer, 0);
        }
        nbre_assert(mD3D9Buffer);
        void* pDest = NULL;
        mD3D9Buffer->Lock(0, mSizeInBytes, (void**)&pDest, 0);
        nsl_memcpy(pDest, mBufferData, mSizeInBytes);
        mD3D9Buffer->Unlock();
        mModified = FALSE;
    }
    return mD3D9Buffer;
}

void*
NBRE_D3D9HardwareVertexBuffer::Lock(uint32 offset, LockOptions lockOptions)
{
    mModified = TRUE;
    return (mBufferData+offset);
}

void*
NBRE_D3D9HardwareVertexBuffer::Lock(uint32 offset, uint32 length, LockOptions lockOptions)
{
    mModified = TRUE;
    return (mBufferData+offset);
}

void
NBRE_D3D9HardwareVertexBuffer::UnLock()const
{
}

void
NBRE_D3D9HardwareVertexBuffer::WriteData( uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer)
{
    mModified = TRUE;
    nbre_assert((offset + length) <= mSizeInBytes);
    nsl_memcpy(mBufferData+offset, pSource, length);
}

void
NBRE_D3D9HardwareVertexBuffer::ReadData( uint32 offset, uint32 length, void* pDest ) const
{
    nbre_assert((offset + length) <= mSizeInBytes);
    nsl_memcpy(pDest, mBufferData + offset, length);
}

void 
NBRE_D3D9HardwareVertexBuffer::Resize( uint32 length )
{
    if(mSizeInBytes < length)
    {
        mSizeInBytes = length;
        NBRE_DELETE[] mBufferData;
        mBufferData = NBRE_NEW uint8[mSizeInBytes];
        if (mD3D9Buffer)
        {
            mD3D9Buffer->Release();
            mD3D9Buffer = NULL;
        }
    }
}
