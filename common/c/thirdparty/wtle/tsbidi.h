/*****************************************************************************
 *
 *  tsbidi.h
 *
 *  WTLE Shaping Interface -- Bidi Algorithm
 *
 *  Copyright (C) 2005, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TS_BIDI_H
#define TS_BIDI_H

#include "tsconfig.h"

TS_BEGIN_HEADER

#define TS_NEUTRAL_DIRECTION    0       /* text with no inherent direction */
#define TS_LTR_DIRECTION        1       /* left-to-right text */
#define TS_RTL_DIRECTION        2       /* right-to-left text */


/*****************************************************************************
 *
 *  Typedef for text flow direction.
 *
 *  Description:
 *      Text direction is used for the bidirectional algorithm.
 *
 *      <TABLE>
 *      <B>Type                         <B>Description
 *      TS_NEUTRAL_DIRECTION            text with no inherent direction
 *      TS_LTR_DIRECTION                left-to-right text
 *      TS_RTL_DIRECTION                right-to-left text
 *      </TABLE>
 *
 *  <GROUP shaperbidi>
 */
typedef TsUInt8 TsTextDirection;

#ifndef TS_NO_BIDIRECTIONAL



/*****************************************************************************
 *
 *  The bidi text object.
 *
 *  Description:
 *
 *      The TsBidiTextObj object provides an abstracted interface to the
 *      bidi code.  The user must populate the object with a pointer to text
 *      data (including level and type storage) and override data and pointers
 *      to functions which can store and retrieve that data.
 *
 *      <PRE>
 *      text
 *          -Description:
 *              pointer to the user's bidi text data which must
 *              be capable of storing characters, levels, and types.
 *
 *      overrides
 *          -Description:
 *              pointers to the user's directional override data (if desired)
 *
 *      getChar
 *          -Description:
 *              retrieves a character
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index from which to retrieve a char
 *          -Returns:
 *              a character
 *
 *      getLevel
 *          -Description:
 *              retrieves a bidi level (that was previously stored with setLevel)
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index from which to retrieve a level
 *          -Returns:
 *              the bidi level of the char at the specified index
 *
 *      setLevel
 *          -Description:
 *              stores a bidi level
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index at which to store a level
 *              level       - [in] the level to store
 *          -Returns:
 *              none
 *
 *      getOverride
 *          -Description:
 *              retrieves a directional override (if desired)
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index at which to retrieve the override
 *          -Returns:
 *              the override value
 *
 *      getType
 *          -Description:
 *              retrieves a bidi type (that was previously stored with setType)
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index at which to retrieve the Bidi type
 *          -Returns:
 *              the bidi type
 *
 *      setType
 *          -Description:
 *              stores a bidi type
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index at which to store the type
 *              type        - [in] the type to store
 *          -Returns:
 *              none
 *
 *      swapChars
 *          -Description:
 *              swaps the positions of two characters (used for reordering)
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index1      - [in] the index of one of two chars to swap
 *              index2      - [in] the index of the other of two chars to swap
 *          -Returns:
 *              none
 *
 *      mirrorChar
 *          -Description:
 *              mirrors a character, e.g. '[' becomes ']'
 *          -Parameters:
 *              textObj     - [in] the bidi text object
 *              index       - [in] the index at which to mirror a char
 *          -Returns:
 *              none
 *
 *      </PRE>
 *
 *  <GROUP bidi>
 */
typedef struct TsBidiTextObj_
{
    void      *text;
    void      *overrides;
    TsUInt32 (*getChar)(struct TsBidiTextObj_ *textObj, TsInt32 index);
    TsUInt8  (*getLevel)(struct TsBidiTextObj_ *textObj, TsInt32 index);
    void     (*setLevel)(struct TsBidiTextObj_ *textObj, TsInt32 index, TsUInt8 level);
    TsUInt8  (*getOverride)(struct TsBidiTextObj_ *textObj, TsInt32 index);
    TsUInt8  (*getType)(struct TsBidiTextObj_ *textObj, TsInt32 index);
    void     (*setType)(struct TsBidiTextObj_ *textObj, TsInt32 index, TsUInt8 type);
    void     (*swapChars)(struct TsBidiTextObj_ *textObj, TsInt32 index1, TsInt32 index2);
    void     (*mirrorChar)(struct TsBidiTextObj_ *textObj, TsInt32 index);
} TsBidiTextObj;


