/*****************************************************************************
 *
 *  tslayout.h  - Defines the interface to the TsLayout class.
 *
 *  Copyright (C) 2002-2007, 2009, 2012 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSLAYOUT_H
#define TSLAYOUT_H

#include "tstext.h"
#include "tslayctl.h"
#include "tslayopt.h"
#include "tsunienc.h"
#include "ts2d.h"
#include "tsdc.h"
#include "tstextcn.h"
#include "tseudata.h"
#include "tstxtinf.h"

TS_BEGIN_HEADER

struct TsLayout_;


/*****************************************************************************
 *
 *  Rich text layout object.
 *
 *  Description:
 *
 *      The TsLayout object is a rich text layout object.
 *
 *  <GROUP layout>
 */
typedef struct TsLayout_ TsLayout;


/*****************************************************************************
 *
 *  A structure for returning information about a layout.
 *
 *  Version:
 *      Version 3.0 changes:
 *      <TABLE>
 *      <B>Change                   <B>Upgrade instructions
 *      nextLayoutOrigin removed    use (advanceBox.x1, advanceBox.y2) instead of (nextLayoutOrigin.x, nextLayoutOrigin.y)
 *      nextCharOrigin removed      use TsLayout_getLineInfo() to get the advanceBox.x2 (or advanceBox.x1 if RTL) of the last line
 *      </TABLE>
 *
 *      Version 3.2 changes:
 *          The behavior of the function regarding the handling of the case of a newline
 *          at the end of the text has changed. It has been changed back to the
 *          behavior in releases prior to 3.0. That is, if the last line of the layout
 *          is blank, its startIndex, endIndex will be that of the newline at the end of the
 *          text and the endIndex of the penultimate line will also have that value. In 3.0,
 *          the endIndex was set to one less than the end of text index.
 *
 *  <GROUP layout>
 */
typedef struct
{
    TsInt32 numLines;           /* number of lines in the layout */
    TsInt32 numContainersUsed;  /* number of containers actually used */
    TsRect boundingBox;         /* text layout bounding box */
    TsRect advanceBox;          /* text layout advance box */
    TsUInt32 composeStatus;     /* Flag indicating whether composition completed or not and reason why if not */
    /* Note: nextLayoutOrigin has been removed, see version notes above */
    /* Note: nextCharOrigin has been removed, see version notes above */
} TsLayoutInfo;


/*****************************************************************************
 *
 *  Macros that define compose status, for use with TsLayoutInfo's composeStatus.
 *
 *  <GROUP layout>
 */
#define TS_COMPOSE_COMPLETE             0x00
#define TS_COMPOSE_CONTAINER_FULL       0x01
#define TS_COMPOSE_COULD_NOT_FIT_WIDTH  0x02


/*****************************************************************************
 *
 *  A structure for returning information about a container in the layout.
 *
 *  <GROUP layout>
 */
typedef struct
{
    TsInt32 numLines;           /* count of lines in the container */
    TsInt32 firstLineNumber;    /* index of first line in container */
    TsRect boundingBox;         /* text container bounding box */
    TsRect advanceBox;          /* text container advance box */
} TsContainerInfo;


/*****************************************************************************
 *
 *  A structure for returning information about line in the layout.
 *
 *  <GROUP layout>
 */
typedef struct
{
    TsInt32  numGlyphs;     /* number of glyphs on the line */
    TsInt32  startIndex;    /* TsText character index of first character in line */
    TsInt32  endIndex;      /* TsText character index of last character in line */
    TsRect   boundingBox;   /* actual "inked" area of the line -- relative to line's origin*/
    TsRect   advanceBox;    /* box spanning origin to next advance point -- relative to line's origin */
    TsUInt8  bidiBaseLevel; /* bidi base level for line -- odd if Right-To-Left, even if Left-To-Right */
    TsPoint  lineOrigin;    /* coordinates for line's origin, relative to the layout */
    TsUInt32 lineFlags;     /* flags like TS_LINE_COMPOSED and TS_LINE_DISPLAYED */
    TsInt32  baseline;      /* amount to add to lineOrigin.y to get baseline */
    TsInt32  top;           /* amount to add to lineOrigin.y to get top of line (used for overline, and background color) */
    TsInt32  bottom;        /* amount to add to lineOrigin.y to get bottom of line (used for background color) */
    TsAlignType alignment;  /* alignment of line */
} TsLineInfo;


/*****************************************************************************
 *
 *  Macros that define line status, for use with TsLineInfo's lineFlags.
 *
 *  <GROUP layoutedit>
 */
#define TS_LINE_COMPOSED  0x01
#define TS_LINE_DISPLAYED 0x02


/*****************************************************************************
 *
 *  A structure for returning information about a glyph in the layout.
 *
 *  <GROUP layout>
 */
typedef struct
{
    TsInt32 sourceIndex;    /* character index in TsText block */
    TsInt32 glyphID;        /* glyph index in font */
    TsInt32 x;              /* the x coordinate relative to the line's origin */
    TsInt32 y;              /* the y coordinate relative to the line's origin */
    TsFixed caretX;         /* caret x position, may be different from glyph x position in diacritics, ligatures, etc. This is the position at the leading edge of the glyph. */
    TsRect boundingBox;     /* bounding box relative to the glyph's position (x,y) */
    TsUInt16 boundaryFlags; /* boundary flags at this glyph -- see tsglypos.h for #defines of these boundary flags */
} TsGlyphInfo;


/******************************************************************************
 *
 *  Parameters used when drawing strings.
 *
 *  <GROUP glyph>
 *
 */
typedef struct TsDrawGlyphParams_
{
    TsColor textColor;      /* color used to fill main part of text */
    TsColor edgeColor;      /* color used for glyph edges */
    TsColor shadowColor;    /* reserved for future use */
    TsColor backgroundColor;/* color used for background layer */
} TsDrawGlyphParams;


/******************************************************************************
 *
 *  Initializes TsDrawGlyphParameters parameters to default values.
 *
 *  Description:
 *      The default colors are all TS_COLOR_BLACK.
 *
 *  Parameters:
 *      params - [in] pointer to params structure
 *
 *  Return value:
 *      void
 *
 *  <GROUP glyph>
 */
TS_EXPORT(void)
TsDrawGlyphParams_init(TsDrawGlyphParams *params);


/*****************************************************************************
 *
 *  A structure that forms a tuple to identify an inline image.
 *
 *  Note:
 *      If sourceTag = 'Extn' then inline image was externally defined by user.
 *      If sourceTag = 'Wsd*' then inline image was internally defined by
 *      substitution dictionary.
 *
 *  <GROUP layout>
 */
typedef struct TsInlineImageID_
{
    TsUInt32 imageID;           /* numeric ID */
    TsTag sourceTag;            /* used to indicate who set this image */
    TsFixed xSize;              /* x-dimension scaling factor of image */
    TsFixed ySize;              /* y-dimension scaling factor of image */
    TsFontStyle *fontStyle;     /* font style - if this image comes from a font */
} TsInlineImageID;


/*****************************************************************************
 *
 *  A structure that defines inline image metrics needed for layout.
 *
 *  <GROUP layout>
 */
typedef struct TsInlineImageMetrics_
{
    TsFixed xAdvance;       /* x-dimension advance width, in fractional pixels */
    TsFixed yAdvance;       /* y-dimension advance width, in fractional pixels */
    TsRect boundingBox;     /* bounding box of image, with respect to its origin */
} TsInlineImageMetrics;


/*****************************************************************************
 *
 *  A structure that tracks inline image handles, metrics, and reference counts.
 *
 *  <GROUP layout>
 */
typedef struct TsInlineImageRecord_
{
    TsUInt32 imageHandle; /* image handle */
    TsInlineImageMetrics imageMetrics; /* image metrics */
    TsUInt32 handlesAvailable; /* total handles requested */
    TsUInt32 handlesInUse;     /* handles currently in use */
} TsInlineImageRecord;


