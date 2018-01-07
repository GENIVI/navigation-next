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

#include "nbredebugray.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"

NBRE_Matrix4x4d NBRE_DebugRay::mIdentity;

NBRE_DebugRay::NBRE_DebugRay(NBRE_Context& context, int32 overlayId):
       mContext(context),
       mOverlayId(overlayId), mShardVertex(NULL), mIndexData(NULL),
       mPass(NBRE_NEW NBRE_Pass())
{
    *mPass = *(context.mDefaultPass->GetPass());
}

NBRE_DebugRay::~NBRE_DebugRay()
{
    NBRE_DELETE mShardVertex;
    NBRE_DELETE mIndexData;

}

void NBRE_DebugRay::SetLine(const NBRE_Point3f& pt1, const NBRE_Point3f& pt2)
{
    mPoint[0] = pt1;
    mPoint[1] = pt2;
}

void NBRE_DebugRay::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();

    NBRE_IRenderPal& rp = *(mContext.mRenderPal);
    NBRE_HardwareVertexBuffer* vertextBuf;

    if(mShardVertex == NULL)
    {
        NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
        NBRE_VertexElement* posElem1 = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
        decalration->GetVertexElementList().push_back(posElem1);

        vertextBuf = rp.CreateVertexBuffer(sizeof(float) * 3, 2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

        mShardVertex = NBRE_NEW NBRE_VertexData(0);
        mShardVertex->SetVertexDeclaration(decalration);
        mShardVertex->AddBuffer(vertextBuf);
    }
    else
    {
        vertextBuf = mShardVertex->GetBuffer(0);
    }

    float* v = (float*)vertextBuf->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);

    for(int32 i = 0; i < 2; ++i)
    {
        v[3*i+0] = mPoint[i].x;
        v[3*i+1] = mPoint[i].y;
        v[3*i+2] = mPoint[i].z;
    }
    vertextBuf->UnLock();

    if(mIndexData == NULL)
    {
        static uint16 indics[] =
        {
            0, 1,
        };
        NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, sizeof(indics)/sizeof(uint16), FALSE, NBRE_HardwareBuffer::HBU_STATIC);

        indexBuf->WriteData(0, sizeof(indics), indics, FALSE);

        mIndexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());
    }

    renderOp.SetVertexData(mShardVertex);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_LINE_LIST);

    renderOp.SetPass(mPass.get());
    renderOp.SetTransform(&mIdentity);

}

int32 NBRE_DebugRay::OverlayId() const
{
    return mOverlayId;
}


void NBRE_DebugRay::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

nb_boolean NBRE_DebugRay::Visible() const
{
    return TRUE;
}

void NBRE_DebugRay::SetColor(const NBRE_Color& c)
{
    mPass->SetColor(c);
}

void NBRE_DebugRay::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mContext.mSurfaceManager);
}
