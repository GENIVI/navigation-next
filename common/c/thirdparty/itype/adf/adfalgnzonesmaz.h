/**
 *-----------------------------------------------------------------------------------
 *    Filename: ADFAlgnZonesMAZ.h
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Copyright 2004-2007 Mitsubishi Electric Research Laboratories (MERL)
 *    An API for automatic grid fitting using Multiple Alignment Zones (MAZ)
 *    Eric Chan and Ron Perry
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    This file describes a single API for both the floating point and the fixed point
 *    implementations of the MAZ alignment zone detection and grid fitting system for
 *    outline-based glyphs and the MAZ alignment zone detection and grid fitting system
 *    for uniform-width stroke-based glyphs.
 *
 *    The documentation in this file uses the term "real-valued coordinates" to refer
 *    to real numbers such as 0, 1.7, 3/11, and the constant PI. In the floating point
 *    API and implementation, real-valued coordinates are represented using the ADF_F32
 *    floating point data type. In the fixed point API and implementation, real-valued
 *    coordinates are represented using the ADF_I1616 fixed point data type.
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    To avoid multiple inclusion of header files
 *-----------------------------------------------------------------------------------
 */
#ifndef ADFALGNZONESMAZ_H
#define ADFALGNZONESMAZ_H


#if defined(FS_EDGE_HINTS) || defined(FS_EDGE_RENDER)


/**
 *-----------------------------------------------------------------------------------
 *    Required include files for this header file (not its implementation)
 *-----------------------------------------------------------------------------------
 */
#include "adftypesystem.h"
#include "adffixedmath.h"


/**
 *-----------------------------------------------------------------------------------
 *    To make functions accessible from C++ code
 *-----------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif


/**
 *-----------------------------------------------------------------------------------
 *    START: FIXED POINT MATH ONLY
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    The ADFPenCmdFx data structure is identical to the ADFPenCmd data structure (see
 *    ADFTypeSystem.h) with the exception that the elements x, y, cx, and cy are
 *    represented as ADF_I1616 fixed point values instead of ADF_F32 floating point
 *    values.
 *-----------------------------------------------------------------------------------
 */
typedef struct {
    ADF_U32      opCode;   /* ADF_PEN_MOVETO_CMD, ADF_PEN_LINETO_CMD, etc. */
    ADF_I1616    x;        /* The x coordinate of the pen command */
    ADF_I1616    y;        /* The y coordinate of the pen command */
    ADF_I1616    cx;       /* The x coordinate of the control point of curvto commands */
    ADF_I1616    cy;       /* The y coordinate of the control point of curvto commands */
    ADF_I1616    cx2;       /* The x coordinate of the control point of curvto commands */
    ADF_I1616    cy2;       /* The y coordinate of the control point of curvto commands */
    ADF_U32      HcollapsePriority;
    ADF_U32      VcollapsePriority;
}    ADFPenCmdFx;
/**
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    The ADF_R32 and MAZPenCmd typedefs allow most of the MAZ grid fitting code to be
 *    shared between the floating point and fixed point implementations. The typedefs
 *    also allow the floating point and fixed point implementations to share a single
 *    API. For the fixed point API and implementation, define ADF_R32 to be an
 *    ADF_I1616 fixed point data type and define MAZPenCmd to be an ADFPenCmdFx data
 *    structure.
 *-----------------------------------------------------------------------------------
 */
typedef ADF_I1616 ADF_R32;
typedef ADFPenCmdFx MAZPenCmd;


/**
 *-----------------------------------------------------------------------------------
 *    END: FIXED POINT MATH ONLY
 *-----------------------------------------------------------------------------------
 */

#ifdef FS_EDGE_HINTS

/**
 *-----------------------------------------------------------------------------------
 *    PUBLIC FUNCTIONS FOR PERFORMING MAZ ALIGNMENT ZONE DETECTION AND GRID FITTING
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    ADFGridFitMAZOutline() performs MAZ alignment zone detection and grid fitting on
 *    the specified sequence of pen commands penCmds representing an outline-based
 *    glyph. Although ADFGridFitMAZOutline() can be invoked on any outline-based glyph,
 *    its algorithms are designed and optimized for CJK glyphs. Upon entry, libInst is
 *    a valid Saffron library instance, penCmds is the contiguous array of numPenCmds
 *    pen commands to be grid fit, and ppem is the number of pixels per em that will be
 *    used to perform the grid fitting. The coordinates of each pen command in the
 *    penCmds array must be specified in non-negative real-valued image coordinates.
 *    Grid fitting is performed in-place (i.e., the computed results are written to the
 *    x, y, cx, and cy coordinates of each pen command in the penCmds array, thereby
 *    overwriting their original values). ADFGridFitMAZOutline() returns true (i.e., a
 *    non-zero value) on success, false (i.e., zero) on failure. Upon failure, no
 *    changes are made to the coordinates in the penCmds array.
 *-----------------------------------------------------------------------------------
 */
