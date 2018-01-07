/*****************************************************************************
 *
 *  tslayopt.h  - LayoutOptions object interface.
 *
 *  Copyright (C) 2005-2007, 2008, 2012 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSLAYOUTOPTIONS_H
#define TSLAYOUTOPTIONS_H

#include "tstypes.h"
#include "tsevent.h"
#include "tsfixed.h"
#include "ts2d.h"
#include "tstag.h"
#include "tsrsrc.h"
#include "tsmem.h"
#include "tsdynarr.h"
#include "tsshaper.h" /* for TsLineBreakModel */
#include "tssubsdi.h"
#include "tseudata.h"

TS_BEGIN_HEADER


/******************************************************************************
 *
 *  Macro indicating the maximum number of tab stops that WTLE supports.
 *
 *  <GROUP layoutoptions>
 */
#define TS_MAX_TABSTOP_COUNT 254


/******************************************************************************
 *
 *  Macro indicating left alignment of a tabstop.
 *
 *  <GROUP layoutoptions>
 */
#define TS_TABSTOP_ALIGNMENT_LEFT 0


/******************************************************************************
 *
 *  Macro indicating right alignment of a tabstop.
 *
 *  <GROUP layoutoptions>
 */
#define TS_TABSTOP_ALIGNMENT_RIGHT 1


/******************************************************************************
 *
 *  Macro indicating center alignment of a tabstop.
 *
 *  <GROUP layoutoptions>
 */
#define TS_TABSTOP_ALIGNMENT_CENTER 2





/******************************************************************************
 *
 *  Enumeration of highlighting modes.
 *
 *  Description:
 *      Text in the current selection is highlighted upon display according
 *      to this mode.
 *
 *      The default is reverse video. The display code will call
 *      TsDC_highlightRect for the selected rectangles. Proper behavior
 *      requires that the class derived from TsDC implement the highlightRect
 *      function. Everything in the selection rectangles is displayed with
 *      reverse video.
 *
 *      Color mode highlights text by drawing a colored background and
 *      and then colored text. The background and text colors are
 *      set using TsLayoutOptions_setHighlightBackgroundColor() and
 *      TsLayoutOptions_setHighlightTextColor(). The colors of edge effects,
 *      underline, strikeout, and overline are not affected by this mode.
 *      The color of inline images is also not affected.
 *
 *      Highlighting may be disabled using TS_HIGHLIGHT_DISABLED.
 *
 *
 *  <GROUP layoutoptions>
 */
typedef enum
{
    TS_HIGHLIGHT_REVERSE_VIDEO_MODE = 0,     /* selected text displayed reverse video */
    TS_HIGHLIGHT_COLOR_MODE,                 /* selected text displayed with user-specified colors */
    TS_HIGHLIGHT_DISABLED,                   /* selected text displayed without highlighting */
    TS_HIGHLIGHT_MAINTAIN_TEXT_COLOR_MODE    /* selected text displayed in original color with user-specified background */
} TsHighlightMode;


/******************************************************************************
 *
 *  Enumeration of boolean layout option tags.
 *
 *  Description:
 *      The default value for all boolean options is FALSE. The name of the
 *      option describes the affect of setting a specific option to TRUE.
 *
 *      <B>Layout algorithm options</B> - enable/disable specific layout algorithms.
 *      Most are self explanatory. The option TS_LIMIT_BOUNDING_BOX_TO_COMP_WIDTH
 *      ensures that the full glyph fits within the composition width or the
 *      the text container's x-direction boundaries. This is not recommended
 *      for general use since text will not line up properly. It may be useful
 *      when the certainty of text fitting within a container is more important
 *      than typographic accuracy. Note that some algorithms may be disabled
 *      at build time as well in order to reduce code size.
 *
 *      <B>Substitution dictionaries</B> - switches to enable/disable the various
 *      dictionaries. To be fully active a dictionary must (1) be added
 *      to its owner object using either TsLayoutControl_addDictionary or
 *      TsLayoutOptions_addDictionary and (2) be enabled using this
 *      function with the appropriate tag.
 *
 *      <B>Debugging option</B> - switch to enable printing of the information
 *      about the glyphs on each line of the layout. Enabled in debug builds
 *      only.  The data is output using TS_PRINTF_DEBUG so that macro
 *      must also be defined to something other than a no-op to see the output.
 *
 *      <B>Draw options</B> - these are used for diagnostic purposes. Certain
 *      marks, lines, and boxes will be drawn in the text area to show
 *      bounding box, baseline, text direction, and so forth. The build
 *      option TS_ENABLE_LAYOUT_MARKS must be defined as well.
 *
 *  <GROUP layoutoptions>
 */
