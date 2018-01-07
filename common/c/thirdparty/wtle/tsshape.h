/*****************************************************************************
 *
 *  tsshape.h - Declares the abstract interface for a shape.
 *
 *  Copyright (C) 2006 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential
 *
 ****************************************************************************/

#ifndef TSSHAPE_H
#define TSSHAPE_H


#include "tspoly.h"  /* in real abstract implementation, this is removed */


TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  An abstract class that defines the interface to a geometric shape.
 *
 *
 * currently this is implemented purely by direct derivation from TsPolygon *
 *
 *  <GROUP 2dgraphics>
 */
#if 0
typedef struct TsShape_
{
    void *shapeObj;

    /* virtual function table */
    TsInt32 (*yIntersects)(void *shapeObj, TsInt32 y, TsBool loMatch, TsDynamicArray /* <TsInt32> */ *xCoords);

} TsShape;
#endif

typedef TsPolygon TsShape;

#if 0
/*****************************************************************************
 *
 *  Calculates x-coordinate intersections with shape at given y coordinate.
 *
 *  Parameters:
 *      shape       - [in] pointer to the shape object
 *      y           - [in] y coordinate
 *      loMatch     - [in] boolean to match low y endpoint of segments
 *      xCoords     - [out] array in which to insert x intersections
 *
 *  Return value:
 *      TsInt32 value of next higher y coordinate having segment transition
 *
 *  Notes:
 *      "loMatch" can generally be TRUE or FALSE, of your choosing.  Useful
 *      to change the polarity when more accuracy is desired intersecting low
 *      or high y endpoints of segments.  FALSE matches high values, TRUE for
 *      low.  Illustration may prove useful.  With this polygon:
 *
 *          x= 0   1   2
 *              _______
 *             |___    |   ___ find intersections at this y-value
 *                 |___|
 *
 *      when intersections are found at the indicated y-value, if "loMatch"
 *      is TRUE, the x-coordinates will be at 1 and 2.  Alternatively, if
 *      "loMatch" is FALSE, the x-coordinates will be at 0 and 2.  (I am
 *      assuming lower y values are on the top, higher toward the bottom.)
 *
 *      Return value indicates the next higher y value where the number of
 *      segments will change and a known transition of the number of
 *      intersections will occur.
 *
 *      Inserts into the given array the x coordinates where the given
 *      y coordinate intersects the shape.  The insertion of the
 *      x coordinates into the array is done using
 *      TsDynamicArray_insertDuplicates().  Duplicates, obviously, are
 *      stored.  But, also, note that an important implication follows:
 *      IMPORTANT:  The array of x coordinates will be sorted upon
 *      insertion.  In order to sort properly, it is HIGHLY RECOMMENDED to
 *      initialize the TsDynamicArray with a valid TsInt32 comparison
 *      function such as "TsInt32_compare", declared in ts2d.h.
 *
 *      The array will have x coordinates inserted without being otherwise
 *      modified or cleared.  Any coordinates it previously contains will
 *      remain.  This is useful for creating a sorted list of intersections
 *      with multiple shapes.
 *
 *      Currently this is implemented purely by direct derivation from TsPolygon.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsFixed)
TsShape_yIntersects(TsShape *shape, TsInt32 y, TsBool loMatch, TsDynamicArray /* <TsFixed> */ *xCoords);

/*****************************************************************************
 *
 *  Calculates x-coordinate intersections with shape at given y coordinate.
 *
 *  See comments for TsPolygon_yIntersectsEx.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsShape_yIntersectsEx(TsPolygon *polygon, TsInt32 xorigin, TsInt32 yorigin,
           TsInt32 y, TsBool loMatch, TsDynamicArray /* <TsInt32> */ *xCoords);
#endif

#define TsShape_yIntersects(t, y, l, c) TsPolygon_yIntersects(t, y, l, c)
#define TsShape_yIntersectsEx(t, xo, yo, y, l, c) TsPolygon_yIntersectsEx(t, xo, yo, y, l, c)


#define TsShape_copyHandle(p)    TsPolygon_copyHandle((p))
#define TsShape_releaseHandle(p) TsPolygon_releaseHandle((p))
#define TsShape_getMemMgr(p) TsPolygon_getMemMgr((p))

TS_END_HEADER


#endif /* TSSHAPE_H */
