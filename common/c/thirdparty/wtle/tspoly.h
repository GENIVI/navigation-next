/*****************************************************************************
 *
 *  tspoly.h - Defines the interface for a polygon.
 *
 *  Copyright (C) 2006 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential
 *
 ****************************************************************************/

#ifndef TSPOLY_H
#define TSPOLY_H

#include "ts2d.h"
#include "tsdynarr.h"
#include "tseudata.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  A structure denoting a polygon or "polyline" on the z-plane.
 *
 *  Notes:
 *      Operations will automatically connect the first and last points
 *      when the polygon is "closed" (as opposed to an "open" "polyline").
 *
 *      Whether this actually is a "closed" polygon or an "open" "polyline"
 *      can be determined from the context of its use.  For example, it would
 *      be assumed that it's a "closed" polygon in the usage case of:
 *          anchored objects for wrap-around text
 *          filling
 *
 *      For an operation such as:
 *          stroking
 *      the user can force a "closed" polygon by explicitly adding a
 *      final/last point that matches the first point.  Otherwise, the
 *      "open" "polyline" stroke would conclude at the last given point.
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsPolygon_
{
#ifdef TS_USE_EXTERNAL_USERDATA
    /* IMPORTANT: If externalUserData is used then it must be the first
       element in structure. This supports a particular IDL mechanism. */
    TsExternalUserData externalUserData;
#endif

    TsDynamicArray /* <TsPoint> */ points;  /* 16.16 fixed point coordinates */

   /* below items are private: for use only by TsPolygon_XXX methods */
    TsDynamicArray /* <TsLine> */ ySortedSegments;

    TsMemMgr *memMgr;
    TsRefCount refCount;

} TsPolygon;

/*****************************************************************************
 *
 *  Construct (allocate memory for) and initialize a new memory managed
 *  polygon object.
 *
 *  Parameters:
 *      memMgr  - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      TsPolygon pointer to new polygon structure, NULL if error.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsPolygon *)
TsPolygon_newex(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Construct (allocate memory for) and initialize a new polygon object.
 *
 *  Parameters:
 *      none
 *
 *  Return value:
 *      TsPolygon pointer to new polygon structure, NULL if error.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsPolygon *)
TsPolygon_new(void);


/*****************************************************************************
 *
 *  Delete the memory associated with a polygon, including the polygon itself.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsPolygon_delete(TsPolygon *polygon);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsPolygon object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsPolygon_releaseHandle.
 *
 *  Parameters:
 *      polygon   - [in] this
 *
 *  Return value:
 *      Pointer to polygon object.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsPolygon *)
TsPolygon_copyHandle(TsPolygon *polygon);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsPolygon object.
 *
 *  Description:
 *      This function reverses the action of TsPolygon_copyHandle.
 *      If this is the last reference then the polygon is deleted.
 *
 *  Parameters:
 *      polygon   - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsPolygon_releaseHandle(TsPolygon *polygon);


/*****************************************************************************
 *
 *  Returns the object's memory manager.
 *
 *  Description:
 *      This function returns the memory manager the object was created with,
 *      or NULL. For use in dynamic array copy functions.
 *
 *  Parameters:
 *      polygon   - [in] this
 *
 *  Return value:
 *      pointer to the TsMemMgr that was used to create this object, or NULL
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsMemMgr *)
TsPolygon_getMemMgr(TsPolygon *polygon);

/*****************************************************************************
 *
 *  Append a point to the end of a polygon.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *      point       - [in] pointer to the TsPoint information to be appended
 *
 *  Return value:
 *      TsInt32 number of points in polygon or -1 if there is an error
 *
 *  Notes:
 *      The polygon will be appended to without first being cleared.  Any
 *      points it previously contains will remain.  This is useful for
 *      constructing a multi-shape "path".
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsPolygon_appendPoint(TsPolygon *polygon, TsPoint *point);


/*****************************************************************************
 *
 *  Append the two points of a TsLine object to a polygon.
 *
 *  Parameters:
 *      polygon   - [in] pointer to the polygon structure
 *      line      - [in] pointer to the TsLine information to be appended
 *
 *  Return value:
 *      TsInt32 number of points in polygon or -1 if an error occurs
 *
 *  Notes:
 *      The polygon will be appended to without first being cleared.  Any
 *      points it previously contains will remain.  This is useful for
 *      constructing a multi-shape "path".
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsPolygon_appendLine(TsPolygon *polygon, TsLine *line);


/*****************************************************************************
 *
 *  Append the four corner points of a TsRect object to a polygon.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *      rectangle   - [in] pointer to the TsRect information to be appended
 *
 *  Return value:
 *      TsInt32 number of points in polygon or -1 if an error occurs
 *
 *  Notes:
 *      The polygon will be appended to without first being cleared.  Any
 *      points it previously contains will remain.  This is useful for
 *      constructing a multi-shape "path".
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsPolygon_appendRect(TsPolygon *polygon, TsRect *rectangle);


/*****************************************************************************
 *
 *  Ensure that last point in polygon is equal to first point.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *
 *  Return value:
 *      TsInt32 number of points in polygon or -1 if an error occurs
 *
 *  Notes:
 *      Enforces that this is a "closed" polygon.  If the last point of
 *      the polygon is not equal to the first point, then a new, last
 *      point is appended to the polygon that equals the first point.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsPolygon_close(TsPolygon *polygon);


/*****************************************************************************
 *
 *  Return the number of points in a polygon.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *
 *  Return value:
 *      TsInt32     number of points in polygon
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      p argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  <GROUP 2dgraphics>
 */
