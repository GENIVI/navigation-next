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
#include "nbgmskywall.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbredefaultpass.h"

NBGM_Skywall::NBGM_Skywall(NBRE_Context& nbreContext, nb_boolean transparent, int32 overlayId):
    mNBREContext(nbreContext),
    mTransparent(transparent),
    mOverlayId(overlayId), 
    mVisible(TRUE),
    mShardVertex(NULL),
    mIndexData(NULL)
{
}

NBGM_Skywall::~NBGM_Skywall()
{
    NBRE_DELETE mShardVertex;
    NBRE_DELETE mIndexData;
}

void NBGM_Skywall::SetSize(float width, float height, float distance)
{
    NBRE_IRenderPal& rp = *(mNBREContext.mRenderPal);

    NBRE_HardwareVertexBuffer* vertextBuf = NULL;
    NBRE_HardwareVertexBuffer* texcordBuf = NULL;

    if(mShardVertex == NULL)
    {
        mShardVertex = NBRE_NEW NBRE_VertexData(0);
        NBRE_VertexDeclaration* decalration = mNBREContext.mRenderPal->CreateVertexDeclaration();
        mShardVertex->SetVertexDeclaration(decalration);

        //Create position buffer
        NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0,
                                                                  NBRE_VertexElement::VET_FLOAT3,
                                                                  NBRE_VertexElement::VES_POSITION, 0);
        decalration->GetVertexElementList().push_back(posElem);
        vertextBuf = rp.CreateVertexBuffer(sizeof(float) * 3, 4, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        mShardVertex->AddBuffer(vertextBuf);

        //Create texture coordinate buffer if sky is not mTransparent
        //if(!mTransparent)
        //@todo: Actually, the transparent sky doesn't need texture coordinate buffer indeed.
        // On Sumsun Galaxy 3 device, map rendering could be abnormal if not set texture buffer.
        // For now, we just comment out this line.
        // This change should be no harm except a little more memory.
        {
            NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(1, 0,
                                                                      NBRE_VertexElement::VET_FLOAT2,
                                                                      NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
            decalration->GetVertexElementList().push_back(texElem);
            texcordBuf = rp.CreateVertexBuffer(sizeof(float) * 2, 4, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
            mShardVertex->AddBuffer(texcordBuf);
        }
    }
    else
    {
        vertextBuf = mShardVertex->GetBuffer(0);
        if(mShardVertex->GetBufferCount() > 1)
        {
            texcordBuf = mShardVertex->GetBuffer(1);
        }
    }

    if(vertextBuf)
    {
        float vertex[] =
        {
            -width/2, distance, 0,
            width/2, distance, 0,
            -width/2, distance, height,
            width/2, distance, height,
        };
        
        vertextBuf->WriteData(0, sizeof(vertex), vertex, FALSE);
    }

    if(texcordBuf)
    {
        float tX = 1.f;
        float tY = 1.f;
        const float imageRatio = 2.f;
        if(height != 0.f && width != 0.f)
        {
            float r = width/height;
            if(r >= 1.f)
            {
                tX = r/imageRatio;
            }
            else if(r < 1.f)
            {
                tY = 1.f/r;
            }
        }
        float texcord[] =
        {
            0.f, 0.f,
            tX,  0.f,
            0.f, tY,
            tX,  tY,
        };
        texcordBuf->WriteData(0, sizeof(texcord), texcord, FALSE);
    }

    if(mIndexData == NULL)
    {
        static uint16 indics[] =
        {
            0, 1, 2,
            1, 3, 2,
        };
        NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, sizeof(indics)/sizeof(uint16), TRUE, NBRE_HardwareBuffer::HBU_STATIC);

        indexBuf->WriteData(0, sizeof(indics), indics, TRUE);

        mIndexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());
    }
}

void NBGM_Skywall::SetShader(const NBRE_ShaderPtr& shader)
{
    mShader = shader;
}


void NBGM_Skywall::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if(mShardVertex == NULL || mIndexData == NULL)
        return;

    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();

    renderOp.SetVertexData(mShardVertex);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_TRIANGLE_LIST);

    renderOp.SetPass(mShader->GetPass(0).get());
    renderOp.SetTransform(&mTransform);
}

int32 NBGM_Skywall::OverlayId() const
{
    return mOverlayId;
}

void NBGM_Skywall::NotifyCamera(const NBRE_Camera& camera)
{

    NBRE_Vector3f dir = camera.WorldRight();    
    float lq = (float)nsl_sqrt(dir.x * dir.x + dir.y * dir.y);

    if(NBRE_Math::IsZero(lq, 0.00001f))
    {
        //camera is vertical, do nothing
        mTransform.Identity();
    }
    else
    {
        float x = dir.x/lq;
        float y = dir.y/lq;
        //rotate sky to face camera
        mTransform = NBRE_Matrix4x4d(
          x,  -y, 0.f, 0.f,
          y,   x, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f);

    }
    //move sky to camera center
    NBRE_Vector3d pos = camera.WorldPosition();
    mTransform = NBRE_Transformationd::BuildTranslateMatrix(pos.x, pos.y, 0)*mTransform;
}

nb_boolean NBGM_Skywall::Visible() const
{
    return mVisible;
}

void NBGM_Skywall::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mNBREContext.mSurfaceManager);
}

void NBGM_Skywall::SetOverlayId(int32 overlayId)
{
    mOverlayId = overlayId;
}
