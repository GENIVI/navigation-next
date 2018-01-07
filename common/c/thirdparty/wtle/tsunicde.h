/*****************************************************************************
 *
 *  tsunicde.c - Declares Unicode properties interface.
 *
 *  Copyright (C) 2002-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSUNICDE_H
#define TSUNICDE_H

#include "tstypes.h"

TS_BEGIN_HEADER

/* Several Unicode characters */
#define TS_CHAR_TAB                         0x0009  /* CHARACTER TABULATION        */
#define TS_CHAR_LF                          0x000A  /* LINE FEED                   */
#define TS_CHAR_CR                          0x000D  /* CARRIAGE RETURN             */
#define TS_CHAR_INFORMATION_SEPARATOR_FOUR  0x001C  /* INFORMATION SEPARATOR FOUR  */
#define TS_CHAR_INFORMATION_SEPARATOR_THREE 0x001D  /* INFORMATION SEPARATOR THREE */
#define TS_CHAR_INFORMATION_SEPARATOR_TWO   0x001E  /* INFORMATION SEPARATOR TWO   */
#define TS_CHAR_SPACE                       0x0020  /* SPACE                       */
#define TS_CHAR_FULL_STOP                   0x002E  /* FULL STOP                   */
#define TS_CHAR_SOLIDUS                     0x002F  /* SOLIDUS                     */
#define TS_CHAR_NEXT_LINE                   0x0085  /* NEXT LINE (NEL)             */
#define TS_CHAR_NO_BREAK_SPACE              0x00A0  /* NO-BREAK SPACE              */
#define TS_CHAR_SECTION_SIGN                0x00A7  /* SECTION SIGN                */
#define TS_CHAR_SOFT_HYPHEN                 0x00AD  /* SOFT HYPHEN                 */
#define TS_CHAR_MIDDLE_DOT                  0x00B7  /* MIDDLE DOT                  */
#define TS_CHAR_DEVANAGARI_LETTER_RA        0x0930  /* DEVANAGARI LETTER RA        */
#define TS_CHAR_DEVANAGARI_LETTER_RRA       0x0931  /* DEVANAGARI LETTER RRA       */
#define TS_CHAR_DEVANAGARI_SIGN_HALANT      0x094D  /* DEVANAGARI SIGN VIRAMA      */
#define TS_CHAR_DEVANAGARI_DANDA            0x0964  /* DEVANAGARI DANDA            */
#define TS_CHAR_DEVANAGARI_DOUBLE_DANDA     0x0965  /* DEVANAGARI DOUBLE DANDA     */
#define TS_CHAR_THAI_VOWEL_SIGN_SARA_AA     0x0E32  /* THAI VOWEL SIGN SARA AA     */
#define TS_CHAR_THAI_VOWEL_SIGN_SARA_AM     0x0E33  /* THAI VOWEL SIGN SARA AM     */
#define TS_CHAR_THAI_NIKKHAHIT              0x0E4D  /* THAI NIKKHAHIT              */
#define TS_CHAR_ZWNJ                        0x200C  /* ZERO WIDTH NON_JOINER       */
#define TS_CHAR_ZWJ                         0x200D  /* ZERO WIDTH JOINER           */
#define TS_CHAR_HORIZONTAL_ELLIPSIS         0x2026  /* HORIZONTAL ELLIPSIS         */
#define TS_CHAR_LINE_SEPARATOR              0x2028  /* LINE SEPARATOR              */
#define TS_CHAR_PARAGRAPH_SEPARATOR         0x2029  /* PARAGRAPH SEPARATOR         */
#define TS_CHAR_DOTTED_CIRCLE               0x25CC  /* DOTTED CIRCLE               */
#define TS_CHAR_POSTAL_MARK                 0x3012  /* POSTAL MARK                 */
#define TS_CHAR_POSTAL_MARK_FACE            0x3020  /* POSTAL MARK FACE            */
#define TS_CHAR_CIRCLED_POSTAL_MARK         0x3036  /* CIRCLED POSTAL MARK         */
#define TS_CHAR_BYTE_ORDER_MARK             0xFEFF  /* BYTE ORDER MARK             */

