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
#include "nbredefaultspatialdividemanager.h"
#include "nbreintersection.h"
#include "nbrespatialobject.h"

typedef NBRE_Intersection<int32> NBRE_Intersectioni;


NBRE_DefaultSpatialDivideManager::NBRE_DefaultSpatialDivideManager(float zoneSize):
    mZoneSize(zoneSize),
    mRootZone(NULL),
    mMaxDeep(4)
{
}

NBRE_DefaultSpatialDivideManager::~NBRE_DefaultSpatialDivideManager()
{
    NBRE_DELETE mRootZone;
}

static NBRE_AxisAlignedBox3f ConverFloatBox(const NBRE_AxisAlignedBox3i& box, float zoneSize)
{
    NBRE_Point3f minP(box.minExtend.x*zoneSize, box.minExtend.y*zoneSize, box.minExtend.z*zoneSize);
    NBRE_Point3f maxP(box.maxExtend.x*zoneSize, box.maxExtend.y*zoneSize, box.maxExtend.z*zoneSize);

    return NBRE_AxisAlignedBox3f(minP, maxP);
}

static int32 DivideTwo(const int32 number, nb_boolean up)
{
    int32 ret = number;
    ret >>= 1;

    if(up)
    {
        if(number > (ret<<1))
        {
            ++ret;
        }
    }
    else
    {
        if(number < (ret<<1))
        {
            --ret;
        }
    }

    return ret;
}

static void CalcTwoPower(int32& start, int32& end)
{
    int32 exp = 0;
    while((end - start) > 2)
    {
        ++exp;
        start = DivideTwo(start, FALSE);
        end = DivideTwo(end, TRUE);
    }
    start <<= exp;
    end <<= exp;
}

static int32 Round(double number, double zoneSize, nb_boolean up)
{
    number /= zoneSize;
    int32 intValue = (int32)number;

    if(up)
    {
        if(number > intValue)
        {
            ++intValue;
        }
    }
    else
    {
        if(number < intValue)
        {
            --intValue;
        }
    }

    return intValue;
}

static void CalcRootZoneBox(NBRE_AxisAlignedBox3i& box)
{
    CalcTwoPower(box.minExtend.x, box.maxExtend.x);
    CalcTwoPower(box.minExtend.y, box.maxExtend.y);
    CalcTwoPower(box.minExtend.z, box.maxExtend.z);
}

static NBRE_AxisAlignedBox3i HalfBox(const NBRE_AxisAlignedBox3i& box, int32 i)
{
    NBRE_Point3i midPt = (box.minExtend+box.maxExtend)/2;
    switch(i)
    {
    case 0:
        return NBRE_AxisAlignedBox3i(box.minExtend, NBRE_Point3i(midPt.x, box.maxExtend.y, box.maxExtend.z));
    case 1:
        return NBRE_AxisAlignedBox3i(NBRE_Point3i(midPt.x, box.minExtend.y, box.minExtend.z), box.maxExtend);
    case 2:
        return NBRE_AxisAlignedBox3i(box.minExtend, NBRE_Point3i(box.maxExtend.x, midPt.y, box.maxExtend.z));
    case 3:
        return NBRE_AxisAlignedBox3i(NBRE_Point3i(box.minExtend.x, midPt.y, box.minExtend.z), box.maxExtend);
    case 4:
        return NBRE_AxisAlignedBox3i(box.minExtend, NBRE_Point3i(box.maxExtend.x, box.maxExtend.y, midPt.z));
    case 5:
        return NBRE_AxisAlignedBox3i(NBRE_Point3i(box.minExtend.x, box.minExtend.y, midPt.z), box.maxExtend);
    }
    return NBRE_AxisAlignedBox3i();
}

bool operator == (const NBRE_AxisAlignedBox3i& box1, const NBRE_AxisAlignedBox3i& box2)
{
    return (box1.minExtend == box2.minExtend) && (box1.maxExtend == box2.maxExtend);
}

bool operator != (const NBRE_AxisAlignedBox3i& box1, const NBRE_AxisAlignedBox3i& box2)
{
    return !(box1 == box2) ;
}

static nb_boolean IsIntersect(const NBRE_AxisAlignedBox3i& box1, const NBRE_AxisAlignedBox3i& box2)
{
    return ( (box1.minExtend.x >= box2.maxExtend.x) || (box1.minExtend.y >= box2.maxExtend.y) || (box1.minExtend.z >= box2.maxExtend.z)
          || (box1.maxExtend.x <= box2.minExtend.x) || (box1.maxExtend.y <= box2.minExtend.y) || (box1.maxExtend.z <= box2.minExtend.z) )
        ? FALSE : TRUE;
}

