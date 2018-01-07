/*****************************************************************************
 *
 *  tstxtinf.h - Defines interface to TsTextProcessInfo class.
 *
 *  Copyright (C) 2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSTXTINF_H
#define TSTXTINF_H

#include "tsglypos.h"
#include "ts2d.h"
#include "tsunicde.h"
#include "tsconfig.h"
#ifndef TS_NO_OPENTYPE
#include "otwrdblk.h"
#endif

TS_BEGIN_HEADER

/*****************************************************************************
 *
 * Flags defining the state of the bounding box in a TsTextProcessInfo structure
 * [these are set by WTLE, not WT Shaper]
 *
 */
#define TS_TPI_FLAG_BB_EMPTY    0x00    /* bounding box has not been set */
#define TS_TPI_FLAG_BB_MAX      0x01    /* bounding box is the maximum possible for the glyph in the current fontstyle (fast to compute) */
#define TS_TPI_FLAG_BB_EXACT    0x02    /* bounding box is the actual bounding box for the glyph */


/*****************************************************************************
 *
 *  Macros that define boundary flags, for use with TsTextProcessInfo (& TsGlyphInfo) boundaryFlags.
 *
 */
#define TS_GRAPHEME_START             0x0001
#define TS_GRAPHEME_END               0x0002
#define TS_WORD_START                 0x0004
#define TS_WORD_END                   0x0008
#define TS_POTENTIAL_LINE_START       0x0010
#define TS_POTENTIAL_LINE_END         0x0020
#define TS_REQUIRED_LINE_END          0x0040
#define TS_LINE_BOUNDARY              0x0080 /* [set by WTLE or client, not WT Shaper] */
#define TS_LINE_BOUNDARY_AND_NEW_LINE 0x0100 /* [set by WTLE or client, not WT Shaper] */
#define TS_PARAGRAPH_BOUNDARY         0x0200
#define TS_SHAPING_BOUNDARY           0x0400 /* shaping run ends here */
#define TS_TRAILING_WHITESPACE        0x0800 /* [set by WTLE or client, not WT Shaper] whitespace at end of line, or in a run of whitespace at end of line */
#define TS_OPENTYPE_GPOS_PROCESSED    0x1000 /* this has been through GPOS */
#define TS_INLINE_IMAGE               0x2000 /* [set by WTLE or client, not WT Shaper] this is a placeholder for an inline image */
#define TS_JUSTIFICATION_POINT        0x4000 /* [set by WTLE or client, not WT Shaper] make spacing adjustment at this point for justification */
#define TS_RTL_GLYPH                  0x8000 /* set to 1 if glyph is RTL (Right-To-Left), 0 if LTR (Left-To-Right) */


/*****************************************************************************
 *
 * Starting flags -- used when decomposing a character
 *
 */
#define TS_START_FLAGS   \
    (TS_GRAPHEME_START | \
     TS_WORD_START     | \
     TS_POTENTIAL_LINE_START)

/*****************************************************************************
 *
 * Ending flags -- used when decomposing a character
 *
 */
#define TS_END_FLAGS                 \
    (TS_GRAPHEME_END               | \
     TS_WORD_END                   | \
     TS_POTENTIAL_LINE_END         | \
     TS_REQUIRED_LINE_END          | \
     TS_LINE_BOUNDARY              | \
     TS_LINE_BOUNDARY_AND_NEW_LINE | \
     TS_PARAGRAPH_BOUNDARY         | \
     TS_SHAPING_BOUNDARY           | \
     TS_JUSTIFICATION_POINT)

/*****************************************************************************
 *
 * Flags that are neither starting nor ending -- used when decomposing a character
 *
 */
#define TS_OTHER_FLAGS            \
    (TS_TRAILING_WHITESPACE     | \
     TS_OPENTYPE_GPOS_PROCESSED | \
     TS_INLINE_IMAGE            | \
     TS_RTL_GLYPH)


/*****************************************************************************
 *
 *  Structure for shaping and layout information related to a character
 *
 *  <GROUP shapertext>
 */
typedef struct TsTextProcessInfo_
{
    TsUInt32 character;             /* [client sets] Unicode character */
    TsIndex sourceIndex;            /* [client sets] index of char in orig source data */
    TsIndex logicalOrder;           /* [client sets (optional)] logical order after shaping, etc., but before BiDi reordering */

    TsUInt16 glyphID;               /* [Shaper sets] glyph index into font */
    TsUInt16 boundaryFlags;         /* [Shaper sets] boundaries at this character position */

    TsPoint advance;                /* [client sets after shaping] glyph advance */

    TsRect boundingBox;             /* [client sets after shaping (optional)] glyph "paint" bounding box */

    TsUInt16 flags;                 /* [for internal use by WTLE] additional flags for the glyph, defined above */

    TsContextPosition context;      /* [for internal use by Shaper] contextual information used for shaping */
    TsPositionAdjust posDelta;      /* [Shaper sets] GPOS information */

    TsBidiCharClass bidiType;       /* [for internal use by Shaper] Bidi char type */
    TsUInt8 bidiLevel;              /* [for internal use by Shaper] Bidi embedding level */
} TsTextProcessInfo;

TS_END_HEADER

#endif /* TSTXTINF_H */