ADF_I32 ADFGridFitMAZOutline (void *libInst, MAZPenCmd *penCmds, ADF_I32 numPenCmds,
ADF_R32 ppem);


/**
 *-----------------------------------------------------------------------------------
 *    ADFGridFitMAZStroke() performs MAZ alignment zone detection and grid fitting on
 *    the specified sequence of pen commands penCmds representing a uniform-width
 *    stroke-based glyph. Although ADFGridFitMAZStroke() can be invoked on any
 *    uniform-width stroke-based glyph, its algorithms are designed and optimized for
 *    CJK glyphs. Upon entry, libInst is a valid Saffron library instance, penCmds is
 *    the contiguous array of numPenCmds pen commands to be grid fit, and pathWidth is
 *    a pointer to the path width (i.e., stroke width) of the uniform-width
 *    stroke-based glyph. xMin, yMin, xMax, and yMax are the glyph's bounding box
 *    coordinates (which account for the path width). The coordinates of the pen
 *    commands, the path width, and the bounding box must be specified in non-negative
 *    real-valued image coordinates. Grid fitting is performed in-place (i.e., the
 *    computed results are written to the x, y, cx, and cy coordinates of each pen
 *    command in the penCmds array, thereby overwriting their original values). Upon
 *    exit, pathWidth contains the hinted path width determined during grid fitting.
 *    ADFGridFitMAZStroke() returns true (i.e., a non-zero value) on success, false
 *    (i.e., zero) on failure. Upon failure, no changes are made to pathWidth or to the
 *    coordinates in the penCmds array.
 *-----------------------------------------------------------------------------------
 */
ADF_I32 ADFGridFitMAZStroke (void *libInst, MAZPenCmd *penCmds, ADF_I32 numPenCmds,
                             ADF_R32 *pathWidth, ADF_R32 xMin, ADF_R32 yMin, 
                             ADF_R32 xMax, ADF_R32 yMax);


/**
 *-----------------------------------------------------------------------------------
 *    INTERNAL DATA STRUCTURES AND FUNCTIONS FOR THE IMPLEMENTATIONS AND VALIDATION OF 
 *    THE MAZ ALIGNMENT ZONE DETECTION AND GRID FITTING SYSTEMS
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    MAZ DATA STRUCTURES AND FUNCTIONS FOR OUTLINE-BASED GLYPHS
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    The following data structures and functions are placed in this header file to
 *    accommodate the validation of MAZ alignment zones for outline-based glyphs. To
 *    better understand these data structures and functions and the role they play in
 *    determining MAZ alignment zones, refer to the Algorithm Overview section in
 *    ADFAlgnZonesMAZOutlines.c. The Algorithm Overview section also explains the
 *    terminology used below.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Point representation for an outline-based glyph
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    A MAZOutlinePoint represents a single two-dimensional (x, y) point of an
 *    outline-based glyph. It can represent either an "on-curve" point (i.e., a point
 *    that lies on the outline) or an "off-curve" point (i.e., a point that may or may
 *    not lie on the outline, such as the control point for a quadratic Bezier curve
 *    segment). The elements of a MAZOutlinePoint are:
 *
 *    ox, oy: The original real-valued image coordinates of the point.
 *
 *    hx, hy: The hinted real-valued image coordinates of the point.
 *
 *    flags: A set of bit flags that determine special properties of the point (see
 *    Point Flags section below).
 *
 *    prev: A pointer to the previous point in this point's contour. The points in a
 *    contour form a circular linked list; prev is a pointer to the previous element in
 *    this linked list.
 *
 *    next: A pointer to the next point in this point's contour. The points in a
 *    contour form a circular linked list; next is a pointer to the next element in
 *    this linked list.
 *
 *    contour: A pointer to the contour to which this point belongs.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Point flags
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
#define OUTLINE_OFF_CURVE         0x0001        /* Set if a point is an off-curve point */
#define OUTLINE_X_GRID_FIT        0x0002        /* Set if a point has been grid fit in x */
#define OUTLINE_Y_GRID_FIT        0x0004        /* Set if a point has been grid fit in y */
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Macros that evaluate whether p is an on-curve point or an off-curve point
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
#define IS_ON(p)                (!((p)->flags & OUTLINE_OFF_CURVE))
#define IS_OFF(p)                ((p)->flags      & OUTLINE_OFF_CURVE)
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Forward references
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
struct MAZOutlineContourS;
struct MAZOutlineSegmentS;

