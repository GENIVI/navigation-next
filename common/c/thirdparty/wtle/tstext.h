/*****************************************************************************
 *
 *  tstext.h - defines interface to the TsText class.
 *
 *  Description:
 *      The TsText class stores text and text attributes.
 *      TsText is now an abstract base class. Two derived classes are provided:
 *      TsRichText and TsSimpleText.
 *
 *      TsRichText has these features:
 *          (1) supports all text attributes with multiple ranges
 *          (2) text can be either copied or referenced
 *          (3) text can be edited
 *          (4) when text is edited a message will be sent to observers
 *          (5) designed to be passed in to TsLayout_new() or TsLayout_setText()
 *
 *      TsSimpleText has these features
 *          (1) supports a limited set of attributes
 *          (2) text is copied
 *          (3) text within the object cannot be edited, but the object can be initialize repeatedly
 *          (4) no messages are sent when a change is made
 *          (5) used internally by TsLayout for string-based layout
 *
 *      Several former TsText functions have been redefined as TsRichText functions
 *      because these functions only apply to that derived class. Each of these
 *      TsRichText functions has a backward compatibility macro. These macros
 *      and functions should be used only with objects constructed as TsRichText.
 *
 *      The TsRichText object will work in either copy mode, in which the text is
 *      copied into the object, or reference mode, in which the text is only
 *      referenced. Using a new TsRichText object with a mode-specific function
 *      will set that mode. Further operations allow only that mode.
 *
 *      For example, using TsRichText_insertText() identifies that object as
 *      being copy mode. If you then try to use TsRichText_insertRun() on that
 *      object the function will return a TS_ERR_INVALID_MODE error.
 *
 *      The following table lists the TsRichText-specific functions, the
 *      corresponding backward compatibility macros and text modes.
 *
 *      <TABLE>
 *      Function                    Backward compatibility macro    Text Mode
 *      TsRichText_new()            TsText_new                      new
 *      TsRichText_insertText()     TsText_insertText()             copy
 *      TsRichText_removeText()     TsText_removeText()             copy
 *      TsRichText_insertRun()      TsText_insertRun()              reference
 *      TsRichText_changeRun()      TsText_changeRun()              reference
 *      TsRichText_removeRun()      TsText_removeRun()              reference
 *      TsRichText_getRunRange()    TsText_getRunRange()            reference
 *      </TABLE>
 *
 *      The following table lists the TsSimpleText-specific functions. These
 *      functions do not need backward compatibility macros.
 *
 *      <TABLE>
 *      Function
 *      TsSimpleText_new()
 *      TsRichText_init()
 *      </TABLE>
 *
 *      The derived class-specific functions also use mode checking to
 *      ensure that these functions are called only with an object of the
 *      the correct derived class.
 *
 *  Copyright (C) 2002-2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSTEXT_H
#define TSTEXT_H

#include "tsproj.h"
#include "tsevent.h"
#include "tsunienc.h"
#include "tsbidi.h"
#include "tstextat.h"
#include "tsfontst.h"
#include "tseudata.h"

TS_BEGIN_HEADER



/****************************************************************************/
/**** Base class admin functions  *******************************************/


/*****************************************************************************
 *
 *  Structure for passing TsText message data.
 *
 *  <GROUP tstext-general>
 */
typedef struct TsTextMessageData_
{
    TsIndex index;      /* starting index of change */
    TsLength length;    /* number of characters involved in the change */
} TsTextMessageData;



struct TsText_;


/*****************************************************************************
 *
 *  Abstract clas text object.
 *
 *  Description:
 *
 *      The TsText object is used to store a sequence of text and attributes.
 *
 *  <GROUP tstext-general>
 */
typedef struct TsText_ TsText;


/*****************************************************************************
 *
 *  Destroys a TsText object.
 *
 *  Parameters:
 *      t           - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP tstext-general>
 */
TS_EXPORT(void)
TsText_delete(TsText *t);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsText object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsText_releaseHandle.
 *
 *  Parameters:
 *      t     - [in] this
 *
 *  Return value:
 *      Pointer to a TsText object.
 *
 *  <GROUP tstext-general>
 */
TS_EXPORT(TsText *)
TsText_copyHandle(TsText *t);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsText object.
 *
 *  Description:
 *      This function reverses the action of TsText_copyHandle.
 *
 *  Parameters:
 *      t          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP tstext-general>
 */
TS_EXPORT(void)
TsText_releaseHandle(TsText *t);


/*****************************************************************************
 *
 *  Register an observer with the text object.
 *
 *  Parameters:
 *      text            - [in] this
 *      observer        - [in] pointer to object that is observing the text
 *      callback        - [in] callback function that will be called when
 *                              something about the text object changes
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                        passed in.
 *
 *  <GROUP tstext-general>
 */
TS_EXPORT(TsResult)
TsText_registerObserver(TsText *text, void *observer, TsEventCallback callback);


/*****************************************************************************
 *
 *  Unregister an observer with the text object.
 *
 *  Parameters:
 *      text            - [in] this
 *      observer        - [in] pointer to object that is observing the text
 *                             object
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                        passed in.
 *
 *  <GROUP tstext-general>
 */
TS_EXPORT(TsResult)
TsText_unregisterObserver(TsText *text, void *observer);


/*****************************************************************************
 *
 *  Returns a pointer to an object which contains external data managed by
 *  the user.
 *
 *  Description:
 *      This function returns a TsExternalUserData pointer which allows
 *      the user to access external data that was set by a call to
 *      TsExternalUserData_init.
 *
 *      TS_USE_EXTERNAL_USERDATA must be defined in order to use the
 *      private data mechanism. If not defined this function returns NULL.
 *
 *      The TsExternalUserData object that is returned by this function
 *      becomes invalid when this TsText object is destroyed.
 *      Therefore, you must keep reference to the TsText object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      t        - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP tstext-general>
 */
