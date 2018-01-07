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
#include "nbrebinaryspatialdividemanager.h"
#include "nbreintersection.h"
#include "nbrespatialobject.h"
#include "nbretypeconvert.h"

NBRE_BinarySpatialDivideManager::NBRE_BinarySpatialDivideManager(): mRootZone(NULL)
{
}

NBRE_BinarySpatialDivideManager::~NBRE_BinarySpatialDivideManager()
{
}

void NBRE_BinarySpatialDivideManager::AddSpatialObject(NBRE_ISpatialObject* obj)
{
    if(obj == NULL)
    {
        return;
    }
    if(mRootZone == NULL)
    {
        mRootZone = NBRE_NEW NBRE_BinaryZone();
        mRootZone->SetBoundingbox(NBRE_TypeConvertf::Convert(obj->AxisAlignedBox()));
        mRootZone->AddSpatialObject(obj);
    }
    else 
    {
        if(NBRE_TypeConvertd::Convert(mRootZone->AxisAlignedBox()).Contain(obj->AxisAlignedBox()))
        {
        }
        else if(NBRE_Intersectiond::HitTest(NBRE_TypeConvertd::Convert(mRootZone->AxisAlignedBox()), obj->AxisAlignedBox()))
        {
        }
    }
    //if(obj->AxisAlignedBox())
    mSpatilaObjs.insert(obj);

}

void NBRE_BinarySpatialDivideManager::AddSpatialObject(NBRE_BinaryZone* zone, NBRE_ISpatialObject* obj)
{
    NBRE_BinaryZone* left = zone->LeftChild();
    NBRE_BinaryZone* right = zone->RightChild();

    if(left)
    {
        if(NBRE_TypeConvertd::Convert(left->AxisAlignedBox()).Contain(obj->AxisAlignedBox()))
        {
            zone->AddSubZoneSpatialObject(obj);
            AddSpatialObject(left, obj);
            return;
        }
    }
    
    if(right)
    {
        if(NBRE_TypeConvertd::Convert(right->AxisAlignedBox()).Contain(obj->AxisAlignedBox()))
        {
            zone->AddSubZoneSpatialObject(obj);
            AddSpatialObject(right, obj);
            return;
        }
    }

    if(left && right)
    {
        nb_boolean leftIntersect = NBRE_Intersectiond::HitTest(NBRE_TypeConvertd::Convert(left->AxisAlignedBox()), obj->AxisAlignedBox());
        nb_boolean rightIntersect = NBRE_Intersectiond::HitTest(NBRE_TypeConvertd::Convert(right->AxisAlignedBox()), obj->AxisAlignedBox());

        if(leftIntersect && rightIntersect)
        {
            zone->AddSpatialObject(obj);
        }
        else if(leftIntersect)
        {
            left->AxisAlignedBox().Merge(NBRE_TypeConvertf::Convert(obj->AxisAlignedBox()));
            AddSpatialObject(left, obj);
        }
        else if(rightIntersect)
        {
            right->AxisAlignedBox().Merge(NBRE_TypeConvertf::Convert(obj->AxisAlignedBox()));
            AddSpatialObject(right, obj);
        }
        else 
        {
            NBRE_AxisAlignedBox3d box = NBRE_TypeConvertd::Convert(left->AxisAlignedBox());
            box.Merge(NBRE_TypeConvertd::Convert(right->AxisAlignedBox()));
            if(NBRE_Intersectiond::HitTest(box, obj->AxisAlignedBox()))
            {
                box = obj->AxisAlignedBox();
                box.Merge(NBRE_TypeConvertd::Convert(left->AxisAlignedBox()));
                if(NBRE_Intersectiond::HitTest(box, obj->AxisAlignedBox()))
                {
                    box = obj->AxisAlignedBox();
                    box.Merge(NBRE_TypeConvertd::Convert(right->AxisAlignedBox()));

                    right->SetBoundingbox(NBRE_TypeConvertf::Convert(box));
                    AddSpatialObject(right, obj);
                }
                else
                {
                    left->SetBoundingbox(NBRE_TypeConvertf::Convert(box));
                    AddSpatialObject(left, obj);
                }
            }
            else
            {
                zone->RemoveLeftChild();
                zone->RemoveRightChild();

                zone->CreateLeftChild();
                zone->LeftChild()->SetBoundingbox(NBRE_TypeConvertf::Convert(box));
                zone->LeftChild()->SetLeftChild(left);
                zone->LeftChild()->SetRightChild(right);

                zone->CreateRightChild();
                zone->RightChild()->SetBoundingbox(NBRE_TypeConvertf::Convert(obj->AxisAlignedBox()));
                zone->RightChild()->AddSpatialObject(obj);
            }
        }
        return ;
    }

    nbre_assert(left || right);

    NBRE_SpatialObjectList& children = zone->SpatialObjects();
    NBRE_AxisAlignedBox3d box = NBRE_TypeConvertd::Convert(obj->AxisAlignedBox());

    for(NBRE_SpatialObjectList::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        if(!NBRE_Intersectiond::HitTest(box, (*iter)->AxisAlignedBox()))
        {
            zone->CreateLeftChild();
            zone->LeftChild()->SetBoundingbox(NBRE_TypeConvertf::Convert((*iter)->AxisAlignedBox()));
            zone->LeftChild()->AddSpatialObject(*iter);

            zone->CreateRightChild();
            zone->RightChild()->SetBoundingbox(NBRE_TypeConvertf::Convert(obj->AxisAlignedBox()));
            zone->RightChild()->AddSpatialObject(obj);

            children.erase(iter);
            return;
        }
    }

    zone->AddSpatialObject(obj);

    //if(NBRE_TypeConvertd::Convert(zone->AxisAlignedBox()).Contain(obj->AxisAlignedBox()))
    //{
    //    //AddSpatialObject(zone->LeftChild(), );
    //}
    //else if(NBRE_Intersectiond::HitTest(NBRE_TypeConvertd::Convert(zone->AxisAlignedBox()), obj->AxisAlignedBox()))
    //{
    //}
    //else
    //{
    //}
}

void NBRE_BinarySpatialDivideManager::RemoveSpatialObject(NBRE_ISpatialObject* obj)
{
    if(obj == NULL)
    {
        return;
    }


    mSpatilaObjs.erase(obj);
}

void NBRE_BinarySpatialDivideManager::WalkHitTestSpatialObject(NBRE_ISpaceHitTest* hittest, NBRE_Visitor& visitor)
{

    for(SpatialObjectArray::const_iterator pObj = mSpatilaObjs.begin(); pObj != mSpatilaObjs.end(); ++pObj)
    {
        if(hittest->Intersect(NBRE_TypeConvertf::Convert((*pObj)->AxisAlignedBox())))
        {
            (*pObj)->Accept(visitor);
        }
    }
}

void NBRE_BinarySpatialDivideManager::WalkAllSpatialObject(NBRE_Visitor& visitor)
{
    for(SpatialObjectArray::const_iterator pObj = mSpatilaObjs.begin(); pObj != mSpatilaObjs.end(); ++pObj)
    {
        (*pObj)->Accept(visitor);
    }
}
