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
#include "nbrenode.h"
#include "nbrevector3.h"
#include "nbretransformation.h"
#include "nbrespatialdividestrategy.h"
#include "nbretypeconvert.h"

NBRE_Node::NBRE_Node():
    mTransformOutOfDate(FALSE), 
    mRight(1, 0, 0), 
    mUp(0, 1, 0), 
    mDirection(0, 0, 1),
    mScale(1.0f, 1.0f, 1.0f),
    mParent(NULL), 
    mParentPositionNotified(FALSE),
    mNeedUpdatePos(TRUE),
    mSpatialDivideStrategy(NULL)
{
}

NBRE_Node::~NBRE_Node()
{
    for(NBRE_NodeList::iterator pNode = mChildrenNode.begin(); pNode != mChildrenNode.end(); ++pNode)
    {
        (*pNode)->SetParent(NULL);
        (*pNode)->SetSpatialDivideStrategy(NULL);
    }

}

uint32
NBRE_Node::GetChildrenNum() const
{
    uint32 total = 0;
    for(NBRE_NodeList::const_iterator pNode = mChildrenNode.begin(); pNode != mChildrenNode.end(); ++pNode)
    {
        total += (*pNode)->GetChildrenNum();
    }
    total += mMovableObjects.size();
    return total;
}

void NBRE_Node::UpdatePosition()
{
    if(mNeedUpdatePos || mParentPositionNotified)
    {
        if(mNeedUpdatePos)
        {
            UpdateTransform();

            for(MovableObjArray::iterator i = mMovableObjects.begin(); i < mMovableObjects.end(); ++i)
            {
                (*i)->NotifyUpdatePosition();
                if(mSpatialDivideStrategy)
                {
                    mSpatialDivideStrategy->UpdateSpatialObject((*i)->SpatialObject());
                }
            }
        }

        for(NBRE_NodeList::iterator pNode = mChildrenNode.begin(); pNode != mChildrenNode.end(); ++pNode)
        {
            (*pNode)->UpdatePosition();
        }
    }
    mNeedUpdatePos = FALSE;
    mParentPositionNotified = FALSE;

}


void NBRE_Node::SetParent(NBRE_Node* parent)
{
    mParent = parent;
}

void NBRE_Node::SetSpatialDivideStrategy(NBRE_ISpatialDivideStrategy* strategy)
{
    if(mSpatialDivideStrategy)
    {
        for(MovableObjArray::iterator i = mMovableObjects.begin(); i < mMovableObjects.end(); ++i)
        {
            mSpatialDivideStrategy->RemoveSpatialObject((*i)->SpatialObject());
        }
    }

    if(strategy && (!mNeedUpdatePos))
    {
        for(MovableObjArray::iterator i = mMovableObjects.begin(); i < mMovableObjects.end(); ++i)
        {
            (*i)->NotifyUpdatePosition();
            strategy->AddSpatialObject((*i)->SpatialObject());
        }
    }

    mSpatialDivideStrategy = strategy;

    for(NBRE_NodeList::iterator pNode = mChildrenNode.begin(); pNode != mChildrenNode.end(); ++pNode)
    {
        (*pNode)->SetSpatialDivideStrategy(strategy);
    }
}

void NBRE_Node::AddChild(const NBRE_NodePtr& node)
{
    nbre_assert(node->mParent == NULL);
    node->SetParent(this);
    node->SetSpatialDivideStrategy(mSpatialDivideStrategy);
    mChildrenNode.push_back(node);

    node->RequestPositionUpdate();
    RequestPositionUpdate();
}

nb_boolean NBRE_Node::RemoveChild(NBRE_Node* node)
{
    for(NBRE_NodeList::iterator pNode = mChildrenNode.begin(); pNode != mChildrenNode.end(); ++pNode)
    {
        if(pNode->get() == node)
        {
            mChildrenNode.erase(pNode);

            node->SetParent(NULL);
            node->SetSpatialDivideStrategy(NULL);

            return TRUE;
        }
        if((*pNode)->RemoveChild(node))
        {
            return TRUE;
        }
    }
    return FALSE;

}