TS_EXPORT(TsExternalUserData *)
TsText_getExternalUserdata(TsText *t);



/****************************************************************************/
/**** Functions to set/get attributes - implemented in all classes **********/


/*****************************************************************************
 *
 *  Gets the index range of a paragraph in a TsText.
 *
 *  Parameters:
 *      t           - [in] this
 *      paragraph   - [in] the paragraph number
 *      startIndex  - [out] returns starting index of the paragraph
 *      endIndex    - [out] returns the ending index of the paragraph
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-text>
 */

TS_EXPORT(TsResult)
TsText_getParagraphRange(TsText *t, TsInt32 paragraph, TsInt32 *startIndex, TsInt32 *endIndex);


/*****************************************************************************
 *
 *  Gets the number of paragraphs in a TsText.
 *
 *  Parameters:
 *      t           - [in] this
 *
 *  Return value:
 *      TsInt32 - the number of paragraphs.
 *
 *  <GROUP tstext-text>
 */

TS_EXPORT(TsInt32)
TsText_getParagraphCount(TsText *t);


/*****************************************************************************
 *
 *  Gets the paragraph number for a given index in a TsText.
 *
 *  Parameters:
 *      t           - [in] this
 *      index       - [in] the index of a character in the paragraph
 *
 *  Return value:
 *      TsInt32
 *
 *  <GROUP tstext-text>
 */
TS_EXPORT(TsInt32)
TsText_getParagraph(TsText *t, TsInt32 index);


/*****************************************************************************
 *
 *  Returns the text length in characters.
 *
 *  Parameters:
 *      t           - [in] this
 *
 *  Return value:
 *      TsInt32
 *
 *  <GROUP tstext-text>
 */
TS_EXPORT(TsInt32)
TsText_length(TsText *t);


/*****************************************************************************
 *
 *  Returns the character at a given index as a UTF32 value.
 *
 *  Parameters:
 *      t           - [in] this
 *      index       - [in] index of the character to return
 *
 *  Return value:
 *      TsUInt32
 *
 *  <GROUP tstext-text>
 */
TS_EXPORT(TsUInt32)
TsText_getChar(TsText *t, TsInt32 index);


/******************************************************************************
 *
 *  Returns this object's plain text as a string buffer.
 *
 *  Description:
 *      This function returns a string buffer and data about the
 *      contents of that buffer.
 *
 *      The string buffer is returned by reference via the buf pointer.
 *
 *      The data returned by 'properties' is the same as obtained
 *      by calling TsText_getProperties.
 *
 *      A TS_ERR_ARGUMENT_IS_NULL_PTR error will occur if 't' or 'buf' is NULL.
 *      The other arguments can be NULL if that argument is not needed.
 *
 *      This function is implemented by the derived class.
 *      If a particular derived class does not support this functionality,
 *      the function will return an TS_ERR_FUNCTION_NOT_SUPPORTED error.
 *
 *      TsRichText supports this functionality.  But it will return an
 *      TS_ERR_FUNCTION_NOT_SUPPORTED error if the TsRichText is
 *      comprised of more than one string. This only occurs when the
 *      text was added by reference using  TsRichText_insertRun
 *      and there is more than one run.
 *
 *  Notes:
 *      This function provides direct access to internal data.
 *      The 'buf' pointer is returned by reference to avoid a copy
 *      operation. The contents must be treated as read only.
 *      Also, the 'buf' pointer is not reference counted. It must not
 *      be stored or dereferenced after the calling routine has
 *      completed or after any changes have been made to TsText.
 *
 *  Parameters:
 *      t           - [in] pointer to string object
 *      buf         - [out] returns pointer to the string buffer
 *      nUnits      - [out] returns the number of code units in the buffer
 *      encoding    - [out] returns the character encoding
 *      length      - [out] returns the string length
 *      properties  - [out] returns a bit field containing text string properties
 *
 *  Return value:
 *      TS_OK or error code.
 *
 *  <GROUP tsstring>
 */
TS_EXPORT(TsResult)
TsText_getString(TsText *t, void **buf, TsInt32 *nUnits, TsEncoding *encoding, TsInt32 *length, TsUInt32 *properties);


/*****************************************************************************
 *
 *  Sets whether line spacing attributes are treated as paragraph attributes
 *  or as character attributes.
 *
 *  Description:
 *      Line spacing attributes (lineSpacing, lineSpacingAmount, lineSpacingBaseline
 *      linePaddingAfter, and linePaddingBefore) are by default treated as
 *      paragraph attributes. This means that the attribute value is applied
 *      to the entire paragraph even if the range that is specified when
 *      setting the attribute spans only part of the paragraph.
 *
 *      If the application requires that line spacing be treated as a
 *      character attribute then call this function with a value of FALSE.
 *      This means that the attribute value is applied to only the
 *      range of characters specified when setting the attribute.
 *
 *  Notes:
 *      This call will fail in the case that any of the line spacing attributes
 *      have been modified in this object and the call would have changed
 *      the mode.
 *
 *      It is strongly recommended that this function be called before
 *      setting any attributes.
 *
 *  Parameters:
 *      t            - [in] this
 *      isParaAttrib - [in] TRUE if line spacing attributes are paragraph attributes
 *
 *  Return value:
 *      TsResult     -  TS_OK if function call succeeds;
 *                      TS_ERR_LINE_SPACING_ATTRIBUTES_CHANGED if any line
 *                      spacing attribute has already been modified for this
 *                      text object (and the mode would have changed);
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if the passed in object is NULL.
 *
 *  <GROUP tstext-text>
 */
