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

#include "nbgmmapcameraimpl.h"

NBGM_MapCameraImpl::NBGM_MapCameraImpl()
    :mRotateAngle(0.0f)
    ,mTiltAngle(0.0f)
    ,mViewPortDistance(0.0f)
    ,mHorizonDistance(0)
    ,mViewPortChanged(FALSE)
    ,mCameraChanged(FALSE)
{
}

NBGM_MapCameraImpl::~NBGM_MapCameraImpl()
{
}

void
NBGM_MapCameraImpl::SetViewCenter(double mercatorX, double mercatorY)
{
    if(mViewCenter.x != mercatorX || mViewCenter.y != mercatorY)
    {
        mViewCenter.x = mercatorX;
        mViewCenter.y = mercatorY;
        mCameraChanged = TRUE;
    }
}

void
NBGM_MapCameraImpl::SetHorizonDistance(double horizonDistance)
{
    if(mHorizonDistance != horizonDistance)
    {
        mHorizonDistance = horizonDistance;
        mCameraChanged = TRUE;
    }
}

void
NBGM_MapCameraImpl::SetViewPointDistance(float distance)
{
    if(mViewPortDistance != distance)
    {
        mViewPortDistance = distance;
        mCameraChanged = TRUE;
    }
}

void
NBGM_MapCameraImpl::SetRotateAngle(float angle)
{
    if(mRotateAngle != angle)
    {
        mRotateAngle = angle;
        mCameraChanged = TRUE;
    }
}

void
NBGM_MapCameraImpl::SetTiltAngle(float angle)
{
    if(angle < 0 || mTiltAngle != angle)
    {
        mTiltAngle = angle;
        mCameraChanged = TRUE;
    }
}

void
NBGM_MapCameraImpl::SetViewSize(int32 x, int32 y, uint32 width, uint32 height)
{
    int32 maxExtendX = mViewSize.minExtend.x + width;
    int32 maxExtendY = mViewSize.minExtend.y + height;
    if(mViewSize.minExtend.x != x || mViewSize.minExtend.y != y || mViewSize.maxExtend.x != maxExtendX || mViewSize.maxExtend.y != maxExtendY)
    {
        mViewSize.minExtend.x = x;
        mViewSize.minExtend.y = y;
        mViewSize.maxExtend.x = x + width;
        mViewSize.maxExtend.y = y + height;
        mViewPortChanged = TRUE;
    }
}

void
NBGM_MapCameraImpl::SetPerspective(float fov, float aspect)
{
    if(mPerspectiveConfig.mFov != fov || mPerspectiveConfig.mAspect != aspect)
    {
        mPerspectiveConfig.mFov = fov;
        mPerspectiveConfig.mAspect = aspect;
        mViewPortChanged = TRUE;
    }
}

NBRE_AxisAlignedBox2i
NBGM_MapCameraImpl::GetViewSize()
{
    return mViewSize;
}

NBRE_Vector2d
NBGM_MapCameraImpl::GetViewCenter()
{
    return mViewCenter;
}

NBRE_PerspectiveConfig
NBGM_MapCameraImpl::GetPerspectiveConfig()
{
    return mPerspectiveConfig;
}

float
NBGM_MapCameraImpl::GetRotateAngle()
{
    return mRotateAngle;
}

float
NBGM_MapCameraImpl::GetTiltAngle()
{
    return mTiltAngle;
}

float
NBGM_MapCameraImpl::GetViewPortDisantce()
{
    return mViewPortDistance;
}

double
NBGM_MapCameraImpl::GetHorizonDistance()
{
    return mHorizonDistance;
}