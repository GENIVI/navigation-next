/*****************************************************************************
 *
 *  tscmb.h
 *
 *  Copyright (C) 2005, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSCMB_H
#define TSCMB_H

#include "tsconfig.h"

TS_BEGIN_HEADER

/******************************************************************************
 *
 *  Enumeration of Unicode combining class types.
 *
 *  <GROUP normalization>
 *
 */
typedef enum
{
    TS_COMB_CLASS_000,      /* Combining class   0 */
    TS_COMB_CLASS_001,      /* Combining class   1 */
    TS_COMB_CLASS_007,      /* Combining class   7 */
    TS_COMB_CLASS_008,      /* Combining class   8 */
    TS_COMB_CLASS_009,      /* Combining class   9 */
    TS_COMB_CLASS_010,      /* Combining class  10 */
    TS_COMB_CLASS_011,      /* Combining class  11 */
    TS_COMB_CLASS_012,      /* Combining class  12 */
    TS_COMB_CLASS_013,      /* Combining class  13 */
    TS_COMB_CLASS_014,      /* Combining class  14 */
    TS_COMB_CLASS_015,      /* Combining class  15 */
    TS_COMB_CLASS_016,      /* Combining class  16 */
    TS_COMB_CLASS_017,      /* Combining class  17 */
    TS_COMB_CLASS_018,      /* Combining class  18 */
    TS_COMB_CLASS_019,      /* Combining class  19 */
    TS_COMB_CLASS_020,      /* Combining class  20 */
    TS_COMB_CLASS_021,      /* Combining class  21 */
    TS_COMB_CLASS_022,      /* Combining class  22 */
    TS_COMB_CLASS_023,      /* Combining class  23 */
    TS_COMB_CLASS_024,      /* Combining class  24 */
    TS_COMB_CLASS_025,      /* Combining class  25 */
    TS_COMB_CLASS_026,      /* Combining class  26 */
    TS_COMB_CLASS_027,      /* Combining class  27 */
    TS_COMB_CLASS_028,      /* Combining class  28 */
    TS_COMB_CLASS_029,      /* Combining class  29 */
    TS_COMB_CLASS_030,      /* Combining class  30 */
    TS_COMB_CLASS_031,      /* Combining class  31 */
    TS_COMB_CLASS_032,      /* Combining class  32 */
    TS_COMB_CLASS_033,      /* Combining class  33 */
    TS_COMB_CLASS_034,      /* Combining class  34 */
    TS_COMB_CLASS_035,      /* Combining class  35 */
    TS_COMB_CLASS_036,      /* Combining class  36 */
    TS_COMB_CLASS_084,      /* Combining class  84 */
    TS_COMB_CLASS_091,      /* Combining class  91 */
    TS_COMB_CLASS_103,      /* Combining class 103 */
    TS_COMB_CLASS_107,      /* Combining class 107 */
    TS_COMB_CLASS_118,      /* Combining class 118 */
    TS_COMB_CLASS_122,      /* Combining class 122 */
    TS_COMB_CLASS_129,      /* Combining class 129 */
    TS_COMB_CLASS_130,      /* Combining class 130 */
    TS_COMB_CLASS_132,      /* Combining class 132 */
    TS_COMB_CLASS_202,      /* Combining class 202 */
    TS_COMB_CLASS_214,      /* Combining class 214 */
    TS_COMB_CLASS_216,      /* Combining class 216 */
    TS_COMB_CLASS_218,      /* Combining class 218 */
    TS_COMB_CLASS_220,      /* Combining class 220 */
    TS_COMB_CLASS_222,      /* Combining class 222 */
    TS_COMB_CLASS_224,      /* Combining class 224 */
    TS_COMB_CLASS_226,      /* Combining class 226 */
    TS_COMB_CLASS_228,      /* Combining class 228 */
    TS_COMB_CLASS_230,      /* Combining class 230 */
    TS_COMB_CLASS_232,      /* Combining class 232 */
    TS_COMB_CLASS_233,      /* Combining class 233 */
    TS_COMB_CLASS_234,      /* Combining class 234 */
    TS_COMB_CLASS_240      /* Combining class 240 */
} TsCombClassType;