typedef enum
{
    /* Enable/disable specific layout algorithms */
    TS_DISABLE_BIDI = 0,                    /* Disables the bidirectional algorithm */
    TS_DISABLE_COMPLEX_SHAPING,             /* Disables all complex shaping */
    TS_DISABLE_NORMALIZATION,               /* Disables Unicode text normalization */
    TS_DISABLE_OPENTYPE_LAYOUT,             /* Disables OpenType layout table processing */
    TS_DISABLE_DIACRITICS_POSITIONING,      /* Disables the diacritics positioning system */
    TS_DISABLE_THAI_LINE_BREAKING,          /* Disables the Thai dictionary-based line breaking algorithm */
    TS_ENABLE_KERNING,                      /* Enables pairwise kerning if supported by font engine (not yet supported by layout engine) */
    TS_LIMIT_BOUNDING_BOX_TO_COMP_WIDTH,    /* Ensures that the text bounding box does not exceed the width of the text container */
    TS_ENABLE_FORCED_MONOSPACE,             /* Enables forced monospace mode, for use in spacing grapheme choices for multi-tap input */
    TS_LIMIT_WIDTH_TO_LONGEST_LINEBREAK_SEQUENCE, /* Limits composition width to that of longest linebreak sequence (for use in block layout mode only) */
    TS_TRUNCATE_ON_TAB,                     /* Causes truncation to occur when a tab character is encountered */
    TS_ENABLE_EXACT_BOUNDING_BOX,           /* Enables use of exact bounding box in layout computations */

    /* Enable/disable the application of algorithms that would not normally be run based on the properties of the text */
    TS_FORCE_NFC,                           /* Forces NFC, for cases where cases where composition would not normally occur, but is desired */
    TS_FORCE_FULL_SHAPING,                  /* Forces full shaping of text, including OpenType shaping (where applicable), even for scripts like Latin and CJK that don't normally need it */

    /* Enable substitution dictionaries */
    TS_ENABLE_SUBSTITUTION_DICTIONARY_1,    /* Enable substitution dictionary 1 (owned by TsLayoutControl) */
    TS_ENABLE_SUBSTITUTION_DICTIONARY_2,    /* Enable substitution dictionary 2 (owned by TsLayoutControl) */
    TS_ENABLE_SUBSTITUTION_DICTIONARY_A,    /* Enable substitution dictionary A (owned by TsLayoutOptions) */
    TS_ENABLE_SUBSTITUTION_DICTIONARY_B,    /* Enable substitution dictionary B (owned by TsLayoutOptions) */

    /* Enable/disable character substitution when there is an inline image problem during compose */
    TS_NO_CHAR_SUBSTITUTION_IF_INLINE_IMAGE_LOAD_FAILS, /* If an inline image fails, show the original character sequence instead of doing character substitution. */

    /* Enable/disable insertion of truncation character */
    TS_INSERT_TRUNCATION_CHAR_IF_LINE_FEED,         /* Enables insertion of the truncation character when a line feed is encountered (applies only to string-based layout) */
    TS_INSERT_TRUNCATION_CHAR_IF_CONTAINER_OVERFLOW,/* Enables insertion of the truncation character when text exceeds the available space */

    /* Enable printing of glyph information for each line */
    TS_DUMP_ALL_LINE_INFORMATION,           /* For each line, print the details of each glyph using the TS_PRINTF_DEBUG macro */

    /* Enable draw options - used for layout diagnostics */
    TS_DRAW_LINE_ADVANCE_BOX,               /* For each text line draw a rectangle showing its advance box */
    TS_DRAW_LINE_BOUNDING_BOX,              /* For each text line draw a rectangle showing the bounding box */
    TS_DRAW_BASELINE,                       /* For each line draw the text baseline */
    TS_DRAW_TEXT_DIRECTION,                 /* Draw a colored line indicating the text direction */
    TS_DRAW_POTENTIAL_LINE_BREAKS,          /* Draw a vertical line between graphemes where line breaks can occur */
    TS_DRAW_WORD_BOUNDARIES,                /* Draw a vertical line at each word boundary */
    TS_DRAW_SHAPING_BOUNDARIES,             /* Draw a vertical line at each shaping boundary */
    TS_DRAW_TEXT_CONTAINERS,                /* Draw a rectangle at edge of text containers */
    TS_DRAW_ANCHORED_OBJECTS,               /* Draw a rectangle at edge of anchored objects */

    /* Enable/disable highlighting behavior */
    TS_HIGHLIGHT_TO_CONTAINER_WIDTH,        /* Text selection will extend to full container width */


    /* Note: options below this point are tracked using the flags2 member of the TsLayoutOptions structure. If
             new option(s) are inserted above, the option above will shift below this lines and any use of the
             internal TsLayoutOptions_get1 macro for that option will have to be switched to TsLayoutOptions_get2. */

    TS_REDUCE_MEMORY_USAGE,                   /* If TRUE, memory usage will be reduced which may incur an increase in the number of mallocs and frees, and a decrease in performance */

    /* The below flag is a derived flag which is for internal use only.
       Clients should not directly set this flag; but can get its value. */

    TS_EXACT_BOUNDING_BOX_REQUIRED,          /* If TRUE, the exact bounding box will be used in layout calculations. Will be set to TRUE if TS_ENABLE_EXACT_BOUNDING_BOX, TS_LIMIT_BOUNDING_BOX_TO_COMP_WIDTH, or TS_ENABLE_FORCED_MONOSPACE is TRUE */

    /* The below is an internal flag that should not be accessed by clients. */

    TS_ELLIPSIZE_IN_MIDDLE                   /* If TRUE, the string will be composed such that the truncation character (or ellipsis if none set) will be placed in the middle. */

} TsLayoutOptionsTag;

