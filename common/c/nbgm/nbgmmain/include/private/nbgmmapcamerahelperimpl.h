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

    @file nbgmmapcamerahelperimpl.h
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

#ifndef _NBGM_MAP_CAMERA_HELPER_IMPL_H
#define _NBGM_MAP_CAMERA_HELPER_IMPL_H

#include "nbgmmapcamerahelper.h"
#include "nbrevector2.h"
#include "nbrevector3.h"
#include "nbrecamera.h"

class NBRE_Viewport;

class NBGM_MapCameraHelperImpl : public NBGM_MapCameraHelper
{
public:
    NBGM_MapCameraHelperImpl();
    ~NBGM_MapCameraHelperImpl();

public:
    virtual void SyncToCamera(NBGM_MapCamera& mapCamera);
    virtual void SetViewCenter(double mercatorX, double mercatorY);
    virtual void GetViewCenter(double& mercatorX, double& mercatorY);
    virtual void SetHorizonDistance(double horizonDistance);
    virtual double GetHorizonDistance();
    virtual void SetRotateAngle(float angle);
    virtual float GetRotateAngle();
    virtual void SetViewPointDistance(float distance);
    virtual float GetViewPointDistance();
    virtual float GetCameraHeight();
    virtual void SetTiltAngle(float angle);
    virtual float GetTiltAngle();
    virtual void SetViewSize(int32 x, int32 y, uint32 width, uint32 height);
    virtual void SetPerspective(float fov, float aspect);
    virtual bool StartMove(float screenX, float screenY);
    virtual bool MoveTo(float screenX, float screenY);
    virtual void SetRotateCenter(float screenX, float screenY);
    virtual void Rotate(float angle);
    virtual void Tilt(float angle);
    virtual void Zoom(float deltaH);
    virtual bool ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY, bool useLimitation = true);
    virtual bool MapPositionToScreen(float& screenX, float& screenY, double mercatorX, double mercatorY);
    virtual bool GetFrustumPositionInWorld(double frustumHeight, std::vector<NBGM_Point2d64>& positions);
    virtual bool ScreenToMapDirection(float screen1X, float screen1Y, float screen2X, float screen2Y, float& direction);

private:
    void RestrictViewCenter();
    float CalculateViewPointSkyHeight();
    nb_boolean IsValidScreenPosition(float screenX, float screenY);

private:
    nb_boolean mIsStartMoved;
    NBRE_Vector3d mStartMovedPosition;
    NBRE_Vector3d mRotatePosition;
    float  mViewPortDistance;
    double mHorizonDistance;
    float mSkyHeightOnViewport;

    NBRE_Viewport* mViewport;
    NBRE_CameraPtr mCamera;
    NBRE_NodePtr mMapCenter;

};

#endif