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

    @file nbrecollisiondetector2d.h
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
#ifndef _NBRE_COLLISION_DETECTOR_2D_H_
#define _NBRE_COLLISION_DETECTOR_2D_H_
#include "nbretypes.h"
#include "nbreicollisionprimitive.h"
#include "nbreaxisalignedbox2.h"
#include "nbrecollisionobject2d.h"
#include "nbreispatialpartition2d.h"
#include "nbrepolyline2.h"
#include "nbreiboundtree2d.h"
#include "nbrecollisionobjectbuilder2d.h"

/*! \addtogroup NBRE_Scene
*  @{
*/
/** Collision detector provides collision detection functions.
It uses spatial partition objects to query potential collision objects.(broad phase)
Then do accurate primitive test on these objects.(narrow phase)
Client can use their own process if custom logic needed.
 */
class NBRE_CollisionDetector2d
{
public:
    /*! Test if object collide with objects in spatial partition.
    @param spatialPartition, spatial partition which contains objects to test.  
    @param primitive, test object.
    @param mask, each object in spatial partition has a mask. Only the objects contains the mask bit will be included in the test.
    @param collideObjects, collided objects.
    @return is collision found
    */
    static nb_boolean HitTest(NBRE_ISpatialPartition2d* spatialPartition, NBRE_ICollisionPrimitive2d* primitive, uint32 mask, NBRE_CollisionObject2dList& collideObjects);
    
    /*! Test if object collide with objects in spatial partition.
    @param spatialPartition, spatial partition which contains objects to test.  
    @param obj, test object.
    @param mask, each object in spatial partition has a mask. Only the objects contains the mask bit will be included in the test.
    @param collideObjects, collided objects.
    @return is collision found
    */
    static nb_boolean HitTest(NBRE_ISpatialPartition2d* spatialPartition, NBRE_CollisionObject2d* obj, uint32 mask, NBRE_Vector<NBRE_CollisionObject2d*>& collideObjects);
    
    /*! Test if object collide with objects in spatial partition.
        Because this function doesn't return actually collided objects, it runs faster in most cases.
    @param spatialPartition, spatial partition which contains objects to test.  
    @param primitive, test object.
    @param mask, each object in spatial partition has a mask. Only the objects contains the mask bit will be included in the test.
    @return is collision found
    */
    static nb_boolean HitTest(NBRE_ISpatialPartition2d* spatialPartition, NBRE_CollisionObject2d* obj, uint32 mask);
};

/*! @} */
#endif
