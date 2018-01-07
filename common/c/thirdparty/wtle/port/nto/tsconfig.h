/*****************************************************************************
 *
 *  tsconfig.h  - Defines build configuration values and options.
 *
 *  Copyright (C) 2002, 2007, 2009, 2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSCONFIG_H
#define TSCONFIG_H

#include "tstypes.h"

/*****************************************************************************
 *
 *  Build configuration option that will enable a new allocation strategy per
 *  request of RIM.  With this strategy, the number of lines will double
 *  when the allocation is exceeded, starting at a single line.
 *
 *  To enable this strategy OpenType processing, define RIM_ALTERNATE_ALLOCATION.
 *
 */
#define RIM_ALTERNATE_ALLOCATION


/*****************************************************************************
 *
 *  Build configuration option that specifies a build configuration string.
 *  Defaults to "Default".  May be redefined in order to give a name
 *  to a particular build.  If redefined then it must be as a string constant.
 *
 *  <GROUP tsconfig>
 */
#ifndef TS_CONFIG_STRING
#define TS_CONFIG_STRING "Default"
#endif


/*****************************************************************************
 *
 *  Build configuration option that defines the amount of RAM, in bytes,
 *  that the iType font rasterizer system initially defines as its heap.
 *  The iType heap is used for working memory, internal font data, and the
 *  glyph cache.
 *
 *  When iType uses external memory the heap is not a contiguous block,
 *  but instead is just an amount of memory that iType can use before it must
 *  automatically give some of it back.
 *
 *  It is recommended that you change this value as appropriate for your
 *  system requirements. It must be large enough to avoid iType cache
 *  thrashing, but cannot be so large as to consume too much of the system
 *  resources. Typical values are smaller than the ~4 MB value given below.
 *
 *  This value is only used for the iType font engine.
 *
 *  <GROUP tsconfig>
 */
#ifndef ITYPE_HEAP_SIZE
#define ITYPE_HEAP_SIZE   4000000
#endif


/*****************************************************************************
 *
 *  Build configuration option that enables the drawing of layout marks.
 *
 *  Description:
 *      If defined then the build includes code to draw layout marks
 *      such as bounding box, baseline, and advance box.
 *
 *      The marks are:
 *          - for diagnostic and instructional purposes
 *          - are not needed for normal layout
 *          - are drawn by TsLayout_display functions.
 *          - are only are only drawn if the corresponding run-time
 *            TsLayoutOptions are enabled
 *
 *      Undefining this macro will reduce code a small amount.
 *
 *  Notes:
 *      This option is not applicable to the WorldType Shaper product.
 *
 *  <GROUP tsconfig>
 */
//#define TS_ENABLE_LAYOUT_MARKS


/*****************************************************************************
 *
 *  Build configuration option that will reduce code requirements when
 *  the functionality is not necessary for a given deployment.
 *
 *  To eliminate OpenType processing, define TS_NO_OPENTYPE.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_NO_OPENTYPE */


/*****************************************************************************
 *
 *  Build configuration option that will reduce code requirements when
 *  the functionality is not necessary for a given deployment.
 *
 *  To eliminate Indic script shaping, define TS_NO_INDIC.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_NO_INDIC */


/*****************************************************************************
 *
 *  Build configuration option that will reduce code requirements when
 *  the functionality is not necessary for a given deployment.
 *
 *  To eliminate bidirectional processing, define TS_NO_BIDIRECTIONAL.
 *
 *  Note that Arabic and N'Ko are bidi-dependent scripts.
 *  If TS_NO_BIDIRECTIONAL is defined then Arabic and N'Ko shaping are
 *  disabled with the forced definition of TS_NO_ARABIC and TS_NO_NKO at
 *  the bottom of this file.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_NO_BIDIRECTIONAL */