#define HF_HREDUCE          0x1
#define HF_VREDUCE          0x2
#define HF_HORVREDUCE       0x3
#define HF_XDELTA           0x4
#define HF_YDELTA           0x8
#define HF_PROCESSEDYDELTA  0x10
#define HF_PROCESSEDXDELTA  0x20
#define HF_YLINEALIGNED     0x40

/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
typedef struct MAZOutlinePointS {
    ADF_R32                    ox;              /* Original (unhinted) x coordinate */
    ADF_R32                    oy;              /* Original (unhinted) y coordinate */
    ADF_R32                    hx;              /* Hinted x coordinate */
    ADF_R32                    hy;              /* Hinted y coordinate */
    ADF_I32                    flags;           /* Records special properties of point */
    ADF_I32                    hintFlags;       /* bit 0 - H stroke reduce 
                                                 * bit 1 - V stroke reduce
                                                 * bit 2 - X stroke delta
                                                 * bit 3 - Y stroke delta */
    ADF_R32                    Xdelta;
    ADF_R32                    Ydelta;
    ADF_I32                    pointNum;
    struct MAZOutlinePointS    *prev;           /* Previous point in the contour */
    struct MAZOutlinePointS    *next;           /* Next point in the contour */
    struct MAZOutlineContourS  *contour;        /* Contour to which this point belongs */
}    MAZOutlinePoint;
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Contour representation for an outline-based glyph
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    A MAZOutlineContour represents a sequence of points that define a closed path in
 *    an outline-based glyph. The MAZOutlineContour data structure is divided into two
 *    parts. The first part applies to all contours of a glyph (i.e., both root
 *    contours and internal contours), while the second part applies only to root
 *    contours and the radicals they represent. See the Algorithm Overview section in
 *    ADFAlgnZonesMAZOutlines.c for the definitions of root contours, internal
 *    contours, and radicals.
 *
 *    Radicals are represented implicitly by the MAZOutlineContour data structure. The
 *    elements in the second part of the data structure describe the segments and
 *    segment pairs contained in the radical.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    The following elements apply to all contours:
 *
 *    xMin, xMax, yMin, yMax: The real-valued image coordinates of this contour's
 *    bounding box before grid fitting is applied.
 *
 *    bboxArea: The area (in real-valued image coordinates) of this contour's bounding
 *    box before grid fitting is applied.
 *
 *    points: A pointer to the first point in this contour.
 *
 *    radical: A pointer to the root contour that contains this contour. If this
 *    contour is itself the root contour, then this element points to itself (i.e., if
 *    C is a pointer to a root contour, then C->radical == C).
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    The following elements apply only to root contours and the radicals they
 *    represent (they are undefined for other contours):
 *
 *    hSeg: A pointer to the first horizontal simple segment in this root contour's
 *    radical. All horizontal simple segments in this radical form a linked list and
 *    hSeg is a pointer to the first element of this linked list.
 *
 *    vSeg: A pointer to the first vertical simple segment in this root contour's
 *    radical. All vertical simple segments in this radical form a linked list and vSeg
 *    is a pointer to the first element of this linked list.
 *
 *    hMergeSeg: A pointer to the first horizontal simple segment comprising the first
 *    horizontal merged segment in this root contour's radical. All horizontal merged
 *    segments in this radical form a linked list and hMergeSeg is a pointer to the
 *    first element of this linked list.
 *
 *    vMergeSeg: A pointer to the first vertical simple segment comprising the first
 *    vertical merged segment in this root contour's radical. All vertical merged
 *    segments in this radical form a linked list and vMergeSeg is a pointer to the
 *    first element of this linked list.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
