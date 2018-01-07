/*****************************************************************************
*
*  tstextcn.h - defines interface to the TsTextContainer class.
*
*  Copyright (C) 2006 Monotype Imaging Inc. All rights reserved.
*
*  Confidential Information of Monotype Imaging Inc.
*
****************************************************************************/

#ifndef TSTEXTCONTAINER_H
#define TSTEXTCONTAINER_H

#include "tstext.h"
#include "tsevent.h"
#include "tstextat.h"
#include "tsshape.h"
#include "tseudata.h"


TS_BEGIN_HEADER

struct TsTextContainer_;

/*****************************************************************************
 *
 *  An enumeration of the possible values for the direction in which glyphs
 *  flow in the text container.
 *
 *  <GROUP textcontainer>
 */
typedef enum {
    TsGlyphsFlowLeftToRight, /* default (needed for Latin, Greek, etc.)             */
    TsGlyphsFlowRightToLeft, /* needed for Arabic, Hebrew, etc.                     */
    TsGlyphsFlowTopToBottom, /* not implemented yet, needed for vertical CJK, etc.  */
    TsGlyphsFlowBottomToTop  /* not implemented yet, needed for Ogham(?)            */
} TsGlyphsFlow;

/*****************************************************************************
 *
 *  An enumeration of the possible values for the direction in which lines
 *  flow in the text container.
 *
 *  <GROUP textcontainer>
 */
typedef enum {
    TsLinesFlowTopToBottom, /* default                                              */
    TsLinesFlowBottomToTop, /* not implemented, not needed(?)                       */
    TsLinesFlowLeftToRight, /* not implemented yet, needed for vertical Mongolian   */
    TsLinesFlowRightToLeft, /* not implemented yet, needed for vertical CJK         */
    TsLinesFlowNone         /* if used, do not advance line if no fit               */
} TsLinesFlow;

/*****************************************************************************
 *
 *  A structure containing the TsGlyphsFlow and TsLinesFlow values for the
 *  text container. Passed into TsTextContainer_getRect.
 *
 *  <GROUP textcontainer>
 */
typedef struct TsTextFlow_
{
    TsGlyphsFlow glyphsFlow;    /* direction that glyphs flow in the container */
    TsLinesFlow  linesFlow;     /* direction that lines flow in the container */
} TsTextFlow;



/*****************************************************************************
 *
 *  A text container object.
 *
 *  Description:
 *       The TsTextContainer object encapsulates information about a given text
 *      container. A pointer to a TsTextContainer object is obtained by calling
 *      TsTextContainer_new. This pointer should then be passed to the
 *      TsTextContainer_ functions.
 *
 *  <GROUP textcontainer>
 */
typedef struct TsTextContainer_ TsTextContainer;