/*****************************************************************************
 *
 *  Defines callback information for inline image handling.
 *
 *  Description:
 *      view - void pointer that is passed back with each callback function.
 *      It can be used as the "this" pointer for the callbacks.
 *
 *      getInlineImage - callback constructs a representation of this image
 *          using data it has or data provided passed in. Uses the imageID
 *          to uniquely define an image. When the imageID data matches a previous
 *          imageID the function may return the same imageHandle or it may
 *          choose to return a different handle, if desired.
 *
 *          <TABLE>
 *          view            [in] pointer to callback's "this"
 *          data            [in] if the image data comes from the font this pointer is defined
 *                          otherwise it is NULL.
 *
 *          imageID         [in] tuple of data that defines images
 *          imageMetrics    [in/out] if data is defined then these metrics are defined
 *                          if data is NULL then these metrics should be filled in by callback function
 *
 *          imageHandle     [out] function should return a unique handle for this image
 *          startIndex      [in] start index of the text that is being replaced
 *          endIndex        [in] end index of the text that is being replaced
 *          Return          TS_OK if success; TS_ERR_INLINE_IMAGE otherwise
 *          </TABLE>
 *
 *      drawInlineImage - callback draws the inline image in the device context.
 *
 *          <TABLE>
 *          view            [in] pointer to callback's "this"
 *          imageHandle     [in] the handle returned by getInlineImage
 *          x               [in] x position in DC coordinates when drawing this image
 *          y               [in] y position in DC coordinates when drawing this image
 *          dc              [in] pointer to device context
 *          Return          TS_OK if success
 *          </TABLE>
 *
 *      releaseInlineImage -
 *          <TABLE>
 *          view            [in] pointer to callback's "this"
 *          imageHandle     [in] the handle returned by getInlineImage
 *          Return          TS_OK if success; TS_ERR_INLINE_IMAGE otherwise
 *          </TABLE>
 *
 *  <GROUP layout>
 */
typedef struct TsInlineImageCallbacks_
{
    void *view; /* The "this" pointer for callbacks. */
    TsResult (*getInlineImage)(void *view, void *data, TsInlineImageID *imageID, TsInlineImageMetrics *metrics, TsUInt32 *imageHandle, TsUInt32 startIndex, TsUInt32 endIndex); /* getInlineImage callback */
    TsResult (*drawInlineImage)(void *view, TsUInt32 imageHandle, TsInt32 x, TsInt32 y, TsDC *dc); /* drawInlineImage callback */
    TsResult (*releaseInlineImage)(void *view, TsUInt32 imageHandle); /* releaseInlineImage callback */
} TsInlineImageCallbacks;


/*****************************************************************************
 *
 *  Set callback information for inline image handling.
 *
 *  Description:
 *      Calling this function with a valid callback structure enables
 *      inline image handling. The callback structure contains a "view"
 *      element and several client-defined callback functions.
 *
 *      This call is only necessary if inline image handling is needed.
 *      Usually the call is made only once. Calling the function with
 *      a NULL callbacks argument disables inline image handling for this
 *      layout.
 *
 *  Parameters:
 *      layout      - [in] this
 *      callbacks   - [in] structure that defines the callback information
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_setInlineImageCallbacks(TsLayout *layout, TsInlineImageCallbacks *callbacks);


/*****************************************************************************
 *
 *  Get callback information for inline image handling.
 *
 *  Description:
 *      Returns the current inline image callback information.
 *
 *  Parameters:
 *      layout      - [in] this
 *      callbacks   - [out] structure that defines the callback information
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getInlineImageCallbacks(TsLayout *layout, TsInlineImageCallbacks *callbacks);


/*****************************************************************************
 *
 *  Get inline image metrics.
 *
 *  Description:
 *
 *  Parameters:
 *      layout       - [in]  this
 *      imageHandle  - [in]  handle to image
 *      imageMetrics - [out] metrics
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getInlineImageMetrics(TsLayout *layout, TsUInt32 imageHandle, TsInlineImageMetrics *imageMetrics);



/*****************************************************************************
 *
 *  Creates a new TsLayout object.
 *
 *  Description:
 *      When the layout is intended for use with strings, the text argument
 *      should be NULL, otherwise it should be defined.
 *
 *      The control object should generally pre-constructed. It may be used
 *      with one or more layout objects. As a convenience, you may specify
 *      NULL for this argument. In that case the TsLayout_new function
 *      will construct the control object. That is only recommended if
 *      you have only one layout. If multiple layouts are constructed with
 *      NULL arguments, they each will construct a control object. This
 *      is inefficient.
 *
 *      The options object should generally pre-constructed. It may be used
 *      with one or more layout objects. As a convenience, you may specify
 *      NULL for this argument. In that case the TsLayout_new function
 *      will construct the options object. That is only recommended if
 *      you have only one layout. If multiple layouts are constructed with
 *      NULL arguments, they each will construct an options object. This
 *      is inefficient.
 *
 *  Parameters:
 *      text        - [in] rich text source data
 *      control     - [in] layout control object
 *      options     - [in] layout options
 *
 *  Return value:
 *      Pointer to new TsLayout object or NULL upon error.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsLayout *)
TsLayout_new(TsText *text, TsLayoutControl *control, TsLayoutOptions *options);



/*****************************************************************************
 *
 *  Creates a new memory managed TsLayout object.
 *
 *  Description:
 *      See description for TsLayout_new.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      text        - [in] rich text source data
 *      control     - [in] layout control object
 *      options     - [in] layout options
 *
 *  Return value:
 *      Pointer to new TsLayout object or NULL upon error.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsLayout *)
TsLayout_newex(TsMemMgr *memMgr, TsText *text, TsLayoutControl *control, TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Frees a TsLayout object.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP layout>
 */
TS_EXPORT(void)
TsLayout_delete(TsLayout *layout);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsLayout object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsLayout_releaseHandle.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      Pointer to a layout object.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsLayout *)
TsLayout_copyHandle(TsLayout *layout);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsLayout object.
 *
 *  Description:
 *      This function reverses the action of TsLayout_copyHandle.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP layout>
 */
TS_EXPORT(void)
TsLayout_releaseHandle(TsLayout *layout);


