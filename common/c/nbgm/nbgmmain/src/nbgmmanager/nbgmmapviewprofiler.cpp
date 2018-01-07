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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */
#include "nbgmmapviewprofiler.h"
#include "nbrecommon.h"
#include "palclock.h"
#include "nbreoverlay.h"
#include "nbrelog.h"
#include "nbrefilelogger.h"

NBGM_MapViewProfiler::NBGM_MapViewProfiler(int32 layerStride, int32 layerMain):
    mEnableVerbose(FALSE),
    mLayerStride(layerStride),
    mLayerMain(layerMain),
    mRenderedFrameCount(0),
    mRenderedFrameAccumulatedTime(0),
    mLastUpdateFPS(0)
{
    Reset();
}

NBGM_MapViewProfiler::~NBGM_MapViewProfiler()
{
    PMConstIterator end = mMapLayerProfiles.end();
    PMConstIterator i = mMapLayerProfiles.begin();
    for(; i!=end; ++i)
    {
        NBRE_DELETE i->second;
    }
}

void NBGM_MapViewProfiler::Reset()
{
    nsl_memset(&mProfile, 0, sizeof(NBGM_MapViewProfile));
}

void NBGM_MapViewProfiler::Update()
{
    PMConstIterator end = mMapLayerProfiles.end();
    PMConstIterator  i;
    mProfile.mTotalTrianglesNum = 0;
    for(i = mMapLayerProfiles.begin(); i!=end; ++i)
    {
        mProfile.mTotalTrianglesNum += i->second->mTrianglesNum;

        if(i->second->mOrigianlId <= TID_BR11)
        {
            mProfile.mRasterTrianglesNum +=  i->second->mTrianglesNum;
            mProfile.mRasterRenderTime += i->second->mRenderTime;
        }
        else if(i->second->mOrigianlId == TID_DVA)
        {
            mProfile.mAreaTrianglesNum +=  i->second->mTrianglesNum;
            mProfile.mAreaRenderTime += i->second->mRenderTime;
        }
        else if(i->second->mOrigianlId == TID_DVR)
        {
            mProfile.mRoadTrianglesNum +=  i->second->mTrianglesNum;
            mProfile.mRoadRenderTime += i->second->mRenderTime;
        }
        else if(i->second->mOrigianlId == TID_LM3D)
        {
            mProfile.mL3DTrianglesNum +=  i->second->mTrianglesNum;
            mProfile.m3DBRenderTime += i->second->mRenderTime;
        }
        else if(i->second->mOrigianlId == TID_B3D)
        {
            mProfile.mB3DTrianglesNum +=  i->second->mTrianglesNum;
            mProfile.m3DBRenderTime += i->second->mRenderTime;
        }
        else if(i->second->mOrigianlId == TID_LABLE)
        {
            mProfile.mLableTrianglesNum += i->second->mTrianglesNum;
            mProfile.mLableRenderTime += i->second->mRenderTime;
        }

        i->second->mTrianglesNum = 0;
        i->second->mRenderTime = 0;
    }
}

void
NBGM_MapViewProfiler::Dump(NBGM_MapViewProfile& profile)
{
    profile = mProfile;
}

