/*****************************************************************************
 *
 *  tsnrmtxt.h
 *
 *  Copyright (C) 2005, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 *
 ****************************************************************************/

#ifndef TSNRMTXT_H
#define TSNRMTXT_H

#include "tsproj.h"
#include "tsmem.h"


#ifndef TS_NO_NORMALIZATION

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  The normalization text object.
 *
 *  Description:
 *
 *      The TsNormText object provides an abstracted interface to the
 *      normalization code.  The user must populate the object with a
 *      pointer to text and pointers to functions which can set characters
 *      and get characters in that text.
 *
 *      <PRE>
 *
 *      text
 *          -Description:
 *              pointer to the user's normalization text data which must
 *              be capable of storing characters and (if desired) the
 *              original index of those characters in the source text
 *      memMgr
 *          -Description:
 *              memory manager object (or NULL). Used in increaseSize function.
 *      size
 *          -Description:
 *              current size of the buffer (in entries, not bytes!)
 *      growth
 *          -Description:
 *              the size (in entries) by which to grow the buffer
 *      getChar
 *          -Description:
 *              retrieves a character and (if desired) a source index
 *          -Parameters:
 *              textObj     - [in] the normalization text object
 *              index       - [in] the index from which to retrieve a char
 *              charData    - [out] the actual character
 *              sourceIndex - [out] the index of the char in the original text
 *          -Returns:
 *              none
 *      setChar
 *          -Description:
 *              stores a character and (if desired) a source index
 *          -Parameters:
 *              textObj     - [in/out] the normalization text object
 *              index       - [in] the index at which to store a char
 *              charData    - [in] the actual character
 *              sourceIndex - [in] the index of the char in the original text
 *          -Returns:
 *              none
 *      increaseSize
 *          -Description:
 *              increases the size by the number of characters specified
 *          -Parameters:
 *              textObj     - [in/out] the normalization text object
 *              charsToAdd  - [in] the number of characters by which to increase
 *
 *      </PRE>
 *
 *  <GROUP normalization>
 */

typedef struct TsNormTextObj_
{
    void *text;
    TsMemMgr *memMgr;
    TsInt32 size;
    TsInt32 growth;
    TsResult (*getChar)(struct TsNormTextObj_ *textObj, TsInt32 index, TsUInt32 *charData, TsInt32 *sourceIndex);
    TsResult (*setChar)(struct TsNormTextObj_ *textObj, TsInt32 index, TsUInt32  charData, TsInt32  sourceIndex);
    TsResult (*increaseSize)(struct TsNormTextObj_ *textObj, TsUInt32 charsToAdd);
} TsNormTextObj;

TS_END_HEADER

#endif /* TS_NO_NORMALIZATION */

#endif /* TSNRMTXT_H */

