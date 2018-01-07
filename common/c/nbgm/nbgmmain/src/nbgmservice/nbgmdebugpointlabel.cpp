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
#include "nbgmdebugpointlabel.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbgmlayoutmanager.h"

NBGM_DebugPointLabel::NBGM_DebugPointLabel(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_BillboardSet* billboardSet, const NBRE_WString& text, const NBRE_FontMaterial& material, float fontHeight, const NBRE_Vector3d& position)
:NBGM_LayoutElement(nbgmContext, text)
,mBillboardSet(billboardSet)
,mPosition(position)
,mFontHeight(fontHeight)
,mFontMaterial(material)
,mPriority(0)
,mScale(1.0f)
,mParentNode(parentNode)
,mVisible(FALSE)
,mTextAreaEntity(NULL)
,mNeedUpdate(TRUE)
,mInitDone(FALSE)
{
    nbre_assert(parentNode != NULL && billboardSet != NULL);
}

NBGM_DebugPointLabel::~NBGM_DebugPointLabel()
{
    if(mNode)
    {
        mParentNode->RemoveChild(mNode.get());
        mBillboardSet->RemoveBillboard(mNode);
    }
}

nb_boolean
NBGM_DebugPointLabel::IsElementVisible()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (!layoutManager.IsInNearPlaneFrontSide(mPosition))
    {
        return FALSE;
    }

    if (mNeedUpdate)
    {   
        if (mTextAreaEntity)
        {
            mNode->RemoveEntity(mNode->GetEntities()[0]);
        }
        mTextAreaEntity = NBRE_NEW NBRE_TextAreaEntity(mNBGMContext.renderingEngine->Context(), mText, mFontMaterial, 1.0f, PAL_TA_CENTER);
        mNode->AddEntity(NBRE_BillboardEntityPtr(mTextAreaEntity));        
        mNeedUpdate = FALSE;
    }

    double pixelsPerUnit = 0;
    if (!layoutManager.PixelsPerUnit(mPosition, pixelsPerUnit))
    {
        return FALSE;
    }

    mScale = mFontHeight / pixelsPerUnit;
    mNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
    
    return TRUE;
}

void 
NBGM_DebugPointLabel::UpdateRegion()
{
}

nb_boolean 
NBGM_DebugPointLabel::IsElementRegionAvailable()
{
    return TRUE;
}

int32 
NBGM_DebugPointLabel::GetLayoutPriority() const
{
    return mPriority;
}

NBRE_Vector3d 
NBGM_DebugPointLabel::GetPosition()
{
    return mPosition;
}

void
NBGM_DebugPointLabel::AddToScene()
{
    if (!mInitDone)
    {
        NBGM_LayoutElement::AddToSence();
        mNode = mBillboardSet->AddBillboard();
        mNode->SetPosition(mPosition);
        mParentNode->AddChild(mNode);
        mInitDone = TRUE;
    }
}

void 
NBGM_DebugPointLabel::SetVisible(nb_boolean value)
{
    mVisible = value;
    if (mNode)
    {
        mNode->SetVisible(value);
    }
}

void
NBGM_DebugPointLabel::RefreshFontMaterial()
{
    if (mTextAreaEntity)
    {
        mTextAreaEntity->SetVisible(FALSE);
        mTextAreaEntity->Rebuild();
    }

    RemoveFromVisibleSet();
}

void 
NBGM_DebugPointLabel::AppendText(const NBRE_WString& text)
{
    mText += "\n";
    mText += text;
    mNeedUpdate = TRUE;
}