#define TS_UNICODE_MAX_VALUE 0x010FFFF


/******************************************************************************
 *
 *  Enumeration of general category values.
 *
 *  Notes:
 *      Nothing in Unicode requires the particular values defined below,
 *      but WTLE requires that the values be defined explicitly.
 *      This is done so they match the specific values encoded
 *      in the Unicode general character properties data file.
 *      That properties data is generated using an internal Python tool
 *      which defines the values used for each general category.
 *
 *  <GROUP uniprops>
 *
 */
typedef enum
{
    TS_GC_OTHER_NOT_ASSIGNED        = 0,    /* Cn  */
    TS_GC_LETTER_UPPERCASE          = 1,    /* Lu  */
    TS_GC_LETTER_LOWERCASE          = 2,    /* Ll  */
    TS_GC_LETTER_TITLECASE          = 3,    /* Lt  */
    TS_GC_LETTER_MODIFIER           = 4,    /* Lm  */
    TS_GC_LETTER_OTHER              = 5,    /* Lo  */
    TS_GC_MARK_NON_SPACING          = 6,    /* Mn  */
    TS_GC_MARK_SPACING_COMBINING    = 7,    /* Mc  */
    TS_GC_MARK_ENCLOSING            = 8,    /* Me  */
    TS_GC_NUMBER_DECIMAL_DIGIT      = 9,    /* Nd  */
    TS_GC_NUMBER_LETTER             = 10,   /* Nl  */
    TS_GC_NUMBER_OTHER              = 11,   /* No  */
    TS_GC_PUNCTUATION_CONNECTOR     = 12,   /* Pc  */
    TS_GC_PUNCTUATION_DASH          = 13,   /* Pd  */
    TS_GC_PUNCTUATION_OPEN          = 14,   /* Ps  */
    TS_GC_PUNCTUATION_CLOSE         = 15,   /* Pe  */
    TS_GC_PUNCTUATION_INITIAL_QUOTE = 16,   /* Pi  */
    TS_GC_PUNCTUATION_FINAL_QUOTE   = 17,   /* Pf  */
    TS_GC_PUNCTUATION_OTHER         = 18,   /* Po  */
    TS_GC_SYMBOL_MATH               = 19,   /* Sm  */
    TS_GC_SYMBOL_CURRENCY           = 20,   /* Sc  */
    TS_GC_SYMBOL_MODIFIER           = 21,   /* Sk  */
    TS_GC_SYMBOL_OTHER              = 22,   /* So  */
    TS_GC_SEPARATOR_SPACE           = 23,   /* Zs  */
    TS_GC_SEPARATOR_LINE            = 24,   /* Zl  */
    TS_GC_SEPARATOR_PARAGRAPH       = 25,   /* Zp  */
    TS_GC_OTHER_CONTROL             = 26,   /* Cc  */
    TS_GC_OTHER_FORMAT              = 27,   /* Cf  */
    TS_GC_OTHER_SURROGATE           = 28,   /* Cs  */
    TS_GC_OTHER_PRIVATE_USE         = 29    /* Co  */
} TsGeneralCategory;


/******************************************************************************
 *
 *  Enumeration of line break types.
 *
 *  Notes:
 *      Nothing in Unicode requires the particular values defined below,
 *      but WTLE requires that the values be defined explicitly.
 *      This is done so they match the specific values encoded
 *      in the Unicode general character properties data file.
 *      That properties data is generated using an internal Python tool
 *      which defines the values used for each line break type.
 *
 *  <GROUP uniprops>
 *
 */