/*****************************************************************************
 *
 *  Register an observer with the layout object.
 *
 *  Parameters:
 *      layout          - [in] this
 *      observer        - [in] pointer to object that is observing the layout
 *      callback        - [in] callback function that is called when something
 *                             about the layout changes
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                      passed in.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_registerObserver(TsLayout *layout, void *observer, TsEventCallback callback);


/*****************************************************************************
 *
 *  Unregister an observer with the layout object.
 *
 *  Parameters:
 *      layout          - [in] this
 *      observer        - [in] pointer to object that is observing the layout
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                      passed in.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_unregisterObserver(TsLayout *layout, void *observer);


/*****************************************************************************
 *
 *  Composes the entire rich text within a rectangular region.
 *
 *  Description:
 *      This is the compose function for block-based layout mode.
 *
 *      This function composes the entire TsText rich text object that was
 *      previously set using TsLayout_new or TsLayout_setText. The composition
 *      occurs within a single rectangular block with width equal
 *      to the 'lineWidth' parameter and with an open-ended height.
 *      The layout origin is the upper-left-hand corner of this block.
 *
 *      The corresponding display function is TsLayout_display.
 *
 *      A simple use scenario involves this sequence of calls:
 *      <code>
 *          TsLayout_setText(layout, text);
 *          TsLayout_compose(layout, width, &layoutInfo);
 *          TsLayout_display(layout, dc, x, y);
 *      </code>
 *
 *      Calling this function removes any text containers previously added
 *      using TsLayout_addTextContainer.
 *
 *      Do not mix block-based layout calls with those of other layout modes
 *      within the same TsLayout object.
 *
 *  Parameters:
 *      layout      - [in]  this
 *      lineWidth   - [in]  line width, including indents, in pixels
 *      layoutInfo  - [out] returns information about the layout results
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_compose(TsLayout *layout, TsInt32 lineWidth, TsLayoutInfo *layoutInfo);


/*****************************************************************************
 *
 *  Composes a specific line of the rich text.
 *
 *  Description
 *      This is the compose function for line-based layout mode.
 *
 *      The user calls this function in a loop until there are no more
 *      characters left to compose. The user can specify a different
 *      width for each line.
 *
 *      This function is used in conjunction with TsLayout_composeLineInit,
 *      TsLayout_positionLine, and TsLayout_displayLine.
 *
 *      The function TsLayout_composeLineInit should be called once prior
 *      to actually composing any lines.
 *
 *      A simple use scenario involves this sequence of calls:
 *      <code>
 *          TsLayout_setText(layout, text);
 *          TsLayout_composeLineInit(layout);
 *
 *          ....loop over lines ....
 *          TsLayout_composeLine(layout, ...);
 *          TsLayout_positionLine(layout, x, y, lineNumber);
 *          TsLayout_displayLine(layout, dc, x, y, lineNumber);
 *          ....end of loop ....
 *      </code>
 *
 *      Calling this function removes any text containers previously added
 *      using TsLayout_addTextContainer.
 *
 *      Do not mix line-based layout calls with those of other layout modes
 *      within the same TsLayout object.
 *
 *      The characters referenced by startIndex and endIndex
 *      may include additional characters that were composed into single
 *      characters (e.g. by normalization into form NFC).  For this reason,
 *      charsLeftToCompose should be used instead of endIndex for the purpose
 *      of determining if there are any characters left to compose.
 *
 *  Version:
 *      Version 3.2 changes:
 *         If the boundary flags of last character in the text has the
 *         required line end bit set (e.g. a carriage return or newline), this
 *         function no longer adds an extra blank line at the end of the
 *         layout.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      lineWidth       - [in]  line width, including indents, in pixels
 *      ascenderLimit   - [in]  min y value for ascenders (i.e. min boundingBox.y1), set to TS_INT32_MIN if not needed, error if exceeded
 *      descenderLimit  - [in]  max y value for descenders (i.e. max boundingBox.y2), set to TS_INT32_MAX if not needed, error if exceeded
 *      lineNumber      - [in]  the line number (zero-based) from which, or to which, to compose
 *      preview         - [in]  set to TRUE to get preview of startIndex/endIndex/charsLeftToCompose at lineWidth, but not actually compose
 *      startIndex         - [out] the index of the first character in the line (first in logical order)
 *      endIndex           - [out] the index of the last character in the line (last in logical order)
 *      charsLeftToCompose - [out] the number of characters left to compose
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_composeLine(TsLayout *layout, TsInt32 lineWidth, TsInt32 ascenderLimit, TsInt32 descenderLimit, TsInt32 lineNumber, TsBool preview, TsInt32 *startIndex, TsInt32 *endIndex, TsInt32 *charsLeftToCompose);


/*****************************************************************************
 *
 *  Initializes the layout for line based composition.
 *
 *  Description:
 *      It prepares the layout for line based composition.
 *      This function is used only with line-based layout mode
 *
 *      See TsLayout_composeLine for details on when to use this function.
 *
 *  Parameters:
 *      layout          - [in]  this
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_composeLineInit(TsLayout *layout);


/*****************************************************************************
 *
 *  Set the position of a line in line based layout.
 *
 *  Description:
 *      This function sets the location of a line within a layout, when using
 *      line based layout mode. Call this function prior to calling
 *      TsLayout_displayLine.
 *
 *      See TsLayout_composeLine for details on when to use this function.
 *
 *
 *  Parameters:
 *      layout          - [in]  this
 *      x               - [in]  x-coordinate of line in layout coordinates
 *      y               - [in]  y-coordinate of line in layout coordinates
 *      lineNumber      - [in]  the line number for which to set the position
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_positionLine(TsLayout *layout, TsInt32 x, TsInt32 y, TsInt32 lineNumber);


/*****************************************************************************
 *
 *  Composes the entire rich text block using text containers.
 *
 *  Description:
 *
 *      This is the compose function for container-based layout mode.
 *
 *      This function composes the entire TsText rich text object that was
 *      previously set using TsLayout_new or TsLayout_setText. The composition
 *      occurs within one or more text containers that have been set using
 *      TsLayout_setTextContainer. The containers may vary in size and shape
 *      and may contain anchored objects. The text will flow in sequence
 *      starting with the container zero, then container one, and so on.
 *
 *      The corresponding display function is TsLayout_displayContainers.
 *
 *      A simple use scenario involves this sequence of calls:
 *      <code>
 *          TsLayout_addTextContainer(layout, container, id, origin);
 *
 *          TsLayout_setText(layout, text);
 *          TsLayout_composeContainers(layout, NULL);
 *          TsLayout_displayContainers(layout, dc, x, y);
 *      </code>
 *
 *      Do not mix container-based layout calls with those of other layout modes
 *      within the same TsLayout object.
 *
 *  Notes:
 *      The composeRect argument is currently ignored. This argument may be
 *      specified as NULL. In future releases, this argument will be used to
 *      improve performance by only composing text that may fall within
 *      the composeRect. In that sense the argument will form a composition
 *      clipping rectangle.
 *
 *  Parameters:
 *      layout      - [in]  this
 *      composeRect - [in]  rectangle in which text must be composed, currently not used - can be NULL
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_composeContainers(TsLayout *layout, TsRect *composeRect);


/*****************************************************************************
 *
 *  Set the text.
 *
 *  Parameters:
 *      layout      - [in] this
 *      text        - [in] pointer to new text object
 *
 *  Return value:
 *      TsResult    - TS_OK if function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if either parameter is NULL.
 *
 *  Notes:
 *      This function releases the handle of the existing TsText object and
 *      increases the reference count on the new TsText object.

 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_setText(TsLayout *layout, TsText *text);


/*****************************************************************************
 *
 *  Gets the current text object.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      TsText *    - pointer to the current text object, or NULL if the
 *                    passed in layout is NULL.
 *
 *  Notes:
 *      This function increases the reference count on the returned object
 *      so TsText_releaseHandle needs to be called on the object when you are
 *      done with it.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsText *)
TsLayout_getText(TsLayout *layout);


/*****************************************************************************
 *
 *  Set control object.
 *
 *  Description:
 *      The TsLayoutControl object contains layout information that is
 *      intended to be shared across one or more TsLayout objects. This
 *      data includes language and substitution dictionaries.
 *
 *      Generally there exists only one TsLayoutControl object. It
 *      can be added using TsLayout_new or by calling this function.
 *
 *  Parameters:
 *      layout      - [in] this
 *      control     - [in] pointer to new control
 *
 *  Return value:
 *      TsResult    - TS_OK if function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if either parameter is NULL.
 *  Notes:
 *      This function releases the handle of the existing control and
 *      increases the reference count on the new control object.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_setControl(TsLayout *layout, TsLayoutControl *control);


/*****************************************************************************
 *
 *  Get the current control object.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      TsLayoutControl*  - pointer to the currently set layout control object,
 *                          or NULL if the passed in layout is NULL
 *
 *  Notes:
 *      This function increases the reference count on the returned layout
 *      control object so TsLayoutControl_releaseHandle needs to be called
 *      on the object when you are done with it.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsLayoutControl *)
TsLayout_getControl(TsLayout *layout);


/*****************************************************************************
 *
 *  Set layout options.
 *
 *  Description:
 *      Set the layout options used for composing text. Calling this
 *      function may invalidate the composition.
 *
 *      This function releases the handle of the existing options and increases
 *      the reference count on the new options object.
 *
 *  Parameters:
 *      layout      - [in] this
 *      options     - [in] pointer to new layout options
 *
 *  Return value:
 *      TsResult    - TS_OK if function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if either parameter is NULL.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_setOptions(TsLayout *layout, TsLayoutOptions *options);


/*****************************************************************************
 *
 *  Gets the current options.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      TsLayoutOptions*  - pointer to the currently set layout options, or
 *                          NULL if the passed in layout is NULL.
 *
 *  Notes:
 *      This function increases the reference count on the returned object
 *      so TsLayoutOptions_releaseHandle needs to be called on the object
 *      when you are done with it.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsLayoutOptions *)
TsLayout_getOptions(TsLayout *layout);


/*****************************************************************************
 *
 *  Gets the advance width of a string.
 *
 *  Description:
 *      This function returns the advance width of a string. All characters
 *      in the string have the same fontStyle.
 *
 *      The advance width is the x-direction distance in pixels
 *      from the origin of the first character in the string to the origin
 *      of the character following the string.
 *
 *      nUnits is the number of code units in the string. A code unit
 *      is one encoding unit. It takes one or more code units to
 *      specify a character. A UTF-8 code unit is one byte long
 *      A UTF-16 code unit is two bytes long. The string does not have
 *      to be NULL terminated.
 *
 *      This function is useful when the client already has some layout
 *      functionality but needs complex shaping support. The advance
 *      width returned here is that of a properly shaped sequence of
 *      glyphs corresponding with the input string.
 *
 *      For best performance, reuse the TsLayout object within the layout
 *      loop and use this instance only for working with strings.
 *
 *      If full layout/display features are also needed then construct
 *      one or more separate TsLayout objects.
 *
 *  Notes:
 *      The width value returned by this function is the advance width
 *      not the width of the bounding box width. The advance width is the
 *      distance from the origin of the first character to the origin
 *      of the next character following the string. Due to the way
 *      characters are designed, the character bounding box may extend
 *      to the left or right of its origin and right edge of a character
 *      is not generally aligned with the next character's origin.
 *      Interpreting the bounding box as advance width would lead to
 *      character placement errors.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      width           - [out] advance width, in pixels
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getStringWidth(TsLayout *layout,
                        void *string, TsInt32 nUnits, TsEncoding encoding,
                        TsFontStyle *fontStyle, TsInt32 *width);


/*****************************************************************************
 *
 *  Gets the advance width of a string but does not stop at newlines.
 *
 *  Description:
 *      See also TsLayout_getStringWidth.
 *
 *      Behavior is similar to TsLayout_getStringWidth, except that the
 *      accumulation of the advance widths will not stop at a mandatory line
 *      break character. If the TS_INSERT_TRUNCATION_CHAR_IF_LINE_FEED layout
 *      option has been set (and a truncation character has been set), the
 *      accumulation will stop and the width of the truncation character will
 *      be included in the returned width.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      width           - [out] advance width, in pixels
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getStringWidthIgnoringNewlines(TsLayout *layout,
                        void *string, TsInt32 nUnits, TsEncoding encoding,
                        TsFontStyle *fontStyle, TsInt32 *width);


/*****************************************************************************
 *
 *  Composes and then displays a single string.
 *
 *  Description:
 *
 *      nUnits is the number of code units in the string. A code unit
 *      is one encoding unit. It takes one or more code units to
 *      specify a character. A UTF-8 code unit is one byte long.
 *      A UTF-16 code unit is two bytes long. The string does not have
 *      to be NULL terminated.
 *
 *      This function is useful when the client already has some layout
 *      functionality but needs complex shaping support.
 *
 *      For best performance, reuse the TsLayout object within the layout
 *      loop and use this instance only for working with strings.
 *
 *      If full layout/display features are also needed then construct
 *      one or more separate TsLayout objects.
 *
 *      If dc is NULL then the function executes but nothing is drawn.
 *
 *      If drawParams is NULL then default glyph colors are used.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      dc              - [in]  pointer to device context object
 *      x               - [in]  x-coordinate or string origin in device coordinates
 *      y               - [in]  y-coordinate of string origin in device coordinates
 *      drawParams      - [in]  structure that specifies colors used to render glyphs
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_drawString(TsLayout *layout,
                        void *string, TsInt32 nUnits, TsEncoding encoding,
                        TsFontStyle *fontStyle, TsDC *dc, TsInt32 x, TsInt32 y, TsDrawGlyphParams *drawParams);


/*****************************************************************************
 *
 *  A structure for returning information about a string that was
 *  composed in a call to TsLayout_composeString.
 *
 *
 *  <GROUP layout>
 */
