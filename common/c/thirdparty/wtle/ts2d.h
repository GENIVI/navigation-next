/*****************************************************************************
 *
 *  ts2d.h - Defines the interface for some simple 2D graphics types and methods.
 *
 *  Copyright (C) 2002-2006, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TS2D_H
#define TS2D_H

#include "tstypes.h"
#include "tsfixed.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  Compares two TsFixed values.
 *
 *  Parameters:
 *      c1       - [in] void pointer to first TsFixed value
 *      c2       - [in] void pointer to second TsFixed value
 *
 *  Return value:
 *      TsInt32 value greater than, equal to, or less than zero,
 *      accordingly as the value pointed to by c1 is greater than,
 *      equal to, or less than the value pointed to by c2.
 *
 *  Notes:
 *      This function can be used as a TsObjectCompareFunc as required
 *      by TsDynamicArray_new and TsDynamicArray_init when TsFixed is used
 *      as the array member and comparison is desired.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsFixed_compare(void *c1, void *c2);


/*****************************************************************************
 *
 *  Compares two TsInt32 values.
 *
 *  Parameters:
 *      c1       - [in] void pointer to first TsInt32 value
 *      c2       - [in] void pointer to second TsInt32 value
 *
 *  Return value:
 *      TsInt32 value greater than, equal to, or less than zero,
 *      accordingly as the value pointed to by c1 is greater than,
 *      equal to, or less than the value pointed to by c2.
 *
 *  Notes:
 *      This function can be used as a TsObjectCompareFunc as required
 *      by TsDynamicArray_new and TsDynamicArray_init when TsInt32 is used
 *      as the array member and comparison is desired.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsInt32_compare(void *c1, void *c2);


/*****************************************************************************
 *
 *  A structure denoting a two-dimensional position value (x, y).
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsPoint_
{
    TsInt32 x;      /* the x position */
    TsInt32 y;      /* the y position */
} TsPoint;


/*****************************************************************************
 *
 *  Gets the x and y coordinates of a point structure.
 *
 *  Parameters:
 *      point       - [in] pointer to the point structure
 *      x           - [out] x position
 *      y           - [out] x position
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsPoint_getCoords(const TsPoint *point, TsInt32 *x, TsInt32 *y);


/*****************************************************************************
 *
 *  Sets the x and y coordinates of a point structure.
 *
 *  Parameters:
 *      point       - [in/out] pointer to the point structure
 *      x           - [in] x position
 *      y           - [in] x position
 *
 *  Return value:
 *      TsPoint structure with x and y assigned.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsPoint)
TsPoint_setCoords(TsPoint *point, TsInt32 x, TsInt32 y);


/*****************************************************************************
 *
 *  A structure denoting a line or line segment.
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsLine_
{
    TsPoint p1;
    TsPoint p2;

} TsLine;


/*****************************************************************************
 *
 *  Intersects a TsFixed y coordinate value with a line and returns the
 *  TsFixed x coordinate.
 *
 *  Parameters:
 *      line        - [in] pointer to the line structure
 *      y           - [in] y coordinate
 *
 *  Return value:
 *      TsFixed     x coordinate at which line intersects given y coordinate.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsFixed)
TsLine_yIntersect(TsLine *line, TsFixed y);


/*****************************************************************************
 *
 *  Intersects a y coordinate value with a line and returns the x coordinate.
 *
 *  Parameters:
 *      l           - [in] pointer to the line structure
 *      xorigin     - [in] x location of line in coordinate space
 *      yorigin     - [in] y location of line in coordinate space
 *      y           - [in] y coordinate
 *
 *  Return value:
 *      TsInt32     x coordinate at which line intersects given y coordinate.
 *
 *  Notes:
 *      This function is intended for internal use only.
 *
 *      The TsLine parameter contains points which are 16.16 values. The
 *      return value is the x coordinate at which the line intersects the
 *      y coordinate after the line has been translated by (xorigin,yorigin).
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsLine_yIntersectEx(TsLine *l, TsInt32 xorigin, TsInt32 yorigin, TsInt32 y);


/*****************************************************************************
 *
 *  Intersects a x coordinate value with a line and returns the y coordinate.
 *
 *  Parameters:
 *      l           - [in] pointer to the line structure
 *      xorigin     - [in] x location of line in coordinate space
 *      yorigin     - [in] y location of line in coordinate space
 *      x           - [in] x coordinate
 *
 *  Return value:
 *      TsInt32     y coordinate at which line intersects given x coordinate.
 *
 *  Notes:
 *      This function is intended for internal use only.
 *
 *      The TsLine parameter contains points which are 16.16 values. The
 *      return value is the y coordinate at which the line intersects the
 *      x coordinate after the line has been translated by (xorigin,yorigin).
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsInt32)
TsLine_xIntersectEx(TsLine *l, TsInt32 xorigin, TsInt32 yorigin, TsInt32 x);


/*****************************************************************************
 *
 *  A structure denoting a rectangle.
 *
 *  Remarks:
 *      The coordinates define the dimensions of the rectangle.  The bottom right
 *      coordinate is outside of the rectangle.  When expressed in pixels, the x2
 *      column and y2 row are outside of the rectangle.  This affects the
 *      interpretation of such functions as TsRect_intersect and others that
 *      take a TsRect argument.
 *
 *      Some graphics systems make the other choice, so this must be accounted
 *      for in the port-specific rendering code.
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsRect_
{
    TsInt32 x1;     /* x-coordinate of upper-left corner */
    TsInt32 y1;     /* y-coordinate of upper-left corner */
    TsInt32 x2;     /* x-coordinate of lower-right corner */
    TsInt32 y2;     /* y-coordinate of lower-right corner */
} TsRect;


