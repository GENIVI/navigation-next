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

#include "nbgmmapcamerahelperimpl.h"
#include "nbreveiwport.h"
#include "nbgmconst.h"
#include "nbreintersection.h"
#include "nbretypeconvert.h"
#include "nbretransformation.h"
#include "nbgmmapcamera.h"

static const float MIN_TILT_ANGLE = 58.f;
static const float VIEW_POINT_DISTANCE_FACTOR = 10.f;


static double GetLimitedMercatorX(double mercatorX)
{
    if(mercatorX >  NBRE_Math::Pi64 || mercatorX < -NBRE_Math::Pi64)
    {
        // 1 Avoid massive cycles.
        int count = (int)(mercatorX/ NBRE_Math::PiTwo64);
        mercatorX -= count* NBRE_Math::PiTwo64;
        // 2 Check mercatorX again due to float precision, probably need one or two more iterations.
        while(mercatorX > NBRE_Math::Pi64)
        {
            mercatorX -= NBRE_Math::PiTwo64;
        }
        while(mercatorX < -NBRE_Math::Pi64)
        {
            mercatorX += NBRE_Math::PiTwo64;
        }
    }
    return mercatorX;
}

NBGM_MapCameraHelperImpl::NBGM_MapCameraHelperImpl():
    mIsStartMoved(FALSE),
    mViewPortDistance(1000),
    mHorizonDistance(0),
    mSkyHeightOnViewport(0.0f),
    mViewport(NULL),
    mCamera(NBRE_NEW NBRE_Camera()),
    mMapCenter(NBRE_NEW NBRE_Node())
{
    mCamera->LookAt(NBRE_Vector3f(0.0f, 0.0f, 1.0f),
                    NBRE_Vector3f(0.0f, 0.0f, 0.0f),
                    NBRE_Vector3f(0.0f, 1.0f, 0.0f));
    mMapCenter->AddChild(mCamera);

    mViewport = NBRE_NEW NBRE_Viewport(0, 0, 0, 0);
    mViewport->SetCamera(mCamera);
}

NBGM_MapCameraHelperImpl::~NBGM_MapCameraHelperImpl()
{
    NBRE_DELETE mViewport;
}

void
NBGM_MapCameraHelperImpl::SyncToCamera(NBGM_MapCamera& mapCamera)
{
    NBRE_AxisAlignedBox2i box = mViewport->GetRect();
    const NBRE_PerspectiveConfig& config = mCamera->GetFrustum().GetPerspectiveConfig();
    NBRE_Vector2i boxSize = box.maxExtend - box.minExtend;
    NBRE_Vector3d viewCenter = mMapCenter->Position();
    mapCamera.SetViewSize(box.minExtend.x, box.minExtend.y, boxSize.x, boxSize.y);
    mapCamera.SetPerspective(config.mFov, config.mAspect);
    mapCamera.SetHorizonDistance(GetHorizonDistance());
    mapCamera.SetViewCenter(viewCenter.x, viewCenter.y);
    mapCamera.SetViewPointDistance(GetViewPointDistance());
    mapCamera.SetTiltAngle(GetTiltAngle());
    mapCamera.SetRotateAngle(GetRotateAngle());
}

void
NBGM_MapCameraHelperImpl::SetViewCenter( double mercatorX, double mercatorY )
{
    mMapCenter->SetPosition(NBRE_Vector3d(mercatorX, mercatorY, 0));
    RestrictViewCenter();
}

void
NBGM_MapCameraHelperImpl::GetViewCenter( double& mercatorX, double& mercatorY )
{
    mercatorX = mMapCenter->Position().x;
    mercatorY = mMapCenter->Position().y;
}

void
NBGM_MapCameraHelperImpl::SetHorizonDistance(double horizonDistance)
{
    mHorizonDistance = METER_TO_MERCATOR(horizonDistance);
    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
}

double
NBGM_MapCameraHelperImpl::GetHorizonDistance()
{
    double distance = MERCATER_TO_METER(mHorizonDistance);
    return distance;
}

void
NBGM_MapCameraHelperImpl::SetRotateAngle( float angle )
{
    mMapCenter->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), -angle));
}

float
NBGM_MapCameraHelperImpl::GetRotateAngle()
{
    NBRE_Vector3f right = mMapCenter->Right();
    float angle = NBRE_Math::DirectionToHeading(right.x, right.y);
    while(angle > 360)
    {
        angle -= 360;
    }

    while(angle < 0)
    {
        angle += 360;
    }
    return 360-angle;
}

