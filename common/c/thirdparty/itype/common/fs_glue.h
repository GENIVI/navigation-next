
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* Copyright: 1987-1990 by Apple Computer, Inc., all rights reserved. */

/* fs_glue.h */

#ifndef FS_GLUE_H
#define FS_GLUE_H


/* two routines need _DS_ arguments */
#include "fs_object.h"

#define POINTSPERINCH              72
#define MAX_ELEMENTS               2
#define MAX_TWILIGHT_CONTOURS      1

#define TWILIGHTZONE 0 /* The point storage */
#define GLYPHELEMENT 1 /* The actual glyph */

#define USE_INTEGER_SCALING        0x0008

/* public phantom points start here */
#define LEFTSIDEBEARING   0
#define RIGHTSIDEBEARING  1
#define TOPSIDEBEARING    2
#define BOTTOMSIDEBEARING 3

/* private phantom points start here */
#define ORIGINPOINT       4
#define LEFTEDGEPOINT     5
#define TOPORIGINPOINT    6
#define TOPEDGEPOINT      7

#define PHANTOMCOUNT      8

typedef struct {
    FS_SHORT xMin;
    FS_SHORT yMin;
    FS_SHORT xMax;
    FS_SHORT yMax;
} sfnt_BBox;

typedef struct {
    FS_SHORT         numberContours;
    FS_SHORT        *endPoints;               /** vector: indexes into x[], y[] **/
    FS_SHORT         numberInstructions;
    FS_BYTE         *instructions;            /** vector **/
    FS_BYTE         *flags;                   /** vector **/
    sfnt_BBox        bbox;
} sfnt_PackedSplineFormat;

typedef struct {
    FS_USHORT advanceWidth;
    FS_SHORT leftSideBearing;
    } HMTX;

typedef struct {
    FS_USHORT advanceHeight;
    FS_SHORT topSideBearing;
    } VMTX;

/*** Offset table ***/
typedef struct {
    FS_LONG            interpreterFlagsOffset;
    FS_LONG            startPointOffset;
    FS_LONG            endPointOffset;
    FS_LONG            oldXOffset;
    FS_LONG            oldYOffset;
    FS_LONG            scaledXOffset;
    FS_LONG            scaledYOffset;
    FS_LONG            newXOffset;
    FS_LONG            newYOffset;
    FS_LONG            onCurveOffset;
    FS_LONG            MAZOutlineContoursOffset;
    FS_LONG            MAZOutlinePointsOffset;
    FS_LONG            MAZHSegsOffset;
    FS_LONG            MAZVSegsOffset;
    FS_LONG            MAZOutlineContourListOffset;
    FS_LONG            MAZOutlineSegPairsOffset;
    FS_LONG            RTGAHPiecesOffset;
    FS_LONG            RTGAHStrokesOffset;
    FS_LONG            RTGAHStrokesPlistOffset;
    FS_LONG            RTGAHEdgesOffset;
    FS_LONG            RTGAHXminOffset;
    FS_LONG            RTGAHYminOffset;
    FS_LONG            RTGAHXmaxOffset;
    FS_LONG            RTGAHYmaxOffset;
} fsg_OffsetInfo;


/*  #define COMPSTUFF  */

/*** Element Information ***/
typedef struct {
    FS_LONG             stackBaseOffset;
    fsg_OffsetInfo      offsets[MAX_ELEMENTS];
    fnt_ElementType     interpreterElements[MAX_ELEMENTS];
} fsg_ElementInfo;

typedef struct {
    FS_ULONG  Offset;
    FS_ULONG  Length;
} fsg_OffsetLength;

/*** The Internal Key ***/
typedef struct fsg_SplineKey {
    SFNT            *sfnt;
    TTF_MAXP        *maxp;                 /* shortcut for sfnt->lfnt->ttf->maxp */

    FS_USHORT        glyphIndex;           /* */

    FS_BYTE*         memoryBases[4];       /* array of memory handles */

    fsg_ElementInfo  elementInfoRec;       /* element info structure */

    FS_USHORT        emResolution;         /* used to be FS_LONG <4> */

    FS_FIXED         fixedPointSize;       /* user point size */
    FS_FIXED         interpScalar;         /* scalar for instructable things */
    FS_FIXED         metricScalar;         /* scalar for metric things */

    transMatrix      currentTMatrix;       /* Current Transform Matrix */
    FS_BYTE          identityTransformation;
    
    FS_USHORT        fontFlags;            /* copy of header.flags */

    FS_FIXED         pixelDiameter;

    FS_USHORT        state;                 /* for error checking purposes */
    FS_LONG          scanControl;           /* flags for dropout control etc.  */
    
    /* for key->memoryBases[PRIVATE_FONT_SPACE_BASE] */
    FS_LONG          offset_storage;
    FS_LONG          offset_controlValues;
        
    FS_LONG          cvtCount;

    FS_USHORT        numberOf_LongHorMetrics;
#ifdef FS_CFFR 
    /* allocated sizeof elementPtr */
    FS_SHORT max_contours;
    FS_SHORT max_points;
#endif
    /* new for nested-composite rewrite */
    FS_SHORT        num_comps;
    FS_SHORT        num_contours;
    FS_LONG         num_points;
    FS_BYTE         localTIsIdentity;
    
    FIXED_VECTOR    tInfo;    
    FS_BYTE         executePrePgm;
    FS_BYTE         executeFontPgm;
    FS_BYTE         read_programs;            /* read 'fpgm' and 'prep' ? */
    FS_BYTE         useMyMetrics;
    fnt_GlobalGraphicStateType globalGS;
    
    } fsg_SplineKey;


/* for the key->state field */
#define FS_INITIALIZED  0x0001
#define FS_NEWSFNT      0x0002
#define FS_NEWTRANS     0x0004

#define GLOBALGSTATE(key0)    ((fnt_GlobalGraphicStateType*) \
                    &key0->globalGS)

/***************/
/** INTERFACE **/
/***************/

extern FS_ULONG fsg_PrivateFontSpaceSize(_DS_ fsg_SplineKey *key);
extern FS_LONG fsg_GridFit(_DS_ fsg_SplineKey *key, FS_BOOLEAN useHints);
#ifdef FS_CFFR 
extern FS_LONG cff_GridFit(_DS_ fsg_SplineKey *key, FS_BOOLEAN useHints);
#endif
/***************/


/* matrix routines */

extern FS_VOID fsg_ReduceMatrix(fsg_SplineKey* key);

/*
 *    Used in FontScaler.c and MacExtra.c, lives in FontScaler.c
 */
FS_LONG fsg_RunFontProgram(_DS_ fsg_SplineKey* key );


/* 
** Other externally called functions.  Prototype calls added on 4/5/90
*/
FS_VOID fsg_IncrementElement(fsg_SplineKey *key, FS_LONG n, FS_LONG numPoints, FS_LONG numContours);

FS_VOID fsg_InitInterpreterTrans(fsg_SplineKey *key  );

FS_VOID fsg_SetUpElement(_DS_ fsg_SplineKey *key, FS_LONG n);

FS_ULONG fsg_WorkSpaceSetOffsets(_DS_ fsg_SplineKey *key);

FS_LONG fsg_SetDefaults(fsg_SplineKey* key );

FS_LONG fsg_RunPreProgram(_DS_ fsg_SplineKey *key);


#endif  /* FS_GLUE_H */