/*****************************************************************************
 *
 *  The bidi stack object.
 *
 *  Description:
 *
 *      The TsBidiStackObj object provides an abstracted interface to a
 *      stack for use in the bidi code.  The user must populate the object
 *      with a pointer to stack data and pointers to functions to push and
 *      pop data to and from the stack.
 *
 *      The push and pop functions should return TRUE if successful,
 *      FALSE otherwise.
 *
 *  <GROUP bidi>
 */
typedef struct TsBidiStackObj_
{
    void    *stack;                                                 /* void pointer to stack */
    TsBool (*push)(struct TsBidiStackObj_ *stackObj, TsUInt8 item); /* push function */
    TsBool (*pop)(struct TsBidiStackObj_ *stackObj, TsUInt8 *item); /* pop function */
} TsBidiStackObj;


/*****************************************************************************
 *
 *  Determine the Bidi levels for use in reordering the text.
 *
 *  Parameters:
 *      textObj         - [in/out] the bidi text object
 *      startIndex      - [in] the index at which to start processing
 *      endIndex        - [in] the index at which to stop processing
 *      stackObj        - [in/out] the bidi stack object
 *      hasExplicitParagraphEmbeddingLevel - [in] TRUE if explicit level given
 *      explicitParagraphEmbeddingLevel    - [in] the explicit level
 *      implicitParagraphEmbeddingLevel    - [out] the implicit level
 *      bidiNeeded                         - [out] TRUE if reordering needed
 *
 *  Return value:
 *      none
 *
 *  <GROUP bidi>
 */
TS_EXPORT(void)
TsBidi_determineLevels(
    TsBidiTextObj  *textObj,
    TsInt32        startIndex,
    TsInt32        endIndex,
    TsBidiStackObj *stackObj,
    TsBool          hasExplicitParagraphEmbeddingLevel,
    TsUInt8         explicitParagraphEmbeddingLevel,
    TsUInt8        *implicitParagraphEmbeddingLevel,
    TsBool         *bidiNeeded
);

/*****************************************************************************
 *
 *  Reorder the text based on the bidi levels.
 *
 *  Parameters:
 *      textObj                 - [in/out] the bidi text object
 *      startIndex              - [in] the index at which to start processing
 *      endIndex                - [in] the index at which to stop processing
 *      paragraphEmbeddingLevel - [in] the paragraph embedding level
 *
 *  Return value:
 *      none
 *
 *  <GROUP bidi>
 */
TS_EXPORT(void)
TsBidi_reorderByLevels(
    TsBidiTextObj * textObj,
    TsInt32 startIndex,
    TsInt32 endIndex,
    TsUInt8  paragraphEmbeddingLevel
);

/*****************************************************************************
 *
 *  Find the index of the end of the current paragraph.
 *
 *  Parameters:
 *      textObj                 - [in] the bidi text object
 *      endIndex                - [in] the index at which to stop looking
 *      startOfParaIndex        - [in] the index from which to start looking
 *
 *  Return value:
 *      The index of the end of the current paragraph.
 *
 *  <GROUP bidi>
 */
TS_EXPORT(TsInt32)
TsBidi_findEndOfParagraph(
    TsBidiTextObj * textObj,
    TsInt32 endIndex,
    TsInt32 startOfParaIndex
);


/*****************************************************************************
 *
 *  Mirrors any glyphs that are required to be mirrored per Bidi rule L4.
 *
 *  Parameters:
 *      textObj                 - [in] the bidi text object
 *      startIndex              - [in] the index at which to start processing
 *      endIndex                - [in] the index at which to stop processing
 *
 *  Return value:
 *      none
 *
 *  <GROUP bidi>
 */
TS_EXPORT(void)
TsBidi_processMirroredChars(
    TsBidiTextObj * textObj,
    TsInt32 startIndex,
    TsInt32 endIndex
);


#endif /* TS_NO_BIDIRECTIONAL */

TS_END_HEADER

#endif /* TS_BIDI_H */
