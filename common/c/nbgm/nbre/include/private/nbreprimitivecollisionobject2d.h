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

    @file nbreprimitivecollisionobject2d.h
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
#ifndef _NBRE_PRIMITIVE_COLLISION_OBJECT_2D_H_
#define _NBRE_PRIMITIVE_COLLISION_OBJECT_2D_H_
#include "nbretypes.h"
#include "nbrecollisiontypes.h"
#include "nbreaxisalignedbox2.h"
#include "nbrecollisionobject2d.h"
#include "nbreicollisionprimitive.h"

/*! \addtogroup NBRE_Scene
*  @{
*/
/** Primitive collision object which only contains one primitive.
The function of this class is same as NBRE_CompositeCollisionObject2d, 
but NBRE_PrimitiveCollisionObject2d run a little faster and takes less memory.
 */
class NBRE_PrimitiveCollisionObject2d:
    public NBRE_CollisionObject2d
{
public:
    NBRE_PrimitiveCollisionObject2d(void* owner, NBRE_ICollisionPrimitive2d* primitive);
    virtual ~NBRE_PrimitiveCollisionObject2d();

    NBRE_ICollisionPrimitive2d* GetPrimitive() { return mPrimitive; }

    virtual NBRE_CollisionObjectType GetType() { return NBRE_COT_SINGLE; }
    virtual nb_boolean HitTest(NBRE_CollisionObject2d* obj);
    virtual nb_boolean HitTest(NBRE_ICollisionPrimitive2d* primitive);
    virtual const NBRE_AxisAlignedBox2d& GetAABB() { return mPrimitive->GetAABB(); }

private:
    NBRE_ICollisionPrimitive2d* mPrimitive;
};

/*! @} */
#endif
