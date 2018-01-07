
/* Copyright (C) 2001-2012 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* Encoding:   US_ASCII    Tab Size:   8   Indentation:    4  */

/* fs_function.h */

#ifndef FS_FUNCTION_H
#define FS_FUNCTION_H

#include "fs_fnt.h"
#include "fs_glue.h"
#include "fs_ltt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef NULL
#define NULL 0
#endif

#if defined(FS_BITMAPS) || defined(FS_GRAYMAPS) || defined(FS_EDGE_RENDER)
#define FS_RENDER      /* defines code that is common all renderers */
#endif

#if defined(FS_EDGE_TECH)
#error Use FS_EDGE_HINTS and/or FS_EDGE_RENDER instead of FS_EDGE_TECH
#endif
#if defined(FS_EDGE_HINTS) && !defined(FS_HINTS)
#error FS_EDGE_HINTS requires FS_HINTS be defined
#endif
#if defined(FS_EDGE_RENDER) || defined(FS_EDGE_HINTS)
#define FS_EDGE_TECH   /* defines code that is common to both rendering and hinting */
#endif

#if defined(FS_CACHE_BITMAPS) || defined(FS_CACHE_GRAYMAPS)    || defined(FS_CACHE_OUTLINES) || \
    defined(FS_CACHE_PHASED)  || defined(FS_CACHE_EDGE_GLYPHS) || defined(FS_CACHE_ADVANCE)  || \
    defined(FS_CACHE_OPENVG)
#define FS_CACHE
#endif

/* Build-time check of configuration dependencies */
#if defined(FS_CONTOUR_WINDING_DETECTION) && !defined(FS_BITMAPS)
#error FS_CONTOUR_WINDING_DETECTION requires FS_BITMAPS be defined
#endif

#if defined(FS_PHASED) && !defined(FS_BITMAPS)
#error Phased Graymap(FS_PHASED) requires FS_BITMAPS be defined
#endif

#if defined(FS_MULTI_PROCESS) && !defined(FS_INT_MEM)
#error Multiprocess (FS_MULTI_PROCESS) requires FS_INT_MEM be defined
#endif

#if defined(FS_MULTI_PROCESS) && defined(FS_CFFR)
#error Multiprocess (FS_MULTI_PROCESS) is not compatible with FS_CFFR
#endif

/* "mode" values for FS_get_table() */
#define TBL_QUERY        (FS_BYTE)1
#define TBL_EXTRACT      (FS_BYTE)2

/* mask values */
extern FS_CONST FS_BYTE fs_mask[8];

/*************************************************************************/
/*** the API calls ... application level calls, static link or library ***/
/*************************************************************************/

/* Debug function definitions */
#ifdef FS_DUMP
#ifdef FS_MULTI_PROCESS
ITYPE_API FS_VOID dump_bitmap(_DS_ FS_BITMAP *p);
ITYPE_API FS_VOID dump_graymap(_DS_ FS_GRAYMAP *gmap);
ITYPE_API FS_VOID dump_outline(_DS_ FS_OUTLINE *outl);
#else
/*****************************************************************************
 *
 *  Prints contents of FS_BITMAP including metrics and an ASCII 
 *  representation of glyph.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      p            - [in] reference to a bitmap glyph
 *
 *  Return Value:
 *      None
 *
 *  <GROUP fsdump>
 */
ITYPE_API FS_VOID dump_bitmap(FS_BITMAP *p);


/*****************************************************************************
 *
 *  Prints contents of FS_GRAYMAP including metrics and an ASCII
 *  representation of glyph.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      gmap        - [in] reference to a graymap glyph
 *
 *  Return Value:
 *      None
 *
 *  <GROUP fsdump>
 */
ITYPE_API FS_VOID dump_graymap(FS_GRAYMAP *gmap);


#ifdef FS_ICONS
ITYPE_API FS_VOID dump_icon(FS_GLYPHMAP *icon);
#endif

/*****************************************************************************
 *
 *  Prints contents of FS_OUTLINE as a list of curve elements and associated
 *  coordinates.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      outl        - [in] reference to an outline glyph
 *
 *  Return Value:
 *      None
 *
 *  <GROUP fsdump>
 */
ITYPE_API FS_VOID dump_outline(FS_OUTLINE *outl);
#endif /* not FS_MULTI_PROCESS */

FS_VOID internal_dump_outline(_DS_ FS_OUTLINE *outl);

#endif /* FS_DUMP */



/* Data sources returned by FS_get_ascender_descender_leading function
   <TABLE>
   FsAscDescLeadSource_UNKNOWN
   FsAscDescLeadSource_WIN_MAC_COMPAT
   FsAscDescLeadSource_TTF_HHEA
   FsAscDescLeadSource_MS_COMPAT
   FsAscDescLeadSource_FONTSET_MIXED
   FsAscDescLeadSource_VDMX_TABLE
   </TABLE>
   <GROUP itypeenums> 
*/
typedef enum {
    FsAscDescLeadSource_UNKNOWN,
    FsAscDescLeadSource_WIN_MAC_COMPAT,
    FsAscDescLeadSource_TTF_HHEA,
    FsAscDescLeadSource_MS_COMPAT,
    FsAscDescLeadSource_FONTSET_MIXED,
    FsAscDescLeadSource_VDMX_TABLE,

    FsAscDescLeadSource_IGNORE_VDMX_TABLE = 329091     /* not returned - a special case used
                                                        * as an input parameter to ignore
                                                        * VDMX table as possible data source */
} FsAscDescLeadSource;

/* API function definitions that are the same for both single-threaded       */
/* and multi-threaded usage. These functions do not require mutex protection */

/*****************************************************************************
 *
 *  Returns the last error code encountered by an iType function.
 *
 *  Description:
 *      This function is useful in conjunction with functions that
 *      do not return an error code.
 *
 *      For example, if FS_get_bitmap() returns NULL rather than a valid pointer to
 *      an FS_BITMAP structure, this indicates an error.  The client
 *      would then call FS_error() to get the error.
 *
 *      Each FS_STATE structure stores its own last error, so the last
 *      error refers only to calls made using this FS_STATE structure.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *
 *  Return value
 *      Error code
 *      
 *  <GROUP admin>
 */
ITYPE_API FS_LONG FS_error(_DS0_);


/*****************************************************************************
 *
 *  Sets or clears a flag.
 *
 *  Description:
 *      This function is used to set certain flags that affect how the
 *      various FS_get_<I>glyph</I> functions work and how glyphs are made.
 *      Each call to FS_set_flags() sets one flag.  There are three types 
 *      of flags: (1) special effects flags, (2) emboldening flags, and 
 *      (3) on/off control flag pairs. 
 *
 *      The special effects flags are mutually exclusive, as only one special effect
 *      is applied at a time.  The default setting is FLAGS_NO_EFFECT, a normal
 *      glyph with no special effect.  Once an effect is set it remains in effect 
 *      until FS_set_flag() is called with a different effect.  
 *      The special effects are:
 *      <TABLE>
 *      Flag                        Effect on glyph 
 *      ---------------------       ----------------------------
 *      FLAGS_NO_EFFECT (default)   Normal glyph                    
 *      FLAGS_EMBOSSED              Appears raised
 *      FLAGS_ENGRAVED              Appears depressed or etched
 *      FLAGS_OUTLINED              Appears as unfilled outlines (1 pixel wide)
 *      FLAGS_OUTLINED_1PIXEL       Appears as unfilled outlines (1 pixel wide)
 *      FLAGS_OUTLINED_2PIXEL       Appears as unfilled outlines (2 pixels wide)
 *      FLAGS_OUTLINED_UNFILLED     Appears as unfilled outlines (N pixels wide)
 *      FLAGS_OUTLINED_FILLED       Appears as filled outlines   (N pixels wide)
 *      </TABLE>
 *
 *      The outline width N is controlled using FS_set_outline_width().
 *    
 *      Glyphs made with the embossed, engraved, or outlined effects   
 *      appear as an outline around/near 
 *      the edges set by the normal glyph. These effects work for
 *      both bitmaps and graymaps as well as Edge graymaps. 
 *
 *      For example, to set the edge effect to embossed:
 *      <CODE>
 *      FS_set_flags(state, FLAGS_EMBOSSED);
 *      </CODE>
 *
 *      The emboldening flags control which form of pseudo-emboldening is
 *      applied whenever the bold percentage is set to a non-zero value.
 *      See FS_set_bold_pct() and discussion in the User Guide.
 *      By default, regular outline emboldening is used for TrueType outline
 *      fonts and for regular stroke fonts. CJK emboldening is used for
 *      SmartHinted stroke fonts at 26 ppem and below. The emboldening flags
 *      allow explicit control over which type of emboldening is used. 
 *      The emboldening options are:
 *      <TABLE>
 *      Flag                        Effect on glyph 
 *      ---------------------       ----------------------------
 *      FLAGS_REGULARBOLD           Regular outline emboldening is used
 *      FLAGS_REGULARBOLD_ON        Same as FLAGS_REGULARBOLD
 *      FLAGS_REGULARBOLD_OFF       Default emboldening is used
 *      FLAGS_FRACTBOLD             Fractional outline emboldening is used
 *      FLAGS_FRACTBOLD_ON          Same as FLAGS_FRACTBOLD
 *      FLAGS_FRACTBOLD_OFF         Default emboldening is used
 *      FLAGS_ADD_WEIGHT            CJK emboldening is used
 *      FLAGS_ADD_WEIGHT_ON         Same as FLAGS_ADD_WEIGHT
 *      FLAGS_ADD_WEIGHT_OFF        Default emboldening is used
 *      </TABLE>
 *
 *      It is recommended that you use the on/off versions of these flags. Emboldening
 *      flags are off by default.
 *
 *      The on/off feature control flag pairs each control a certain aspect of glyph
 *      generation.  Flags are initialized to their defaults. After that
 *      the current setting is the last one set; a flag remains set until 
 *      a call to FS_set_flags() turns it on or off.  FS_set_flags() can be 
 *      called at any time.  The flags pairs are:
 *      <TABLE>
 *      Flag                                Default              
 *      ---------------------               ---------
 *      FLAGS_CHECK_CONTOUR_WINDING_ON     
 *      FLAGS_CHECK_CONTOUR_WINDING_OFF     X
 *      FLAGS_CMAP_ON                       X
 *      FLAGS_CMAP_OFF
 *      FLAGS_DROPOUTS_ON              
 *      FLAGS_DROPOUTS_OFF                  X
 *      FLAGS_HINTS_ON                      X
 *      FLAGS_HINTS_OFF
 *      FLAGS_AUTOHINT_ON                   X
 *      FLAGS_AUTOHINT_OFF
 *      FLAGS_AUTOHINT_YONLY_ON
 *      FLAGS_AUTOHINT_YONLY_OFF            X
 *      FLAGS_FORCE_AUTOHINT
 *      FLAGS_FORCE_AUTOHINT_OFF            X
 *      FLAGS_GRAYSCALE_ON                 
 *      FLAGS_GRAYSCALE_OFF                 X
 *      FLAGS_SOFTENED_ON
 *      FLAGS_SOFTENED_OFF                  X
 *      FLAGS_VERTICAL_ON               
 *      FLAGS_VERTICAL_OFF                  X
 *      FLAGS_VERTICAL_ROTATE_RIGHT_ON               
 *      FLAGS_VERTICAL_ROTATE_RIGHT_OFF     X
 *      FLAGS_VERTICAL_ROTATE_LEFT_ON               
 *      FLAGS_VERTICAL_ROTATE_LEFT_OFF      X
 *      FLAGS_DEFAULT_CSM_ON                 
 *      FLAGS_DEFAULT_CSM_OFF               X
 *      </TABLE>
 *
 *      The CHECK_CONTOUR_WINDING flag pair controls whether the 
 *      FS_get_<I>glyph</I> functions perform a check for invalid contour
 *      windings.  See the User Guide for a discussion of this issue.
 *
 *      The CMAP flag pair controls whether the <I>id</I> argument to the
 *      various FS_get_<I>glyph</I> functions is treated as a character code
 *      (FLAGS_CMAP_ON), or a glyph index (FLAGS_CMAP_OFF).  Similarly, it
 *      affects the interpretation of the <I>id</I> arguments to 
 *      FS_get_kerning().  By default, the flag is on.  This flag
 *      does not affect the FS_map_char function.  
 *      Turn off the flag when you are working with glyph indices
 *      directly.  For example, you may wish to first call FS_map_char 
 *      then later call FS_get_<I>glyph</I>.  This is needed when implementing
 *      glyph subsitution methods.
 *  
 *      The DROPOUTS flag pair controls whether to apply extra dropout
 *      protection.  Normally (FLAGS_DROPOUTS_OFF) dropouts are processed
 *      as specified by the font or by an individual character.  To force the
 *      rasterizer to use extra dropout protection, despite what a font or
 *      character specifies, set FLAGS_DROPOUTS_ON.
 *
 *      The HINTS flag pair controls whether hinting is enabled when 
 *      generating characters.  Normally, hinting is enabled 
 *      (FLAGS_HINTING_ON).  To disable hinting set FLAGS_HINTING_OFF.
 *      Disabling hinting may be appropriate in some use cases such as
 *      when displaying moving text.
 *
 *      The AUTOHINT flag pair controls whether autohinting is used when
 *      generating graymaps from unhinted outlines. Normally, this autohinting
 *      is enabled. To disable grayscale autohinting, set FLAGS_AUTOHINT_OFF.
 *      Normally grayscale autohinting only applies to unhinted glyphs.
 *
 *      The AUTOHINT_YONLY flag pair controls whether autohinting applies only
 *      in the Y direction. This is useful when using subpixel rendering. This
 *      flag only applies when autohinting is enabled. The default is to 
 *      autohint in both directions (AUTOHINT_YONLY_OFF).
 *
 *      The FORCE_AUTOHINT pair controls whether grayscale autohinting is
 *      always applied, regardless of whether the glyph is hinted or not.
 *      You can also use set this flag to force autohinting when retrieving
 *      outlines using FS_get_outline().
 *
 *      The GRAYSCALE flag pair controls the stroke width and autohinting applied
 *      during rendering of stroke font glyphs. This flag is set internally
 *      to the appropriate value for bitmaps and graymaps. If you are requesting
 *      an outline using FS_get_outline, you can use this flag to control
 *      the type of outline returned from a stroke font. Grayscale rendering
 *      uses fractional stroke widths and square endcaps whereas bitmap
 *      rendering used integer stroke widths and round endcaps.
 *
 *      The SOFTENED flag pair controls whether special filtering is applied
 *      to graymaps: this filtering serves to soften all edges.  
 *      Normally this effect is off.  Set FLAGS_SOFTENED_ON
 *      to obtain this special effect.  Note that the softness effect
 *      cannot be used with phased graymaps.
 *      a special filtering effect.
 *
 *      Note that the following vertical writing mode flags are mutually exclusive.
 *      Turning one mode on automatically disables the other modes. Likewise,
 *      turning one mode off automatically turns all modes off.
 *
 *      The VERTICAL flag pair controls whether the glyphs returned
 *      by the FS_get_<I>glyph</I> functions are intended for vertical
 *      layout.  The default is horizontal layout (FLAGS_VERTICAL_OFF).
 *      If FLAGS_VERTICAL_ON is set then glyph metrics are defined
 *      according to vertical, top-to-bottom layout.  An exception
 *      is that this flag does not affect glyphs returned by FS_get_icon().
 * 
 *      The VERTICAL_ROTATE_RIGHT flag pair controls whether the glyphs returned
 *      by the FS_get_<I>glyph</I> functions are intended for vertical
 *      layout with the glyphs rotated to the right and centered on the vertical
 *      layout center line. The default is horizontal layout (FLAGS_VERTICAL_ROTATE_RIGHT_OFF).
 *      If FLAGS_VERTICAL_ROTATE_RIGHT_ON is set then glyph metrics are defined
 *      according to vertical, top-to-bottom layout but with the glyph rotated 90
 *      degrees to the right.  An exception is that this flag does not affect glyphs 
 *      returned by FS_get_icon().
 *
 *      The VERTICAL_ROTATE_LEFT flag pair controls whether the glyphs returned
 *      by the FS_get_<I>glyph</I> functions are intended for vertical
 *      layout with the glyphs rotated to the left and centered on the vertical
 *      layout center line. The default is horizontal layout (FLAGS_VERTICAL_ROTATE_LEFT_OFF).
 *      If FLAGS_VERTICAL_ROTATE_LEFT_ON is set then glyph metrics are defined
 *      according to vertical, top-to-bottom layout but with the glyph rotated 90
 *      degrees to the left.  An exception is that this flag does not affect glyphs 
 *      returned by FS_get_icon().
 *
 *      The DEFAULT_CSM flag pair controls whether default CSM values are
 *      used during Edge(TM) rendering. Normally, CSM values are stored
 *      in the font and are used during rendering. If such values are 
 *      not in the font, default CSM values are automatically used. 
 *      If values are in the font but you want to use your defaults
 *      set using FS_set_csm(), then set the flag FLAGS_DEFAULT_CSM_ON.
 *      Use FLAGS_DEFAULT_CSM_OFF to return to using values in the font.
 *
 *      FLAGS_FIAB_ON and FLAGS_FIAB_OFF are deprecated and have no effect
 *      if set. 
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      flag        - [in] flag value
 *
 *  Return Value:
 *      SUCCESS, or error code if input is invalid.
 *
 *  Note:
 *      This function sets one flag at a time.  Do not try to set
 *      multiple flags using a bitwise OR on the flags.  Instead,
 *      make one call for each flag that you wish to set.
 *      
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_flags(_DS_ FS_ULONG flag);


/*****************************************************************************
 *
 *  Retrieves the current set of flags in effect.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *
 *  Return value:
 *      Current flags word (user will need to parse bit values)
 * 
 *    See Also:
 *      FS_set_flags()
 *      
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_get_flags(_DS0_);


/*****************************************************************************
 *
 *  Sets the amount of emboldening to use (default is 0).
 *
 *  Description:
 *      This function sets the amount of emboldening used when generating
 *      glyphs.  The normal, default value is 0. The function is optional;
 *      if not called glyphs have the normal boldness.
 *
 *      The bold value is expressed as a percentage of the em box in
 *      16.16 fixed point units.  The recommended range is 0-20%.
 *      The valid range is 0-100%.  Thinning the glyph below normal is not
 *      supported.
 *
 *      The emboldening effect achieved here is called 'pseudo bold'.  This
 *      is in contrast to a true bold font which is bold by design.
 *      Pseudo bold is used to save font memory, or when a bold design
 *      of a particular font is not available.  This function will
 *      affect both outline and stroke fonts.
 *
 *      This function must be called before FS_set_scale() to have any
 *      effect.  To change the current boldness you must first call
 *      FS_set_bold_pct() then call FS_set_scale(), even if the scaling
 *      parameters have not changed.
 *
 *      If FS_PSEUDO_BOLD is not defined then this function will return ERR_BOLD_UNDEF.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      pct         - [in] boldness value in 16.16 fixed point notation.
 *
 *  Return Value:
 *      SUCCESS, or ERR_BAD_PERCENT if pct is outside valid range (0-100%).
 *
 *  Version:
 *      Prior to Version 2.1 this function had no effect on stroke fonts.
 *
 *  Examples:
 *      To set the bold percentage to 10% (6554 is 10% of 65536):
 *      <CODE>
 *      FS_set_bold_pct(6554);
 *      </CODE>
 *
 *  Version:
 *      Prior to Version 2.4, if FS_PSEUDO_BOLD was not defined this function would still return SUCCESS.
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_bold_pct(_DS_ FS_FIXED pct);


/*****************************************************************************
 *
 *  Sets the stroke width as a percentage of the em square (default is 0).
 *
 *  Description:
 *      This function set the stroke width as a percentage of the em square.
 *      The stroke width only affects stroke fonts.  iType uses the
 *      percent value here to compute the actual stroke width, ensuring
 *      that the minimum stroke width is one pixel.
 *
 *      This value should be adjusted to achieve the desired normal weight
 *      of the font.  To get a bold effect, use this function in combination
 *      with FS_set_bold_pct().
 *
 *      The recommended range is 0-12%.  The valid range is 0-50%.
 *
 *      This function must be called before FS_set_scale() to have any
 *      effect.  To change the current boldness you must first call
 *      FS_set_stroke_pct() then call FS_set_scale(), even if the scaling
 *      parameters have not changed.
 *
 *      If FS_STIK is not defined then this function will return ERR_STIK_UNDEF.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      pct         - [in] stroke width value in 16.16 fixed point notation.
 *
 *  Return Value:
 *      SUCCESS, or error code if input is invalid.
 *
 *  Version:
 *      Prior to Version 2.4, if FS_STIK was not defined this function would still return SUCCESS.
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_stroke_pct(_DS_ FS_FIXED pct);

/*****************************************************************************
 *
 *  Sets the width of N-pixel outline
 *
 *  Description:
 *      This function set the width of N-pixel outline.
 *      If not set, the default width is 1.
 *      Valid values for width are 1-10.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      width       - [in] width of N-pixel
 *
 *  Return Value:
 *      SUCCESS, or error code if input is invalid.
 *
 *  Version:
 *      Introduced in version 3.1
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_outline_width(_DS_ FS_USHORT width);

/*****************************************************************************
 *
 *  Sets the opacity of N-pixel outline.
 *
 *  Description:
 *      This function set the opacity of an N-pixel outline. It has no
 *      effect on regular glyph images, only those with the outline effect.
 *
 *      If not set, the default opacity is 65536 (1.0 in fixed point).
 *      The valid range for opacity is 0 - 655360 (0.0 - 10.0 in fixed point).
 *
 *      Opacity effectively defines the maximum gray level used
 *      for mixing the outline with the background. Note that the opacity value
 *      can exceed 1.0 (limit is 10.0). If the opacity is greater than 1.0
 *      gray values are increased up to the limit imposed by the bit depth 
 *      of the graymap requested. The allows a greater range of effects, 
 *      particularly for the soft outline effect.
 *
 *  Example:
 *      Using 4-bit graymaps, an opacity of 49152 (0.75 in fixed point) will produce 
 *      a maximum gray value of 11 instead of the normal 15. This gives 12 shades 
 *      (including zero) of grayscale mixing with the background. An opacity of 
 *      13107 (0.2 in fixed point)will produce 4 levels of grayscale mixing. 
 *      Values below 4370 (0.06668 in fixed point) will result a glyph image with no 
 *      gray values. Such opacity values are considered valid, but may not be useful.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      opacity     - [in] opacity (16.16 fixed point)
 *
 *  Return Value:
 *      SUCCESS, or error code if input is invalid.
 *
 *  Version:
 *      Introduced in version 3.1 
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_outline_opacity(_DS_ FS_FIXED opacity);


/*****************************************************************************
 *
 *  Sets the default CSM values for Edge(TM) rendering.
 *
 *  Description:
 *      This function sets the default CSM parameters used in Edge(TM) rendering.
 *      These values are used if the font does not contain recommended values
 *      or if the FLAGS_DEFAULT_CSM_ON flag has been set. 
 *      
 *      CSM parameters are in 16.16 fixed point (FS_FIXED).
 *      Inside cutoff values are generally positive.
 *      Outside cutoff values are generally negative.
 *      Specifying a gamma other than 65536 (1.0) may incur a 
 *      performance penalty.
 *
 *  Parameters:
 *      state         - [in] pointer to FS_STATE structure
 *      insideCutOff  - [in] inside cutoff value.
 *      outsideCutOff - [in] outside cutoff value.
 *      gamma         - [in] transition function gamma value.
 *
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  Version:
 *      Introduced in iType 4.1.
 *
 *  Examples:
 *      To set the CSM values to ( 0.56, -0.42, 1.0):
 *      <CODE>
 *      FS_set_csm( 36700, -27525, 65536);
 *      </CODE>
 *
 *  <GROUP statecontrol>
 */
