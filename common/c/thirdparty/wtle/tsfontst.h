/*****************************************************************************
 *
 *  tsfontst.h - Defines the interface to the font style class.
 *
 *  Copyright (C) 2003, 2008 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSFONTST_H
#define TSFONTST_H

#include "tsproj.h"
#include "tsglyph.h"
#include "ts2d.h"
#include "tstag.h"
#include "tsfntref.h"
#include "tseudata.h"
#ifndef TS_NO_OPENTYPE
#include "otlayout.h"
#include "otwrdblk.h"
#endif


TS_BEGIN_HEADER

/* These macros are designed for use with the types defined below.
 * See those definitions for descriptions of how to use these macros.
 */

#define TS_BASELINE_NORMAL          0
#define TS_BASELINE_SUPERSCRIPT     1
#define TS_BASELINE_SUBSCRIPT       2

#define TS_EDGE_NORMAL              0       /* normal text without any edge effect */
#define TS_EDGE_EMBOSSED            1       /* text has a raised appearance */
#define TS_EDGE_ENGRAVED            2       /* text has a depressed appearance */
#define TS_EDGE_OUTLINED            3       /* text is just an outline */
#define TS_EDGE_DROP_SHADOW         4       /* text has a shadow */
#define TS_EDGE_RAISED              5       /* similar to embossed, but more of a shadow effect */
#define TS_EDGE_DEPRESSED           6       /* similar to engraved, but more of a shadow effect */
#define TS_EDGE_UNIFORM             7       /* normal text with a colored edge around it, outline is filled */
#define TS_EDGE_UNFILLED            8       /* normal text with a colored edge around it, outline is unfilled*/
#define TS_EDGE_GLOW                9       /* normal text with a soft "glowing" colored edge around it */

#define TS_EMPHASIS_NORMAL          0x00    /* normal text */
#define TS_EMPHASIS_BOLD            0x01    /* bold */
#define TS_EMPHASIS_ITALIC          0x02    /* italic */
#define TS_EMPHASIS_BOLDITALIC      0x03    /* bold and italic: (TS_EMPHASIS_BOLD | TS_EMPHASIS_ITALIC) */
#define TS_EMPHASIS_UNDERLINE       0x04    /* underlined */
#define TS_EMPHASIS_STRIKEOUT       0x08    /* strikeout */
#define TS_EMPHASIS_OVERLINE        0x10    /* overlined */

#define TS_REGULAR_BOLD             0x0001  /* regular outline emboldening */
#define TS_FRACTIONAL_BOLD          0x0002  /* fractional outline emboldending */
#define TS_ADD_WEIGHT_BOLD          0x0004  /* CJK emboldening */
#define TS_CHECK_CONTOUR_WINDING    0x0008  /* controls whether check on invalid contour winding is done */
#define TS_DROPOUTS                 0x0010  /* controls whether extra dropout protection is applied */
#define TS_DISABLE_HINTS            0x0020  /* disables hinting */
#define TS_DISABLE_AUTOHINT         0x0040  /* disables autohinting */
#define TS_FORCE_AUTOHINT           0x0080  /* autohinting is applied even whether glyph is hinted or not */ 
#define TS_SOFTENED                 0x0100  /* enables softening algorithm when rendering grayscale glyphs */
#define TS_ENABLE_FIAB              0x0200  /* enables FIAB (deprecated as of iType 4.4 and WTLE 3.3 */

#define TS_RENDER_DEFAULT           0x0000  /* use the font engine's default rendering */
#define TS_RENDER_BINARY            0x0001  /* binary,    1 bit per pixel */
#define TS_RENDER_GRAYSCALE2        0x0002  /* grayscale, 2 bits per pixel */
#define TS_RENDER_GRAYSCALE4        0x0004  /* grayscale, 4 bits per pixel */
#define TS_RENDER_GRAYSCALE8        0x0008  /* grayscale, 8 bits per pixel */
#define TS_RENDER_GRAYSCALE         TS_RENDER_GRAYSCALE4  /* grayscale antialiasing */
#define TS_RENDER_EDGE_TECH         0x0400  /* Edge Technology grayscale rendering (4-bit grayscale) */
#define TS_RENDER_DISTANCE_FIELD    0x0800  /* Adaptive distance field (Edge Technology required) */
#define TS_RENDER_RASTER_ICON       0x0020  /* raster icon */
#define TS_RENDER_VECTOR_ICON       0x0040  /* vector icon */


/*****************************************************************************
 *
 *  Enumerated list of font metrics.
 *
 *  Description:
 *      These metrics are used as arguments to various TsFont and/or
 *      TsFontStyle functions that return font metrics.
 *
 *  <GROUP fontparam>
 */