typedef struct
{
    TsPoint advance;            /* The advance width and height of the composed string, in fixed point values */
    TsInt32 nUnitsConsumed;     /* number of code units used in the composition */
    TsInt32 nCharsConsumed;     /* number of characters used in the composition */
    TsInt8  truncation;         /* reason the string was truncated or zero if not truncated */
} TsStringInfo;


#define TS_TRUNCATION_NONE     0x0   /* string was not truncated */
#define TS_TRUNCATION_WIDTH    0x1   /* string was truncated because the maximum width was reached */
#define TS_TRUNCATION_NEWLINE  0x2   /* string was truncated because a newline or paragraph marker was encountered */
#define TS_TRUNCATION_TAB      0x4   /* string was truncated because a tab character was encountered */
#define TS_TRUNCATION_MIDDLE   0x8   /* string was truncated in the middle */

#define TS_MAXIMUM_COMPOSITION_WIDTH 0x00007EFF /* note: real max is 0x7FFF [i.e. (2^15)-1 for 16.16 signed fixed point,
                                                 * but subtracting 0x0100 as a safety margin for case of right-aligned glyph
                                                 * extending beyond right edge of layout
                                                 */


/*****************************************************************************
 *
 *  Compose a single string.
 *
 *  Description:
 *      This function composes a string.
 *
 *      nUnits is the number of code units in the string. A code unit
 *      is one encoding unit. It takes one or more code units to
 *      specify a character. A UTF-8 code unit is one byte long.
 *      A UTF-16 code unit is two bytes long. The string does not have
 *      to be NULL terminated.
 *
 *      This function is useful when the client already has some layout
 *      functionality but needs complex shaping support.
 *
 *      For best performance, reuse the TsLayout object within the layout
 *      loop and use this instance only for working with strings.
 *
 *      If full layout/display features are also needed then construct
 *      one or more separate TsLayout objects.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      maxWidth        - [in]  maximum width for string, if zero or TS_MAXIMUM_COMPOSITION_WIDTH, no maximum width
 *      alignment       - [in]  requested alignment of the string (with respect to the display point)
 *      params          - [in]  structure for additional input parameters; can be NULL
 *      info            - [out] filled in with information about the composed string; see TsStringInfo; can be NULL
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_composeString(TsLayout *layout, void *string, TsInt32 nUnits,
                       TsEncoding encoding, TsFontStyle *fontStyle, TsInt32 maxWidth,
                       TsAlignType alignment, TsStringParams *params, TsStringInfo *info);


/*****************************************************************************
 *
 *  Display a composed single string to a device context.
 *
 *  Description:
 *      This function displays or redisplays a previously composed string.
 *
 *      The TsLayout object should be constructed with the text
 *      argument set to NULL when intended for use with this function.
 *
 *      If full layout/display features are also needed then construct
 *      one or more separate TsLayout objects.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      dc              - [in]  pointer to device context object
 *      x               - [in]  x-coordinate or string origin in device coordinates
 *      y               - [in]  y-coordinate of string origin in device coordinates
 *      drawParams      - [in]  structure that specifies colors used to render glyphs
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_displayString(TsLayout *layout, TsDC *dc, TsInt32 x, TsInt32 y, TsDrawGlyphParams *drawParams);


/*****************************************************************************
 *
 *  Composes and then displays a single string with ellipsis in the middle.
 *
 *  Description:
 *
 *      nUnits is the number of code units in the string. A code unit
 *      is one encoding unit. It takes one or more code units to
 *      specify a character. A UTF-8 code unit is one byte long.
 *      A UTF-16 code unit is two bytes long. The string does not have
 *      to be NULL terminated.
 *
 *      This function is useful when the client already has some layout
 *      functionality but needs complex shaping support.
 *
 *      For best performance, reuse the TsLayout object within the layout
 *      loop and use this instance only for working with strings.
 *
 *      If full layout/display features are also needed then construct
 *      one or more separate TsLayout objects.
 *
 *      If dc is NULL then the function executes but nothing is drawn.
 *
 *      If drawParams is NULL then default glyph colors are used.
 *
 *  Notes:
 *      See description of TsLayout_composeEllipsizedString for a
 *      description of the ellipsis in middle behavior.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      dc              - [in]  pointer to device context object
 *      x               - [in]  x-coordinate or string origin in device coordinates
 *      y               - [in]  y-coordinate of string origin in device coordinates
 *      maxWidth        - [in]  maximum width for string, must be non-zero and less than TS_MAXIMUM_COMPOSITION_WIDTH
 *      drawParams      - [in]  structure that specifies colors used to render glyphs
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_drawEllipsizedString(TsLayout *layout,
                        void *string, TsInt32 nUnits, TsEncoding encoding,
                        TsFontStyle *fontStyle, TsDC *dc, TsInt32 x, TsInt32 y,
                        TsInt32 maxWidth, TsDrawGlyphParams *drawParams);


/*****************************************************************************
 *
 *  Compose a single string with ellipsis in the middle.
 *
 *  Description:
 *      This function composes a string and put an ellipsis in the middle.
 *
 *      nUnits is the number of code units in the string. A code unit
 *      is one encoding unit. It takes one or more code units to
 *      specify a character. A UTF-8 code unit is one byte long.
 *      A UTF-16 code unit is two bytes long. The string does not have
 *      to be NULL terminated.
 *
 *      This function is useful when the client already has some layout
 *      functionality but needs complex shaping support.
 *
 *      For best performance, reuse the TsLayout object within the layout
 *      loop and use this instance only for working with strings.
 *
 *      If full layout/display features are also needed then construct
 *      one or more separate TsLayout objects.
 *
 *      An heuristic algorithm determines the placement of the ellipsis and
 *      which glyphs appear on either side of the ellipsis. The behavior is as
 *      follows: Complete graphemes are guaranteed to appear on each side of
 *      of the ellipsis. The ellipsis will be padded equally on either side 
 *      with any leftover space between the width of glyphs that fit and the
 *      ellipsis. If the found breakpoint on either side is not a potential
 *      line break, the algorithm will look one grapheme away (to left on left
 *      side, to right on the right side) to see if that grapheme is a
 *      potential line break, and if it is, will move the breakpoint to that
 *      location, to improve the appearance of the resulting ellipsized string.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      string          - [in]  pointer to string data
 *      nUnits          - [in]  number of code units in string data
 *      encoding        - [in]  encoding used in the string
 *      fontStyle       - [in]  pointer to fontStyle
 *      maxWidth        - [in]  maximum width for string, must not be 0 and be less than TS_MAXIMUM_COMPOSITION_WIDTH
 *      alignment       - [in]  requested alignment of the string (with respect to the display point)
 *      params          - [in]  structure for additional input parameters; can be NULL
 *      info            - [out] filled in with information about the composed string; see TsStringInfo; can be NULL
 *
 *
 *  Return value:
 *      TsResult code
 *      TS_OK if function completes successfully
 *      TS_ERR_ARGUMENT_OUT_OF_RANGE if the maxWidth is 0 or >= TS_MAXIMUM_COMPOSITION_WIDTH
 *      TS_ERR_COULD_NOT_FIT_WIDTH if the given maxWidth is not large enough to
 *          fit the ellipsis plus at least one grapheme on either side
 *      TS_ERR_MALLOC_FAIL if memory cannot be allocated.
 *
 * Notes:
 *      If a truncation character has been set by a call to TsLayoutOptions_setTruncationCharacter()
 *      that character will be used instead of the ellipsis.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_composeEllipsizedString(TsLayout *layout, void *string, TsInt32 nUnits,
                       TsEncoding encoding, TsFontStyle *fontStyle, TsInt32 maxWidth,
                       TsAlignType alignment, TsStringParams *params, TsStringInfo *info);


/*****************************************************************************
 *
 *  Finds the width of the next line break sequence.
 *
 *  Description:
 *
 *      This function finds the width of the next "unbreakable"
 *      (from a line breaking perspective) sequence of text.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      width           - [out] sequence width, in pixels
 *      startSourceIndex- [out] start index of sequence
 *      endSourceIndex  - [out] end index of sequence
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getWidthOfNextLineBreakSequence(TsLayout *layout, TsInt32 *width, TsInt32 *startSourceIndex, TsInt32 *endSourceIndex);


/*****************************************************************************
 *
 *  Finds the width of the longest remaining line break sequence.
 *
 *  Description:
 *
 *      This function finds the width of the longest remaining "unbreakable"
 *      (from a line breaking perspective) sequence of text.
 *
 *  Parameters:
 *      layout          - [in]  this
 *      width           - [out] sequence width, in pixels
 *      startSourceIndex- [out] start index of sequence
 *      endSourceIndex  - [out] end index of sequence
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getWidthOfLongestRemainingLineBreakSequence(TsLayout *layout, TsInt32 *width, TsInt32 *startSourceIndex, TsInt32 *endSourceIndex);


/*****************************************************************************
 *
 *  Finds the width of the remaining text up to a mandatory line break.
 *
 *  Description:
 *
 *      This function finds the width of the remaining text
 *      up to and including a mandatory line break character (e.g. CR, LF, etc.).
 *
 *  Parameters:
 *      layout          - [in]  this
 *      width           - [out] sequence width, in pixels
 *      startSourceIndex- [out] start index of sequence
 *      endSourceIndex  - [out] end index of sequence
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getWidthOfRemainingTextUntilMandatoryLineBreak(TsLayout *layout, TsInt32 *width, TsInt32 *startSourceIndex, TsInt32 *endSourceIndex);


/*****************************************************************************
 *
 *  Retrieves information about a layout.
 *
 *  Description:
 *      This function fills in a TsLayoutInfo structure.
 *
 *      The advanceBox is given in layout coordinates. If the layout spans more
 *      than one text container then the advanceBox will span all portions of the
 *      containers that are used.
 *
 *      The boundingBox is given in layout coordinates. If the layout spans more
 *      than one text container then the boundingBox will span all portions of the
 *      containers that are used. The boundingBox is the union of the boundingBoxes
 *      of each line in the layout.
 *
 *  Version:
 *      Version 3.1 changes:
 *          Bounding box may be approximate. See TsLayout_getLineInfo for a
 *          more complete description of exact versus approximate bounding box.
 *
 *  Parameters:
 *      layout      - [in]  this
 *      layoutInfo  - [out] TsLayoutInfo structure that is populated
 *
 *  Return value:
 *      TsResult      TS_OK on success.
 *                    TS_ERR_ARGUMENT_IS_NULL_PTR if either of the parameters
 *                    is NULL.
 *                    TS_ERR_INVALID_LAYOUT_MODE if the composition mode of
 *                    the layout is line based.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getLayoutInfo(TsLayout *layout, TsLayoutInfo *layoutInfo);


/*****************************************************************************
 *
 *  Retrieves information about a text container in a layout.
 *
 *  Description:
 *      This function fills in a TsContainerInfo structure.
 *
 *      The advanceBox is given in container coordinates.
 *
 *      The boundingBox is given in container coordinates.
 *      The boundingBox is the union of the boundingBoxes of each line in
 *      the container.
 *
 *  Version:
 *      Version 3.1 changes:
 *          Bounding box may be approximate. See TsLayout_getLineInfo for a
 *          more complete description of exact versus approximate bounding box.
 *
 *
 *  Parameters:
 *      layout         - [in]  this
 *      containerIndex - [in] index of text container within layout
 *      containerInfo  - [out] TsContainerInfo structure that is populated
 *
 *  Return value:
 *      TsResult      TS_OK on success.
 *                    TS_ERR_ARGUMENT_IS_NULL_PTR if either of the pointers
 *                    is NULL
 *                    TS_ERR_INVALID_LAYOUT_MODE if the composition mode of
 *                    the layout is line based.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getContainerInfo(TsLayout *layout, TsInt32 containerIndex, TsContainerInfo *containerInfo);


/*****************************************************************************
 *
 *  Returns information about a single line in the layout.
 *
 *  Description:
 *      If the alignment for the paragraph in which the line
 *      occurs is dynamic (i.e. TS_ALIGN_LEFT_DYNAMIC or
 *      TS_ALIGN_RIGHT_DYNAMIC), then info->alignment is the
 *      resolved alignment for the line (i.e. TS_ALIGN_LEFT or
 *      TS_ALIGN_RIGHT) based on the bidi directionality of the
 *      paragraph.
 *
 *      The info structure returns the line's bounding box in coordinates
 *      relative to the line's upper left hand corner. The line bounding box
 *      is computed as the union of the glyph bounding boxes, either exact
 *      or approximate depending on layout options and whether the exact
 *      glyph bounding box needed to be computed for some reason.
 *
 *      By default the boundingBox is approximate. It will always be
 *      greater than or equal to the exact boundingBox. It may exceed
 *      the exact box by many pixels depending on the font properties.
 *
 *      Setting the either TS_ENABLE_EXACT_BOUNDING_BOX or
 *      TS_LIMIT_BOUNDING_BOX_TO_COMP_WIDTH to TRUE in the layout options
 *      will force the bounding box to be exact. Depending on use scenario
 *      this may incur a significant performance penalty.
 *
 *      If the line being queried is an empty last line, the endIndex
 *      member of the lineInfo will be one less than the startIndex.
 *
 *  Version:
 *          Version 3.1 changes: boundingBox returned within info parameter
 *          is now either approximate or exact depending on layout options.
 *          This change is part of a performance improvement affecting
 *          certain use scenarios.
 *
 *  Parameters:
 *      layout      - [in] this
 *      lineNumber  - [in] layout line number
 *      info        - [out] returns information about the line
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getLineInfo(TsLayout *layout, TsInt32 lineNumber, TsLineInfo *info);


/*****************************************************************************
 *
 *  Returns information about a single glyph in the layout.
 *
 *  Description:
 *      If needBoundingBox is TRUE then the glyph's exact bounding box
 *      is computed and returned within the info parameter. This will
 *      slow down the function. Depending on layout options, caching options,
 *      and font engine, the slowdown may be quite significant.
 *
 *  Parameters:
 *      layout          - [in] this
 *      lineNumber      - [in] layout line number
 *      glyphIndex      - [in] which glyph in the line
 *      needBoundingBox - [in] set to TRUE if need bounding box info
 *      info            - [out] returns information about the glyph
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_getGlyphInfo(TsLayout *layout, TsInt32 lineNumber, TsInt32 glyphIndex, TsBool needBoundingBox, TsGlyphInfo *info);


/*****************************************************************************
 *
 *  Returns number of lines in layout.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      TsInt32
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsInt32)
TsLayout_getNumberOfLines(TsLayout *layout);


/*****************************************************************************
 *
 *  Maps the text character index to line number and glyph index.
 *
 *  Description:
 *      The layout has a reference to a TsText object. The 'index' parameter
 *      is the index of a character within that text object. This function
 *      returns the line number and glyph index corresponding to that
 *      character.
 *
 *      There is not in general a one-to-one correspondence between a character
 *      and a glyph. The glyph index that is returned actually corresponds
 *      with the one that is at the cursor position corresponding with
 *      the character index.
 *
 *  Parameters:
 *      layout      - [in] this
 *      index       - [in] index of character in text object
 *      lineNumber  - [out] the line number corresponding with this character
 *      glyphIndex  - [out] the glyph index corresponding with this character
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_mapCharIndex(TsLayout *layout, TsInt32 index, TsInt32 *lineNumber, TsInt32 *glyphIndex);


/*****************************************************************************
 *
 *  Displays an entire block-based layout on a device.
 *
 *  Description:
 *      This function is used to display text that has been previously composed
 *      using TsLayout_compose.
 *
 *      See TsLayout_compose for more details on how to use this function.
 *
 *      If the dc parameter may be set to NULL this function just returns.
 *
 *  Parameters:
 *      layout      - [in] this
 *      dc          - [in] device context
 *      originX     - [in] x-coordinate of layout origin in device coordinates
 *      originY     - [in] y-coordinate of layout origin in device coordinates
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_display(TsLayout *layout, TsDC *dc, TsInt32 originX, TsInt32 originY);


/*****************************************************************************
 *
 *  Displays an entire container-based layout on a device.
 *
 *  Description:
 *      This function is used to display text that has been previously composed
 *      using TsLayout_composeContainers.
 *
 *      See TsLayout_compose for more details on how to use this function.
 *
 *      If the dc parameter may be set to NULL this function just returns.
 *
 *  Parameters:
 *      layout      - [in] this
 *      dc          - [in] device context
 *      originX     - [in] x-coordinate of layout origin in device coordinates
 *      originY     - [in] y-coordinate of layout origin in device coordinates
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_displayContainers(TsLayout *layout, TsDC *dc, TsInt32 originX, TsInt32 originY);


/*****************************************************************************
 *
 *  Displays a specific line of the layout on a device.
 *
 *  Description:
 *      This is the display function for line-based layout.
 *
 *      See TsLayout_composeLine for details on how to use this function.
 *
 *      The dc parameter may be set to NULL. This will compute final location
 *      of line, but will not display anything. This may be useful when using
 *      you wish to know how text will be displayed without actually doing it.
 *
 *  Version:
 *      Prior to Version 3.0, the originX and originY arguments formed a point
 *      that positioned the line in absolute display coordinates.
 *
 *      Version 3.0 introduced an API change. The layout always stores
 *      coordinates in layout coordinates relative to the layout origin,
 *      which is the upper left-hand corner. The function TsLayout_positionLine
 *      must be used to specify each line's position within the layout.
 *      In this function, the originX and originY arguments forms a point
 *      that positions the layout origin in absolute display coordinates.
 *      Typically, the same originX and originY values would be used for
 *      each line.
 *
 *  Parameters:
 *      layout      - [in] this
 *      dc          - [in] device context
 *      originX     - [in] x-coordinate of layout origin in device coordinates
 *      originY     - [in] y-coordinate of layout origin in device coordinates
 *      lineNumber  - [in] the line number (zero-based) to display
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsResult)
TsLayout_displayLine(TsLayout *layout, TsDC *dc, TsInt32 originX, TsInt32 originY, TsInt32 lineNumber);


/*****************************************************************************
 *
 *  Macro that defines invalid caret index.
 *
 *  <GROUP layoutedit>
 */
