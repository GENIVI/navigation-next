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
#include "nbreuniformgridspatialpartition2d.h"
#include "nbrecollisionaabb2d.h"

NBRE_UniformGridSpatialPartition2d::NBRE_UniformGridSpatialPartition2d(const NBRE_AxisAlignedBox2d& aabb, const NBRE_Vector2i& cellCounts)
:mAABB(aabb)
,mCellCounts(cellCounts)
{
    nsl_assert(!aabb.IsNull() && cellCounts.x > 0 && cellCounts.y > 0);
    int32 count = cellCounts.x * cellCounts.y;
    for (int32 i = 0; i < count; ++i)
    {
        mCells.push_back(NULL);
    }
    NBRE_Vector2d size = mAABB.GetSize();
    mScale.x = mCellCounts.x / size.x;
    mScale.y = mCellCounts.y / size.y;
}

NBRE_UniformGridSpatialPartition2d::~NBRE_UniformGridSpatialPartition2d()
{
    for (uint32 i = 0; i < mCells.size(); ++i)
    {
        NBRE_DELETE mCells[i];
    }
    for (uint32 i = 0; i < mObjects.size(); ++i)
    {
        NBRE_DELETE mObjects[i];
    }
}

nb_boolean
NBRE_UniformGridSpatialPartition2d::QueryPotentialHitObjects(NBRE_ICollisionPrimitive2d* primitive, uint32 mask, NBRE_CollisionObject2dList& results)
{
    typedef NBRE_Set<NBRE_CollisionObject2d*> CollisionObjectSet;
    nb_boolean result = FALSE;
    CollisionObjectSet cls;
    NBRE_CollisionPrimitiveType type = primitive->GetType();

    switch (type)
    {
    case NBRE_CPT_AABB2:
    default:
        {
            NBRE_AxisAlignedBox2i cellBounds;
            WorldToCell(primitive->GetAABB(), cellBounds);
            
            for (int32 j = cellBounds.minExtend.y; j <= cellBounds.maxExtend.y; ++j)
            {
                for (int32 i = cellBounds.minExtend.x; i <= cellBounds.maxExtend.x; ++i)
                {
                    int32 idx = j * mCellCounts.x + i;
                    Cell* cell = mCells[idx];

                    if (cell)
                    {   
                        for (uint32 k = 0; k < cell->objects.size(); ++k)
                        {
                            NBRE_CollisionObject2d* cellObj = cell->objects[k];
                            if (cellObj->GetMask() & mask)
                            {
                                cls.insert(cell->objects[k]);
                            }
                        }
                    }
                }
            }
        }
    }

    if (cls.size() > 0)
    {
        for (CollisionObjectSet::iterator it = cls.begin(); it != cls.end(); ++it)
        {
            results.push_back(*it);
        }
        result = TRUE;
    }
    return result;
}

void
NBRE_UniformGridSpatialPartition2d::AddCollisionObject(NBRE_CollisionObject2d* obj)
{
    if (mObjectCells.find(obj) != mObjectCells.end())
    {
        return;
    }

    mObjects.push_back(obj);

    NBRE_AxisAlignedBox2i cellBounds;
    WorldToCell(obj->GetAABB(), cellBounds);
    if (cellBounds.IsNull())
    {
        return;
    }

    CellList objCells;
    for (int32 j = cellBounds.minExtend.y; j <= cellBounds.maxExtend.y; ++j)
    {
        for (int32 i = cellBounds.minExtend.x; i <= cellBounds.maxExtend.x; ++i)
        {
            int32 idx = j * mCellCounts.x + i;

            if (mCells[idx] == NULL)
            {
                mCells[idx] = NBRE_NEW Cell;
            }

            Cell* cell = mCells[idx];
            cell->objects.push_back(obj);
            objCells.push_back(cell);
        }
    }
    mObjectCells[obj] = objCells;
}

void
NBRE_UniformGridSpatialPartition2d::RemoveCollisionObject(NBRE_CollisionObject2d* obj)
{
    ObjectCellMap::iterator it = mObjectCells.find(obj);
    if (it == mObjectCells.end())
    {
        return;
    }

    for (uint32 i = 0; i < mObjects.size(); ++i)
    {
        if (mObjects[i] == obj)
        {
            mObjects.erase(mObjects.begin() + i);
            break;
        }
    }

    CellList& objCells = it->second;
    for (uint32 i = 0; i < objCells.size(); ++i)
    {
        Cell* cell = objCells[i];
        for (uint32 j = 0; j < cell->objects.size(); ++j)
        {
            if (cell->objects[j] == obj)
            {
                cell->objects.erase(cell->objects.begin() + j);
                break;
            }
        }
    }
    mObjectCells.erase(it);
}

void 
NBRE_UniformGridSpatialPartition2d::Clear()
{
    mObjectCells.clear();
    for (uint32 i = 0; i < mCells.size(); ++i)
    {
        NBRE_DELETE mCells[i];
    }
    mCells.clear();
    for (uint32 i = 0; i < mObjects.size(); ++i)
    {
        NBRE_DELETE mObjects[i];
    }
    mObjects.clear();
}

void
NBRE_UniformGridSpatialPartition2d::WorldToCell(const NBRE_Vector2d& worldPosition, NBRE_Vector2i& cellPosition)
{
    cellPosition.x = (int32)((worldPosition.x - mAABB.minExtend.x) * mScale.x);
    cellPosition.y = (int32)((worldPosition.y - mAABB.minExtend.y) * mScale.y);
}

void
NBRE_UniformGridSpatialPartition2d::WorldToCell(const NBRE_AxisAlignedBox2d& worldAABB, NBRE_AxisAlignedBox2i& cellAABB)
{
    WorldToCell(worldAABB.minExtend, cellAABB.minExtend);
    WorldToCell(worldAABB.maxExtend, cellAABB.maxExtend);
    
    // if out of bound, limit to boundary cells
    if (cellAABB.minExtend.x >= mCellCounts.x)
    {
        cellAABB.minExtend.x = cellAABB.maxExtend.x = mCellCounts.x - 1;
    }
    else if (cellAABB.maxExtend.x < 0)
    {
        cellAABB.minExtend.x = cellAABB.maxExtend.x = 0;
    }
    else 
    {
        if (cellAABB.minExtend.x < 0)
        {
            cellAABB.minExtend.x = 0;
        }
        if (cellAABB.maxExtend.x >= mCellCounts.x)
        {
            cellAABB.maxExtend.x = mCellCounts.x - 1;
        }
    }

    if (cellAABB.minExtend.y >= mCellCounts.y)
    {
        cellAABB.minExtend.y = cellAABB.maxExtend.y = mCellCounts.y - 1;
    }
    else if (cellAABB.maxExtend.y < 0)
    {
        cellAABB.minExtend.y = cellAABB.maxExtend.y = 0;
    }
    else 
    {
        if (cellAABB.minExtend.y < 0)
        {
            cellAABB.minExtend.y = 0;
        }
        if (cellAABB.maxExtend.y >= mCellCounts.y)
        {
            cellAABB.maxExtend.y = mCellCounts.y - 1;
        }
    }
}
