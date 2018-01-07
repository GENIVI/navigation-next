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
#include "nbgmpolyline2entity.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"

NBGM_Polyline2Entity::NBGM_Polyline2Entity(NBRE_Context& nbreContext, const NBRE_Polyline2f& polyline, const NBRE_Color& color)
    :mContext(nbreContext)
    ,mPolyline(polyline)
    ,mVertexData(NULL)
    ,mIndexData(NULL)
    ,mColor(color)
    ,mVisible(TRUE)
{
    mPassCache = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    mPassCache->SetEnableCullFace(FALSE);
    UpdateVertexBuffer();
}

NBGM_Polyline2Entity::~NBGM_Polyline2Entity()
{
    NBRE_DELETE mVertexData;
    mVertexData = NULL;
    NBRE_DELETE mIndexData;
    mIndexData = NULL;
}

void 
NBGM_Polyline2Entity::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if (!mVisible)
    {
        return;
    }

    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();
    renderOp.SetVertexData(mVertexData);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_LINE_STRIP);
    renderOp.SetPass(mPassCache.get());
    renderOp.SetTransform(&mTransform);
}

void 
NBGM_Polyline2Entity::SetTransform(const NBRE_Matrix4x4f& m)
{
    mTransform = NBRE_TypeConvertd::Convert(m);
}

void 
NBGM_Polyline2Entity::Rebuild(const NBRE_Polyline2f& polyline)
{
    mPolyline = polyline;
    UpdateVertexBuffer();
}

void 
NBGM_Polyline2Entity::UpdateVertexBuffer()
{
    NBRE_DELETE mVertexData;
    mVertexData = NULL;
    NBRE_DELETE mIndexData;
    mIndexData = NULL;

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (2 + 4), mPolyline.VertexCount(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (2) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, mPolyline.VertexCount(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    PolylineEntityVertex* pVertex = static_cast<PolylineEntityVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    for (uint16 i = 0; i < mPolyline.VertexCount(); ++i)
    {
        pVertex[i].position = mPolyline.Vertex(i);
        pVertex[i].color = mColor;
        pIndex[i] = i;
    }

    vertextBuff->UnLock();
    indexBuff->UnLock();
}

void 
NBGM_Polyline2Entity::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

int32 
NBGM_Polyline2Entity::OverlayId() const
{
    return -1;
}

nb_boolean 
NBGM_Polyline2Entity::Visible() const
{
    return mVisible;
}

void
NBGM_Polyline2Entity::UpdateSurfaceList()
{
}
