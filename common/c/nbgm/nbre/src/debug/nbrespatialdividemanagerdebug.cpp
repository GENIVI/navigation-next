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
#include "nbrespatialdividemanagerdebug.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbredefaultspatialdividemanager.h"

NBRE_Matrix4x4f NBRE_SpatialDivideManagerDebug::mIdentity;

NBRE_SpatialDivideManagerDebug::NBRE_SpatialDivideManagerDebug(NBRE_Context& context, NBRE_DefaultSpatialDivideManager& spatialMgr, int32 overlayId):
        mContext(context),
        mSpatialDivideManager(spatialMgr),
        mOverlayId(overlayId),
        mVisible(TRUE),
        mOnlyShowHasObj(FALSE),
        mCurrentBoundingBox(0)
{
}

NBRE_SpatialDivideManagerDebug::~NBRE_SpatialDivideManagerDebug()
{
    for (DebugBoundingBoxArray::iterator i = mDebugBoundingBoxs.begin(); i != mDebugBoundingBoxs.end(); ++i)
    {
        NBRE_DELETE *i;
    }

}

void NBRE_SpatialDivideManagerDebug::UpdateRenderQueue(NBRE_BinaryZone* zone, NBRE_Overlay& overlay)
{
    if(zone)
    {
        if(zone->SpatialObjects().size() || (!mOnlyShowHasObj))
        {
            NBRE_DebugBoundingBox* currentBoundingBox;
            if(mCurrentBoundingBox < static_cast<int32>(mDebugBoundingBoxs.size()))
            {
                currentBoundingBox = mDebugBoundingBoxs[mCurrentBoundingBox];
            }
            else
            {
                currentBoundingBox = NBRE_NEW NBRE_DebugBoundingBox();
                mDebugBoundingBoxs.push_back(currentBoundingBox);
            }
            currentBoundingBox->SetBoundingBox(zone->AxisAlignedBox());
            currentBoundingBox->UpdateRenderQueue(mContext, overlay);
            ++mCurrentBoundingBox;
        }

        UpdateRenderQueue(zone->LeftChild(), overlay);
        UpdateRenderQueue(zone->RightChild(), overlay);
    }
}

void NBRE_SpatialDivideManagerDebug::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    mCurrentBoundingBox = 0;
    UpdateRenderQueue(mSpatialDivideManager.RootZone(), overlay);
}

int32 NBRE_SpatialDivideManagerDebug::OverlayId() const
{
    return mOverlayId;
}


void NBRE_SpatialDivideManagerDebug::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

void NBRE_SpatialDivideManagerDebug::SetVisible(nb_boolean visible)
{
    mVisible = visible;
}

nb_boolean NBRE_SpatialDivideManagerDebug::Visible() const
{
    return mVisible;
}

void NBRE_SpatialDivideManagerDebug::SetOnlyShowHasObj(nb_boolean show)
{
    mOnlyShowHasObj = show;
}

void NBRE_SpatialDivideManagerDebug::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mContext.mSurfaceManager);
}