typedef enum
{
    TS_LB_AI = 0,    /* AMBIGUOUS           -- AI act like AL when the resolved EAW is N otherwise act as ID */
    TS_LB_AL = 1,    /* ALPHABETIC          -- are alphabetic characters or symbols that are used with alphabetic characters */
    TS_LB_B2 = 2,    /* BREAK_BOTH          -- provide a line break opportunity before and after the character */
    TS_LB_BA = 3,    /* BREAK_AFTER         -- generally provide a line break opportunity after the character */
    TS_LB_BB = 4,    /* BREAK_BEFORE        -- generally provide a line break opportunity before the character */
    TS_LB_BK = 5,    /* MANDATORY_BREAK     -- cause a line break (after) */
    TS_LB_CB = 6,    /* CONTINGENT_BREAK    -- provide a line break opportunity contingent on additional information */
    TS_LB_CL = 7,    /* CLOSE_PUNCTUATION   -- prohibit a line break before */
    TS_LB_CM = 8,    /* COMBINING_MARK      -- prohibit a line break between the character and the preceding character */
    TS_LB_CR = 9,    /* CARRIAGE_RETURN     -- cause a line break (after), except between CR and LF */
    TS_LB_EX = 10,   /* EXCLAMATION         -- prohibit line break before */
    TS_LB_GL = 11,   /* GLUE                -- prohibit line breaks before or after */
    TS_LB_HY = 12,   /* HYPHEN              -- provide a line break opportunity after the character, except in numeric context */
    TS_LB_ID = 13,   /* IDEOGRAPHIC         -- break before or after, except in some numeric context */
    TS_LB_IN = 14,   /* INSEPERABLE         -- allow only indirect line breaks between pairs */
    TS_LB_IS = 15,   /* INFIX_NUMERIC       -- prevent breaks after any and before numeric */
    TS_LB_LF = 16,   /* LINE_FEED           -- cause a line break (after) */
    TS_LB_NL = 17,   /* NEXT_LINE           -- cause a line break (after) */
    TS_LB_NS = 18,   /* NONSTARTER          -- (e.g. small kana) allow only indirect line break before */
    TS_LB_NU = 19,   /* NUMERIC             -- form numeric expressions for line breaking purposes */
    TS_LB_OP = 20,   /* OPEN_PUNCTUATION    -- prohibit a line break after */
    TS_LB_PO = 21,   /* POSTFIX_NUMERIC     -- do not break following a numeric expression */
    TS_LB_PR = 22,   /* PREFIX_NUMERIC      -- don't break in front of a numeric expression */
    TS_LB_QU = 23,   /* QUOTATION           -- act like they are both opening and closing */
    TS_LB_SA = 24,   /* COMPLEX_CONTEXT     -- provide a line break opportunity contingent on additional, language specific context analysis */
    TS_LB_SG = 25,   /* SURROGATE           -- should not occur in well-formed text */
    TS_LB_SP = 26,   /* SPACE               -- generally provide a line break opportunity after the character, enable indirect breaks */
    TS_LB_SY = 27,   /* BREAK_SYMBOLS       -- prevent a break before, and allow a break after */
    TS_LB_WJ = 28,   /* WORD_JOINER         -- prohibit line breaks before or after */
    TS_LB_XX = 29,   /* UNKNOWN             -- are all characters with (as yet) unknown line breaking behavior or unassigned code positions */
    TS_LB_ZW = 30,   /* ZWSPACE             -- provide a break opportunity */
    TS_LB_H2 = 31,   /* HANGUL_LV_SYLLABLE  -- all characters of Hangul Syllable Type LV */
    TS_LB_H3 = 32,   /* HANGUL_LVT_SYLLABLE -- all characters of Hangul Syllable Type LVT */
    TS_LB_JL = 33,   /* HANGUL_L_JAMO       -- Hangul L Jamo */
    TS_LB_JV = 34,   /* HANGUL_V_JAMO       -- Hangul V Jamo */
    TS_LB_JT = 35    /* HANGUL_T_JAMO       -- Hangul T Jamo */
} TsLineBreakType;