/*****************************************************************************
 *
 *  Gets the two points that define a rectangle.
 *
 *  Parameters:
 *      rect        - [in] pointer to the rectangle structure
 *      p1          - [out] upper-left corner
 *      p2          - [out] lower-right corner
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_getPoints(const TsRect *rect, TsPoint *p1, TsPoint *p2);


/*****************************************************************************
 *
 *  Gets all four points that define a rectangle.
 *
 *  Parameters:
 *      rect        - [in] pointer to the rectangle structure
 *      p0          - [out] upper-left corner
 *      p1          - [out] upper-right corner
 *      p2          - [out] lower-right corner
 *      p3          - [out] lower-left corner
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_getAllPoints(const TsRect *rect, TsPoint *p0, TsPoint *p1, TsPoint *p2, TsPoint *p3);


/*****************************************************************************
 *
 *  Initializes a rectangle using two points.
 *
 *  Description:
 *      Using this function ensures that the rectangle is always stored
 *      such that the coordinates of p2 are greater than or equal to the
 *      coordinates of p1, in both the x and y directions.
 *
 *  Parameters:
 *      rect        - [out] pointer to the rectangle structure
 *      p1          - [in] pointer to the first point
 *      p2          - [in] pointer to the second point
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_setPoints(TsRect *rect, const TsPoint *p1, const TsPoint *p2);


/*****************************************************************************
 *
 *  Initializes a rectangle using the four coordinates of two points.
 *
 *  Description:
 *      Using this function ensures that the  rectangle is always stored
 *      such that the coordinates of the second point are greater than or
 *      equal to the first point in both the x and y directions.
 *
 *  Parameters:
 *      rect        - [out] pointer to the rectangle structure
 *      x1          - [in] x coordinate of first point
 *      y1          - [in] y coordinate of first point
 *      x2          - [in] x coordinate of second point
 *      y2          - [in] y coordinate of second point
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_setCoords(TsRect *rect, TsInt32 x1, TsInt32 y1, TsInt32 x2, TsInt32 y2);


/*****************************************************************************
 *
 *  Initializes a rectangle as the intersection of two other rectangles.
 *
 *  Parameters:
 *      rect        -  [out] pointer to the rectangle structure
 *      rect1       -  [in] pointer to first rectangle
 *      rect2       -  [in] pointer to second rectangle
 *
 *  Return value:
 *      TRUE if intersection is defined (has an area), else FALSE.
 *
 *  Remarks:
 *      Both rect1 and rect2 must be normalized, i.e. x2 >= x1 and y2 >= y1.
 *      This can be ensured by using any of the TsRect initializer functions
 *      to initialize those rectangles.
 *
 *      If the return value is FALSE, the intersect values will be zero.
 *
 *  See also:
 *      Remarks under TsRect.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsBool)
TsRect_intersect(TsRect *rect, const TsRect *rect1, const TsRect *rect2);


/*****************************************************************************
 *
 *  Initializes a rectangle as the union of two other rectangles.
 *
 *  Parameters:
 *      rect        -  [out] pointer to the rectangle structure
 *      rect1       -  [in] pointer to first rectangle
 *      rect2       -  [in] pointer to second rectangle
 *
 *  Return value:
 *      none
 *
 *  Remarks:
 *      Both rect1 and rect2 must be normalized, i.e. x2 >= x1 and y2 >= y1.
 *      This can be ensured by using any of the TsRect initializer functions
 *      to initialize those rectangles.
 *
 *  See also:
 *      Remarks under TsRect.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_union(TsRect *rect, const TsRect *rect1, const TsRect *rect2);


/*****************************************************************************
 *
 *  Offsets a rectangle by an amount x, y
 *
 *  Parameters:
 *      rect        - [in/out] pointer to the rectangle structure.
 *      x           - [in] amount to offset x-position.
 *      y           - [in] amount to offset y-position.
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_offset(TsRect *rect, TsInt32 x, TsInt32 y);


/*****************************************************************************
 *
 *  Gets the distance magnitudes from a point to the nearest point on a rectangle.
 *
 *  Parameters:
 *      rect        - [in] pointer to the rectangle structure
 *      point       - [in] pointer to the point
 *      distance    - [out] distance magnitudes -- (0,0) if on or inside the rectangle
 *
 *  Return value:
 *      none
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsRect_getDistance(TsRect *rect, TsPoint *point, TsPoint *distance);


/*****************************************************************************
 *
 *  Calculates the sine of an angle.
 *
 *  Parameters:
 *      a           - [in] angle in degrees as a 16.16 fixed point number
 *
 *  Return value:
 *      sin(a) as a 16.16 fixed point number.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsFixed)
TsAngle_sin(TsFixed a);


/*****************************************************************************
 *
 *  Calculates the cosine of an angle.
 *
 *  Parameters:
 *      a           - [in] angle in degrees as a 16.16 fixed point number
 *
 *  Return value:
 *      cos(a) as a 16.16 fixed point number.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsFixed)
TsAngle_cos(TsFixed a);


/*****************************************************************************
 *
 *  Calculates the tangent of an angle.
 *
 *  Parameters:
 *      a           - [in] angle in degrees as a 16.16 fixed point number
 *
 *  Return value:
 *      tan(a) as a 16.16 fixed point number.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsFixed)
TsAngle_tan(TsFixed a);


/*****************************************************************************
 *
 *  Macro that returns the cosine of 45 degrees as 16.16 fixed point number.
 *
 *  <GROUP 2dgraphics>
 */