#ifdef FS_EDGE_RENDER
ITYPE_API FS_LONG FS_set_csm(_DS_ FS_FIXED insideCutOff, FS_FIXED outsideCutOff, FS_FIXED gamma);
#endif

/*****************************************************************************
 *
 *  Gets the CSM values used for Edge(TM) rendering the specified glyph index.
 *
 *  Description:
 *      This function gets the current unadjusted CSM parameters used 
 *      in Edge(TM) rendering for a particular glyph at the current 
 *      scale size. Values either come from the ADFH table in the font
 *      or are the default CSM values if the FLAGS_DEFAULT_CSM_ON flag has 
 *      been set. 
 *      
 *      CSM parameters are in 16.16 fixed point (FS_FIXED).
 *
 *  Parameters:
 *      state         - [in] pointer to FS_STATE structure
 *      index         - [in] index of glyph whose CSM values are requested
 *      insideCutOff  - [out] inside cutoff value.
 *      outsideCutOff - [out] outside cutoff value.
 *      gamma         - [out] transition function gamma value.
 *
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  Version:
 *      Introduced in iType 5.1.
 *
 *  <GROUP statecontrol>
 */
#ifdef FS_EDGE_RENDER
ITYPE_API FS_LONG FS_get_csm(_DS_ FS_USHORT index, FS_FIXED *insideCutOff, FS_FIXED *outsideCutOff, FS_FIXED *gamma);
#endif

/*****************************************************************************
 *
 *  Sets the CSM adjustment values for Edge(TM) rendering.
 *
 *  Description:
 *      This function sets adjustment values that are applied to
 *      the CSM parameters used in Edge(TM) rendering.
 *      Adjustment values are specified in terms of sharpness and
 *      thickness adjustments and are applied to the CSM values
 *      currently in use (either from the font or defaults).
 *
 *      The final adjustment is PPEM specific. The CSM adjustments
 *      are specified in terms of a slope and offset for thickness
 *      and sharpness. The slope makes the adjustment of each a 
 *      linear function of PPEM size.
 *
 *      CSM adjustments are in 16.16 fixed point (FS_FIXED).
 *      Positive offset values increase thickness/sharpness.
 *      Negative offset values decrease thickness/sharpness (i.e. less sharp).
 *      Positive slope values increase thickness/sharpness as a function of PPEM.
 *      Negative slope values decrease thickness/sharpness as a function of PPEM.
 *      Adjustments do not affect gamma.
 *
 *  Parameters:
 *      state            - [in] pointer to FS_STATE structure
 *      sharpnessOffset  - [in] sharpness adjustment offset
 *      sharpnessSlope   - [in] sharpness adjustment slope (vs PPEM)
 *      thicknessOffset  - [in] thickness adjustment offset
 *      thicknessSlope   - [in] thickness adjustment slope (vs PPEM)
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  Version:
 *      Introduced in iType 5.1.
 *
 *  <GROUP statecontrol>
 */
#ifdef FS_EDGE_RENDER
ITYPE_API FS_LONG FS_set_csm_adjustments(_DS_ FS_FIXED sharpnessOffset, FS_FIXED sharpnessSlope,
                                              FS_FIXED thicknessOffset, FS_FIXED thicknessSlope);
#endif

/*****************************************************************************
 *
 *  Gets the CSM adjustment values for Edge(TM) rendering.
 *
 *  Description:
 *      This function gets adjustment values that are applied to
 *      the CSM parameters used in Edge(TM) rendering.
 *      Adjustment values are specified in terms of sharpness and
 *      thickness adjustments and are applied to the CSM values
 *      currently in use (either from the font or defaults).
 *
 *      The final adjustment is PPEM specific. The CSM adjustments
 *      are specified in terms of a slope and offset for thickness
 *      and sharpness. The slope makes the adjustment of each a 
 *      linear function of PPEM size.
 *
 *      sharpnessAdjustment = sharpnessOffset + PPEM * sharpnessSlope
 *
 *      thicknessAdjustment = thicknessOffset + PPEM * thicknessSlope
 *
 *      adjustedOutsideCutoff = outsideCutoff + (sharpnessAdjustment - thicknessAdjustment)
 *
 *      adjustedInsideCutoff  = insideCutoff  - (sharpnessAdjustment + thicknessAdjustment)
 *
 *      Note the sign of the adjustments as applied to the cutoffs.
 *      Note that outside cutoffs are negative by definition.
 *
 *      CSM adjustments are in 16.16 fixed point (FS_FIXED).
 *      Positive offset values increase thickness/sharpness.
 *      Negative offset values decrease thickness/sharpness (i.e. less sharp).
 *      Positive slope values increase thickness/sharpness as a function of PPEM.
 *      Negative slope values decrease thickness/sharpness as a function of PPEM.
 *      Adjustments do not affect gamma.
 *
 *  Parameters:
 *      state            - [in] pointer to FS_STATE structure
 *      sharpnessOffset  - [out] sharpness adjustment offset
 *      sharpnessSlope   - [out] sharpness adjustment slope (vs PPEM)
 *      thicknessOffset  - [out] thickness adjustment offset
 *      thicknessSlope   - [out] thickness adjustment slope (vs PPEM)
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  Version:
 *      Introduced in iType 5.1.
 *
 *
 *  <GROUP statecontrol>
 */
#ifdef FS_EDGE_RENDER
ITYPE_API FS_LONG FS_get_csm_adjustments(_DS_ FS_FIXED *sharpnessOffset, FS_FIXED *sharpnessSlope,
                                              FS_FIXED *thicknessOffset, FS_FIXED *thicknessSlope);
#endif

/*****************************************************************************
 *
 *  Returns the current version of iType. 
 *
 *  Description:
 *      The version is returned as a single FS_ULONG.  The upper 16 bits
 *      give the major version, typically a small integer.  The lower
 *      16 bits give the minor version as a multiple of 100 to allow
 *      for interim releases.
 *
 *      For example, in version 2.4 the major version is set to 2 and the
 *      minor version is set to 400.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *
 *  Return value:
 *      Version number
 *
 *  Note: 
 *      There is also a compiler define FS_VERSION_STRING that defines the 
 *      iType version as a string, e.g. "4.0.0". This define may be used
 *      to initialize string constants in calling applications.
 *      FS_VERSION_STRING was introduced in iType 4.0.
 *      
 *  <GROUP admin>
 */
ITYPE_API FS_ULONG FS_get_version(_DS0_);


/*****************************************************************************
 *
 *  How much runtime-allocated memory are we using?
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *
 *  Return value:
 *      Current amount of heap used (in bytes)
 *      
 *  Version:
 *      Introduced in iType 2.4. 
 *
 *  <GROUP fsmemmgr>
 */
ITYPE_API FS_ULONG FS_get_heap_used(_DS0_);