/*****************************************************************************
 *
 *  Gets the combining class of a given character.
 *
 *  Parameters:
 *      inputChar  - [in] character for which to get the combining class
 *
 *  Return value:
 *      The combining class of the character.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsCombClassType)
TsNorm_getCombiningClass(TsUInt32 inputChar);


#ifndef TS_NO_NORMALIZATION

/*****************************************************************************
 *
 *  Canonically reorders a run of combining marks.
 *
 *  Parameters:
 *      inputTextObj    - [in] text object containing the run
 *      inputStartIndex - [in] start index of the run of combining marks
 *      inputEndIndex   - [in] end index of the run of combining marks
 *
 *  Return value:
 *      result code -- TS_OK if all went well, error code otherwise.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsResult)
TsNorm_canonicallyReorderCombiningMarks(
    TsNormTextObj *inputTextObj,
    TsInt32       inputStartIndex,
    TsInt32       inputEndIndex
);


/*****************************************************************************
 *
 *  Finds the end index of a run of combining marks.
 *
 *  Parameters:
 *      inputTextObj  - [in] text object containing the run
 *      runStartIndex - [in] start index of the run of combining marks
 *      inputEndIndex - [in] end index of run of text to search
 *      runEndIndex   - [out] the index of the end of the run of combining marks.
 *
 *  Return value:
 *      result code -- TS_OK if all went well, error code otherwise.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsResult)
TsNorm_findEndOfRunOfCombiningMarks(
    TsNormTextObj *inputTextObj,
    TsInt32       runStartIndex,
    TsInt32       inputEndIndex,
    TsInt32       *runEndIndex
);


/*****************************************************************************
 *
 *  Finds the end index of a run of combining marks and combining letters
 *  (combining letters are letters that can combine with other letters to
 *   form composed letters).
 *
 *  Parameters:
 *      inputTextObj  - [in] text object containing the run
 *      runStartIndex - [in] start index of the run of combining marks
 *      inputEndIndex - [in] end index of run of text to search
 *      runEndIndex   - [out] the index of the end of the run of combining marks and combining letters.
 *
 *  Return value:
 *      result code -- TS_OK if all went well, error code otherwise.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsResult)
TsNorm_findEndOfRunOfCombiningLettersAndMarks(
    TsNormTextObj *inputTextObj,
    TsInt32       runStartIndex,
    TsInt32       inputEndIndex,
    TsInt32      *runEndIndex
);


/*****************************************************************************
 *
 *  Determines if a character is a combining letter
 *  (combining letters are letters that can combine with other letters to
 *   form composed letters).
 *
 *  Parameters:
 *      inputChar - [in] character to check
 *
 *  Return value:
 *      TRUE if the character is a combining letter.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsBool)
TsNorm_isCombiningLetter(TsUInt32 inputChar);


/*****************************************************************************
 *
 *  Determines if a character is composition excluded
 *  (i.e. the character is excluded from composition).
 *
 *  Parameters:
 *      inputChar - [in] character to check
 *
 *  Return value:
 *      TRUE if the character is composition excluded.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsBool)
TsNorm_isCompositionExcluded(TsUInt32 inputChar);


/*****************************************************************************
 *
 *  Returns all of the combining data in one function call
 *  (i.e. combining class, is combining letter?, is composition excluded?).
 *
 *  Parameters:
 *      inputChar              - [in]  character to check
 *      combiningClass         - [out] combining class
 *      isCombiningLetter      - [out] TRUE if it is a combining letter
 *      isCompositionExcluded  - [out] TRUE if it is composition excluded
 *
 *  Return value:
 *      none
 *
 *  <GROUP normalization>
 */
TS_EXPORT(void)
TsNorm_getCombiningData(TsUInt32 inputChar, TsCombClassType *combiningClass, TsBool *isCombiningLetter, TsBool *isCompositionExcluded);


TS_END_HEADER

#endif /* TS_NO_NORMALIZATION */

#endif /* TSCMB_H */

