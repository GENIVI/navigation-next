/*****************************************************************************
 *
 *  openvgdc.h - Defines interface to OpenVG-specific device context.
 *
 *  Copyright (C) 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifdef TS_ENABLE_OPENVG
#include <VG/openvg.h>
#include <EGL/egl.h>
#include "tsdc.h"
#include "tsdynarr.h"

TS_BEGIN_HEADER

/*****************************************************************************
 *
 *  Macro to assign a TsColor value to an OpenVG color represented by anarray
 *  of floating point values.
 *
 *  Notes:
 *      The color parameter is used more than once, so an expression should
 *      not be passed.
 *
 *  <GROUP openvg>
 */
#define TSCOLOR2VGCOLOR(color, vgcolor) {\
    vgcolor[0] = (VGfloat)(TS_GET_RED(color) / 255.0);\
    vgcolor[1] = (VGfloat)(TS_GET_GREEN(color) / 255.0);\
    vgcolor[2] = (VGfloat)(TS_GET_BLUE(color) / 255.0);\
    vgcolor[3] = (VGfloat)(TS_GET_ALPHA(color) / 255.0);}

/*****************************************************************************
 *
 *  Macro to convert a TsFixed value to a floating point value.
 *
 *  <GROUP openvg>
 */
#define TsFixed_toVGfloat(x) (VGfloat)( (x) / 65536.0)

/*****************************************************************************
 *
 *  Macro to check the result of a call to the OpenVG library and set a
 *  result code.
 *
 *  <GROUP openvg>
 */
#if 1
#define CHECK_VG_RESULT(func)\
    vgResult = vgGetError();\
    if (vgResult != VG_NO_ERROR)\
    {\
        TS_PRINTF_DEBUG(("%s failed. Error code %d (%s).\n", #func, vgResult, VGErrorCodeToString(vgResult)));\
        result = TS_ERR_DC;\
    }

#else
#define CHECK_VG_RESULT(func)
#endif


/*****************************************************************************
 *
 *  A typedef for a OpenvgDC object.
 *
 *  Description:
 *      The OpenvgDC class is derived from the TsDC base class.
 *
 *  <GROUP openvg>
 */
typedef struct
{
    TsDC base;                  /* the base class object. This must be first. */
    void *data;                 /* pointer to structure containing required info (currently is not used) */
    VGfloat transformMatrix[9]; /* client settable transformation matrix that is applied when rendering */
    TsGlyphLayer vgGlyphType;   /* (client settable) type of glyphs that this DC will produce. */
    TsBool useVGFont;           /* if true, use VGFont method, otherwise use drawGlyphs/drawImage method */
    TsDynamicArray vgFontArray; /* array of VGFont wrapping objects that have been created in this DC */
    VGuint *glyphIndices;       /* scratch space for the list of glyph indexes to be drawn */
    VGfloat *Xadjustments;      /* scratch space for the list of x adjustments */
    VGfloat *Yadjustments;      /* scratch space for the list of y adjustments */
    TsInt32 scratchSize;        /* size of above arrays */
} OpenvgDC;


/*****************************************************************************
 *
 *  Returns a pointer to new OpenvgDC object.
 *
 *  Description:
 *      Creates and initializes a new OpenvgDC object.
 *
 *      Each call to this function must be matched with a subsequent call
 *      to OpenvgDC_delete, when finished.
 *
 *  Parameters:
 *      memMgr - [in]   memory manager object (can be NULL if not using memory manager)
 *      data   - [in]   pointer to object containing info about the OpenVG library/egl library
 *
 *  Return value:
 *      Pointer to valid OpenvgDC object (cast to a TsDC *), or NULL on error.
 *
 *  <GROUP openvg>
 */
TS_EXPORT(TsDC *)
OpenvgDC_new(TsMemMgr *memMgr, void *data);



/*****************************************************************************
 *
 *  Deletes a OpenvgDC object.
 *
 *  Remarks:
 *      Each call to this function must be matched with a prior call to
 *      OpenvgDC_new.
 *
 *  Parameters:
 *      dc - [in] pointer to this OpenvgDC object.
 *
 *  Return value:
 *      None
 *
 *  <GROUP openvg>
 */
TS_EXPORT(void)
OpenvgDC_delete(TsDC *dc);


/*****************************************************************************
 *
 *  Set the glyph type to be rendered by the DC.
 *
 *
 *  Parameters:
 *      dc          - [in] pointer to this OpenvgDC object.
 *      vgGlyphType - [in] glyph type
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP openvg>
 */
TS_EXPORT(TsResult)
OpenvgDC_setGlyphType(TsDC *dc, TsGlyphLayer vgGlyphType);


/*****************************************************************************
 *
 *  Set the rendering method to be used by the DC.
 *
 *
 *  Parameters:
 *      dc          - [in] pointer to this OpenvgDC object.
 *      useVgFont   - [in] if true, use the VGFont construct and call
 *                         vgDrawGlyphs on runs of glyphs, otherwise use
 *                         vgDrawPath and vgDrawImage on individual glyphs
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP openvg>
 */
TS_EXPORT(void)
OpenvgDC_useVGFont(TsDC *dc, TsBool useVgFont);


/*****************************************************************************
 *
 *  Utility function to convert OpenVG error code to a string.
 *
 *  Parameters:
 *      err          - [in] error code
 *
 *  Return value:
 *      TsChar *
 *
 *  <GROUP openvg>
 */
TS_EXPORT(TsChar *)
VGErrorCodeToString(VGErrorCode err);

TS_END_HEADER

#endif /* TS_ENABLE_OPENVG */