#define TS_INVALID_CARET (TsInt32)0xffffffff


/*****************************************************************************
 *
 *  Macro that defines the leading edge of a grapheme.
 *
 *  <GROUP layoutedit>
 */
#define TS_LEADING_EDGE 0

/*****************************************************************************
 *
 *  Enumeration of possible caret positions
 *
 *  If you are unsure whether to use LOGICAL or VISUAL motion, we recommend that
 *  you use LOGICAL motion as cursor motion in this mode matches the natural
 *  reading order of the glyphs in the text.
 *
 *  <GROUP layoutedit>
 */
typedef enum
{
    TS_NEXT_LOGICAL_GRAPHEME,       /* next grapheme cluster boundary in logical direction */
    TS_PREVIOUS_LOGICAL_GRAPHEME,   /* previous grapheme cluster boundary in logical direction */
    TS_GRAPHEME_TO_LEFT,            /* grapheme cluster boundary to the left */
    TS_GRAPHEME_TO_RIGHT,           /* grapheme cluster boundary to the right */

    TS_NEXT_LOGICAL_WORD,           /* next word boundary in logical direction */
    TS_PREVIOUS_LOGICAL_WORD,       /* previous word boundary in logical direction */
    TS_WORD_TO_LEFT,                /* word start boundary to the left */
    TS_WORD_TO_RIGHT,               /* word start boundary to the right */
    TS_WORD_EDGE_TO_LEFT,           /* word boundary to the left */
    TS_WORD_EDGE_TO_RIGHT,          /* word boundary to the right */

    TS_NEXT_LOGICAL_LINE_BREAK,     /* next potential line break in logical direction */
    TS_PREVIOUS_LOGICAL_LINE_BREAK, /* previous potential line break in logical direction */
    TS_LINE_BREAK_TO_LEFT,          /* potential line break to the left */
    TS_LINE_BREAK_TO_RIGHT,         /* potential line break to the right */

    TS_NEXT_LINE,                   /* same horizontal position in next line (not the next segment on the current line) */
    TS_PREVIOUS_LINE,               /* same horizontal position in previous line (not the next segment on the current line) */

    TS_BEGIN_TEXT,                  /* before the first logical character in text block */
    TS_END_TEXT,                    /* after last logical character in text block */

    TS_BEGIN_LINE,                  /* logical beginning of line */
    TS_END_LINE,                    /* logical end of line */
    TS_LEFT_SIDE_OF_LINE,           /* left side of the line */
    TS_RIGHT_SIDE_OF_LINE           /* right side of the line */
} TsCaretPosition;