typedef enum
{
    TS_FONT_DO_UNDERLINE,                       /* display routine should draw underline */
    TS_FONT_DO_STRIKEOUT,                       /* display routine should draw strikeout */
    TS_FONT_DO_OVERLINE,                        /* display routine should draw overline */
    TS_FONT_DRAW_EDGE_LAYER,                    /* display routine should draw edge layer */
    TS_FONT_DRAW_EDGE_LAYER_AFTER_TEXT,         /* display routine should draw edge layer after text layer */
    TS_FONT_IS_COMPLEX_TO_DISPLAY,              /* displaying a glyph is more complex due to edge/underline/strikeout/overline/superscript/subscript */
    TS_FONT_IS_SUPERSCRIPT,                     /* font style is superscripted */
    TS_FONT_IS_SUBSCRIPT,                       /* font style is subscripted */
    TS_FONT_IS_BOLD,                            /* font style is emboldened */
    TS_FONT_IS_ITALIC,                          /* font style is italic */
    TS_FONT_IS_PAIRWISE_KERNING_SUPPORTED,      /* font and font engine support pairwise kerning */
    TS_FONT_IS_TO_RENDER_COLOR_ICONS,           /* font style may return colored icons */
    TS_FONT_IS_FIXED_PITCH,                     /* font is fixed pitch */
    TS_FONT_IS_THAI_PRIVATE_USE_AREA_POPULATED, /* font has glyphs in the Thai PUA */
    TS_FONT_X_BASELINE_OFFSET,                  /* offset due to superscript/subscript in x-dir */
    TS_FONT_Y_BASELINE_OFFSET,                  /* offset due to superscript/subscript in y-dir */
    TS_FONT_ASCENDER,                           /* font style ascender */
    TS_FONT_DESCENDER,                          /* font style descender */
    TS_FONT_LEADING,                            /* font style leading */
    TS_FONT_X_SIZE,                             /* x-dir font size in ppem */
    TS_FONT_Y_SIZE,                             /* y-dir font size in ppem */
    TS_FONT_ITALIC_ANGLE,                       /* italic angle */
    TS_FONT_ORIENTATION,                        /* glyph rotation angle */
    TS_FONT_STRIKEOUT_THICKNESS,                /* thickness of strikeout */
    TS_FONT_STRIKEOUT_POSITION,                 /* position of strikeout relative to baseline */
    TS_FONT_UNDERLINE_THICKNESS,                /* thickness of underline */
    TS_FONT_UNDERLINE_POSITION,                 /* position of underline relative to baseline */
    TS_FONT_OVERLINE_THICKNESS,                 /* thickness of overline */
    TS_FONT_OVERLINE_POSITION,                  /* position of overline relative to baseline */
    TS_FONT_CURSOR_ASCENDER,                    /* recommended cursor ascender */
    TS_FONT_CURSOR_DESCENDER,                   /* recommended cursor descender */
    TS_FONT_TYPO_ASCENDER,                      /* typographic ascender */
    TS_FONT_TYPO_DESCENDER,                     /* typographic descender */
    TS_FONT_TYPO_LINEGAP                        /* typographic line gap */
} TsFontMetric;


/*****************************************************************************
 *
 *  Typedef for baseline style.
 *
 *  Description:
 *      <TABLE>
 *      <B>Baseline style               <B>Description
 *      TS_BASELINE_NORMAL              normal baseline
 *      TS_BASELINE_SUPERSCRIPT         baseline adjusted for superscript
 *      TS_BASELINE_SUBSCRIPT           baseline adjusted for subscript
 *      </TABLE>
 *
 *  <GROUP fontparam>
 */
typedef TsUInt8  TsBaselineStyle;


/*****************************************************************************
 *
 *  Typedef for text edge style.
 *
 *  Description:
 *      <TABLE>
 *      <B>Edge style                   <B>Description
 *      TS_EDGE_NORMAL                  normal text without any edge effect
 *      TS_EDGE_EMBOSSED                text has a raised appearance
 *      TS_EDGE_ENGRAVED                text has a depressed appearance
 *      TS_EDGE_OUTLINED                text is just an outline
 *      TS_EDGE_DROP_SHADOW             text has a shadow
 *      TS_EDGE_RAISED                  similar to embossed, but more of a shadow effect
 *      TS_EDGE_DEPRESSED               similar to engraved, but more of a shadow effect
 *      TS_EDGE_UNIFORM                 normal text with a colored edge around it, outline is filled
 *      TS_EDGE_UNFILLED                normal text with a colored edge around it, outline is unfilled
 *      TS_EDGE_GLOW                    normal text with a soft "glowing" colored edge around it
 *      </TABLE>
 *
 *      TS_EDGE_UNIFORM and TS_EDGE_UNFILLED are similar effects: a normal glyph with a
 *      colored outline around it. The difference is in the outlined edge layer.
 *
 *      TS_EDGE_UNIFORM produces a filled outline - if the font engine supports it.
 *      This approach provides a uniform filled background upon which the text layer is drawn.
 *      Use this option to avoid fringing effects that could occur with unfilled outlines.
 *      Filled outlines are available in iType 3.1 and later.
 *
 *      TS_EDGE_UNFILLED produces an unfilled outline. If either color is fully or partially
 *      transparent (using alpha channel) then choose this option in order to retain the view of
 *      the original background.
 *
 *  <GROUP fontparam>
 */
typedef TsUInt8 TsEdgeStyle;


