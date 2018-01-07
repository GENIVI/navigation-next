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

    @file nbgmsky.h
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
#ifndef _NBGM_SKY_H_
#define _NBGM_SKY_H_
#include "nbrenode.h"
#include "nbreentity.h"
#include "nbrevisitor.h"
#include "nbreshader.h"
#include "nbrecontext.h"
#include "nbgmlayoutelement.h"
#include "nbgmlayoutmanager.h"

class NBGM_Skywall;
class NBRE_SurfaceSubView;

/*! \addtogroup NBRE_Service
*  @{
*/

/** Defines an instance of a sky which could contain a box for background and a wall for blocking
*/

class NBGM_Sky: public NBRE_IVisitable, public NBRE_IRenderOperationProvider, public NBGM_LayoutElement, public NBGM_ILayoutProvider
{
public:
    static const int32 InvalideLayer = -1;

public:
    NBGM_Sky(NBGM_Context& context, int32 overlayId, nb_boolean tranparent, nb_boolean cubeSupported, NBRE_SurfaceSubView* subview);
    ~NBGM_Sky();

public:
    void SetWallSize(float width, float height, float distance);
    float GetWallDistance();
    void SetShader(const NBRE_ShaderPtr& shader);
    void SetShaderName(const NBRE_String& name);
    void SetDayNight(nb_boolean isDay);

public:
    ///From NBRE_IVisitable
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_IRenderQueueVisitable
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;
    virtual void UpdateSurfaceList() {};
    virtual uint32 GetPriority() const { return 0; }

    ///From NBRE_LayoutElement
    /// Is element visible
    virtual nb_boolean IsElementVisible() {return TRUE;}
    /// Update the region element takes
    virtual void UpdateRegion();
    /// Is element collide with buffer
    virtual nb_boolean IsElementRegionAvailable() {return TRUE;}
    /// Get position in world space
    virtual NBRE_Vector3d GetPosition() { return NBRE_Vector3d(0,0,0); }

    void SetOverlayId(int32 overlayId);

    virtual void UpdateLayoutList(NBGM_LayoutElementList& layoutElements);

private:
    void Initialize();
    void CreateSkyWallShader();
    void CreateSkyWall();
    void CreateSkyBoxShader();
    void CreateSkyBox();
    void CreateSkyBoxShaderCube();
    void CreateSkyBoxCube();
    void CreateSkyWallShader(const NBRE_String& groupName, const NBRE_String& texName);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_Sky);

private:
    /// render context
    NBRE_Context& mNBREContext;
    int32 mOverlayId;
    nb_boolean mVisible;
    NBRE_ShaderPtr   mShader;
    NBRE_String      mShaderName;
    NBRE_Matrix4x4f  mTransform;
    NBGM_Skywall*    mWall;
    nb_boolean mTransparent;
    nb_boolean mCubeTextureSupported;
    NBRE_Vector<NBRE_VertexData*> mVertexData;
    NBRE_IndexData*  mIndexData;
    float mDistance;
    NBRE_SurfaceSubView* mSubview;
};

/*! @} */

#endif