/* Helper macros that define categories of flags.
   Note: These are dependent on the order in the above enumeration.
   Note: The flag (f) passed to these macros is accessed more than once, so f should not be an evaluated expression. */
#define IS_DRAW_FLAG(f) (((f>=TS_DRAW_LINE_ADVANCE_BOX) && (f<=TS_DRAW_ANCHORED_OBJECTS)) ? TRUE : FALSE)
#define IS_SUBS_FLAG(f) (((f>=TS_ENABLE_SUBSTITUTION_DICTIONARY_1) && (f<=TS_ENABLE_SUBSTITUTION_DICTIONARY_B)) ? TRUE : FALSE)

/* Macro that defines the last option in the above enumeration that can be set by the client */
#define TS_LAST_OPTION  TS_REDUCE_MEMORY_USAGE

typedef TsUInt32 TsOptionsFlagType;


/*****************************************************************************
 *
 *  TsLayout options object
 *
 *  Description:
 *      The TsLayoutOptions object manages options for controlling layout
 *      algorithms, editing and cursor behavior and drawing behavior. The
 *      object may also store two substitution dictionaries. Generally,
 *      one or more TsLayoutOptions objects are prepared in advance of
 *      the layout operations. The user may wish to prepare a specific
 *      set of options for each use scenario or application.
 *
 *      When pointer to a TsLayoutOptions object is passed to the TsLayout
 *      constructor. The TsLayout registers as an observer of the TsLayoutOptions
 *      object. If any changes are made to the TsLayoutOptions object
 *      a message is sent to all observers indicating the change.
 *
 *  <GROUP layoutoptions>
 */
typedef struct TsLayoutOptions_ TsLayoutOptions;


/*****************************************************************************
 *
 *  Creates a new TsLayoutOptions object and initializes it to default values.
 *
 *  Parameters:
 *      none
 *
 *  Return value:
 *      Pointer to new TsLayoutOptions object.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsLayoutOptions *)
TsLayoutOptions_new(void);


/*****************************************************************************
 *
 *  Creates a new memory managed TsLayoutOptions object and initializes it to
 *  default values.
 *
 *  Parameters:
 *      memMgr     - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new TsLayoutOptions object.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsLayoutOptions *)
TsLayoutOptions_newex(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Initializes a TsLayoutOptions object to default values.
 *
 *  Description:
 *
 *      <TABLE>
 *      <B>Option                   <B>Default value
 *      All TRUE/FALSE options      FALSE
 *      HighlightBackgroundColor    TS_COLOR_WHITE
 *      HighlightColorMode          TS_HIGHLIGHT_REVERSE_VIDEO_MODE
 *      HighlightTextColor          TS_COLOR_BLACK
 *      HyphenChar                  0, which indicates no hyphen characer is inserted
 *      LineBreakModel              TS_LINE_BREAK_BIT_USE_LEGAL_LINE_BREAK_IF_ABLE | TS_LINE_BREAK_BIT_OK_TO_BREAK_ON_GRAPHEME
 *      TabStop                     No explicit tab stops
 *      TabStopPositionDefault      0
 *      TruncationCharacter         0, which indicates no truncations character is inserted
 *      MonospaceMinPadding         1.0 pixel on each side
 *      MonospaceMinWidth           0.0 pixels minimum width
 *      BaseChar                    TS_CHAR_NO_BREAK_SPACE
 *      DefaultScript               TsTag_DFLT
 *
 *      </TABLE>
 *
 *  Parameters:
 *      options     - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(void)
TsLayoutOptions_defaults(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsLayoutOptions object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsLayoutOptions_releaseHandle.
 *
 *  Parameters:
 *      options     - [in] this
 *
 *  Return value:
 *      Pointer to a layout options object.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsLayoutOptions *)
TsLayoutOptions_copyHandle(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsLayoutOptions object.
 *
 *  Description:
 *      This function reverses the action of TsLayoutOptions_copyHandle.
 *
 *  Parameters:
 *      options          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(void)
TsLayoutOptions_releaseHandle(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Frees a TsLayoutOptions object.
 *
 *  Parameters:
 *     options      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(void)
TsLayoutOptions_delete(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Register an observer with the layout options object.
 *
 *  Parameters:
 *      options         - [in] this
 *      observer        - [in] pointer to object that is observing the layout
 *                             options
 *      callback        - [in] callback function that is called when something
 *                             about the layout options changes
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if any argument is NULL.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_registerObserver(TsLayoutOptions *options, void *observer, TsEventCallback callback);


/*****************************************************************************
 *
 *  Unregister an observer with the layout options object.
 *
 *  Parameters:
 *      options         - [in] this
 *      observer        - [in] pointer to object that is observing the layout
 *                             options
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if either argument is NULL.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_unregisterObserver(TsLayoutOptions *options, void *observer);


/*****************************************************************************
 *
 *  Sets the value of a single option with TsBool type.
 *
 *  Description:
 *      Use this function to set any of the options that have boolean type.
 *      See TsLayoutOptionsTag for a description of tags that
 *      can be used with this function. Unsupported tags are ignored.
 *
 *  Parameters:
 *      options     - [in] this
 *      tag         - [in] tag that indicates which option is being set
 *      value       - [in] value of option
 *
 *  Return value:
 *      none
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_set(TsLayoutOptions *options, TsLayoutOptionsTag tag, TsBool value);



/*****************************************************************************
 *
 *  Sets the highlight background color.
 *
 *  Description:
 *      Sets the background color of selected text. The highlight mode must
 *      be set to TS_HIGHLIGHT_COLOR_MODE for this to take effect. The default
 *      is white (TS_COLOR_WHITE). See also TsHighlightMode.
 *
 *  Parameters:
 *      options     - [in] this
 *      value       - [in] the color of the background of a text selection
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
*/
TS_EXPORT(TsResult)
TsLayoutOptions_setHighlightBackgroundColor(TsLayoutOptions *options, TsColor value);