/******************************************************************************
 *
 *  Enumeration of word break types
 *
 *  <GROUP uniprops>
 *
 */
typedef enum
{
    TS_WB_A_LETTER       =  0,    /* ALetter */
    TS_WB_CR             =  1,    /* CR */
    TS_WB_EXTEND         =  2,    /* Extend */
    TS_WB_EXTEND_NUM_LET =  3,    /* ExtendNumLet */
    TS_WB_FORMAT         =  4,    /* Format */
    TS_WB_KATAKANA       =  5,    /* Katakana */
    TS_WB_LF             =  6,    /* LF */
    TS_WB_MID_LETTER     =  7,    /* MidLetter */
    TS_WB_MID_NUM        =  8,    /* MidNum */
    TS_WB_MID_NUM_LET    =  9,    /* MidNumLet */
    TS_WB_NEWLINE        = 10,    /* Newline */
    TS_WB_NUMERIC        = 11,    /* Numeric */
    TS_WB_OTHER          = 12     /* Other */
} TsWordBreakType;


/******************************************************************************
 *
 *  Enumeration of grapheme property values.
 *
 *  <GROUP uniprops>
 *
 */
typedef enum
{
    TS_GB_CR           =  0, /* CR          */
    TS_GB_LF           =  1, /* LF          */
    TS_GB_CONTROL      =  2, /* Control     */
    TS_GB_EXTEND       =  3, /* Extend      */
    TS_GB_PREPEND      =  4, /* Prepend     */
    TS_GB_SPACING_MARK =  5, /* SpacingMark */
    TS_GB_L            =  6, /* L           */
    TS_GB_V            =  7, /* V           */
    TS_GB_T            =  8, /* T           */
    TS_GB_LV           =  9, /* LV          */
    TS_GB_LVT          = 10, /* LVT         */
    TS_GB_OTHER        = 11  /* Other       */
} TsGraphemeType;



/******************************************************************************
 *
 *  Enumeration of joining types.
 *
 *  <GROUP uniprops>
 *
 */
typedef enum
{
    TS_JT_NON_JOINING       = 0,    /* Does not have a joining form */
    TS_JT_JOIN_CAUSING      = 1,    /* ZWJ and tatweel (kashida) */
    TS_JT_DUAL_JOINING      = 2,    /* Joins on either side */
    TS_JT_LEFT_JOINING      = 3,    /* Joins to the following character */
    TS_JT_RIGHT_JOINING     = 4,    /* Joins to the preceding character */
    TS_JT_TRANSPARENT       = 5     /* Has no effect on joining */
} TsJoiningType;


/******************************************************************************
 *
 *  Enumeration of BiDi character classes.
 *
 *  <GROUP uniprops>
 *
 */
typedef enum
{
    TS_BIDI_L    = 0,     /* LEFT_TO_RIGHT              */
    TS_BIDI_LRE  = 1,     /* LEFT_TO_RIGHT_EMBEDDING    */
    TS_BIDI_LRO  = 2,     /* LEFT_TO_RIGHT_OVERRIDE     */
    TS_BIDI_R    = 3,     /* RIGHT_TO_LEFT              */
    TS_BIDI_AL   = 4,     /* RIGHT_TO_LEFT_ARABIC       */
    TS_BIDI_RLE  = 5,     /* RIGHT_TO_LEFT_EMBEDDING    */
    TS_BIDI_RLO  = 6,     /* RIGHT_TO_LEFT_OVERRIDE     */
    TS_BIDI_PDF  = 7,     /* POP_DIRECTIONAL_FORMAT     */
    TS_BIDI_EN   = 8,     /* EUROPEAN_NUMBER            */
    TS_BIDI_ES   = 9,     /* EUROPEAN_NUMBER_SEPARATOR  */
    TS_BIDI_ET   = 10,    /* EUROPEAN_NUMBER_TERMINATOR */
    TS_BIDI_AN   = 11,    /* ARABIC_NUMBER              */
    TS_BIDI_CS   = 12,    /* COMMON_NUMBER_SEPARATOR    */
    TS_BIDI_NSM  = 13,    /* DIR_NON_SPACING_MARK       */
    TS_BIDI_BN   = 14,    /* BOUNDARY_NEUTRAL           */
    TS_BIDI_B    = 15,    /* PARAGRAPH_SEPARATOR        */
    TS_BIDI_S    = 16,    /* SEGMENT_SEPARATOR          */
    TS_BIDI_WS   = 17,    /* WHITE_SPACE_NEUTRAL        */
    TS_BIDI_ON   = 18     /* OTHER_NEUTRAL              */
} TsBidiCharClass;