/* Backward compatibility macros */
#define TS_NEXT_VISUAL_GRAPHEME TS_GRAPHEME_TO_RIGHT
#define TS_PREVIOUS_VISUAL_GRAPHEME TS_GRAPHEME_TO_LEFT
#define TS_NEXT_VISUAL_WORD TS_WORD_TO_RIGHT
#define TS_PREVIOUS_VISUAL_WORD TS_WORD_TO_LEFT
#define TS_NEXT_VISUAL_LINE_BREAK TS_LINE_BREAK_TO_RIGHT
#define TS_PREVIOUS_VISUAL_LINE_BREAK TS_LINE_BREAK_TO_LEFT
#define TS_BEGIN_VISUAL_LINE TS_LEFT_SIDE_OF_LINE
#define TS_END_VISUAL_LINE TS_RIGHT_SIDE_OF_LINE
#define TS_BEGIN_LOGICAL_LINE TS_BEGIN_LINE
#define TS_END_LOGICAL_LINE TS_END_LINE
#define TS_BEGIN_LOGICAL_TEXT TS_BEGIN_TEXT
#define TS_END_LOGICAL_TEXT TS_END_TEXT
/*
 * Note: TS_BEGIN_VISUAL_TEXT and TS_END_VISUAL_TEXT have been removed from
 * the API. Use TS_BEGIN_TEXT and TS_END_TEXT, followed by TS_LEFT_SIDE_OF_LINE
 * or TS_RIGHT_SIDE_OF_LINE to move to a particular side of the first or
 * last line.
 */

