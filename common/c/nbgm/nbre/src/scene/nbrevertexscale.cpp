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
#include "nbrevertexscale.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrevector2.h"

void
NBRE_VertexScale::ScaleBaseOnNormal( NBRE_VertexData& vertexData, float factor)
{
    NBRE_HardwareVertexBuffer* vertextBuf = NULL;
    NBRE_HardwareVertexBuffer* normalBuf = NULL;

    const NBRE_VertexDeclaration* declaration = vertexData.GetVertexDeclaration();
    const NBRE_VertexDeclaration::VertexElementList& vertexElementList = declaration->GetVertexElementList();
    for (NBRE_VertexDeclaration::VertexElementList::const_iterator iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
    {
        if ((*iter)->Semantic() == NBRE_VertexElement::VES_POSITION)
        {
            vertextBuf = vertexData.GetBuffer((*iter)->Source());
        }
        else if ((*iter)->Semantic() == NBRE_VertexElement::VES_USER_DEFINE)
        {
            normalBuf = vertexData.GetBuffer((*iter)->Source());
        }
    }
    if(vertextBuf == NULL || normalBuf == NULL)
    {
        return;
    }
    uint32 verteicsCount = vertextBuf->GetNumVertices();
    uint32 vertexStride = vertextBuf->GetVertexSize();
    uint32 normalStride  = normalBuf->GetVertexSize();
    uint8* vertexBufferData = static_cast<uint8*>(vertextBuf->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint8* normalBufferData = static_cast<uint8*>(normalBuf->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    for (uint32 i = 0; i < verteicsCount; ++i)
    {
        //get vertex
        float* vertexBuf = reinterpret_cast<float*>(vertexBufferData);
        NBRE_Vector2f vec(vertexBuf[0], vertexBuf[1]);
        //get normal
        float* normalBuf = reinterpret_cast<float*>(normalBufferData);
        NBRE_Vector2f normal(normalBuf[0], normalBuf[1]);

        //change vertex
        vec += normal * factor;

        vertexBuf[0] = vec.x;
        vertexBuf[1] = vec.y;

        //move to next vertex
        vertexBufferData += vertexStride;
        normalBufferData += normalStride;
    }
    vertextBuf->UnLock();
    normalBuf->UnLock();
}
