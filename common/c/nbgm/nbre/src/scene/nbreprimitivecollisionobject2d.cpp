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
#include "nbreprimitivecollisionobject2d.h"
#include "nbrecompositecollisionobject2d.h"

NBRE_PrimitiveCollisionObject2d::NBRE_PrimitiveCollisionObject2d(void* owner, NBRE_ICollisionPrimitive2d* primitive)
:NBRE_CollisionObject2d(owner)
,mPrimitive(primitive)
{
}

NBRE_PrimitiveCollisionObject2d::~NBRE_PrimitiveCollisionObject2d()
{
    NBRE_DELETE mPrimitive;
}

nb_boolean
NBRE_PrimitiveCollisionObject2d::HitTest(NBRE_CollisionObject2d* obj)
{
    nb_boolean result = FALSE;
    NBRE_CollisionObjectType type = obj->GetType();
    
    switch (type)
    {
    case NBRE_COT_SINGLE:
        {
            NBRE_PrimitiveCollisionObject2d* po = (NBRE_PrimitiveCollisionObject2d*)obj;
            result = mPrimitive->HitTest(po->mPrimitive);
        }
        break;
    case NBRE_COT_COMPOSITE:
        {
            NBRE_CompositeCollisionObject2d* po = (NBRE_CompositeCollisionObject2d*)obj;
            result = po->HitTest(mPrimitive);
        }
        break;
    }

    return result;
}

nb_boolean
NBRE_PrimitiveCollisionObject2d::HitTest(NBRE_ICollisionPrimitive2d* primitive)
{
    return mPrimitive->HitTest(primitive);
}