/*****************************************************************************
 *
 *  User-implemented callback provides the target font information for a linked font.
 *
 *  Description:
 *      When a linked font (.ltt file) is created, usually using <I>FontLinker</I>,
 *      each component font may be specified to be found either:
 *      1.  embedded in the .ltt file,
 *      2.  within a device target directory,
 *      3.  at a device memory address, or
 *      4.  dynamically, via a callback function as implemented by the user.
 *
 *      This is the callback that the user must implement for the dynamically-determined
 *      target component font information to be found.
 *
 *      When building the linked font, a unique "callback ID" is associated with
 *      each component for which this dynamic determination is to be used.  The
 *      callback ID is any string that the user desires.
 *
 *      When this callback is utilized, the per-component, unique callback ID will
 *      be passed as the first parameter.  The users implementation of the callback
 *      should use this ID to locate the actual component font and set the
 *      appropriate function parameters which to use to find and load the font.
 *
 *      The function parameters allow returning a memory address at which the font
 *      can be found or, alternatively, a pathname, offset, and length combination
 *      for a file system-based font, either within a resource file or stand-alone.
 *
 *      When returning a pathname for a file system-based font, the buffer into
 *      which the path is put should not be overrun -- the length of the buffer is
 *      given.  The buffer must contain a NULL-terminated string for the path if
 *      the memory-resident address is not used or set.
 *
 *      The callback function should return SUCCESS or an error code as defined
 *      in fs_err.h, appropriately.
 *      
 *  Parameters:
 *      callbackID    - [in] string specified by user when creating the linked font
 *                           to be used as the identifier for the font for which the
 *                           target parameters are requested.
 *      address_p     - [out] if target font is memory-resident, set to the address
 *                            of the font in ROM or RAM.
 *      file_buffer   - [out] buffer into which the pathname of the font should be
 *                            stored if the font is not memory-resident.
 *      buffer_length - [in] length of <I>file_buffer</I> buffer in bytes.
 *      offset_p      - [out] if <I>file_buffer</I> is set to a resource file, store
 *                            any non-zero offset within the resource file at which
 *                            the font will begin.
 *      length_p      - [out] if <I>file_buffer</I> is set to a resource file and
 *                            <I>offset_p</I> is non-zero, store the length of the
 *                            font within the resource file.
 *
 *  Return Value:
 *      SUCCESS, or error code upon failure.
 *
 *  <GROUP fontmanagement>
 */  
FS_ULONG
FS_callback_get_target(FILECHAR *callbackID,
                       void **address_p,
                       FILECHAR *file_buffer, FS_ULONG buffer_length,
                       FS_ULONG *offset_p,
                       FS_ULONG *length_p);

/*****************************************************************************
 *
 *  What is the rotationally invariant measure of size?
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      yppem       - [out] Rounded rotationally invariant yppem.
 *
 *  Return Value:
 *      SUCCESS.
 *      
 *  Version:
 *      Specific to RIM. 
 *
 */
ITYPE_API FS_LONG FS_get_effective_yppem(_DS_ FS_USHORT *yppem);

/*****************************************************************************
 *
 *  Compare two states to determine if they would produce the same output
 *  for a given glyph request.
 *    
 *  Parameters:
 *      sp1         - [in] pointer to FS_STATE structure #1
 *      sp2         - [in] pointer to FS_STATE structure #2
 *      areEqual    - [out] true if the two states would produce the identical glyphmap
 *                          otherwise false.
 *
 *  Return Value:
 *      SUCCESS, or error code upon failure.
 *      
 *  Version:
 *      Specific to RIM. 
 *
 */
ITYPE_API FS_LONG FS_compare_states(FS_STATE *sp1, FS_STATE *sp2, FS_BOOLEAN *areEqual);

/* multi-threaded API calls */
/* For multi-threaded builds, these functions are defined in fs_multi.c       */
/* They provide mutex protection around non-protected FSS_ versions.          */
/* For single-threaded builds, the non-mutex protected FSS_ version is used   */
/* and a macro defines the corresponding FS_ function to be the FSS_ version. */

#ifdef FS_MULTI_THREAD

/*****************************************************************************
 *
 *  Constructs an iType server and initializes the 'master client's' data
 *
 *  Description:
 *      This function should generally be called only once per server, 
 *      upon startup of whichever system controls iType.
 *      There are some differences in the FS_MULTI_PROCESS case.
 *      The corresponding shutdown function is FS_exit().
 *
 *      FS_init does the following:
 *      - validates the basic data types defined in port.h
 *      - validates certain configuration options defined in port.h
 *      - constructs and initializes the server
 *      - initializes the master client's data, stored in FS_STATE
 *      
 *      Constructing the server involves:
 *      - allocating the server structure using malloc
 *      - constructing the heap (see Notes section below) 
 *
 *      The heap is used to store data that is shared among clients:
 *      - list of loaded fonts
 *      - loaded font data
 *      - scaled font data and cache
 *      - glyph cache
 *       
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      heap_size   - [in] the maximum amount of RAM, in bytes, that this 
 *                         iType engine will use for its heap
 *
 *  Return value:
 *      SUCCESS, or an error code if the engine could not be initialized.
 *
 *  Notes:
 *      If FS_MULTI_PROCESS is defined, FS_init() is called by each new process,
 *      in order to create the shared memory segment (or attach to an
 *      existing shared memory segment). When creating the shared memory
 *      segment, the server is also constructed and initialized.
 *        
 *      If FS_MULTI_THREAD is defined, only the master client should 
 *      call FS_init().  Other threads should call FS_new_client().
 *
 *      The master client must allocate an 
 *      FS_STATE structure and pass in a pointer to that structure.
 *      Multiple iType clients are allowed per engine.
 *
 *      If FS_INT_MEM is defined, then the heap is constructed by allocating
 *      a contiguous buffer of heap_size bytes.  If FS_INT_MEM is not 
 *      defined then system memory will be used as needed, up to heap_size bytes,
 *      but no contiguous buffer is allocated here. 
 *
 *      It is an unchecked run-time error to call FS_init() twice for the same
 *      FS_STATE structure without first having called FS_exit.
 *
 *  See also:
 *      FS_exit(), FS_new_client(), FS_end_client(), FS_STATE
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_LONG FS_init(_DS_ FS_ULONG heap_size);



/*****************************************************************************
 *
 *  Sets the current font.  
 *
 *  Description:
 *      This function sets the 'current' font.
 *      The current font is the one used for subsequent calls to get font
 *      information, get glyphs, or get other character/glyph-specific
 *      information.  The current font remains active until another font is 
 *      set, or the current font is deleted.
 *
 *      Each client maintains its own value for current font.
 *
 *      The first time a particular font is set, iType validates
 *      the file and loads vital font tables.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      name        - [in] font name as null terminated string
 *                         This is the name that was used with FS_add_font().
 *
 *  Return Value:
 *      SUCCESS, or error code if font could not be set.
 *
 *      <TABLE>
 *      Return values           Reason
 *      ------------------      --------------------------------
 *      ERR_FONT_NOT_FOUND      font by that name is not found
 *      ERR_FONT_NOT_VALID      current font file is not a valid font
 *      ERR_MALLOC_FAIL         not enough memory to load font.
 *      ERR_NOT_A_TTF           font is not a TTF font
 *      ERR_ACT3_DISK           font is a disk-based ACT3 compressed font
 *      ERR_ACT3_UNDEF          font is ACT3 compressed but FS_ACT3 configuration not defined
 *      ERR_PFR_UNDEF           font is in PFR format but PFR configuration not defined
 *      </TABLE>
 *      
 *  <GROUP statecontrol>
 */ 
ITYPE_API FS_LONG FS_set_font(_DS_ FILECHAR *name);


/*****************************************************************************
 *
 *  Sets the current scaling matrix. 
 *
 *  Description:
 *      The four arguments form a 2x2 scaling matrix. Values are in 16.16
 *      fixed point notation.
 *
 *      The coordinates are computed as follows:
 *          -   x' = x*s00 + y*s01
 *          -   y' = x*s10 + y*s11
 *
 *      For example, the call FS_set_scale(12L<<16, 0, 0, 12L<<16)
 *      specifies a 12 pixel-per-em font, with no rotation or skewing.
 *
 *      For help in creating generalized matrices, look at the
 *      function CSettings::calculate_matrix() in 
 *      samples/win32/wintest/glyphsettings.cpp.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      s00         - [in] row 0, column 0
 *      s01         - [in] row 0, column 1
 *      s10         - [in] row 1, column 0
 *      s11         - [in] row 1, column 1
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *      <TABLE>
 *      Error                   Reason
 *      -------------------     -------------------------------------
 *      ERR_NO_CURRENT_LFNT     no current loaded font
 *      ERR_SCALE_LIMIT         characters scale would exceed limit of +/- 16,384).
 *      ERR_SCALE_DEGENERATE    matrix is degenerate
 *      </TABLE>
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_scale(_DS_ FS_FIXED s00,FS_FIXED s01,FS_FIXED s10,FS_FIXED s11);

/*****************************************************************************
 *
 *  Gets the current scaling matrix. 
 *
 *  Description:
 *      The four arguments form a 2x2 scaling matrix. See FS_set_scale.
 *      Matrix values are in 16.16 fixed point notation.
 *
 *      Coordinates are computed using the matrix as follows:
 *          -   x' = x*s00 + y*s01
 *          -   y' = x*s10 + y*s11
 *
 *      An error code is generated if no current scaling
 *      exists. FS_set_scale should be called before this function. 
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      s00         - [out] row 0, column 0
 *      s01         - [out] row 0, column 1
 *      s10         - [out] row 1, column 0
 *      s11         - [out] row 1, column 1
 *
 *  Return Value:
 *      SUCCESS, or error code. 
 *
 *      <TABLE>
 *      Error                   Reason
 *      -------------------     -------------------------------------
 *      ERR_NO_CURRENT_FNTSET   no current loaded font
 *      ERR_NO_CURRENT_SFNT     no current scaled font (FS_set_scale not called)
 *      </TABLE>
 *
 *  Version:
 *      Introduced in Version 4.0
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_get_scale(_DS_ FS_FIXED *s00,FS_FIXED *s01,FS_FIXED *s10,FS_FIXED *s11);

/*****************************************************************************
*
*  Gets the font design units per em associated with the current 
*  component font that was last used to render a glyph 
*
*  Parameters:
*      state           - [in] pointer to FS_STATE structure
*
*  Return Value:
*      Design units of component font last used or 0 with error code
*
*  <GROUP fontinfo>
*/
ITYPE_API FS_USHORT FS_get_design_units(_DS0_);

/*****************************************************************************
 *
 *  Sets the current cmap table (default is no cmap). 
 *
 *  Description:
 *      The cmap defines the mapping of character value to glyph index.  
 *      The default is 'no cmap', which treats character value as 
 *      glyph index.  This can be useful for testing, but most users
 *      will want to choose a valid cmap.
 *
 *      Choose a cmap based on how you encode characters and on what
 *      cmaps are available in the font, which can be found using FS_font_metrics().
 *      Fonts now typically have a cmap for the Microsoft platform - Unicode encoding, 
 *      designated as platform 3, encoding 1.
 *
 *      Note that the platform does not refer to your target platform, but instead
 *      to one of the platforms registered with the TrueType specification.
 *
 *      If an unsupported cmap is specified, this function returns ERR_CMAP_UNSUPPORTED, and
 *      sets the current cmap to 'no cmap'.  If 'no cmap' is actually desired, then
 *      you may call this function with known invalid values, such as 9999,9999.
 *      The ERR_CMAP_UNSUPPORTED error is still returned, but you will know to ignore it.
 *
 *  Parameters:
 *      state           - [in] pointer to FS_STATE structure
 *      platform        - [in] cmap corresponds with this platform
 *      encoding_type   - [in] cmap encoding (encoding types are platform specific)
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_cmap(_DS_ FS_USHORT platform, FS_USHORT encoding_type);

/*****************************************************************************
 *
 *  Gets the current cmap table platform and encoding. 
 *
 *  Description:
 *      The cmap defines the mapping of character value to glyph index.  
 *      The cmap table in use is determined by the Microsoft platform - Unicode
 *      encoding, e.g. platform 3, encoding 1.
 *
 *      Note that the platform does not refer to your target platform, but instead
 *      to one of the platforms registered with the TrueType specification.
 *
 *      This function returns the platform and encoding previously set using FS_set_cmap.
 *      If no cmap has been specified, the returned platform and encoding values will both
 *      be 0xFFFF. This is not an error condition. It simply means that no cmap has been 
 *      specified. Callers should test for this return condition.
 *
 *
 *  Parameters:
 *      state           - [in] pointer to FS_STATE structure
 *      platform        - [out] cmap corresponds with this platform
 *      encoding_type   - [out] cmap encoding (encoding types are platform specific)
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  Version:
 *      Introduced in Version 4.0
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_get_cmap(_DS_ FS_USHORT *platform, FS_USHORT *encoding_type);

/*****************************************************************************
 *
 *  Sets the cmap table used to retrieve Ideographic Variations (default is 0,5). 
 *
 *  Description:
 *      Some fonts have a special cmap (format 14) for getting Ideographic
 *      Variations of glyphs. See FS_map_char_variant().
 *      The variant cmap defines the mapping used to retrieve these variations.  
 *      The default is platform = 0 and encoding = 5. This function may be used
 *      to change the default variant cmap used by FS_map_char_variant().
 *      The variant cmap should have a format 14 subtable in which variations
 *      are defined. 
 *
 *      Note that the platform does not refer to your target platform, but instead
 *      to one of the platforms registered with the TrueType specification.
 *
 *      This function does not check whether a valid variant cmap is present.
 *      If the variant cmap is not present, FS_map_char_variant() returns
 *      the default variation in all cases.
 *
 *  Parameters:
 *      state           - [in] pointer to FS_STATE structure
 *      variantPlatform - [in] cmap corresponds with this platform
 *      variantEncoding - [in] cmap encoding (encoding types are platform specific)
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  Version:
 *      Introduced in Version 5.0
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_set_cmap_variant(_DS_ FS_USHORT variantPlatform, FS_USHORT variantEncoding);

/*****************************************************************************
 *
 *  Gets the variant cmap table platform and encoding. 
 *
 *  Description:
 *      The variant cmap defines the mapping of Ideographic Variations within a font.
 *      This function returns the mapping in current use by the iType state.
 *      The default mapping is platform = 0, encoding = 5.
 *
 *      Note that the platform does not refer to your target platform, but instead
 *      to one of the platforms registered with the TrueType specification.
 *
 *      This function returns the platform and encoding previously set as the default or
 *      using FS_set_cmap_variant().
 *
 *  Parameters:
 *      state            - [in] pointer to FS_STATE structure
 *      variantPlatform  - [out] cmap corresponds with this platform
 *      variantEncoding  - [out] cmap encoding (encoding types are platform specific)
 *
 *  Return Value:
 *      SUCCESS.
 *
 *  Version:
 *      Introduced in Version 5.0
 *
 *  <GROUP statecontrol>
 */
ITYPE_API FS_LONG FS_get_cmap_variant(_DS_ FS_USHORT *variantPlatform, FS_USHORT *variantEncoding);

/*****************************************************************************
 *
 *  Returns a scaled outline glyph.
 *
 *  Description:
 *      This function returns a glyph as a scaled outline.  This glyph form
 *      may be useful for some users, but most will want to use one of the 
 *      functions that also rasterize the glyph such as FS_get_bitmap()
 *      or FS_get_graymap().
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *
 *  Return Value:
 *      Pointer to FS_OUTLINE structure, or NULL upon error.
 *
 *  See Also:
 *      FS_OUTLINE, FS_free_char(), FS_get_bitmap(), FS_get_graymap()
 *      and the flag FLAGS_GRAYSCALE.
 *
 *  <GROUP getglyphs>
 */ 
ITYPE_API FS_OUTLINE *FS_get_outline(_DS_ FS_ULONG id);


/*****************************************************************************
 *
 *  Gets pixel coordinates for one or more points that define a scaled
 *  outline glyph.
 *
 *  Description:
 *      This function receives a glyph index and an array of one or more point
 *      indices.  It returns the F26.6 pixel coordinates of the points at
 *      those indices, when the glyph is scaled using the current 
 *      scaling matrix.  The coordinates are returned as arrays.
 *
 *      The user must allocate space for the arrays.
 *
 *      This function is likely used when processing GPOS tables, which reference
 *      by index certain points on the glyph.  The glyph points here are the
 *      original ones stored in the font's glyph table for that glyph index.  You
 *      cannot use an FS_OUTLINE glyph for this purpose because the points may
 *      have been modified from the original.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      index       - [in] glyph index
 *      num         - [in] number of points in <I>pts</I>, <I>x</I>, and <I>y</I> arrays
 *      pts         - [in] array of glyph point indices
 *      x_coord     - [out] array of F26DOT6 x-coordinates corresponding with indices in <I>pts</I> array
 *      y_coord     - [out] array of F26DOT6 y-coordinates corresponding with indices in <I>pts</I> array
 *
 *  Return Value:
 *      SUCCESS, or NULL upon error.
 *
 *  <GROUP getglyphinfo>
 */