typedef struct MAZOutlineContourS {

    /**
     *----The following attributes apply to all contours
     */
    ADF_R32                    xMin;            /* Min x-coordinate of bounding box */
    ADF_R32                    xMax;            /* Max x-coordinate of bounding box */
    ADF_R32                    yMin;            /* Min y-coordinate of bounding box */
    ADF_R32                    yMax;            /* Max y-coordinate of bounding box */
    ADF_R32                    bboxArea;        /* This contour's bounding box area */
    ADF_I32                    isOuter;
    MAZOutlinePoint            *points;         /* First point of this contour */
    struct MAZOutlineContourS  *radical;        /* Pointer to the root contour */

    /**
     *----The following attributes apply only to root contours and the radicals
     *----they represent
     */
    struct MAZOutlineSegmentS  *hSeg;            /* First h-simple-seg in this radical */
    struct MAZOutlineSegmentS  *vSeg;            /* First v-simple-seg in this radical */
    struct MAZOutlineSegmentS  *hMergeSeg;       /* First h-merged-seg in this radical */
    struct MAZOutlineSegmentS  *vMergeSeg;       /* First v-merged-seg in this radical */
}    MAZOutlineContour;
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Segment representation for an outline-based glyph
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Simple segments are represented explicitly by the MAZOutlineSegment data
 *    structure. Merged segments and paired segments are represented implicitly (via
 *    pointers and linked lists) by the MAZOutlineSegment data structure. See the
 *    Algorithm Overview section in ADFAlgnZonesMAZOutlines.c for the definitions of
 *    simple segments, merged segments, and paired segments.
 *
 *    Terminology: The documentation below uses the identifier "S" to refer to a simple
 *    segment and the identifier "M" to refer to the merged segment that contains S. A
 *    simple segment S is the "head" of a merged segment M if S is the first element in
 *    the linked list of simple segments that comprise M.
 *
 *    The elements of a MAZOutlineSegment are:
 *
 *    increasing: The Boolean value that is set to true if (1) S is horizontal and is
 *    oriented from left to right or (2) S is vertical and is oriented from bottom to
 *    top. increasing is set to false otherwise.
 *
 *    isMerged: The Boolean value that is set to false during the initialization of S
 *    but later set to true when S is added to a merged segment.
 *
 *    minLocal, maxLocal: If S is horizontal, minLocal and maxLocal are the minimum and
 *    maximum original x coordinates of S, respectively. If S is vertical, minLocal and
 *    maxLocal are the minimum and maximum original y coordinates of S, respectively.
 *    In summary, minLocal and maxLocal are the minimum and maximum original
 *    coordinates of S parallel to the direction of S. minLocal and maxLocal are
 *    represented in real-valued image coordinates.
 *
 *    minMerged, maxMerged: Every simple segment S is contained in a merged segment M.
 *    If S is not the head of M, then minMerged and maxMerged are ignored and
 *    undefined. If S is horizontal and is the head of M, minMerged and maxMerged are
 *    the minimum and maximum original x coordinates across all simple segments in M,
 *    respectively. If S is vertical and is the head of M, minMerged and maxMerged are
 *    the minimum and maximum original y coordinates across all simple segments in M,
 *    respectively. In summary, minMerged and maxMerged are the minimum and maximum
 *    original coordinates of M parallel to the direction of M. minMerged and maxMerged
 *    are represented in real-valued image coordinates.
 *
 *    minPerpLocal, maxPerpLocal: If S is horizontal, minPerpLocal and maxPerpLocal are
 *    the minimum and maximum original y coordinates of S, respectively. If S is
 *    vertical, minPerpLocal and maxPerpLocal are the minimum and maximum original x
 *    coordinates of S, respectively. In summary, minPerpLocal and maxPerpLocal are the
 *    minimum and maximum original coordinates of S perpendicular to the direction of
 *    S. minPerpLocal and maxPerpLocal are represented in real-valued image
 *    coordinates.
 *
 *    minPerpMerged, maxPerpMerged: Every simple segment S is contained in a merged
 *    segment M. If S is not the head of M, then minPerpMerged and maxPerpMerged are
 *    ignored and undefined. If S is horizontal and is the head of M, minPerpMerged and
 *    maxPerpMerged are the minimum and maximum original y coordinates across all
 *    simple segments in M, respectively. If S is vertical and is the head of M,
 *    minPerpMerged and maxPerpMerged are the minimum and maximum original x
 *    coordinates across all simple segments in M, respectively. In summary,
 *    minPerpMerged and maxPerpMerged are the minimum and maximum original coordinates
 *    of M perpendicular to the direction of M. minPerpMerged and maxPerpMerged are
 *    represented in real-valued image coordinates.
 *
 *    p1: A pointer to the first point in S. Note that p1 immediately precedes p2 (see
 *    below) in the ordered sequence of pen commands that describe the input glyph.
 *
 *    p2: A pointer to the second point in S. Note that p2 immediately follows p1 (see
 *    above) in the ordered sequence of pen commands that describe the input glyph.
 *
 *    next: If S is horizontal, next is a pointer to the next horizontal simple segment
 *    in S's radical. If S is vertical, next is a pointer to the next vertical simple
 *    segment in S's radical. A NULL value indicates that S is the last element in the
 *    linked list of simple segments.
 *
 *    mergeChain: A merged segment is represented implicitly by a linked list of simple
 *    segments; mergeChain is the pointer used to implement this linked list. There is
 *    no significance to the ordering of simple segments within the linked list. If S
 *    is horizontal and is contained in a merged segment M, mergeChain points to the
 *    next horizontal simple segment in M (or is set to NULL if S is the last element
 *    of the linked list). Similarly, if S is vertical and is contained in a merged
 *    segment M, mergeChain points to the next vertical simple segment in M (or is set
 *    to NULL if S is the last element of the linked list). Note that a merged segment
 *    may contain only one simple segment.
 *
 *    nextMerge: Merged segments within a radical are connected to each other by a
 *    linked list; nextMerge is the pointer used to implement this linked list. There
 *    is no significance to the ordering of merged segments within the linked list. If
 *    S is not the head of a merged segment, then nextMerge is ignored and undefined.
 *    If S is horizontal, is the head of a merged segment M, and is contained in
 *    radical R, nextMerge points to the first horizontal simple segment in the next
 *    horizontal merged segment in radical R (or is set to NULL if M is the last
 *    element of the linked list). Similarly, if S is vertical, is the head of a merged
 *    segment M, and is contained in radical R, nextMerge points to the first vertical
 *    simple segment in the next vertical merged segment in radical R (or is set to
 *    NULL if M is the last element of the linked list).
 *
 *    pairMax: If S is not the head of a merged segment M, then pairMax is ignored and
 *    undefined. If S is the head of a merged segment M and M is the minimum paired
 *    segment of a segment pair, then pairMax points to the maximum paired segment in
 *    the segment pair (and pairMin (see below) will be set to NULL). Otherwise,
 *    pairMax is set to NULL.
 *
 *    pairMin: If S is not the head of a merged segment M, then pairMin is ignored and
 *    undefined. If S is the head of a merged segment M and M is the maximum paired
 *    segment of a segment pair, then pairMin points to the minimum paired segment in
 *    the segment pair (and pairMax (see above) will be set to NULL). Otherwise,
 *    pairMin is set to NULL.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
