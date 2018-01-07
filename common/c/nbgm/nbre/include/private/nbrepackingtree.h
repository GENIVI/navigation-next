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

@file nbrepackingtree.h
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
#ifndef _NBRE_PACKING_TREE_H
#define _NBRE_PACKING_TREE_H
#include "nbretypes.h"
#include "nbrevector2.h"
#include "nbreaxisalignedbox2.h"

/** \addtogroup NBRE_Resource
*  @{
*/

/** 
A packing tree node.
*/
class NBRE_PackingTreeNode
{
    friend class NBRE_PackingTree;
public:
    NBRE_PackingTreeNode(const NBRE_AxisAlignedBox2i& boundary, NBRE_PackingTreeNode* parent);
    ~NBRE_PackingTreeNode();

public:
    /// Insert a rect into tree, if success, return node and set resultRect to tree region, else return NULL
    NBRE_PackingTreeNode* Insert(const NBRE_Vector2i& rectSize, int32 padding, NBRE_AxisAlignedBox2i& resultRect);

private:
    NBRE_AxisAlignedBox2i mBoundary;
    NBRE_PackingTreeNode* mParent;
    NBRE_PackingTreeNode* mChild[2];
    nb_boolean mIsEmpty;
};

/** 
Packing tree can merge small rectangles into a large one.
Use this tree to merge small textures to improve performance.
*/
class NBRE_PackingTree
{
public:
    NBRE_PackingTree(const NBRE_Vector2i& size, int32 padding);
    ~NBRE_PackingTree();

public:
    /// Insert a rect into tree, if success, return node and set resultRect to tree region, else return NULL
    NBRE_PackingTreeNode* Insert(const NBRE_Vector2i& rectSize, NBRE_AxisAlignedBox2i& resultRect);
    /// Remove a node and recycle the taken space
    void Remove(NBRE_PackingTreeNode* node);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_PackingTree);

private:
    /// Root node
    NBRE_PackingTreeNode* mRoot;
    /// Padding between rect
    int32 mPadding;
};

/** @} */
#endif