ITYPE_API FS_LONG FS_get_gpos_pts(_DS_ FS_USHORT index, FS_USHORT num, FS_USHORT *pts, FS_LONG *x_coord, FS_LONG *y_coord);

/*****************************************************************************
 *
 *  Gets design units, scaling matrix, and x and y "user" scale values
 *  for the component font from which the specified glyph is retrieved.
 *
 *  Description:
 *      This function receives a glyph index.
 *      It returns the design units, the 16.16 fixed point matrix values,
 *      and the x and y "user" scale values (resulting from the
 *      decomposition of the matrix) for the font from which the specified
 *      glyph is found.  This is notable for component fonts of a linked
 *      font where components are scaled differently than the "metric" font
 *      due to height adjustments.
 *
 *      This function is used when processing GPOS table anchor points
 *      associated with the glyph index.  The design units, scaling matrix,
 *      and/or x and y "user" scale values are used to properly ascertain
 *      the relative anchor point position after scaling.  The GPOS processor
 *      needs the values associated with the specific component font of
 *      a linked font set which could contain a different value for design
 *      units and scaling values from the "metric" font of the set.
 *
 *      For caching purposes, the first and last glyph indices in a range
 *      of glyph indices that share the same resultant design units and scale
 *      values will also be returned.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      index       - [in] glyph index
 *      du          - [out] design units
 *      s00         - [out] row 0, column 0
 *      s01         - [out] row 0, column 1
 *      s10         - [out] row 1, column 0
 *      s11         - [out] row 1, column 1
 *      xppm        - [out] x "user" scale value
 *      yppm        - [out] y "user" scale value
 *      idRangeStart- [out] first glyph index in range with same scale values
 *      idRangeEnd  - [out] last glyph index in range with same scale values
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  <GROUP getglyphinfo>
 */
ITYPE_API FS_LONG FS_get_gpos_scale(_DS_ FS_USHORT index, FS_USHORT *du,
                                    FS_FIXED *s00, FS_FIXED *s01, FS_FIXED *s10, FS_FIXED *s11,
                                    FS_FIXED *xppm, FS_FIXED *yppm,
                                    FS_USHORT *idRangeStart, FS_USHORT *idRangeEnd);

/*****************************************************************************
 *
 *  Returns a scaled glyphmap.
 *
 *  Description:
 *      This function returns a glyphmap which can be an icon, a graymap, or a bitmap.
 *      It is a generalized replacement for FS_get_bitmap and FS_get_graymap.
 *
 *      The glyph returned is specified by an id and type. The id value is either
 *      a character value or glyph index, depending on whether FLAGS_CMAP_ON is set.
 *      If FLAGS_CMAP_ON is set, the id represents a character value. This value is
 *      a Unicode value when a Unicode CMAP is used (platform = 3, encoding = 1 or 10).
 *      Otherwise, the id is a character value in the CMAP table selected.
 *      If FLAGS_CMAP_OFF is set, the id represents a glyph index. This applies to icons
 *      as well as regular glyphs. 
 *
 *      The type parameter specifies the type of glyph to return. Valid glyphmap type's are:
 *      <TABLE>
 *      Glyphmap Type            Description
 *      -----------------------  --------------------------------------------------
 *      FS_MAP_ANY_EDGE_GRAYMAP  (FS_MAP_EDGE_GRAYMAP4 | FS_MAP_EDGE_GRAYMAP8 | FS_EDGE_GRAYMAP2)
 *      FS_MAP_EDGE_TECH         (equivalent to FS_MAP_EDGE_GRAYMAP4)
 *      FS_MAP_EDGE_GRAYMAP4     (4-bit Edge(TM) graymap)
 *      FS_MAP_EDGE_GRAYMAP8     (8-bit Edge(TM) graymap)
 *      FS_MAP_EDGE_GRAYMAP2     (2-bit Edge(TM) graymap)
 *      FS_MAP_ANY_GRAYMAP       (FS_MAP_GRAYMAP4 | FS_MAP_GRAYMAP8 | FS_GRAYMAP2)
 *      FS_MAP_GRAYMAP           (equivalent to FS_MAP_GRAYMAP4)
 *      FS_MAP_GRAYMAP4          (4-bit graymap)
 *      FS_MAP_GRAYMAP8          (8-bit graymap)
 *      FS_MAP_GRAYMAP2          (2-bit graymap)
 *      FS_MAP_BITMAP            (1-bit bitmap)
 *      FS_MAP_RASTER_ICON       (size-dependent raster icon)
 *      FS_MAP_VECTOR_ICON       (scalable vector icon)
 *      FS_MAP_ANY_ICON          (FS_MAP_RASTER_ICON | FS_MAP_VECTOR_ICON)
 *      FS_MAP_DISTANCEFIELD     (8-bit encoded distance field map)
 *      </TABLE>
 *
 *      The type parameters are designed so that they can be used in conjunction with
 *      each other by OR'ing them together. 
 *
 *      For example: <CODE>
 *      type = FS_MAP_VECTOR_ICON | FS_MAP_GRAYMAP4 | FS_MAP_BITMAP;
 *      type =   FS_MAP_EDGE_TECH | FS_MAP_GRAYMAP  | FS_MAP_BITMAP;</CODE>
 *      If you "OR" glyphmap type's together, your application code must check the return 
 *      type in the FS_GLYPHMAP data structure and it must provide display code appropriate 
 *      for the type. The function will return the best type available depending on what is
 *      available in the font (icon or glyph), table information in font, and the build 
 *      configuration. 
 *
 *      If the font contains a gasp table, that table will be queried when deciding whether 
 *      to return a graymap or bitmap and whether to use hinting when generating the glyph 
 *      image. If no gasp table is present, preference is given to hinted graymaps over 
 *      bitmaps. The build configuration will also help determine what type of glyphmap to 
 *      return. Only type's included in the build are considered to be candidates to return.
 *
 *      If both Edge graymap and regular graymap type's are included in the requested type, 
 *      the presence of an adfh table (containing Edge parameters) in the font will determine 
 *      whether to return an Edge graymap or regular graymap. If an adfh table exists in the 
 *      font, Edge graymaps are the preferred type of graymap to return when a graymap is
 *      appropriate. 
 *
 *      If the type is set to FS_MAP_DISTANCEFIELD, this function returns a distance field 
 *      map encoded as a graymap. The distance field is generated by the Edge Technology 
 *      renderer which must be defined in the build. The distance field
 *      values are encoded into an 8-bit graymap. The range of the distance
 *      field depends on the current Y ppem scale. This determines the 
 *      symmetric inside and outside cutoff values used during creation.
 *      The relation between Y ppem and cutoff value is as follows:
 *      cutoff = 0.16*ppem + 1
 *      The smallest value (zero) in the graymap maps to a distance equal 
 *      to -cutoff. The largest value (255) in the graymap maps to a distance 
 *      equal to +cutoff.
 *
 *      If Edge graymap caching is enabled (FS_CACHE_EDGE_GLYPHS), the 
 *      distance field map will be cached.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *      type        - [in] type of glyph to create
 *
 *  Return Value:
 *      Pointer to FS_GLYPHMAP structure, or NULL upon error.
 *
 *  See Also:
 *      FS_BITMAP, FS_GRAYMAP, FS_get_bitmap, FS_get_graymap, FS_free_char()
 *
 *  Version:
 *      Introduced in Version 4.0.
 *
 *  <GROUP getglyphs>
 */ 
ITYPE_API FS_GLYPHMAP *FS_get_glyphmap(_DS_ FS_ULONG id, FS_USHORT type);

/*****************************************************************************
 *
 *  Returns a scaled bitmap glyph.
 *
 *  Description
 *      This function returns a glyph as a scaled bitmap.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *
 *  Return Value:
 *      Pointer to FS_BITMAP structure, or NULL upon error.
 *
 *  See Also:
 *      FS_BITMAP, FS_free_char(), FS_GLYPHMAP, FS_get_glyphmap() 
 *
 *  <GROUP getglyphs>
 */ 
ITYPE_API FS_BITMAP *FS_get_bitmap(_DS_ FS_ULONG id);


/*****************************************************************************
 *
 *  Returns a scaled graymap glyph.
 *
 *  Description:
 *      This function returns a glyph as a scaled graymap.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *
 *  Return Value:
 *      Pointer to FS_GRAYMAP structure, or NULL upon error.
 *
 *  See Also:
 *      FS_GRAYMAP, FS_free_char(), FS_GLYPHMAP, FS_get_glyphmap() 
 *
 *  <GROUP getglyphs>
 */ 
ITYPE_API FS_GRAYMAP *FS_get_graymap(_DS_ FS_ULONG id);

/*****************************************************************************
 *
 *  Returns a scaled, phased graymap glyph.
 *
 *  Description:
 *      This function returns a glyph as a graymap that was generated using
 *      the "phased" grayscale algorithm (phasing).
 *
 *      Phasing effectively allows apparent character positioning at ¼ pixel
 *      intervals, and stem weight resolution of ¼ pixel.  In order to obtain this effect,
 *      the line composition algorithm must pass a high-resolution x-position parameter
 *      to FS_get_phased() – this parameter is used to select the appropriate version
 *      of the phased gray map for the character.
 *
 *      Phasing may be appropriate for certain types of display devices.
 *      It is not recommended for general use, however, as it eliminates most
 *      of the benefits of hinting.
 *
 *      Phasing is not the same thing as subpixel rendering.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *      xpos        - [in] x axis position of the origin in fractional pixels
 *
 *  Return Value:
 *      Pointer to FS_GRAYMAP structure, or NULL upon error.
 *
 *  See Also:
 *      FS_GRAYMAP, FS_free_char()
 *
 *  Notes:
 *      To use this function, turn on the FS_PHASED build option.  If FS_PHASED is not 
 *      defined, this function returns a ERR_PHASED_UNDEF error.
 *
 *  <GROUP getglyphs>
 */ 
ITYPE_API FS_GRAYMAP *FS_get_phased(_DS_ FS_ULONG id, FS_FIXED xpos);


/*****************************************************************************
 *
 *  Releases a glyph reference.
 *
 *  Description:
 *      The user must call this function to release a glyph that was
 *      obtained using any of the get_<I>glyph</I> functions.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      p           - [in] reference to a glyph
 *
 *  Return Value:
 *      SUCCESS, or NULL upon error.
 *
 *  <GROUP getglyphs>
 */
ITYPE_API FS_LONG FS_free_char(_DS_ FS_VOID *p);


/*****************************************************************************
 *
 *  Attempts to resize the engine heap.
 *
 *  Description:
 *      This function changes the maximum amount of RAM that can
 *      be used by the engine heap when using the external memory
 *      model.
 *
 *      The heap size can be increased or decreased.  Increasing
 *      heap size changes the maximum allowed value.  It does 
 *      not actually allocate any memory.  Decreasing heap size
 *      frees memory as necessary to meet the requested size. 
 *      It does this by trimming the font and glyph caches
 *      until the heap usage is less than the requested heap size.
 *
 *      When using the INT_MEM (internal) memory model,
 *      this function returns an error.  To resize the heap
 *      you must exit using FS_exit() then restart using FS_init()
 *      with the new size.  All fonts would need to be added again.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      new_size    - [in] the requested new amount of maximum amount of RAM,
 *                         in bytes, that the engine should use for its heap
 *
 *  Return value:
 *      SUCCESS, or ERR_RESIZE_FAILED if the heap could not be reduced
 *      to the requested size, or if the INT_MEM model is being used.
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_LONG FS_resize(_DS_ FS_ULONG new_size);


/*****************************************************************************
 *
 *  Shuts down an iType server.
 *
 *  Description:
 *      This function should generally be called 
 *      only once per server,
 *      upon shutdown of whichever system was used
 *      to call FS_init(). There are some differences in
 *      the FS_MULTI_PROCESS case.
 *
 *      FS_exit does the following
 *        (unconditionally, in the non-FS_MULTI_PROCESS case):
 *        - deletes all cached and temporary objects
 *        - closes all font files
 *        - destroys the server, which frees all heap memory and frees
 *          the server structure.
 *
 *      In the FS_MULTI_PROCESS case, all mapped fonts are deleted, but the
 *      remaining cleanup takes place only when it is the last active process
 *      that calls FS_exit().
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *
 *  Return value:
 *      Always returns SUCCESS.
 *
 *  Notes:     
 *      If FS_MULTI_PROCESS is defined, then each process will call FS_exit()
 *      when it completes. The behavior of FS_exit() will vary depending on
 *      whether there are any active processes other than the process that
 *      called FS_exit().
 *       
 *      If FS_MULTI_THREAD is defined, then only the master client
 *      should call FS_exit().  Other clients must first call FS_end_client().
 *      It is an unchecked run-time error for a non-master client
 *      to call FS_exit().
 *
 *      The master client is responsible for
 *      deleting the FS_STATE structure after calling FS_exit().
 *
 *      It is an unchecked run-time error to make any iType call
 *      after having called FS_exit() for a particular iType instance.
 *
 *  See also:
 *      FS_init(), FS_new_client(), FS_end_client(), FS_STATE
 *
 *  <GROUP admin>
 */   
ITYPE_API FS_LONG FS_exit(_DS0_);


/*****************************************************************************
 *
 *  Adds a font to the current list of known fonts.
 *
 *  Description:
 *      The iType engine maintains a list of known fonts.  This function is
 *      used to add a font to that list.
 *
 *      For our purposes, a "font" is a file that complies with the TrueType
 *      format, or any of the TrueType extensions supported by iType, 
 *      or a PFR Format 0 font.
 *
 *      The file may be file system-based or memory mapped.  iType works
 *      more efficiently with memory-mapped files, both in terms of speed
 *      and heap usage.  This is true even if the file system is memory-based,
 *      which is common with embedded systems.
 *
 *      The name parameter is a client-supplied string.  The string must
 *      be used later to set the current font, using FS_set_font(), or to 
 *      delete a font using FS_delete_font().
 *
 *      If an application calls FS_add_font() with a NULL name, iType will use 
 *      FS_font_name() to obtain an appropriate name for the font, and add it
 *      under that name.  However, it would be preferable to use FS_load_font()
 *      in such cases, or for the application to call FS_font_name() directly
 *      to obtain a font name, and pass that name to FS_add_font().
 *              
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      name        - [in] user supplied null terminated string that
 *                         defines name of the font.
 *      path        - [in] path name of font when font is file system based.
 *                         If memptr is defined this argument should be NULL.
 *      memptr      - [in] address of font when font is memory-mapped in FLASH_ROM, ROM, or RAM.
 *                         If path is defined then this argument should be NULL.
 *      index       - [in] generally this should be set to 0, unless the font is a 
 *                         TrueType collection, in which case this should be set to the
 *                         index of the desired font within the collection.
 *
 *  Return Value:
 *      SUCCESS, or error code if font could not be added.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_FONT_NOT_FOUND          Both the 'path' and 'memptr' inputs were NULL
 *      ERR_FONT_NAME_IN_USE        A different font (with the same name) is already in the list of known fonts
 *      ERR_FONT_NAME_NOT_UNIQUE    The same font (with a different nane) is already in the list of known fonts
 *      </TABLE>
 *
 *  Notes:
 *      iType treats each font within a TrueType collection as a separate font.
 *      You may call this function once for each of the fonts in the collection, 
 *      using the same path or memptr argument.  Each call requires a unique font
 *      <I>name</I> as well as the unique collection <I>index</I> for that font.
 *      The number of fonts in a collection can be found in the FONT_METRICS structure
 *      returned by FS_font_metrics().
 *
 *  See Also:
 *      FS_delete_font(), FS_font_name(), FS_load_font()
 *
 *  <GROUP fontmanagement>
 */  
