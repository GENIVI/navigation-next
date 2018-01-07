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

    @file nbreentity.h
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
#ifndef _NBRE_ENTITY_H_
#define _NBRE_ENTITY_H_

#include "nbretypes.h"
#include "nbremesh.h"
#include "nbresubentity.h"
#include "nbrerenderoperation.h"
#include "nbreipassoperation.h"
#include "nbremovableobject.h"
#include "nbrespatialobject.h"
#include "nbrevertexanimation.h"
#include "nbrecontext.h"
#include "nbrerendersurface.h"
#include "nbrenoneaxisalignedbox3.h"
#include "nbremodel.h"

class NBRE_Overlay;

/*! \addtogroup NBRE_Scene
*  @{
*/

class NBRE_Entity;

class NBRE_IEntityListener
{
public:
    virtual void OnPickedUp(NBRE_Entity* ent) = 0;    
};

/*! Defines an instance of a discrete, movable object based on a Mesh.
@remarks
    The NBRE_Mesh and NBRE_SubMesh classes deal with the definition of the geometry
    used by discrete movable objects. Entities are actual instances of
    objects based on this geometry in the world. Therefore there is
    usually a single set Mesh for a car, but there may be multiple
    entities based on it in the world. Entities are able to override
    aspects of the Mesh it is defined by, such as changing material
    properties per instance (so you can have many cars using the same
    geometry but different textures for example).
*/
class NBRE_Entity: public NBRE_IMovableObject, 
                   public NBRE_IRenderOperationProvider, 
                   public NBRE_ISpatialObject,
                   public NBRE_IPickedUpObject
{
    typedef NBRE_Vector<NBRE_SubEntity*> NBRE_SubEntityList;
    typedef NBRE_Vector<NBRE_PassPtr> PassArray;
    static const int32 InvalideOverlayId = -1;
    typedef NBRE_List<NBRE_RenderSurface*> SurfaceList;

public:
    explicit NBRE_Entity(NBRE_Context& context, int32 overlayId);
    NBRE_Entity(NBRE_Context& context, const NBRE_ModelPtr& model, int32 overlayId);
    ~NBRE_Entity();

public:

    ///Derive From NBRE_IMovableObject
    virtual void NotifyAttached(NBRE_Node* node);
    virtual void NotifyDetatch();
    virtual void NotifyUpdatePosition();
    virtual NBRE_ISpatialObject* SpatialObject();
    virtual void Accept(NBRE_Visitor& visitor);

    ///From NBRE_IRenderOperationProvider
    virtual void NotifyCamera(const NBRE_Camera& camera);
    virtual void UpdateRenderQueue(NBRE_Overlay& overlay);
    virtual int32 OverlayId() const;
    virtual nb_boolean Visible() const;    
    virtual void UpdateSurfaceList();
    virtual uint32 GetPriority() const{return mPriority;}

    ///From NBRE_ISpatialObject
    virtual const NBRE_AxisAlignedBox3d& AxisAlignedBox()const;

    ///From NBRE_IPickedUpObject
    virtual void OnPickedUp();
    virtual nb_boolean IsIntersect(const NBRE_Camera& camera, const NBRE_Ray3f& ray, NBRE_Point3f* pIntersectPoint);

    const NBRE_NoneAxisAlignedBox3d& BoundingBox()const;
    const NBRE_AxisAlignedBox3d& AxisAlignedBox();
    void SetModel(const NBRE_ModelPtr& model);
    const NBRE_ModelPtr& GetModel() const;
    void SetOverlayId(int32 overlayId);
    void SetPickListener(NBRE_IEntityListener* listener);

    const NBRE_SubEntity& GetSubEntity(uint32 index) const;
    NBRE_SubEntity& GetSubEntity(uint32 index);
    uint32 GetSubEntityNumber() const;

    void SetShader(const NBRE_ShaderPtr& shader);
    void SetShaderName(const NBRE_String& name);

    void SetVisible(nb_boolean visable);

    void AddPassOperation(NBRE_IPassOperation* passOperation);

    void SetVertexAnimation(shared_ptr<NBRE_IVertexAnimation> vertexAnimation);

    void SetTextureAnimation(const NBRE_Set<NBRE_SubModel*>& animatedSubModels, NBRE_ITextureAnimation* textureAnimation);

    void AddRefSurface(NBRE_RenderSurface* surface);

    void SetName(const NBRE_String& name);

    void SetPriority(uint32 priority){mPriority = priority;}

    NBRE_Entity* Duplicate();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_Entity);

private:
    void GetRenderOperation(NBRE_SubEntity* entity, NBRE_Overlay& overlay);
    void GetRenderOperation(NBRE_SubEntity* entity, NBRE_RenderOperation& renderOp, const NBRE_PassPtr& pass);
    NBRE_PassPtr GetCachedPass();

private:
    NBRE_Context& mContext;
    /*! The Model that this Entity is based on.
    */
    NBRE_ModelPtr mModel;

    /*! If this entity is visible or not.
    */
    nb_boolean mVisible;

    NBRE_SubEntityList  mSubEntityList;

    /// Every pass will be modified by this chain before rendering
    NBRE_PassOperationList mPassOperations;

    PassArray mPassCaches;
    int32 mCurrentPass;

    int32 mOverlayId;

    NBRE_AxisAlignedBox3d mAxisAlignedBox;     /*!< entity world bounding box*/
    NBRE_NoneAxisAlignedBox3d mBoundingBox;     /*!< entity world bounding box*/
    ///attached node
    NBRE_Node* mParentNode;

    NBRE_IEntityListener* mPickListener;

    SurfaceList mRefSurfaces;
    NBRE_String mEntityName;

    uint32 mPriority;
};

typedef shared_ptr<NBRE_Entity> NBRE_EntityPtr;

/*! @} */

#endif