void
NBGM_MapCameraHelperImpl::SetViewPointDistance( float distance )
{
    mViewPortDistance = distance;

    double mercaterDistance = METER_TO_MERCATOR(distance);

    NBRE_Vector3d position = mCamera->Position();
    position.Normalise();
    position *= mercaterDistance;
    mCamera->SetPosition(position);

    NBRE_PerspectiveConfig perspective = mCamera->GetFrustum().GetPerspectiveConfig();
    double nearDis = mercaterDistance/50;
    if(nearDis > position.z)
    {
        nearDis = position.z/2;
    }
    double farDis = nearDis*20000;
    mCamera->GetFrustum().SetAsPerspective(perspective, static_cast<float> (nearDis), static_cast<float> (farDis));
}

float
NBGM_MapCameraHelperImpl::GetViewPointDistance()
{
    return mViewPortDistance;
}

float
NBGM_MapCameraHelperImpl::GetCameraHeight()
{
    float res = static_cast<float>(mCamera->Position().z);
    res = MERCATER_TO_METER(res);
    return res;
}

void
NBGM_MapCameraHelperImpl::SetTiltAngle( float angle )
{
    if(angle < 0)
    {
        angle = 0;
    }

    mCamera->SetOrientation(NBRE_Orientation(mCamera->Right(), angle));

    NBRE_Matrix4x4d transMat = NBRE_TypeConvertd::Convert(NBRE_Transformationf::BuildRotateDegreesMatrix(angle, mCamera->Right()));

    NBRE_Vector3d pos = mCamera->Position();
    NBRE_Point3d newPos(0, 0, pos.Length());
    newPos = transMat * newPos;
    mCamera->SetPosition(newPos);

    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
}

float
NBGM_MapCameraHelperImpl::GetTiltAngle()
{
    NBRE_Vector3d pos = mCamera->Position();
    pos.Normalise();

    double cosValue = pos.DotProduct(NBRE_Vector3d(0, 0, 1));

    return static_cast<float>(NBRE_Math::RadToDeg(nsl_acos(cosValue)));
}

void
NBGM_MapCameraHelperImpl::SetViewSize( int32 x, int32 y, uint32 width, uint32 height )
{
    mViewport->Update(x, y, width, height);
    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
}

void
NBGM_MapCameraHelperImpl::SetPerspective(float fov, float aspect)
{
    NBRE_PerspectiveConfig perspective = {fov, aspect};
    mCamera->GetFrustum().SetAsPerspective(perspective, mCamera->GetFrustum().GetZNear(), mCamera->GetFrustum().GetZFar());
}


bool
NBGM_MapCameraHelperImpl::StartMove(float screenX, float screenY)
{
    if(IsValidScreenPosition(screenX, screenY))
    {
        NBRE_Ray3d ray = mViewport->GetRay(screenX, screenY);

        NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);

        mIsStartMoved = NBRE_Intersectiond::HitTest(plane, ray, &mStartMovedPosition);

        if(mIsStartMoved)
        {
            return true;
        }
    }
    else
    {
        mIsStartMoved = FALSE;
    }

    return false;
}

bool
NBGM_MapCameraHelperImpl::MoveTo(float screenX, float screenY)
{
    if(IsValidScreenPosition(screenX, screenY) && mIsStartMoved)
    {
        NBRE_Ray3d ray = mViewport->GetRay(screenX, screenY);
        NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);
        NBRE_Vector3d position;
        nb_boolean result = NBRE_Intersectiond::HitTest(plane, ray, &position);
        if(result)
        {
            mMapCenter->Translate(mStartMovedPosition - position);
            RestrictViewCenter();
            return true;
        }
    }

    return false;
}

void
NBGM_MapCameraHelperImpl::SetRotateCenter(float screenX, float screenY)
{
    NBRE_Ray3d ray = mViewport->GetRay(screenX, screenY);

    NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);

    NBRE_Intersectiond::HitTest(plane, ray, &mRotatePosition);
}

void
NBGM_MapCameraHelperImpl::Rotate(float angle)
{
    mMapCenter->Rotate(NBRE_Vector3f(0, 0, 1), mRotatePosition, -angle);
}

