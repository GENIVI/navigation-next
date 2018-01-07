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
#include "nbreaabbtree2d.h"
#include "nbreintersection.h"
#include "nbrecollisiontest.h"

NBRE_AABBTree2d::NBRE_AABBTree2d()
:mRoot(NBRE_NEW Node(NULL))
{
}

NBRE_AABBTree2d::~NBRE_AABBTree2d()
{
    NBRE_DELETE mRoot;
}

nb_boolean
NBRE_AABBTree2d::HitTest(NBRE_ICollisionPrimitive2d* obj)
{
    return mRoot->HitTest(obj);
}

nb_boolean
NBRE_AABBTree2d::HitTest(NBRE_IBoundTree2d* tree)
{
    typedef NBRE_Pair<Node*,Node*> CollisionNodePair;
    typedef NBRE_Vector<CollisionNodePair> CollisionPairList;

    NBRE_AABBTree2d* aabbTree = (NBRE_AABBTree2d*)tree;
    CollisionPairList collisionPairs;

    if (NBRE_Intersectiond::HitTest(mRoot->mAABB, aabbTree->mRoot->mAABB))
    {
        collisionPairs.push_back(CollisionNodePair(this->mRoot, aabbTree->mRoot));
        while(collisionPairs.size() > 0)
        {
            CollisionNodePair& p = collisionPairs.back();
            Node* n1 = p.first;
            Node* n2 = p.second;
            collisionPairs.pop_back();
            if (n1->mPrimitive)
            {
                if (n2->mPrimitive)
                {
                    if (n1->mPrimitive->HitTest(n2->mPrimitive))
                    {
                        return TRUE;
                    }
                }
                else
                {
                    for (uint32 j = 0; j < 2; ++j)
                    {
                        if (NBRE_Intersectiond::HitTest(n1->mPrimitive->GetAABB(), n2->mChildNodes[j]->mAABB))
                        {
                            collisionPairs.push_back(CollisionNodePair(n1, n2->mChildNodes[j]));
                        }
                    }
                }
            }
            else
            {
                if (n2->mPrimitive)
                {
                    for (uint32 i = 0; i < 2; ++i)
                    {
                        if (NBRE_Intersectiond::HitTest(n1->mChildNodes[i]->mAABB, n2->mPrimitive->GetAABB()))
                        {
                            collisionPairs.push_back(CollisionNodePair(n1->mChildNodes[i], n2));
                        }
                    }
                }
                else
                {
                    for (uint32 i = 0; i < 2; ++i)
                    {
                        for (uint32 j = 0; j < 2; ++j)
                        {
                            if (NBRE_Intersectiond::HitTest(n1->mChildNodes[i]->mAABB, n2->mChildNodes[j]->mAABB))
                            {
                                collisionPairs.push_back(CollisionNodePair(n1->mChildNodes[i], n2->mChildNodes[j]));
                            }
                        }
                    }

                }
            }
        }
    }
    return FALSE;
}

void
NBRE_AABBTree2d::BuildTree(NBRE_ICollisionPrimitive2d** primitives, uint32 count)
{
    if (count == 0)
    {
        return;
    }

    if (mRoot)
    {
        NBRE_DELETE mRoot;
        mRoot = NULL;
    }

    NBRE_ICollisionPrimitive2d** vms = NBRE_NEW NBRE_ICollisionPrimitive2d*[count];
    nsl_memcpy(vms, primitives, sizeof(NBRE_ICollisionPrimitive2d*) * count);

    BuildNode(&mRoot, vms, 0, count - 1);

    NBRE_DELETE_ARRAY vms;
}

void
NBRE_AABBTree2d::BuildNode(Node** node, NBRE_ICollisionPrimitive2d** primitives, uint32 beginIndex, uint32 endIndex)
{
    if (beginIndex == endIndex)
    {
        *node = new Node(primitives[beginIndex]);
    }
    else
    {
        *node = new Node(NULL);
        Node* n = *node;

        CalculateAABB(primitives, beginIndex, endIndex, n->mAABB);
        
        uint32 splitIndex = SplitPrimitives(primitives, beginIndex, endIndex);
        BuildNode(&n->mChildNodes[0], primitives, beginIndex, splitIndex - 1);
        BuildNode(&n->mChildNodes[1], primitives, splitIndex, endIndex);
    }
}

static bool
SortPrimitiveByX(NBRE_ICollisionPrimitive2d* primitive1, NBRE_ICollisionPrimitive2d* primitive2)
{
    return primitive1->GetAABB().minExtend.x < primitive2->GetAABB().minExtend.x;
}

static bool
SortPrimitiveByY(NBRE_ICollisionPrimitive2d* primitive1, NBRE_ICollisionPrimitive2d* primitive2)
{
    return primitive1->GetAABB().minExtend.y < primitive2->GetAABB().minExtend.y;
}