TS_EXPORT(TsResult)
TsText_setLineSpacingAttributeMode(TsText *t, TsBool isParaAttrib);


/*****************************************************************************
 *
 *  Gets whether line spacing attributes are treated as paragraph attributes
 *  or as character attributes. TRUE means the mode is set to paragraph attribute.
 *
 *  See also:
 *      TsText_setLineSpacingAttributeMode
 *
 *  Parameters:
 *      t           - [in] this
 *
 *  Return value:
 *      TRUE if mode is paragraph attribute. FALSE if mode is character attribute.
 *
 *  <GROUP tstext-text>
 */

TS_EXPORT(TsBool)
TsText_getLineSpacingAttributeMode(TsText *t);


/*****************************************************************************
 *
 *  Macro that makes a 'Extn' tag.
 *
 *  Description:
 *      This tag is used in the TsInlineImageID structure
 *      which is an argument to the getInlineImage callback function.
 *      This  callback is handled by TsLayout object. We define the macro here
 *      since it is first used, internally, by TsText object.
 *
 *  <GROUP tstext-text>
 */
#define TsTag_Extn TsMakeTag('E','x','t','n')



/* Defines that are bitfied flags which indicate that the text has certain properties.
   These are assigned into the properties member of the TsText object. These flags
   are used to optimize performance by keying off them to bypass algorithms that are not
   applicable.

   The 'properties' member of the TsText object has the following bit assignment.
   Bits  0-15 are reserved for TsString properties
   Bits 16-23 are reserved for TsText properties
   Bits 24-31 are reserved for TsLayout properties
*/

#define TS_TEXT_NEEDS_BIDI                  TS_STRING_NEEDS_BIDI
#define TS_TEXT_NEEDS_DPS                   TS_STRING_NEEDS_DPS
#define TS_TEXT_NEEDS_NFC                   TS_STRING_NEEDS_NFC
#define TS_TEXT_NEEDS_SHAPING               TS_STRING_NEEDS_SHAPING
#define TS_TEXT_HAS_TAB                     TS_STRING_HAS_TAB
#define TS_TEXT_HAS_MANDATORY_LINE_BREAK    TS_STRING_HAS_MANDATORY_LINE_BREAK
#define TS_TEXT_HAS_PARAGRAPH_SEPARATOR     TS_STRING_HAS_PARAGRAPH_SEPARATOR
#define TS_TEXT_HAS_THAI                    TS_STRING_HAS_THAI
#define TS_TEXT_HAS_DIACRITICS              TS_STRING_HAS_DIACRITICS
#define TS_TEXT_HAS_CONTROL_CODES           TS_STRING_HAS_CONTROL_CODES
#define TS_TEXT_IS_COPIED                   TS_STRING_IS_COPIED
#define TS_TEXT_DO_DECODE                   TS_STRING_DO_DECODE

#define TS_TEXT_HAS_BACKGROUND_LAYER        0x00010000
#define TS_TEXT_HAS_EDGE_LAYER              0x00020000
#define TS_TEXT_HAS_UNDERLINE               0x00100000
#define TS_TEXT_HAS_STRIKEOUT               0x00200000
#define TS_TEXT_HAS_OVERLINE                0x00400000


/******************************************************************************
 *
 *  Returns text object properties as a bit field.
 *
 *  Description:
 *      This function returns a set of text properties
 *      that are derived from the properties of the characters in the
 *      text object.
 *
 *      These properties can be used to control whether certain layout
 *      algorithms are needed and whether the original string character
 *      data can be accessed directly by index.
 *
 *      To use, extract the properties from the bit field using TS_GET_FLAG macro.
 *
 *      <TABLE>
 *      The bits are as follows:    Bit is set if:
 *
 *      TS_TEXT_NEEDS_BIDI          text needs bidi algorithm
 *      TS_TEXT_NEEDS_DPS           text needs diacritic positioning system
 *      TS_TEXT_NEEDS_NFC           text needs normalization form C
 *      TS_TEXT_NEEDS_SHAPING       text needs shaping
 *      TS_TEXT_HAS_TAB             text has one or more tab characters
 *      TS_TEXT_IS_COPIED           characters in text are stored by copy (else referenced)
 *      TS_TEXT_DO_DECODE           accessing characters in the text requires that string(s) be decoded
 *                                      (else characters may be able to accessed directly by index)
 *      TS_TEXT_HAS_BACKGROUND_LAYER
 *      TS_TEXT_HAS_EDGE_LAYER      any of the text has an edge layer
 *      TS_TEXT_HAS_UNDERLINE       any of the text is underlined
 *      TS_TEXT_HAS_STRIKEOUT       any of the text is strikeout
 *      TS_TEXT_HAS_OVERLINE        any of the text is overlined
 *
 *      </TABLE>
 *
 *  Parameters:
 *      t           - [in] pointer to text object
 *
 *  Return value:
 *      Returns derived properties as a bit field.
 *
 *  <GROUP tstext-text>
 */
TS_EXPORT(TsUInt32)
TsText_getProperties(TsText *t);


/****************************************************************************/
/**** Set/get text attributes ***********************************************/



/*****************************************************************************
 *
 *  Macro to indicate an index that is always less than or equal to the
 *  start of the text. The +10 is present to avoid wrapping around the edge.
 *
 *  <GROUP tstext-attrib>
 */
#define TS_START_OF_TEXT    (TS_INT32_MIN + 10)


