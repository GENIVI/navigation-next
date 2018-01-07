/*****************************************************************************
 *
 *  tsdlp.h  - Derived layout properties
 *
 *  Copyright (C) 2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/
#ifndef TSDLP_H
#define TSDLP_H

#include "tsconfig.h"

TS_BEGIN_HEADER

/* Derived Layout Property flag bits */
#define TS_DLP_NEEDS_BIDI               0x0001 /* char's bidi class is R, AL, RLE, or RLO                           */
#define TS_DLP_NEEDS_DPS                0x0002 /* char's general class is Mn, excluding Indic chars                 */
#define TS_DLP_NEEDS_NFC                0x0004 /* char can combine with a preceding char in NFC (incl. Hangul Jamo) */
#define TS_DLP_NEEDS_SHAPING            0x0008 /* char belongs to a script that requires shaping                    */
#define TS_DLP_HAS_TAB                  0x0010 /* char is a TAB                                                     */
#define TS_DLP_HAS_MANDATORY_LINE_BREAK 0x0020 /* char's line break class is BK, CR, LF, or NL                      */
#define TS_DLP_HAS_PARAGRAPH_SEPARATOR  0x0040 /* char's bidi class is B                                            */
#define TS_DLP_HAS_THAI                 0x0080 /* char's script is Thai                                             */
#define TS_DLP_HAS_DIACRITICS           0x0100 /* char's general class is Mn (including Indic chars)                */
#define TS_DLP_HAS_CONTROL_CODES        0x0200 /* char's general class is Cf                                        */


/*****************************************************************************
 *
 *  Returns derived layout properties of a given character.
 *
 *  Parameters:
 *      c - [in] the input character
 *
 *  Return value:
 *      TsUInt32
 *
 *  <GROUP dlp>
 */

TS_EXPORT(TsUInt32)
TsDLP_getProperties(TsUInt32 c);

TS_END_HEADER

#endif /* TSDLP_H */

