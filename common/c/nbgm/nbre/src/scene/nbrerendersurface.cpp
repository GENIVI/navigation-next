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
#include "nbrerendersurface.h"
#include "nbrerenderpal.h"
#include "nbrescenemanager.h"
#include "nbrerendertarget.h"
#include "nbresurfacemanager.h"

NBRE_RenderSurface::NBRE_RenderSurface(NBRE_Context& context): mContext(context), mRenderTarget(NULL)
{
}

NBRE_RenderSurface::~NBRE_RenderSurface()
{
    for(SubViewList::iterator pSubView = mSubViewList.begin(); pSubView != mSubViewList.end(); ++pSubView)
    {
        NBRE_DELETE *pSubView;
    }
    mContext.mSurfaceManager->SurfaceDetachRenderTarget(this, mRenderTarget);
}

NBRE_SurfaceSubView&
NBRE_RenderSurface::CreateSubView(int32 left, int32 top, uint32 width, uint32 height)
{
    NBRE_SurfaceSubView* subView = NBRE_NEW NBRE_SurfaceSubView(mContext, left, top, width, height);
    mSubViewList.push_back(subView);
    return *subView;
}

NBRE_SurfaceSubView&
NBRE_RenderSurface::GetSubView(int32 index)
{
    return *mSubViewList[index];
}

void
NBRE_RenderSurface::Update()
{
    for(SubViewList::iterator pSubView = mSubViewList.begin(); pSubView != mSubViewList.end(); ++pSubView)
    {
        (*pSubView)->Update();
    }
}

void
NBRE_RenderSurface::Render()
{
    if(mRenderTarget)
    {
        NBRE_IRenderPal& renderPal = *(mContext.mRenderPal);
        renderPal.SelectRenderTarget(mRenderTarget);
        renderPal.BeginScene();
        for(SubViewList::iterator pSubView = mSubViewList.begin(); pSubView != mSubViewList.end(); ++pSubView)
        {
            (*pSubView)->Render();
        }
        renderPal.EndScene();
    }
}

void
NBRE_RenderSurface::AttachTarget(NBRE_RenderTarget* renderTarget)
{
    if(mRenderTarget == renderTarget)
    {
        return;
    }

    if(mRenderTarget)
    {
        mContext.mSurfaceManager->SurfaceDetachRenderTarget(this, mRenderTarget);
    }

    mRenderTarget = renderTarget;

    if(mRenderTarget)
    {
        mContext.mSurfaceManager->SurfaceAttachToRenderTarget(this, mRenderTarget);
    }
}

NBRE_RenderTarget*
NBRE_RenderSurface::GetTarget()
{
    return mRenderTarget;
}

void
NBRE_RenderSurface::SwitchTarget(NBRE_RenderTarget* renderTarget)
{
    if(!mRenderTarget || !renderTarget)
    {
        return;
    }

    if(mRenderTarget == renderTarget)
    {
        return;
    }

    mContext.mSurfaceManager->SurfaceSwitchRenderTarget(mRenderTarget, renderTarget);

    mRenderTarget = renderTarget;
}