static nb_boolean IsContain(const NBRE_AxisAlignedBox3i& box1, const NBRE_AxisAlignedBox3i& box2)
{
    return ( (box1.minExtend.x <= box2.minExtend.x) && (box1.minExtend.y <= box2.minExtend.y) && (box1.minExtend.z <= box2.minExtend.z)
          && (box1.maxExtend.x >= box2.maxExtend.x) && (box1.maxExtend.y >= box2.maxExtend.y) && (box1.maxExtend.z >= box2.maxExtend.z) )
        ? TRUE : FALSE;
}

static NBRE_AxisAlignedBox3i Intersect(const NBRE_AxisAlignedBox3i& box1, const NBRE_AxisAlignedBox3i& box2)
{
    return NBRE_AxisAlignedBox3i(
        NBRE_Math::Max(box1.minExtend.x, box2.minExtend.x), NBRE_Math::Max(box1.minExtend.y, box2.minExtend.y), NBRE_Math::Max(box1.minExtend.z, box2.minExtend.z),
        NBRE_Math::Min(box1.maxExtend.x, box2.maxExtend.x), NBRE_Math::Min(box1.maxExtend.y, box2.maxExtend.y), NBRE_Math::Min(box1.maxExtend.z, box2.maxExtend.z)
        );
}

static NBRE_AxisAlignedBox3i ConverIntBox(const NBRE_AxisAlignedBox3d& box, float zoneSize)
{    
    if(box.IsNull())
    {
        return NBRE_AxisAlignedBox3i();
    }

    NBRE_Point3i minP(Round(box.minExtend.x, zoneSize, FALSE), Round(box.minExtend.y, zoneSize, FALSE), Round(box.minExtend.z, zoneSize, FALSE));
    NBRE_Point3i maxP(Round(box.maxExtend.x, zoneSize, TRUE), Round(box.maxExtend.y, zoneSize, TRUE), Round(box.maxExtend.z, zoneSize, TRUE));

    NBRE_AxisAlignedBox3i objBox(minP, maxP);

    NBRE_Vector3i size = objBox.GetSize();
    if(size.x == 0)
    {
        objBox.maxExtend.x = objBox.minExtend.x+1;
    }
    if(size.y == 0)
    {
        objBox.maxExtend.y = objBox.minExtend.y+1;
    }
    if(size.z == 0)
    {
        objBox.maxExtend.z = objBox.minExtend.z+1;
    }

    return objBox;
}

void NBRE_DefaultSpatialDivideManager::AddSpatialObject(NBRE_ISpatialObject* obj)
{
    if(obj == NULL)
    {
        return;
    }

    const NBRE_AxisAlignedBox3i& objBox = ConverIntBox(obj->AxisAlignedBox(), mZoneSize);

    if(objBox.IsNull())
    {
        return;
    }

    if(mRootZone)
    {
        if(!IsContain(mRootZoneBox, objBox))
        {
            NBRE_AxisAlignedBox3i oldRootZoneBox = mRootZoneBox;

            mRootZoneBox.Merge(objBox);
            CalcRootZoneBox(mRootZoneBox);

            NBRE_BinaryZone* oldRootZone = mRootZone;

            mRootZone = NBRE_NEW NBRE_BinaryZone();
            mRootZone->SetBoundingbox(ConverFloatBox(mRootZoneBox, mZoneSize));

            AddZone(mRootZone, mRootZoneBox, oldRootZone, oldRootZoneBox);
        }
    }
    else
    {
        mRootZoneBox = objBox;
        CalcRootZoneBox(mRootZoneBox);

        mRootZone = NBRE_NEW NBRE_BinaryZone();
        mRootZone->SetBoundingbox(ConverFloatBox(mRootZoneBox, mZoneSize));
    }

    AddSpatialObject(mRootZone, mRootZoneBox, obj, objBox, mMaxDeep);
}

void NBRE_DefaultSpatialDivideManager::RemoveSpatialObject(NBRE_ISpatialObject* obj)
{
    if(obj == NULL)
    {
        return;
    }

    ObjectZoneMap::iterator pZoneList = mObjZoneMap.find(obj);
    if(pZoneList == mObjZoneMap.end())
    {
        return;
    }

    NBRE_BinaryZoneList& zoneList = pZoneList->second;
    for(NBRE_BinaryZoneList::iterator pZone = zoneList.begin(); pZone != zoneList.end(); ++pZone)
    {
        (*pZone)->RemoveSpatialObject(obj);
    }

    mObjZoneMap.erase(pZoneList);
}

