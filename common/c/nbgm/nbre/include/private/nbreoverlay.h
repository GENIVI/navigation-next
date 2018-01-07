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

/*!--------------------------------------------------------------------------

    @file nbreoverlay.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_OVERLAY_H_
#define _NBRE_OVERLAY_H_

#include "nbretypes.h"
#include "nbrelight.h"
#include "nbrerenderoperation.h"
#include "nbrefontmanager.h"
#include "nbrerenderqueueobserver.h"
#include "nbrecontext.h"

typedef NBRE_Pair<uint32, NBRE_IRenderOperationProvider*> NBRE_PriorityRenderProvider;
struct CompareRenderOperationFunction
{
    bool operator()(const NBRE_PriorityRenderProvider& a, const NBRE_PriorityRenderProvider& b) const
    {
        return a.first < b.first;
    }
};
typedef NBRE_Multiset<NBRE_PriorityRenderProvider, CompareRenderOperationFunction> NBRE_PriorityRenderProviderSet;

class NBRE_Overlay
{
public:
    explicit NBRE_Overlay(NBRE_Context& context);
    ~NBRE_Overlay();

public:
    void SetCamera(NBRE_CameraPtr camera);

    void SetClearColor(const NBRE_Color& c);
    void SetClearFlag(uint32 clearFlag);

    void AppendRenderProvider(NBRE_IRenderOperationProvider* renderProvider);
    void RemoveRenderProvider(NBRE_IRenderOperationProvider* renderProvider);

    void Render(const NBRE_Camera& camera);

    void SetAutoClearRenderProvider(nb_boolean autoClear);
    nb_boolean IsAutoClearRenderProvider();
    void Clear();

    void RegObserver(const NBRE_IRenderQueueObserver& observer);
    void UnregObserver(const NBRE_IRenderQueueObserver& observer);
    uint32 GetLastRenderedTriganglesNum() const;

    NBRE_RenderOperation& CreateRenderOperation();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_Overlay);

private:
    typedef NBRE_Vector<NBRE_RenderOperation*> RenderOperationArray;
    typedef NBRE_List<const NBRE_IRenderQueueObserver*> RenderQueueObserverArray;

private:
    NBRE_Context& mContext;
    nb_boolean mIsAutoClearRenderProvider;
    nb_boolean mIsClearColor;
    NBRE_Color mClearColor;
    uint32 mClearFlag;
    NBRE_CameraPtr mCamera;

    NBRE_PriorityRenderProviderSet mPriorityRenderProviderSet;

    RenderOperationArray mRenderOpList;
    int32 mRenderOpCount;
    RenderQueueObserverArray mRenderQueueObserverArray;
};

typedef NBRE_List<NBRE_Overlay*> NBRE_OverlayList;
typedef NBRE_Map<int32, NBRE_Overlay*> NBRE_OverlayMap;

#endif
