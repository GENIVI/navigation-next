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
#include "nbrecameradebug.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbretypeconvert.h"


NBRE_CameraDebug::NBRE_CameraDebug(NBRE_Context& context, const NBRE_Camera& camera, int32 overlayId):
        mNBREContext(context),
        mCamera(camera),
        mOverlayId(overlayId),
        mVisible(TRUE),
        mDebugFrustum(context, overlayId)
{
}

NBRE_CameraDebug::~NBRE_CameraDebug()
{

}

void NBRE_CameraDebug::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    NBRE_Frustum3f frustum(mCamera.GetFrustum().ExtractMatrix()*NBRE_TypeConvertf::Convert(mCamera.ExtractMatrix()));
    mDebugFrustum.SetFrustum(frustum);
    mDebugFrustum.UpdateRenderQueue(overlay);
}

int32 NBRE_CameraDebug::OverlayId() const
{
    return mOverlayId;
}


void NBRE_CameraDebug::NotifyCamera(const NBRE_Camera& camera)
{
}

nb_boolean NBRE_CameraDebug::Visible() const
{
    return mVisible;
}


void NBRE_CameraDebug::SetVisible(nb_boolean visible)
{
    mVisible = visible;
}

void NBRE_CameraDebug::SetOverlayId(int32 overlayId)
{
    mOverlayId = overlayId;
}

void NBRE_CameraDebug::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mNBREContext.mSurfaceManager);
}
