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
#include "nbrefrustum.h"
#include "nbremath.h"
#include "nbretransformation.h"

const float NBRE_Frustum::INFINITE_FAR_PLANE_ADJUST = 0.00000001f;

NBRE_Frustum::NBRE_Frustum():mProjType(NBRE_PT_PERSPECTIVE),
    mNearDist(1.0f),
    mFarDist(1.0f),
    mConfigChanged(TRUE)
{
    mPerspectiveConfig.mAspect = 1.3f;
    mPerspectiveConfig.mFov = 45.0f;
}

NBRE_Frustum::~NBRE_Frustum()
{

}

void
NBRE_Frustum::SetAsPerspective( const NBRE_PerspectiveConfig& config, float zNear, float zFar )
{
    mProjType = NBRE_PT_PERSPECTIVE;
    mPerspectiveConfig = config;
    mFarDist = zFar;
    mNearDist = zNear;
    mConfigChanged = TRUE;
}

void
NBRE_Frustum::SetAsOrtho( const NBRE_OrthoConfig& config, float zNear, float zFar )
{
    mProjType = NBRE_PT_ORTHO_GRAPHIC;
    mOrthoConfg = config;
    mFarDist = zFar;
    mNearDist = zNear;
    mConfigChanged = TRUE;
}

NBRE_ProjectionType
NBRE_Frustum::GetProjectionType() const
{
    return mProjType;
}

const NBRE_OrthoConfig&
NBRE_Frustum::GetOrthoConfig() const
{
    return mOrthoConfg;
}

const NBRE_PerspectiveConfig&
NBRE_Frustum::GetPerspectiveConfig() const
{
    return mPerspectiveConfig;
}

float
NBRE_Frustum::GetZFar() const
{
    return mFarDist;
}

float
NBRE_Frustum::GetZNear() const
{
    return mNearDist;
}

const NBRE_Matrix4x4f&
NBRE_Frustum::ExtractMatrix() const
{
    if(mConfigChanged)
    {
        UpdateFrustum();
        mConfigChanged = FALSE;
    }
    return mPreCalcedMatrix;
}

void NBRE_Frustum::UpdateFrustum() const
{

    if (mProjType == NBRE_PT_PERSPECTIVE)
    {
        mPreCalcedMatrix = NBRE_Transformationf::BuildPerspectiveMatrix(mPerspectiveConfig.mFov, mPerspectiveConfig.mAspect, mNearDist, mFarDist);
    }
    else
    {
        mPreCalcedMatrix = NBRE_Transformationf::BuildOrthoMatrix(mOrthoConfg.mLeft, mOrthoConfg.mRight, mOrthoConfg.mBottom, mOrthoConfg.mTop, mNearDist, mFarDist);
    }

}