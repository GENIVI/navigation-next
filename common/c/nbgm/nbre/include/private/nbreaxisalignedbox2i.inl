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

inline NBRE_AxisAlignedBox2<int32>::NBRE_AxisAlignedBox2()
{
    SetNull();
}

inline
NBRE_AxisAlignedBox2<int32>::NBRE_AxisAlignedBox2(int32 minX, int32 minY, int32 maxX, int32 maxY)
:minExtend(minX, minY)
,maxExtend(maxX, maxY)
{
}

inline
NBRE_AxisAlignedBox2<int32>::NBRE_AxisAlignedBox2(const NBRE_Vector2<int32>& minExtend, const NBRE_Vector2<int32>& maxExtend)
:minExtend(minExtend)
,maxExtend(maxExtend)
{
}

inline
NBRE_AxisAlignedBox2<int32>&
NBRE_AxisAlignedBox2<int32>::operator = (const NBRE_AxisAlignedBox2& box)
{
    if(this != &box)
    {
        this->minExtend = box.minExtend;
        this->maxExtend = box.maxExtend;
    }
    return *this;
}

inline
NBRE_AxisAlignedBox2<int32>::NBRE_AxisAlignedBox2(const NBRE_AxisAlignedBox2<int32>& box)
:minExtend(box.minExtend)
,maxExtend(box.maxExtend)
{
}

inline
NBRE_AxisAlignedBox2<int32>::~NBRE_AxisAlignedBox2()
{
}

inline void 
NBRE_AxisAlignedBox2<int32>::Merge(int32 x, int32 y)
{
    if (IsNull())
    {
        minExtend.x = maxExtend.x = x;
        minExtend.y = maxExtend.y = y;
    }
    else
    {
        if (x < minExtend.x)
        {
            minExtend.x = x;
        }
        if (x > maxExtend.x)
        {
            maxExtend.x = x;
        }

        if (y < minExtend.y)
        {
            minExtend.y = y;
        }
        if (y > maxExtend.y)
        {
            maxExtend.y = y;
        }
    }
}

inline void 
NBRE_AxisAlignedBox2<int32>::Merge(const NBRE_Vector2<int32>& point)
{
    Merge(point.x, point.y);
}

inline void 
NBRE_AxisAlignedBox2<int32>::Merge(const NBRE_AxisAlignedBox2<int32>& box)
{
    Merge(box.minExtend);
    Merge(box.maxExtend);
}

inline NBRE_Vector2<int32>
NBRE_AxisAlignedBox2<int32>::GetSize() const
{
    return maxExtend - minExtend;
}

inline NBRE_Vector2<int32>
NBRE_AxisAlignedBox2<int32>::GetCenter() const
{
    return (maxExtend + minExtend) / 2;
}

inline NBRE_Vector2<int32> 
NBRE_AxisAlignedBox2<int32>::GetClosestPointTo(const NBRE_Vector2<int32>& point) const
{
    NBRE_Vector2<int32> closestPoint(point);

    if (closestPoint.x < minExtend.x)
    {
        closestPoint.x = minExtend.x;
    }
    else if (closestPoint.x > maxExtend.x)
    {
        closestPoint.x = maxExtend.x;
    }

    if (closestPoint.y < minExtend.y)
    {
        closestPoint.y = minExtend.y;
    }
    else if (closestPoint.y > maxExtend.y)
    {
        closestPoint.y = maxExtend.y;
    }

    return closestPoint;
}

inline void 
NBRE_AxisAlignedBox2<int32>::GetCorners(NBRE_Vector2<int32>* p) const
{
    p[0].x = minExtend.x; p[0].y = minExtend.y;
    p[1].x = maxExtend.x; p[1].y = minExtend.y;
    p[2].x = maxExtend.x; p[2].y = maxExtend.y;
    p[3].x = minExtend.x; p[3].y = maxExtend.y;
}

inline nb_boolean 
NBRE_AxisAlignedBox2<int32>::IsNull() const
{
    return !(minExtend.x <= maxExtend.x 
          && minExtend.y <= maxExtend.y);
}

inline void
NBRE_AxisAlignedBox2<int32>::SetNull()
{
    minExtend.x = minExtend.y = 1;
    maxExtend.x = maxExtend.y = -1;
}