void
NBGM_MapCameraHelperImpl::Tilt(float angle)
{
    NBRE_Vector3f pos = NBRE_TypeConvertf::Convert(mCamera->Position());
    pos.Normalise();
    float cosValue = pos.DotProduct(NBRE_Vector3f(0, 0, 1));
    float theta = static_cast<float>(NBRE_Math::RadToDeg(nsl_acos(cosValue)));

    if(angle < 0)
    {
        if(theta + angle < 0 )
        {
            angle = -theta;
        }
    }

    mCamera->Rotate(mCamera->Right(), NBRE_Vector3d(0, 0, 0), angle);
    mSkyHeightOnViewport = CalculateViewPointSkyHeight();
}


void
NBGM_MapCameraHelperImpl::Zoom(float deltaH)
{
    float distance = mViewPortDistance + MERCATER_TO_METER(deltaH);
    SetViewPointDistance(distance);
}

bool
NBGM_MapCameraHelperImpl::ScreenToMapPosition(float screenX, float screenY, double& mercatorX, double& mercatorY, bool useLimitation )
{
    NBRE_Ray3d centerRay = mViewport->GetRay(mViewport->GetRect().GetSize().x / 2.0f, mViewport->GetRect().GetSize().y / 2.0f);
    NBRE_Ray3d ray = mViewport->GetRay(screenX, screenY);
    NBRE_Vector3d dir = centerRay.direction * mCamera->GetFrustum().GetZFar() / ray.direction.DotProduct(centerRay.direction);
    if((ray.origin + dir).z < 0)
    {
        NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);

        NBRE_Vector3d position;
        if(NBRE_Intersectiond::HitTest(plane, ray, &position))
        {
            mercatorX = position.x;
            mercatorY = position.y;
            if(useLimitation)
            {
                 mercatorX = GetLimitedMercatorX(position.x);
            }
            return true;
        }
    }
    return false;
}

static nb_boolean IsInNearPlaneFrontSide(const NBRE_Frustum3d& frustum, const NBRE_Vector3d& position)
{
    return frustum.GetNear().GetSide(position) != NBRE_PS_BACK_SIDE ? TRUE : FALSE;
}

static nb_boolean WorldToScreen(NBRE_Viewport* viewport, const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition)
{
    viewport->Camera()->UpdatePosition();
    NBRE_Matrix4x4d viewMatrix = viewport->Camera()->ExtractMatrix();
    NBRE_Matrix4x4d projectMatrix = NBRE_TypeConvertd::Convert(viewport->Camera()->GetFrustum().ExtractMatrix());
    NBRE_Matrix4x4d viewportMatrix = NBRE_TypeConvertd::Convert(viewport->ExtractMatrix());

    NBRE_Frustum3d frustum = NBRE_Frustum3d(projectMatrix * viewMatrix);
    if(!IsInNearPlaneFrontSide(frustum, position))
    {
        return FALSE;
    }
    NBRE_Matrix4x4d viewProjectViewportMatrix = viewportMatrix * projectMatrix * viewMatrix;

    screenPosition = NBRE_Transformationd::WorldToScreen(viewProjectViewportMatrix, position);
    return TRUE;
}

bool
NBGM_MapCameraHelperImpl::MapPositionToScreen(float& screenX, float& screenY, double mercatorX, double mercatorY)
{
    NBRE_Vector3d position(mercatorX, mercatorY, 0);
    NBRE_Vector2d screenPosition;
    NBRE_AxisAlignedBox2d viewportRect = NBRE_TypeConvertd::Convert(mViewport->GetRect());
    double posDist = mMapCenter->Position().SquaredDistanceTo(position);
    NBRE_Vector3d leftExtendPosition = position + NBRE_Vector3d(-NBRE_Math::PiTwo64, 0, 0);
    double leftPosDist = mMapCenter->Position().SquaredDistanceTo(leftExtendPosition);
    NBRE_Vector3d rightExtendPosition = position + NBRE_Vector3d(NBRE_Math::PiTwo64, 0, 0);
    double rightPosDist = mMapCenter->Position().SquaredDistanceTo(rightExtendPosition);
    if(posDist > leftPosDist)
    {
        position = leftExtendPosition;
        posDist = leftPosDist;
    }
    if(posDist > rightPosDist)
    {
        position = rightExtendPosition;
    }

    nb_boolean result = WorldToScreen(mViewport, position, screenPosition);

    if(result)
    {
        screenX = static_cast<float> (screenPosition.x);
        screenY = mViewport->GetRect().GetSize().y - static_cast<float> (screenPosition.y);
    }
    return result == TRUE;
}

