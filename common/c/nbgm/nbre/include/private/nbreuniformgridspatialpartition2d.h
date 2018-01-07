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

    @file nbreuniformgridspatialpartition2d.h
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
#ifndef _NBRE_UNIFORM_GRID_SPATIAL_PARTITION_2D_H_
#define _NBRE_UNIFORM_GRID_SPATIAL_PARTITION_2D_H_
#include "nbretypes.h"
#include "nbreicollisionprimitive.h"
#include "nbreaxisalignedbox2.h"
#include "nbrecollisionobject2d.h"
#include "nbreispatialpartition2d.h"

/*! \addtogroup NBRE_Scene
*  @{
*/
/** Uniform grid spatial partition divides limited space into cells with the same size. 
1. Query time complexity is O(cellDensity).
2. Register/Unregister operation is fast
3. A object may exists in multiple cells
4. The cell size is very important. Too large cell size will increase lookup time. Too small size will increase memory cost and Register/Unregister time.
5. It's not suitable for the scene with very different object sizes.
6. The max scene size cannot dynamically change.
 */
class NBRE_UniformGridSpatialPartition2d:
    public NBRE_ISpatialPartition2d
{
    struct Cell
    {
        NBRE_Vector<NBRE_CollisionObject2d*> objects;
    };
    typedef NBRE_Vector<Cell*> CellList;
    typedef NBRE_Map<NBRE_CollisionObject2d*, CellList> ObjectCellMap;

public:
    NBRE_UniformGridSpatialPartition2d(const NBRE_AxisAlignedBox2d& aabb, const NBRE_Vector2i& cellCounts);
    virtual ~NBRE_UniformGridSpatialPartition2d();
    virtual nb_boolean QueryPotentialHitObjects(NBRE_ICollisionPrimitive2d* primitive, uint32 mask, NBRE_CollisionObject2dList& results);
    void AddCollisionObject(NBRE_CollisionObject2d* obj);
    void RemoveCollisionObject(NBRE_CollisionObject2d* obj);
    void Clear();

private:
    void WorldToCell(const NBRE_Vector2d& worldPosition, NBRE_Vector2i& cellPosition);
    void WorldToCell(const NBRE_AxisAlignedBox2d& worldAABB, NBRE_AxisAlignedBox2i& cellAABB);

private:
    NBRE_AxisAlignedBox2d mAABB;
    NBRE_Vector2i mCellCounts;
    NBRE_Vector2d mScale;
    CellList mCells;
    ObjectCellMap mObjectCells;
    NBRE_CollisionObject2dList mObjects;
};

/*! @} */
#endif
