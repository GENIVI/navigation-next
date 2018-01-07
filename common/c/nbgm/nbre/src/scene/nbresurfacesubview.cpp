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
#include "nbresurfacesubview.h"
#include "nbrerenderpal.h"
#include "nbrerendertarget.h"
#include "nbrebillboardnode.h"

NBRE_SurfaceSubView::NBRE_SurfaceSubView(NBRE_Context& context, int32 left, int32 top, uint32 width, uint32 height):
    mContext(context),
    mViewport(left, top, width, height)
{
}

NBRE_SurfaceSubView::~NBRE_SurfaceSubView()
{
    for (BillboardSetMap::iterator it = mBillboardSets.begin(); it != mBillboardSets.end(); ++it)
    {
        NBRE_DELETE it->second;
    }
}

NBRE_Viewport& NBRE_SurfaceSubView::Viewport()
{
    return mViewport;
}

void NBRE_SurfaceSubView::SetSceneManager(const NBRE_SceneManagerPtr& sceneMgr)
{
    mSceneMgr = sceneMgr;
}

void NBRE_SurfaceSubView::Update()
{
    mOverlayManager->Begin();
    NBRE_Overlay* overlay = mOverlayManager->GetOverlay();
    while(overlay)
    {
        if(overlay->IsAutoClearRenderProvider())
        {
            overlay->Clear();
        }
        mOverlayManager->Next();
        overlay = mOverlayManager->GetOverlay();
    }

    if(mSceneMgr)
    {
        mSceneMgr->UpdateOverlay(*mViewport.Camera(), *mOverlayManager);
    }

    RenderQueueVisitor visitor(*mOverlayManager, *mViewport.Camera());
    for(VisitableList::iterator pVisitable = mVisitables.begin(); pVisitable != mVisitables.end(); ++pVisitable)
    {
		if (*pVisitable != NULL)
			(*pVisitable)->Accept(visitor);
    }
}

void NBRE_SurfaceSubView::RenderQueueVisitor::Visit(NBRE_IRenderOperationProvider* updateRenderQueue, NBRE_SurfaceManager* /*surfaceManager*/)
{
    if(updateRenderQueue->Visible())
    {
        NBRE_Overlay* overlay = mOverlayManager.FindOverlay(updateRenderQueue->OverlayId());
        if(overlay)
        {
            updateRenderQueue->UpdateSurfaceList();
            overlay->AppendRenderProvider(updateRenderQueue);
        }
    }
}

void NBRE_SurfaceSubView::Render()
{
    NBRE_IRenderPal& renderPal = *(mContext.mRenderPal);

    mViewport.Apply(renderPal);
    // Set all lights in world space
    if(mSceneMgr)
    {
        renderPal.UseLights(mSceneMgr->GetLights());
    }

    mOverlayManager->Begin();
    NBRE_Overlay* overlay = mOverlayManager->GetOverlay();
    while(overlay)
    {
        overlay->Render(*mViewport.Camera());
        mOverlayManager->Next();
        overlay = mOverlayManager->GetOverlay();
    }
}

void NBRE_SurfaceSubView::AddVisitable(NBRE_IVisitable* visitable)
{
    mVisitables.push_back(visitable);
}

void NBRE_SurfaceSubView::AddBillboardSet(int32 overlayId)
{
    if (mBillboardSets.find(overlayId) == mBillboardSets.end())
    {
        NBRE_BillboardSet* bs = NBRE_NEW NBRE_BillboardSet(mContext, overlayId);
        mBillboardSets[overlayId] = bs;
        mVisitables.push_back(bs);
    }
}

NBRE_BillboardSet* NBRE_SurfaceSubView::GetBillboardSet(int32 overlayId)
{
    BillboardSetMap::iterator it = mBillboardSets.find(overlayId);
    if (it != mBillboardSets.end())
    {
        return it->second;
    }
    return NULL;
}

void NBRE_SurfaceSubView::SetOverlayManager(NBRE_IOverlayManager* overlayManager)
{
    mOverlayManager = overlayManager;
}
