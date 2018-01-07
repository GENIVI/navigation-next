/*****************************************************************************
 *
 *  tstextat.h - Declares text attributes.
 *
 *  Copyright (C) 2005, 2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSTEXTAT_H
#define TSTEXTAT_H

#include "tsproj.h"
#include "tsfixed.h"
#include "ts2d.h"
#include "tstag.h"

TS_BEGIN_HEADER

/* These macros are designed for use with the types defined below.
 * See those definitions for descriptions of how to use these macros.
 */

#define TS_ALIGN_LEFT           0x00    /* left aligned */
#define TS_ALIGN_CENTER         0x01    /* center aligned */
#define TS_ALIGN_RIGHT          0x02    /* right aligned */
#define TS_ALIGN_LEFT_DYNAMIC   0x04    /* left aligned unless bidi base level of the paragraph is odd (meaning right-to-left), in which case it is right-aligned (default) */
#define TS_ALIGN_RIGHT_DYNAMIC  0x08    /* right aligned unless the bidi base level of the paragraph is even (meaning left-to-right), in which case it is left-aligned */

#define TS_ALIGN_BASELINE       0x00    /* aligned to baseline */
#define TS_ALIGN_TOP            0x10    /* aligned to top */
#define TS_ALIGN_MIDDLE         0x20    /* aligned to middle */
#define TS_ALIGN_BOTTOM         0x40    /* aligned to bottom */

#define TS_ALIGN_DEFAULT        (TS_ALIGN_LEFT | TS_ALIGN_BASELINE)

#define TS_VERTICAL_ALIGNMENT   0xF0    /* extract vertical component of bitfied alignment value */
#define TS_HORIZONTAL_ALIGNMENT 0x0F    /* extract horizontal component of bitfied alignment value */

#define TS_SINGLE_LINE          0       /* single spacing */
#define TS_150_LINE             1       /* one and a half line spacing */
#define TS_DOUBLE_LINE          2       /* double spacing */
#define TS_EXACT_LINE_SPACING   3       /* line spacing set to lineSpacingAmount */
#define TS_MULTIPLE_LINE        4       /* line spacing set to multiple of line spacing * lineSpacingAmount */



/*****************************************************************************
 *
 * Typedef for text alignment type.
 *
 * Description:
 *      Alignment types are used as a text attribute and with the string
 *      functions to determine how text is aligned on a line.
 *
 *      There are types for both horizontal and vertical alignment.
 *      The usage may restrict the types that can be used.
 *      Some uses allow a horizontal and a vertical alignment type
 *      to be logically ORed as a single argument.
 *
 *      <TABLE>
 *      <B>Horizontal Alignment Types   <B>Description
 *      TS_ALIGN_LEFT                   left aligned
 *      TS_ALIGN_CENTER                 center aligned
 *      TS_ALIGN_RIGHT                  right aligned
 *      TS_ALIGN_LEFT_DYNAMIC           left aligned unless bidi base level of the paragraph is odd
 *      TS_ALIGN_RIGHT_DYNAMIC          right aligned unless the bidi base level of the paragraph is even
 *      </TABLE>
 *
 *      <TABLE>
 *      <B>Vertical Alignment Types     <B>Description
 *      TS_ALIGN_TOP                    top aligned
 *      TS_ALIGN_MIDDLE                 middle aligned
 *      TS_ALIGN_BOTTOM                 bottom aligned
 *      TS_ALIGN_BASELINE               baseline aligned
 *      </TABLE>
 *
 * <GROUP textattrib>
 */
typedef TsUInt8 TsAlignType;


/*****************************************************************************
 *
 *  Typedef for line spacing settings.
 *
 *  Description:
 *      This type is used as part of the line spacing calculation.
 *
 *      In the table below the value 'LS' is the nominal line spacing
 *      determined from font metrics.
 *
 *      <TABLE>
 *      <B>Type                         <B>Line spacing
 *      TS_SINGLE_LINE                  1.0 * LS (single spacing)
 *      TS_150_LINE                     1.5 * LS (1.5 spacing)
 *      TS_DOUBLE_LINE                  2.0 * LS (double spacing)
 *      TS_MULTIPLE_LINE                lineSpacingAmount * LS
 *      TS_EXACT_LINE_SPACING           spacing is set equal to the line spacing amount
 *      </TABLE>
 *  <GROUP textattrib>
 */
typedef TsUInt8 TsLineSpacing;



/* Deprecated ******************************************************************************/

/* These types and values are now deprecated */

typedef TsLineStyle TsStrikeOutStyle;       /* Use TsLineStyle instead of TsStrikeOutStyle */
typedef TsLineStyle TsUnderlineStyle;       /* Use TsLineStyle instead of TsUnderlineStyle */
typedef TsTag TsLanguage;                   /* Use TsTag instead of TsLanguage */


/* Use TS_LINE macros rather than TS_UNDERLINE or TS_STRIKEOUT macros. */

#define TS_UNDERLINE_NONE           TS_LINE_NONE
#define TS_UNDERLINE_SINGLE         TS_LINE_SINGLE_SOLID
#define TS_UNDERLINE_DOUBLE         TS_LINE_DOUBLE_SOLID
#define TS_STRIKEOUT_NONE           TS_LINE_NONE
#define TS_STRIKEOUT_SINGLE         TS_LINE_SINGLE_SOLID
#define TS_STRIKEOUT_DOUBLE         TS_LINE_DOUBLE_SOLID


TS_END_HEADER

#endif /* TSTEXTAT_H */