/*****************************************************************************
 *
 *  Typedef for text emphasis style.
 *
 *  Description:
 *      Emphasis style is a bit field so the available options can be
 *      logically OR'd to produce the desired result.
 *
 *      The TS_EMPHASIS_BOLDITALIC value is defined as:
 *      TS_EMPHASIS_BOLD | TS_EMPHASIS_ITALIC.
 *
 *      Underline/strikeout/overline values were added to emphasis
 *      in Version 3.1. This was done: (1) to provide a mechanism for defining
 *      these attributes when using string-based layout, (2) to support the
 *      MIDP3 specification which defines underline as part of the font attributes,
 *      and (3) to allow for logic that detect fonts that are underlined by design
 *      and thus avoid drawing the underline in this case.
 *
 *      When using emphasis to control underline/strikeout/overline the line
 *      color is set equal to the text color.
 *
 *      If using string-based layout then emphasis is the only way to control
 *      these attributes.
 *
 *      If not using string-based layout then you have the choice of using
 *      this emphasis attribute or using the normal text attribute functions:
 *      TsText_setUnderline, TsText_setStrikeout and TsText_setOverline.
 *      When using the text functions the line color is determined from
 *      the values set using the TsText_setUnderlineColor,
 *      TsText_setSrikeoutColor and TsText_setOverlineColor functions.
 *
 *      It may be more efficient to use the text attribute approach since
 *      it leads to fewer font styles. Each font style takes up space
 *      for the object and for its metrics cache.
 *
 *      We recommend choosing one approach or the other. As soon as any part of
 *      the text is assigned a non-default underline attribute then the underline
 *      part of emphasis is ignored throughout the whole layout. The same
 *      logic is applied to strikeout and overline.
 *
 *      Note that the ability to provide bold and italic is a function of
 *      the font engine and the font. For example, if a bold font is not available
 *      and if the font engine's pseudo-bold feature is disabled, then the
 *      glyph will not be emboldened.
 *
 *  Description:
 *      <TABLE>
 *      <B>Emphasis style               <B>Description
 *      TS_EMPHASIS_NORMAL              Normal
 *      TS_EMPHASIS_BOLD                Bold
 *      TS_EMPHASIS_ITALIC              Italic
 *      TS_EMPHASIS_BOLDITALIC          Bold and Italic
 *      TS_EMPHASIS_UNDERLINE           Underlined
 *      TS_EMPHASIS_STRIKEOUT           Strikeout
 *      TS_EMPHASIS_OVERLINE            Overlined
 *      </TABLE>
 *
 *  <GROUP fontparam>
 */
typedef TsUInt8 TsEmphasisStyle;


/*****************************************************************************
 *
 *  Typedef for font-engine specific rendering flags
 *
 *  Description:
 *      This type should be treated as a bit field. Use these values to
 *      control font engine behavior.
 *
 *      The values below map to iType 4.4's flag settings. For other font engines
 *      the values are mapped as best as possible. Some may have no effect.
 *
 *      WTLE allows the flags so be set in any combination, but the underlying
 *      font engine will enforce mutual exclusion rules on certain combinations.
 *      WTLE will not generate an error if mutual exclusion rules are applied
 *      or if the behavior is not supported by the font engine, so you should
 *      consult the font engine documentation to understand the effect of
 *      setting a flag or combination of flags.
 *
 *      The bold flags do not actually enable psuedo-emboldening; instead they
 *      specify the method used when psuedo-emboldening is applied. If no
 *      bold flag is set then the default emboldening algorithm is applied.
 *      iType enforces a mutual exclusion rule for the boldness flags.
 *      If more than one is set then preference is given to the top-most choice in
 *      the list below. That occurs because WTLE calls iType with the flags
 *      in bottom-to-top of the order shown below.
 *
 *      The hinting flags have some exclusions. See iType documentation for 
 *      specific rules. Where exclusions apply, preference is given to the top-most
 *      choice in the list below.
 *
 *      <TABLE>
 *      <B>Flag                         <B>Effect of setting bit
 *      TS_REGULAR_BOLD                 enables regular outline emboldening method
 *      TS_FRACTIONAL_BOLD              enables fractional outline emboldending method
 *      TS_ADD_WEIGHT_BOLD              enables CJK emboldening method
 *      TS_CHECK_CONTOUR_WINDING        enables check on invalid contour winding
 *      TS_DROPOUTS                     enables extra dropout protection
 *      TS_DISABLE_HINTS                disables hinting
 *      TS_DISABLE_AUTOHINT             disables autohinting
 *      TS_FORCE_AUTOHINT               autohinting is applied even whether glyph is hinted or not
 *      TS_SOFTENED                     enables softening algorithm when rendering grayscale glyphs
 *      TS_ENABLE_FIAB                  (deprecated - as of WTLE 3.3 this flag does nothing)
 *      </TABLE>
 *
 *  <GROUP fontparam>
 */
typedef TsUInt16 TsRenderFlags;

/*****************************************************************************
 *
 *  Typedef for render style.
 *
 *  Description:
 *      RenderStyle is treated as a bit field.
 *      The bits may be logically OR'd to produce a requested render style.
 *      The font engine will look at the request and deliver a glyph type
 *      according to its particular capabilities. This may be affected by
 *      build options.
 *
 *      <B>When using iType:</B>
 *
 *      iType 4.0 and later support a precedence style request.
 *      If the build/font combination supports a renderStyle then the
 *      glyph will be rendered that way, otherwise it will choose the
 *      next lower precedence renderStyle that is set in the bit field
 *      according to this precedence table:
 *
 *      <TABLE>                     <Rendering>
 *      TS_RENDER_VECTOR_ICON       Return a vector icon
 *      TS_RENDER_RASTER_ICON       Return a raster icon
 *      TS_RENDER_EDGE_TECH         Use Edge Technology
 *      TS_RENDER_GRAYSCALE4        4-bit grayscale
 *      TS_RENDER_GRAYSCALE8        8-bit grayscale
 *      TS_RENDER_GRAYSCALE2        2-bit grayscale
 *      TS_RENDER_BINARY
 *      </TABLE>
 *
 *      If there is no lower precedence renderStyle then the glyph
 *      generation may fail or the font engine glue code may substitute
 *      a lower precedence operation.
 *
 *  Version:
 *      Prior to Version 3.0, this attribute was given as a single value
 *      from an enumerated list. Version 3.0 changed the enum to a bit field
 *      and allowed multiple values to be selected, as described above.
 *      The full advantage of selecting multiple render style bits
 *      requires using iType 4.0 or later.
 *
 *      Several of the former enumerated render styles have been deprecated
 *      since they these styles are not properly supported by WTLE and/or
 *      support for this feature has been removed from iType. These symbols
 *      have been defined with backward compatibility macros.
 *
 *      <TABLE>
 *      <B>Render Style                 <B>Maps to
 *      TS_RENDER_PHASED                TS_RENDER_GRAYSCALE
 *      TS_RENDER_SUBPIXEL_BINARY       TS_RENDER_BINARY
 *      TS_RENDER_SUBPIXEL_GRAYSCALE    TS_RENDER_GRAYSCALE
 *      TS_RENDER_COLORED_ICON          TS_RENDER_RASTER_ICON
 *      </TABLE>
 *
 *  <GROUP textattrib>
 */
