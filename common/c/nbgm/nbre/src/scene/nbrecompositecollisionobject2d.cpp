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
#include "nbrecompositecollisionobject2d.h"
#include "nbreprimitivecollisionobject2d.h"

NBRE_CompositeCollisionObject2d::NBRE_CompositeCollisionObject2d(void* owner, NBRE_IBoundTree2d* boundTree)
:NBRE_CollisionObject2d(owner)
,mBoundTree(boundTree)
,mPrimitiveChnaged(FALSE)
{
}

NBRE_CompositeCollisionObject2d::~NBRE_CompositeCollisionObject2d()
{
    NBRE_DELETE mBoundTree;
    for (uint32 i = 0; i < mPrimitives.size(); ++i)
    {
        NBRE_DELETE mPrimitives[i];
    }
}

void
NBRE_CompositeCollisionObject2d::AddPrimitive(NBRE_ICollisionPrimitive2d* primitive)
{
    mPrimitives.push_back(primitive);
    mPrimitiveChnaged = TRUE;
}

void
NBRE_CompositeCollisionObject2d::Build()
{
    if (mPrimitiveChnaged && mPrimitives.size() > 0)
    {
        mBoundTree->BuildTree(&mPrimitives[0], mPrimitives.size());
        mPrimitiveChnaged = FALSE;
    }
}

nb_boolean
NBRE_CompositeCollisionObject2d::HitTest(NBRE_CollisionObject2d* obj)
{
    nb_boolean result = FALSE;
    NBRE_CollisionObjectType type = obj->GetType();
    
    switch (type)
    {
    case NBRE_COT_SINGLE:
        {
            NBRE_PrimitiveCollisionObject2d* po = (NBRE_PrimitiveCollisionObject2d*)obj;
            result = mBoundTree->HitTest(po->GetPrimitive());
        }
        break;
    case NBRE_COT_COMPOSITE:
        {
            NBRE_CompositeCollisionObject2d* po = (NBRE_CompositeCollisionObject2d*)obj;
            result = mBoundTree->HitTest(po->mBoundTree);
        }
        break;
    }
    return result;
}

nb_boolean
NBRE_CompositeCollisionObject2d::HitTest(NBRE_ICollisionPrimitive2d* primitive)
{
    return mBoundTree->HitTest(primitive);
}

void
NBRE_CompositeCollisionObject2d::GetPrimitives(NBRE_Vector<NBRE_ICollisionPrimitive2d*>& result)
{
    for (uint32 i = 0; i < mPrimitives.size(); ++i)
    {
        result.push_back(mPrimitives[i]);
    }
}