/*****************************************************************************
 *
 *  Saves the mark and point.
 *
 *  Description:
 *      The client application calls this function to save the mark and point
 *      before composing & displaying the text.  The client application should
 *      then call TsLayout_restoreMarkAndPoint() after composing and displaying
 *      the text.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(void)
TsLayout_saveMarkAndPoint(TsLayout *layout);


/*****************************************************************************
 *
 *  Restores the mark and point.
 *
 *  Description:
 *      The client application calls this function to restore the mark and point
 *      after composing & displaying the text.  The client application should
 *      have called TsLayout_saveMarkAndPoint() before composing and displaying
 *      the text.
 *
 *  Parameters:
 *      layout      - [in] this
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_restoreMarkAndPoint(TsLayout *layout);


/*****************************************************************************
 *
 *  Sets the highlighting range.
 *
 *  Description:
 *      The client application calls this function to give information
 *      about where to draw the text caret or selection highlight.
 *      The mark and point refer to logical positions between characters
 *      in the text. If the mark and point are the same, a text caret will
 *      be drawn - otherwise a highlight will be drawn over the selection
 *      at display time.
 *
 *  Parameters:
 *      layout      - [in] this
 *      markIndex   - [in] character index of the mark position
 *      pointIndex  - [in] character index of the point position
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_setSelection(TsLayout *layout, TsInt32 markIndex, TsInt32 pointIndex);


/*****************************************************************************
 *
 *  Gets the currently selected characters.
 *
 *  Parameters:
 *      layout      - [in] this
 *      markIndex   - [out] returns character index of the mark position
 *      pointIndex  - [out] returns character index of the point position
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_getSelection(TsLayout *layout, TsInt32 *markIndex, TsInt32 *pointIndex);


/*****************************************************************************
 *
 *  Returns the nearest character index and offset after a move.
 *
 *  Parameters:
 *      layout           - [in] this
 *      startIndex       - [in] character index before the move
 *      startIndexOffset - [in] offset from the leading edge of the index
 *      type             - [in] type of movement relative to startIndex
 *      resultIndex      - [out] returns character index after the move
 *      resultOffset     - [out] returns offset from the leading edge
 *
 *  Return value:
 *      TsResult
 *
 *  Notes:
 *      For startIndexOffset and resultOffset, a value of 0 indicates the
 *      leading edge of the grapheme at the index. A non-zero value indicates
 *      the trailing edge. (These will always be zero or positive.) The
 *      magnitude of these parameters indicate the number of characters in
 *      the grapheme.
 *      For visual mode next left or right grapheme motion, this function
 *      stops at each visual slot between grapheme. In previous versions,
 *      this function advanced to the leading edge of the next visual
 *      grapheme.
 *
 *      For example:
 *          Given logical buffer abcABCdef  and  visual display: abcCBAdef
 *                               012345678                       012543678
 *
 *       Previously, if at the (leading edge of) 'c' (index 2) advancing
 *       visually to the right, a call to this function would return index
 *       5 (the 'C') and a call to _getCaretInfo would return the leading
 *       edge of 'C' and the cursor would be displayed on the right side
 *       of 'C', thus skipping the slot between c and C.
 *
 *      Now, if at the leading edge of 'c' (startIndex=2, startIndexOffset=0)
 *      advancing to the right will return the trailing edge of 'c'
 *      (resultIndex=2, resultOffset=1).
 *
 *      To achieve the previous behavior, client code should skip returned
 *      non-zero resultOffset values.
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_getValidPosition(TsLayout *layout, TsInt32 startIndex, TsInt32 startIndexOffset,
                          TsCaretPosition type, TsInt32 *resultIndex, TsInt32 *resultOffset);

/*****************************************************************************
 *
 *  Gets the index of character nearest a point in layout coordinates.
 *
 *  Description:
 *      Use this function to position the caret after a mouse click or to
 *      determine the mark or point values for TsLayout_setSelection().
 *
 *  Parameters:
 *      layout      - [in] this
 *      x           - [in] x-coordinate in layout coordinates
 *      y           - [in] y-coordinate in layout coordinates
 *      index       - [out] returns index of character nearest the point
 *      offset      - [out] returns the offset from the leading edge of index
 *
 *  Return value:
 *      TsResult - TS_OK if the point could be mapped to a character position.
 *
 *  Notes:
 *     An offset of 0 indicates the leading edge of the returned index. A
 *     positive value indicates the trailing edge. If non-zero, the
 *     magnitude of offset indicates the number of characters in the grapheme.
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_getIndexForXY(TsLayout *layout, TsInt32 x, TsInt32 y, TsInt32 *index, TsInt32 *offset);


/*
    Note: TsLayout_getXYForIndex() has been removed from the API.

    For the same functionality, please use:

    TsCaretInfo caret;
    TsLayout_getCaretInfo(layout, startIndex, startIndexOffset, TRUE, &caret);

    Then use the x and y members of the caret.

*/

/*
    Note: TsLayout_arrowLeft() and TsLayout_arrowRight() have been removed from the API.

    For the same functionality with LOGICAL motion, please use:
        TsLayout_getValidPosition(layout, startIndex, startIndexOffset,
                                  TS_NEXT_LOGICAL_GRAPHEME, &resultIndex, &resultOffset);
        TsLayout_getValidPosition(layout, startIndex, startIndexOffset, startIndexOffset,
                                  TS_PREVIOUS_LOGICAL_GRAPHEME, &resultIndex, &resultOffset);

    For the same functionality with VISUAL motion, please use:
        TsLayout_getValidPosition(layout, startIndex, startIndexOffset,
                                  TS_RIGHT_VISUAL_GRAPHEME, &resultIndex, &resultOffset);
        TsLayout_getValidPosition(layout, startIndex, startIndexOffset,
                                  TS_LEFT_VISUAL_GRAPHEME, &resultIndex, &resultOffset);

    If you are unsure whether to use LOGICAL or VISUAL motion, we recommend that
    you use LOGICAL motion as cursor motion in this mode matches the natural
    reading order of the glyphs in the text.
*/

/*****************************************************************************
 *
 *  Gets the character index after moving with an up arrow key press.
 *
 *  Note that this differs from using
 *  TsLayout_getValidPosition(...TS_PREVIOUS_LINE...) in that TsLayout_arrowUp()
 *  allows you to preserve the X position using the 'axis' parameter, so that
 *  even if the cursor snaps to the end a short line, it will go back to the
 *  'axis' position in the next line long enough to contain glyphs near that
 *  position.
 *
 *  Parameters:
 *      layout            - [in] this
 *      lineNumber        - [in] line number before the move
 *      axis              - [in] find index closest to this axis
 *      resultIndex       - [out] returns character index after the move
 *      resultIndexOffset - [out] returns offset from the leading edge
 *
 *  Return value:
 *      Character index after the move.
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_arrowUp(TsLayout *layout, TsInt32 lineNumber, TsInt32 axis,
                            TsInt32 *resultIndex, TsInt32 *resultIndexOffset);


/*****************************************************************************
 *
 *  Gets the character index after moving with a down arrow key press.
 *
 *  Note that this differs from using
 *  TsLayout_getValidPosition(...TS_NEXT_LINE...) in that TsLayout_arrowDown()
 *  allows you to preserve the X position using the 'axis' parameter, so that
 *  even if the cursor snaps to the end a short line, it will go back to the
 *  'axis' position in the next line long enough to contain glyphs near that
 *  position.
 *
 *  Parameters:
 *      layout            - [in] this
 *      lineNumber        - [in] line number before the move
 *      axis              - [in] find caret closest to this axis
 *      resultIndex       - [out] returns character index after the move
 *      resultIndexOffset - [out] returns offset from the leading edge
 *
 *  Return value:
 *      Character index after the move.
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_arrowDown(TsLayout *layout, TsInt32 lineNumber, TsInt32 axis,
                            TsInt32 *resultIndex, TsInt32 *resultIndexOffset);


/*****************************************************************************
 *
 * Macros for indicating properties of a caret. These will be OR'ed together
 * in the flags member of the TsCaretInfo structure. Clients can use this
 * information to create custom carets, or decide whether or not to display
 * the caret.
 */