typedef TsUInt16 TsRenderStyle;


/*****************************************************************************
 *
 *  A structure that describes font style parameters.
 *
 *  Description:
 *
 *      This structure is used to create TsFontStyle objects;
 *      it is a parameter to the function TsFont_createFontStyle().
 *
 *  <GROUP fontparam>
 */
typedef struct TsFontParam_
{
                                /* description  (default value) */

    TsFixed size;               /* height in ppem (20.0 ppem) */
    TsFixed width;              /* width in ppem; (0 - treated as equal to size) */
    TsFixed skew;               /* skew angle - degrees clockwise from vertical (0.0) */
    TsFixed orientation;        /* glyph rotation - degrees counter clockwise */

    TsFixed bold_pct;           /* boldness value (0) */
    TsFixed stroke_pct;         /* stroke width (0 - treated as default value) */
    TsFixed edgeThickness;      /* edge thickness in pixels (0 - treated as default, which is 1.0 pixels */
    TsFixed outlineOpacity;     /* opacity of outlines, used primarily with glow effect (TS_FIXED_ONE) */
    TsFixed shadowLength_x;     /* x component of the shadow offset vector in pixels, can be +, -, or 0 (0) */
    TsFixed shadowLength_y;     /* y component of the shadow offset vector in pixels, can be +, -, or 0 (0) */

    TsEmphasisStyle emphasis;   /* emphasis such as bold/italics (TS_EMPHASIS_NORMAL) */
    TsBaselineStyle baseline;   /* text baseline style (TS_BASELINE_NORMAL) */
    TsEdgeStyle edge;           /* text edge style (TS_EDGE_NORMAL) */
    TsRenderStyle render;       /* rendering style (TS_RENDER_DEFAULT) */
    TsRenderFlags renderFlags;  /* controls font-engine specific behavior */
} TsFontParam;


/*****************************************************************************
 *
 *  Sets up default values in a TsFontParam object.
 *
 *  Parameters:
 *      param       - [in] TsFontParam object pointer
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontparam>
 */
TS_EXPORT(void)
TsFontParam_defaults(TsFontParam *param);



struct TsFontStyle_;

/*****************************************************************************
 *
 *  The font style object.
 *
 *  Description:
 *      This object provides an interface to a font style which primarily
 *      provides TsGlyph objects.  The TsFontStyle class is an abstract class
 *      using the C++ definition, and must be derived from for concrete
 *      font style services.
 *
 *      The TsFontStyle "is a" TsFont, in essence, containing a pointer
 *      to the TsFont from which a TsFontStyle was created.
 *
 *  <GROUP fontstyle>
 */
typedef struct TsFontStyle_ TsFontStyle;


/*****************************************************************************
 *
 *  The font style virtual function table.
 *
 *  Description:
 *      This structure provides the "virtual function" pointers that are
 *      necessary for the "abstract class" TsFontStyle to provide font style
 *      services.  This structure must be properly initialized and passed
 *      to the TsFontStyle constructor via TsFontStyle_new.
 *
 *      The getBoundingBox function should return the scaled bounding box for the
 *      whole font.
 *
 *      The getGlyphAdvance function pointer can be set to NULL if no
 *      implementation is available.
 *
 *  <GROUP fontstyle>
 */
typedef struct TsFontStyleFuncs_
{
    void (*deleteIt)(void *data); /* delete the derived class */
    TsResult (*mapChar)(void *data, TsInt32 charID, TsInt32 *glyphID); /* map character to glyph index */
    TsResult (*getGlyphPts)(void *data, TsUInt16, TsInt16, TsUInt16 *, TsFixed26_6 *, TsFixed26_6 *); /* get the glyph outline points */
    TsGlyph *(*getGlyph)(void *data, TsInt32 glyphID, TsGlyph *glyph, TsGlyphLayer layers); /* get a glyph */
    TsBool  (*getBoolMetric) (void *data, TsFontMetric metric); /* get a fontStyle metric that has boolean type */
    TsFixed (*getFixedMetric) (void *data, TsFontMetric metric); /* get a fontStyle metric that has fixed-point integer type */
    TsInt32 (*getInt32Metric) (void *data, TsFontMetric metric); /* get a fontStyle metric that has 32-bit integer type */
    TsResult (*getKerning) (void *data, TsInt32 glyphID_1, TsInt32 glyphID_2, TsFixed *dx, TsFixed *dy); /* get pairwise kerning */
    void (*designUnits2pixels)(TsFixed dupem, TsMatrix *m, TsInt16 xdu, TsInt16 ydu, TsFixed *xp, TsFixed *yp); /* convert from design units to pixels */
    TsResult (*getBoundingBox) (void *data, TsRect *boundingBox); /* get the font's approximate bounding box - dilated for edge effects, pseudo-bold and so forth */
    TsResult (*getGlyphAdvance) (void *data, TsInt32 glyphID, TsPoint *advance); /* get the advance width for a glyph */
    TsResult (*getGlyphMetrics) (void *data, TsInt32 glyphID, TsGlyphMetrics *metrics); /* get the metrics for a glyph */
    TsResult (*getGlyphScale)(void *data, TsUInt16, TsUInt16 *, TsMatrix *, TsFixed *, TsFixed *); /* get the design units and scale matrix for a glyph's font. */
                                                                                                   /* last two arguments return x and y scales from the decomposed matrix */
} TsFontStyleFuncs;