/*****************************************************************************
 *
 *  Build configuration option that will reduce code requirements when
 *  the functionality is not necessary for a given deployment.
 *
 *  To eliminate Arabic script shaping, define TS_NO_ARABIC.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_NO_ARABIC */

/*****************************************************************************
 *
 *  Build configuration option that will reduce code requirements when
 *  the functionality is not necessary for a given deployment.
 *
 *  To eliminate N'Ko script shaping, define TS_NO_NKO.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_NO_NKO */

/*****************************************************************************
 *
 *  Build configuration option that will reduce code requirements when
 *  the functionality is not necessary for a given deployment.
 *
 *  To eliminate script normalization, define TS_NO_NORMALIZATION.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_NO_NORMALIZATION */


/*****************************************************************************
 *
 *  Build configuration option that affects the y-position of each
 *  baseline.
 *
 *  Define this option to obtain backward compatibility with the
 *  baseline's y-position prior to the correction introduced in WTLE 2.2.1.
 *  The baseline of each line will be positioned one pixel below the
 *  correct location with respect to coordinate system, advance box,
 *  selection rectangle, and background color rectangle.
 *
 *  Undefine this option to obtain the fix for WTL-496
 *  'Baseline is too low by one pixel'. Each baseline will be positioned
 *  correctly with respect to the coordinate system, advance box,
 *  selection rectangle, and background color rectangle.
 *
 *  We recommend that new users undefine this option.
 *  Existing users may wish to define this option initially to obtain
 *  backward compatibility and then undefine it when it is convenient
 *  to do so. The change will affect output by one pixel so regression
 *  test baselines will need to be updated and display position may
 *  need to adjusted.
 *
 *  Version:
 *      As of Version 3.0.0 this option is undefined by default.
 *      All reference images assume that this option is not defined.
 *
 *  Notes:
 *      This option is not applicable to the WorldType Shaper product.
 *
 *  <GROUP tsconfig>
 */
/* #define TS_ADJUST_BASELINE_FOR_BACKWARD_COMPATIBILITY_WITH_2_2_0 */


/*****************************************************************************
 *
 *  Arabic and N'Ko are bidi-dependent scripts. If TS_NO_BIDIRECTIONAL
 *  is defined then Arabic and N'Ko shaping must be disabled.
 *
 *  DO NOT EDIT THIS CODE.
 *
 *  <GROUP tsconfig>
 */
#if defined(TS_NO_BIDIRECTIONAL)
#define TS_NO_ARABIC
#define TS_NO_NKO
#endif


/*****************************************************************************
 *
 *  Indic shaping requires OpenType. If TS_NO_OPENTYPE
 *  is defined then Indic shaping must be disabled.
 *
 *  DO NOT EDIT THIS CODE.
 *
 *  <GROUP tsconfig>
 */
#if defined(TS_NO_OPENTYPE)
#define TS_NO_INDIC
#endif


/*****************************************************************************
 *
 *  Build configuration option to enable a cmap cache.
 *
 *  Description:
 *      If this option is defined then each TsFont will store a
 *      character map (cmap) cache.
 *
 *      Defining this option will improve performance. It will consume
 *      an extra 1024 bytes of RAM per TsFont.
 *
 *      iType has a similar caching mechanism. If WTLE is the primary iType
 *      client then it is recommended that the WTLE cache be enabled
 *      and that the iType cmap be disabled by undefining FS_CACHE_CMAP
 *      within iType's fsconfig.h file.
 *
 *      If iType has multiple clients then it is recommended that iType
 *      cmap cache be used and that the WTLE cmap be disabled by
 *      undefining TS_CACHE_MAP below.
 *
 *  Notes:
 *      This option is not applicable to the WorldType Shaper product.
 *
 *  <GROUP tsconfig>
 */
/* Disable since FS_CACHE_CMAP is defined in iType and iType has other clients
#ifndef TS_USE_CMAP_CACHE
#define TS_USE_CMAP_CACHE
#endif
*/


