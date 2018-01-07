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
#include "nbreoverlay.h"
#include "nbreentity.h"

NBRE_Overlay::NBRE_Overlay(NBRE_Context& context):
mContext(context),
mIsAutoClearRenderProvider(TRUE),
mIsClearColor(FALSE),
mClearFlag(0),
mRenderOpCount(0)
{
}

NBRE_Overlay::~NBRE_Overlay()
{
    for(RenderOperationArray::iterator pRenderOp = mRenderOpList.begin(); pRenderOp != mRenderOpList.end(); ++pRenderOp)
    {
        NBRE_DELETE (*pRenderOp);
    }
}

void NBRE_Overlay::AppendRenderProvider(NBRE_IRenderOperationProvider* renderProvider)
{
    mPriorityRenderProviderSet.insert(NBRE_PriorityRenderProvider(renderProvider->GetPriority(), renderProvider));
}

void NBRE_Overlay::RemoveRenderProvider(NBRE_IRenderOperationProvider* renderProvider)
{
    for (NBRE_PriorityRenderProviderSet::iterator i = mPriorityRenderProviderSet.begin(); i != mPriorityRenderProviderSet.end(); ++i)
    {
        if ((i->second) == renderProvider)
        {
            mPriorityRenderProviderSet.erase(i);
            return;
        }
    }
}

void NBRE_Overlay::Render(const NBRE_Camera& camera)
{
    for(RenderQueueObserverArray::iterator i = mRenderQueueObserverArray.begin(); i != mRenderQueueObserverArray.end(); ++i)
    {
        (*i)->OnBegineRender(*this);
    }

    NBRE_IRenderPal& renderPal = *(mContext.mRenderPal);

    if(mIsClearColor)
    {
        renderPal.SetClearColor(mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
    }
    if(mClearFlag)
    {
        renderPal.Clear(mClearFlag);
    }

    if(mPriorityRenderProviderSet.size() == 0)
    {
        return;
    }

    mRenderOpCount = 0;
    for(NBRE_PriorityRenderProviderSet::iterator pRenderProvider = mPriorityRenderProviderSet.begin();
        pRenderProvider != mPriorityRenderProviderSet.end(); ++pRenderProvider)
    {
        pRenderProvider->second->NotifyCamera(camera);
        pRenderProvider->second->UpdateRenderQueue(*this);
    }

    if (mCamera.get() != NULL)
    {
        renderPal.SetProjectionTransform(mCamera->GetFrustum().ExtractMatrix());
        renderPal.SetViewTransform(mCamera->ExtractMatrix());
    }
    else
    {
        renderPal.SetProjectionTransform(camera.GetFrustum().ExtractMatrix());
        renderPal.SetViewTransform(camera.ExtractMatrix());
    }

    NBRE_RenderOperation::ResetLastRenderState(mContext);
    int32 i = 0;
    for(RenderOperationArray::iterator pRenderOp = mRenderOpList.begin(); (pRenderOp != mRenderOpList.end())&&(i < mRenderOpCount); ++pRenderOp, ++i)
    {
        (*pRenderOp)->Render();
    }
    for(RenderQueueObserverArray::iterator i = mRenderQueueObserverArray.begin(); i != mRenderQueueObserverArray.end(); ++i)
    {
        (*i)->OnEndRender(*this);
    }

}

void NBRE_Overlay::Clear()
{   
    mPriorityRenderProviderSet.clear();
}

void NBRE_Overlay::SetClearColor(const NBRE_Color& c) 
{ 
    mIsClearColor = TRUE;
    mClearColor = c; 
}

void NBRE_Overlay::SetClearFlag(uint32 clearFlag)
{
    mClearFlag = clearFlag;
}

void NBRE_Overlay::SetCamera(NBRE_CameraPtr camera)
{
    mCamera = camera;
}

void NBRE_Overlay::RegObserver(const NBRE_IRenderQueueObserver& observer)
{
    mRenderQueueObserverArray.push_back(&observer);
}

void NBRE_Overlay::UnregObserver(const NBRE_IRenderQueueObserver& observer)
{
    for(RenderQueueObserverArray::iterator i = mRenderQueueObserverArray.begin(); i != mRenderQueueObserverArray.end(); ++i)
    {
        if(*i == &observer)
        {
            mRenderQueueObserverArray.erase(i);
            break;
        }
    }
}

uint32 NBRE_Overlay::GetLastRenderedTriganglesNum() const
{
    int32 i = 0;
    uint32 num = 0;
    for(RenderOperationArray::const_iterator pRenderOp = mRenderOpList.begin(); (pRenderOp != mRenderOpList.end())&&(i < mRenderOpCount); ++pRenderOp, ++i)
    {
        num += (*pRenderOp)->GetTriangleNum();
    }
    return num;
}

NBRE_RenderOperation& NBRE_Overlay::CreateRenderOperation()
{
    if(mRenderOpCount < static_cast<int32>(mRenderOpList.size()))
    {
        return *mRenderOpList[mRenderOpCount++];
    }
    ++mRenderOpCount;
    NBRE_RenderOperation* op = NBRE_NEW NBRE_RenderOperation(mContext);
    mRenderOpList.push_back(op);
    return *op;
}

void NBRE_Overlay::SetAutoClearRenderProvider(nb_boolean autoClear)
{
    mIsAutoClearRenderProvider = autoClear;
}

nb_boolean NBRE_Overlay::IsAutoClearRenderProvider()
{
    return mIsAutoClearRenderProvider;
}

