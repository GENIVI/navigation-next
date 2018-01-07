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
#include "nbrecollisiondetector2d.h"
#include "nbrecollisionaabb2d.h"
#include "nbreprimitivecollisionobject2d.h"
#include "nbrecompositecollisionobject2d.h"
#include "nbrecollisionpolygon2d.h"
#include "nbrevertexindexdata.h"

nb_boolean
NBRE_CollisionDetector2d::HitTest(NBRE_ISpatialPartition2d* spatialPartition, NBRE_ICollisionPrimitive2d* primitive, uint32 mask, NBRE_CollisionObject2dList& collideObjects)
{
    nb_boolean result = FALSE;
    NBRE_CollisionObject2dList potentialHitObjects;
    spatialPartition->QueryPotentialHitObjects(primitive, mask, potentialHitObjects);

    if (potentialHitObjects.size() == 0)
    {
        return result;
    }

    for (uint32 i = 0; i < potentialHitObjects.size(); ++i)
    {
        NBRE_CollisionObject2d* obj = potentialHitObjects[i];
        if (obj->HitTest(primitive))
        {
            collideObjects.push_back(obj);
            result = TRUE;
        }
    }
    return result;
}

nb_boolean
NBRE_CollisionDetector2d::HitTest(NBRE_ISpatialPartition2d* spatialPartition, NBRE_CollisionObject2d* obj, uint32 mask, NBRE_Vector<NBRE_CollisionObject2d*>& collideObjects)
{
    nb_boolean result = FALSE;
    NBRE_CollisionObject2dList potentialHitObjects;
    NBRE_CollisionAABB2d primitive(obj->GetAABB());
    spatialPartition->QueryPotentialHitObjects(&primitive, mask, potentialHitObjects);

    if (potentialHitObjects.size() == 0)
    {
        return result;
    }

    for (uint32 i = 0; i < potentialHitObjects.size(); ++i)
    {
        NBRE_CollisionObject2d* o = potentialHitObjects[i];
        if (o != obj && o->HitTest(obj))
        {
            collideObjects.push_back(o);
            result = TRUE;
        }
    }
    return result;
}

nb_boolean
NBRE_CollisionDetector2d::HitTest(NBRE_ISpatialPartition2d* spatialPartition, NBRE_CollisionObject2d* obj, uint32 mask)
{
    NBRE_CollisionObject2dList potentialHitObjects;
    NBRE_CollisionAABB2d primitive(obj->GetAABB());
    spatialPartition->QueryPotentialHitObjects(&primitive, mask, potentialHitObjects);

    if (potentialHitObjects.size() == 0)
    {
        return FALSE;
    }

    for (uint32 i = 0; i < potentialHitObjects.size(); ++i)
    {
        NBRE_CollisionObject2d* o = potentialHitObjects[i];
        if (o != obj && o->HitTest(obj))
        {
            return TRUE;
        }
    }
    return FALSE;
}