bool
NBGM_MapCameraHelperImpl::GetFrustumPositionInWorld(double frustumHeight, std::vector<NBGM_Point2d64>& positions)
{
    positions.resize(4);
    NBRE_Camera& camera  = *mViewport->Camera().get();
    camera.UpdatePosition();
    NBRE_Frustum &frustum = camera.GetFrustum();
    NBRE_Frustum3d frustum3D(NBRE_TypeConvertd::Convert(frustum.ExtractMatrix()) * camera.ExtractMatrix());
    NBRE_Vector3d corners[8];
    frustum3D.GetCorners(corners);
    NBRE_Planed plane(NBRE_Vector3d(0, 0, 1), 0);
    NBRE_Vector3d points[4];
    double epsilon = VECTOR_EPSILON;
    // left top
    nb_boolean ret = NBRE_Intersectiond::HitTest(plane, corners[3], corners[7], points, epsilon);
    if(!ret)
    {
        ret = NBRE_Intersectiond::HitTest(plane, corners[4], corners[7], points, epsilon);
    }

    if(!ret)
        return FALSE;

    // right top
    ret = NBRE_Intersectiond::HitTest(plane, corners[2], corners[6], points+1, epsilon);
    if(!ret)
    {
        ret = NBRE_Intersectiond::HitTest(plane, corners[5], corners[6], points+1, epsilon);
    }
    // right bottom
    ret = ret?NBRE_Intersectiond::HitTest(plane, corners[1], corners[5], points+2, epsilon):FALSE;
    // left bottom
    ret = ret?NBRE_Intersectiond::HitTest(plane, corners[0], corners[4], points+3, epsilon):FALSE;

    if(ret)
    {
        NBRE_Ray3d ray(points[0], points[1]-points[0]);
        double origin = ray.DistanceTo(points[2]);
        double refDistance = frustumHeight;
        double horizonDistance = mHorizonDistance;
        float halfFov = frustum.GetPerspectiveConfig().mFov/2.f;
        float maxAngle = GetTiltAngle() + halfFov;
        float minAngle = GetTiltAngle() - halfFov;
        double minDistance = origin;
        if(frustum.GetProjectionType() == NBRE_PT_PERSPECTIVE && maxAngle < 90 && minAngle > 0)
        {
            double realDistance = camera.Position().z*(nsl_tan(NBRE_Math::DegToRad(maxAngle)) - nsl_tan(NBRE_Math::DegToRad(minAngle)));
            minDistance = NBRE_Math::Min<double>(minDistance, realDistance);
        }
        if(refDistance > 0)
        {
            minDistance = NBRE_Math::Min<double>(minDistance, refDistance);
        }
        if(horizonDistance > 0)
        {
            minDistance = NBRE_Math::Min<double>(minDistance, horizonDistance);
        }

        if( GetTiltAngle() >= MIN_TILT_ANGLE)
        {
            float viewPointDistance = METER_TO_MERCATOR(GetViewPointDistance());
            float distancInEyeSpace = VIEW_POINT_DISTANCE_FACTOR*viewPointDistance;
            float baseDistance = (float)(camera.Position().z*(nsl_tan(NBRE_Math::DegToRad(GetTiltAngle())) - nsl_tan(NBRE_Math::DegToRad(minAngle))));
            float fogDistance = baseDistance + (distancInEyeSpace - viewPointDistance)/sin(NBRE_Math::DegToRad(GetTiltAngle()));
            minDistance = NBRE_Math::Min<double>(minDistance, fogDistance);
        }

        nbre_assert(origin != 0);
        double t = minDistance/origin;
        points[0] = NBRE_LinearInterpolated::Lerp(points[3], points[0], t);
        points[1] = NBRE_LinearInterpolated::Lerp(points[2], points[1], t);

        for(uint32 i = 0; i < 4; i++)
        {
            positions[i].x = points[i].x;
            positions[i].y = points[i].y;
        }
    }

    return ret == TRUE;
}

void
NBGM_MapCameraHelperImpl::RestrictViewCenter()
{
    NBRE_Vector3d position = mMapCenter->Position();
    position.x = GetLimitedMercatorX(position.x);
    mMapCenter->SetPosition(position);
}