void NBRE_DefaultSpatialDivideManager::UpdateSpatialObject(NBRE_ISpatialObject* obj)
{
    if(obj == NULL)
    {
        return;
    }

    RemoveSpatialObject(obj);
    AddSpatialObject(obj);
}

void NBRE_DefaultSpatialDivideManager::WalkHitTestSpatialObject(NBRE_ISpaceHitTest* hittest, NBRE_Visitor& visitor)
{
    SpatialObjectSet objSet;
    WalkHitTestSpatialObject(mRootZone, hittest, objSet);

    for(NBRE_SpatialObjectList::const_iterator pObj = objSet.begin(); pObj != objSet.end(); ++pObj)
    {
        (*pObj)->Accept(visitor);
    }
}

void NBRE_DefaultSpatialDivideManager::WalkAllSpatialObject(NBRE_Visitor& visitor)
{
    for(ObjectZoneMap::iterator pObj = mObjZoneMap.begin(); pObj != mObjZoneMap.end(); ++pObj)
    {
        pObj->first->Accept(visitor);
    }
}

void NBRE_DefaultSpatialDivideManager::AddSpatialObject(NBRE_BinaryZone* zone, 
                                                  const NBRE_AxisAlignedBox3i& zoneBox, 
                                                  NBRE_ISpatialObject* obj,
                                                  const NBRE_AxisAlignedBox3i& objBox,
                                                  int32 deep)
{
    mObjZoneMap[obj].push_back(zone);
    if((objBox == zoneBox) || (deep < 0))
    {
        zone->AddSpatialObject(obj);
    }
    else
    {
        zone->AddSubZoneSpatialObject(obj);
        NBRE_Vector3i size = zoneBox.GetSize();
        int32 left, right; 
        if(size.x > size.y)
        {
            if(size.x > size.z)
            {
                left = 0;
                right = 1;
            }
            else
            {
                left = 4;
                right = 5;
            }
        }
        else
        {
            if(size.y > size.z)
            {
                left = 2;
                right = 3;
            }
            else
            {
                left = 4;
                right = 5;
            }
        }

        NBRE_BinaryZone* child = NULL;

        NBRE_AxisAlignedBox3i leftZoneBox = HalfBox(zoneBox, left);
        NBRE_AxisAlignedBox3i rightZoneBox = HalfBox(zoneBox, right);
        nb_boolean leftIntersect = IsIntersect(leftZoneBox, objBox);
        nb_boolean rightIntersect = IsIntersect(rightZoneBox, objBox);

        if(leftIntersect && rightIntersect)
        {
            --deep;
        }

        if(leftIntersect)
        {
            child = zone->CreateLeftChild();
            child->SetBoundingbox(ConverFloatBox(leftZoneBox, mZoneSize));
            AddSpatialObject(child, leftZoneBox, obj, Intersect(leftZoneBox, objBox), deep);
        }

        if(rightIntersect)
        {
            child = zone->CreateRightChild();
            child->SetBoundingbox(ConverFloatBox(rightZoneBox, mZoneSize));
            AddSpatialObject(child, rightZoneBox, obj, Intersect(rightZoneBox, objBox), deep);
        }
    }
}

void NBRE_DefaultSpatialDivideManager::RefreshSpatialObject(NBRE_BinaryZone* zone, const NBRE_AxisAlignedBox3i& zoneBox)
{
    for(ObjectZoneMap::iterator pObj = mObjZoneMap.begin(); pObj != mObjZoneMap.end(); ++pObj)
    {
        pObj->second.clear();
    }

    for(ObjectZoneMap::iterator pObj = mObjZoneMap.begin(); pObj != mObjZoneMap.end(); ++pObj)
    {
        NBRE_AxisAlignedBox3i box = ConverIntBox(pObj->first->AxisAlignedBox(), mZoneSize);

        if(box.IsNull())
        {
            continue;
        }

        AddSpatialObject(zone, zoneBox, pObj->first, box, mMaxDeep);
    }
}