#define TS_FIXED_COS45  (46341)


/*****************************************************************************
 *
 *  A structure denoting a 2x2 matrix.
 *
 *  Description:
 *      Coefficients are entered as 16.16
 *      because the matrix coordinates are generally real numbers where the
 *      integer portion is on the order of the point size.  If greater
 *      range is needed then don't use this structure.
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsMatrix_
{
    TsFixed        a;   /* row 0, column 0 */
    TsFixed        b;   /* row 0, column 1 */
    TsFixed        c;   /* row 1, column 0 */
    TsFixed        d;   /* row 1, column 1 */
} TsMatrix;


/*****************************************************************************
 *
 *  Sets the matrix coefficients to values reflecting the identity matrix.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the matrix structure
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_ident(TsMatrix *matrix);


/*****************************************************************************
 *
 *  Copies one matrix to another.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the destination matrix structure
 *      source      - [in] pointer to the source matrix structure
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_copy(TsMatrix *matrix, const TsMatrix *source);


/*****************************************************************************
 *
 *  Compares two matrices, returning boolean representing equality.
 *
 *  Parameters:
 *      matrix      - [in] pointer to the first matrix structure
 *      source      - [in] pointer to the second matrix structure
 *
 *  Return value:
 *      TsBool -- boolean TRUE or FALSE if matrices are equal or not, resp.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(TsBool)
TsMatrix_equal(const TsMatrix *matrix, const TsMatrix *source);


/*****************************************************************************
 *
 *  Gets the matrix coefficients.
 *
 *  Parameters:
 *      matrix      - [in] pointer to the matrix structure
 *      a           - [out] row1, column 1
 *      b           - [out] row1, column 2
 *      c           - [out] row2, column 1
 *      d           - [out] row2, column 2
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_get(const TsMatrix *matrix, TsFixed *a, TsFixed *b, TsFixed *c,  TsFixed *d);


/*****************************************************************************
 *
 *  Sets the matrix coefficients.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the matrix structure
 *      a           - [in] row1, column 1
 *      b           - [in] row1, column 2
 *      c           - [in] row2, column 1
 *      d           - [in] row2, column 2
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_set(TsMatrix *matrix, TsFixed a, TsFixed b, TsFixed c,  TsFixed d);


/*****************************************************************************
 *
 *  Sets the matrix coefficients to values reflecting scale on X and Y axes.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the matrix structure
 *      scaleX      - [in] scale factor along X axis
 *      scaleY      - [in] scale factor along Y axis
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_scale(TsMatrix *matrix, TsFixed scaleX, TsFixed scaleY);


/*****************************************************************************
 *
 *  Sets the matrix coefficients to values reflecting rotation about "z" axis.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the matrix structure
 *      angle       - [in] rotation angle in degrees
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_rotate(TsMatrix *matrix, TsFixed angle);


/*****************************************************************************
 *
 *  Sets the matrix coefficients to values reflecting skew on X and Y axes.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the matrix structure
 *      skewX       - [in] angle of skew on X axis in degrees
 *      skewY       - [in] angle of skew on Y axis in degrees
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_skew(TsMatrix *matrix, TsFixed skewX, TsFixed skewY);


/*****************************************************************************
 *
 *  Transform a point from x,y to u,v coordinates using transform matrix.
 *  Values are passed as TsPoint structures.
 *
 *  Parameters:
 *      matrix      - [in] pointer to the matrix structure
 *      p           - [in] pointer to original point
 *      t           - [out] pointer to transformed point
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_transform(const TsMatrix *matrix, const TsPoint *p, TsPoint *t);


/*****************************************************************************
 *
 *  Transform a point from x,y to u,v coordinates using transform matrix.
 *  Values are passed as fixed-point coordinates.
 *
 *  Parameters:
 *      matrix      - [in] pointer to the matrix structure
 *      x           - [in] x value
 *      y           - [in] y value
 *      u           - [out] u value
 *      v           - [out] v value
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_transform2(const TsMatrix *matrix, TsFixed x, TsFixed y, TsFixed *u, TsFixed *v);


/******************************************************************************
 *
 *  Multiply two matrices storing the product in a third matrix.
 *
 *  Parameters:
 *      C       - [out] pointer to matrix structure for result
 *      A       - [in] pre-multiplicand matrix pointer
 *      B       - [in] post-multiplicand matrix pointer
 *
 *  Return value:
 *      void
 *
 *  Remarks:
 *      Matrix A premultiplies matrix B.
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_multiply(TsMatrix *C, const TsMatrix *A, const TsMatrix *B);


/*****************************************************************************
 *
 *  Initializes the matrix given certain font spatial characteristics.
 *
 *  Description:
 *      This function computes a 2x2 scaling matrix used by iType, UFST,
 *      and possibly other font engines given size, orientation angle and skew
 *      angle.
 *
 *      The scaleX and scaleY arguments control the font size in the x and y
 *      directions respectively.
 *
 *      Orientation controls the orientation of the glyph. The angle is given
 *      in degrees counter clockwise from the horizontal. iType and UFST both return
 *      glyph advance width metrics that work when the baseline is rotated to
 *      the same angle.
 *
 *      The skewX argument is used to produce a skewed glyph that has an italic
 *      appearance. This angle is defined as degrees clockwise from the vertical.
 *      Note the difference in definition from the orientation. A skew angle
 *      of 15 degrees is typical.
 *
 *      Angles are defined in degrees using a 16.16 number.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the initialized matrix structure.
 *      scaleX      - [in] font size in x-direction pixels per em
 *      scaleY      - [in] font size in y-direction pixels per em
 *      angle       - [in] orientation in degrees counterclockwise from horizontal
 *      skewX       - [in] character X skew angle in degrees clockwise from vertical
 *
 *  Version:
 *      In Versions 3.0.x and earlier the angle argument assumed a clockwise definition.
 *      Later versions use the more typical counter-clockwise definition. This should
 *      have no effect on users since support for rotated text was not available
 *      in earlier versions.
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix_init(TsMatrix *matrix, TsFixed scaleX, TsFixed scaleY, TsFixed angle, TsFixed skewX);


/*****************************************************************************
 *
 *  A structure denoting a 3x3 matrix.
 *
 *  Description:
 *      Coefficients are entered as 16.16
 *      because the matrix coordinates are generally real numbers where the
 *      integer portion is on the order of the point size.  If greater
 *      range is needed then don't use this structure.
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsMatrix33_
{
    TsFixed        a00;   /* row 0, column 0 */
    TsFixed        a01;   /* row 0, column 1 */
    TsFixed        a02;   /* row 0, column 2 */
    TsFixed        a10;   /* row 1, column 0 */
    TsFixed        a11;   /* row 1, column 1 */
    TsFixed        a12;   /* row 1, column 2 */
    TsFixed        a20;   /* row 2, column 0 */
    TsFixed        a21;   /* row 2, column 1 */
    TsFixed        a22;   /* row 2, column 2 */
} TsMatrix33;