typedef struct MAZOutlineSegmentS {
    ADF_I32                    increasing;    /* Boolean: true if S is increasing */
    ADF_I32                    isMerged;      /* Boolean: true when S is merged */
    ADF_I32                    dontMerge;     /* Boolean: true when S shouldn't be merged */
    ADF_R32                    minLocal;      /* Min parallel orig. coordinate of S */
    ADF_R32                    maxLocal;      /* Max parallel orig. coordinate of S */
    ADF_R32                    minMerged;     /* Min parallel orig. coordinate of M */
    ADF_R32                    maxMerged;     /* Max parallel orig. coordinate of M */
    ADF_R32                    minPerpLocal;  /* Min perp original coordinate of S */
    ADF_R32                    maxPerpLocal;  /* Max perp original coordinate of S */
    ADF_R32                    minPerpMerged; /* Min perp original coordinate of M */
    ADF_R32                    maxPerpMerged; /* Max perp original coordinate of M */
    MAZOutlinePoint            *p1;           /* First point of S */
    MAZOutlinePoint            *p2;           /* Second point of S */
    struct MAZOutlineSegmentS  *next;         /* Next simple segment in S's radical */
    struct MAZOutlineSegmentS  *mergeChain;   /* Next simple segment in M */
    struct MAZOutlineSegmentS  *nextMerge;    /* First simple seg in next merged seg */
    struct MAZOutlineSegmentS  *pairMax;      /* Pointer to maximum paired segment */
    struct MAZOutlineSegmentS  *pairMin;      /* Pointer to minimum paired segment */
}    MAZOutlineSegment;
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Segment pair representation for an outline-based glyph
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    A MAZOutlineSegPair represents either a horizontal segment pair or a vertical
 *    segment pair. The documentation below uses the identifier P to refer to a segment
 *    pair variable. Let P = {S1,S2}, where S1 is the minimum paired segment of P and
 *    S2 is the maximum paired segment of P. The concepts of segment pairs, minimum
 *    paired segments, maximum paired segments, child segment pairs, segment pair
 *    trees, rounding up, and rounding down are described in the Algorithm Overview
 *    section in ADFAlgnZonesMAZOutlines.c.
 *
 *    The elements of a MAZOutlineSegPair are:
 *
 *    width: P's original pair width.
 *
 *    cOrig: If P is a horizontal segment pair, cOrig is the original coordinate of P
 *    (i.e., the original middle y coordinate of S1). If P is a vertical segment pair,
 *    cOrig is the original coordinate of P (i.e., the original middle x coordinate of
 *    S1).
 *
 *    cMin, cMax: If P is a horizontal segment pair, cMin and cMax are the minimum and
 *    maximum original x coordinates over all points in S1 and S2, respectively. If P
 *    is a vertical segment pair, cMin and cMax are the minimum and maximum original y
 *    coordinates over all points in S1 and S2, respectively.
 *
 *    cHint: If P is a horizontal segment pair, cHint is the hinted y coordinate of S1.
 *    If P is a vertical segment pair, cHint is the hinted x coordinate of S1.
 *
 *    cHintLow: The maximum integer not greater than (cOrig - 0.5) (i.e., cHintLow is
 *    the "floor" of (cOrig - 0.5)). Example #1: if cOrig is 7.4, then cHintLow is 6.
 *    Example #2: if cOrig is 9.5, then cHintLow is 9.
 *
 *    canRoundDownSolo: A Boolean set to true if P can be rounded down (i.e., aligned
 *    to cHintLow) without colliding with any of its children in their current state.
 *
 *    canRoundDownTree: A Boolean set to true if the segment pair tree with root P can
 *    be rounded down without creating any collisions whatsoever in the segment pair
 *    tree.
 *
 *    seg: A pointer to the MAZOutlineSegment data structure that represents S1.
 *
 *    link: A temporary pointer used for linked list traversals of segment pairs.
 *
 *    children: A fixed length array of pointers to child segment pairs.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    MAZ_OUTLINE_MAX_CHILDREN is the maximum number of children a MAZOutlineSegPair
 *    can have. Experiments on thousands of outline-based glyphs from different
 *    typefaces show that segment pairs have 1 child on average and at most 3 children.
 *    Segment pairs with 4 or more children are extremely rare and occur only in highly
 *    complex glyphs. Collisions may not be resolved between P and all of its children
 *    if P has more than MAZ_OUTLINE_MAX_CHILDREN children.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