/*****************************************************************************
 *
 *  Gets the particular Bidi character type.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsBidiCharDirection result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBidiCharClass)
TsUnicode_getBidiCharType(TsUInt32 c);


/*****************************************************************************
 *
 *  Returns the type of joining character (Unicode table 8-2).
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsJoiningType result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsJoiningType)
TsUnicode_getJoiningType(TsUInt32 c);


/*****************************************************************************
 *
 *  Checks if the Unicode character is mirrored.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBool)
TsUnicode_isMirrored(TsUInt32 c);


/*****************************************************************************
 *
 *  Return bidi mirror for a character.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsUInt32 result (input character if no mirror).
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsUInt32)
TsUnicode_getCharMirror(TsUInt32 c);


/*****************************************************************************
 *
 *  Returns the general category.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsGeneralCategory result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsGeneralCategory)
TsUnicode_getGeneralCategory(TsUInt32 c);


/*****************************************************************************
 *
 *  Returns the line break type.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsLineBreakType result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsLineBreakType)
TsUnicode_getLineBreakType(TsUInt32 c);


/*****************************************************************************
 *
 *  Returns word break type.
 *
 *  Parameters:
 *      c               - [in] Unicode character
 *
 *  Return value:
 *      TsWordBreakType
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsWordBreakType)
TsUnicode_getWordBreakType(TsUInt32 c);


/*****************************************************************************
 *
 *  Returns grapheme property.
 *
 *  Parameters:
 *      c               - [in] Unicode character
 *
 *  Return value:
 *      TsGraphemeType
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsGraphemeType)
TsUnicode_getGraphemeType(TsUInt32 c);


/*****************************************************************************
 *
 *  Checks if the Unicode character is a combining mark.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBool)
TsUnicode_isCombiningMark(TsUInt32 c);


/*****************************************************************************
 *
 *  Checks if the Unicode character is non-spacing.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBool)
TsUnicode_isNonSpacing(TsUInt32 c);


/*****************************************************************************
 *
 *  Checks if the Unicode character is printable.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBool)
TsUnicode_isPrint(TsUInt32 c);


/*****************************************************************************
 *
 *  Checks if the Unicode character forces the line to end.
 *
 *  Parameters:
 *      c                    - [in] Unicode character to check
 *      isOnGraphemeBoundary - [in] True if c is has its grapheme boundary flag set
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBool)
TsUnicode_isRequiredLineEndChar(TsUInt32 c, TsBool isOnGraphemeBoundary);


/*****************************************************************************
 *
 *  Checks if the Unicode character is a whitespace character.
 *
 *  Description:
 *      A character is considered whitespace if:
 *      its general category is Zs, Zl, Zp except for U+00a0 and U+202f OR
 *      U+0009 <= c <= U+000d OR
 *      U+001C <= c <= U+001f OR
 *      c == U+0085.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(TsBool)
TsUnicode_isWhitespace(TsUInt32 c);


/*****************************************************************************
 *
 *  Macro that checks if the Unicode character is a TS_CHAR_NO_BREAK_SPACE or
 *  a TS_CHAR_SPACE.
 *
 *  Parameters:
 *      c                    - [in] Unicode character to check
 *
 *  Return value:
 *      TsBool result.
 *
 *  <GROUP uniprops>
 */
