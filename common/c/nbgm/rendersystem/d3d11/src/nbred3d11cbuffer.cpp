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
#include "nbred3d11cbuffer.h"
#include "nbred3d11fixedfuncstate.h"
#include "nbrelog.h"
#include "nbred3d11renderpal.h"

NBRE_D3D11CBuffer::NBRE_D3D11CBuffer(uint32 size, uint32 slot, ID3D11Device *d3d11):mSize(size),mSlot(slot),mBuffer(NULL),mD3D11(d3d11)
{
    Create();
}

NBRE_D3D11CBuffer::~NBRE_D3D11CBuffer()
{
     NBRE_SAFE_RELEASE(mBuffer);
}

PAL_Error
NBRE_D3D11CBuffer::Create()
{
    if(mBuffer == NULL)
    {
        if(mSize%16 != 0)
        {
            mSize += 16 - (mSize % 16);
            NBRE_DebugLog(PAL_LogSeverityInfo, "NBRE_D3D11GenearlCBuffer::Create Warning! The size is invalid.");
        }
        if(mSize > D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11GenearlCBuffer::Create Failed! The size is invalid.");
            return PAL_ErrUnsupported;
        }
        D3D11_BUFFER_DESC cbDesc;
        cbDesc.ByteWidth = static_cast<UINT>(mSize);
        cbDesc.Usage = D3D11_USAGE_DEFAULT;
        cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags = 0;
        cbDesc.MiscFlags = 0;
        cbDesc.StructureByteStride = 0;
        HRESULT hr = mD3D11->CreateBuffer(&cbDesc, NULL, &mBuffer);
        if (FAILED(hr))
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_D3D11GenearlCBuffer::Create failed!");
            return PAL_Failed;
        }
    }
    return PAL_Ok;
}