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
#include "nbrecamera.h"
#include "nbrevector4.h"
#include "nbremath.h"
#include "nbretransformation.h"
#include "nbretypeconvert.h"
#include "nbrelog.h"

NBRE_Camera::NBRE_Camera()
{
}

NBRE_Camera::~NBRE_Camera()
{

}

void NBRE_Camera::CopyPosition(const NBRE_Camera& cam)
{
    NBRE_Node::CopyPosition(cam);

    mFrustum = cam.mFrustum;
}

NBRE_Camera* 
NBRE_Camera::CreateWorldCamera()
{
    NBRE_Camera* camera = NBRE_NEW NBRE_Camera();
    camera->mPosition = NBRE_Transformationd::CoordinatesGetTranslate(mWorldTransform);
    camera->mRight = NBRE_TypeConvertf::Convert(NBRE_Transformationd::CoordinatesGetXAxis(mWorldTransform));
    camera->mUp = NBRE_TypeConvertf::Convert(NBRE_Transformationd::CoordinatesGetYAxis(mWorldTransform));
    camera->mDirection = NBRE_TypeConvertf::Convert(NBRE_Transformationd::CoordinatesGetZAxis(mWorldTransform));
    camera->mWorldTransform = mWorldTransform;
    camera->mFrustum = mFrustum;
    return camera;
}

void
NBRE_Camera::LookAt( const NBRE_Vector3f& eye, const NBRE_Vector3f& at, const NBRE_Vector3f& up )
{
    mDirection = eye - at;
    mDirection.Normalise();
    mRight = up.CrossProduct(mDirection);
    mRight.Normalise();
    mUp = mDirection.CrossProduct(mRight);
    mPosition = NBRE_TypeConvertd::Convert(eye);

    RequestPositionUpdate();
}


void
NBRE_Camera::Slide(const NBRE_Vector3f& deltaPos)
{
    NBRE_Vector4f dp(deltaPos, 0.f);

    NBRE_Vector4f fdp = NBRE_Matrix4x4f(
        mRight.x, mUp.x, mDirection.x, 0,
        mRight.y, mUp.y, mDirection.y, 0,
        mRight.z, mUp.z, mDirection.z, 0,
        0.f,        0.f,          0.f, 1.f) * dp;

    mPosition.x += fdp.x;
    mPosition.y += fdp.y;
    mPosition.z += fdp.z;

    RequestPositionUpdate();
}

void
NBRE_Camera::UpdatePosition()
{
    nb_boolean update = mNeedUpdatePos;

    NBRE_Node::UpdatePosition();

    if(update)
    {
        mModelViewTransform = mWorldTransform.Inverse();
    }
}

void
NBRE_Camera::Profile() const
{
    NBRE_DebugLog(PAL_LogSeverityInfo, "================NBRE_Camera::Profile================\r"
                  "NBRE_Camera::Position(%f, %f, %f)\r"
                  "NBRE_Camera::Right(%f, %f, %f)\r"
                  "NBRE_Camera::Up(%f, %f, %f)\r"
                  "NBRE_Camera::Direction(%f, %f, %f)\r"
                  "NBRE_Camera::Frustum(type=%d, near=%f, far=%f)",
                  mPosition.x, mPosition.y, mPosition.z,
                  mRight.x, mRight.y, mRight.z,
                  mUp.x, mUp.y, mUp.z,
                  mDirection.x, mDirection.y, mDirection.z,
                  GetFrustum().GetProjectionType(),
                  GetFrustum().GetZNear(),
                  GetFrustum().GetZFar());
}