float
NBGM_MapCameraHelperImpl::CalculateViewPointSkyHeight()
{
    float skyHeight = 0.f;
    float viewPointHight = static_cast<float> (mViewport->GetRect().GetSize().y);
    if(mHorizonDistance == 0)
    {
        return skyHeight;
    }
    NBRE_Frustum& frustum = mCamera->GetFrustum();
    if(frustum.GetProjectionType() == NBRE_PT_PERSPECTIVE)
    {
        NBRE_Vector3f dir = mCamera->Direction();
        dir.Normalise();
        float tiltAngle = NBRE_Math::RadToDeg(nsl_acos(dir.DotProduct(NBRE_Vector3f(0, 0, 1))));
        float bottomAngle = tiltAngle - frustum.GetPerspectiveConfig().mFov/2.f;
        float topAngle = tiltAngle + frustum.GetPerspectiveConfig().mFov/2.f;
        float bottomDistance = static_cast<float>(nsl_tan(NBRE_Math::DegToRad(bottomAngle))*mCamera->WorldPosition().z);
        float topDistance = static_cast<float>(nsl_tan(NBRE_Math::DegToRad(topAngle))*mCamera->WorldPosition().z);
        if(bottomAngle >= 90.f)
        {
            return skyHeight;
        }
        if( (topAngle >= 90.f || mHorizonDistance < topDistance) && mHorizonDistance > bottomDistance)
        {
            float horizonAngle = NBRE_Math::RadToDeg(static_cast<float>(nsl_atan(mHorizonDistance / mCamera->WorldPosition().z)));
            float deltaAngle = horizonAngle - tiltAngle;
            float deltaLength = nsl_tan(NBRE_Math::DegToRad(deltaAngle))*(viewPointHight/2.f)/nsl_tan(NBRE_Math::DegToRad(frustum.GetPerspectiveConfig().mFov/2.f));
            skyHeight = viewPointHight/2.f - deltaLength;
            return skyHeight;
        }
    }
    return skyHeight;
}

bool
NBGM_MapCameraHelperImpl::ScreenToMapDirection(float screen1X, float screen1Y, float screen2X, float screen2Y, float& direction)
{
    NBRE_Vector3d point1 = mViewport->GetWorldPosition(screen1X, screen1Y);
    NBRE_Vector3d point2 = mViewport->GetWorldPosition(screen2X, screen2Y);
    NBRE_Planed plane(mCamera->WorldPosition(), point1, point2);
    NBRE_Planed ground(NBRE_Vector3d(0, 0, 1), 0);
    NBRE_Ray3d ray;
    if(NBRE_Intersectiond::HitTest(ground, plane, &ray))
    {
        float angle = NBRE_Math::DirectionToHeading((float) ray.direction.x, (float) ray.direction.y);
        angle = 90 - angle;
        while(angle > 360)
        {
            angle -= 360;
        }

        while(angle < 0)
        {
            angle += 360;
        }
        direction = angle;
        return true;
    }
    return false;
}

nb_boolean
NBGM_MapCameraHelperImpl::IsValidScreenPosition(float screenX, float screenY)
{
    NBRE_Ray3d ray = mViewport->GetRay(screenX, screenY);
    NBRE_Vector3d right = NBRE_TypeConvertd::Convert(mMapCenter->Right());
    NBRE_Vector3d skyNormal(NBRE_Vector3d(-right.y, right.x, 0));
    skyNormal.Normalise();
    skyNormal *= mHorizonDistance;
    NBRE_Vector3d skyPosition = NBRE_Vector3d(mCamera->WorldPosition().x, mCamera->WorldPosition().y, 0) + skyNormal;
    skyNormal.Normalise();
    double d = skyPosition.DotProduct(skyNormal);
    NBRE_Planed sky(skyNormal, d);
    NBRE_Planed ground(NBRE_Vector3d(0, 0, 1), 0);

    double projectionOnSkyNormal = sky.normal.DotProduct(ray.direction);
    double projectionOnGroundNormal = ground.normal.DotProduct(ray.direction);

    if(projectionOnGroundNormal > projectionOnSkyNormal)
    {
        NBRE_Vector3d position;
        if(NBRE_Intersectiond::HitTest(ground, ray, &position))
        {
            NBRE_Vector3d direction = mCamera->WorldPosition() - position;
            double distance = sky.normal.DotProduct(direction);
            if(distance < mHorizonDistance)
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    else
    {
        NBRE_Vector3d position;
        if(NBRE_Intersectiond::HitTest(sky, ray, &position))
        {
            if(position.z > 0)
            {
                return FALSE;
            }
        }
        return TRUE;
    }
}