/*****************************************************************************
 *
 *  Sets the highlight mode.
 *
 *  Description:
 *      Determines the highlighting behavior when text is selected.
 *
 *  Parameters:
 *      options     - [in] this
 *      mode        - [in] the highlight mode to set (see TsHighlightMode)
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setHighlightMode(TsLayoutOptions *options, TsHighlightMode mode);


/*****************************************************************************
 *
 *  Sets the highlight text color.
 *
 *  Description:
 *      Sets the text color of selected text. The highlight mode must
 *      be set to TS_HIGHLIGHT_COLOR_MODE for this to take effect. The default
 *      is black (TS_COLOR_BLACK). See also TsHighlightMode.
 *
 *  Parameters:
 *      options     - [in] this
 *      value       - [in] the color of the text of a text selection
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setHighlightTextColor(TsLayoutOptions *options, TsColor value);


/*****************************************************************************
 *
 *  Sets the hyphenation character.
 *
 *  Description:
 *      Sets which character, if any, is inserted at the end of a line to
 *      indicate hyphenation. The default value is 0, which indicates that no
 *      hyphen character is inserted. To visually indicate hyphenation, set the
 *      value to a valid Unicode character.
 *      For example, setting the value to U+002D will cause the HYPHEN-MINUS
 *      character to be inserted at the end of hyphenated lines.
 *      The actual glyph comes from the font in effect at the point of hyphenation.
 *      See also TsLineBreakModel.
 *
 *  Parameters:
 *      options     - [in] this
 *      hyphenChar  - [in] the Unicode value of the character to use for
 *                         hyphenation
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setHyphenChar(TsLayoutOptions *options, TsUInt32 hyphenChar);






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
 *      <B> </B>                                            <B> </B>
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
 *  Version:
 *      In Version 2.2 and prior releases the TsLineBreakModel was defined
 *      as an enumerated list. In Version 3.0 the TsLineBreakModel was changed to a bit
 *      field to allow more flexible selection of various line break option combinations.
 *      Backward compatibility was maintained by defining a macro for each of the former
 *      enum elements as in the table below. These names are deprecated in favor of
 *      the bit field macros described above.
 *
 *      <TABLE>
 *      TS_LINE_BREAK_LEGAL_ELSE_GRAPHEME       break on legal line break point if possible, otherwise break on grapheme boundary
 *      TS_LINE_BREAK_LEGAL_ELSE_ERROR          break only on legal line break points, return an error if not possible to fit
 *      TS_LINE_BREAK_GRAPHEME                  break on grapheme boundary, even if it is not a legal break point
 *      </TABLE>
 *
 *  Parameters:
 *      options     - [in] this
 *      model       - [in] the line break model to be set
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setLineBreakModel(TsLayoutOptions *options, TsLineBreakModel model);


/*****************************************************************************
 *
 *  Sets a tab stop.
 *
 *  Parameters:
 *      options         - [in] this
 *      position        - [in] location of tab stop
 *      alignment       - [in] the alignment of the tab stop
 *      leader          - [in] leader character (not supported)
 *
 *  Return value:
 *      TsResult        - TS_OK if the function succeeds.
 *                        TS_ERR_ARGUMENT_OUT_OF_RANGE if the alignment is not
 *                          one of TS_TABSTOP_ALIGNMENT_LEFT,
 *                          TS_TABSTOP_ALIGNMENT_RIGHT,
 *                          or TS_TABSTOP_ALIGNMENT_CENTER.
 *
 *  Notes:
 *      The following tab stop functionality is supported:
 *          Setting of Left, Right, or Center aligned tab stops, when the text
 *          alignment matches the inherent directionality of the text (left
 *          aligned LTR text, and right aligned RTL text).
 *
 *          The position of the tab stop is determined by the base directionality
 *          of the text. For RTL text, the position is measured from the right
 *          edge of the layout.
 *
 *      Setting of the Leader character is not supported in this release. The
 *      value passed in is ignored when creating the layout. The value passed
 *      to this function will be returned by a call to TsLayoutOptions_getTabStop.
 *
 *      If the passed in position matches that of an existing tab stop, the
 *      new information being passed in overrides (replaces) the existing
 *      tab stop.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setTabStop(TsLayoutOptions *options, TsFixed position, TsUInt8 alignment, TsUInt16 leader);


/*****************************************************************************
 *
 *  Removes a tab stop.
 *
 *  Parameters:
 *      options         - [in] this
 *      index           - [in] the index of the tab stop to remove
 *
 *  Return value:
 *      TsResult        - TS_OK if the function succeeds;
 *                        TS_ERR_ARGUMENT_OUT_OF_RANGE if there is no tab stop
 *                        at the given index.
 *
 *  Notes:
 *         To remove all the current tab stops, the client should call this
 *         function in a loop until it returns TS_ERR_ARGUMENT_OUT_OF_RANGE.
 *         Note that when a tab stop is removed, that (the index of) any tabs
 *         stops with an index greater than the one passed in will be
 *         shifted down.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_clearTabStop(TsLayoutOptions *options, TsUInt8 index);


/*****************************************************************************
 *
 *  Sets the default tab stop position.
 *
 *  Description:
 *      The function is used to set the default tab stop position. The default
 *      value is 0.  Set this value to control the amount of space inserted
 *      for a tab character.
 *
 *  Parameters:
 *      options     - [in] this
 *      pos         - [in] the tab stop position to be set
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setTabStopPositionDefault(TsLayoutOptions *options, TsFixed pos);


/*****************************************************************************
 *
 *  Sets the truncation character.
 *
 *  Description:
 *      Sets which character, if any, is inserted when text is truncated
 *      and when the proper switch is set to enable this insertion.
 *
 *      The default value is 0, which indicates that no character
 *      is inserted. To visually indicate truncation, set the value to a
 *      valid Unicode character.  For example, setting the value to U+2026
 *      will insert a HORIZONTAL ELLLIPSIS (...) character at the end of
 *      truncated text. The actual glyph comes from the font in effect
 *      at the point of truncation.
 *
 *      If you want to insert the truncation character when the layout exceeds the available space then you must also set:
 *      TsLayoutOptions_set(options, TS_INSERT_TRUNCATION_CHAR_IF_CONTAINER_OVERFLOW, TRUE);
 *      Overflow affects only certain layout modes as described in this table:
 *
 *      <TABLE>
 *      <B>Layout mode          <B>Reason
 *      Block-based             Will not truncate due space considerations
 *      Line-based              Will not truncate since container concept is owned by client
 *      Container-based         Will truncate if available containers are full
 *      String-based            Will truncate if using TsLayout_composeString and maxWidth is specified.
 *                              Will not truncate if using TsLayout_drawString.
 *      </TABLE>
 *
 *      If you want to insert the truncation character when a line feed is enountered then you must also set:
 *      TsLayoutOptions_set(options, TS_INSERT_TRUNCATION_CHAR_IF_LINE_FEED, TRUE);
 *      This option only applies to string-based layout.
 *
 *      Note that the truncation character is not inserted if the text truncates due to a tab character.
 *      See also TsLayoutOptions_set and TS_TRUNCATE_ON_TAB.
 *
 *  Parameters:
 *      options         - [in] this
 *      truncationChar  - [in] the Unicode value of the character to be
 *                             inserted when truncation occurs
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setTruncationChar(TsLayoutOptions *options, TsUInt32 truncationChar);


/******************************************************************************
 *
 *  Enumeration of single line truncation options.
 *
 *  Description:
 *      Determines special truncation behavior for container based layouts with
 *      a single container that have the maximum number of lines set to one.
 *      This option applies only to layouts that meet these criteria.
 *      With TS_SINGLE_LINE_TRUNCATION_NONE, the regular truncation mechanism is in effect.
 *      Any other value overrides the default truncation mechanism, and determines
 *      placement of the truncation character as indicated.
 *
 *      As with the regular truncation algorithm, a truncation character must
 *      be set via a call to TsLayoutOptions_setTruncationChar.
 *
 *  <GROUP layoutoptions>
 */
