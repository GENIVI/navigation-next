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

    @file nbrecamera.h
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
#ifndef _NBRE_CAMERA_H_
#define _NBRE_CAMERA_H_
#include "nbretypes.h"
#include "nbrevector3.h"
#include "nbrematrix4x4.h"
#include "nbrefrustum.h"
#include "nbresharedptr.h"
#include "nbrenode.h"

/** \addtogroup NBRE_RenderEngine
*  @{
*/

/** The whole scene will be rendered from the cameras point of view.
*/
class NBRE_Camera: public NBRE_Node
{

public:
    NBRE_Camera();
    ~NBRE_Camera();

public:

    void CopyPosition(const NBRE_Camera& cam);
    ///Derive From NBRE_Node
    virtual void UpdatePosition();

    NBRE_Camera* CreateWorldCamera();

    void LookAt(const NBRE_Vector3f& eye, const NBRE_Vector3f& at, const NBRE_Vector3f& up);

    void Slide(const NBRE_Vector3f& deltaPos);

    /** Rolls the camera anticlockwise, around its local z axis.
    */
    void Roll(float angle)
    {
        Rotate(Direction(), Position(), angle);
    }
    /** Rotates the camera anticlockwise around it's local y axis.
    */
    void Yaw(float angle)
    {
        Rotate(Up(), Position(), angle);
    }

    /** Pitches the camera up/down anticlockwise around it's local x axis.
    */
    void Pitch(float angle)
    {
        Rotate(Right(), Position(), angle);
    }

    const NBRE_Matrix4x4d& ExtractMatrix()const {return mModelViewTransform;}

    const NBRE_Frustum& GetFrustum() const { return mFrustum; }
    NBRE_Frustum& GetFrustum() { return mFrustum; }

    void Profile() const;
private:
    DISABLE_COPY_AND_ASSIGN(NBRE_Camera);

private:

    NBRE_Matrix4x4d mModelViewTransform;
    NBRE_Frustum mFrustum;
};

typedef shared_ptr<NBRE_Camera> NBRE_CameraPtr;
typedef shared_ptr<const NBRE_Camera> NBRE_ConstCameraPtr;

/** @} */
#endif