const NBRE_NodeList& NBRE_Node::GetChildren()const
{
    return mChildrenNode;
}

void NBRE_Node::SetPosition(const NBRE_Vector3f& pos)
{
    SetPosition(NBRE_TypeConvertd::Convert(pos));

}

void NBRE_Node::SetPosition(const NBRE_Vector3d& pos)
{
    if (mPosition != pos)
    {
        RequestPositionUpdate();
    }
    mPosition = pos;

}

void NBRE_Node::Translate( const NBRE_Vector3f& deltaPos )
{
    Translate(NBRE_TypeConvertd::Convert(deltaPos));
}

void NBRE_Node::Translate( const NBRE_Vector3d& deltaPos )
{
    mPosition += deltaPos;

    RequestPositionUpdate();
}

void NBRE_Node::Rotate(const NBRE_Vector3f& axis, float theta)
{
    NBRE_Matrix4x4f transMat = NBRE_Transformationf::BuildRotateDegreesMatrix(theta, axis);

    // merge current matrix to camera matrix
    mRight      = transMat * mRight;
    mUp         = transMat * mUp;
    mDirection  = transMat * mDirection;

    RequestPositionUpdate();
}

void NBRE_Node::Rotate( const NBRE_Vector3f& axis, const NBRE_Vector3d& pos, float theta )
{
    NBRE_Vector3d vec = mPosition - pos;

    NBRE_Matrix4x4f transMat = NBRE_Transformationf::BuildRotateDegreesMatrix(theta, axis);
    NBRE_Matrix4x4d transMatd = NBRE_TypeConvertd::Convert(transMat);

    // merge current matrix to camera matrix
    vec         = transMatd * vec;
    mRight      = transMat * mRight;
    mUp         = transMat * mUp;
    mDirection  = transMat * mDirection;

    mPosition = vec + pos;

    RequestPositionUpdate();
}

void NBRE_Node::SetScale(const NBRE_Vector3f& scale)
{
    if(mScale != scale)
    {
        RequestPositionUpdate();
        
    }

    mScale = scale;

}

void NBRE_Node::SetOrientation(const NBRE_Orientation& orientation)
{
    NBRE_Matrix4x4f transMat = NBRE_Transformationf::BuildRotateDegreesMatrix(orientation.mAngle, orientation.mAxis);

    mRight = NBRE_Transformationf::CoordinatesGetXAxis(transMat);
    mUp = NBRE_Transformationf::CoordinatesGetYAxis(transMat);
    mDirection = NBRE_Transformationf::CoordinatesGetZAxis(transMat);

    RequestPositionUpdate();
}

const NBRE_Vector3d& NBRE_Node::Position() const
{
    return mPosition;
}

NBRE_Vector3d NBRE_Node::WorldPosition() const
{
    UpdateTransform();
    return NBRE_Transformationd::CoordinatesGetTranslate(mWorldTransform);
}

const NBRE_Vector3f& NBRE_Node::Direction() const
{
    return mDirection;
}

NBRE_Vector3f NBRE_Node::WorldDirection() const
{
    UpdateTransform();
    return NBRE_TypeConvertf::Convert(NBRE_Transformationd::CoordinatesGetZAxis(mWorldTransform));
}

const NBRE_Vector3f& NBRE_Node::Up() const
{
    return mUp;
}

const NBRE_Vector3f& NBRE_Node::Right() const
{
    return mRight;
}

NBRE_Vector3f NBRE_Node::WorldRight() const
{
    UpdateTransform();
    return NBRE_TypeConvertf::Convert(NBRE_Transformationd::CoordinatesGetXAxis(mWorldTransform));
}


void NBRE_Node::NotifyParentPositionChanged()
{
    if(mParentPositionNotified)
    {
        return;
    }

    mParentPositionNotified = TRUE;
    if(mParent)
    {
        mParent->NotifyParentPositionChanged();
    }
}


void NBRE_Node::SetTransform(const NBRE_Matrix4x4d& m)
{
    mLocaleTransform = m;
    RequestPositionUpdate();
}


const NBRE_Matrix4x4d& NBRE_Node::LocaleTransform() const
{
    UpdateTransform();
    return mLocaleTransform;
}