typedef enum
{
    TS_SINGLE_LINE_TRUNCATION_NONE = 0,         /* Default. */
    TS_SINGLE_LINE_TRUNCATION_BEGINNING,        /* place truncation character at beginning */
    TS_SINGLE_LINE_TRUNCATION_MIDDLE,           /* place truncation character in middle */
    TS_SINGLE_LINE_TRUNCATION_END               /* place truncation character at end */
} TsSingleLineTruncation;


/*****************************************************************************
 *
 *  Sets the single line container truncation mode.
 *
 *  Description:
 *    Sets the single line container truncation mode. This value deterimines
 *    the truncation behavior for container based layouts with a single
 *    container that has a maximum number of lines set to one. For layouts
 *    that meet these criteria, the values have the effect described above.
 *
 *  Parameters:
 *      options         - [in] this
 *      truncationMode  - [in] an TsSingleLineTruncation value
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                    TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setSingleLineTruncation(TsLayoutOptions *options, TsSingleLineTruncation truncationMode);


/*****************************************************************************
 *
 *  Sets the monospace minimum padding.
 *
 *  Description:
 *      Sets the minimum amount of padding (in fractional pixels) to add on
 *      each side of a grapheme in monospace mode, i.e.:
 *      newWidth = minPadding + oldWidth + minPadding;
 *
 *      The default is 1.0 pixels of padding on each side.
 *
 *  Parameters:
 *      options         - [in] this
 *      minPadding      - [in] fractional pixels of padding
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setMonospaceMinPadding(TsLayoutOptions *options, TsFixed minPadding);


/*****************************************************************************
 *
 *  Sets the monospace minimum width.
 *
 *  Description:
 *      Sets the minimum width (in fractional pixels) to use for monospace
 *      spacing, i.e. if no grapheme cluster is wider than minWidth, then
 *      minWidth is what will be used for the spacing calculations.
 *
 *      The default is 0.0 pixels minimum width.
 *
 *  Parameters:
 *      options         - [in] this
 *      minWidth        - [in] minimum width to use in fractional pixels
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setMonospaceMinWidth(TsLayoutOptions *options, TsFixed minWidth);


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
 *      options         - [in] this
 *      baseChar        - [in] the Unicode value of the character to be
 *                             inserted as a base character for isolated
 *                             diacritics
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  Notes:
 *      Passing U+0000 for baseChar, will result in the base character being
 *      set to the default value (U+00A0).
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setBaseChar(TsLayoutOptions *options, TsUInt32 baseChar);


/*****************************************************************************
 *
 *  Sets the default script for the text.
 *
 *  Description:
 *      Sets the OpenType script tag that will be used for isolated runs
 *      of characters that have the "Common" or "Inherited" Unicode
 *      script property.
 *
 *      For more information, see:
 *          http://unicode.org/Public/UNIDATA/Scripts.txt
 *          http://www.unicode.org/reports/tr44/
 *          http://www.microsoft.com/typography/OTSPEC/scripttags.htm
 *
 *      The default is TsTag_DFLT.
 *
 *  Parameters:
 *      options         - [in] this
 *      defaultScript   - [in] the TsTag value of the default script to
 *                             be applied to the text. See otscript.h for
 *                             the script tag definitions.
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                    TS_OK otherwise.
 *
 *
 *  <GROUP layoutoptions>
 */

