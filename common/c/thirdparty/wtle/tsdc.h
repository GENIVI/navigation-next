/*****************************************************************************
 *
 *  tsdc.h - Defines interface to the device context class.
 *
 *  Copyright (C) 2003-2004 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSDC_H
#define TSDC_H

#include "tsmem.h"
#include "ts2d.h"
#include "tsimage.h"
#include "tsfontst.h"
#include "tsglyph.h"
#include "tsdlist.h"


TS_BEGIN_HEADER


/******************************************************************************
 *
 *  Private structure that contains device context object data.
 *
 */
typedef struct TsDC_
{
    TsByte alpha;
    TsByte alphaTransferBitsPerPixel;
    TsFixed gamma;
    TsFixed invgamma;
    TsBool isClipRectDefined;
    TsRect clipRect;
    TsMemMgr *memMgr;
    struct TsDC_Funcs_ const *func;
    TsByte alphaT[256];
    TsByte *gammaT;
    TsByte *invgammaT;
} TsDC;


/* Note: keep this lint comment here, do not put it between the Doc-o-matic
         comment block and the macro definition. */
/*lint -emacro(704, TS_INTERP_COLOR) */
/******************************************************************************
 *
 *  Private macro to interpolate color channel's foreground with background.
 *
 *  Description:
 *      Interpolates a color channel's foreground color with the background
 *      color weighted by the n value (ranging from 0-255).
 *      The interpolation uses a right shift of signed quantity.
 *      Although, the behavior of this shift is not defined in ANSI C,
 *      it works consistently on our test platforms.
 *
 *      The macro is used by reference code. You will likely want to implement
 *      your own rendering code using a TsDC derived class.
 *
 *  Notes:
 *      This macro evaluates the b argument more than once so the argument
 *      should never be an expression with side effects.
 *
 *      The lint 704 warning: (Info -- Shift right of signed quantity (long))
 *      is suppressed for the TS_INTERP_COLOR macro.
 *
 */
#define TS_INTERP_COLOR(b, f, n)   (b + ((((f-b) * n) + 128) >>8))


/*****************************************************************************
 *
 *  The device context virtual function table.
 *
 *  Description:
 *      This structure provides the "virtual function" pointers that
 *      are necessary for the TsDC abstract base class
 *      to provide device abstraction.
 *
 *  <GROUP devicecontext>
 */
typedef struct TsDC_Funcs_
{
    TsResult(*getPixel)        (TsDC *dc, TsInt32 x, TsInt32 y, TsColor *color);    /* get a pixel value */
    TsResult(*setPixel)        (TsDC *dc, TsInt32 x, TsInt32 y, TsColor color);     /* set a pixel value */
    TsResult(*getDeviceClip)   (TsDC *dc, TsRect *clip, TsBool *haveClip);          /* get clipping rectangle that may be inherent to device or set in some other way */
    TsResult(*setDeviceClip)   (TsDC *dc, const TsRect *clip);                      /* set the device's clipping rectangle */
    TsResult(*fillRect)        (TsDC *dc, const TsRect *rect, TsColor color);       /* draw a filled rectangle */
    TsResult(*highlightRect)   (TsDC *dc, const TsRect *rect);                      /* highlight a rectangle - may be reverse video */
    TsResult(*drawLine)        (TsDC *dc, TsInt32 x1, TsInt32 y1, TsInt32 x2, TsInt32 y2, TsPen *pen);  /* draw a line */
    TsResult(*drawImage)       (TsDC *dc, TsImageType type, const void *imageData, TsInt32 x, TsInt32 y); /* draw a single glyph image */
    TsResult(*drawGlyphs)      (TsDC *dc, TsDisplayListEntry *entry, TsInt32 numGlyphs, TsFontStyle *style, TsGlyphLayer layer,
                                                            TsColor color, TsInt32 x, TsInt32 y, TsRect *clipRect); /* draw a run of glyphs */
    TsResult(*setTransform)    (TsDC *dc, TsMatrix33 *m);                           /* set the transform to apply */
} TsDC_Funcs;