ITYPE_API FS_LONG FS_add_font(_DS_ FILECHAR *name, FILECHAR *path, FS_BYTE *memptr, FS_ULONG index);


/*****************************************************************************
 *
 *  Adds a font (located at specified offset) to the current list of known fonts.
 *
 *  Description:
 *      This function is identical to FS_add_font() except that it handles fonts
 *      located at the specified offset within a file.
 *              
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      name        - [in] user supplied null terminated string that
 *                         defines name of the font.
 *      path        - [in] path name of font when font is file system based.
 *                         If memptr is defined this argument should be NULL.
 *      memptr      - [in] address of font when font is memory-mapped in FLASH_ROM, ROM, or RAM.
 *                         If path is defined then this argument should be NULL.
 *      index       - [in] generally this should be set to 0, unless the font is a 
 *                         TrueType collection, in which case this should be set to the
 *                         index of the desired font within the collection.
 *      data_offset - [in] offset (in bytes) to start of font within file
 *      data_length - [in] length (in bytes) of font within file
 *
 *  Return Value:
 *      SUCCESS, or error code if font could not be added.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_FONT_NOT_FOUND          Both the 'path' and 'memptr' inputs were NULL
 *      ERR_FONT_NAME_IN_USE        A different font (with the same name) is already in the list of known fonts
 *      ERR_FONT_NAME_NOT_UNIQUE    The same font (with a different nane) is already in the list of known fonts
 *      </TABLE>
 *
 *  See Also:
 *      FS_add_font(), FS_delete_font(), FS_font_name(), FS_load_font()
 *
 *  Version:
 *      Introduced in iType 3.1. 
 *
 *  <GROUP fontmanagement>
 */  
ITYPE_API FS_LONG FS_add_font_with_offset(_DS_ FILECHAR *name, FILECHAR *path, FS_BYTE *memptr, FS_ULONG index,
                                          FS_ULONG data_offset, FS_ULONG data_length);

/*****************************************************************************
 *
 *  Return font name given in the font's name table
 *
 *  Description:
 *      The function FS_font_name() returns the name in a font's name table. 
 *      This function can be used by an application (or by iType, internally)
 *      to obtain an appropriate name for a font to be added to the list of
 *      known fonts, via FS_add_font() or FS_load_font().
 *
 *      The caller must allocate space for the <I>name</I> name-buffer.
 *      The <I>name_len</I> should be at least 2 bytes although a larger
 *      value is recommended, up to MAX_FONT_NAME_LEN.
 *
 *      An error will be returned if the font could not be loaded, if the 
 *      allocated buffer was too small, or if no name was found.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      path        - [in] path name of font when font is file system based.
 *                           If memptr is defined this argument should be NULL.
 *      memptr      - [in] address of font when font is memory-mapped in ROM or RAM.
 *                           If path is defined then this argument should be NULL.
 *      index       - [in] generally this should be set to 0, unless the font is a 
 *                           TrueType collection, in which case this should be set to the
 *                           index of the desired font within the collection.
 *      name_len    - [in] length of <I>name</I> buffer in bytes, min = 2, max = MAX_FONT_NAME_LEN
 *      name        - [out] buffer that will contain returned font name as null terminated string
 *                            (allocated by caller)

 *  Return value:
 *      Error code
 *      
 *  See Also:
 *      FS_add_font(), FS_load_font()
 *
 *  <GROUP fontmanagement>
 */
ITYPE_API FS_LONG FS_font_name(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len, FILECHAR *name);

/*****************************************************************************
 *
 *  Return font name given in the font's name table (accessing font with offset)
 *
 *  Description:
 *      This function is identical to FS_font_name() except that it handles fonts
 *      located at the specified offset within a file.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      path        - [in] path name of font when font is file system based.
 *                         If memptr is defined this argument should be NULL.
 *      memptr      - [in] address of font when font is memory-mapped in ROM or RAM.
 *                         If path is defined then this argument should be NULL.
 *      index       - [in] generally this should be set to 0, unless the font is a 
 *                         TrueType collection, in which case this should be set to the
 *                         index of the desired font within the collection.
 *      name_len    - [in] length of <I>name</I> buffer in bytes, min = 2, max = MAX_FONT_NAME_LEN
 *      name        - [out] buffer that will contain returned font name as null terminated string
 *                            (allocated by caller)
 *      data_offset - [in] offset (in bytes) to start of font within file
 *      data_length - [in] length (in bytes) of font within file
 *
 *  Return value:
 *      Error code
 *      
 *  See Also:
 *      FS_font_name(), FS_add_font(), FS_load_font()
 *
 *  Version:
 *      Introduced in iType 3.1. 
 *
 *  <GROUP fontmanagement>
 */
ITYPE_API FS_LONG FS_font_name_with_offset(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len, 
                                           FILECHAR *name, FS_ULONG data_offset, FS_ULONG data_length);

/*****************************************************************************
 *
 *  Return the name of the current font.
 *
 *  Description:
 *      The function FS_get_name() returns name of the currently
 *      set font.  This is the same name what would have been
 *      returned from FS_load_font().
 *
 *      The caller must allocate space for the <I>name</I> name-buffer.
 *      The <I>name_len</I> should be at least 2 bytes although a larger
 *      value is recommended, up to MAX_FONT_NAME_LEN.
 *
 *      An error will be returned if the glyph id does not exist within the linked font.
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      name_len    - [in] length of <I>name</I> buffer in bytes, min = 2, max = MAX_FONT_NAME_LEN
 *      name        - [out] buffer that will contain returned font name as null terminated string
 *                            (allocated by caller)

 *  Return value:
 *      Error code
 *      
 *  Version:
 *      Introduced in iType 3.1. 
 *
 *  <GROUP fontmanagement>
 */
ITYPE_API FS_LONG FS_get_name(_DS_ FS_ULONG name_len, FILECHAR *name);

/*****************************************************************************
 *
 *  Adds a font to the current list of known fonts and returns font name.
 *
 *  Description:
 *      The iType engine maintains a list of known fonts.  This function is
 *      used to add a font to that list.  It is similar to FS_add_font()
 *      except that instead of specifying a name for the font, the name is 
 *      extracted from the font itself (by calling FS_font_name()), 
 *      and returned via an argument.
 *
 *      The caller must allocate space for the <I>name</I> name-buffer.
 *      The <I>name_len</I> should be at least 2 bytes although a larger
 *      value is recommended, up to MAX_FONT_NAME_LEN.  The name 
 *      is guaranteed to be different from that of other loaded/added fonts.
 *      The font name is ideally the name found in the font, but it may have
 *      been modified to be made unique or to fit into the <I>name</I> buffer.
 *
 *      In the very unlikely event that the name cannot be unique and fit into
 *      the buffer, the function returns an error, and will not unload the font.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      path        - [in] path name of font when font is file system based.
 *                         If memptr is defined this argument should be NULL.
 *      memptr      - [in] address of font when font is memory-mapped in ROM or RAM.
 *                         If path is defined then this argument should be NULL.
 *      index       - [in] generally this should be set to 0, unless the font is a 
 *                         TrueType collection, in which case this should be set to the
 *                         index of the desired font within the collection.
 *      name_len    - [in] length of <I>name</I> buffer in bytes, min = 2, max = MAX_FONT_NAME_LEN
 *      name        - [out] buffer containing returned font name as null terminated string
 *
 *  Return Value:
 *      SUCCESS, or error code if font could not be added.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_FONT_NOT_FOUND          Both the 'path' and 'memptr' inputs were NULL
 *      ERR_FONT_BUFFER_TOO_SMALL   Buffer supplied is too small to hold the font name that was generated
 *      ERR_DUPLICATED_FONT_NAME    Could not generate a font name (internal error)
 *      </TABLE>
 *
 *  See Also:
 *      FS_add_font(), FS_delete_font()
 *
 *  <GROUP fontmanagement>
 */  
ITYPE_API FS_LONG FS_load_font(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len, FILECHAR *name);

/*****************************************************************************
 *
 *  Adds a font (located at specified offset) to the current list of known fonts and returns font name.
 *
 *  Description:
 *      This function is identical to FS_load_font() except that it handles fonts
 *      located at the specified offset within a file.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      path        - [in] path name of font when font is file system based.
 *                         If memptr is defined this argument should be NULL.
 *      memptr      - [in] address of font when font is memory-mapped in ROM or RAM.
 *                         If path is defined then this argument should be NULL.
 *      index       - [in] generally this should be set to 0, unless the font is a 
 *                         TrueType collection, in which case this should be set to the
 *                         index of the desired font within the collection.
 *      name_len    - [in] length of <I>name</I> buffer in bytes, min = 2, max = MAX_FONT_NAME_LEN
 *      name        - [out] buffer containing returned font name as null terminated string
 *      data_offset - [in] offset (in bytes) to start of font within file
 *      data_length - [in] length (in bytes) of font within file
 *
 *  Return Value:
 *      SUCCESS, or error code if font could not be added.
 *
 *      <TABLE>
 *      Return values               Reason
 *      ------------------          --------------------------------
 *      ERR_FONT_NOT_FOUND          Both the 'path' and 'memptr' inputs were NULL
 *      ERR_FONT_BUFFER_TOO_SMALL   Buffer supplied is too small to hold the font name that was generated
 *      ERR_DUPLICATED_FONT_NAME    Could not generate a font name (internal error)
 *      </TABLE>
 *
 *  See Also:
 *      FS_load_font(), FS_add_font(), FS_delete_font()
 *
 *  Version:
 *      Introduced in iType 3.1. 
 *
 *  <GROUP fontmanagement>
 */  
ITYPE_API FS_LONG FS_load_font_with_offset(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len,
                                           FILECHAR *name, FS_ULONG data_offset, FS_ULONG data_legnth);

ITYPE_API FS_LONG FS_get_font_path(_DS_ FILECHAR *name,
                                   FS_BYTE **memptr_p,
                                   FILECHAR *path_buffer, FS_ULONG buffer_length,
                                   FS_ULONG *index,
                                   FS_ULONG *offset_p,
                                   FS_ULONG *length_p);

/*****************************************************************************
 *
 *  Deletes a font from the current list of known fonts.
 *
 *  Description:
 *      The iType engine maintains a list of known fonts.  This function is
 *      used to delete a font from that list. Actually, the reference count of
 *      the font is decremented - the font will be eligible for deletion after
 *      its reference count goes to zero (so that the same font can be used
 *      safely by multiple clients).     
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      name        - [in] font name as null terminated string.
 *                         This is the name that was used with FS_add_font().
 *
 *  Return Value:
 *      SUCCESS, or error code if font could not be deleted.
 *
 *      <TABLE>
 *      Return values           Reason
 *      ------------------      --------------------------------
 *      ERR_FONT_IN_USE         font is in use by another client
 *      ERR_FONT_NOT_FOUND      font by that name is not found
 *      ERR_DELETE_FONT         reference count for that font is already zero
 *      </TABLE>
 *
 *  See Also:
 *      FS_set_font(), FS_add_font().
 *
 *  <GROUP fontmanagement>
 */ 
ITYPE_API FS_LONG FS_delete_font(_DS_ FILECHAR *name);

