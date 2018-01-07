/*****************************************************************************
 *
 *  tsshaper.h - Interface to TsShaper class.
 *
 *  Copyright (C) 2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifndef TSSHAPER_H
#define TSSHAPER_H

#include "tsshaperfont.h"
#include "tsshapertext.h"
#include "tsbidi.h"

TS_BEGIN_HEADER


struct TsShaper_;


/*****************************************************************************
 *
 *  A shaper object.
 *
 *  Description:
 *      The TsShaper object provides APIs for shaping and bidi processing
 *      of a TsShaperText object using a TsShaperFont object.
 *
 *  <GROUP shaper>
 */
typedef struct TsShaper_ TsShaper;


/* The public API */


/*****************************************************************************
 *
 *  Creates a new memory managed TsShaper object.
 *
 *  Description:
 *      TsShaper constructor
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new TsShaper object or NULL upon error.
 *
 *  <GROUP shaper>
 */
TS_EXPORT(TsShaper *)
TsShaper_new(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Frees a TsShaper object.
 *
 *  Description:
 *      TsShaper destructor
 *
 *  Parameters:
 *      shaper      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP shaper>
 */
TS_EXPORT(void)
TsShaper_delete(TsShaper *shaper);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsShaper object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsShaper_releaseHandle.
 *
 *  Parameters:
 *      shaper      - [in] this
 *
 *  Return value:
 *      Pointer to a shaper object.
 *
 *  <GROUP shaper>
 */
TS_EXPORT(TsShaper *)
TsShaper_copyHandle(TsShaper *shaper);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsShaper object.
 *
 *  Description:
 *      This function reverses the action of TsShaper_copyHandle.
 *
 *  Parameters:
 *      shaper     - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP shaper>
 */
TS_EXPORT(void)
TsShaper_releaseHandle(TsShaper *shaper);


/*****************************************************************************
 *
 *  Sets the grapheme, word, and line break flags in a range of text.
 *
 *  Description:
 *      Sets the following boundary flags:
 *      <TABLE>
 *          <B>Flag</B>                 <B>Description</B>
 *          TS_GRAPHEME_START           start of a grapheme cluster
 *          TS_GRAPHEME_END             end of a grapheme cluster
 *          TS_WORD_START               start of a word
 *          TS_WORD_END                 end of a word
 *          TS_POTENTIAL_LINE_START     first char after a line break point
 *          TS_POTENTIAL_LINE_END       last char before a line break point
 *          TS_REQUIRED_LINE_END        char forces a line break
 *          TS_PARAGRAPH_BOUNDARY       char ends paragraph
 *     </TABLE>
 *
 *     Note that these flags may be changed in during shaping,
 *     especially grapheme boundaries for Indic scripts.
 *
 *     This method also pads isolated diacritics with a base character
 *     as specified by TsShaper_optionsSetBaseChar() or U+00A0 by default.
 *
 *     This method normalizes Thai text before marking Thai word boundaries:
 *     the SARA_AM character decomposes to NIKHAHIT SARA_AA.
 *
 *     Note that both the padding of the isolated diacritics and the
 *     normalization of the Thai text can result in an increases in the length
 *     of the text, as indicated in arrayGrowth.
 *
 *  Parameters:
 *      shaper          - [in]     pointer to a shaper object
 *      shaperText      - [in,out] pointer to a TsShaperText object
 *      thaiDict        - [in]     pointer to Thai dictionary, NULL if not used
 *      startIndex      - [in]     index into array of text at which to begin setting flags
 *      endIndex        - [in]     index into array of text at which to stop setting flags
 *      arrayGrowth     - [out]    the number of characters by which the text grew (due to decomposition)
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP shaper>
 */
TS_EXPORT(TsResult)
TsShaper_markBoundaries(TsShaper *shaper, TsShaperText *shaperText, void *thaiDict, TsInt32 startIndex, TsInt32 endIndex, TsInt32 *arrayGrowth);


/*****************************************************************************
 *
 *  Shape the glyphs.
 *
 *  Description:
 *      Uses OpenType or legacy approaches (in absence of OpenType) to determine
 *      the correct contextual glyph forms for characters in the specified range
 *      of the TsShaperText object.
 *
 *  Parameters:
 *      shaper          - [in]     pointer to a shaper object
 *      shaperFont      - [in]     pointer to a TsShaperFont object
 *      shaperText      - [in,out] pointer to a TsShaperText object
 *      startIndex      - [in]     index into array of text at which to begin shaping
 *      endIndex        - [in]     index into array of text at which to stop shaping
 *      data            - [in]     user data object
 *      arrayGrowth     - [out]    the number of characters by which the text grew (due to decomposition)
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP shaper>
 */
TS_EXPORT(TsResult)
TsShaper_shape(TsShaper *shaper, TsShaperFont *shaperFont, TsShaperText *shaperText,
               TsInt32 startIndex, TsInt32 endIndex, void *data, TsInt32 *arrayGrowth);

/*****************************************************************************
 *
 *  Apply kerning.
 *
 *  Description:
 *      Uses old style kern table format 0
 *      to apply kerning in the specified range
 *      of the TsShaperText object.
 *
 *  Parameters:
 *      shaper          - [in]     pointer to a shaper object
 *      shaperFont      - [in]     pointer to a TsShaperFont object
 *      shaperText      - [in,out] pointer to a TsShaperText object
 *      startIndex      - [in]     index into array of text at which to begin shaping
 *      endIndex        - [in]     index into array of text at which to stop shaping
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP shaper>
 */
TS_EXPORT(TsResult)
TsShaper_kern(TsShaper *shaper, TsShaperFont *shaperFont, TsShaperText *shaperText,
               TsInt32 startIndex, TsInt32 endIndex);



#ifndef TS_NO_BIDIRECTIONAL


/*****************************************************************************
 *
 *  Initializes a TsShaper's TsShaperBidi object.
 *
 *  Description:
 *      Initializes the TsShaperBidi object that is container in the
 *      TsShaper object.  This does not need to be called if the bidi
 *      APIs will not be used.  It should be called before any of the other
 *      bidi methods.
 *
 *  Parameters:
 *      shaper     - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP shaperbidi>
 */
TS_EXPORT(void)
TsShaper_bidiInit(TsShaper *shaper);


/*****************************************************************************
 *
 *  Set the getOverride callback function and associated data.
 *
 *  Description:
 *      This is optional and is not normally needed, but is available in
 *      case a higher-level protocol (e.g. HTML, XML, etc.) overrides the
 *      normal directionality of characters in the text.
 *
 *  Parameters:
 *      shaper          - [in] this
 *      getOverride     - [in] callback function for getting overrides
 *      data            - [in] data associated with getOverride
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP shaperbidi>
 */
TS_EXPORT(TsResult)
TsShaper_bidiSetOverride(TsShaper *shaper, TsTextDirection (*getOverride)(struct TsBidiTextObj_ *textObj, TsInt32 index), void *data);


/*****************************************************************************
 *
 *  Summary:
 *      Sets the bidi levels for a paragraph of text.
 *
 *  Description:
 *      This function analyzes the text in the TsShaperText object so that it
 *      can be reordered later.  It should be called for a paragraph of text
 *      at a time.
 *
 *  Parameters:
 *      shaper          - [in] this
 *      shaperText      - [in/out] pointer to TsShaperText object
 *      startIndex      - [in] the index at which to start processing
 *      endIndex        - [in] the index at which to stop processing
 *      baseLevel       - [out] the bidi base level of the paragraph
 *      bidiNeeded      - [out] TRUE if bidi reordering is needed
 *      direction       - [in] explicit direction (TS_LTR_DIRECTION or TS_RTL_DIRECTION); TS_NEUTRAL_DIRECTION if not explicit
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP shaperbidi>
 */
TS_EXPORT(TsResult)
TsShaper_bidiSetLevels(TsShaper *shaper, TsShaperText *shaperText, TsInt32 startIndex, TsInt32 endIndex, TsUInt8 *baseLevel, TsBool *bidiNeeded, TsTextDirection direction);


/*****************************************************************************
 *
 *  Mirrors any glyphs that are required to be mirrored per Bidi rule L4.
 *
 *  Description:
 *      This should be called after TsShaper_bidiSetLevels(), but before
 *      TsShaper_shape() to ensure that the OpenType shaping is done with
 *      the proper glyphs.
 *
 *  Parameters:
 *      shaper          - [in] this
 *      shaperText      - [in/out] pointer to TsShaperText object
 *      startIndex      - [in] the index at which to start processing
 *      endIndex        - [in] the index at which to stop processing
 *
 *  Return value:
 *      TsResult- TS_OK if all went well, error code otherwise
 *
 *
 *  <GROUP shaperbidi>
 */
TS_EXPORT(TsResult)
TsShaper_bidiMirrorChars(TsShaper *shaper, TsShaperText *shaperText, TsInt32 startIndex, TsInt32 endIndex);


/*****************************************************************************
 *
 *  Reorders the text in a line based on bidi levels.
 *
 *  Description:
 *      Reorders the characters within a line based on their bidi levels.
 *      This should be called after TsShaper_shape().
 *
 *  Parameters:
 *      shaper          - [in] this
 *      shaperText      - [in/out] pointer to TsShaperText object
 *      startIndex      - [in] the index at which to start processing
 *      endIndex        - [in] the index at which to stop processing
 *      baseLevel       - [in] the bidi base level of the line
 *
 *  Return value:
 *      TsResult        - TS_OK on success;
*                         TS_ERR_ARGUMENT_IS_NULL_PTR if shaper is NULL;
*                         TS_ERR_MALLOC_FAIL on out of memory condition
 *
 *  <GROUP shaperbidi>
 */
TS_EXPORT(TsResult)
TsShaper_bidiReorder(TsShaper *shaper, TsShaperText *shaperText, TsInt32 startIndex, TsInt32 endIndex, TsUInt8 baseLevel);


#endif /* TS_NO_BIDIRECTIONAL */


/*****************************************************************************
 *
 *  Sets the language.
 *
 *  Description:
 *      Sets the language tag to be used for OpenType shaping (e.g. to
 *      differentiate between Hindi and Marathi glyphs in a Devanagari font,
 *      or between Arabic and Farsi glyphs in an Arabic font).
 *
 *      By default this is set to 0.
 *
 *      See otlangs.h for language tag definitions.
 *
 *  Parameters:
 *      shaper      - [in] this
 *      script      - [in] the language TsTag value to set
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if shaper is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsResult)
TsShaper_optionsSetLanguage(TsShaper *shaper, TsTag language);


/******************************************************************************
 *
 *  Enumeration of boolean shaping option tags.
 *
 *  Description:
 *      The default value for all boolean options is FALSE. The name of the
 *      option describes the affect of setting a specific option to TRUE.
 *
 */
typedef enum
{
    TS_SHAPER_DISABLE_BIDI = 0,                    /* Disables the bidirectional algorithm */
    TS_SHAPER_DISABLE_COMPLEX_SHAPING,             /* Disables all complex shaping */
    TS_SHAPER_DISABLE_OPENTYPE_LAYOUT,             /* Disables OpenType layout table processing */
    TS_SHAPER_DISABLE_THAI_LINE_BREAKING,          /* Disables the Thai dictionary-based line breaking algorithm */
    TS_SHAPER_ENABLE_KERNING                       /* Enables old kern table processing */
} TsShaperOptionsTag;


/*****************************************************************************
 *
 *  Sets the value of a single option with TsBool type.
 *
 *  Description:
 *      Use this function to set any of the options that have boolean type.
 *
 *      See table below for a description of tags that can be used with
 *      this function. Unsupported tags are ignored.
 *
 *      <TABLE>
 *      <B>Tag</B>                              <B>Description</B>
 *      TS_SHAPER_DISABLE_BIDI                  Disables the bidirectional algorithm
 *      TS_SHAPER_DISABLE_COMPLEX_SHAPING       Disables all complex shaping
 *      TS_SHAPER_DISABLE_OPENTYPE_LAYOUT       Disables OpenType layout table processing
 *      TS_SHAPER_DISABLE_THAI_LINE_BREAKING    Disables the Thai dictionary-based line breaking algorithm
 *      TS_SHAPER_ENABLE_KERNING                Enables old kern table processing
 *      </TABLE>
 *
 *  Parameters:
 *      shaper      - [in] this
 *      tag         - [in] tag that indicates which option is being set
 *      value       - [in] value of option
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if shaper is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsResult)
TsShaper_optionsSet(TsShaper *shaper, TsShaperOptionsTag tag, TsBool value);


/* Line break model macros. See TsShaper_optionsSetLineBreakModel for description */

#define TS_LINE_BREAK_BIT_USE_LEGAL_LINE_BREAK_IF_ABLE 0x0001 /* try to line break at legal (per Unicode) line break points */
#define TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GRAPHEME      0x0002 /* OK to break at grapheme boundaries if not able (or not allowed) to break at legal line break points */
#define TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GLYPH         0x0004 /* OK to break at glyphs if not able (or not allowed) to break at legal points or grapheme boundaries */
#define TS_LINE_BREAK_BIT_KOREAN_USES_EAST_ASIAN_STYLE 0x0008 /* East Asian style = OK to break within a word, default is Western style (break after spaces, etc.) */
#define TS_LINE_BREAK_BIT_WRAP_WHITESPACE              0x0010 /* wrap trailing whitespace at end of line to next line if it passes composition width */
#define TS_LINE_BREAK_BIT_HIDE_INITIAL_WRAPPED_WHITESPACE 0x0020 /* hide initial whitespace of current line if it was wrapped from previous line */
#define TS_TRUNCATE_ON_GRAPHEME                           0x0040 /* truncate on grapheme boundary, even if it is not a valid line break point in the current line model */
#define TS_LINE_BREAK_LEGAL_ELSE_GRAPHEME            (TS_LINE_BREAK_BIT_USE_LEGAL_LINE_BREAK_IF_ABLE | TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GRAPHEME)
#define TS_LINE_BREAK_LEGAL_ELSE_ERROR               (TS_LINE_BREAK_BIT_USE_LEGAL_LINE_BREAK_IF_ABLE)
#define TS_LINE_BREAK_GRAPHEME                       (TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GRAPHEME)


/*****************************************************************************
 *
 *  Typedef for a bitfield that contains the line break model.
 *
 *  <GROUP shaperoptions>
 */
typedef TsUInt32 TsLineBreakModel;


/*****************************************************************************
 *
 *  Sets the line break model.
 *
 *  Description:
 *      This option exposes control over how lines will break.
 *
 *      The 'model' parameter is treated as a bitfield. Construct the bitfield
 *      by logically ORing from the choices in the table below:
 *
 *      <TABLE>
 *      <B>Line Break Model</B>                             <B>Description</B>
 *      TS_LINE_BREAK_BIT_USE_LEGAL_LINE_BREAK_IF_ABLE      try to line break at legal (per Unicode) line break points
 *      TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GRAPHEME           OK to break at grapheme boundaries if not able (or not allowed) to break at legal line break points
 *      TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GLYPH              OK to break at glyphs if not able (or not allowed) to break at legal points or grapheme boundaries
 *      TS_LINE_BREAK_BIT_KOREAN_USES_EAST_ASIAN_STYLE      East Asian style = OK to break within a word, default is Western style (break after spaces, etc.)
 *      TS_LINE_BREAK_BIT_WRAP_WHITESPACE                   wrap trailing whitespace at end of line to next line if the whitespace exceeds composition width
 *      TS_LINE_BREAK_BIT_HIDE_FIRST_WRAPPED_WHITESPACE     hide the first trailing whitespace if it would otherwise have wrapped
 *      </TABLE>
 *
 *      The default line break model = (TS_LINE_BREAK_BIT_USE_LEGAL_LINE_BREAK_IF_ABLE | TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GRAPHEME)
 *
 *      Note that most of these bits have no effect on WT Shaper,
 *      only on WTLE's line breaking algorithm.  However
 *      TS_LINE_BREAK_BIT_KOREAN_USES_EAST_ASIAN_STYLE and
 *      TS_LINE_BREAK_BIT_WRAP_WHITESPACE do affect where potential linebreaks
 *      are flagged by WT Shaper.
 *
 *  Parameters:
 *      shaper      - [in] this
 *      model       - [in] the line break model to be set
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if shaper is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsResult)
TsShaper_optionsSetLineBreakModel(TsShaper *shaper, TsLineBreakModel model);


/*****************************************************************************
 *
 *  Sets the base character for isolated diacritics.
 *
 *  Description:
 *      Sets the base character that will be inserted for any isolated
 *      diacritics that appear in the text.
 *
 *      The default is TS_CHAR_NO_BREAK_SPACE (U+00A0).
 *
 *  Parameters:
 *      shaper      - [in] this
 *      baseChar    - [in] Unicode value of the character to be inserted
 *                         as a base character for isolated diacritics
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if shaper is NULL;
 *                  TS_OK otherwise.
 *
 *  Notes:
 *      Passing U+0000 for baseChar, will result in the base character being
 *      set to the default value (U+00A0).
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsResult)
TsShaper_optionsSetBaseChar(TsShaper *shaper, TsUInt32 baseChar);


/*****************************************************************************
 *
 *  Sets the default script.
 *
 *  Description:
 *      Sets the OpenType script tag that will be used for isolated runs of
 *      characters that have the "Common" or "Inherited" Unicode script
 *      property.
 *
 *      For more information, see:
 *          http://unicode.org/Public/UNIDATA/Scripts.txt
 *          http://www.unicode.org/reports/tr44/
 *          http://www.microsoft.com/typography/OTSPEC/scripttags.htm
 *
 *      The default is TsTag_DFLT.
 *
 *      See otscript.h for script tag definitions.
 *
 *  Parameters:
 *      shaper      - [in] this
 *      script      - [in] the script TsTag value to set
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if shaper is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsResult)
TsShaper_optionsSetDefaultScript(TsShaper *shaper, TsTag script);


/*****************************************************************************
 *
 *  Gets the language.
 *
 *  Description:
 *      Gets the language tag.
 *
 *  Parameters:
 *      shaper      - [in] this
 *
 *  Return value:
 *      TsTag    - language tag.
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsTag)
TsShaper_optionsGetLanguage(TsShaper *shaper);


/*****************************************************************************
 *
 *  Gets the value of a single option with TsBool type.
 *
 *  Description:
 *      Use this function to get any of the options that have boolean type.
 *
 *      See TsShaper_optionsSet for a table of tags that can be used with
 *      this function.
 *
 *  Parameters:
 *      shaper      - [in] this
 *      tag         - [in] tag that indicates which option is being queried
 *
 *  Return value:
 *      TsBool      - value of option
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsBool)
TsShaper_optionsGet(TsShaper *shaper, TsShaperOptionsTag tag);


/*****************************************************************************
 *
 *  Gets the line break model.
 *
 *  Description:
 *      Gets the line break model. The returned value is a bitfield of the
 *      currently set models OR'd together.
 *
 *  Parameters:
 *      shaper              - [in] this
 *
 *  Return value:
 *      TsLineBreakModel    - the line break model
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsLineBreakModel)
TsShaper_optionsGetLineBreakModel(TsShaper *shaper);


/*****************************************************************************
 *
 *  Gets the base character for isolated diacritics.
 *
 *  Description:
 *      Gets the base character that will be inserted for any isolated
 *      diacritics that appear in the text.
 *
 *  Parameters:
 *      shaper      - [in] this
 *
 *  Return value:
 *      TsUInt32    - the Unicode value of the character to be inserted
 *                    as a base character for isolated diacritics
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsUInt32)
TsShaper_optionsGetBaseChar(TsShaper *shaper);


/*****************************************************************************
 *
 *  Gets the default script.
 *
 *  Description:
 *      Gets the default script tag.
 *
 *  Parameters:
 *      shaper  - [in] this
 *
 *  Return value:
 *      TsTag   - the script TsTag value.
 *
 *  <GROUP shaperoptions>
 */
TS_EXPORT(TsTag)
TsShaper_optionsGetDefaultScript(TsShaper *shaper);


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
 *      becomes invalid when this TsShaper object is destroyed.
 *      Therefore, you must keep a reference to the TsShaper object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      shaper     - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP shaper>
 */
TS_EXPORT(TsExternalUserData *)
TsShaper_getExternalUserdata(TsShaper *shaper);

TS_END_HEADER

#endif /* TSSHAPER_H */
