
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* Copyright: 1987-1990 by Apple Computer, Inc., all rights reserved. */

/* fs_fontscal.h */


#ifndef FS_FONTSCAL_H
#define FS_FONTSCAL_H

#include "fs_object.h"

/*** Memory shared between all fonts and sizes and transformations ***/
#define KEY_PTR_BASE                0 /* Constant Size ! */
#define VOID_FUNC_PTR_BASE          1 /* Constant Size ! */
#define WORK_SPACE_BASE             2 /* size is sfnt dependent, can't be shared between grid-fitting and scan-conversion */

/*** Memory that can not be shared between fonts and different sizes, can not dissappear after InitPreProgram() ***/
#define PRIVATE_FONT_SPACE_BASE     3 /* size is sfnt dependent */
#define MAX_MEMORY_AREAS            4 /* this index is not used for memory */

#define NONVALID 0xffff

/*
 * Output data structure to the Font Scaler.
 */
typedef struct {
    FS_LONG         memorySizes[MAX_MEMORY_AREAS];
    FS_USHORT       glyphIndex;

    /* Spline Data */
    FS_USHORT       outlinesExist;
    FS_USHORT       numberOfContours;
    F26DOT6        *xPtr, *yPtr;
    FS_SHORT       *startPtr;
    FS_SHORT       *endPtr;
    FS_BYTE        *onCurve;
    /* End of spline data */

    FS_USHORT       emResolution;
    F26DOT6        *scaledCVT;
    } fs_GlyphInfoType;

/*
 * Input data structure to the Font Scaler.
 *
*/
typedef struct {
    FS_FIXED version;
    FS_BYTE* memoryBases[MAX_MEMORY_AREAS];
    SFNT*    sfnt;
    
    union {
        struct {
            FS_USHORT      platformID;
            FS_USHORT      specificID;
            FS_USHORT      languageID;
            } newsfnt;
        struct {
            FS_FIXED       pointSize;
            FS_SHORT       xResolution;
            FS_SHORT       yResolution;
            FS_FIXED       pixelDiameter;
            transMatrix    matrix;
            } newtrans;
        struct {
            FS_USHORT      characterCode;
            FS_USHORT      glyphIndex;
            } newglyph;
        } param;
    } fs_GlyphInputType;

#ifndef FIXEDSQRT2
#define FIXEDSQRT2 0x00016A0AL
#endif

/* prototypes */
FS_LONG fs_NewSfnt(_DS_ fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr );
FS_LONG fs_NewTransformation(_DS_ fs_GlyphInputType *inputPtr, fs_GlyphInfoType *outputPtr );

#endif  /* FS_FONTSCAL_H */