/*****************************************************************************
 *
 *  Returns advance width. 
 *
 *  Description:
 *      This function returns the fixed point advance width for a given ID at
 *      the current scale using the fastest method available.
 *
 *      The <I>advance</I> is the distance from the current origin to
 *      that of the next character. This structure supplies the
 *      advance in two forms: (1) integer and (2) fractional pixels.
 *      Most of the time, the integer pixel advance will give the
 *      best intercharacter spacing results. You must use the
 *      fractional pixel advance whenever both i_dx and i_dy are both
 *      zero as happens when the text is rotated. See the
 *      iType User Guide for a detailed explanation of text layout
 *      issues.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *      type        - [in] type of glyph for advance
 *      i_dx        - [out] integer x-direction advance
 *      i_dy        - [out] integer y-direction advance
 *      dx          - [out] returns x-direction advance (fixed 16.16)
 *      dy          - [out] returns y-direction advance (fixed 16.16)
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  See Also:
 *      FS_get_outline, FS_get_bitmap, FS_get_graymap, FS_get_glyphmap.
 *
 *  Version:
 *      Introduced in iType 4.1. 
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_LONG FS_get_advance(_DS_ FS_ULONG id, FS_USHORT type, 
                                 FS_SHORT *i_dx, FS_SHORT *i_dy, 
                                 FS_FIXED *dx, FS_FIXED *dy);

/*****************************************************************************
 *
 *  Gets glyph-specific metrics. 
 *
 *  Description:
 *      This function returns unscaled glyph metrics in integer form for the 
 *      specified glyph index. Both horizontal and vertical metrics are 
 *      returned. The glyph metrics are relative to the FONT_METRICS unitsPerEm.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      index       - [in] glyph index for desired metrics
 *      lsb         - [out] unscaled left side bearing for horizontal writing
 *      aw          - [out] unscaled advance width for horizontal writing
 *      tsb         - [out] unscaled topside bearing for vertical writing
 *      ah          - [out] unscaled advance height for vertical writing
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  See Also:
 *      FONT_METRICS, FS_font_metrics.
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_LONG FS_get_glyph_metrics(_DS_ FS_USHORT index, FS_SHORT *lsb, FS_SHORT *aw, FS_SHORT *tsb, FS_SHORT *ah);

/*****************************************************************************
 *
 *  Gets font-specific metrics. 
 *
 *  Description:
 *      This function initializes a FONT_METRICS structure using the
 *      current font.  The structure must be allocated by the calling program.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      fm          - [out] pointer to a font metrics structure declared by caller.
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  See Also:
 *      FONT_METRICS.
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_LONG FS_font_metrics(_DS_ FONT_METRICS *fm);

/*****************************************************************************
 *
 *  Gets font-specific ascender, descender, and leading.
 *
 *  Description:
 *      This function gets the fixed-point pixel values for ascender,
 *      descender, and leading of a scaled font based on current
 *      configuration, flags, and a rule set.
 *
 *      Note that the sign convention for the ascender and descender values
 *      are different. The ascender is positive if the ascender is above
 *      the baseline. The descender value is positive if the 
 *      descender falls below the baseline. Normally, the return value for
 *      both ascender and descender will be positive.
 *
 *      The ascender and descender will be from VDMX table data if the
 *      font contains a VDMX table and the active PPEM is listed in the
 *      VDMX data.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      ascender    - [out] pointer to a fixed-point value to store the ascender result.
 *      descender   - [out] pointer to a fixed-point value to store the descender result.
 *      leading     - [out] pointer to a fixed-point value to store the leading result.
 *      source      - [out] pointer to a FsAscDescLeadSource enumerated value to store the source of the returned information.
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  See Also:
 *      FONT_METRICS.
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_LONG FS_get_ascender_descender_leading(_DS_ FS_FIXED *ascender, FS_FIXED *descender, FS_FIXED *leading, FsAscDescLeadSource *source);

/*****************************************************************************
 *
 *  Gets the ppem size required to height restrict a font to a particular
 *  pixel height.
 *
 *  Description:
 *      This function gets the pixel-per-em (ppem) size that would be required 
 *      to render all glyphs in the current font so that they do not exceed a 
 *      specified pixel height. The ppem size returned must not exceed a 
 *      specified max ppem value.
 *
 *      Using the returned ppem size, you can construct a scale matrix that
 *      would cause all glyphs rendered from the current font to be within
 *      the specified pixel height.
 *
 *  Parameters:
 *      state        - [in] pointer to FS_STATE structure
 *      pixel_height - [in] the desired max pixel height for all glyphs.
 *      max_ppem     - [in] the max ppem size that can be returned.
 *
 *  Return Value:
 *      ppem size required to render glyphs within pixel_height.
 *
 *  See Also:
 *      FS_set_scale.
 *
 *  Version:
 *      Introduced in iType 4.1. 
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_USHORT FS_get_ppem_size(_DS_ FS_USHORT pixel_height, FS_USHORT max_ppem);

/*****************************************************************************
 *
 * Get parsed table contents for some TrueType tables
 *
 *  Description:
 *      This function returns TrueType table information for certain tables
 *      (as parsed data structures, rather than as raw TrueType tables).
 *      This table information is already used by iType internally - we simply
 *      give the user's application access to the same information (in order
 *      to perform composition and layout operations, for example). 
 *
 *      The calling application is responsible for allocating a pointer to an
 *      area of the appropriate size, and freeing the memory when it is done: 
 *      iType will simply copy the requested information into that area.
 *
 *      An error will be returned if there is no current loaded font, or 
 *      if the requested table was not found.  
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      tag         - [in] 4-byte character tag for table to be retrieved:
 *                      one of TAG_head, TAG_hhea, TAG_vhea, TAG_maxp, TAG_OS2,
 *                      or TAG_name
 *      tableptr    - [out] pointer to area that will hold table information: 
 *                      allocated as one of sizeof(TTF_HEAD), sizeof(TTF_HHEA),
 *                      sizeof(TTF_VHEA), sizeof(TTF_MAXP), sizeof(TTF_OS2),  
 *                      or sizeof(TTF_NAME).
 *
 *  Return value:
 *      Error code
 *
 *    Notes:
 *      The tables that can currently be retrieved are: 
 *          head, hhea, vhea, maxp, OS/2, and name.
 * 
 *      The information in the name-table structure is not a complete copy of
 *      the TrueType name table; rather, it is the portion of the table that
 *      is used by iType. If you need access to the complete name table, you
 *      will need to load the table via FS_get_table(), and parse it yourself.
 *
 *    See Also:
 *      FS_get_table()
 *      
 *  Version:
 *      Introduced in iType 2.4. 
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_LONG FS_get_table_structure(_DS_ FS_ULONG tag, FS_VOID *tableptr);


/*****************************************************************************
 *
 *  Get effective minimum and maximum Y values
 *
 *  Description:
 *      The API function FS_get_vdmx_pair() will get the ymin and ymax
 *      pair, if any exists, from the VDMX table in the current font at the
 *      current scale. These values are the highest ascender and 
 *      lowest descender that will be returned for any glyph.
 *
 *      Positive values for yMax and yMin mean that the value
 *      lies above the baseline. Negative values are below the
 *      baseline. 
 *    
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      yMax        - [out] pointer to maximum Y value obtained
 *      yMin        - [out] pointer to minimum Y value obtained
 *
 *  Return Value:
 *      SUCCESS, or error code if information could not be obtained.
 *
 *      <TABLE>
 *      Return values             Reason
 *      ------------------------  ----------------------------------
 *      ERR_NO_CURRENT_LFNT       No current loaded font
 *      ERR_NO_CURRENT_SFNT       No current scaled font
 *      ERR_TABLE_NOT_FOUND       The font does not have a VDMX table
 *      ERR_TABLE_UNSUPPORTED     The PFR format does not support VDMX information
 *      ERR_VDMX_RATIO            Current x/y aspect ratio is not in the table    
 *      ERR_yPelHeight_NOT_FOUND  Current y scaling (in pixels) is not in the table
 *      ERR_NOT_VANILLA           Current scaling is rotated or obliqued
 *      </TABLE>
 *
 *    Notes:
 *      Because values such as usWinAscent and usWinDescent can be inaccurate
 *      due to TrueType instructions causing rounding and hinting differences,
 *      the VDMX table was defined. This table, if it exists in a font, can 
 *      provide exact y-dimension max and min for a given size. The values
 *      returned are the same as if the user rendered each every glyph in the
 *      font at the current size and recorded the highest and lowest y-values.
 *      There can be multiple sets of data in the table for a single font, 
 *      so a match must be found based on the current scaling.
 *
 *      The values in the VDMX table are not guaranteed to be reliable if 
 *      rotation or obliquing is in effect, so an error code is returned
 *      if any of these are enabled when the VDMX query function is called.
 *
 *  Version:
 *      Introduced in iType 2.4. 
 *
 *  <GROUP getglyphinfo>
 */
ITYPE_API FS_LONG FS_get_vdmx_pair(_DS_ FS_SHORT *yMax, FS_SHORT *yMin);


/*****************************************************************************
 *
 *  Maps a character value through current cmap, returning a glyph index.
 *
 *  Description:
 *      This function is used to translate an input character id 
 *      (typically a Unicode value) into a glyph-index value. 
 *      The current font and current cmap are used to obtain the glyph-index
 *      value.
 *
 *      If a cmap entry for this id does not exist, it returns the
 *      'missing glyph index', which is always glyph index 0. This applies to
 *      linked fonts as well as regular fonts. 
 *
 *      This function is not affected by the status of the 
 *      FLAGS_CMAP_OFF/FLAGS_CMAP_ON flags.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value
 *
 *  Return Value:
 *      Glyph index.
 *
 *  <GROUP getglyphinfo>
 */
ITYPE_API FS_USHORT FS_map_char(_DS_ FS_ULONG id);

/*****************************************************************************
 *
 *  Maps a glyph index through current cmap, returning a Unicode value.
 *
 *  Description:
 *      This function is used to translate an input glyph index 
 *      into a Unicode value using the 3,1 or 3,10 platform,encoding scheme. 
 *      Currently, this function is for internal use only.
 *      The current font and current cmap are used to obtain the Unicode 
 *      value.
 *
 *      If a cmap entry for this index does not exist, it returns the
 *      value 0xFFFF.  This function is not 
 *      affected by the status of the FLAGS_CMAP_OFF/FLAGS_CMAP_ON flags.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      gi          - [in] glyph index
 *
 *  Return Value:
 *      Unicode id.
 *
 */
ITYPE_API FS_ULONG FS_inverse_map_char(_DS_ FS_ULONG gi);

/*****************************************************************************
 *
 *  Returns a selected glyph index variant for a Unicode ID if present in the font.
 *
 *  Description:
 *      This function is used to get glyph variations. A Unicode code point
 *      is followed by a variation selector in the following ranges
 *       U180B  -  U180D   Mongolian Free Variation Selectors
 *       UFE00  -  UFE0F   Variation Selectors (typically math symbols)
 *       UE0100 -  UE01EF  Variation Selectors Supplement (typically ideographic variations)
 *
 *      The default glyph index is stored in the Unicode cmap (usually either 3,1 or 3,10). 
 *      The variation selector specifies the variation to use based on the variant cmap
 *      in effect. See FS_set_cmap_variant() and FS_get_cmap_variant().
 *      Ideographic variations are typically stored in cmap 0,5. This is the iType default
 *      for the variant cmap.
 *
 *      If the variation selector specified is zero, this function will return the default
 *      glyph index for the Unicode code point specified by the ID.
 *
 *      If the variation selector specified is non-zero, this function will return
 *      the glyph index of the non-default variant if present, or zero if not found.
 *
 *      If the variant cmap is not present in the font, the function will return zero
 *      and set an error condition in the state. This error condition should be considered
 *      non-fatal and the default variant should be used.
 *
 *      If the variant cmap is present, but the variant is not found in the cmap, the 
 *      function will return a glyph index of zero. A return value of zero means that 
 *      you should use the default variant as returned by FS_map_char().
 *
 *      This function is not affected by the status of the 
 *      FLAGS_CMAP_OFF/FLAGS_CMAP_ON flags.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] Unicode code point value
 *      varSelector - [in] variation selector
 *
 *  Return Value:
 *      Glyph index of variant if non-default variant is present for selector.
 *      Otherwise zero (use default variant)
 *
 *  <GROUP getglyphinfo>
 */
ITYPE_API FS_USHORT FS_map_char_variant(_DS_ FS_ULONG id, FS_ULONG varSelector);

/*****************************************************************************
 *
 *  Returns the kerning adjustment for a character pair.
 *
 *  Description:
 *      Kerning is an adjustment to nominal glyph spacing that takes
 *      into account the character pair.  The <I>id</I> arguments are
 *      treated as character value if FLAGS_CMAP_ON is set (the default),
 *      and as glyph index if FLAGS_CMAP_OFF is set.
 *
 *      This kerning applies to horizontal text layout.  Assuming no text
 *      rotation, the <I>id1</I> argument is always the character to the left, 
 *      regardless of whether the text reads left-to-right or right-to-left.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id1         - [in] character value or glyph index of first character
 *      id2         - [in] character value or glyph index of second character
 *      _dx         - [out] returns x-direction kerning adjustment as 16.16 fixed point number 
 *      _dy         - [out] returns y-direction kerning adjustment as 16.16 fixed point number
 *
 *  Return Value:
 *      SUCCESS, or NULL upon error.
 *
 *  <GROUP getglyphinfo>
 */
ITYPE_API FS_LONG FS_get_kerning(_DS_ FS_ULONG id1, FS_ULONG id2, FS_FIXED *_dx, FS_FIXED *_dy);


/*****************************************************************************
 *
 *  Resizes a block of memory within the iType heap.
 *      
 *  Description:
 *      This function resizes a block of memory that was obtained
 *      using a previous call to FS_malloc(), FS_calloc(), or FS_resize().
 *
 *      If this function is successful, the contents of the old block
 *      are copied to the new memory block.  If the function fails
 *      then the old pointer is still valid.
 *
 *  Notes:     
 *      You must use FS_free() to release this memory.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      p           - [in] pointer to existing block of memory
 *      n           - [in] new size in bytes
 *
 *  Return Value:
 *      Pointer to a block of memory, or NULL if the new block
 *      could not be allocated.
 *
 *  <GROUP fsmemmgr>
 */
ITYPE_API FS_VOID *FS_realloc(_DS_ FS_VOID * p, FS_ULONG n);


/*****************************************************************************
 *
 *  Frees a block of memory within the iType heap.
 *      
 *  Description:
 *      This function frees a block of memory that was obtained
 *      using a previous call to FS_malloc(), FS_calloc(), or FS_resize().
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      p           - [in] pointer to memory block
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  <GROUP fsmemmgr>
 */
ITYPE_API FS_LONG FS_free(_DS_ FS_VOID *p);


/*****************************************************************************
 *
 *  Returns a reference to a font table.
 *
 *  Description:
 *      When <I>mode</I> is defined as TBL_EXTRACT
 *      this function allows the user to retrieve the raw data of any 
 *      TrueType table, specified by <I>tag</I>, from the current font.
 *      The function returns a reference to the table, not a copy.
 *      If the return value is NULL then there was an error.  Use
 *      FS_error() to determine what went wrong.
 *
 *      When finished, the user must release the reference using
 *      FS_free_table().  Do not use FS_free.
 *
 *      When <I>mode</I> is defined as TBL_QUERY
 *      this function allows the user to query whether a 
 *      TrueType table, specified by <I>tag</I>, exists in the 
 *      current font.  The function returns '1' if the table exists,
 *      else it returns '0'.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      tag         - [in] 4-byte character tag value
 *      mode        - [in] indicates mode of this call 
 *      len         - [out] pointer to size of table in bytes
 *
 *  Return Value:
 *      Reference to a font table, or boolean value.
 *
 *    See Also:
 *      FS_get_table_structure()
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_BYTE *FS_get_table(_DS_ FS_ULONG tag, FS_BYTE mode, FS_ULONG *len);


/*****************************************************************************
 *
 *  Frees a font table reference.
 *
 *  Description:
 *      This function frees a table reference that was returned by 
 *      FS_font_table() using the TBL_EXTRACT mode.
 *      
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      p           - [in] pointer to font table
 *
 *  Return Value:
 *      SUCCESS, or error code.
 *
 *  <GROUP fontinfo>
 */
ITYPE_API FS_LONG FS_free_table(_DS_ FS_VOID *p);


/*****************************************************************************
 *
 *  Constructs a new iType client.
 *
 *  Description:
 *      This function constructs a new iType client.  You must pass
 *      it a pointer to a valid, previously initialized client.
 *      That "seed" client may have come from a previous call to FS_init()
 *      or FS_new_client().
 *
 *      This function allocates a new client (FS_STATE) structure using iType heap
 *      memory, initializes the new client data, attaches to the
 *      same iType server as used by the "seed" client, and returns a pointer
 *      to the new client.  This function must be matched with a call to
 *      the destructor, FS_end_client().
 *
 *      Beginning with iType 4.0, this function is always available and does not
 *      require that FS_MULTI_THREAD be defined. It can be used to support multiple 
 *      threads with shared data.  It is also useful within a single thread, when 
 *      the client application has an object model that benefits from each graphics 
 *      object having its own iType client, while sharing the same server data.
 *
 *  Parameters:
 *      state      - [in] pointer to a valid, previously initialized FS_STATE structure
 *      heapsize   - [in] the increase in heap_size recommended by this client
 *
 *  Return value:
 *      Pointer to the new client, an FS_STATE structure.
 *
 *  Notes:
 *      This function is always available and does not require FS_MULTI_THREAD be defined.
 *
 *      The heap size parameter is the recommended increase in the server's current
 *      heap size.  The function adds this value to the current heap size and attempts
 *      to resize the heap to the total.  In most cases, this value should be 0, because
 *      the contents of the heap are shared among the various clients.
 *
 *  See also:
 *      FS_end_client(), FS_init(), FS_STATE
 *
 *  Version:
 *      Introduced in iType 2.3. Available in single-threaded mode in iType 4.0.
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_STATE *FS_new_client(_DS_ FS_ULONG heapsize);


/*****************************************************************************
 *
 *  Destroys an iType client.
 *
 *  Description:
 *      This function is the destructor for iType clients that were 
 *      constructed using FS_new_client().
 *
 *      Before calling this function, all glyph and table references that
 *      were returned by this client, must be freed using FS_free_char() 
 *      and FS_free_table(), respectively.  If these references are not
 *      freed, the memory associated with them will remain un-available
 *      until the iType is completely exited using FS_exit().
 *
 *      Once this function is called, the state pointer is invalid.  If it
 *      is to remain in scope, it should be assigned to NULL.
 *
 *  Parameters:
 *      state     - [in] pointer to the FS_STATE structure to be destroyed
 *
 *  Return value:
 *      SUCCESS, or an iType error code if the engine could not be destroyed.
 *
 *  Notes:
 *      This function is always available and does not require FS_MULTI_THREAD be defined.
 *
 *  See also:
 *      FS_new_client(), FS_init(), FS_STATE
 *
 *  Version:
 *      Introduced in iType 2.3. Available in single-threaded mode in iType 4.0.
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_ULONG FS_end_client(_DS0_);


/*****************************************************************************
 *
 *  Filters and returns all font names available to client.
 *
 *  Description:
 *      Provide a list of fonts available to the current client. Arguments
 *      specify style and typeface properties of the font named to be returned.
 *
 *      This function will return only those fonts available to the client
 *      and its chain of client parentage.
 * 
 *      This function returns a list of font names whose fonts match the
 *      specified style and typeface properties.  The font names will be
 *      returned in a FILECHAR buffer (string) containing a comma-separated
 *      list of font names. The string will be NULL-terminated and will not
 *      exceed the length specified upon return.  The list string will be
 *      allocated using system memory management and the caller will be
 *      responsible for freeing this memory.
 *
 *      The value to which the parameter "list" points will be set to the
 *      allocated buffer with the returned list.
 *
 *      The value to which the parameter "lstlen" points will be set to the
 *      overall length of the buffer (which may be greater than the string
 *      length (strlen) of the buffer).
 *
 *      The returned list can be separated by the caller and the individual
 *      font names can be used in FS_set_font(). The function will return
 *      only those font names that the current client has added or were
 *      added by any one of the current clients parent clients. Fonts added
 *      by other clients who are not parents of the current client will not
 *      be returned.
 *
 *  Parameters:
 *      state      - [in] pointer to the FS_STATE structure
 *      style      - [in] FS_STYLE enumerator value
 *      face       - [in] FS_FACE enumerator value
 *      list       - [in] pointer to a FILECHAR pointer to be set upon return
 *      lstlen     - [in] pointer to an FS_ULONG to be set up return
 *
 *  Return value:
 *      SUCCESS, or an iType error code.
 *
 *  Version:
 *      Introduced in iType 4.0.
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_LONG FS_get_available_fonts(_DS_ FS_STYLE style, FS_FACE face, FILECHAR **list, FS_ULONG *lstlen);

/*****************************************************************************
 *
 *  Set an external outline for rendering.
 *
 *  Description:
 *      Allows the caller to specify an externally generated outline so that
 *      iType will use that outline when rendering glyphs. Once an outline has 
 *      been set, it remains in effect until a new outline has been set or a 
 *      NULL outline has been specified. 
 *
 *      When an external outline has been set, any FS_get_glyphmap(), FS_get_bitmap(),
 *      FS_get_graymap(), or FS_get_phased() call will return a glyph rendered using
 *      the specified outline regardless of the glyph ID specified in the call. 
 *      FS_get_outline() will return the specified outline data. 
 *
 *      Either scaled or unscaled outlines may be specified. The doScaling parameter
 *      controls whether iType applies scaling to the outline. 
 *
 *      If doScaling is false, the outline is assumed to have been scaled externally 
 *      and no additional scaling is applied. The unitsPerEm parameter is ignored. 
 *
 *      if doScaling is true, iType will apply its current scale transformation (set
 *      by FS_set_scale()) to the outline. In this case, you must specify a unitsPerEm
 *      value that will be used by iType during scaling. 
 *
 *  Parameters:
 *      state      - [in] pointer to the FS_STATE structure
 *      outl       - [in] outline data in FS_OUTLINE form
 *      doScaling  - [in] boolean flag to indicate whether scaling should be 
 *                          applied to the outline
 *      unitsPerEm - [in] if doScaling is TRUE, the font units per EM to be used during
 *                          scaling
 *
 *  Return value:
 *      SUCCESS, or an iType error code.
 *
 *  Version:
 *      Introduced in iType 4.0.
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_LONG FS_set_outline(_DS_ FS_OUTLINE *outl, FS_BOOLEAN doScaling, FS_USHORT unitsPerEm);

/*****************************************************************************
 *
 *  Obtains a block of memory from the iType heap and
 *  initializes it to zeros.
 *      
 *  Notes:
 *      You must use FS_free() to release this memory.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      n           - [in] size of block to allocate, in bytes
 *
 *  Return Value:
 *      Pointer to a block of memory, or NULL if error.
 *
 *  <GROUP fsmemmgr>
 */
