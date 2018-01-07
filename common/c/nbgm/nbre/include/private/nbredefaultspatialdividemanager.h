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

    @file nbredefaultspatialdividemanager.h
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
#ifndef _NBRE_SPATIAL_DIVIDE_MANAGER_H_
#define _NBRE_SPATIAL_DIVIDE_MANAGER_H_
#include "nbretypes.h"
#include "nbrecommon.h"
#include "nbrezone.h"
#include "nbrefrustum3.h"
#include "nbreaxisalignedbox3.h"
#include "nbrespatialdividestrategy.h"

typedef NBRE_Vector3<int32> NBRE_Point3i;
typedef NBRE_Vector3<int32> NBRE_Vector3i;

/** \addtogroup NBRE_Scene
*  @{
*/


/** this class is used to divide spatial object and find spatial object quickly.
 */
typedef NBRE_List<NBRE_BinaryZone*> NBRE_BinaryZoneList;

class NBRE_DefaultSpatialDivideManager: public NBRE_ISpatialDivideStrategy
{
    typedef NBRE_Map<NBRE_ISpatialObject*, NBRE_BinaryZoneList> ObjectZoneMap;
    typedef NBRE_Set<NBRE_ISpatialObject*> SpatialObjectSet;


public:
    NBRE_DefaultSpatialDivideManager(float zoneSize);
    ~NBRE_DefaultSpatialDivideManager();

public:
    NBRE_BinaryZone* RootZone();

    ///Derive From NBRE_IVisitable
    virtual void AddSpatialObject(NBRE_ISpatialObject* obj);
    virtual void RemoveSpatialObject(NBRE_ISpatialObject* obj);
    virtual void UpdateSpatialObject(NBRE_ISpatialObject* obj);
    virtual void WalkAllSpatialObject(NBRE_Visitor& visitor);
    virtual void WalkHitTestSpatialObject(NBRE_ISpaceHitTest* hittest, NBRE_Visitor& visitor);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_DefaultSpatialDivideManager);

private:
    void AddSpatialObject(NBRE_BinaryZone* zone, const NBRE_AxisAlignedBox3i& zoneBox, NBRE_ISpatialObject* obj, const NBRE_AxisAlignedBox3i& objBox, int32 deep);
    void AddZone(NBRE_BinaryZone* zone, const NBRE_AxisAlignedBox3i& zoneBox, NBRE_BinaryZone* subZone, const NBRE_AxisAlignedBox3i& subZoneBox);
    void WalkHitTestSpatialObject(NBRE_BinaryZone* zone, const NBRE_Frustum3f& frustum, SpatialObjectSet& spatialSet);

    void WalkHitTestSpatialObject(NBRE_BinaryZone* zone, NBRE_ISpaceHitTest* hittest, SpatialObjectSet& spatialSet);

    void RefreshSpatialObject(NBRE_BinaryZone* zone, const NBRE_AxisAlignedBox3i& zoneBox);

    void RefreshObjectZoneMap(NBRE_BinaryZone* zone);


private:

    const float mZoneSize;
    NBRE_BinaryZone* mRootZone;
    NBRE_AxisAlignedBox3i mRootZoneBox;
    ObjectZoneMap mObjZoneMap;
    int32 mMaxDeep;
};

/*! @} */

#endif
