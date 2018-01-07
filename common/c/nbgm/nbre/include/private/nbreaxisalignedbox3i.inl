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

/** \addtogroup NBRE_Core
*  @{
*/
//! Axis aligned bounding box 3D integer version
/** 
*/
class NBRE_AxisAlignedBox3i: public NBRE_AxisAlignedBox3<int32>
{
public:
    NBRE_AxisAlignedBox3i();
    NBRE_AxisAlignedBox3i(int32 minX, int32 minY, int32 minZ, int32 maxX, int32 maxY, int32 maxZ);
    NBRE_AxisAlignedBox3i(const NBRE_Vector3<int32>& minExtend, const NBRE_Vector3<int32>& maxExtend);
    NBRE_AxisAlignedBox3i(const NBRE_AxisAlignedBox3i& box);
    const NBRE_AxisAlignedBox3i& operator = (const NBRE_AxisAlignedBox3i&);
};

inline
NBRE_AxisAlignedBox3i::NBRE_AxisAlignedBox3i():NBRE_AxisAlignedBox3<int32>(1, 1, 1, 0, 0, 0)
{
}

inline
NBRE_AxisAlignedBox3i::NBRE_AxisAlignedBox3i(int32 minX, int32 minY, int32 minZ, int32 maxX, int32 maxY, int32 maxZ)
:NBRE_AxisAlignedBox3<int32>(minX, minY, minZ, maxX, maxY, maxZ)
{
}

inline
NBRE_AxisAlignedBox3i::NBRE_AxisAlignedBox3i(const NBRE_Vector3<int32>& minExtend, const NBRE_Vector3<int32>& maxExtend)
:NBRE_AxisAlignedBox3<int32>(minExtend, maxExtend)
{
}

inline
NBRE_AxisAlignedBox3i::NBRE_AxisAlignedBox3i(const NBRE_AxisAlignedBox3i& box)
:NBRE_AxisAlignedBox3<int32>(box)
{
}

inline
const NBRE_AxisAlignedBox3i& 
NBRE_AxisAlignedBox3i::operator = (const NBRE_AxisAlignedBox3i& box)
{
    if(&box != this)
    {
        minExtend = box.minExtend;
        maxExtend = box.maxExtend;
    }
    return *this;
}
/** @} */