/*****************************************************************************
 *
 *  Sets the matrix coefficients to values reflecting the identity matrix.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the matrix structure
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix33_ident(TsMatrix33 *matrix);


/*****************************************************************************
 *
 *  Copies one 3x3 matrix to another.
 *
 *  Parameters:
 *      matrix      - [out] pointer to the destination matrix structure
 *      source      - [in] pointer to the source matrix structure
 *
 *  Return value:
 *      void
 *
 *  <GROUP 2dgraphics>
 */
TS_EXPORT(void)
TsMatrix33_copy(TsMatrix33 *matrix, const TsMatrix33 *source);




/****************************************************************************
 *
 *  Typedef for a color value.
 *
 *  Description:
 *      This value represents a color value.  The current implementation
 *      supports ARGB8888 or indexed color.  Do not access these TsColor
 *      values directly, as the implementation may change.
 *      Always use the macros to set and get color values.
 *
 *  <GROUP 2dgraphics>
 */
typedef TsUInt32 TsColor;


#define TSBYTEMASK  (0xFF)


/****************************************************************************
 *
 *  A macro that returns the alpha component of a TsColor value.
 *
 *  <GROUP 2dgraphics>
 */
#define TS_GET_ALPHA(color) (TsByte)(((color) >> 24) & TSBYTEMASK)


