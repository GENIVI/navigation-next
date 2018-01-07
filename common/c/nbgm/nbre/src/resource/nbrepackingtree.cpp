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
#include "nbrepackingtree.h"
#include "nbrecommon.h"
#include "nbreintersection.h"

NBRE_PackingTreeNode::NBRE_PackingTreeNode(const NBRE_AxisAlignedBox2i& boundary, NBRE_PackingTreeNode* parent)
:mBoundary(boundary)
,mParent(parent)
,mIsEmpty(TRUE)
{
    mChild[0] = NULL;
    mChild[1] = NULL;
}

NBRE_PackingTreeNode::~NBRE_PackingTreeNode()
{
    NBRE_DELETE mChild[0];
    NBRE_DELETE mChild[1];
}

NBRE_PackingTreeNode*
NBRE_PackingTreeNode::Insert(const NBRE_Vector2i& rectSize, int32 padding, NBRE_AxisAlignedBox2i& resultRect)
{
    if (mChild[0] != NULL)
    {
        // non-leaf node
        NBRE_PackingTreeNode* resultNode = mChild[0]->Insert(rectSize, padding, resultRect);
        if (resultNode)
        {
            return resultNode;
        }
        else
        {
            return mChild[1]->Insert(rectSize, padding, resultRect);
        }
    }
    else
    {
        // leaf node
        if (!mIsEmpty)
        {
            // node used
            return NULL;
        }

        NBRE_Vector2i nodeSize(mBoundary.maxExtend.x - mBoundary.minExtend.x + 1 - padding, mBoundary.maxExtend.y - mBoundary.minExtend.y + 1 - padding);
        if (nodeSize.x < rectSize.x || nodeSize.y < rectSize.y)
        {
            // not enough space
            return NULL;
        }

        if (nodeSize == rectSize)
        {
            // rect exactly fits node
            resultRect = mBoundary;
            resultRect.minExtend = mBoundary.minExtend;
            resultRect.maxExtend.x = mBoundary.maxExtend.x - padding;
            resultRect.maxExtend.y = mBoundary.maxExtend.y - padding;
            mIsEmpty = FALSE;
            return this;
        }

        // node region is greater than rect
        // split node

        int32 dx = nodeSize.x - rectSize.x;
        int32 dy = nodeSize.y - rectSize.y;

        if (dx > dy)
        {
            // split along x axis
            mChild[0] = NBRE_NEW NBRE_PackingTreeNode(NBRE_AxisAlignedBox2i(
                mBoundary.minExtend.x, mBoundary.minExtend.y,
                mBoundary.minExtend.x + rectSize.x - 1 + padding, mBoundary.maxExtend.y), this);
            mChild[1] = NBRE_NEW NBRE_PackingTreeNode(NBRE_AxisAlignedBox2i(
                mBoundary.minExtend.x + rectSize.x + padding, mBoundary.minExtend.y,
                mBoundary.maxExtend.x, mBoundary.maxExtend.y), this);
        }
        else
        {
            // split along y axis
            mChild[0] = NBRE_NEW NBRE_PackingTreeNode(NBRE_AxisAlignedBox2i(
                mBoundary.minExtend.x, mBoundary.minExtend.y,
                mBoundary.maxExtend.x, mBoundary.minExtend.y + rectSize.y - 1 + padding), this);
            mChild[1] = NBRE_NEW NBRE_PackingTreeNode(NBRE_AxisAlignedBox2i(
                mBoundary.minExtend.x, mBoundary.minExtend.y + rectSize.y + padding,
                mBoundary.maxExtend.x, mBoundary.maxExtend.y), this);
        }
        return mChild[0]->Insert(rectSize, padding, resultRect);
    }
}

NBRE_PackingTree::NBRE_PackingTree(const NBRE_Vector2i& size, int32 padding)
:mPadding(padding)
{
    mRoot = NBRE_NEW NBRE_PackingTreeNode(NBRE_AxisAlignedBox2i(0, 0, size.x - 1, size.y - 1), NULL);
}

NBRE_PackingTree::~NBRE_PackingTree()
{
    NBRE_DELETE mRoot;
}

NBRE_PackingTreeNode*
NBRE_PackingTree::Insert(const NBRE_Vector2i& rectSize, NBRE_AxisAlignedBox2i& resultRect)
{
    return mRoot->Insert(rectSize, mPadding, resultRect);
}

void
NBRE_PackingTree::Remove(NBRE_PackingTreeNode* node)
{
    while (node)
    {
        node->mIsEmpty = TRUE;
        NBRE_DELETE node->mChild[0];
        NBRE_DELETE node->mChild[1];
        node->mChild[0] = NULL;
        node->mChild[1] = NULL;

        if (node->mParent)
        {
            NBRE_PackingTreeNode* p = node->mParent;
            NBRE_PackingTreeNode* sibling = NULL;
            if (p->mChild[0] == node)
            {
                sibling = p->mChild[1];
            }
            else
            {
                sibling = p->mChild[0];
            }

            if (sibling->mChild[0] == NULL && sibling->mIsEmpty)
            {
                p->mChild[0] = NULL;
                p->mChild[1] = NULL;
                p->mIsEmpty = TRUE;
                delete sibling;
                delete node;
                node = p;
            }
            else
            {
                node = NULL;
            }
        }
        else
        {
            node = NULL;
        }
    }
}
