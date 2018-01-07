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

    @file nbgmanimation.h
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
#ifndef _NBGM_ANIMATION_H_
#define _NBGM_ANIMATION_H_
#include "nbretypes.h"
#include "nbrecommon.h"
#include "nbrescenemanager.h"
#include "nbgmdefaultoverlaymanager.h"


struct NBGM_Context;
class NBRE_SurfaceSubView;
class NBRE_Overlay;
class NBRE_Entity;
class NBRE_RenderSurface;
class NBGM_MapViewProfiler;
class NBRE_RenderEngine;

/*! \addtogroup NBRE_Service
*  @{
*/

/** This class is used to show transparent 3d building
*/

class NBGM_Animation: public NBRE_IVisitable, public NBRE_IRenderOperationProvider
{

public:
    NBGM_Animation(NBRE_RenderEngine* renderEngine);
    ~NBGM_Animation();

public:
    void Initialize();
    void OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h);

	void ShootPicture1(NBRE_RenderSurface* surface);
	void ShootPicture2(NBRE_RenderSurface* surface);

	void Begin();
	void End(NBRE_RenderSurface* surface);

	void Move(float xPercent);

    // NBRE_IVisitable members
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_IRenderQueueVisitable
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;    
    virtual void UpdateSurfaceList();
    virtual uint32 GetPriority() const { return 0; }

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Animation);

private:
    void CreateRectVertex();

private:
    NBRE_RenderEngine* mRenderEngine;

    NBRE_RenderSurface* mSurface; 
    NBRE_SurfaceSubView* mSubView;
    NBRE_SceneManager* mSceneManager;

	NBRE_RenderSurface* mMapSurface; 

    NBRE_VertexData* mShardVertex;
    NBRE_IndexData* mIndexData;

    NBRE_Pass* mPass1;
    NBRE_Pass* mPass2;

    NBRE_Matrix4x4d mVertexTransform1;
    NBRE_Matrix4x4d mVertexTransform2;
    NBRE_DefaultOverlayManager<int32>* mOverlayManager;
    int32 mMaxOverlayId;
};

/*! @} */

#endif