/*****************************************************************************
 *
 *  Creates a TsTextContainer object.
 *
 *  Parameters:
 *      none
 *
 *  Return value:
 *      Pointer to new TsTextContainer object, or NULL if memory for the new
 *      object could not be allocated.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsTextContainer *)
TsTextContainer_new(void);


/*****************************************************************************
 *
 *  Creates a memory managed TsTextContainer object.
 *
 *  Parameters:
 *      memMgr  - [in] pointer to a memory manager object or NULL
 *
 *  Return value:
 *      Pointer to new TsTextContainer object, or NULL if memory for the new
 *      object could not be allocated.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsTextContainer *)
TsTextContainer_newex(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Frees a TsTextContainer object.
 *
 *  Parameters:
 *      container   - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(void)
TsTextContainer_delete(TsTextContainer *container);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsTextContainer object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsTextContainer_releaseHandle.
 *
 *  Parameters:
 *      container   - [in] this
 *
 *  Return value:
 *      Pointer to text container object.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsTextContainer *)
TsTextContainer_copyHandle(TsTextContainer *container);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsTextContainer object.
 *
 *  Description:
 *      This function reverses the action of TsTextContainer_copyHandle.
 *
 *  Parameters:
 *      container   - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(void)
TsTextContainer_releaseHandle(TsTextContainer *container);


/*****************************************************************************
 *
 *  Register an observer with the text container object.
 *
 *  Parameters:
 *      container       - [in] this
 *      observer        - [in] pointer to object that is observing the text
 *                             container
 *      callback        - [in] callback function that is called when something
 *                             about the text container changes
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                      passed in.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_registerObserver(TsTextContainer *container, void *observer, TsEventCallback callback);


/*****************************************************************************
 *
 *  Unregister an observer with the text container object.
 *
 *  Parameters:
 *      container       - [in] this
 *      observer        - [in] pointer to object that is observing the text
 *                             container
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                      passed in.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_unregisterObserver(TsTextContainer *container, void *observer);


/*****************************************************************************
 *
 *  Set the size of the TsTextContainer object.
 *
 *  Parameters:
 *      container       - [in] this
 *      x               - [in] size in x direction
 *      y               - [in] size in y direction
 *
 *  Return value:
 *      TsResult        - TS_OK if the function completes successfully;
 *                      TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is
 *                      passed in.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_setSize(TsTextContainer *container, TsInt32 x, TsInt32 y);


/*****************************************************************************
 *
 *  Get the size of the TsTextContainer object.
 *
 *  Parameters:
 *      container   - [in] this
 *      x           - [out] size in x direction
 *      y           - [out] size in y direction
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_getSize(TsTextContainer *container, TsInt32 *x, TsInt32 *y);


/*****************************************************************************
 *
 *  Set the maximum number of lines that the TsTextContainer object will have.
 *
 *  Parameters:
 *      container   - [in] this
 *      maxLines    - [in] maximum number of lines object will contain
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in;
 *                  TS_ERR_ARGUMENT_OUT_OF_RANGE if maxLines < 1.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_setMaxLines(TsTextContainer *container, TsInt32 maxLines);


/*****************************************************************************
 *
 *  Get the maximum number of lines that the TsTextContainer object will have.
 *
 *  Parameters:
 *      container   - [in] this
 *
 *  Return value:
 *      TsInt32     - the maximum number of lines; or -1 if the passed in
 *                  pointer is NULL.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsInt32)
TsTextContainer_getMaxLines(TsTextContainer *container);


/*****************************************************************************
 *
 *  Add an anchored shape to the TsTextContainer object.
 *
 *  Parameters:
 *      container   - [in] this
 *      shape       - [in] pointer to the shape object
 *      x           - [in] left position of anchored object
 *      y           - [in] top position of anchored object
 *      shapeID     - [in] ID that refers to this anchored object
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in;
 *                  TS_ERR if the shape could not be added to the container.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_addAnchoredShape(TsTextContainer *container, TsShape *shape,
                                 TsInt32 x, TsInt32 y, TsInt32 shapeID);


/*****************************************************************************
 *
 *  Remove one or more anchored shapes from the TsTextContainer object.
 *
 *  Parameters:
 *      container   - [in] this
 *      shapeID     - [in] ID that refers to the anchored object(s) to
 *                         be removed
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in;
 *                  TS_ERR_INVALID_SHAPE_ID if there is no shape has an ID
 *                  matching shapeID in the container.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_removeAnchoredShape(TsTextContainer *container, TsInt32 shapeID);


/*****************************************************************************
 *
 *  Gets the number of anchored objects that are in the TsTextContainer object.
 *
 *  Parameters:
 *      container   - [in] this
 *
 *  Return value:
 *      TsInt32     - the number of anchored objects that are in container or
 *                    -1 if an invalid container is passed in.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsInt32)
TsTextContainer_getAnchoredShapeCount(TsTextContainer *container);


/*****************************************************************************
 *
 *  Gets a rectangle from the TsTextContainer object.
 *
 *  Parameters:
 *      container     - [in] this
 *      textFlow      - [in] glyph & line flow directions
 *      proposedRect  - [in] try to fit this rectangle
 *      actualRect    - [out] actual fit, may be advanced in textFlow direction
 *      remainingSegments - [out] number of segments remaining on the given line
 *
 *  Return value:
 *      TsResult    - TS_OK if the function completes successfully;
 *                  TS_ERR_ARGUMENT_IS_NULL_PTR if a NULL pointer is passed in;
 *                  TS_ERR_UNSUPPORTED_LINE_FLOW if the linesFlow member
 *                          of the textFlow is not TsLinesFlowTopToBottom;
 *                  TS_ERR_UNSUPPORTED_GLYPHS_FLOW if the glyphsFlow member
 *                          of the textFlow is not TsGlyphsFlowLeftToRight or
 *                          TsGlyphsFlowRightToLeft;
 *                  TS_ERR_ARGUMENT_OUT_OF_RANGE if the proposedRect lies
 *                        outside of the boundaries of the container.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsResult)
TsTextContainer_getRect(TsTextContainer *container, TsTextFlow *textFlow,
              TsRect *proposedRect, TsRect *actualRect, TsInt32 *remainingSegments);


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
 *      becomes invalid when this TsTextContainer object is destroyed.
 *      Therefore, you must keep reference to the TsTextContainer object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      container   - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP textcontainer>
 */
TS_EXPORT(TsExternalUserData *)
TsTextContainer_getExternalUserdata(TsTextContainer *container);


TS_END_HEADER

#endif /* TSTEXTCONTAINER_H */