void NBGM_MapViewProfiler::Dump()
{
    /*NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_MapViewProfile: =================================begin=================================\r\n"
                                        "Loaded tile number is ------------------------------- %u\r\n"
                                        "Render a frame costs -------------------------------- %u ms\r\n"
                                        "Rendered total triangles count is ------------------- %u",
                  mProfile.mLoadedTileNum,
                  mProfile.mTotalRenderTime,
                  mProfile.mTotalTrianglesNum);*/

    if(mEnableVerbose)
    {
        /*uint32 otherNum = mProfile.mTotalTrianglesNum
                        - mProfile.mRoadTrianglesNum
                        - mProfile.mAreaTrianglesNum
                        - mProfile.mB3DTrianglesNum
                        - mProfile.mL3DTrianglesNum
                        - mProfile.mRasterTrianglesNum
                        - mProfile.mLableTrianglesNum;

        uint32 otherTime = mProfile.mTotalRenderTime
                        - mProfile.mUpdatePositionTime
                        - mProfile.mLayoutLabelTime
                        - mProfile.mRoadRenderTime
                        - mProfile.mAreaRenderTime
                        - mProfile.m3DBRenderTime
                        - mProfile.mRasterRenderTime
                        - mProfile.mLableRenderTime;*/
        /*NBRE_DebugLog(PAL_LogSeverityMajor, "UpdatePositionTime is ------------------------------- %u ms\r\n"
                                            "LayoutLabelTime    is ------------------------------- %u ms\r\n"
                                            "Rendered road triangles count is -------------------- %u\r\n"
                                            "Rendered road cost ---------------------------------- %u ms\r\n\r\n"
                                            "Rendered area triangles count is -------------------- %u\r\n"
                                            "Rendered area cost ---------------------------------- %u ms\r\n\r\n"
                                            "Rendered 3D Building triangles count is ------------- %u\r\n"
                                            "Rendered 3D landmarks triangles count is ------------ %u\r\n"
                                            "Rendered 3D map layer cost -------------------------- %u ms\r\n\r\n"
                                            "Rendered raster tiles triangles count is ------------ %u\r\n"
                                            "Rendered raster tiles cost -------------------------- %u ms\r\n\r\n"
                                            "Rendered lables triangles count is ------------------ %u\r\n"
                                            "Rendered lables cost -------------------------------- %u ms\r\n\r\n"
                                            "Rendered others triangles count is ------------------ %u\r\n"
                                            "Rendered others triangles cost ---------------------- %u ms",
                      mProfile.mUpdatePositionTime,
                      mProfile.mLayoutLabelTime,
                      mProfile.mRoadTrianglesNum,
                      mProfile.mRoadRenderTime,
                      mProfile.mAreaTrianglesNum,
                      mProfile.mAreaRenderTime,
                      mProfile.mB3DTrianglesNum,
                      mProfile.mL3DTrianglesNum,
                      mProfile.m3DBRenderTime,
                      mProfile.mRasterTrianglesNum,
                      mProfile.mRasterRenderTime,
                      mProfile.mLableTrianglesNum,
                      mProfile.mLableRenderTime,
                      otherNum,
                      otherTime);*/
    }
}

void NBGM_MapViewProfiler::EnableVerboseProfiling(nb_boolean enable)
{
    mEnableVerbose = enable;
}

void NBGM_MapViewProfiler::OnBegineRender( const NBRE_RenderQueue& queue ) const
{
    ProfileMap::const_iterator pProfile = mMapLayerProfiles.find(&queue);
    if(pProfile != mMapLayerProfiles.end())
    {
        pProfile->second->beginRenderTime = PAL_ClockGetTimeMs();
    }
}

void NBGM_MapViewProfiler::OnEndRender( const NBRE_RenderQueue& queue ) const
{
    ProfileMap::const_iterator pProfile = mMapLayerProfiles.find(&queue);
    if(pProfile != mMapLayerProfiles.end())
    {
        pProfile->second->mRenderTime = PAL_ClockGetTimeMs() - pProfile->second->beginRenderTime;
        pProfile->second->mTrianglesNum = queue.GetLastRenderedTriganglesNum();
    }
}

void NBGM_MapViewProfiler::AddRenderQueue(uint8 originalId, NBRE_RenderQueue& queue )
{
    if(mMapLayerProfiles.find(&queue) != mMapLayerProfiles.end())
    {
        return;
    }
    MapLayerProfile* prifile = NBRE_NEW MapLayerProfile;
    prifile->mOrigianlId =originalId;
    prifile->mRenderQueue = &queue;
    prifile->mRenderTime = 0;
    prifile->mTrianglesNum = 0;
    mMapLayerProfiles[&queue] = prifile;

    queue.RegObserver(*this);
}

void NBGM_MapViewProfiler::AddOverlay(uint32 overlayId, NBRE_Overlay& overlay)
{
    uint8 originalId = static_cast<uint8> ((overlayId - mLayerMain) / mLayerStride);
    AddRenderQueue(originalId, overlay);
}