/****************************************************************************
 *
 *  A macro that returns the red component of a TsColor value.
 *
 *  <GROUP 2dgraphics>
 */
#define TS_GET_RED(color)  (TsByte)(((color) >> 16) & TSBYTEMASK)


/****************************************************************************
 *
 *  A macro that returns the green component of a TsColor value.
 *
 *  <GROUP 2dgraphics>
 */
#define TS_GET_GREEN(color) (TsByte)(((color) >> 8) & TSBYTEMASK)


/****************************************************************************
 *
 *  A macro that returns the blue component of a TsColor value.
 *
 *  <GROUP 2dgraphics>
 */
#define TS_GET_BLUE(color) (TsByte)((color) & TSBYTEMASK)


/****************************************************************************
 *
 *  A macro that returns a TsColor value initialized with 8-bit
 *  red, green, blue components.
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_RGB(r,g,b) TS_COLOR_ARGB(255, r, g, b)



 /****************************************************************************
 *
 *  Macro that returns a TsColor value set to black (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_BLACK TS_COLOR_RGB(0, 0, 0)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to white (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_WHITE TS_COLOR_RGB(255, 255, 255)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to dark gray (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_DARK_GRAY TS_COLOR_RGB(0x77, 0x77, 0x77)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to red (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_RED TS_COLOR_RGB(255, 0, 0)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to green (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_GREEN TS_COLOR_RGB(0, 255, 0)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to blue (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_BLUE TS_COLOR_RGB(0, 0, 255)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to cyan (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_CYAN TS_COLOR_RGB(0, 255, 255)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to magenta (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_MAGENTA TS_COLOR_RGB(255, 0, 255)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set to yellow (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_YELLOW TS_COLOR_RGB(255, 255, 0)


/****************************************************************************
 *
 *  Macro that returns a TsColor value set a fully transparent color (ARGB).
 *
 *  <GROUP 2dgraphics>
 */
#define TS_COLOR_TRANSPARENT TS_COLOR_ARGB(0, 0, 0, 0)



/*****************************************************************************
 *
 *  Typedef for line styles.
 *
 *  <GROUP 2dgraphics>
 */
typedef TsInt8 TsLineStyle;

#define TS_LINE_NONE            0   /* no line */
#define TS_LINE_SINGLE_SOLID    1   /* single solid line */
#define TS_LINE_DOUBLE_SOLID    2   /* two solid lines with space */
#define TS_LINE_BROKEN          3   /* a single broken or dashed line */
#define TS_LINE_DOTTED          4   /* a single dotted line */



/*****************************************************************************
 *
 *  Typedef for a structure that defines a pen to draw lines.
 *
 *  <GROUP 2dgraphics>
 */
typedef struct TsPen_
{
    TsLineStyle style;      /* line style */
    TsInt32 thickness;      /* thickness of line */
    TsColor color;          /* color of line */
} TsPen;


/* Deprecated *************************************************************/


typedef TsFixed TsAngle;    /* Use TsFixed instead */


TS_END_HEADER


#endif /* TS2D_H */