#define MAZ_OUTLINE_MAX_CHILDREN 4
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
typedef struct MAZOutlineSegPairS
{
    ADF_R32 width;                     /* P's original pair width */
    ADF_R32 cOrig;                     /* Original middle coordinate of S1 */
    ADF_R32 cMin;                      /* Minimum original coordinate of S1 and S2 */
    ADF_R32 cMax;                      /* Maximum original coordinate of S1 and S2 */
    ADF_I32 cHint;                     /* Hinted coordinate of S1 */
    ADF_I32 cHintLow;                  /* Floor of (cOrig - 0.5) */
    ADF_I32 canRoundDownSolo;          /* True: P can be rounded down */
    ADF_I32 canRoundDownTree;          /* True: P's seg pair tree can be rounded down */
    ADF_I32 numChildren;               /* Number of children of P */
    ADF_I32 numParents;                /* Number of parents of P */
    ADF_I32 hintFlags;                 /* bit 0 - H stroke reduce 
                                        * bit 1 - V stroke reduce
                                        * bit 2 - X stroke delta
                                        * bit 3 - Y stroke delta */
    ADF_R32 Xdelta;
    ADF_R32 Ydelta;
    MAZOutlineSegment *seg;              /* Pointer to S1 */
    struct MAZOutlineSegPairS *link;     /* Temp pointer used for linked list traversals */
    struct MAZOutlineSegPairS *children[MAZ_OUTLINE_MAX_CHILDREN]; /* Child seg pairs */
    struct MAZOutlineSegPairS *parents[MAZ_OUTLINE_MAX_CHILDREN];  /* Child seg pairs */
}    MAZOutlineSegPair;
/**
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    ADFGridFitMAZOutlineInternal() performs MAZ alignment zone detection and grid
 *    fitting on the specified sequence of pen commands penCmds representing an
 *    outline-based glyph. Although ADFGridFitMAZOutlineInternal() can be invoked on
 *    any outline-based glyph, its algorithms are designed and optimized for CJK
 *    glyphs. Upon entry, libInst is a valid Saffron library instance, penCmds is the
 *    contiguous array of numPenCmds pen commands to be grid fit, and ppem is the
 *    number of pixels per em that will be used to perform the grid fitting. The
 *    coordinates of each pen command in the penCmds array must be specified in
 *    non-negative real-valued image coordinates. Grid fitting is performed in-place
 *    (i.e., the computed results are written to the x, y, cx, and cy coordinates of
 *    each pen command in the penCmds array, thereby overwriting their original
 *    values). 
 *
 *    Upon exit, outPoints points to a contiguous array of MAZOutlinePoints,
 *    outContours points to a contiguous array of MAZOutlineContours, outHSegs points
 *    to a contiguous array of horizontal simple segments, outVSegs points to a
 *    contiguous array of vertical simple segments, outNumPoints contains the number of
 *    points in the outPoints array, outNumContours contains the number of contours in
 *    the outContours array, outOuterClockwise contains a Boolean that is set to true
 *    if ADFGridFitMAZOutlineInternal() determined that outer contours of the glyph are
 *    oriented clockwise and false otherwise, and outMem points to a contiguous block
 *    of memory that must be freed by the caller. Freeing outMem also frees outPoints,
 *    outContours, outHSegs, and outVSegs, thereby invalidating all five pointers. The
 *    caller should not free outPoints, outContours, outHSegs, or outVSegs directly.
 *
 *    Note that radicals, merged segments, and segment pairs are represented implicitly
 *    by the returned data structures.
 *
 *    ADFGridFitMAZOutlineInternal() returns true (i.e., a non-zero value) on success,
 *    false (i.e., zero) on failure. Upon failure, no changes are made to the
 *    coordinates in the penCmds array and no data is written to outPoints,
 *    outContours, outHSegs, outVSegs, outNumPoints, outNumContours, outOuterClockwise,
 *    or outMem.
 *-----------------------------------------------------------------------------------
 */