/*****************************************************************************
 *
 *  Macro to indicate an index that is always greater than or equal to the
 *  end of the text. The -10 is present to avoid wrapping around the edge.
 *
 *  <GROUP tstext-attrib>
 */
#define TS_END_OF_TEXT      (TS_INT32_MAX - 10)


/*****************************************************************************
 *
 *  Macro that returns TRUE if index is within range, else returns FALSE.
 *
 *  Notes:
 *      This macro evaluates the index argument more than once so the
 *      argument should never be an expression with side effects.
 *
 *  <GROUP tstext-attrib>
 */
#define TS_NOT_IN_RANGE(index, start, end) (index<start || index>end ? TRUE : FALSE)


/*****************************************************************************
 *
 *  Sets the paragraph alignment attribute.
 *
 *  Description:
 *      Controls how text is aligned within the composition area.
 *
 *      <TABLE>
 *      <B>Value                <B>Text is:
 *      TS_ALIGN_LEFT           Left aligned
 *      TS_ALIGN_CENTER         Centered
 *      TS_ALIGN_RIGHT          Right aligned
 *      TS_ALIGN_LEFT_DYNAMIC   Left aligned unless bidi base level of the 
 *                              paragraph is odd (meaning right-to-left), in
 *                              which case it is right-aligned (default).
 *
 *      TS_ALIGN_RIGHT_DYNAMIC  Right aligned unless the bidi base level of
 *                              the paragraph is even (meaning left-to-right),
 *                              in which case it is left-aligned.
 *      </TABLE>
 *
 *  Version:
 *      Version 2.1 and before - The startIndex and endIndex values are ignored for
 *      this attribute. The attribute value applies to the whole text block.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setAlignment(TsText *t, TsAlignType value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets background color attribute.
 *
 *  Description:
 *      This attribute controls background color.
 *      The default value is TS_COLOR_TRANSPARENT.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setBackgroundColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the bidirectional override attribute.
 *
 *  Description:
 *      This attribute forces a particular text direction, overriding the
 *      normal direction that is derived from context.
 *
 *      <TABLE>
 *      <B>Value                <B>Text is:
 *      TS_NEUTRAL_DIRECTION    Direction determined from character properties (default)
 *      TS_LTR_DIRECTION        Forces left-to-right direction
 *      TS_RTL_DIRECTION        Forces right-to-left direction
 *      </TABLE>
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setBiDiOverride(TsText *t, TsTextDirection value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets additional spacing between characters.
 *
 *  Description:
 *      When defined, space is added to the default spacing between
 *      characters. The value is given in fractional pixels.
 *      The default value is 0.0. Negative values are allowed,
 *      but should be used with care to avoid overlapping characters.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setCharSpacing(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);



/*****************************************************************************
 *
 *  Structure for defining a character substitution callback.
 *
 *  Description:
 *      WTLE will call the remove() callback when a substitution is removed,
 *      passing the view pointer and the inline image ID as parameters.
 *
 *      Be sure to set view & remove to NULL if not used!
 *
 *  <GROUP tstext-attrib>
 */
typedef struct TsCharSubstitutionCallback_
{
    void *view; /* The "this" pointer for callbacks. */
    void (*remove)(void *view, TsUInt32 imageID);
} TsCharSubstitutionCallback;

/*****************************************************************************
 *
 *  Structure for defining a character substitution.
 *
 *  Description:
 *      This structure is used to set/get the character substitution attribute.
 *
 *      Be sure to set callback.view and callback.remove to NULL if unused!
 *
 *  <GROUP tstext-attrib>
 */
typedef struct TsCharSubstitution_
{
    TsUInt32 outputChar;    /* output character Unicode value */
    TsUInt32 imageID;       /* image handle value */
    TsCharSubstitutionCallback callback;
} TsCharSubstitution;