ITYPE_API FS_VOID *FS_calloc(_DS_ FS_ULONG n);


/*****************************************************************************
 *
 *  Obtains a block of memory from the iType heap.
 *      
 *  Notes:
 *      You must use FS_free() to release this memory.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      n           - [in] size of block to allocate, in bytes
 *
 *  Return Value:
 *      Pointer to a block of memory, or NULL on error.
 *
 *  <GROUP fsmemmgr>
 */
ITYPE_API FS_VOID *FS_malloc(_DS_ FS_ULONG n);

#else  /* not FS_MULTI_THREAD */

/*****************************************************************************
 *
 *  Constructs an iType server and uses externally allocated memory
 *
 *  Description:
 *      This function should generally be called only once per server, 
 *      upon startup of whichever system controls iType.
 *      It is identical to FS_init() except that the user must allocate
 *      a block of memory that will serve as iType's internal heap.
 *
 *      This function is only available if FS_INT_MEM is defined AND
 *      FS_MULTI_THREAD and FS_MULTI_PROCESS are not defined.
 *
 *      FS_init_ex does the following:
 *      - validates the basic data types defined in port.h
 *      - validates certain configuration options defined in port.h
 *      - constructs and initializes the server
 *      - initializes the master client's data, stored in FS_STATE
 *
 *      The caller must provide a contiguous buffer (sysheap) of heap_size bytes.
 *      This buffer will be managed by iType's internal memory manager as a heap.
 *
 *      The heap is used to store data that is shared among clients:
 *      - list of loaded fonts
 *      - loaded font data
 *      - scaled font data and cache
 *      - glyph cache
 *       
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      sysheap     - [in] pointer to external memory block
 *      heap_size   - [in] the maximum amount of RAM, in bytes, that this 
 *                         iType engine will use for its heap
 *
 *  Return value:
 *      SUCCESS, or an error code if the engine could not be initialized.
 *
 *  Notes:
 *
 *      FS_INT_MEM must be defined. The external memory block provided through
 *      the sysheap argument must consist of at least heap_size contiguous bytes.
 *
 *      It is an unchecked run-time error to call FS_init_ex() twice for the same
 *      FS_STATE structure without first having called FS_exit.
 *
 *  See also:
 *      FS_init(), FS_exit(), FS_new_client(), FS_end_client(), FS_STATE
 *  
 *  <GROUP admin>
 */
ITYPE_API FS_LONG FS_init_ex(_DS_ FS_VOID *sysheap, FS_ULONG heap_size);

#endif /* FS_MULTI_THREAD */

/* For those API functions that require mutex protection when FS_MULTI_THREAD   */
/* is defined, two versions of the function exist - an FSS_ version without     */
/* mutex protection and an FS_ version with mutex protection. The FS_ version   */
/* calls the FSS_ version after setting the mutex lock.                         */
/* When the code is built without FS_MULTI_THREAD defined, the FS_ version is   */
/* defined to be the FSS_ version since no mutex lock is required.              */

#ifdef FS_MULTI_THREAD
/* multi-threaded API calls - the FSS_ version is not exported */
#define EXPORT_ITYPE_FSS  
#else
/* single threaded API calls - the FSS_ version is exported via ITYPE_API */
#define EXPORT_ITYPE_FSS ITYPE_API
#endif

/* define non-mutex versions of API functions if used, see also FS_ definitions     */
/* for multithreaded builds, these functions are not exported via EXPORT_ITYPE_FSS  */
EXPORT_ITYPE_FSS FS_LONG FSS_init(_DS_ FS_ULONG heapsize);
EXPORT_ITYPE_FSS FS_LONG FSS_set_font(_DS_ FILECHAR *name);
EXPORT_ITYPE_FSS FS_LONG FSS_set_scale(_DS_ FS_FIXED s00,FS_FIXED s01,FS_FIXED s10,FS_FIXED s11);
EXPORT_ITYPE_FSS FS_LONG FSS_get_scale(_DS_ FS_FIXED *s00,FS_FIXED *s01,FS_FIXED *s10,FS_FIXED *s11);
EXPORT_ITYPE_FSS FS_USHORT FSS_get_design_units(_DS0_);
EXPORT_ITYPE_FSS FS_LONG FSS_set_cmap(_DS_ FS_USHORT platform, FS_USHORT encoding);
EXPORT_ITYPE_FSS FS_LONG FSS_get_cmap(_DS_ FS_USHORT *platform, FS_USHORT *encoding);
EXPORT_ITYPE_FSS FS_OUTLINE *FSS_get_outline(_DS_ FS_ULONG id);
EXPORT_ITYPE_FSS FS_LONG FSS_get_gpos_scale(_DS_ FS_USHORT index, FS_USHORT *du,
                                            FS_FIXED *s00, FS_FIXED *s01, FS_FIXED *s10, FS_FIXED *s11,
                                            FS_FIXED *xppm, FS_FIXED *yppm,
                                            FS_USHORT *idRangeStart, FS_USHORT *idRangeEnd);
EXPORT_ITYPE_FSS FS_LONG FSS_get_gpos_pts(_DS_ FS_USHORT index, FS_USHORT num, FS_USHORT *pts, FS_LONG *x, FS_LONG *y);
EXPORT_ITYPE_FSS FS_GLYPHMAP *FSS_get_glyphmap(_DS_ FS_ULONG id, FS_USHORT type);
EXPORT_ITYPE_FSS FS_BITMAP *FSS_get_bitmap(_DS_ FS_ULONG id);
EXPORT_ITYPE_FSS FS_GRAYMAP *FSS_get_graymap(_DS_ FS_ULONG id);
EXPORT_ITYPE_FSS FS_GRAYMAP *FSS_get_phased(_DS_ FS_ULONG id, FS_FIXED xpos);
EXPORT_ITYPE_FSS FS_LONG FSS_free_char(_DS_ FS_VOID *p);
EXPORT_ITYPE_FSS FS_LONG FSS_resize(_DS_ FS_ULONG newsize);
EXPORT_ITYPE_FSS FS_LONG FSS_exit(_DS0_);
EXPORT_ITYPE_FSS FS_LONG FSS_add_font(_DS_ FILECHAR *name, FILECHAR *path, FS_BYTE *memptr, FS_ULONG index);
EXPORT_ITYPE_FSS FS_LONG FSS_add_font_with_offset(_DS_ FILECHAR *name, FILECHAR *path, FS_BYTE *memptr, FS_ULONG index,
                                           FS_ULONG data_offset, FS_ULONG data_length);
EXPORT_ITYPE_FSS FS_LONG FSS_font_name(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len, FILECHAR *name);
EXPORT_ITYPE_FSS FS_LONG FSS_font_name_with_offset(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len, FILECHAR *name,
                                            FS_ULONG data_offset, FS_ULONG data_length);
EXPORT_ITYPE_FSS FS_LONG FSS_load_font(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len, FILECHAR *name);
EXPORT_ITYPE_FSS FS_LONG FSS_load_font_with_offset(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG name_len,
                                            FILECHAR *name, FS_ULONG data_offset, FS_ULONG data_length);
EXPORT_ITYPE_FSS FS_LONG FSS_get_font_path(_DS_ FILECHAR *name,
                                    FS_BYTE **memptr_p,
                                    FILECHAR *path_buffer, FS_ULONG buffer_length,
                                    FS_ULONG *index,
                                    FS_ULONG *offset_p,
                                    FS_ULONG *length_p);
EXPORT_ITYPE_FSS FS_LONG FSS_get_link_info(_DS_ FILECHAR *path, FS_BYTE *memptr, FsLtt *info);
EXPORT_ITYPE_FSS FS_LONG FSS_get_name(_DS_ FS_ULONG name_len, FILECHAR *name);
EXPORT_ITYPE_FSS FS_LONG FSS_delete_font(_DS_ FILECHAR *name); 
EXPORT_ITYPE_FSS FS_LONG FSS_get_advance(_DS_ FS_ULONG id, FS_USHORT type, FS_SHORT *i_dx, FS_SHORT *i_dy, FS_FIXED *dx, FS_FIXED *dy);
EXPORT_ITYPE_FSS FS_LONG FSS_get_glyph_metrics(_DS_ FS_USHORT index, FS_SHORT *lsb, FS_SHORT *aw, FS_SHORT *tsb, FS_SHORT *ah);
EXPORT_ITYPE_FSS FS_LONG FSS_font_metrics(_DS_ FONT_METRICS *fm);
EXPORT_ITYPE_FSS FS_LONG FSS_get_ascender_descender_leading(_DS_ FS_FIXED *ascender, FS_FIXED *descender, FS_FIXED *leading, FsAscDescLeadSource *source);
EXPORT_ITYPE_FSS FS_LONG FSS_get_table_structure(_DS_ FS_ULONG tag, FS_VOID *tableptr);
EXPORT_ITYPE_FSS FS_LONG FSS_get_vdmx_pair(_DS_ FS_SHORT *yMax, FS_SHORT *yMin);
EXPORT_ITYPE_FSS FS_USHORT FSS_get_ppem_size(_DS_ FS_USHORT pixel_height, FS_USHORT max_ppem);
EXPORT_ITYPE_FSS FS_USHORT FSS_map_char(_DS_ FS_ULONG id);
EXPORT_ITYPE_FSS FS_ULONG FSS_inverse_map_char(_DS_ FS_ULONG gi);
EXPORT_ITYPE_FSS FS_USHORT FSS_map_char_variant(_DS_ FS_ULONG id, FS_ULONG varSelector);
EXPORT_ITYPE_FSS FS_LONG FSS_get_kerning(_DS_ FS_ULONG id1, FS_ULONG id2, FS_FIXED *_dx, FS_FIXED *_dy);
EXPORT_ITYPE_FSS FS_BYTE *FSS_get_table(_DS_ FS_ULONG tag, FS_BYTE mode, FS_ULONG *len);
EXPORT_ITYPE_FSS FS_LONG FSS_free_table(_DS_ FS_VOID *p);
EXPORT_ITYPE_FSS FS_STATE *FSS_new_client(_DS_ FS_ULONG heapsize);
EXPORT_ITYPE_FSS FS_ULONG FSS_end_client(_DS0_);
EXPORT_ITYPE_FSS FS_LONG FSS_get_available_fonts(_DS_ FS_STYLE, FS_FACE, FILECHAR **, FS_ULONG *);
EXPORT_ITYPE_FSS FS_LONG FSS_set_outline(_DS_ FS_OUTLINE *outl, FS_BOOLEAN doScaling, FS_USHORT unitsPerEm);
EXPORT_ITYPE_FSS FS_VOID *FSS_calloc(_DS_ FS_ULONG n);
EXPORT_ITYPE_FSS FS_VOID *FSS_malloc(_DS_ FS_ULONG n);
EXPORT_ITYPE_FSS FS_VOID *FSS_realloc(_DS_ FS_VOID *p, FS_ULONG n);
EXPORT_ITYPE_FSS FS_LONG FSS_free(_DS_ FS_VOID * p);

/* for single threaded builds, the FS_ version is the same as the FSS_ version */
#ifndef FS_MULTI_THREAD

#define FS_init FSS_init
#define FS_set_font FSS_set_font
#define FS_set_scale FSS_set_scale
#define FS_get_scale FSS_get_scale
#define FS_get_design_units FSS_get_design_units
#define FS_set_cmap FSS_set_cmap
#define FS_get_cmap FSS_get_cmap
#define FS_get_outline FSS_get_outline
#define FS_get_gpos_scale FSS_get_gpos_scale
#define FS_get_gpos_pts FSS_get_gpos_pts
#define FS_get_glyphmap FSS_get_glyphmap
#define FS_get_bitmap FSS_get_bitmap
#define FS_get_graymap FSS_get_graymap
#define FS_get_phased FSS_get_phased
#define FS_free_char FSS_free_char
#define FS_resize FSS_resize
#define FS_exit FSS_exit
#define FS_add_font FSS_add_font
#define FS_add_font_with_offset FSS_add_font_with_offset
#define FS_font_name FSS_font_name
#define FS_font_name_with_offset FSS_font_name_with_offset
#define FS_get_name FSS_get_name
#define FS_get_link_info FSS_get_link_info
#define FS_load_font FSS_load_font
#define FS_load_font_with_offset FSS_load_font_with_offset
#define FS_get_font_path FSS_get_font_path
#define FS_delete_font FSS_delete_font
#define FS_get_advance FSS_get_advance
#define FS_get_glyph_metrics FSS_get_glyph_metrics
#define FS_font_metrics FSS_font_metrics
#define FS_get_ascender_descender_leading FSS_get_ascender_descender_leading
#define FS_get_vdmx_pair FSS_get_vdmx_pair
#define FS_get_ppem_size FSS_get_ppem_size
#define FS_get_table_structure FSS_get_table_structure
#define FS_map_char FSS_map_char
#define FS_map_char_variant FSS_map_char_variant
#define FS_inverse_map_char FSS_inverse_map_char
#define FS_get_kerning FSS_get_kerning
#define FS_get_table FSS_get_table
#define FS_free_table FSS_free_table
#define FS_new_client FSS_new_client
#define FS_end_client FSS_end_client
#define FS_get_available_fonts FSS_get_available_fonts
#define FS_set_outline FSS_set_outline
#define FS_calloc FSS_calloc
#define FS_malloc FSS_malloc
#define FS_realloc FSS_realloc
#define FS_free FSS_free