/*****************************************************************************
 *
 *  Gets the color at a given pixel.
 *
 *  Description:
 *      Macro that calls the derived class's 'getPixel'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *      <CODE>
 *
 *      TsResult TsDC_getPixel(TsDC *dc, TsInt32 x, TsInt32 y, TsColor *color);
 *
 *      </CODE>
 *
 *  Parameters:
 *      dc          - [in] this
 *      x           - [in] x coordinate
 *      y           - [in] y coordinate
 *      color       - [out] color at pixel location x,y
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_getPixel(dc, x, y, color) ((dc != NULL) ? \
    (dc)->func->getPixel((dc), (x), (y), (color)) : TS_ERR_ARGUMENT_IS_NULL_PTR)



/*****************************************************************************
 *
 *  Sets the color at a given pixel.
 *
 *  Description:
 *      Macro that calls the derived class's 'setPixel'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *      <CODE>
 *
 *      TsResult TsDC_setPixel(TsDC *dc, TsInt32 x, TsInt32 y, TsColor color);
 *
 *      </CODE>
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      x           - [in] x coordinate
 *      y           - [in] y coordinate
 *      color       - [in] color value to set at x,y
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_setPixel(dc, x, y, color) ((dc != NULL) ? (dc)->func->setPixel((dc), (x), (y), (color)) \
                                                     : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Gets a clipping rectangle that optionally may be stored as part of
 *  the device data.
 *
 *  Description:
 *      Macro that calls the derived class's 'getDeviceClip'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_getDeviceClip(TsDC *dc, TsRect *clipRect, TsBool *haveClip);
 *      </CODE>
 *
 *      The relevance of this function depends on how device
 *      context is defined as part of port. This function is not currently
 *      used by the core WTLE software.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      clipRect    - [out] returns clipping rectangle
 *      haveClip    - [out] True if rectangle is defined, else FALSE.
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_getDeviceClip(dc, clipRect, haveClip) ((dc != NULL) ? (dc)->func->getDeviceClip((dc), (clipRect), (haveClip)) \
                                                                 : TS_ERR_ARGUMENT_IS_NULL_PTR)



/*****************************************************************************
 *
 *  Sets a clipping rectangle that optionally may be stored as part of
 *  the device data.
 *
 *  Description
 *      Macro that calls the derived class's 'setDeviceClip'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_setDeviceClip(TsDC *dc, TsRect *clipRect);
 *      </CODE>
 *
 *      The relevance of this function depends on how device
 *      context is defined as part of port. This function is not currently
 *      used by the core WTLE software.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      clipRect    - [in] pointer to clipping rectangle
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_setDeviceClip(dc, clipRect) ((dc != NULL) ? (dc)->func->setDeviceClip((dc), (clipRect) \
                                                       : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Fill a rectangle.
 *
 *  Description:
 *      Macro that calls the derived class's 'fillRect'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_fillRect(TsDC *dc, const TsRect *rect, TsColor color);
 *      </CODE>
 *
 *      The right and lower edges of the rectangle are not included as
 *      part of the fill.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      rect        - [in] pointer to rectangle to fill
 *      color       - [in] color value for fill
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_fillRect(dc, rect, color) ((dc != NULL) ? (dc)->func->fillRect((dc), (rect), (color)) \
                                                     : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Highlight a rectangle.
 *
 *  Description:
 *      Macro that calls the derived class's 'highlightRect'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_highlightRect(TsDC *dc, const TsRect *rect);
 *      </CODE>
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      rect        - [in] pointer to rectangle to highlight
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_highlightRect(dc, rect) ((dc != NULL) ? (dc)->func->highlightRect((dc), (rect)) \
                                                   : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Draw a line.
 *
 *  Description:
 *      Macro that calls the derived class's 'drawLine'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_drawLine(TsDC *dc, TsInt32 x1, TsInt32 y1, TsInt32 x2, TsInt32 y2, TsPen *pen);
 *      </CODE>
 *
 *      The line is drawn from coordinates x1, y1 to x2, y2.
 *      The line does not include the pixel x2, y2.
 *      The style, thickness, and color of the
 *      line are controlled by the 'pen' argument and the
 *      capabilities of the specific device context.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      x1          - [in] x-coordinate of start of line
 *      y1          - [in] y-coordinate of start of line
 *      x2          - [in] x-coordinate of end of line
 *      y2          - [in] y-coordinate of end of line
 *      pen         - [in] pointer to pen used to draw line
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_drawLine(dc, x1, y1, x2, y2, pen) \
      ((dc != NULL) ? (dc)->func->drawLine((dc), (x1), (y1), (x2), (y2), (pen)) \
                    : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Draw an image.
 *
 *  Description:
 *      Macro that calls the derived class's 'drawImage'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_drawImage(TsDC *dc, TsImageType type, const void *image, TsInt32 x, TsInt32 y);
 *      </CODE>
 *
 *      The upper left hand corner of the image is put at coordinate x,y.
 *      Several image types are defined, corresponding to several glyph types.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      type        - [in] one of several image types that are supported
 *      image       - [in] pointer to image data
 *      x           - [in] x-coordinate of image location
 *      y           - [in] y-coordinate of image location
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_drawImage(dc, type, image, x, y) \
      ((dc != NULL) ? (dc)->func->drawImage((dc), (type), (image), (x), (y)) \
                    : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Draw a run of glyphs
 *
 *  Description:
 *      Macro that calls the derived class's 'drawGlyphs'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *
 *      <CODE>
 *      TsResult TsDC_drawGlyphs(TsDC *dc, TsDisplayListEntry *entry, TsUInt32 numGlyphs,
 *                                      TsFontStyle *style, TsGlyphLayer, layer, TsColor color, TsInt32 x, TsInt32 y, TsRect *clipRect);
 *      </CODE>
 *
 *      The upper left hand corner of the image is put at coordinate x,y.
 *      Several image types are defined, corresponding to several glyph types.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      entry       - [in] pointer to first entry of display list for this run
 *      numGlyphs   - [in] the number of glyphs to draw
 *      style       - [in] fontStyle for this run
 *      layer       - [in] which layer to draw (only one layer at a time)
 *      color       - [in] color for this run
 *      x           - [in] x-coordinate of line in display coordinates
 *      y           - [in] y-coordinate of line in display coordinates
 *      clipRect    - [in] clipRect - if NULL then clip rect includes all glyphs
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_drawGlyphs(dc, entry, numGlyphs, style, layer, color, x, y, clipRect) \
      ((dc != NULL) ? (dc)->func->drawGlyphs((dc), (entry), (numGlyphs), (style), (layer), (color), (x), (y), (clipRect)) \
                    : TS_ERR_ARGUMENT_IS_NULL_PTR)


/*****************************************************************************
 *
 *  Sets the transform to apply.
 *
 *  Description:
 *      Macro that calls the derived class's 'setTransform'
 *      function as declared in TsDC_Funcs.
 *      The equivalent function call would be:
 *      <CODE>
 *
 *      TsResult TsDC_setTransform(TsDC *dc, TsMatrix33 *m);
 *
 *      </CODE>
 *
 *      This matrix would be set/used for example in the case that your device
 *      context is capable of performing rotation/shearing of glyphs in
 *      hardware.
 *
 *  Notes:
 *      This "function" is implemented as a macro and it evaluates the
 *      dc argument more than once so the argument should never be an
 *      expression with side effects.
 *
 *  Parameters:
 *      dc          - [in] this
 *      m           - [in] the 3x3 matrix
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
#define TsDC_setTransform(dc, m) ((dc != NULL) ? (dc)->func->setTransform((dc), (m)) \
                                                     : TS_ERR_ARGUMENT_IS_NULL_PTR)



/*****************************************************************************
 *
 *  Initialize a device context object.
 *
 *  Description:
 *      This function should be called by the derived class initializer.
 *      Generally, client code will not call this function.
 *
 *  Parameters:
 *      dc          - [in] this
 *      f           - [in] pointer to function pointer array
 *      memMgr      - [in] memory manager object
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
TsResult
TsDC_init(TsDC *dc, const TsDC_Funcs *f, TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Finishes a device context object.
 *
 *  Description:
 *      This function should be called by the derived class done function.
 *      Generally, client code will not call this function.
 *
 *      This function cleans up the base class upon a "done".
 *
 *  Parameters:
 *      dc          - [in] this
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
TsResult
TsDC_done(TsDC *dc);


/*****************************************************************************
 *
 *  Gets the current clipping rectangle used by the device context.
 *
 *  Parameters:
 *      dc          - [in]  handle to device context object
 *      clipRect    - [out]  clipping rectangle
 *      haveClip    - [out] True if rectangle is defined, else FALSE.
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
TS_EXPORT(TsResult)
TsDC_getClipRect(TsDC *dc, TsRect *clipRect, TsBool *haveClip);


/*****************************************************************************
 *
 *  Sets the current device context clipping rectangle.
 *
 *  Notes:
 *      Be careful when using this macro references the dc argument more than once.
 *      This can cause side effects.
 *
 *  Parameters:
 *      dc          - [in]  handle to device context object
 *      clipRect    - [in]  clipping rectangle
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
TS_EXPORT(TsResult)
TsDC_setClipRect(TsDC *dc, const TsRect *clipRect);


/*****************************************************************************
 *
 *  Sets the current gamma value.
 *
 *  Description:
 *      This function sets a gamma value that can be used by the device
 *      context to account for non-linearities in the display system so that
 *      alpha blending occurs in a linear space.
 *
 *      The gamma value is applied to the base class and then forwarded to
 *      the derived classes for possible use there. The base class uses the
 *      value within the reference rendering code.
 *
 *      If gamma <= 0.0 then the gamma is set to 1.0.
 *
 *  Parameters:
 *      dc          - [in]  handle to device context object
 *      gamma       - [in]  gamma as a 16.16 fixed point number
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
TS_EXPORT(TsResult)
TsDC_setGamma(TsDC *dc, TsFixed gamma);


/*****************************************************************************
 *
 *  Gets a transfer function for use during alpha blending.
 *
 *  Description:
 *      This function is used internally by TsDC and it may be used by
 *      classes derived from TsDC as well. There is no need to call this
 *      function externally.
 *
 *      The function returns a transfer function, alphaT, containing
 *      n = 2^bitsPerPixel values:
 *          If n>1:
 *              alphaT[i] = (alpha * i)/(n-1)
 *
 *      The TsDC caches a transfer function so that it doesn't need to
 *      re-compute the function unless the parameters have changed.
 *
 *  Parameters:
 *      dc              - [in]  handle to device context object
 *      alpha           - [in]  current alpha channel value (0-255)
 *      bitsPerPixel    - [in]  the bits per pixel of image being blitted
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP devicecontext>
 */
TS_EXPORT(const TsByte *)
TsDC_getAlphaTransfer(TsDC *dc, TsByte alpha, TsInt32 bitsPerPixel);


TS_END_HEADER


#endif /* TSDC_H */