#define TS_CARET_FLAG_SUPERSCRIPT           0x0001 /* caret is superscript */
#define TS_CARET_FLAG_SUBSCRIPT             0x0002 /* caret is subscript */
#define TS_CARET_FLAG_BOLD                  0x0004 /* caret is bold */
#define TS_CARET_FLAG_ITALIC                0x0008 /* caret is italic */
#define TS_CARET_FLAG_BEGIN_LINE            0x0010 /* character at the requested location is at the logical beginning of a line */
#define TS_CARET_FLAG_END_LINE              0x0020 /* character at the requested location is at the logical end of a line */
#define TS_CARET_FLAG_LEFT_SIDE_OF_LINE     0x0040 /* character at the requested location is at the left side of a line */
#define TS_CARET_FLAG_RIGHT_SIDE_OF_LINE    0x0080 /* character at the requested location is at the right side of a line */
#define TS_CARET_FLAG_NON_VISIBLE           0x0100 /* the character at the caret is a non displayed character (e.g. a control character) */
#define TS_CARET_FLAG_WHITESPACE            0x0200 /* the character at the caret is a whitespace character */


/*****************************************************************************
 *
 *  A structure for returning information about suggested caret.
 *
 *  <GROUP layoutedit>
 */
typedef struct
{
    TsInt32 x;                  /* x-coordinate of cursor */
    TsInt32 y;                  /* y-coordinate of cursor (baseline) */
    TsInt32 caretAscender;      /* extent of cursor above the baseline (in pixels) */
    TsInt32 caretDescender;     /* extent of cursor below the baseline (in pixels) */
    TsInt32 width;              /* cursor width */
    TsFixed angle;              /* the angle of the cursor */
    TsInt32 lineNumber;         /* line number that the caret is on */
    TsInt32 glyphIndex;         /* index of the glyph that the caret is on */
    TsInt32 flags;              /* flags indicating properties of the caret. */
    TsTextDirection direction;  /* cursor direction (this will never be TS_NEUTRAL_DIRECTION) */
} TsCaretInfo;


/*****************************************************************************
 *
 *  Get information for making a caret or cursor.
 *
 *  Description:
 *      This function returns information about a cursor at the give source
 *      index. The client also specifies (through the sourceIndexOffset
 *      parameter) whether the leading or trailing edge is being requested.
 *      Some index/edge combinations at bidi boundaries have an alternate
 *      cursor location, which can also be retrieved with this function.
 *
 *  Parameters:
 *      layout            - [in] this
 *      sourceIndex       - [in] character index corresponding to caret point
 *      sourceIndexOffset - [in] indicator of leading/trailing edge
 *      primary           - [in] indicates which caret position to retrieve
 *      info              - [out] returns information about caret
 *
 *  Return value:
 *      TsResult
 *
 *  Notes:

 *     To request the caret for the leading edge of a grapheme, set
 *     sourceIndexOffset to TS_LEADING_EDGE. To request the trailing edge
 *     of a grapheme, set sourceIndexOffset to a non-zero value. i. e.
 *     the offset returned from a call to TsLayout_getIndexForXY,
 *     TsLayout_getValidPosition, TsLayout_arrowUp, or TsLayout_arrowDown.
 *
 *     To request the caret for the visual location of the glyph corresponding
 *     to sourceIndex, set primary to TRUE. Set primary to FALSE to retrieve
 *     the alternate cursor position. In most cases, the primary and
 *     alternate caret will be at the same location. They may have different
 *     locations at bidi junctions.
 *
 *     For example:
 *     Given logical buffer:  abcABCdef  and visual display: abcCBAdef
 *                    index:  012345678                      012543678
 *
 *     To get the primary cursor for the trailing edge of 'c', call
 *     TsLayout_getCaretInfo with:
 *           sourceIndex = 2; sourceIndexOffset = 1;  primary = TRUE.
 *
 *     To get the alternate cursor for the trailing edge of 'c', call
 *     TsLayout_getCaretInfo with:
 *           sourceIndex = 2; sourceIndexOffset = 1;  primary = FALSE.
 *
 *     The alternate cursor in this case would be displayed at the leading
 *      edge of 'A'.
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_getCaretInfo(TsLayout *layout, TsInt32 sourceIndex, TsInt32 sourceIndexOffset,
                          TsBool primary, TsCaretInfo *info);


/*****************************************************************************
 *
 *  Add a text container to a layout.
 *
 *  Parameters:
 *      layout      - [in] this
 *      container   - [in] the container to add
 *      containerID - [in] unique ID number for container
 *      containerOrigin - [in] the x,y coordinates for the container's origin
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_addTextContainer(TsLayout *layout, TsTextContainer *container, TsInt32 containerID, TsPoint containerOrigin);


/*****************************************************************************
 *
 *  Remove a text container from a layout.
 *
 *  Parameters:
 *      layout      - [in] this
 *      containerID - [in] unique ID number of the container to remove
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layoutedit>
 */
TS_EXPORT(TsResult)
TsLayout_removeTextContainer(TsLayout *layout, TsInt32 containerID);


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
 *      becomes invalid when this TsLayout object is destroyed.
 *      Therefore, you must keep reference to the TsLayout object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      layout     - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP layout>
 */
TS_EXPORT(TsExternalUserData *)
TsLayout_getExternalUserdata(TsLayout *layout);



#ifndef TS_NDEBUG

/*****************************************************************************
 *
 *  Typedef of a callback function for printing shaping information
 *
 *  Description:
 *      If a callback function of this type is set using
 *      TsLayout_debug_setShapingInfoCallback, it will be called for each line.
 *
 *  Parameters:
 *      data        - user data pointer
 *      start       - index into tpiArray at which the line starts
 *      end         - index into tpiArray at which the line ends
 *      tpiArray    - pointer to the layout's internal array
 *
 *  <GROUP layout>
 */
typedef void (*shapingInfoCBFunc)(void *data, TsInt32 start, TsInt32 end, TsTextProcessInfo* tpiArray);

/*****************************************************************************
 *
 *  Sets a pointer to a callback function to be called during composition
 *  to allow printing of shaping information.
 *
 *  Description:
 *      This function allows the client to set a callback function that
 *      will be called during composition so that information in the
 *      TsTextProcessInfo array can be printed.
 *
 *  Parameters:
 *      layout      - [in] this
 *      data        - [in] a userdata pointer that will be passed back to the callback
 *      func        - [in] the callback function that will be called for each line during composition
 *
 *  Return value:
 *      None.
 *
 *  Notes:
 *      This is available in debug build only. The data at the TsTextProcessInfo
 *      pointer that is passed to the callback should not be modified within the callback.
 *
 *  <GROUP layout>
 */
TS_EXPORT(void)
TsLayout_debug_setShapingInfoCallback(TsLayout *layout, void *data, shapingInfoCBFunc func);

#endif



/**************** Deprecated ************************************************/



/*****************************************************************************
 *
 *  Highlights the selected text.
 *
 *  Description:
 *      As of WTLE 2.2 this function is deprecated. This function now
 *      does nothing. Highlighting now occurs as part of the TsLayout_display,
 *      TsLayout_displayContaines, and TsLayout_displayLine functions.
 *  Parameters:
 *      layout      - [in] this
 *      dc          - [in] device context
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP layout-deprecated>
 */
TS_EXPORT(TsResult)
TsLayout_highlightSelection(TsLayout *layout, TsDC *dc);




TS_END_HEADER

#endif /* TSLAYOUT_H */