void NBRE_DefaultSpatialDivideManager::AddZone(NBRE_BinaryZone* zone, 
                                               const NBRE_AxisAlignedBox3i& zoneBox, 
                                               NBRE_BinaryZone* subZone, 
                                               const NBRE_AxisAlignedBox3i& subZoneBox)
{
    NBRE_Vector3i size = zoneBox.GetSize();
    int32 left, right; 
    if(size.x > size.y)
    {
        if(size.x > size.z)
        {
            left = 0;
            right = 1;
        }
        else
        {
            left = 4;
            right = 5;
        }
    }
    else
    {
        if(size.y > size.z)
        {
            left = 2;
            right = 3;
        }
        else
        {
            left = 4;
            right = 5;
        }
    }

    nb_boolean needRefresh = TRUE;

    NBRE_AxisAlignedBox3i zoneChildBox;

    zoneChildBox = HalfBox(zoneBox, left);
    if(zoneChildBox == subZoneBox)
    {
        zone->SetLeftChild(subZone);
        needRefresh = FALSE;
    }
    else if(IsContain(zoneChildBox, subZoneBox))
    {
        NBRE_BinaryZone* child = zone->CreateLeftChild();
        child->SetBoundingbox(ConverFloatBox(zoneChildBox, mZoneSize));
        AddZone(child, zoneChildBox, subZone, subZoneBox);
        needRefresh = FALSE;
    }
    else
    {
        zoneChildBox = HalfBox(zoneBox, right);
        if(zoneChildBox == subZoneBox)
        {
            zone->SetRightChild(subZone);
            needRefresh = FALSE;
        }
        else if(IsContain(zoneChildBox, subZoneBox))
        {
            NBRE_BinaryZone* child = zone->CreateRightChild();
            child->SetBoundingbox(ConverFloatBox(zoneChildBox, mZoneSize));
            AddZone(child, zoneChildBox, subZone, subZoneBox);
            needRefresh = FALSE;
        }
    }

    if(needRefresh)
    {
        RefreshSpatialObject(zone, zoneBox);
        NBRE_DELETE subZone;
    }

    RefreshObjectZoneMap(zone);
}

void NBRE_DefaultSpatialDivideManager::RefreshObjectZoneMap(NBRE_BinaryZone* zone)
{
    for(ObjectZoneMap::iterator pObj = mObjZoneMap.begin(); pObj != mObjZoneMap.end(); ++pObj)
    {
        pObj->second.push_back(zone);
        zone->AddSubZoneSpatialObject(pObj->first);
    }
}

void NBRE_DefaultSpatialDivideManager::WalkHitTestSpatialObject(NBRE_BinaryZone* zone, const NBRE_Frustum3f& frustum, SpatialObjectSet& spatialSet)
{
    if(zone)
    {
        if(NBRE_Intersectionf::HitTest(frustum, zone->AxisAlignedBox()))
        {
            const NBRE_SpatialObjectList& children = zone->SpatialObjects();
            spatialSet.insert(children.begin(), children.end());

            if(NBRE_Intersectionf::Contain(frustum, zone->AxisAlignedBox()))
            {
                const NBRE_SpatialObjectList& allChildren = zone->SubZoneSpatialObjects();
                spatialSet.insert(allChildren.begin(), allChildren.end());
                    //spatialSet.insert(children.begin(), children.end());
            }
            else
            {
                WalkHitTestSpatialObject(zone->LeftChild(), frustum, spatialSet);
                WalkHitTestSpatialObject(zone->RightChild(), frustum, spatialSet);
            }
        }
    }
}

void NBRE_DefaultSpatialDivideManager::WalkHitTestSpatialObject(NBRE_BinaryZone* zone, NBRE_ISpaceHitTest* hittest, SpatialObjectSet& spatialSet)
{
    if(zone)
    {
        if(hittest->Intersect(zone->AxisAlignedBox()))
        {
            const NBRE_SpatialObjectList& children = zone->SpatialObjects();
            spatialSet.insert(children.begin(), children.end());

            if(hittest->Contain(zone->AxisAlignedBox()))
            {
                const NBRE_SpatialObjectList& allChildren = zone->SubZoneSpatialObjects();
                spatialSet.insert(allChildren.begin(), allChildren.end());
                    //spatialSet.insert(children.begin(), children.end());
            }
            else
            {
                WalkHitTestSpatialObject(zone->LeftChild(), hittest, spatialSet);
                WalkHitTestSpatialObject(zone->RightChild(), hittest, spatialSet);
            }
        }
    }
}

NBRE_BinaryZone* NBRE_DefaultSpatialDivideManager::RootZone()
{
    return mRootZone;
}