#define TsPolygon_length(p) ((p!=NULL) ? TsDynamicArray_length(&(p)->points) : 0)


/*****************************************************************************
 *
 *  Clear the polygon structure to contain zero (0) points.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsPolygon_clear(TsPolygon *polygon);


/*****************************************************************************
 *
 *  Calculates x-coordinate intersections with polygon at given y coordinate.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *      y           - [in] y coordinate
 *      loMatch     - [in] boolean to match low y endpoint of segments
 *      xCoords     - [out] array in which to insert x intersections
 *
 *  Return value:
 *      TsFixed value of next higher y coordinate having segment transition
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
 *      y coordinate intersects the polygon.  The insertion of the x
 *      coordinates into the array is done using
 *      TsDynamicArray_insertDuplicates().  Duplicates, obviously, are
 *      stored.  But, also, note that an important implication follows:
 *      IMPORTANT:  The array of x coordinates will be sorted upon
 *      insertion.  In order to sort properly, it is HIGHLY RECOMMENDED to
 *      initialize the TsDynamicArray with a valid TsFixed comparison
 *      function such as "TsFixed_compare", declared in ts2d.h.
 *
 *      The array will have x coordinates inserted without being otherwise
 *      modified or cleared.  Any coordinates it previously contains will
 *      remain.  This is useful for creating a sorted list of intersections
 *      with multiple polygons.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsFixed)
TsPolygon_yIntersects(TsPolygon *polygon, TsFixed y, TsBool loMatch, TsDynamicArray /* <TsFixed> */ *xCoords);


/*****************************************************************************
 *
 *  Calculates x-coordinate intersections with polygon at given y coordinate.
 *
 *  Parameters:
 *      polygon     - [in] pointer to the polygon structure
 *      xorigin     - [in] x location of polygon in coordinate space
 *      yorigin     - [in] y location of polygon in coordinate space
 *      y           - [in] y coordinate
 *      loMatch     - [in] boolean to match low y endpoint of segments
 *      xCoords     - [out] array in which to insert x intersections
 *
 *  Return value:
 *      TsInt32 value of next higher y coordinate having segment transition
 *
 *  Notes:
 *      The function is intended for internal use only.
 *
 *      See also the notes for TsPolygon_yIntersects. This function operates
 *      in a similar fashion.
 *
 *      The points in the TsPolygon structure are 16.16 values. (Therefore
 *      have a maximum x,y values of 32K-1.) This function operates in
 *      coordinate space (max value = TS_INT32_MAX), and (xorigin, yorigin)
 *      is the location of the polygon in coordinate space.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsPolygon_yIntersectsEx(TsPolygon *polygon, TsInt32 xorigin, TsInt32 yorigin,
           TsInt32 y, TsBool loMatch, TsDynamicArray /* <TsInt32> */ *xCoords);


/*****************************************************************************
 *
 *  Returns a pointer to an object which contains external data managed by
 *  the user.
 *
 *  Description:
 *      This function returns a TsExternalUserData pointer which allows
 *      the user to access external data that was set by a call to
 *      TsExternalUserData_init.
 *
 *      TS_USE_EXTERNAL_USERDATA must be defined in order to use the
 *      private data mechanism. If not defined this function returns NULL.
 *
 *      The TsExternalUserData object that is returned by this function
 *      becomes invalid when this TsPolygon object is destroyed.
 *      Therefore, you must keep reference to the TsPolygon object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      polygon     - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsExternalUserData *)
TsPolygon_getExternalUserdata(TsPolygon *polygon);


TS_END_HEADER


#endif /* TSPOLY_H */