TS_EXPORT(TsResult)
TsLayoutOptions_setDefaultScript(TsLayoutOptions *options, TsTag value);


/*****************************************************************************
 *
 *  Gets the value of a single option with TsBool type.
 *
 *  Description:
 *      Returns the value of a single option having a TsBool type.
 *      See TsLayoutOptionsTag for a description of tags that
 *      can be used with this function. If the tag is unsupported
 *      the function returns FALSE.
 *
 *  Parameters:
 *      options         - [in] this
 *      tag             - [in] tag that indicates which option to get
 *
 *  Return value:
 *      Option value
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsBool)
TsLayoutOptions_get(TsLayoutOptions *options, TsLayoutOptionsTag tag);


/*****************************************************************************
 *
 * Macro that evaluates to TRUE if any substitution dictionary is enabled.
 *
 *  Notes:
 *     This macro evaluates the opts argument more than once so the
 *     argument should never be an expression with side effects.
 *
 *  <GROUP layoutoptions>
 */
#define TsLayoutOptions_getSubstitutionsEnabled(opts)                          \
        (TsLayoutOptions_get((opts), TS_ENABLE_SUBSTITUTION_DICTIONARY_1) ||   \
        TsLayoutOptions_get((opts), TS_ENABLE_SUBSTITUTION_DICTIONARY_2) ||    \
        TsLayoutOptions_get((opts), TS_ENABLE_SUBSTITUTION_DICTIONARY_A) ||    \
        TsLayoutOptions_get((opts), TS_ENABLE_SUBSTITUTION_DICTIONARY_B))


