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

    @file nbgmmapviewprofiler.h
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

#ifndef _NBGM_MAPVIEW_IMPL_PROFILER_H_
#define _NBGM_MAPVIEW_IMPL_PROFILER_H_
#include "nbretypes.h"
#include "nbrerenderqueueobserver.h"
#include "nbgmmapview.h"
#include "nbgmmapviewprofile.h"

class NBRE_FileLogger;
enum TEMP_LAYER_ID
{
    TID_BR0 = 0,
    TID_BR1,
    TID_BR2,
    TID_BR3,
    TID_BR4,
    TID_BR5,
    TID_BR6,
    TID_BR7,
    TID_BR8,
    TID_BR9,
    TID_BR10,
    TID_BR11,
    TID_DVA = 48,
    TID_DVR = 80,
    TID_B3D = 144,
    TID_LM3D = 152,
    TID_LABLE,
    TID_OTHERS
};

class NBGM_MapViewProfiler:public NBRE_IRenderQueueObserver
{
public:
    NBGM_MapViewProfiler(int32 layerStride, int32 layerMain);
    ~NBGM_MapViewProfiler();

public:
    void Dump();
    void Dump(NBGM_MapViewProfile& profile);

    void Reset();
    void Update();

    void EnableVerboseProfiling(nb_boolean enable);
    void AddRenderQueue(uint8 originalId, NBRE_RenderQueue& queue);
    void AddOverlay(uint32 overlayId, NBRE_Overlay& overlay);
    void RemoveRenderQueue(NBRE_RenderQueue& queue );

    void SetLoadedTileNum(uint32 val) { mProfile.mLoadedTileNum = val; }
    void SetLayoutLabelTime(uint32 val) { mProfile.mLayoutLabelTime = val; }
    void SetUpdatePositionTime(uint32 val) { mProfile.mUpdatePositionTime = val; }
    void SetTotalRenderTime(uint32 val) { mProfile.mTotalRenderTime = val; }
    void SetTotalTriangleNum(uint32 val) {mProfile.mTotalTrianglesNum = val; }

    void SetFPS(float val) {mProfile.mFPS = val; }
    void SetPrintFPS(nb_boolean val) {mProfile.mPrintFPS = val; }

    const NBGM_MapViewProfile& GetMapViewProfile() const {return mProfile;}

    void BeginFrame();
    void PrepareFrame();
    void UpdateLabel();
    void UpdatePosition();
    void Render();
    void PostRender();
    void EndFrame(NBRE_FileLogger& fileLoger);

public:
    virtual void OnBegineRender(const NBRE_RenderQueue& queue) const;
    virtual void OnEndRender(const NBRE_RenderQueue& queue) const;

private:
    struct MapLayerProfile
    {
        uint8  mOrigianlId;
        NBRE_RenderQueue* mRenderQueue;
        uint32 beginRenderTime;

        uint32 mRenderTime;
        uint32 mTrianglesNum;
    };
    struct FrameProfile
    {
        uint32 beginTime;
        uint32 endTime;
        uint32 totalTime;
        uint32 prepareTime;
        uint32 updateLabelTime;
        uint32 updatePositionTime;
        uint32 renderTime;
        uint32 postRenderTime;

        void Dump(const NBGM_MapViewProfile& profile, NBRE_FileLogger& fileLoger) const;
    };
    typedef NBRE_Map<const void*, MapLayerProfile*> ProfileMap;
    typedef ProfileMap::const_iterator PMConstIterator;

    ProfileMap mMapLayerProfiles;
    nb_boolean mEnableVerbose;
    int32 mLayerStride;
    int32 mLayerMain;

    mutable NBGM_MapViewProfile mProfile;

    FrameProfile mCurrentFrame;

    uint32 mRenderedFrameCount;
    uint32 mRenderedFrameAccumulatedTime;
    uint32 mLastUpdateFPS;
};
#endif