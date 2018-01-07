/*****************************************************************************
 *
 *  tsglypos.h - Defines some glyph position and contextual utilities.
 *
 *  Copyright (C) 2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSGLYPOS_H
#define TSGLYPOS_H

#include "tstypes.h"

TS_BEGIN_HEADER

/*****************************************************************************
 *
 *   NOP (or non-existent Unicode character) in following definition is
 *   to denote that the Word Block arrays have been expanded due to a GSUB
 *   Multiple lookup type, glyph IDs were added, but there is no direct Unicode
 *   character associated with the added Glyph ID.
 *
 *  <GROUP shapertext>
 */
#define TsUnicode_NOP 0

/*****************************************************************************
 *
 *  Glyph index used when no glyph could be found for a character.
 *
 *  <GROUP shapertext>
 */
#define TsGlyphID_MissingGlyph 0x0000

/*****************************************************************************
 *
 *  NOP (or deleted glyph) in following definition is to denote that
 *  a glyph has been forgone. We used to use 0x0001 but have seen some
 *  fonts that use this, so now use 0xFFFE
 *
 *  <GROUP shapertext>
 */
#define TsGlyphID_NOP 0xFFFE



/*****************************************************************************
 *
 *  Possible contexts of a character -- used internally in shaping process
 *
 *  <GROUP shapertext>
 */
typedef enum
{
    /* initial value, no specific context */
    TS_CONTEXT_NONE,

    /* Arabic & N'Ko script */
    TS_ISOLATED,
    TS_INITIAL,
    TS_MEDIAL,
    TS_FINAL,

    /* -------------------- Begin Indic Scripts Section -------------------- */

    /* Consonants */
    TS_INDIC_CONTEXT_C_REPH,
    TS_INDIC_CONTEXT_C_VATTU,
    TS_INDIC_CONTEXT_C_RAKAR,
    TS_INDIC_CONTEXT_C_BASE,
    TS_INDIC_CONTEXT_C_PRE,
    TS_INDIC_CONTEXT_C_POST,
    TS_INDIC_CONTEXT_C_POST_HALANT,  /* i.e. base consonant's halant moved after a post-base consonant */
    TS_INDIC_CONTEXT_C_BELOW,
    TS_INDIC_CONTEXT_C_BELOW_HALANT, /* i.e. base consonant's halant moved after a below-base consonant */

    /* Matras */
    TS_INDIC_CONTEXT_M_ABOVE,
    TS_INDIC_CONTEXT_M_BELOW,
    TS_INDIC_CONTEXT_M_POST,
    TS_INDIC_CONTEXT_M_PRE,
    TS_INDIC_CONTEXT_M_PRE_INIT, /* Note: This only seems to be needed for \u09C7 and \u09C8. */

    /* Length Marks */
    TS_INDIC_CONTEXT_LM_ABOVE,
    TS_INDIC_CONTEXT_LM_BELOW,
    TS_INDIC_CONTEXT_LM_POST,

    /* Stress Marks (e.g. Udatta, Anudatta, Acute, Grave) */
    TS_INDIC_CONTEXT_SM_ABOVE,
    TS_INDIC_CONTEXT_SM_BELOW,

    /* Vowel Modifiers */
    TS_INDIC_CONTEXT_VM_ABOVE,
    TS_INDIC_CONTEXT_VM_BELOW,
    TS_INDIC_CONTEXT_VM_POST,

    /* Independent Vowels */
    TS_INDIC_CONTEXT_VO,

    /* Special Cases */
    TS_INDIC_CONTEXT_POST_RO /* used for Khmer 'cfar' (Conjunct Form After Ro) */

    /* -------------------- End Indic Scripts Section -------------------- */

} TsContextPosition;


enum
{
    TS_POSADJ_NONE = 0,
    TS_POSADJ_COMBINING, /* mark to base, mark, or ligature */
    TS_POSADJ_CONNECTED, /* cursive attachment */
    TS_POSADJ_LIGACARET  /* ligature caret */
};


/*****************************************************************************
 *
 *  Position adjustment type -- used to indicate if a GPOS position adjustment
 *  of a glyph is relative to another glyph, and if so, the relationship between
 *  the glyphs.  Possible types are:
 *      TS_POSADJ_NONE - not relative to another glyph
 *      TS_POSADJ_COMBINING - mark to base, mark, or ligature
 *      TS_POSADJ_CONNECTED - cursive attachment
 *      TS_POSADJ_LIGACARET - ligature caret
 *
 *  Note that there can be a position adjust even in the case of TS_POSADJ_NONE,
 *  the NONE just means that it is not relative to another glyph.
 *
 *  <GROUP shapertext>
 */
typedef TsInt8 TsPAType;

/*****************************************************************************
 *
 *  Position adjustment coordinate (16 bit signed integer)
 *
 *  <GROUP shapertext>
 */
typedef TsInt16 TsPACoord;

/*****************************************************************************
 *
 *  Position adjustments for placement & advance of a glyph
 *
 *  <GROUP shapertext>
 */
typedef struct
{
    TsPACoord   xPlacement;      /* x placement adjustments if found in GPOS */
    TsPACoord   yPlacement;      /* y placement adjustments if found in GPOS */
    TsPACoord   xAdvance;        /* x advance adjustments if found in GPOS */
    TsPACoord   yAdvance;        /* y advance adjustments if found in GPOS */
} TsPAPlace;

/*****************************************************************************
 *
 *  Position adjustment -- ligature caret positioning information
 *
 *  <GROUP shapertext>
 */
typedef struct
{
    TsPACoord   xCaretLTR;          /* x caret position if defined in GDEF */
    TsPACoord   yCaretLTR;          /* y caret position if defined in GDEF */
    TsPACoord   xCaretRTL;          /* x caret position if defined in GDEF */
    TsPACoord   yCaretRTL;          /* y caret position if defined in GDEF */
} TsPACaret;

/*****************************************************************************
 *
 *  Position adjustment structure -- see TsPAType, TsPAPlace, & TsPACaret
 *
 *  <GROUP shapertext>
 */
typedef struct TsPositionAdjust_
{
    TsPAType indexType;
    TsInt8   index;
    union
    {
        TsPAPlace place;
        TsPACaret caret;
    } u;
} TsPositionAdjust;



/*****************************************************************************
 *
 *  Set TsPositionAdjust object positioning values to default (zero).
 *
 *  Parameters:
 *      pa        - TsPositionAdjust pointer
 *
 *  Return value:
 *      None
 *
 *  Notes:
 *      This function has been macro-ized to improve performance. Doing
 *      direct assignments was found to be faster than calling memset.
 *      This macro evaluates the pac argument more than once so
 *      the argument should never be an expression with side effects.
 *
 *      This macro is equivalent to this functional form:
 *      <CODE>
 *          TS_EXPORT(void)
 *          TsPositionAdjust_defaultPos(TsPositionAdjust *adj);
 *      </CODE>
 *
 *  <GROUP shapertext>
 */
#define TsPositionAdjust_defaultPos(pa) (pa)->indexType = 0; (pa)->index = 0;                       \
                                        (pa)->u.place.xPlacement = 0; (pa)->u.place.yPlacement = 0; \
                                        (pa)->u.place.xAdvance = 0; (pa)->u.place.yAdvance = 0;


TS_END_HEADER

#endif /* TSGLYPOS_H */