void NBGM_MapViewProfiler::RemoveRenderQueue(NBRE_RenderQueue& queue )
{
    ProfileMap::iterator iter = mMapLayerProfiles.find(&queue);
    if(iter == mMapLayerProfiles.end())
    {
        return;
    }
    queue.UnregObserver(*this);
    NBRE_DELETE iter->second;
    mMapLayerProfiles.erase(iter);
}

void NBGM_MapViewProfiler::BeginFrame()
{
    nsl_memset(&mCurrentFrame, 0, sizeof(mCurrentFrame));
    mCurrentFrame.beginTime = PAL_ClockGetTimeMs();
    ++mRenderedFrameCount;
}

void NBGM_MapViewProfiler::PrepareFrame()
{
    mCurrentFrame.prepareTime = PAL_ClockGetTimeMs() - mCurrentFrame.beginTime;
}

void NBGM_MapViewProfiler::UpdatePosition()
{
    mCurrentFrame.updatePositionTime = PAL_ClockGetTimeMs() - mCurrentFrame.beginTime
        - mCurrentFrame.prepareTime;
    SetUpdatePositionTime(mCurrentFrame.updatePositionTime);
}

void NBGM_MapViewProfiler::UpdateLabel()
{
    mCurrentFrame.updateLabelTime = PAL_ClockGetTimeMs() - mCurrentFrame.beginTime
        - (mCurrentFrame.prepareTime + mCurrentFrame.updatePositionTime);
    SetLayoutLabelTime(mCurrentFrame.updateLabelTime);
}

void NBGM_MapViewProfiler::Render()
{
    mCurrentFrame.renderTime = PAL_ClockGetTimeMs() - mCurrentFrame.beginTime
        - (mCurrentFrame.prepareTime + mCurrentFrame.updatePositionTime + mCurrentFrame.updateLabelTime);
}

void NBGM_MapViewProfiler::PostRender()
{
    mCurrentFrame.postRenderTime = PAL_ClockGetTimeMs() - mCurrentFrame.beginTime
        - (mCurrentFrame.prepareTime + mCurrentFrame.updatePositionTime + mCurrentFrame.updateLabelTime + mCurrentFrame.renderTime);
}

void NBGM_MapViewProfiler::EndFrame(NBRE_FileLogger& fileLoger)
{
    mCurrentFrame.endTime = PAL_ClockGetTimeMs();
    mCurrentFrame.totalTime = mCurrentFrame.endTime - mCurrentFrame.beginTime;
    SetTotalRenderTime(mCurrentFrame.totalTime);

    SetPrintFPS(FALSE);
    if (mCurrentFrame.beginTime - mLastUpdateFPS > 1000)
    {
        SetPrintFPS(TRUE);
        SetFPS(1000.0f / (mRenderedFrameAccumulatedTime / (float) mRenderedFrameCount));
        mRenderedFrameCount = 0;
        mRenderedFrameAccumulatedTime = 0;
        mLastUpdateFPS = mCurrentFrame.beginTime;
    }
    mRenderedFrameAccumulatedTime += mCurrentFrame.totalTime;

    mCurrentFrame.Dump(mProfile, fileLoger);

    Update();
}

void NBGM_MapViewProfiler::FrameProfile::Dump(const NBGM_MapViewProfile& profile, NBRE_FileLogger& fileLoger) const
{
    fileLoger.Write("%u,%u,%u,%u,%u,%u,%u\n",
        beginTime,
        totalTime,
        prepareTime,
        updateLabelTime,
        updatePositionTime,
        renderTime,
        postRenderTime);

    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM rendering time %u,%u,%u,%u,%u,%u,%u\n",
        beginTime,
        totalTime,
        prepareTime,
        updateLabelTime,
        updatePositionTime,
        renderTime,
        postRenderTime);

    if(profile.mTotalRenderTime > 35)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM Tile number of each frame = %d\n"
                                            "NBGM Render one frame time = %dms\n"
                                            "NBGM Triangle number of each frame = %d\n",
                                            profile.mLoadedTileNum,
                                            profile.mTotalRenderTime,
                                            profile.mTotalTrianglesNum);
    }

    if(profile.mPrintFPS)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM FPS = %f\n", profile.mFPS);
    }
}
