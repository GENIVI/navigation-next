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

    @file nbrenode.h
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
#ifndef _NBRE_NODE_H_
#define _NBRE_NODE_H_
#include "nbretypes.h"
#include "nbrematrix4x4.h"
#include "nbrevector3.h"
#include "nbreaxisalignedbox3.h"
#include "nbrelight.h"
#include "nbresharedptr.h"
#include "nbreorientation.h"
#include "nbrevisitor.h"
#include "nbremovableobject.h"

class NBRE_Node;
typedef shared_ptr<NBRE_Node> NBRE_NodePtr;

typedef NBRE_List<NBRE_NodePtr> NBRE_NodeList;

class NBRE_ISpatialDivideStrategy;

/*! \addtogroup NBRE_Scene
*  @{
*/

class NBRE_Node
{

typedef NBRE_Vector<NBRE_MovableObjectPtr> MovableObjArray;

public:
    NBRE_Node();
    virtual ~NBRE_Node();

public:
    uint32 GetChildrenNum() const;

    void CopyPosition(const NBRE_Node& node);

    ///update node's current position, notify movable
    virtual void UpdatePosition();

    /** Adds an instance of a scene object to this node.
    @remarks
        Scene objects can include Entity objects, Camera objects, Light objects, 
        ParticleSystem objects etc. Anything that subclasses from MovableObject.
    */
    void AttachObject(const NBRE_MovableObjectPtr& obj);
    
    /** Reports the number of objects attached to this node.
    */
    int32 AttachedObjectsNumber() const;

    /** Retrieves a pointer to an attached object.
    @remarks Retrieves by index, see alternate version to retrieve by name. The index
    of an object may change as other objects are added / removed.
    */
    NBRE_IMovableObject* GetAttachedObject(int32 index);

    /** Detaches the indexed object from this scene node.
    @remarks
        Detaches by index, see the alternate version to detach by name. Object indexes
        may change as other objects are added / removed.
    */
    NBRE_MovableObjectPtr DetachObject(int32 index);

    /** Detaches an object by pointer. */
    void DetachObject(NBRE_IMovableObject* obj);


    /** Detaches all objects attached to this node.
    */
    void DetachAllObjects();

    void SetParent(NBRE_Node* parent);
    void SetSpatialDivideStrategy(NBRE_ISpatialDivideStrategy* strategy);

    void AddChild(const NBRE_NodePtr& node);
    nb_boolean RemoveChild(NBRE_Node* node);
    const NBRE_NodeList& GetChildren()const;

    void SetPosition(const NBRE_Vector3f& pos);
    void SetPosition(const NBRE_Vector3d& pos);
    void SetScale(const NBRE_Vector3f& scale);
    void SetOrientation(const NBRE_Orientation& orient);

    /// Retrieves the node's locale position.
    const NBRE_Vector3d& Position() const;

    /// Retrieves the node's world position.
    NBRE_Vector3d WorldPosition() const;

    /// Retrieves the node's locale direction.
    const NBRE_Vector3f& Direction() const;

    /// Gets the node's world direction.
    NBRE_Vector3f WorldDirection() const;

    /// Gets the camera's up vector.
    const NBRE_Vector3f& Up() const;

    /// Gets the node's world up.
    NBRE_Vector3f WorldUp() const;

    /// Gets the camera's right vector.
    const NBRE_Vector3f& Right() const;

    /// Gets the node's world right.
    NBRE_Vector3f WorldRight() const;

    void Translate( const NBRE_Vector3f& deltaPos );
    void Translate( const NBRE_Vector3d& deltaPos );
    void Rotate(const NBRE_Vector3f& axis, float theta);
    void Rotate(const NBRE_Vector3f& axis, const NBRE_Vector3d& pos, float theta);

    void SetTransform(const NBRE_Matrix4x4d& m);

    //Get Locale Matrix
    const NBRE_Matrix4x4d& LocaleTransform() const; 

    //Get World Matrix
    const NBRE_Matrix4x4d& WorldTransform() const;

    ///TODO: why need this? Accept() comes from NBRE_IVistable?
    virtual void Accept(NBRE_Visitor& visitor);

protected:
    void NotifyParentPositionChanged();
    void UpdateTransform() const;
    void SetTransformOutOfDate();
    void RequestPositionUpdate();
    void SetParentPositionUpdated();
    void SetMoveableUpdated();

protected:

    mutable NBRE_Matrix4x4d mLocaleTransform;
    mutable NBRE_Matrix4x4d mWorldTransform;
    mutable nb_boolean mTransformOutOfDate;
 
    NBRE_Vector3f   mRight;         //u
    NBRE_Vector3f   mUp;            //v
    NBRE_Vector3f   mDirection;     //n

    /// Stores the position/translation of the node relative to its parent.
    NBRE_Vector3d mPosition;

    /// Stores the scaling factor applied to this node
    NBRE_Vector3f mScale;

    NBRE_Node* mParent;
    NBRE_NodeList mChildrenNode;

    MovableObjArray mMovableObjects;

    nb_boolean mParentPositionNotified;
    
    nb_boolean mNeedUpdatePos;

    NBRE_ISpatialDivideStrategy* mSpatialDivideStrategy;
};

/*! @} */
#endif
