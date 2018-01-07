/*****************************************************************************
 *
 *  tsnfd.h
 *
 *  Copyright (C) 2005, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 *
 ****************************************************************************/

#ifndef TSNFD_H
#define TSNFD_H

#include "tsconfig.h"
#include "tsnrmtxt.h"

#ifndef TS_NO_NORMALIZATION

TS_BEGIN_HEADER

/*****************************************************************************
 *
 *  Decomposes a string to Normalization Form D (NFD).
 *
 *  Parameters:
 *      inputTextObj    - [in] the input text object
 *      inputStartIndex - [in] the start index for the input text
 *      inputEndIndex   - [in] the end index for the input text
 *
 *      outputTextObj    - [out] the output text object
 *      outputStartIndex - [in]  the start index for the output text
 *      outputEndIndex   - [out] the actual end index of the NFD decomposed text (this is an "out" only, not an "in" -- the "in" end index is (size-1) and can grow)
 *
 *  Return value:
 *      Result code -- TS_OK if all went well, error code otherwise.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsResult)
TsNorm_decomposeStringToNFD(
    TsNormTextObj *inputTextObj,
    TsInt32       inputStartIndex,
    TsInt32       inputEndIndex,
    TsNormTextObj *outputTextObj,
    TsInt32       outputStartIndex,
    TsInt32       *outputEndIndex
);


/*****************************************************************************
 *
 *  Decomposes a composite character into two component characters.
 *
 *  Parameters:
 *      inputChar       - [in] the input character to decompose
 *      pDecompChar1    - [out] the first of two decomposed characters
 *      pDecompChar2    - [out] the second of two decomposed characters
 *
 *  Return value:
 *      TRUE if character decomposed, FALSE otherwise.
 *
 *  <GROUP normalization>
 */
TS_EXPORT(TsBool)
TsNorm_decomposeChar(TsUInt32 inputChar, TsUInt32 * pDecompChar1, TsUInt32 * pDecompChar2);

TS_END_HEADER

#endif /* TS_NO_NORMALIZATION */

#endif /* TSNFD_H */

