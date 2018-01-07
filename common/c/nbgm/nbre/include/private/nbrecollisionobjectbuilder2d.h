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

    @file nbrecollisionobjectbuilder2d.h
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
#ifndef _NBRE_COLLISION_OBJECT_BUILDER_2D_H_
#define _NBRE_COLLISION_OBJECT_BUILDER_2D_H_
#include "nbretypes.h"
#include "nbreicollisionprimitive.h"
#include "nbreaxisalignedbox2.h"
#include "nbrecollisionobject2d.h"
#include "nbreispatialpartition2d.h"
#include "nbrepolyline2.h"
#include "nbreiboundtree2d.h"

class NBRE_VertexData;
class NBRE_IndexData;
/*! \addtogroup NBRE_Scene
*  @{
*/
/** Collision object builder is an utility which simplifies the creation of collision objects.
 */
class NBRE_CollisionObjectBuilder2d
{
public:
    static NBRE_CollisionObject2d* CreateAABB2d(void* owner, const NBRE_AxisAlignedBox2d& aabb, uint32 mask);
    static NBRE_CollisionObject2d* CreatePolygon2d(void* owner, const NBRE_Vector2d* vertices, uint32 vertexCount, uint32 mask);
    static NBRE_CollisionObject2d* CreatePolyline2d(void* owner, const NBRE_Polyline2d& polyline, double width, NBRE_IBoundTree2d* boundTree, uint32 mask);
    static NBRE_CollisionObject2d* CreatePolyline2d(void* owner, const NBRE_Polyline2d& polyline, NBRE_IBoundTree2d* boundTree, uint32 mask);
    static NBRE_CollisionObject2d* CreateTriangleMesh2d(void* owner, NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_IBoundTree2d* boundTree, uint32 mask);
    static NBRE_CollisionObject2d* CreateCircle2d(void* owner, const NBRE_Vector2d& center, double radius, uint32 mask);
};

/*! @} */
#endif