/*****************************************************************************
 *
 *  Sets a character substitution for a range of characters.
 *
 *  Description:
 *      This function sets the character substitution attribute for a range
 *      of characters. The attribute value defines a replacement character
 *      and an inline image ID. The attribute is applied to a range of
 *      characters within the text object.
 *
 *      The type of character substitution depends on the values of
 *      value.outputChar and value.imageID. The intended effect is similar to
 *      an entry in a substitution dictionary.
 *
 *      If value.outputChar is zero and value.imageID is zero -
 *      any substitutions within the range are cleared.
 *      This will include any substitutions that are only partly
 *      within the range. This is the default behavior.
 *
 *      If value.outputChar is non-zero and value.imageID is zero -
 *      the range of characters will be replaced by the substChar
 *      for purposes of layout rules for bidi and line breaking.
 *      The substChar will be rendered as a glyph.
 *
 *      If value.outputChar is non-zero and value.imageID is nonzero -
 *      the range of characters will be replaced by the substChar
 *      for purposes of layout rules for bidi and line breaking.
 *      The value.outputChar will rendered as an inline image using the inline
 *      image callbacks. The value.imageID will be used to specify the image.
 *
 *      If value.substChar is zero and value.imageID is nonzero -
 *      the range of characters will be treated as a single character
 *      (corresponding with the first character in the range)
 *      for purposes of layout rules for bidi and line breaking.
 *
 *      This rules for updating the attribute upon an edit are
 *      different than those for other character attributes.
 *
 *      - A substitution will be removed when any character in
 *        the substitution is deleted
 *      - A substitution will be removed if any character(s) are
 *        inserted inside an existing substitution
 *      - A substitution will be removed if a newly defined
 *        substitution overlaps with it
 *      - Removing or inserting characters adjacent to a substitution
 *        will have no effect on  the substitution except for moving
 *       the range to correspond with the edit.
 *
 *      Setting this attribute has no effect on the indexes or
 *      values of characters stored in the TsText object.
 *      The actual substitution is performed outside of this object.
 *      For example, calling TsText_getChar(t, substIndex) will return
 *      the original character, not the substituted one.
 *
 *      If value.callback.remove is not NULL, WTLE will call
 *      value.callback.remove() when a substition is removed.
 *
 *      Be sure to set value.callback.remove (and value.callback.view) 
 *      to NULL if not using the callback feature.
 *
 *  Notes:
 *      If the substitution range spans a CR and/or LF, which marks
 *      a new paragraph, then the substitution will suppress the line
 *      feed. In general, you should avoid defining substitutions that
 *      span multiple paragraphs.
 *
 *      The layout code uses value.char to determine the character properties
 *      of the whole substitution.  Avoid using any character that indicates
 *      a new paragraph such as CR or LF, any non-printing characters and
 *      punctuation characters. For example, if you set value.char to 0x0A or 0x0D
 *      then a newline is inserted but not the image.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] the range of characters is substituted with this character/image
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setCharSubstitution(TsText *t, TsCharSubstitution value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets edge color attribute.
 *
 *  Description:
 *      This attribute controls the color of the edge when an edge effect
 *      such as 'uniform outline' is applied.
 *      Setting edge effect is part of the font style parameters.
 *      The default value is TS_COLOR_BLACK.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setEdgeColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the font style attribute.
 *
 *  Description:
 *      This attribute is a pointer to an abstract description of a
 *      styled font, i.e. a font with attributes such as size, boldness,
 *      and so forth.
 *
 *      This attribute does not have a default value. The layout
 *      requires a valid setting for each character so it is recommended
 *      that this function be called such that is spans any character
 *      range using TS_START_OF_TEXT and TS_END_OF_TEXT with the
 *      desired default value, before setting the attribute over
 *      specific ranges.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setFontStyle(TsText *t, TsFontStyle *value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the amount the first line of each paragraph is indented.
 *
 *  Description:
 *      The indent amount is given in fixed point pixels. The default is 0.0.
 *      The indent is in addition to any left indent or right indent settings.
 *      The effect depends on the alignment attribute:
 *
 *  <TABLE>
 *      <B> Alignment Value             <B>Effect
 *      TS_ALIGN_LEFT                   Indents from the left
 *      TS_ALIGN_RIGHT                  Indents from the right
 *      TS_ALIGN_CENTER                 No change
 *      TS_ALIGN_LEFT_DYNAMIC           Indents from dynamically determined alignment
 *      TS_ALIGN_RIGHT_DYNAMIC          Indents from dynamically determined alignment
 *  </TABLE>
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setIndent(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the inline image ID.
 *
 *  Description:
 *      This function calls TsText_setCharSubstitution() with its
 *      value parameter set to (0, value).
 *
 *      It is recommended that you call TsText_setCharSubstitution()
 *      directly.
 *
 *      This function will be deprecated.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] image ID (0 = no image ID)
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setInlineImageID(TsText *t, TsUInt32 value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Enables/disable justification of a paragraph.
 *
 *  Description:
 *      If enabled then the text block is justified. The last line in the
 *      paragraph is generally not justified. It is aligned according to
 *      the alignment type set with TsText_setAlignment.
 *
 *      If disabled then the paragraph is aligned using the alignment type
 *      set with TsText_setAlignment.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setJustification(TsText *t, TsBool value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the language attribute.
 *
 *  Description:
 *      Identifying the language of a text run can help clarify proper text
 *      layout.
 *
 *      Generally this value should be zero, indicating default
 *      handling, unless the specific language of the run is
 *      known.  Language tag macros are given in otlangs.h.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLanguage(TsText *t, TsTag value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the amount the left side of each paragraph in the range is indented.
 *
 *  Description:
 *      The left indent amount is given in fixed-point pixels.
 *      The default value is 0.0.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLeftIndent(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets line spacing attribute.
 *
 *  Description:
 *      The line spacing attribute uses the TsLineSpacing type.
 *      The default value is TS_SINGLE_LINE.
 *
 *  <TABLE>
 *      <B> Line Spacing Value  <B>Effect
 *      TS_SINGLE_LINE          Single line spacing
 *      TS_150_LINE             One and a half line spacing
 *      TS_DOUBLE_LINE          Double line spacing
 *      TS_EXACT_LINE_SPACING   Line spacing is given by the line spacing amount attribute
 *      TS_MULTIPLE_LINE        Line spacing is single line spacing * line spacing amount
 *  </TABLE>
 *
 *  Version:
 *      Version 3.2:
 *          The location of the baseline when using TS_EXACT_LINE_SPACING
 *          changed in this version. It is now set proportional to the ascender,
 *          descender and leading values of the current fontstyle; previously
 *          it was set to the ascender.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLineSpacing(TsText *t, TsLineSpacing value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets line spacing amount attribute.
 *
 *  Description:
 *      The affect of the line spacing amount attribute depends on the
 *      line spacing type as set by TsText_setLineSpacing.
 *      The default value is 0.0.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLineSpacingAmount(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the baseline explicitly when using the exact line spacing type.
 *
 *  Description:
 *      Allows control of the location of the baseline when using the line
 *      spacing type is TS_EXACT_LINE_SPACING. The attribute value specifies
 *      the distance from the top of the line.
 *
 *      The default value is 0.0.
 *
 *      If this api is not used to set the baseline when TS_EXACT_LINE_SPACING
 *      is set, the baseline will be set proportional to the ascender,
 *      descender and leading value of the current fontstyle.
 *
 *  Version:
 *      Version 3.2:
 *          The location of the baseline when using TS_EXACT_LINE_SPACING
 *          changed in this version. It is now set proportional to the ascender,
 *          descender and leading values of the current fontstyle; previously
 *          it was set to the ascender.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLineSpacingBaseline(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets text overline attribute.
 *
 *  Description:
 *      This attribute controls the overline style. The default is TS_LINE_NONE
 *      which means the text is not overlined. The line thickness is a function
 *      of scaled font size and the underline thickness metric stored within the font.
 *
 *      Overline can also be set using the TsFontStyle, specifically the
 *      'emphasis' element in TsFontParams. See TsEmphasisStyle for details.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setOverline(TsText *t, TsLineStyle value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets overline color attribute.
 *
 *  Description:
 *      This attribute controls overline color.
 *      The default value is TS_COLOR_BLACK.
 *
 *      If the overline attribute is set using the emphasis part of TsFontStyle
 *      then the color set here is ignored. In that case the overline
 *      color is set to the text color.
  *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setOverlineColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the amount the right side of each paragraph in the range is indented.
 *
 *  Description:
 *      The right indent amount is given in fixed-point pixels.
 *      The default value is 0.0.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setRightIndent(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets a color for future use in drawing shadows.
 *
 *  Description:
 *      This attribute is reserved for future use with glyphs that have
 *      both a shadow and another edge type. This attribute is not
 *      currently used. To set the color of shadows used in edge
 *      effects such as TS_EDGE_DROP_SHADOW, use TsText_setEdgeColor.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setShadowColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the extra space after each paragraph.
 *
 *  Description:
 *      The space after amount is given in fixed-point pixels.
 *      The default value is 0.0.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Version:
 *      Version 2.x - The startIndex and endIndex values are ignored for
 *      this attribute. The attribute value applies to the whole text block.
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setSpaceAfter(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the extra space before a paragraph.
 *
 *  Description:
 *      The space before amount is given in fixed-point pixels.
 *      The default value is 0.0.
 *
 *  Version:
 *      Version 2.x - The startIndex and endIndex values are ignored for
 *      this attribute. The attribute value applies to the whole text block.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setSpaceBefore(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets an amount of extra space to pad after each line.
 *
 *  Description:
 *      Line padding is a space between lines in addition to that calculated
 *      using the line spacing formula. It may be positive or negative.
 *      Line padding affects the advance box calculation.
 *
 *      'Line padding after' adds space after each line. It does not affect the
 *      position of the first line. A positive value increases the line spacing
 *      and a negative value decreases the line spacing.
 *
 *      The value is given in fixed-point pixels; the default is 0.0.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLinePaddingAfter(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets an amount of extra space to pad before each line.
 *
 *  Description:
 *      Line padding is a space between lines in addition to that calculated
 *      using the line spacing formula. It may be positive or negative.
 *      Line padding affects the advance box calculation.
 *
 *      'Line padding before' adds space before each line. This attribute
 *      differs from 'line padding after' because it affects the position
 *      of the first line.  A positive value increases the line spacing
 *      and a negative value decreases the line spacing.
 *
 *      The value is given in fixed-point pixels; the default is 0.0.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setLinePaddingBefore(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets strikeout attribute.
 *
 *  Description:
 *      This attribute controls the strikeout style. The default is TS_LINE_NONE
 *      which means the text does not have a strikeout line. The line thickness
 *      is a function of scaled font size and the strikeout thickness metric
 *      stored within the font.
 *
 *      Strikeout can also be set using the TsFontStyle, specifically the
 *      'emphasis' element in TsFontParams. See TsEmphasisStyle for details.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setStrikeout(TsText *t, TsLineStyle value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets strikeout color attribute.
 *
 *  Description:
 *      This attribute controls strikeout color.
 *      The default value is TS_COLOR_BLACK.
 *
 *      If the strikeout attribute is set using the emphasis part of TsFontStyle
 *      then the color set here is ignored. In that case the strikeout
 *      color is set to the text color.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setStrikeoutColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets text color attribute.
 *
 *  Description:
 *      This attribute controls the text color.
 *      The default value is TS_COLOR_BLACK.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setTextColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets the paragraph base text direction attribute.
 *
 *  Description:
 *      The default value is TS_NEUTRAL_DIRECTION which means the base
 *      direction is determined from the text in each paragraph.
 *
 *  Version:
 *      Version 2.1 and before  - The startIndex and endIndex values are ignored for
 *      this attribute. The attribute value applies to the whole text block.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setTextDirection(TsText *t, TsTextDirection value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets text underline attribute.
 *
 *  Description:
 *      This attribute controls the underline style. The default is TS_LINE_NONE
 *      which means the text is not underlined. The line thickness is a function
 *      of scaled font size and the underline thickness metric stored within the font.
 *
 *      Underline can also be set using the TsFontStyle, specifically the
 *      'emphasis' element in TsFontParams. See TsEmphasisStyle for details.
 *
 *
 *  Version 2.1:
 *      The TsUnderlineStyle type has been deprecated. Use TsLineStyle instead.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setUnderline(TsText *t, TsLineStyle value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets underline color attribute.
 *
 *  Description:
 *      This attribute controls underline color.
 *      The default value is TS_COLOR_BLACK.
 *
 *      If the underline attribute is set using the emphasis part of TsFontStyle
 *      then the color set here is ignored. In that case the underline
 *      color is set to the text color.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setUnderlineColor(TsText *t, TsColor value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Sets additional spacing between words.
 *
 *  Description:
 *      When defined, space is added to the default spacing between
 *      words. The value is given in fractional pixels.
 *      The default value = 0. Negative values are allowed,
 *      but should be used with care to avoid overlapping characters.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setWordSpacing(TsText *t, TsFixed value, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Marks section of text where substitutions by way of dictionary
 *  will be supressed.
 *
 *  Description:
 *      When defined, substitutions that would normally be made
 *      by the use of one or more substitution dictionaries
 *      will be suppressed.  A common use case for this could be
 *      when displaying a web page url that often contains somewhat
 *      random characters.  This could prevent a smiley face appearing
 *      in the middle of a long url for example.
 *
 *  Parameters:
 *      t           - [in] this
 *      value       - [in] attribute value
 *      startIndex  - [in] index of first character in range
 *      endIndex    - [in] index of last character in range
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsResult)
TsText_setSubstitutionSuppression(TsText *t, TsBool value, TsInt32 startIndex, TsInt32 endIndex);



TS_EXPORT(TsAlignType)
TsText_getAlignment(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getBackgroundColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsTextDirection)
TsText_getBiDiOverride(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getCharSpacing(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsCharSubstitution)
TsText_getCharSubstitution(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getEdgeColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


/*****************************************************************************
 *
 *  Gets the font style for a range of characters.
 *
 *  Parameters:
 *      t           - [in] this
 *      index       - [in] index at which to get the font style
 *      startIndex  - [out] index of first character in range
 *      endIndex    - [out] index of last character in range
 *
 *  Notes:
 *     This function increments the reference count on the TsFontStyle object
 *     that is returned; so TsFontStyle_releaseHandle must be called on the
 *     returned object once you are done with it.
 *
 *  Return value:
 *      TsFontStyle
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsFontStyle *)
TsText_getFontStyle(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);



TS_EXPORT(TsFixed)
TsText_getIndent(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);



/*****************************************************************************
 *
 *  Gets the inline image ID.
 *
 *  Parameters:
 *      t           - [in] this
 *      index       - [in] index at which to get the font style
 *      startIndex  - [out] index of first character in range
 *      endIndex    - [out] index of last character in range
 *      value       - [out] pointer to structure into which parameters are copied
 *
 *  Notes:
 *
 *  Return value:
 *      TsUInt32    image ID (0 = no image ID).
 *
 *  <GROUP tstext-attrib>
 */