#define TsUnicode_isBaseSpace(c) ((c == TS_CHAR_NO_BREAK_SPACE) || (c == TS_CHAR_SPACE))


/*****************************************************************************
 *
 *  Returns non-spacing & is-printable information
 *  for use by Diacritics Positioning System.
 *
 *  Description:
 *     Same as TsUnicode_isNonSpacing() and TsUnicode_isPrint(),
 *     but wrappe into a single method.
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *      isNonSpacing    - [out] TRUE if the Unicode character is non-spacing
 *      isPrint         - [out] TRUE if the Unicode character is printable
 *
 *  Return value:
 *      void
 *
 *  <GROUP uniprops>
 */
TS_EXPORT(void)
TsUnicode_getDPSInfo(TsUInt32 c, TsBool *isNonSpacing, TsBool *isPrint);

/*****************************************************************************
 *
 *  Orientations used by WTLE's Diacritics Positioning System.
 *
 *  The orientation refers to how the bounding box of the diacritic glyph
 *  should be positioned relative to the bounding box of the base glyph.
 *
 *  <GROUP uniprops>
 */
typedef enum
{
    MARK_ABOVE_CENTERED_OVER_BASE,
    MARK_ABOVE_CENTERED_OVER_BASE_ATTACHED,
    MARK_ABOVE_CENTERED_OVER_BASE_ALIGNED_AND_TOP_ALIGNED,
    MARK_ABOVE_LEFT_OF_RIGHT_EDGE_BY_ONE_THIRD_AND_TOP_ALIGNED,
    MARK_ABOVE_LEFT_OF_RIGHT_EDGE_BY_ONE_THIRD_AND_ONE_THIRD_ABOVE_EDGE,
    MARK_ABOVE_ALIGNED_WITH_RIGHT_OF_HIGHEST_MARK,
    MARK_ABOVE_LEFT_ALIGNED,  /* i.e. left edge of mark aligned with left edge of base */
    MARK_ABOVE_RIGHT_ALIGNED, /* i.e. right edge of mark aligned with right edge of base */
    MARK_ABOVE_RIGHT_ATTACHED,
    MARK_ABOVE_RIGHT_CENTERED_OVER_RIGHT_EDGE,
    MARK_ABOVE_RIGHT_JUTTING_OUT_ONE_THIRD, /* i.e. one third of its width is right of the base */
    MARK_ABOVE_NO_HORIZONTAL_ADJUSTMENT,
    MARK_ABOVE_LEFT_OF_RIGHT_EDGE_BY_ONE_THIRD_ATTACHED,
    MARK_BELOW_CENTERED_UNDER_BASE,
    MARK_BELOW_CENTERED_UNDER_BASE_ATTACHED,
    MARK_BELOW_LEFT_ALIGNED,
    MARK_BELOW_RIGHT_ALIGNED,
    MARK_BELOW_RIGHT_ALIGNED_ATTACHED,
    MARK_CENTERED,
    MARK_HORIZONTALLY_CENTERED, /* i.e. no vertical adjustment */
    MARK_LEFT,
    MARK_RIGHT
} TsMarkOrientation;

/*****************************************************************************
 *
 *  Returns information for use by WTLE's Diacritics Positioning System
 *
 *  Description:
 *
 *  Parameters:
 *      c               - [in] Unicode character to check
 *
 *  Return value:
 *      TsMarkOrientation
 *
 *  <GROUP uniprops>
 */

TS_EXPORT(TsMarkOrientation)
TsUnicode_getMarkOrientation(TsUInt32 c);

TS_END_HEADER

#endif /* TSUNICDE_H */