/******************************************************************************
 *
 *  Creates and initializes a new TsFontStyle object.
 *
 *  Parameters:
 *      font        - [in] TsFont pointer of "parent" font
 *      fontParam   - [in] font attributes
 *      funcs       - [in] a properly initialized TsFontStyleFuncs structure.
 *      data        - [in] the derived class' private data.  This void pointer
 *                     will be passed as the first argument to each of the
 *                     functions defined in TsFontStyleFuncs.
 *
 *  Return value:
 *      Pointer to TsFontStyle object, or NULL if error.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFontStyle *)
TsFontStyle_new(TsFont *font, TsFontParam *fontParam,
                const TsFontStyleFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Creates and initializes a new memory managed TsFontStyle object.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      font        - [in] TsFont pointer of "parent" font
 *      fontParam   - [in] font attributes
 *      funcs       - [in] a properly initialized TsFontStyleFuncs structure.
 *      data        - [in] the derived class' private data.  This void pointer
 *                     will be passed as the first argument to each of the
 *                     functions defined in TsFontStyleFuncs.
 *
 *  Return value:
 *      Pointer to TsFontStyle object, or NULL if error.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFontStyle *)
TsFontStyle_newex(TsMemMgr *memMgr, TsFont *font, TsFontParam *fontParam,
                const TsFontStyleFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Deletes a TsFontStyle object.
 *
 *  Parameters:
 *      style       - [in] TsFontStyle pointer
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(void)
TsFontStyle_delete(TsFontStyle *style);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsFontStyle object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsFontStyle_releaseHandle.
 *
 *  Parameters:
 *      fontStyle   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      Pointer to font style object.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFontStyle *)
TsFontStyle_copyHandle(TsFontStyle *fontStyle);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsFontStyle object.
 *
 *  Description:
 *      This function reverses the action of TsFontStyle_copyHandle.
 *
 *  Parameters:
 *      fontStyle   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      void
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(void)
TsFontStyle_releaseHandle(TsFontStyle *fontStyle);


/*****************************************************************************
 *
 *  Compares two TsFontSyle objects for equality.
 *
 *  Description:
 *      This function will compare two TsFontStyle objects and return an
 *      indication as to whether they both refer to the same font style.
 *
 *  Parameters:
 *      s1      - [in] pointer to first TsFontStyle object
 *      s2      - [in] pointer to second TsFontStyle object
 *
 *  Return value:
 *      TsBool value, TRUE if TsFontSpec objects are equal, FALSE otherwise.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsBool)
TsFontStyle_compare(TsFontStyle *s1, TsFontStyle *s2);


/******************************************************************************
 *
 *  Gets the X baseline offset, in fixed-point pixels per em,
 *  of the glyphs being generated with this style.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      TsFixed X baseline offset
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getXbaselineOffset(TsFontStyle *style);


/******************************************************************************
 *
 *  Gets the Y baseline offset, in fixed-point pixels per em,
 *  of the glyphs being generated with this style.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      TsFixed Y baseline offset
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getYbaselineOffset(TsFontStyle *style);

#ifndef TS_NO_OPENTYPE

/******************************************************************************
 *
 *  Finds and returns the OpenType table layout object associated with
 *  the script and language system, if present, for the TsFontStyle.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      script  - [in] script
 *      langSys - [in] language system
 *
 *  Return value:
 *      TsOtLayout pointer
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsOtLayout *)
TsFontStyle_findOtLayout(TsFontStyle *style, TsTag script, TsTag langSys);


/******************************************************************************
 *
 *  Use OpenType tables that are available to shape some text.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      word    - [in/out] pointer to TsWordBlock of text to be shaped
 *      layout  - [in] OpenType layout object as returned from TsFontStyle_findOtLayout
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_otShape(TsFontStyle *style, TsOtWordBlock *word, TsOtLayout *layout);

#endif /* TS_NO_OPENTYPE */

