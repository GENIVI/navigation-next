/*****************************************************************************
 *
 *  tsimage.h - Defines interface to TsGlyphImage class
 *
 *  Copyright (C) 2004 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSIMAGE_H
#define TSIMAGE_H

#include "tstypes.h"

TS_BEGIN_HEADER

/******************************************************************************
 *
 *  Enumeration of image types.
 *
 *  <GROUP render>
 *
 */
typedef enum TsImageType_
{
    TS_IMAGE_BINARY,            /* 1-bit image */
    TS_IMAGE_GRAYSCALE,         /* 2 or more bit image */
    TS_IMAGE_ICON,              /* Colored icon image */
    TS_IMAGE_VGPATHSCALED,      /* OpenVG scaled path */
    TS_IMAGE_VGPATHUNSCALED,    /* OpenVG scaled path */
    TS_IMAGE_VGBITMAP,          /* OpenVG bitmap image */
    TS_IMAGE_VGGRAYMAP          /* OpenVG graymap image */
} TsImageType;


/******************************************************************************
 *
 *  Enumeration of image compression types.
 *
 *  <GROUP render>
 *
 */
typedef enum TsCompressionType_
{
    TS_IMAGE_COMPRESSION_NONE = 0,  /* Not compressed */
    TS_IMAGE_COMPRESSION_UFST       /* UFST-grayscale compressed (UFST only) */
} TsCompressionType;


/******************************************************************************
 *
 *  Structure for storing a glyph image.
 *
 *  Description:
 *      The TsGlyph object can use this structure when calling TsDC_drawImage.
 *      Defining a generic glyph image type allows the rendering code to
 *      be written without relying on font-engine specific types.
 *
 *  <GROUP render>
 *
 */
typedef struct TsGlyphImage_
{
    TsImageType type;       /* type of image */
    TsInt16 width;          /* image width in pixels*/
    TsInt16 height;         /* image height in pixels */
    TsInt16 xOffset;        /* x-coordinate offset from glyph origin to image origin */
    TsInt16 yOffset;        /* y-coordinate offset from glyph origin to image origin */
    TsInt16 bpl;            /* bytes per line (if applicable) */
    TsInt16 org;            /* image organization */
    TsCompressionType comp; /* compression technique */
    TsByte bitsPerPixel;    /* bits per pixel */
    TsColor color;          /* color for 'on' pixels */
    TsSize size;            /* size in bytes of image data - or 0 if equal to (bpl * height) */
    TsByte *bits;           /* image data */
} TsGlyphImage;


TS_END_HEADER


#endif /* TSIMAGE_H */