TS_EXPORT(TsUInt32)
TsText_getInlineImageID(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsBool)
TsText_getJustification(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsTag)
TsText_getLanguage(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getLeftIndent(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getLinePaddingAfter(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getLinePaddingBefore(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsLineSpacing)
TsText_getLineSpacing(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getLineSpacingAmount(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getLineSpacingBaseline(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsLineStyle)
TsText_getOverline(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getOverlineColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getRightIndent(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getShadowColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getSpaceAfter(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getSpaceBefore(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsLineStyle)
TsText_getStrikeout(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getStrikeoutColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getTextColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsTextDirection)
TsText_getTextDirection(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsLineStyle)
TsText_getUnderline(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsColor)
TsText_getUnderlineColor(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);


TS_EXPORT(TsFixed)
TsText_getWordSpacing(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);

TS_EXPORT(TsBool)
TsText_getSubstitutionSuppression(TsText *t, TsInt32 index, TsInt32 *startIndex, TsInt32 *endIndex);

/****************************************************************************/
/**** TsSimpleText functions  ***********************************************/


/*****************************************************************************
 *
 *  TsSimpleText is class derived from TsText for storing text and attributes.
 *
 *  Description:
 *
 *      The TsSimpleText object is derived from TsText. This object is
 *      used internally for string-based layout. It supports a limited
 *      set of attributes. The text and attributes can be changed but
 *      inserting text or other editing features are not supported.
 *
 *  <GROUP tssimpletext>
 */
typedef struct TsSimpleText_ TsSimpleText;


/*****************************************************************************
 *
 *  A structure for specifying string-based layout input parameters.
 *
 *  <GROUP tssimpletext>
 */
typedef struct
{
    TsTag           language;   /* language of the text */
    TsTextDirection direction;  /* allows override of text direction */
} TsStringParams;


/*****************************************************************************
 *
 *  Creates and initializes a TsSimpleText object.
 *
 *  Parameters:
 *      memMgr      - [in] memory manager object, may be NULL.
 *
 *  Return value:
 *      Pointer to new TsSimpleText object.
 *
 *  <GROUP tssimpletext>
 */
TS_EXPORT(TsText *)
TsSimpleText_new(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Initializes a TsSimpleText object with text and attributes.
 *
 *  Parameters:
 *      t               - [in]  pointer to TsSimpleText object
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      stringParams    - [in]  string-based layout input parameters
 *
 *  Return value:
 *      TsResult value
 *
 *  <GROUP tssimpletext>
 */
TS_EXPORT(TsResult)
TsSimpleText_init(TsText *t, void *string, TsInt32 nUnits, TsEncoding encoding,
                  TsFontStyle *fontStyle, TsStringParams *stringParams);



/****************************************************************************/
/**** TsRichText functions  *************************************************/


/*****************************************************************************
 *
 *  TsRichText is class derived from TsText for storing text and attributes.
 *
 *  Description:
 *
 *      The TsRichText object is derived from TsText. Text can be added
 *      and later edited. A full set of attributes can set using the
 *      TsText functions and later accessed using the get functions.
 *
 *
 *  <GROUP tsrichtext>
 */
typedef struct TsRichText_ TsRichText;


/*****************************************************************************
 *
 *  Creates and initializes a TsRichText object.
 *
 *  Parameters:
 *      memMgr      - [in] memory manager object, may be NULL.
 *
 *  Return value:
 *      Pointer to new TsRichText object derived from TsText.
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsText *)
TsRichText_new(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Inserts a text run by copy to a TsText object.
 *
 *  Description:
 *      This function inserts a text run into the text object. The data is
 *      copied.
 *
 *      The text is inserted before the 'insertIndex'.
 *      If insertIndex is TS_END_OF_TEXT then the new text is appended.
 *
 *      It is a checked run-time error to mix copied and referenced
 *      text within the same TsText object.
 *
 *  Parameters:
 *      t           - [in] this
 *      run         - [in] pointer to a run of text
 *      nUnits      - [in] length of the text run in code units
 *      encoding    - [in] encoding of the text in the run
 *      insertIndex - [in] the new text run is inserted before this index
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsResult)
TsRichText_insertText(TsText *t, void *run, TsInt32 nUnits, TsEncoding encoding, TsInt32 insertIndex);


 /*****************************************************************************
 *
 *  Deletes text from a text object that has copied text.
 *
 *  Description:
 *      This function deletes some text from the text object. The data is
 *      copied.
 *
 *      The 'startIndex' and 'endIndex' arguments indicate the range of text to
 *      delete. If the range is empty then this function does nothing. The
 *      startIndex must be less than or equal to endIndex; otherwise the
 *      function will return TS_ERR_ARGUMENT_OUT_OF_RANGE.
 *
 *      It is a checked run-time error to mix copied and referenced
 *      text within the same TsText object.
 *
 *  Parameters:
 *      t           - [in] this
 *      startIndex  - [in] index of first deleted character
 *      endIndex    - [in] index of last deleted character
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsResult)
TsRichText_removeText(TsText *t, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Insert a text run by reference.
 *
 *  Description:
 *      This function inserts a text run by reference. A text run is
 *      a contiguous block of text data. 'By reference' means that
 *      the TsText object maintains a pointer to the run rather than
 *      copying the text data. The client should not free the
 *      run until the run has been removed from the TsText object
 *      using TsText_removeRun or TsText_changeRun, or the TsText object
 *      has been deleted using TsText_delete.
 *
 *      The text is logically inserted before the run indicated by the
 *      insertHere parameter, a pointer to another run already
 *      referenced by the TsText object. If insertHere is NULL
 *      then the new run is appended to the end of the text block.
 *
 *      It is a checked run-time error to mix copied and referenced
 *      text within the same TsText object.
 *
 *  Parameters:
 *      t           - [in] this
 *      run         - [in] pointer to a run of text
 *      nUnits      - [in] length of the text run in code units
 *      encoding    - [in] encoding of the text in the run
 *      insertHere  - [in] the new text run is inserted before this run
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsResult)
TsRichText_insertRun(TsText *t, void *run, TsInt32 nUnits, TsEncoding encoding, void *insertHere);


/*****************************************************************************
 *
 *  Change a referenced text run.
 *
 *  Description:
 *      This function changes a referenced text run. The text pointed to by
 *      oldRun is replaced by the text pointed to by newRun. The oldRun
 *      and newRun pointers can be the same.
 *
 *      It is a checked run-time error to mix copied and referenced
 *      text within the same TsText object.
 *
 *  Parameters:
 *      t           - [in] this
 *      newRun      - [in] the new text run
 *      nUnits      - [in] length of the new text run in code units
 *      encoding    - [in] encoding of the text in the new run
 *      oldRun      - [in] the old text run that is to be changed (may be the same as old)
 *      hints       - [in] optional hints about the change (not currently supported)
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsResult)
TsRichText_changeRun(TsText *t, void *newRun, TsInt32 nUnits, TsEncoding encoding, void *oldRun, TsInt32 hints);


/*****************************************************************************
 *
 *  Removes a referenced text run from a TsText object.
 *
 *  Parameters:
 *      t           - [in] this
 *      run         - [in] the text run to remove
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsResult)
TsRichText_removeRun(TsText *t, void *run);


/*****************************************************************************
 *
 *  Gets the index range of a run in a TsText.
 *
 *  Parameters:
 *      t           - [in] this
 *      run         - [in] the text run referenced by this TsText object
 *      startIndex  - [out] returns starting index of the run
 *      endIndex    - [out] returns the ending index of the run
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP tsrichtext>
 */
TS_EXPORT(TsResult)
TsRichText_getRunRange(TsText *t, void *run, TsInt32 *startIndex, TsInt32 *endIndex);



/* Backward compatibility macros for TsRichText-specific functions */


#define TsText_new() \
        TsRichText_new(NULL)

#define TsText_insertText(t, run, nUnits, encoding, insertIndex)\
        TsRichText_insertText(t, run, nUnits, encoding, insertIndex)

#define TsText_removeText(t, startIndex, endIndex) \
        TsRichText_removeText(t, startIndex, endIndex)

#define TsText_insertRun(t, run, nUnits, encoding, insertHere) \
        TsRichText_insertRun(t, run, nUnits, encoding, insertHere)

#define TsText_changeRun(t, newRun, nUnits, encoding, oldRun, hints) \
        TsRichText_changeRun(t, newRun, nUnits, encoding, oldRun, hints)

#define TsText_removeRun(t, run) \
        TsRichText_removeRun(t, run)

#define TsText_getRunRange(t, run, startIndex, endIndex) \
        TsRichText_getRunRange(t, run, startIndex, endIndex)



TS_END_HEADER

#endif /* TSTEXT_H */