ADF_I32 ADFGridFitMAZOutlineInternal (void *libInst, MAZPenCmd *penCmds, ADF_I32
numPenCmds, ADF_R32 ppem, MAZOutlinePoint **outPoints, MAZOutlineContour
**outContours, MAZOutlineSegment **outHSegs, MAZOutlineSegment **outVSegs, ADF_I32
*outNumPoints, ADF_I32 *outNumContours, ADF_I32 *outOuterClockwise, void **outMem);






/**
 *-----------------------------------------------------------------------------------
 *    MAZ DATA STRUCTURES AND FUNCTIONS FOR UNIFORM-WIDTH STROKE-BASED GLYPHS
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    The following data structures and functions are placed in this header file to
 *    accommodate the validation of MAZ alignment zones for uniform-width stroke-based
 *    glyphs. To better understand these data structures and functions and their role
 *    in performing grid fitting on uniform-width stroke-based glyphs, refer to the
 *    Algorithm Overview section in ADFAlgnZonesMAZStrokes.c. The Algorithm Overview
 *    section also explains the terminology used below.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Stroke-based segment representation for a uniform-width stroke-based glyph
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    A MAZStrokeSegment represents either a horizontal segment or a vertical segment.
 *    The documentation below uses the identifier S to refer to a segment variable. The
 *    concepts of segments, child segments, segment trees, rounding up, and rounding
 *    down are described in the Algorithm Overview section in ADFAlgnZonesMAZStrokes.c.
 *
 *    The elements of a MAZStrokeSegment are:
 *
 *    cOrig: If S is horizontal, cOrig is the original y coordinate of S. If S is
 *    vertical, cOrig is the original x coordinate of S.
 *
 *    cMin, cMax: If S is horizontal, cMin and cMax are the minimum and maximum
 *    original x coordinates of S, respectively. If S is vertical, cMin and cMax are
 *    the minimum and maximum original y coordinates of S, respectively.
 *
 *    cHint: If S is horizontal, cHint is the hinted y coordinate of S (i.e., the
 *    integer y coordinate to which S is aligned). If S is vertical, cHint is the
 *    hinted x coordinate of S (i.e., the integer x coordinate to which S is aligned).
 *
 *    cHintLow: The maximum integer not greater than cOrig (i.e., cHintLow is the
 *    "floor" of cOrig). Example #1: if cOrig is 7.6, then cHintLow is 7. Example #2:
 *    if cOrig is 9.0, then cHintLow is 9.
 *
 *    canRoundDownSolo: A Boolean set to true if S can be rounded down (i.e., aligned
 *    to cHintLow) without colliding with any of its children in their current state.
 *
 *    canRoundDownTree: A Boolean set to true if the segment tree with root S can be
 *    rounded down without creating any collisions whatsoever in the segment tree.
 *
 *    isCollapsed: A Boolean set to true if S has been aligned to one of its child
 *    segments during collision resolution via merging. This occurs when S collides
 *    when one of its children and the collision cannot be resolved via rounding.
 *
 *    link: A temporary pointer used for linked list traversals of segments.
 *
 *    children: A fixed length array of pointers to child segments.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    MAZ_STROKE_MAX_CHILDREN is the maximum number of children a MAZStrokeSegment can
 *    have. Experiments on thousands of uniform-width stroke-based glyphs from
 *    different typefaces show that segments have 1 child on average and at most 3
 *    children. Segments with 4 or more children are extremely rare and occur only in
 *    highly complex glyphs. Collisions may not be resolved between S and all of its
 *    children if S has more than MAZ_STROKE_MAX_CHILDREN children.
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
#define MAZ_STROKE_MAX_CHILDREN 4
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
typedef struct MAZStrokeSegmentS
{
    ADF_R32 cOrig;                  /* Original segment coordinate */
    ADF_R32 cMin;                   /* Original minimum coordinate */
    ADF_R32 cMax;                   /* Original maximum coordinate */
    ADF_I32 cHint;                  /* Hinted segment coordinate */
    ADF_I32 cHintLow;               /* Floor of cOrig */
    ADF_I32 canRoundDownSolo;       /* True: S can be rounded down */
    ADF_I32 canRoundDownTree;       /* True: S's segment tree can be rounded down */
    ADF_I32 isCollapsed;            /* True: S has been aligned to 1 of its children */
    ADF_I32 numChildren;            /* Number of children of this segment */
    ADF_I32 numParents;
    ADF_I32 hintFlags;              /* bit 0 - H stroke reduce 
                                     * bit 1 - V stroke reduce
                                     * bit 2 - X stroke delta
                                     * bit 3 - Y stroke delta */
    struct MAZStrokeSegmentS *link; /* Temp pointer used for linked list traversals */
    struct MAZStrokeSegmentS *children[MAZ_STROKE_MAX_CHILDREN];  /* Child segments */
    struct MAZStrokeSegmentS *parents[MAZ_STROKE_MAX_CHILDREN];  /* Child segments */
}    MAZStrokeSegment;