/******************************************************************************
 *
 *  Returns indication as to whether this TsFontStyle object handles
 *  rendering of colored icons.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      TsBool, TRUE is TsFontStyle renders colored icons, FALSE otherwise.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsBool)
TsFontStyle_isToRenderColorIcons(TsFontStyle *style);


/******************************************************************************
 *
 *  Returns a boolean metric that is derived from the font style.
 *
 *  Description:
 *      The metric argument may be:
 *
 *      <TABLE>
 *      <B>metric value                             <B>Returns TRUE if
 *      TS_FONT_DO_UNDERLINE                        display routine should draw underline
 *      TS_FONT_DO_STRIKEOUT                        display routine should draw strikeout
 *      TS_FONT_DO_OVERLINE                         display routine should draw overline
 *      TS_FONT_IS_SUPERSCRIPT                      font style is superscripted
 *      TS_FONT_IS_SUBSCRIPT                        font style is subscripted
 *      TS_FONT_DRAW_EDGE_LAYER                     font style has an edge layer
 *      TS_FONT_DRAW_EDGE_LAYER_AFTER_TEXT          font style indicates that the edge layer should be drawn after the text layer
 *      TS_FONT_IS_COMPLEX_TO_DISPLAY               font style has some complexity (edge/superscript/subscript/underline/strikeout/overline)
 *      TS_FONT_IS_BOLD                             font style is emboldened
 *      TS_FONT_IS_ITALIC                           font style is italic
 *      TS_FONT_IS_PAIRWISE_KERNING_SUPPORTED       font and font engine support pairwise kerning
 *      TS_FONT_IS_TO_RENDER_COLOR_ICONS            font style may return colored icons
 *      TS_FONT_IS_FIXED_PITCH                      font is fixed pitch
 *      TS_FONT_IS_THAI_PRIVATE_USE_AREA_POPULATED  font has glyphs in the Thai PUA
 *      </TABLE>
 *
 *      This function is implemented by the derived class. The default value for
 *      unsupported metrics is FALSE.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      metric  - [in] type of metric being requested
 *
 *  Return value:
 *      Metric value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsBool)
TsFontStyle_getBoolMetric(TsFontStyle *style, TsFontMetric metric);


/******************************************************************************
 *
 *  Returns a 16.16 fixed-point metric that is derived from the font style.
 *
 *  Description:
 *      The metric argument may be:
 *
 *      <TABLE>
 *      <B>metric value                 <B>Return value
 *      TS_FONT_X_BASELINE_OFFSET       offset due to superscript/subscript in x-dir [fractional pixels]
 *      TS_FONT_Y_BASELINE_OFFSET       offset due to superscript/subscript in x-dir [fractional pixels]
 *      TS_FONT_ASCENDER                ascender [fractional pixels]
 *      TS_FONT_DESCENDER               descender [fractional pixels]
 *      TS_FONT_LEADING                 leading [fractional pixels]
 *      TS_FONT_X_SIZE                  font size in x-dir [ppem]
 *      TS_FONT_Y_SIZE                  font size in y-dir [ppem]
 *      TS_FONT_ITALIC_ANGLE            italic angle [degrees clockwise from vertical]
 *      TS_FONT_ORIENTATION             glyph rotation angle [degrees ccw from horizontal]
 *      </TABLE>
 *
 *      This function is implemented by the derived class. The default value for
 *      unsupported metrics is 0.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      metric  - [in] type of metric being requested
 *
 *  Return value:
 *      Metric value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getFixedMetric(TsFontStyle *style, TsFontMetric metric);


/******************************************************************************
 *
 *  Returns an integer metric that is derived from the font style.
 *
 *  Description:
 *      The metric argument may be:
 *
 *      <TABLE>
 *      <B>metric value                 <B>Return value [integer pixels]
 *      TS_FONT_STRIKEOUT_THICKNESS     thickness of strikeout line
 *      TS_FONT_STRIKEOUT_POSITION      position of strikeout relative to baseline
 *      TS_FONT_UNDERLINE_THICKNESS     thickness of underline
 *      TS_FONT_UNDERLINE_POSITION      position of underline relative to baseline
 *      TS_FONT_OVERLINE_THICKNESS      thickness of overline
 *      TS_FONT_OVERLINE_POSITION       position of overline relative to baseline
 *      TS_FONT_CURSOR_ASCENDER         recommended cursor ascender
 *      TS_FONT_CURSOR_DESCENDER        recommended cursor descender
 *
 *      This function is implemented by the derived class. The default value for
 *      unsupported metrics is 0.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      metric  - [in] type of metric being requested
 *
 *  Return value:
 *      Metric value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsInt32)
TsFontStyle_getInt32Metric(TsFontStyle *style, TsFontMetric metric);


/******************************************************************************
 *
 *  Maps a character through the font cmap to produce a glyphID.
 *
 *  Description:
 *      This function is similar to TsFont_mapChar, except it accounts
 *      for mapping that may be a function of style and handles errors:
 *
 *      If charID is equal to TsGlyphID_NOP the glyphID is returned as
 *      TsGlyphID_NOP.
 *
 *      If the result is not TS_OK then the glyphID is returned as
 *      TsGlyphID_MissingGlyph.
 *
 *  Parameters:
 *      style   - [in] handle to this font style object
 *      charID  - [in] character value.
 *      glyphID - [out] function returns the glyphID here.
 *
 *  Return value:
 *      TS_OK on success, else error code.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_mapChar(TsFontStyle *style, TsInt32 charID, TsInt32 *glyphID);


/******************************************************************************
 *
 *  Gets a TsGlyph associated with a fontStyle and a glyph index value.
 *
 *  Description:
 *      Gets the glyph at 'index' in the font style represented by fontStyle.
 *      The index value must have been obtained using the
 *      TsFontStyle_mapChar method, or, if not a colored icon using the
 *      TsFont_mapChar method of the TsFont that is the "parent" of
 *      the TsFontStyle object
 *
 *  Parameters:
 *      style     - [in] pointer to TsFontStyle object
 *      glyphID   - [in] glyphID within font (TrueType calls this glyph index)
 *
 *  Return value:
 *      Pointer to TsGlyph object or NULL if error.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsGlyph *)
TsFontStyle_getGlyph(TsFontStyle *style, TsInt32 glyphID);


/******************************************************************************
 *
 *  Gets a TsGlyph associated with a fontStyle and a glyph index value.
 *
 *  Description:
 *      Gets the glyph at 'index' in the font style represented by fontStyle.
 *      The index value must have been obtained using the
 *      TsFontStyle_mapChar method, or, if not a colored icon using the
 *      TsFont_mapChar method of the TsFont that is the "parent" of
 *      the TsFontStyle object
 *
 *  Parameters:
 *      style     - [in] pointer to TsFontStyle object
 *      glyphID   - [in] glyphID within font (TrueType calls this glyph index)
 *      layers    - [in] bitfield that specifies which glyph layers will comprise glyph
 *
 *  Return value:
 *      Pointer to TsGlyph object or NULL if error.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsGlyph *)
TsFontStyle_getGlyphLayers(TsFontStyle *style, TsInt32 glyphID, TsGlyphLayer layers);


/******************************************************************************
 *
 *  Returns the glyph metrics of a particular glyph.
 *
 *  Description:
 *      Glyph metrics are returned within the 'metrics' structure.
 *
 *      If the metrics can't be obtained then the metrics structure is
 *      initialized to 0 and the function returns FALSE.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object
 *      glyphID     - [in] glyphID within font (TrueType calls this glyph index)
 *      metrics     - [out] pointer to metrics structure.
 *
 *  Return value:
 *      Result value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_getGlyphMetrics(TsFontStyle *style, TsInt32 glyphID, TsGlyphMetrics *metrics);


/******************************************************************************
 *
 *  Returns the scaled font bounding box in pixels.
 *
 *  Description:
 *      This function returns the maximum bounding box for all of the
 *      glyphs in the font, scaled to the size of this font style.
 *      The bounding box includes any adjustments needed for hinting,
 *      pseudo-italics, pseudo-bold, and edge effects.
 *
 *      The bounding box may calculation may require estimates for
 *      the various effects. This bounding box is intended for use
 *      when the exact bounding box is not required, but when a maximum
 *      bounding box would be useful.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object
 *      boundingBox - [out] bounding box in integer pixels
 *
 *  Return value:
 *      Result value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_getBoundingBox(TsFontStyle *style, TsRect *boundingBox);


/******************************************************************************
 *
 *  Returns advance of a glyph in 16.16 pixels.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object
 *      glyphID     - [in] glyphID within font (TrueType calls this glyph index)
 *      advance     - [out] advance value in 16.16. pixels
 *
 *  Return value:
 *      Result value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_getGlyphAdvance(TsFontStyle *style, TsInt32 glyphID, TsPoint *advance);


/******************************************************************************
 *
 *  Release a TsGlyph associated with a fontStyle.
 *
 *  Description:
 *      If the glyph was obtained from TsFontStyle_getGlyph then you must
 *      call this function to return it to the fontStyle. The glyph
 *      will either be deleted or returned to the cache.
 *
 *  Parameters:
 *      style     - [in] pointer to TsFontStyle object
 *      glyph     - [in] pointer to glyph object.
 *
 *  Return value:
 *      none
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(void)
TsFontStyle_releaseGlyph(TsFontStyle *style, TsGlyph *glyph);


/******************************************************************************
 *
 *  Converts x and y values from font design units to pixels.
 *
 *  Description:
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      du      - [in] design units per em
 *      m       - [in] scale matrix
 *      xdu     - [in] x value in design units
 *      ydu     - [in] y value in design units
 *      xp      - [out] equivalent x value in fixed point pixels
 *      yp      - [out] equivalent y value in fixed point pixels
 *
 *  Return value:
 *      none
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(void)
TsFontStyle_designUnits2pixels(TsFontStyle *style, TsFixed du, TsMatrix *m, TsInt16 xdu, TsInt16 ydu, TsFixed *xp, TsFixed *yp);

/******************************************************************************
 *
 *  Gets the true x-dimension size, in fixed-point pixels per em,
 *  of the glyphs being generated with this style.
 *
 *  Description:
 *      The size attributes that are passed in to a font engine are
 *      indicate requested size of a full size glyph.  The true
 *      size may be different due to superscript/subscript settings,
 *      width adjustment, or size substitution, if the glyphs in the
 *      font are not scalable.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      TsFixed X size
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getXSize(TsFontStyle *style);

/******************************************************************************
 *
 *  Gets the true y-dimension size, in fixed-point pixels per em,
 *  of the glyphs being generated with this style.
 *
 *  Description:
 *      The size attributes that are passed in to a font engine are
 *      indicate requested size of a full size glyph.  The true
 *      size may be different due to superscript/subscript settings,
 *      width adjustment, or size substitution, if the glyphs in the
 *      font are not scalable.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *
 *  Return value:
 *      TsFixed Y size
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getYSize(TsFontStyle *style);

/*****************************************************************************
 *
 *  Returns the scaled font ascender value in fixed point pixel units.
 *
 *  Parameters:
 *      fStyle      - [in] pointer to TsFontStyle object.
 *
 *  Return value:
 *      TsFixed ascender value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getAscender(TsFontStyle *fStyle);


/*****************************************************************************
 *
 *  Returns the scaled font descender value in fixed point pixel units.
 *
 *  Parameters:
 *      fStyle      - [in] pointer to TsFontStyle object.
 *
 *  Return value:
 *      TsFixed descender value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getDescender(TsFontStyle *fStyle);


/*****************************************************************************
 *
 *  Returns the scaled font leading value in fixed point pixel units.
 *
 *  Parameters:
 *      style      - [in] pointer to TsFontStyle object.
 *
 *  Return value:
 *      TsFixed leading value
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFixed)
TsFontStyle_getLeading(TsFontStyle *style);


/*****************************************************************************
 *
 *  Returns a pairwise kerning adjustment for a glyph pair.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object.
 *      glyphID_1   - [in] glyphID of first glyph in pair
 *      glyphID_2   - [in] glyphID of second glyph in pair
 *      dx          - [out] returns the x-dimension kerning value
 *      dy          - [out] returns the y-dimension kerning value
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_getKerning(TsFontStyle *style, TsInt32 glyphID_1, TsInt32 glyphID_2, TsFixed *dx, TsFixed *dy);


/*****************************************************************************
 *
 *  Returns a reference to the TsFont used to construct this font style.
 *
 *  Notes:
 *      The TsFont object is reference counted. You will need to call
 *      TsFont_releaseHandle on the pointer that is returned, once
 *      you are finished with it.
 *
 *  Parameters:
 *      style      - [in] pointer to TsFontStyle object.
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsFont *)
TsFontStyle_getFont(TsFontStyle *style);

/******************************************************************************
 *
 *  Gets a pointer to a memory-mapped table.
 *
 *  Description:
 *      The table obtained using this function must be released using
 *      TsFontStyle_releaseTable. This must be done before releasing the handle
 *      to the fontstyle.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      tag     - [in] 4-byte table tag
 *
 *  Return value:
 *      Pointer to table.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(void *)
TsFontStyle_getTable(TsFontStyle *style, TsTag tag);


/******************************************************************************
 *
 *  Releases a pointer to a memory-mapped table.
 *
 *  Parameters:
 *      style   - [in] pointer to TsFontStyle object
 *      table   - [in] pointer to table
 *
 *  Return value:
 *      TS_OK if released OK.  TS_ERR if table could not be found or released.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_releaseTable(TsFontStyle *style, void *table);


/*****************************************************************************
 *
 *  Get the original font params used to construct this font style.
 *
 *  Description:
 *      This function fills the fontParam structure with the values
 *      that were used to construct this font style.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object.
 *      fontParam   - [out] pointer to font parameters structure
 *
 *  Return value:
 *      none
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(void)
TsFontStyle_getFontParams(TsFontStyle *style, TsFontParam *fontParam);


/*****************************************************************************
 *
 *  Retrieves the cached X and Y coordinate values of the point at index
 *  "pointIndex" for the glyph at index "glyphID".  The point is scaled
 *  and styled per the TsFontStyle provided.  The coordinates returned are
 *  in 26.6 fixed point pixels.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object.
 *      glyphID     - [in] index to glyph in font referenced in style.
 *      pointIndex  - [in] index to original design point in glyph outline.
 *      x           - [out] pointer to TsFixed26_6 in 26.6 fixed point pixels.
 *      y           - [out] pointer to TsFixed26_6 in 26.6 fixed point pixels.
 *
 *  Return value:
 *      TsBool      - TRUE if point was available in cache, FALSE otherwise.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsBool)
TsFontStyle_getGPOSpoint(TsFontStyle *style, TsUInt16 glyphID,
                         TsUInt16 pointIndex, TsFixed26_6 *x, TsFixed26_6 *y);

/*****************************************************************************
 *
 *  Retrieves design units, scaling matrix, and X and Y "user" scale values
 *  for the styled font to which the specified glyph at "glyphID" belongs.
 *  The scaling matrix and the x and y "user" scale values (resulting from the
 *  decomposition of the matrix) are returned in 16.16 fixed point format.
 *
 *  Parameters:
 *      style       - [in] pointer to TsFontStyle object.
 *      glyphID     - [in] index to glyph in font referenced in style.
 *      du          - [out] design units
 *      m           - [out] scaling matrix
 *      xppm        - [out] x "user" scale value
 *      yppm        - [out] y "user" scale value
 *
 *  Return Value:
 *      TsBool      - TRUE if successful, FALSE otherwise.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsBool)
TsFontStyle_getGPOSscale(TsFontStyle *style, TsUInt16 glyphID,
                         TsUInt16 *du, TsMatrix *m, TsFixed *xppm, TsFixed *yppm);

/*****************************************************************************
 *
 *  Adds the X and Y coordinate values of the points indexed in the point
 *  index array "pointIndices" to the internal point cache structure.
 *  The cached points may be retrieved using the above-mentioned method
 *  TsFontStyle_getGPOSpoint().  The points are original design points on
 *  the glyph outline of glyph at index "glyphID".  The points are scaled
 *  and styled per the TsFontStyle provided.  The number of points and length
 *  of the point index array is specified with the argument "numPts".
 *
 *  Parameters:
 *      style        - [in] pointer to TsFontStyle object.
 *      glyphID   - [in] index to glyph in font referenced in style.
 *      numPts       - [in] number of point indices provided.
 *      pointIndices - [in] indices of original design points on glyph outline.
 *
 *  Return value:
 *      TsResult     - TS_ERR if cache cannot be allocated, the result of
 *                     the TsFontStyleFuncs call getGlyphPts(), otherwise.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsResult)
TsFontStyle_addGPOSpoints(TsFontStyle *style, TsUInt16 glyphID,
                          TsInt16 numPts, TsUInt16 *pointIndices);


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
 *      becomes invalid when this TsFontStyle object is destroyed.
 *      Therefore, you must keep reference to the TsFontStyle object
 *      until the TsExternalUserData object is no longer needed.
 *
 *  Parameters:
 *      style       - [in] this
 *
 *  Return value:
 *      pointer to the TsExternalUserData object if a successful call was
 *      previously made to TsExternalUserData_init, NULL otherwise.
 *
 *  <GROUP fontstyle>
 */
TS_EXPORT(TsExternalUserData *)
TsFontStyle_getExternalUserdata(TsFontStyle *style);

#ifdef TS_USE_OPENTYPE_CACHE
/*****************************************************************************
 *
 *  Returns a pointer to an object which contains OpenType cache data.
 *
 *  Parameters:
 *      style       - [in] this
 *
 *  Return value:
 *      Pointer to the TsOtCache object if a successful, NULL otherwise.
 *
 *  <GROUP fontstyle>
 */

TS_EXPORT(TsOtCache *)
TsFontStyle_getOpenTypeCache(TsFontStyle *style);
#endif /* TS_USE_OPENTYPE_CACHE */


/***** Deprecated ************************************************************/

/* Use new TsRenderStyle types rather than these types */
#define TS_RENDER_PHASED                TS_RENDER_GRAYSCALE
#define TS_RENDER_SUBPIXEL_BINARY       TS_RENDER_BINARY
#define TS_RENDER_SUBPIXEL_GRAYSCALE    TS_RENDER_GRAYSCALE
#define TS_RENDER_COLORED_ICON          TS_RENDER_RASTER_ICON


TS_END_HEADER

#endif /* TSFONTST_H */