double
NBRE_AABBTree2d::GetSplitTotalArea(NBRE_ICollisionPrimitive2d** primitives, uint32 beginIndex, uint32 endIndex, uint32 splitFrom)
{
    double area = 0;
    NBRE_AxisAlignedBox2d aabb;
    aabb.SetNull();
    CalculateAABB(primitives, beginIndex, beginIndex + splitFrom - 1, aabb);
    area += GetAABBArea(aabb);
    aabb.SetNull();
    CalculateAABB(primitives, beginIndex + splitFrom, endIndex, aabb);
    area += GetAABBArea(aabb);
    return area;
}

uint32
NBRE_AABBTree2d::SplitPrimitives(NBRE_ICollisionPrimitive2d** primitives, uint32 beginIndex, uint32 endIndex)
{
    uint32 count = endIndex - beginIndex + 1;
    uint32 splitFrom = count / 2;
    if (count > 2)
    {
        // Calculate the total node area if split along x axis
        NBRE_ICollisionPrimitive2d** vms = NBRE_NEW NBRE_ICollisionPrimitive2d*[count];
        nsl_memcpy(vms, primitives + beginIndex, sizeof(NBRE_ICollisionPrimitive2d*) * count);
        std::sort(vms, vms + count - 1, SortPrimitiveByX);
        double areaX = GetSplitTotalArea(vms, 0, count - 1, splitFrom);

        // Calculate the total node area if split along y axis
        NBRE_ICollisionPrimitive2d** vmsY = NBRE_NEW NBRE_ICollisionPrimitive2d*[count];
        nsl_memcpy(vmsY, primitives + beginIndex, sizeof(NBRE_ICollisionPrimitive2d*) * count);
        std::sort(vmsY, vmsY + count - 1, SortPrimitiveByY);
        double areaY = GetSplitTotalArea(vmsY, 0, count - 1, splitFrom);

        nsl_memcpy(primitives + beginIndex, areaY < areaX ? vmsY : vms, sizeof(NBRE_ICollisionPrimitive2d*) * count);
        
        NBRE_DELETE_ARRAY vms;
        NBRE_DELETE_ARRAY vmsY;
    }
    return beginIndex + splitFrom;
}

void
NBRE_AABBTree2d::CalculateAABB(NBRE_ICollisionPrimitive2d** primitives, uint32 beginIndex, uint32 endIndex, NBRE_AxisAlignedBox2d& result)
{
    result.SetNull();
    for (uint32 i = beginIndex; i <= endIndex; ++i)
    {
        result.Merge(primitives[i]->GetAABB());
    }
}

double
NBRE_AABBTree2d::GetAABBArea(const NBRE_AxisAlignedBox2d& aabb)
{
    double dx = aabb.maxExtend.x - aabb.minExtend.x;
    double dy = aabb.maxExtend.y - aabb.minExtend.y;
    return dx * dy;
}

void
NBRE_AABBTree2d::GetNodeAABBs(NBRE_Vector<NBRE_AxisAlignedBox2d>& aabbs)
{
    NBRE_Vector<Node*> ns;
    ns.push_back(mRoot);
    while (ns.size() > 0)
    {
        Node* n = ns.back();
        ns.pop_back();
        if (!n->mPrimitive)
        {
            aabbs.push_back(n->mAABB);
            ns.push_back(n->mChildNodes[0]);
            ns.push_back(n->mChildNodes[1]);
        }
        else
        {
            aabbs.push_back(n->mPrimitive->GetAABB());
        }
    }
}

void
NBRE_AABBTree2d::GetPrimitives(NBRE_Vector<NBRE_ICollisionPrimitive2d*>& result)
{
    NBRE_Vector<Node*> ns;
    ns.push_back(mRoot);
    while (ns.size() > 0)
    {
        Node* n = ns.back();
        ns.pop_back();
        if (n->mPrimitive)
        {
            result.push_back(n->mPrimitive);
        }
        else
        {
            ns.push_back(n->mChildNodes[0]);
            ns.push_back(n->mChildNodes[1]);
        }
    }
}

NBRE_AABBTree2d::Node::Node(NBRE_ICollisionPrimitive2d* obj)
:mPrimitive(obj)
{
    mChildNodes[0] = mChildNodes[1] = NULL;
}

NBRE_AABBTree2d::Node::~Node()
{
    NBRE_DELETE mChildNodes[0];
    NBRE_DELETE mChildNodes[1];
}

nb_boolean
NBRE_AABBTree2d::Node::HitTest(NBRE_ICollisionPrimitive2d* obj)
{
    if (mPrimitive)
    {
        return mPrimitive->HitTest(obj);
    }
    else
    {
        if (NBRE_Intersectiond::HitTest(mAABB, obj->GetAABB()))
        {
            if (mChildNodes[0]->HitTest(obj))
            {
                return TRUE;
            }
            if (mChildNodes[1]->HitTest(obj))
            {
                return TRUE;
            }
        }
        return FALSE;
    }
}

