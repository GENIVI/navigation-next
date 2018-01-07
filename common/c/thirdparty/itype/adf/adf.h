
/* Copyright (C) 2008 Monotype Imaging Inc. All rights reserved. */

/* Confidential Information of Monotype Imaging Inc. */

/* adf.h */


#ifndef ADF_H
#define ADF_H

#ifdef FS_EDGE_TECH

#include "adftypesystem.h"

#ifdef FS_EDGE_RENDER
#include "adfimplicit.h"
#endif

#ifdef FS_EDGE_TECH

#define SIZEOFADFHEADER 8
#define SIZEOFCSMRANGE  4
#define SIZEOFCSMRECORD 14
#define SIZEOFALZIXRECORD 6

#endif /* FS_EDGE_TECH */

/* Data structures for Edge rendering */
#ifdef FS_EDGE_RENDER

typedef struct csmrange_ {
    FS_USHORT endGlyphIndex;        /* index of last glpyh in range               */
    FS_USHORT offset;               /* offset to CSM entry in CSMentries[]        */
} CSMRANGE;

typedef struct csmrecord_ {
    FS_USHORT size;                 /* lower limit of size range (inclusive)      */
    FS_FIXED insideCutoff;          /* ADF inside cutoff value for size range     */
    FS_FIXED outsideCutoff;         /* ADF outside cutoff value for size range    */
    FS_FIXED gamma;                 /* ADF gamma value for size range             */
} CSMRECORD;

typedef struct csmentry_ {
    FS_USHORT numberOfCSMRecords;   /* number of CSM records in this entry        */
    CSMRECORD CSMrecords[1];        /* array of CSM records for this entry        */
} CSMENTRY;

typedef struct alzixrecord_ {
    FS_USHORT    startGlyphIndex;    /* Glyph index of first character in range            */
    FS_USHORT    endGlyphIndex;      /* Glyph index of last character in range             */
    FS_USHORT    startValueIndex;    /* Index into value array of first character in range */
                                     /* if 0xffff then this range uses MAZ,                */
                                     /* otherwise this range uses BAZ and                  */ 
                                     /* index = startValueIndex + gindex – startGlyphIndex */
} ALZIXRECORD;

typedef struct alzindexarray_ {
    FS_USHORT    numberOfAlZIndices;  /* Number of entries in ALZ indexing array, at least 1 */
    ALZIXRECORD    AlZIXrecords[1];   /* Array of ALZ indexing records                       */
} ALZINDEXARRAY;


#endif /* FS_EDGE_RENDER */

/* Common methods for hinting and rendering */
#ifdef FS_EDGE_TECH

FS_VOID startADF(_DS0_);
FS_VOID stopADF(_DS0_);
FS_VOID adfGetEdgeHintData(_DS_ TTF *ttf, FS_ULONG glyph_index, FS_USHORT *EdgeHintType,
                           FS_SHORT *noCenter, FS_SHORT *cvtstart, 
                           FS_SHORT *numylines, FS_SHORT *isrighttoleft);

#endif /* FS_EDGE_TECH */

/* Edge rendering methods */
#ifdef FS_EDGE_RENDER

FS_GLYPHMAP *get_ADF_graymap(_DS_ FS_ULONG id, FS_USHORT index, FS_USHORT type);
FS_GRAYMAP *get_distance_field(_DS_ FS_ULONG id, FS_USHORT index);
FS_VOID adfSetDefaultRenderAttrs(ADFRenderAttrs  *adfRenderAttrs);
FS_VOID adfGetCSMValues(_DS_ TTF *ttf, FS_ULONG glyph_index, ADFRenderAttrs *adfRenderAttrs);
FS_VOID adfComputeTypographicScaling(_DS_ SFNT *sfnt, ADFRenderAttrs  *adfRenderAttrs);
FS_VOID adfCopyRenderAttrs(ADFRenderAttrs  *srcRenderAttrs, ADFRenderAttrs  *dstRenderAttrs);
ADFPath *make_ADF_path(_DS_ FS_OUTLINE *outl, ADF_U32 id, FS_GRAYMAP **g, 
                              FS_FIXED *scaled_dx, FS_FIXED *scaled_dy);
FS_VOID get_ADF_adjustments(_DS_ FS_OUTLINE *outl, FS_ULONG id, FS_USHORT index,
                            FS_FIXED *adj_x, FS_FIXED *adj_y);
#endif /* FS_EDGE_RENDER */

#endif /* FS_EDGE_TECH */

#endif /* ADF_H */
