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
#include "palmath.h"


template<typename T> NBRE_LineClipResult 
NBRE_Clipping<T>::ClipLineByRect(const NBRE_AxisAlignedBox2<T>& rect, NBRE_Vector2<T>& np1, NBRE_Vector2<T>& np2)
{
    // Original position before clipping
    T x1 = np1.x;
    T x2 = np2.x;
    T y1 = np1.y;
    T y2 = np2.y;
    // P-Code for p1 & p2
    uint8 p1code = 0;
    uint8 p2code = 0;

    if (y1 < rect.minExtend.y)
    {
        p1code |= 8;
    }
    else if (y1 > rect.maxExtend.y)
    {
        p1code |= 4;  
    }

    if (x1 < rect.minExtend.x)
    {
        p1code |= 1;  
    }
    else if (x1 > rect.maxExtend.x)
    {
        p1code |= 2;
    }

    if (y2 < rect.minExtend.y)
    {
        p2code |= 8;
    }
    else if (y2 > rect.maxExtend.y)
    {
        p2code |= 4;  
    }

    if (x2 < rect.minExtend.x)
    {
        p2code |= 1;
    }
    else if (x2 > rect.maxExtend.x)
    {
        p2code |= 2;
    }

    // Both end points in the same side
    if ((p1code & p2code))  
        return NBRE_LCR_OUTSIDE;  

    // Both end points in center area
    if (p1code == 0 && p2code == 0)  
    {  
        np1.x = x1;  
        np1.y = y1;  
        np2.x = x2;
        np2.y = y2;        
        return NBRE_LCR_INSIDE;
    }  

    // Get new position of p1 (np1.x, np1.y)
    switch(p1code)  
    {  
    case 0: // C   
        {  
            np1.x = x1;  
            np1.y = y1;  
        } 
        break;  
    case 8: // N   
        {  
            np1.y = rect.minExtend.y;  
            np1.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;  
    case 4: // S   
        {  
            np1.y = rect.maxExtend.y;  
            np1.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;
    case 1: // W   
        {  
            np1.x = rect.minExtend.x;  
            np1.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 2: // E   
        {  
            np1.x = rect.maxExtend.x;  
            np1.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 9: // NW   
        {  
            // Assume intersect with yMin
            np1.y = rect.minExtend.y;  
            np1.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            // If wrong, intersect with xMin
            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)
            {  
                np1.x = rect.minExtend.x;  
                np1.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 10: // NE   
        {  
            np1.y = rect.minExtend.y;  
            np1.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)  
            {  
                np1.x = rect.maxExtend.x;  
                np1.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 6: // SE   
        {  
            np1.y = rect.maxExtend.y;  
            np1.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)  
            {  
                np1.x = rect.maxExtend.x;  
                np1.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 5: // SW   
        {  
            np1.y = rect.maxExtend.y;  
            np1.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x)  
            {  
                np1.x = rect.minExtend.x;  
                np1.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    }  

    // Get new position of p2 (np2.x, np2.y), same as p1
    switch(p2code)  
    {  
    case 0: // C   
        {  
            np2.x = x2;  
            np2.y = y2;  
        } 
        break;  
    case 8: // N
        {  
            np2.y = rect.minExtend.y;  
            np2.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;  
    case 4: // S   
        {  
            np2.y = rect.maxExtend.y;  
            np2.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  
        } 
        break;  
    case 1: // W   
        {  
            np2.x = rect.minExtend.x;  
            np2.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 2: // E   
        {  
            np2.x = rect.maxExtend.x;  
            np2.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
        } 
        break;  
    case 9: // NW   
        {
            np2.y = rect.minExtend.y;  
            np2.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)
            {  
                np2.x = rect.minExtend.x;  
                np2.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 10: // NE   
        {  
            np2.y = rect.minExtend.y;  
            np2.x = x1 + (rect.minExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)  
            {  
                np2.x = rect.maxExtend.x;  
                np2.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 6: // SE   
        {  
            np2.y = rect.maxExtend.y;  
            np2.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)  
            {  
                np2.x = rect.maxExtend.x;  
                np2.y = y1 + (rect.maxExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    case 5: // SW   
        {  
            np2.y = rect.maxExtend.y;  
            np2.x = x1 + (rect.maxExtend.y - y1) * (x2 - x1) / (y2 - y1);  

            if (np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x)  
            {  
                np2.x = rect.minExtend.x;  
                np2.y = y1 + (rect.minExtend.x - x1) * (y2 - y1) / (x2 - x1);  
            }  
        } 
        break;  
    }  

    if (np1.x < rect.minExtend.x || np1.x > rect.maxExtend.x ||  
        np1.y < rect.minExtend.y || np1.y > rect.maxExtend.y ||  
        np2.x < rect.minExtend.x || np2.x > rect.maxExtend.x ||  
        np2.y < rect.minExtend.y || np2.y > rect.maxExtend.y)  
    {  
        return NBRE_LCR_OUTSIDE;
    }

    if (p1code == 0)
    {
        return NBRE_LCR_CLIP_END;
    }
    else if (p2code == 0)
    {
        return NBRE_LCR_CLIP_START;
    }
    else
    {
        return NBRE_LCR_CLIP_BOTH;
    }
}

template<typename T> typename NBRE_Clipping<T>::Polyline2List
NBRE_Clipping<T>::ClipByRect(const NBRE_AxisAlignedBox2<T>& rect, const NBRE_Polyline2<T>& screenPolyline)
{
    Polyline2List result;
    NBRE_Vector< NBRE_Vector2<T> > pl;
    for (uint32 i = 0; i < screenPolyline.VertexCount() - 1; ++i)
    {
        NBRE_Vector2<T> p0 = screenPolyline.Vertex(i);
        NBRE_Vector2<T> p1 = screenPolyline.Vertex(i + 1);
        NBRE_LineClipResult clipResult = ClipLineByRect(rect, p0, p1);

        if (clipResult == NBRE_LCR_OUTSIDE)
        {
            continue;
        }

        if (pl.size() == 0)
        {
            pl.push_back(p0);
        }

        pl.push_back(p1);
        if (clipResult == NBRE_LCR_CLIP_END || clipResult == NBRE_LCR_CLIP_BOTH)
        {
            result.push_back(NBRE_Polyline2<T>(pl));
            pl.clear();
        }
    }
    // if polyline ended inside
    if (pl.size() > 0)
    {
        nbre_assert(pl.size() >= 2);
        result.push_back(NBRE_Polyline2<T>(pl));
        pl.clear();
    }
    return result;
}

template<typename T> NBRE_LineClipResult 
NBRE_Clipping<T>::ClipByPlane(const NBRE_Plane<T>& plane, NBRE_Vector3<T>& np1, NBRE_Vector3<T>& np2)
{
    T t1 = plane.normal.DotProduct(np1) + plane.d;
    T t2 = plane.normal.DotProduct(np2) + plane.d;

    if (t1 >= 0)
    {
        if (t2 >= 0)
        {
            return NBRE_LCR_INSIDE;
        }
        else
        {
            T denominator = plane.normal.DotProduct(np2 - np1);
            nbre_assert(denominator != 0);
            T t = t1 / -denominator;
            np2 = np1 * (1 - t) + np2 * t;
            return NBRE_LCR_CLIP_END;
        }
    }
    else
    {
        if (t2 >= 0)
        {
            T denominator = plane.normal.DotProduct(np2 - np1);
            nbre_assert(denominator != 0);
            T t = t1 / -denominator;
            np1 = np1 * (1 - t) + np2 * t;
            return NBRE_LCR_CLIP_START;
        }
        else
        {
            return NBRE_LCR_OUTSIDE;
        }
    }
}

template<typename T> void 
NBRE_Clipping<T>::ClipByPlane(const NBRE_Plane<T>& plane, const NBRE_Polyline3<T>& polyline, NBRE_Vector< NBRE_Polyline3<T> >& result)
{
    NBRE_Vector< NBRE_Vector3<T> > pl;
    for (uint32 i = 0; i < polyline.VertexCount() - 1; ++i)
    {
        NBRE_Vector3<T> p0 = polyline.Vertex(i);
        NBRE_Vector3<T> p1 = polyline.Vertex(i + 1);
        NBRE_LineClipResult clipResult = ClipByPlane(plane, p0, p1);

        if (clipResult == NBRE_LCR_OUTSIDE)
        {
            continue;
        }

        if (pl.size() == 0)
        {
            pl.push_back(p0);
        }

        pl.push_back(p1);
        if (clipResult == NBRE_LCR_CLIP_END)
        {
            result.push_back(NBRE_Polyline3<T>(pl));
            pl.clear();
        }
    }
    // if polyline ended inside
    if (pl.size() > 0)
    {
        nbre_assert(pl.size() >= 2);
        result.push_back(NBRE_Polyline3<T>(pl));
        pl.clear();
    }
}
