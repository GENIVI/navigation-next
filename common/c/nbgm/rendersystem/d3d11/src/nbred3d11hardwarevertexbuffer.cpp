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
#include "nbred3d11hardwarevertexbuffer.h"
#include "nbrecommon.h"
#include "nbred3d11renderpal.h"

NBRE_D3D11HardwareVertexBuffer::NBRE_D3D11HardwareVertexBuffer(uint32 vertexSize, uint32 numVertices, nb_boolean needRead, Usage usage, NBRE_D3D11RenderPal *pal)
    :NBRE_HardwareVertexBuffer(vertexSize, numVertices, needRead, usage),mD3D11Buffer(NULL),mPal(pal)
{
     mBufferData = NBRE_NEW uint8[mSizeInBytes];
     nsl_memset(mBufferData, 0, mSizeInBytes);
     if(mSizeInBytes)
     {
          CreateD3DVertexBuffer();
     }
}

NBRE_D3D11HardwareVertexBuffer::~NBRE_D3D11HardwareVertexBuffer()
{
    NBRE_SAFE_RELEASE(mD3D11Buffer);
    NBRE_DELETE_ARRAY mBufferData;
}

void
NBRE_D3D11HardwareVertexBuffer::CreateD3DVertexBuffer()
{
    D3D11_BUFFER_DESC desc;
    desc.Usage            = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth        = static_cast<UINT>(mSizeInBytes);
    desc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags        = 0;
    mPal->GetActiveD3D11Device()->CreateBuffer(&desc, NULL, &mD3D11Buffer);
}

ID3D11Buffer*
NBRE_D3D11HardwareVertexBuffer::GetD3DVertextBuffer()
{
    if (!mD3D11Buffer || mShadowUpdate)
    {
        if(mD3D11Buffer == NULL)
        {
            CreateD3DVertexBuffer();
        }

        D3D11_MAPPED_SUBRESOURCE mappedSubResource = {0};
        if (SUCCEEDED(mPal->GetImmediateContext()->Map(mD3D11Buffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedSubResource)))
        {
            void *pRet = static_cast<void*>(static_cast<char*>(mappedSubResource.pData));
            nsl_memcpy(pRet, mBufferData, mSizeInBytes);
            mPal->GetImmediateContext()->Unmap(mD3D11Buffer, 0);
            mShadowUpdate = FALSE;
        }
    }
    return mD3D11Buffer;
}

void*
NBRE_D3D11HardwareVertexBuffer::Lock( uint32 offset, LockOptions lockOptions )
{
    nbre_assert(offset <= mSizeInBytes);
    return Lock(offset, mSizeInBytes - offset, lockOptions);
}

void*
NBRE_D3D11HardwareVertexBuffer::Lock( uint32 offset, uint32 length, LockOptions lockOption )
{
    nbre_assert((offset + length) <= mSizeInBytes);
    switch(lockOption)
    {
    case HBL_WRITE_ONLY:
        mShadowUpdate = TRUE;
        break;
    case HBL_READ_ONLY:
        break;
    default:
        nbre_assert(0);
    }

    mLockStart = offset;
    mLockSize = length;
    return mBufferData + offset;
}

void
NBRE_D3D11HardwareVertexBuffer::UnLock()const
{
}

void
NBRE_D3D11HardwareVertexBuffer::WriteData( uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer)
{
    void* bufferAddr = Lock(offset, length, HBL_WRITE_ONLY);
    nsl_memcpy(bufferAddr, pSource, length);
    UnLock();
}

void
NBRE_D3D11HardwareVertexBuffer::ReadData( uint32 offset, uint32 length, void* pDest ) const
{
    nbre_assert((offset + length) <= mSizeInBytes);
    nsl_memcpy(pDest, mBufferData + offset, length);
}


void 
NBRE_D3D11HardwareVertexBuffer::Resize( uint32 length )
{
    if(mSizeInBytes < length)
    {
        mSizeInBytes = length;
        NBRE_DELETE_ARRAY mBufferData;
        mBufferData = NBRE_NEW uint8[mSizeInBytes];
        NBRE_SAFE_RELEASE(mD3D11Buffer);
    }
}
