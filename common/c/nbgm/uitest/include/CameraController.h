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

#pragma once

#include "stdafx.h"

class NBRE_Camera;

/** camera controller
   @remark
   If you want to control one camera for transforming, you must bind it to CCameraController
   by invoke BindCamera.If you want to change camera to control, just invoke BindCamera again.

   If you want to use camera controller, your test view must can handle OnKeyDown message.

   Operating instruction:
   A,D -- Roll
   S,W -- Yaw
   Q,E -- Pitch
   F,N -- Zoom

   LEFT,RIGHT -- Translate the camera around it's local x axis
   UP,DOWN -- Translate the camera around horizontal plane

   You can get more details in the example named TestCameraControllerView.
*/

class CCameraController
{
public:
    CCameraController(){
        controllerPara.mRotateAnglePerAction = 1.0;
        controllerPara.mSlideDistancePerAction = 1.0;
        m_Camera = NULL;
    }
    ~CCameraController(){}

public:
    void BindCamera(NBRE_Camera* camera){m_Camera = camera;}
    void OnKeyEvent(UINT nChar);
    void SetSlideDistancePerAction(float distance){controllerPara.mSlideDistancePerAction = distance;}
    void SetRotateAnglePerAction(float angle){controllerPara.mRotateAnglePerAction = angle;}

private:
    struct ControlerParameter
    {
        float mSlideDistancePerAction;
        float mRotateAnglePerAction;;
    };

    ControlerParameter controllerPara;
    NBRE_Camera* m_Camera;
};
