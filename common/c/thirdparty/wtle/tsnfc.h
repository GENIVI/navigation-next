/*****************************************************************************
 *
 *  tsnfc.h
 *
 *  Copyright (C) 2005, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifndef TSNFC_H
#define TSNFC_H

#include "tsconfig.h"

#ifndef TS_NO_NORMALIZATION

TS_BEGIN_HEADER

/*****************************************************************************
 *
 *  Composes a string to Normalization Form C (NFC).
 *
 *  Parameters:
 *      inputTextObj    - [in] the input text object
 *      inputStartIndex - [in] the start index for the input text
 *      inputEndIndex   - [in] the end index for the input text
 *
 *      outputTextObj    - [out] the output text object
 *      outputStartIndex - [in]  the start index for the output text
 *      outputEndIndex   - [out] the actual end index of the NFC composed text (this is an "out" only, not an "in" -- the "in" end index is (size-1) and can grow)
 *
 *      bufferForNFDObj        - [out] the buffer text object for temporary NFD storage
 *      (start index is 0, end index is (size-1) and can grow)
 *
 *  Return value:
 *      Result code -- TS_OK if all went well, error code otherwise.
 *
 *  <GROUP normalization>
 */

TS_EXPORT(TsResult)
TsNorm_composeStringToNFC(
    TsNormTextObj *inputTextObj,
    TsInt32       inputStartIndex,
    TsInt32       inputEndIndex,

    TsNormTextObj *outputTextObj,
    TsInt32       outputStartIndex,
    TsInt32       *outputEndIndex,

    TsNormTextObj *bufferForNFDObj
);

TS_END_HEADER

#endif /* TS_NO_NORMALIZATION */

#endif /* TSNFC_H */