/*****************************************************************************
 *
 *  Gets the highlight background color.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The color of the background of a text selection when the highlight
 *      mode is set to TS_HIGHLIGHT_COLOR_MODE.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsColor)
TsLayoutOptions_getHighlightBackgroundColor(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the highlight mode.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The highlight mode.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsHighlightMode)
TsLayoutOptions_getHighlightMode(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the highlight text color.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The color of the text of a text selection when the highlight
 *      mode is set to TS_HIGHLIGHT_COLOR_MODE.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsColor)
TsLayoutOptions_getHighlightTextColor(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the hyphenation character.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The Unicode value of character used for hyphenation. Zero means
 *      that no hyphen character will be inserted.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsUInt32)
TsLayoutOptions_getHyphenChar(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the line breaking model.
 *
 *  Description:
 *      Returns the value of the line break model.
 *      See TsLayoutOptions_setLineBreak model for a description of the
 *      line break model bitfield entries.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The line break model.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsLineBreakModel)
TsLayoutOptions_getLineBreakModel(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets information about a tab stop.
 *
 *  Parameters:
 *      options         - [in]  this
 *      index           - [in]  the index of the tab stop
 *      position        - [out] location of tab stop
 *      alignment       - [out] alignment, one of the TS_TABSTOP_ALIGNMENT_xxx values
 *      leader          - [out] leader character (not supported)
 *
 *  Return value:
 *      TsResult        - TS_OK if the function succeeds.
 *                        TS_ERR_ARGUMENT_OUT_OF_RANGE if there is no tab stop
 *                        at the given index
 *
 *  Notes:
 *      To get the information for all of the current tab stops, call this function
 *      in a loop until it returns TS_ERR_ARGUMENT_OUT_OF_RANGE.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_getTabStop(TsLayoutOptions *options, TsUInt8 index, TsFixed *position, TsUInt8 *alignment, TsUInt16 *leader);


/*****************************************************************************
 *
 *  Gets the default tab stop position.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The default tab stop position.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsFixed)
TsLayoutOptions_getTabStopPositionDefault(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the truncation character.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The Unicode value of the character used when truncation occurs. Zero
 *      means that no truncation character will be inserted.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsUInt32)
TsLayoutOptions_getTruncationChar(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the single line truncation mode.
 *
 *  Description:
 *    Gets the value of the truncation mode in effect for single line
 *    containers.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      TsSingleLineTruncation value
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsSingleLineTruncation)
TsLayoutOptions_getSingleLineTruncation(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the monospace minimum padding.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      Fractional pixels of padding
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsFixed)
TsLayoutOptions_getMonospaceMinPadding(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the monospace minimum width.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      Minimum width (in fractional pixels) to use in monospacing.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsFixed)
TsLayoutOptions_getMonospaceMinWidth(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the base character for isolated diacritics.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The Unicode value of the character used as the base character when
 *      isolated diacritics appears in the text.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsUInt32)
TsLayoutOptions_getBaseChar(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the default script.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The TsTag value of the default script.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsTag)
TsLayoutOptions_getDefaultScript(TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Macro that makes a 'WsdA' tag for use when adding a
 *  substitution dictionary.
 *
 *  <GROUP layoutoptions>
 */
#define TsTag_WsdA TsMakeTag('W','s','d','A')


/*****************************************************************************
 *
 *  Macro that makes a 'WsdB' tag for use when adding a
 *  substitution dictionary.
 *
 *  <GROUP layoutoptions>
 */
#define TsTag_WsdB TsMakeTag('W','s','d','B')


/*****************************************************************************
 *
 *  Adds a substitution dictionary to a TsLayoutOptions object.
 *
 *  Description:
 *
 *      Supported data items:
 *
 *      <TABLE>
 *      <B>Tag          Description</B>
 *      WsdA            Substitution dictionary
 *      WsdB            Substitution dictionary
 *      </TABLE>
*
 *  Parameters:
 *      options     - [in] this
 *      tag         - [in] data identifier
 *      resource    - [in] data file
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_addDictionary(TsLayoutOptions *options, TsTag tag, TsResource *resource);


/*****************************************************************************
 *
 *  Removes a substitution dictionary from a TsLayoutOptions object.
 *
 *  Parameters:
 *      options         - [in] this
 *      tag             - [in] data identifier (must be TsTag_WsdA or TsTag_WsdB)
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_removeDictionary(TsLayoutOptions *options, TsTag tag);


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
 *      becomes invalid when this TsLayoutOptions object is destroyed.
 *      Therefore, you must keep reference to the TsLayoutOptions object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      options     - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsExternalUserData *)
TsLayoutOptions_getExternalUserdata(TsLayoutOptions *options);



/*** OPTIONS BELOW THIS LINE ARE DEPRECATED **********************************/

/******************************************************************************
 *
 *  Enumeration of cursor models.
 *
 * Description:
 *      Controls how the cursor (caret) works with bidirectional text.
 *
 *  Version:
 *      As of WTLE 3.0 this option is deprecated.
 *      See the User Guide for a description of how to control the cursor.
 *
 *  <GROUP layoutoptions>
 */
typedef enum
{
    TS_CURSOR_BOTH_LEADING  = 0,    /* Leading model */
    TS_CURSOR_LEFT_LEADING,         /* Left leading mixed model */
    TS_CURSOR_RIGHT_LEADING,        /* Right leading mixed model */
    TS_CURSOR_NONE_LEADING          /* Neither leading (not used) */
} TsCursorModel;


