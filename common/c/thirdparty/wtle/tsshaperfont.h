/*****************************************************************************
 *
 *  tsshaperfont.h - Interface to TsShaperFont class.
 *
 *  Copyright (C) 2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifndef TSSHAPERFONT_H
#define TSSHAPERFONT_H

#include "tsbase.h"     /* Base layer public API */
#include "otlayout.h"
#include "tsshapertext.h"

TS_BEGIN_HEADER

struct TsShaperFont_;


/*****************************************************************************
 *
 *  The TsShaperFont object.
 *
 *  Description:
 *      This object provides an interface between the client's font data and
 *      the shaping code. The client can set up the call back functions once,
 *      and then reuse this object by using TsShaperFont_setData &
 *      TsShaperFont_clearData.
 *
 *  <GROUP shaperfont>
 */
typedef struct TsShaperFont_ TsShaperFont;


/*****************************************************************************
 *
 *  Enumerated list of shaper font metrics.
 *
 *  Description:
 *      These metrics are used as arguments to 
 *      TsShaperFont functions that return font metrics.
 *
 *  <GROUP shaperfont>
 */
typedef enum
{
    TS_SHAPER_FONT_IS_PAIRWISE_KERNING_SUPPORTED,      /* font and font engine support pairwise kerning */
    TS_SHAPER_FONT_IS_THAI_PRIVATE_USE_AREA_POPULATED /* font has glyphs in the Thai PUA */
} TsShaperFontMetric;

/*****************************************************************************
 *
 *  The shaperFont virtual function table.
 *
 *  Description:
 *      This structure provides the "virtual function" pointers that are
 *      necessary for the "abstract class" TsShaperFont to provide font style
 *      services.  This structure must be properly initialized and passed
 *      to the TsShaperFont constructor via TsShaperFont_new.
 *
 *  <GROUP shaperfont>
 */
typedef struct TsShaperFontFuncs_
{
    void (*designUnits2pixels)(void *data, TsFixed dupem, TsMatrix *m, TsInt16 xdu, TsInt16 ydu, TsFixed *xp, TsFixed *yp); /* convert from design units to pixels */
    /* the above function (pointer) is retained purely to get access to fixed point math functions */
    TsBool (*getGPOSpoint)(void *data, TsUInt16 glyphID, TsUInt16 pointIndex, TsFixed26_6 *x, TsFixed26_6 *y); /* may be NULL */
    TsFixed (*getXSize)(void *data);     /* return pixels per EM */
    TsFixed (*getYSize)(void *data);     /* return pixels per EM */
    TsResult (*mapChar)(void *data, TsInt32 charID, TsInt32 *glyphID); /* map character to glyph index */
    TsOtCache *(*getOpenTypeCache)(void *data); /* return pointer to client-allocated TsOtCache object, or NULL if caching is not being used */
    TsOtLayout *(*findOtLayout)(void *data, TsTag script, TsTag langSys); /* return pointer to a TsOtLayout object corresponding to the given script and language, or NULL if the font does not have tables for those values */
    TsBool (*getBoolMetric)(void *data, TsShaperFontMetric metric); /* get shaper font boolean metric */
    TsResult (*getKerning)(void *data, TsInt32 glyphID_1, TsInt32 glyphID_2, TsFixed *dx, TsFixed *dy);
    TsBool (*getGPOSscale)(void *data, TsUInt16 glyphID, TsUInt16 *du, TsMatrix *m, TsFixed *xppm, TsFixed *yppm);
} TsShaperFontFuncs;



