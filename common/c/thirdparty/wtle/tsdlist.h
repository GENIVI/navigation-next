/*****************************************************************************
 *
 *  tsdlist.h - Defines types associated with TsDisplayList.
 *
 *  Copyright (C) 2002, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSDLIST_H
#define TSDLIST_H

#include "ts2d.h"
#include "tsfixed.h"
#include "tstypes.h"
#include "tstextat.h"

TS_BEGIN_HEADER

/*****************************************************************************
 *
 *  Public structure with information about the display list of a TsLayout.
 *
 *  This is made public for use in the call to TsDC_drawGlyphs.
 */
typedef struct TsDisplayListEntry_
{
    TsFixed glyphX;             /* glyph x position */
    TsFixed glyphY;             /* glyph y position */
    TsFixed caretX;             /* caret x position, may be different from glyph x posititon in diacritics, ligatures, etc. */
    TsIndex sourceIndex;        /* index of char in original source data */
    TsIndex logicalOrder;       /* logical order after shaping, etc., but before BiDi reordering */
    TsUInt16 glyphID;           /* glyph ID */
    TsUInt16 boundaryFlags;     /* boundaries at this character position */
} TsDisplayListEntry;


/*****************************************************************************
 *
 *  Private structure containing information about a line in the display list.
 *
 *  Do not access directly.
 */
typedef struct TsDisplayListLineEntry_
{
    TsInt32 glyphIndex;         /* index into layout->displayList */
    TsLength lineLength;        /* number of glyphs in "glyphs" array */
    TsRect boundingBox;         /* union of all glyph "paint" BBs */
    TsRect advanceBox;          /* margins or glyph "advance" extents */
    TsRect alignmentBox;        /* like advanceBox, but excludes trailing whitespace */
    TsUInt8 bidiBaseLevel;      /* base level of paragraph in which line falls */
    TsInt32 compositionWidth;   /* max width for the line (from which left & right indents are subtracted) */
    TsInt32 originalCompositionWidth; /* original width available for line (before left & right indents were subtracted) */
    TsPoint relLineOrigin;      /* relative X & Y coordinates for line (relative to its text container origin) */
    TsPoint absLineOrigin;      /* absolute X & Y coordinates for line (relative to the layout) */
    TsUInt8 lineFlags;          /* flags for indicating if a line has been displayed yet, etc. (e.g. TS_LINE_COMPOSED, TS_LINE_DISPLAYED) */
    TsInt32 startIndex;         /* index into textInfoArray of first char in line */
    TsInt32 endIndex;           /* index into textInfoArray of last char in line */
    TsFixed justificationAdjustment; /* amount to adjust each line break point to make line justified */
    TsFixed baseline;           /* amount to add to (rel|abs)LineOrigin.y to get baseline */
    TsInt32 top;                /* amount to add to (rel|abs)lineOrigin.y to get top of line (used for overline, and background color) */
    TsInt32 bottom;             /* amount to add to (rel|abs)lineOrigin.y to get bottom of line (used for background color) */
    TsInt32 containerIndex;     /* index to the text container which contains this line */
    TsAlignType alignment;      /* alignment of line */
    TsInt32 minLogicalOrder;    /* minimum logical order for line */
    TsInt32 maxLogicalOrder;    /* maximum logical order for line */
    TsInt32 minSourceIndex;     /* minimum source index for line */
    TsInt32 maxSourceIndex;     /* maximum source index for line */
} TsDisplayListLineEntry;

TS_END_HEADER

#endif /* TSDLIST_H */
