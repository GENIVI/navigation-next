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

    @file nbgmmapcameraimpl.h
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

#ifndef _NBGM_MAP_CAMERA_IMPL_H
#define _NBGM_MAP_CAMERA_IMPL_H

#include "nbgmmapcamera.h"
#include "nbreaxisalignedbox2.h"
#include "nbrefrustum.h"

class NBGM_MapCameraImpl : public NBGM_MapCamera
{
public:
    NBGM_MapCameraImpl();
    ~NBGM_MapCameraImpl();

public:
    virtual void SetViewCenter(double mercatorX, double mercatorY);
    virtual void SetHorizonDistance(double horizonDistance);
    virtual void SetViewPointDistance(float distance);
    virtual void SetRotateAngle(float angle);
    virtual void SetTiltAngle(float angle);
    virtual void SetViewSize(int32 x, int32 y, uint32 width, uint32 height);
    virtual void SetPerspective(float fov, float aspect);

public:
    NBRE_AxisAlignedBox2i GetViewSize();
    NBRE_Vector2d GetViewCenter();
    NBRE_PerspectiveConfig GetPerspectiveConfig();
    float GetRotateAngle();
    float GetTiltAngle();
    float GetViewPortDisantce();
    double GetHorizonDistance();

    nb_boolean GetCameraChanged() { return mCameraChanged; }
    nb_boolean GetViewPortChanged() { return mViewPortChanged; }
    void ResetCameraChangedFlag() { mCameraChanged = FALSE; }
    void ResetViewPortChangedFlag() { mViewPortChanged = FALSE; }

private:
    NBRE_AxisAlignedBox2i mViewSize;
    NBRE_Vector2d mViewCenter;
    NBRE_PerspectiveConfig mPerspectiveConfig;
    float  mRotateAngle;
    float  mTiltAngle;
    float  mViewPortDistance;
    double mHorizonDistance;

    nb_boolean mViewPortChanged;
    nb_boolean mCameraChanged;
};

#endif