#endif /* NOT FS_MULTI_THREAD */


/* Multi-process specific API function definitions */

#ifdef FS_MULTI_PROCESS
#if defined(_WIN32) && !defined(__MARM_ARMI__)
FS_LONG getpagesize();
#endif
FS_LONG FS_create_shared_mem(_DS_ FS_LONG size, FILECHAR *name);
FS_VOID *FS_attach_shared_mem(_DS_ FS_LONG hMapObject, FS_LONG size);
FS_LONG FS_detach_shared_mem(_DS_ FS_VOID *shared_mem, FS_LONG size);
FS_VOID FS_delete_shared_mem(_DS_ FS_LONG hMapObject);
FS_LONG FS_open_shared_mem(_DS_ FILECHAR *name);
#endif

#if defined(FS_MULTI_PROCESS) || defined(FS_MAPPED_FONTS)
/* the following defined in fs_mapped.c */
FS_BYTE *MF_get_mapped_font(_DS_ FILECHAR *path);
FS_VOID  MF_delete_mapped_font(_DS_ FILECHAR *path);

/* the following defined in port-specific file -- fs_mfont.c */
FS_BYTE *MF_map_font(_DS_ MAPPEDFNT *mappedfnt);
FS_VOID  MF_unmap_font(MAPPEDFNT *mappedfnt);

#endif /* FS_MULTI_PROCESS */

/********************************************************************************/
/* Private functions that are not really part of the api ... but they live here */
/********************************************************************************/

/* exports from "fs_api.c" */
int IDIV(int n,int d);
int IMOD(int n, int d);
FILECHAR *FS_strdup(_DS_ FILECHAR *s);
int FS_streq(FILECHAR *a, FILECHAR *b);
FS_OUTLINE *internal_user_get_outline(_DS_ FS_ULONG id);
FS_BYTE fontfile_type(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG data_offset);
void get_ref_lines(_DS0_);

/* exports from "fs_autohint.c" */    
#ifdef FS_STIK
FS_SHORT autohint_stik(_DS_ FS_OUTLINE *stik, FS_FIXED sw);
FS_VOID autohint_other (fnt_LocalGraphicStateType *gs, FS_BOOLEAN int_sw); 
FS_VOID autohint_latin (fnt_LocalGraphicStateType *gs, FS_BOOLEAN int_sw); 
FS_VOID autohint_mono  (fnt_LocalGraphicStateType *gs, FS_BOOLEAN int_sw);
FS_VOID autohint_arabic(fnt_LocalGraphicStateType *gs, FS_BOOLEAN int_sw);
#endif /* FS_STIK */

FS_LONG fix_fnt(_DS_ LFNT *lfnt);
FS_LONG load_fnt(_DS_ LFNT *lfnt);
FS_VOID unload_fnt(_DS_ LFNT *lfnt);
FS_VOID unload_cmap_cache(_DS_ LFNT *lfnt);
#ifdef GRID_DEBUG
FS_VOID dump_hints(int n, F26DOT6 *old, F26DOT6 *new);
#endif

/* exports from "fs_xmalloc.c" */
FS_VOID delete_lfnt(_DS_ LFNT *lfnt);
FS_VOID kill_fntset(_DS_ FNTSET *fntset);
int get_some_back(_DS0_);

/* exports from "fs_bestfit.c" */
#ifdef FS_INT_MEM
FS_ULONG __init_heap(FS_STATE *sp, FS_ULONG heapsize, FS_VOID *sys_heap);
void __term_heap(FS_STATE *sp);
FS_VOID * fs__malloc(_DS_ FS_ULONG bytes);
FS_LONG fs__free(_DS_ FS_VOID *ptr);
FS_VOID * fs__realloc(_DS_ FS_VOID *ptr, FS_ULONG n);
#endif


#ifndef FS_NO_FS_LUMP
CACHE_ENTRY *CACHE_ENTRY_calloc(_DS0_);
void CACHE_ENTRY_free(_DS_ CACHE_ENTRY *c);
#endif

/* exports from "fs_scale.c" */
FS_LONG scale_font(_DS_ SFNT *sfnt,FS_FIXED s00,FS_FIXED s01,FS_FIXED s10,FS_FIXED s11);
FS_LONG scale_font_ttf(_DS_ SFNT *sfnt,FS_FIXED s00,FS_FIXED s01,FS_FIXED s10,FS_FIXED s11);
FS_LONG modify_scale(FS_FIXED *scale, FS_FIXED tan_i_a);
FS_LONG get_scale_inputs(FS_FIXED *scale,FS_FIXED *xppm,FS_FIXED *yppm,FS_FIXED *tan_s);
FS_LONG check_sfnt(_DS0_);

/* exports from "fs_ttf.c" */
TTF *load_ttf(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_ULONG data_offset, FS_ULONG data_length, FS_BOOLEAN validate);
FS_VOID unload_ttf(_DS_ TTF *ttf);
FS_VOID *ttf_read(_DS_ TTF *ttf, FS_ULONG offset, FS_ULONG size);
FS_VOID ttf_read_buf(_DS_ TTF *ttf, FS_ULONG offset, FS_ULONG size, FS_BYTE *buf);
FS_VOID *get_ttf_table(_DS_ TTF *ttf, FS_ULONG tag);
int get_ttf_table_info(_DS_ TTF *ttf, FS_ULONG tag, FS_ULONG *off, FS_ULONG *len);
FS_VOID swap_ttc_header(TTC_HEADER *p);
FS_VOID swap_ttf_header(TTF_HEADER *p);
FS_VOID swap_head(TTF_HEAD *p);
FS_VOID swap_hhea(TTF_HHEA *p);
FS_VOID swap_vhea(TTF_VHEA *p);
FS_VOID swap_maxp(TTF_MAXP *p);
TTF_OS2 *get_os2_table(_DS_ TTF *ttf);
FS_VOID get_os2(TTF_OS2 *p, FS_BYTE *raw);
TTF_NAME *get_abbreviated_name(_DS_ FS_BYTE *p);
TTF_NAME *get_name(_DS_ TTF *ttf);
FS_VOID ptrs_to_offsets(_DS_ TTF *ttf);

#ifdef FS_CFFR
/* exports from "fs_cff.c" */
CFF_TAB *load_cff(_DS_  TTF *ttf);
void unload_cff(_DS_ CFF_TAB *cff);
#endif 

/* exports from "fs_fio.c" */
FS_FILE *FS_open(_DS_ FILECHAR *path);
FS_VOID FS_close(_DS_ FS_FILE *fp);
FS_LONG FS_seek(_DS_ FS_FILE *fp, FS_ULONG offset, int whence);
FS_ULONG FS_read(_DS_ FS_FILE *fp, FS_BYTE *buf, FS_ULONG size);

/* exports from "fs_outline.c" */
#ifdef FS_RENDER
FS_OUTLINE *setup_outline(_DS_ FS_SHORT nc, FS_LONG n_types, FS_LONG n_points);
FS_OUTLINE *make_outline(_DS_ SFNT *sfnt, FS_ULONG index);
FS_OUTLINE *make_outline_ttf(_DS_ SFNT *sfnt, FS_ULONG index);
FS_OUTLINE *copy_outline(_DS_ FS_OUTLINE *outl, FS_BOOLEAN user_mem);
FS_OUTLINE *find_or_make_outline(_DS_ LFNT *lfnt, SFNT *sfnt,
                                 FS_ULONG id, FS_ULONG index);
FS_OUTLINE *internal_get_outline(_DS_ FS_ULONG id);
#ifdef FS_PSEUDO_BOLD
FS_OUTLINE *embolden_outline(_DS_ FS_OUTLINE *outl,FS_LONG *n_types, FS_LONG *n_points); 
#endif
#endif /* FS_RENDER */
#ifdef FS_EDGE_RENDER
FS_GRAYMAP* make_ADF_graymap(_DS_ FS_OUTLINE *outl, FS_ULONG id, FS_USHORT index,FS_USHORT type);
#endif /* FS_EDGE_RENDER */



/* exports from "fs_icon.c" */
#ifdef FS_ICONS
FS_GLYPHMAP *get_icon(_DS_ FS_USHORT id, FS_USHORT type);
#endif /* FS_ICONS */

/* exports from "fs_cache.c" */
#ifdef FS_CACHE
FS_VOID delete_fs_cache(_DS_ SFNT *sfnt);
FS_VOID *find_in_cache(_DS_ FS_USHORT type, FS_ULONG index);
FS_OUTLINE* find_outline_in_cache(_DS_ FS_ULONG index);
FS_BITMAP*  find_bitmap_in_cache(_DS_ FS_ULONG index);
FS_GRAYMAP* find_graymap_in_cache(_DS_ FS_ULONG index, FS_USHORT cache_type);
FS_ADVANCE* find_advance_in_cache(_DS_ FS_ULONG index, FS_USHORT type);
FS_GRAYMAP* find_phased_in_cache(_DS_ FS_ULONG index, FS_SHORT phase);
FS_LONG save_to_cache(_DS_ FS_USHORT type, FS_ULONG index, FS_BYTE *p);
FS_LONG save_outline_to_cache(_DS_ FS_ULONG index, FS_OUTLINE *p);
FS_LONG save_bitmap_to_cache(_DS_ FS_ULONG index, FS_BITMAP *p);
FS_LONG save_graymap_to_cache(_DS_ FS_ULONG index, FS_GRAYMAP *p, FS_USHORT cache_type);
FS_LONG save_advance_to_cache(_DS_ FS_ULONG index, FS_USHORT type, FS_ADVANCE *p);
FS_LONG save_phased_to_cache(_DS_ FS_ULONG index, FS_SHORT phase, FS_GRAYMAP *p);
#ifdef FS_DUMP
FS_VOID dump_cache(_DS_ FILECHAR *s, CACHE_ENTRY **cache, FS_BOOLEAN refd);
#endif
#if defined(FS_OPENVG) && defined(FS_CACHE_OPENVG)
FS_VGGLYPH* find_openvg_in_cache(_DS_ FS_ULONG index, FS_USHORT cache_type);
FS_LONG save_openvg_to_cache(_DS_ FS_ULONG index, FS_VGGLYPH *p, FS_USHORT cache_type);
#endif
#endif /* FS_CACHE */

/* exports from "fs_sbit.c" */
FS_BITMAP *get_embedded_bitmap(_DS_ SFNT *sfnt, FS_USHORT gIndex);
FS_GRAYMAP *get_embedded_graymap(_DS_ SFNT *sfnt, FS_USHORT gIndex, FS_USHORT type);
#ifdef FS_EMBEDDED_BITMAP
TTF_EBLC_PARTIAL *get_EBLC_partial(_DS_ TTF *ttf);
#endif    /* FS_EMBEDDED_BITMAP */

/* exports from "fs_cmap.c" */
FS_LONG map_font(_DS_ FS_USHORT platform, FS_USHORT encoding);
FS_USHORT map_char(_DS_ FS_ULONG id, FS_ULONG varSelector);
FS_USHORT LFNT_map_char(_DS_ LFNT *lfnt, FS_ULONG cmap_offset, FS_ULONG id, FS_ULONG varSelector);
FS_ULONG inverse_map_char(_DS_ FS_ULONG gi);

/* exports from "fs_glue.c" */
FS_VOID delete_key(_DS_ FS_VOID * p);
#ifdef FS_RENDER
FS_VOID get_glyph_metrics(_DS_ fsg_SplineKey *key, FS_USHORT gIndex, fsg_Metrics *metrics);
int get_dropout_control(fsg_SplineKey *key);
FS_VOID get_glyph_parms(_DS_ TTF *ttf, FS_USHORT gIndex, FS_ULONG *offset, FS_ULONG *length);
FS_BYTE get_nstk_bit(_DS_ FS_USHORT index);
FS_VOID get_glyph_advance(_DS_ fsg_SplineKey *key, FS_USHORT gIndex, 
                          FS_FIXED *dx, FS_FIXED *dy);
#endif /* FS_RENDER */
#ifdef FS_HINTS
int which_script(_DS_ int index, FS_ULONG *unicode);
#endif /* FS_HINTS */
/* exports from "fs_stik.c" */
#ifdef FS_RENDER    
#ifdef FS_STIK
FS_OUTLINE *expand_stik(_DS_ FS_OUTLINE *stik, FS_LONG *_nt, FS_LONG *_np);
FS_OUTLINE *bitmap_expand_stik(_DS_ FS_OUTLINE *stik, FS_LONG *_nt, FS_LONG *_np);
FS_VOID draw_stik(_DS_ FS_OUTLINE *outl);
#endif
#endif    

/* exports from "fs_bitio.c" */
#if defined(FS_ACT3) || defined (FS_CCC)
BITIO *MTX_BITIO_Create( _DS_ FS_VOID* memPtr, FS_LONG memSize );
FS_ULONG MTX_BITIO_ReadValue( BITIO *t, FS_LONG numberOfBits );
FS_ULONG MTX_BITIO_Read32( BITIO *t);
FS_ULONG MTX_BITIO_Read8( BITIO *t);
#endif /* defined(FS_ACT3) || defined (FS_CCC) */

/* exports from "fs_tableptr.c" */
#ifdef DEBUG_TABLE_PTR
FS_VOID dump_table_ptrs(_DS_ FILECHAR *s);
#endif
FS_BOOLEAN any_used_fntset_or_unshared_lfnt_table_ptrs(_DS_ FNTSET *fntset);
FS_BOOLEAN any_used_lfnt_table_ptrs(_DS_ FNTSET *fntset, LFNT *lfnt);
int squeeze_table_ptrs(_DS_ FNTSET *fntset);
FS_VOID kill_table_ptrs(_DS0_);

#ifdef FS_PFRR
/* exports from "fs_pfr.c" */
FS_LONG scale_font_pfr(_DS_ SFNT *sfnt,FS_FIXED s00,FS_FIXED s01,FS_FIXED s10,FS_FIXED s11);
FS_VOID *load_pfr(_DS_ FILECHAR *path, FS_BYTE *memptr, FS_ULONG index, FS_FILE *fp);
FS_VOID unload_pfr(_DS_ FS_VOID *pfr);
FS_OUTLINE *make_outline_pfr(_DS_ SFNT *sfnt, FS_ULONG index);
FS_LONG pfr_font_metrics(_DS_ LFNT *lfnt, FONT_METRICS *fm);
#endif /* PFR */

/* exports from "fs_kern.c" */
FS_VOID delete_kern(_DS_ LFNT *lfnt);
FS_LONG load_kerning(_DS_ LFNT *lfnt);

/* exports from "fs_typeset.c" */
FS_ULONG TYPESET_init(_DS_ TYPESET *typeset, FNTSET *fntset);
FS_ULONG TYPESET_clear(_DS_ TYPESET *typeset);
FS_ULONG TYPESET_destroy(_DS_ TYPESET *typeset);

/* exports from "fs_fnt.c" */
#ifndef GLOBAL_STATIC_FUNCTION_PTRS_OK
FS_VOID fnt_initInterpFuncs( FS_VOID );
#endif

/* exports from "fs_rtgah.c" */
#ifdef FS_HINTS 
void rtgah(fnt_LocalGraphicStateType *gs); 
#endif

#ifdef __cplusplus
    }
#endif

#endif /* FS_FUNCTION_H */
