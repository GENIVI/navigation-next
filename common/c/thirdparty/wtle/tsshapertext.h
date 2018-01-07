/*****************************************************************************
 *
 *  tsshapertext.h  - Defines the interface to the TsShaperText class.
 *
 *  Copyright (C) 2005-2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSSHAPERTEXT_H
#define TSSHAPERTEXT_H

#include "tsmem.h"
#include "tstxtinf.h"

TS_BEGIN_HEADER

/*****************************************************************************
 *
 *  Shaper text object
 *
 *  Description:
 *      Holds all of the text-related data used in the shaping process.
 *
 *  <GROUP shapertext>
 */
typedef struct TsShaperText_
{
    /* public data - client reads/writes  */
    TsTextProcessInfo *textInfoArray; /* contains characters, glyph indices, boundary flags, etc. (see TsTextProcessInfo for more info.) */
    TsLength textInfoArrayLen;        /* the number of characters that are stored in textInfoArray */

    /* public data - client only reads */
    TsLength capacity;                /* the number of characters that can be stored in textInfoArray (use TsShaperText_grow to increase) */
    TsBool decompositionOccurred;     /* indicates one or more characters decomposed (i.e. one composite character was replaced with two or more component characters) */

    /* private data - client should not access these */
    TsMemMgr *memMgr;         /* memory manager object or NULL (set by TsShaperText_new) */
    TsLength subArrayStart;   /* used internally in OpenType processing */
    TsLength subArrayLength;  /* used internally in OpenType processing */
} TsShaperText;


/*****************************************************************************
 *
 *  Creates a new memory managed TsShaperText object.
 *
 *  Description:
 *      Allocates space for the object and sets all its members to zero. If
 *      a memory manager is passed in, it will be used to allocate space for
 *      this object and any memory used by this object.
 *
 *      This object is not reference counted.
 *
 *      A call to this function should be followed by a call to TsShaperText_init.
 *
 *  Parameters:
 *      memMgr          - [in] pointer to a TsMemMgr, or NULL
 *
 *  Return value:
 *      TsShaperText    - newly created object or NULL if insufficient memory
 *
 *  <GROUP shapertext>
 */
TS_EXPORT(TsShaperText *)
TsShaperText_new(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Delete a TsShaperText object.
 *
 *  Description:
 *      Frees memory associated with the object. Frees memory allocated for
 *      the textInfoArray if TsShaperText_done has not been called yet.
 *
 *  Parameters:
 *      shaperText      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP shapertext>
 */
TS_EXPORT(void)
TsShaperText_delete(TsShaperText *shaperText);


/*****************************************************************************
 *
 *  Initializes a TsShaperText object.
 *
 *  Description:
 *      Allocates space for the object's internal textInfoArray and sets all
 *      members to zero.
 *
 *  Parameters:
 *      shaperText      - [in] this
 *      minCapacity     - [in] minimum size required for the object's
 *                             textInfoArray
 *
 *  Return value:
 *      TsResult        - TS_OK on success
 *                        TS_ERR_MALLOC_FAIL if insufficient memory
 *
 *  <GROUP shapertext>
 */
TS_EXPORT(TsResult)
TsShaperText_init(TsShaperText *shaperText, TsLength minCapacity);


/*****************************************************************************
 *
 *  Resets a TsShaperText object.
 *
 *  Description:
 *      Frees memory allocated for the object's internal textInfoArray and
 *      resets all members to zero.
 *
 *  Parameters:
 *      shaperText      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP shapertext>
 */
TS_EXPORT(void)
TsShaperText_done(TsShaperText *shaperText);


/*****************************************************************************
 *
 *  Increases the capacity of the object's textInfoArray.
 *
 *  Parameters:
 *      derived         - [in] this (a shaperText cast to void *)
 *      minCapacity     - [in] new required minimum capacity
 *
 *  Return value:
 *      TsResult        - TS_OK on success
 *                        TS_ERR_MALLOC_FAIL if insufficient memory
 *
 *  Notes:
 *      On failure, the original content of the textInfoArray remains.
 *
 *  <GROUP shapertext>
 */
TS_EXPORT(TsResult)
TsShaperText_grow(void *derived, TsLength minCapacity);

TS_END_HEADER

#endif /* TSSHAPERTEXT_H */