/*****************************************************************************
 *
 *  Build configuration option to enable a font metrics cache.
 *
 *  Description:
 *      If this option is defined then each TsFontStyle will store a
 *      font metrics cache.
 *
 *      Defining this option will improve performance, but more RAM
 *      is used. The exact cache size is compiler dependent -
 *      on the order of 2048 bytes of RAM per fontStyle.
 *
 *      The cache is not created when the baseline is rotated or if the
 *      the glyph size is 200 ppem or higher.
 *
 *  Notes:
 *      This option is not applicable to the WorldType Shaper product.
 *
 *  <GROUP tsconfig>
 */
#ifndef TS_USE_GLYPH_METRICS_CACHE
#define TS_USE_GLYPH_METRICS_CACHE
#endif


/*****************************************************************************
 *
 *  Build configuration option to enable an OpenType cache.
 *
 *  Description:
 *      If this option is defined then each TsFontStyle will declare a
 *      cache for OpenType processing results when appropriate.
 *
 *      Defining this option will improve performance when processing
 *      text with certain scripts and when using a font that has
 *      OpenType layout tables. Performance is improved significantly when
 *      using Indic scripts. This method is not that effective for Arabic.
 *
 *      The cache is not created when the baseline is rotated.
 *
 *      The exact cache size is compiler dependent - it allocates on the
 *      order of 3840 bytes of RAM per fontStyle.
 *
 *      If TS_NO_OPENTYPE is defined then this option should not be defined
 *      since it would not be used and would waste space.
 *
 *  Notes:
 *      This option is not applicable to the WorldType Shaper product.
 *
 *  <GROUP tsconfig>
 */
#ifndef TS_NO_OPENTYPE
#define TS_USE_OPENTYPE_CACHE
#endif


/*****************************************************************************
 *
 *  Build configuration option that enables internal memory management.
 *
 *  Description:
 *      If this option is enabled, the TsMemMgr object will use an internal
 *      heap manager for all WTLE objects created with the TsMemMgr.
 *      This allows the user to supply a preallocated heap within which WTLE
 *      will operate. It also allows the user to specify the size of a system
 *      block which the TsMemMgr will allocate once, and dole out blocks from
 *      there, thus avoiding system allocation calls, which are slow on some
 *      platforms. See the doumentation for TsMemMgr_new for more information.
 *
 *  <GROUP tsconfig>
 */
/*#define TS_INT_MEM*/


/*****************************************************************************
 *
 *  Build configuration option to enable memory-mapping the disk-based Thai
 *  dictionary file.
 *
 *  Description:
 *      When this option is enabled, the disk-based Thai dictionary file will
 *      be memory-mapped for access.  Otherwise, RAM is allocated and the file
 *      is copied into this block prior to use.  This option has no effect if
 *      the Thai dictionary file is ROM based or otherwise located with a
 *      memory address.
 *
 *  <GROUP tsconfig>
 */
#define TS_MEMORY_MAP_THAI_DICTIONARY_FILE

/*****************************************************************************
 *
 *  Build configuration option to enable tracing of memory operations.
 *
 *  Description:
 *      When this option is enabled, information about the allocation
 *      or free is printed through the TS_PRINTF_DEBUG macro.
 *
 *  <GROUP tsconfig>
 */
/*#define TS_MEM_TRACING*/


/*****************************************************************************
 *
 *  Build configuration option to enable passing file and line number through
 *  to the system allocation routines.
 *
 *  Description:
 *      When this option is defined, the memory allocation and free macros
 *      (TsMemMgr_malloc etc.) map to a set functions that take the file and
 *      line number in their parameter list. These functions use the
 *      TS_xALLOC_DEBUG and TS_FREE_DEBUG macros, which you can map to
 *      routines on your platform that take file and line number.
 *
 *  <GROUP tsconfig>
 */
/*#define TS_MEM_FILE_AND_LINE_INFO*/


#endif /* TSCONFIG_H */