const NBRE_Matrix4x4d& NBRE_Node::WorldTransform() const
{
    UpdateTransform();
    return mWorldTransform;
}

void NBRE_Node::Accept(NBRE_Visitor& visitor)
{
    for(MovableObjArray::iterator i = mMovableObjects.begin(); i < mMovableObjects.end(); ++i)
    {
        (*i)->Accept(visitor);
    }
}

void NBRE_Node::AttachObject(const NBRE_MovableObjectPtr& obj)
{
    mMovableObjects.push_back(obj);
    obj->NotifyAttached(this);
    if(mSpatialDivideStrategy && (!mNeedUpdatePos))
    {
        obj->NotifyUpdatePosition();
        mSpatialDivideStrategy->AddSpatialObject(obj->SpatialObject());
    }

}

int32 NBRE_Node::AttachedObjectsNumber(void) const
{
    return mMovableObjects.size();
}

NBRE_IMovableObject* NBRE_Node::GetAttachedObject(int32 index)
{
    if(index < static_cast<int32>(mMovableObjects.size()))
    {
        return mMovableObjects[index].get();
    }
    return NULL;
}

NBRE_MovableObjectPtr NBRE_Node::DetachObject(int32 index)
{
    NBRE_MovableObjectPtr obj = mMovableObjects[index];
    obj->NotifyDetatch();
    if(mSpatialDivideStrategy)
    {
        mSpatialDivideStrategy->RemoveSpatialObject(obj->SpatialObject());
    }
    mMovableObjects.erase(mMovableObjects.begin()+index);
    return obj;
}

void NBRE_Node::DetachObject(NBRE_IMovableObject* obj)
{
    for(MovableObjArray::iterator i = mMovableObjects.begin(); i < mMovableObjects.end(); ++i)
    {
        if(i->get() == obj)
        {
            mMovableObjects.erase(i);
            obj->NotifyDetatch();
            if(mSpatialDivideStrategy)
            {
                mSpatialDivideStrategy->RemoveSpatialObject(obj->SpatialObject());
            }
            break;
        }
    }
}

void NBRE_Node::DetachAllObjects()
{
    for(MovableObjArray::iterator i = mMovableObjects.begin(); i < mMovableObjects.end(); ++i)
    {
        (*i)->NotifyDetatch();
        if(mSpatialDivideStrategy)
        {
            mSpatialDivideStrategy->RemoveSpatialObject((*i)->SpatialObject());
        }
    }
    mMovableObjects.clear();
}


void NBRE_Node::UpdateTransform() const
{
    if(mTransformOutOfDate)
    {

        mLocaleTransform = NBRE_Matrix4x4d(
            mRight.x*mScale.x,  mUp.x*mScale.x, mDirection.x*mScale.x,  mPosition.x,
            mRight.y*mScale.y,  mUp.y*mScale.y, mDirection.y*mScale.y,  mPosition.y,
            mRight.z*mScale.z,  mUp.z*mScale.z, mDirection.z*mScale.z,  mPosition.z,
            0.f,                0.f,            0.f,                    1.f);
        if(mParent)
        {
            mWorldTransform = mParent->WorldTransform()*mLocaleTransform;
        }
        else
        {
            mWorldTransform = mLocaleTransform;
        }
        mTransformOutOfDate = FALSE;
    }
}

void NBRE_Node::SetTransformOutOfDate()
{
    mNeedUpdatePos = TRUE;

    mTransformOutOfDate = TRUE;
    for(NBRE_NodeList::iterator pNode = mChildrenNode.begin(); pNode != mChildrenNode.end(); ++pNode)
    {
        (*pNode)->SetTransformOutOfDate();
    }
}

void NBRE_Node::RequestPositionUpdate()
{
    SetTransformOutOfDate();
    NotifyParentPositionChanged();
}

void NBRE_Node::CopyPosition(const NBRE_Node& node)
{
    mRight = node.mRight;
    mUp = node.mUp;
    mDirection = node.mDirection;
    mPosition = node.mPosition;
    mScale = node.mScale;
    RequestPositionUpdate();
}