/*****************************************************************************
 *
 *  Sets the cursor model.
 *
 *  Version:
 *      As of WTLE 3.0 this option is deprecated. Calling this function
 *      has no effect on the layout. See the User Guide for a description
 *      of how to control the cursor.
 *
 *  Parameters:
 *      options     - [in] this
 *      model       - [in] the cursor model to be set (see TsCursorModel)
 *
 *  Return value:
 *      TsResult    - TS_ERR_ARGUMENT_IS_NULL_PTR if options is NULL;
 *                  TS_OK otherwise.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsResult)
TsLayoutOptions_setCursorModel(TsLayoutOptions *options, TsCursorModel model);


/*****************************************************************************
 *
 *  Gets the cursor model.
 *
 *  Version:
 *      As of WTLE 3.0 this option is deprecated.
 *      See the User Guide for a description of how to control the cursor.
 *
 *  Parameters:
 *      options         - [in] this
 *
 *  Return value:
 *      The cursor model.
 *
 *  <GROUP layoutoptions>
 */
TS_EXPORT(TsCursorModel)
TsLayoutOptions_getCursorModel(TsLayoutOptions *options);

/* End of deprecated options ************************************/


/* Private structures: do not access directly ******************/


typedef struct TsTabStop_
{
    TsFixed position;
    TsUInt8 alignment;
    TsUInt16 leader;
} TsTabStop;


struct TsLayoutOptions_
{
#ifdef TS_USE_EXTERNAL_USERDATA
    /* IMPORTANT: If externalUserData is used then it must be the first
       element in structure. This supports a particular IDL mechanism. */
    TsExternalUserData externalUserData;
#endif

    TsDynamicArray /* <TsObserverInfo> */ observers;
    TsDynamicArray /* <TsTabStop>      */ tabStops;

    TsSubstDict *substDictA;          /* substitution dictionary */
    TsSubstDict *substDictB;          /* substitution dictionary */

    TsUInt32 truncationChar;          /* character used by some truncation models to indicate truncation */
    TsUInt32 hyphenChar;              /* character used when hyphenating */
    TsUInt32 baseChar;                /* character used as base character for isolated diacritics */
    TsColor highlightBackgroundColor; /* color of the background of a text selection when the highlight mode is TS_HIGHLIGHT_COLOR_MODE */
    TsColor highlightTextColor;       /* color of the text in a selection when the highlight mode is TS_HIGHLIGHT_COLOR_MODE */
    TsFixed tabStopPositionDefault;   /* position of tab stops if no tab stop is defined */
    TsFixed monospaceMinPadding;      /* amount of padding to add to each side of widest grapheme in monospace mode */
    TsFixed monospaceMinWidth;        /* minimum width to use in monospace mode (in case of very narrow graphemes) */
    TsTag defaultScript;              /* default script tag to be assumed for the text */

    TsRefCount refCount;
    TsRefCount observerCount;
    TsMemMgr *memMgr;                 /* memory manager object or NULL */

    TsOptionsFlagType layoutFlags;    /* layout-related flags stored as bit field */
    TsOptionsFlagType layoutFlags2;   /* layout-related flags stored as bit field */

    TsLineBreakModel lineBreakModel;  /* line break algorithm variants */
    TsHighlightMode highlightMode;    /* control highlighting behavior when text is selected */
    TsCursorModel cursorModel;        /* cursor positioning algorithm models */
    TsSingleLineTruncation sltMode;   /* single line truncation mode */
};

#define TS_OPTIONS_FLAGS2_START TS_REDUCE_MEMORY_USAGE

#define TS_SET_OPTION(flags, tag)   TS_SET_FLAG(flags, (TsUInt32)(1U << (TsUInt32)tag))
#define TS_GET_OPTION(flags, tag)   TS_GET_FLAG(flags, (TsUInt32)(1U << (TsUInt32)tag))
#define TS_CLEAR_OPTION(flags, tag) TS_CLEAR_FLAG(flags, (TsUInt32)(1U << (TsUInt32)tag))

/*lint -emacro(778, TS_SET_OPTION2, TS_GET_OPTION2, TS_CLEAR_OPTION2)*/
#define TS_SET_OPTION2(flags, tag)   TS_SET_FLAG(flags, (TsUInt32)(1U << (TsUInt32)((tag) - (TsUInt32)TS_OPTIONS_FLAGS2_START)))
#define TS_GET_OPTION2(flags, tag)   TS_GET_FLAG(flags, (TsUInt32)(1U << (TsUInt32)((tag) - (TsUInt32)TS_OPTIONS_FLAGS2_START)))
#define TS_CLEAR_OPTION2(flags, tag) TS_CLEAR_FLAG(flags, (TsUInt32)(1U << (TsUInt32)((tag) - (TsUInt32)TS_OPTIONS_FLAGS2_START)))

/* Private fast macros for use in core code. These do not have NULL pointer checking as does the public function. */
#define TsLayoutOptions_get1(opts, tag) TS_GET_OPTION((opts)->layoutFlags, tag)
#define TsLayoutOptions_get2(opts, tag) TS_GET_OPTION2((opts)->layoutFlags2, tag)

/* End of private section ***************************************************/


TS_END_HEADER

#endif /* TSLAYOUTOPTIONS_H */