/**
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    ADFGridFitMAZStrokeInternal() performs MAZ alignment zone detection and grid
 *    fitting on the specified sequence of pen commands penCmds representing a
 *    uniform-width stroke-based glyph. Although ADFGridFitMAZStrokeInternal() can be
 *    invoked on any uniform-width stroke-based glyph, its algorithms are designed and
 *    optimized for CJK glyphs. Upon entry, libInst is a valid Saffron library
 *    instance, penCmds is the contiguous array of numPenCmds pen commands to be grid
 *    fit, and pathWidth is a pointer to the path width (i.e., stroke width) of the
 *    uniform-width stroke-based glyph. xMin, yMin, xMax, and yMax are the glyph's
 *    bounding box coordinates (which account for the path width). The coordinates of
 *    the pen commands, the path width, and the bounding box must be specified in
 *    non-negative real-valued image coordinates. Grid fitting is performed in-place
 *    (i.e., the computed results are written to the x, y, cx, and cy coordinates of
 *    each pen command in the penCmds array, thereby overwriting their original
 *    values).
 *
 *    Upon exit, pathWidth contains the hinted path width determined during grid
 *    fitting, outNumHSegs contains the number of horizontal segments determined during
 *    grid fitting, outNumVSegs contains the number of vertical segments determined
 *    during grid fitting, outHSegs points to a contiguous array of outNumHSegs
 *    horizontal segments determined during grid fitting, outVSegs points to a
 *    contiguous array of outNumVSegs vertical segments determined during grid fitting,
 *    and outMem points to a contiguous block of memory that must be freed by the
 *    caller. Freeing outMem also frees outHSegs and outVSegs, thereby invalidating all
 *    three pointers. The caller should not free outHSegs or outVSegs directly.
 *
 *    ADFGridFitMAZStrokeInternal() returns true (i.e., a non-zero value) on success,
 *    false (i.e., zero) on failure. Upon failure, no changes are made to pathWidth or
 *    to the coordinates in the penCmds array, and no data is written to outNumHSegs,
 *    outNumVSegs, outHSegs, outVSegs, or outMem.
 *-----------------------------------------------------------------------------------
 */
ADF_I32 ADFGridFitMAZStrokeInternal (void *libInst, MAZPenCmd *penCmds, 
                                     ADF_I32 numPenCmds, ADF_R32 *pathWidth, 
                                     ADF_R32 xMin, ADF_R32 yMin, ADF_R32 xMax, 
                                     ADF_R32 yMax, ADF_I32 *outNumHSegs, 
                                     ADF_I32 *outNumVSegs, MAZStrokeSegment **outHSegs,
                                     MAZStrokeSegment **outVSegs, void **outMem);

#endif /* FS_EDGE_HINTS */

/**
 *-----------------------------------------------------------------------------------
 *    End of C++ wrapper
 *-----------------------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif


#endif /* FS_EDGE_HINTS or FS_EDGE_RENDER */

/**
 *-----------------------------------------------------------------------------------
 *    End of ADFALGNZONESMAZ_H
 *-----------------------------------------------------------------------------------
 */
#endif