/*****************************************************************************
 *
 *  Creates a new memory managed TsShaperFont object.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      funcs       - [in] a properly initialized TsShaperFontFuncs structure.
 *      data        - [in] the derived class' private data.  This void pointer
 *                     will be passed as the first argument to each of the
 *                     functions defined in TsShaperFontFuncs.
 *
 *  Return value:
 *      Pointer to new TsShaperFont object or NULL upon error.
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsShaperFont *)
TsShaperFont_new(TsMemMgr *memMgr, const TsShaperFontFuncs *funcs, void *data);


/*****************************************************************************
 *
 *  Sets (resets) the private data pointer of a TsShaperfont object.
 *
 *  Parameters:
 *      shaperFont  - [in] this
 *      data        - [in] the derived class' private data.  This void pointer
 *                     will be passed as the first argument to each of the
 *                     functions defined in TsShaperFontFuncs.
 *
 *  Return value:
 *      none
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(void)
TsShaperFont_setData(TsShaperFont *shaperFont, void *data);


/*****************************************************************************
 *
 *  Clears the private data pointer of a TsShaperfont object.
 *
 *  Parameters:
 *      shaperFont  - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(void)
TsShaperFont_clearData(TsShaperFont *shaperFont);


/*****************************************************************************
 *
 *  Frees a TsShaperFont object.
 *
 *  Parameters:
 *      shaperFont      - [in] this
 *
 *  Return value:
 *      none
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(void)
TsShaperFont_delete(TsShaperFont *shaperFont);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsShaperFont object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsShaperFont_releaseHandle.
 *
 *  Parameters:
 *      shaperFont      - [in] this
 *
 *  Return value:
 *      Pointer to a shaperFont object.
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsShaperFont *)
TsShaperFont_copyHandle(TsShaperFont *shaperFont);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsShaperFont object.
 *
 *  Description:
 *      This function reverses the action of TsShaperFont_copyHandle.
 *
 *  Parameters:
 *      shaperFont     - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(void)
TsShaperFont_releaseHandle(TsShaperFont *shaperFont);


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
 *      becomes invalid when this TsShaperFont object is destroyed.
 *      Therefore, you must keep a reference to the TsShaperFont object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      shaperFont     - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsExternalUserData *)
TsShaperFont_getExternalUserdata(TsShaperFont *shaperFont);


/*****************************************************************************
 *
 *  Maps a Unicode character through the font cmap to produce a glyph index.
 *
 *  Parameters:
 *      shaperFont     - [in] this
 *      charID         - [in] Unicode character
 *      glyphID        - [out] glyph index
 *
 *  Return value:
 *      TS_OK on success, else error code.
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsResult)
TsShaperFont_mapChar(TsShaperFont *shaperFont, TsInt32 charID, TsInt32 *glyphID);

#ifndef TS_NO_OPENTYPE
/******************************************************************************
 *
 *  Finds and returns the OpenType table layout object associated with
 *  the script and language system, if present, for the TsShaperFont.
 *
 *  Parameters:
 *      shaperFont - [in] pointer to TsShaperFont object
 *      script     - [in] script
 *      langSys    - [in] language system
 *
 *  Return value:
 *      TsOtLayout pointer
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsOtLayout *)
TsShaperFont_findOtLayout(TsShaperFont *shaperFont, TsTag script, TsTag langSys);


/******************************************************************************
 *
 *  Use OpenType tables that are available to shape some text.
 *
 *  Parameters:
 *      shaperFont - [in] pointer to TsShaperFont object
 *      shaperText - [in/out] pointer to TsShaperText to be shaped
 *      layout     - [in] OpenType layout object as returned from TsFontStyle_findOtLayout
 *
 *  Return value:
 *      TS_OK on success, else error code.
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsResult)
TsShaperFont_otShape(TsShaperFont *shaperFont, TsShaperText *shaperText, TsOtLayout *layout);
#endif /* TS_NO_OPENTYPE */


/******************************************************************************
 *
 *  Returns a boolean metric.
 *
 *  Description:
 *      The metric argument may be:
 *
 *      <TABLE>
 *      <B>metric value                             <B>Returns TRUE if
 *      TS_SHAPER_FONT_IS_PAIRWISE_KERNING_SUPPORTED       font and font engine support pairwise kerning
 *      TS_SHAPER_FONT_IS_THAI_PRIVATE_USE_AREA_POPULATED  font has glyphs in the Thai PUA
 *      </TABLE>
 *
 *      This function is implemented by the derived class. The default value for
 *      unsupported metrics is FALSE.
 *
 *  Parameters:
 *      shaperFont  - [in] pointer to TsShaperFont object
 *      metric      - [in] type of metric being requested
 *
 *  Return value:
 *      Metric value
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsBool)
TsShaperFont_getBoolMetric(TsShaperFont *shaperFont, TsShaperFontMetric metric);


/*****************************************************************************
 *
 *  Returns a pairwise kerning adjustment for a glyph pair.
 *
 *  Parameters:
 *      shaperFont  - [in] pointer to TsShaperFont object
 *      glyphID_1   - [in] glyphID of first glyph in pair
 *      glyphID_2   - [in] glyphID of second glyph in pair
 *      dx          - [out] returns the x-dimension kerning value
 *      dy          - [out] returns the y-dimension kerning value
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsResult)
TsShaperFont_getKerning(TsShaperFont *shaperFont, TsInt32 glyphID_1, TsInt32 glyphID_2, TsFixed *dx, TsFixed *dy);

/*****************************************************************************
 *
 *  Map chars in text
 *
 *  Parameters:
 *      shaperFont        - [in] pointer to TsShaperFont object
 *      shaperText        - [in] TsShaperText object
 *      startIndex        - [in] start index of range to shape
 *      endIndex          - [in] end index of range to shape
 *      setContext        - [in] if TRUE, set context to TS_CONTEXT_NONE
 *      arrayGrowth       - [out] the number of characters by which the TsShaperText grew
 *
 *  Return value:
 *      TS_OK on success, else error code.
 *
 *  <GROUP shaperfont>
 */
TS_EXPORT(TsResult)
TsShaperFont_mapChars(TsShaperFont *shaperFont, TsShaperText *shaperText,
              TsInt32 startIndex, TsInt32 endIndex,
              TsBool setContext,
              TsInt32 *arrayGrowth
);


TS_END_HEADER

#endif /* TSSHAPERFONT_H */
