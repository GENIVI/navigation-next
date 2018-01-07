/*****************************************************************************
 * 
 *  fs_config.h  - Build configuration options
 *
 *  Description:
 *      This file contains iType build configuration options, 
 *      which allow you to:
 *          - include or exclude features from the build
 *          - select among alternate implementations
 *
 *      The build options are organized in several sections:    
 *          - Modes of operation
 *          - Glyph renderers
 *          - Additional features
 *          - Debugging tools
 *
 *      Glyph rendering options specify what types of glyphs can be rendered
 *      by iType.  Options include embedded bitmaps/graymaps (non-scalable),
 *      scalable bitmaps, and scalable graymaps, Edge Technology or some 
 *      combination of these.
 *
 *      Additional features includes support for TrueType hinting, other font
 *      formats such as stroke fonts and linked fonts, ACT3 and CCC compression,
 *      caching, and advanced features. You should specify only those features
 *      you intend to use in your system. 
 *      
 *                         *****IMPORTANT****
 *  Several of the build options below enable premium features.
 *  Premium features requires special licensing terms with Monotype Imaging
 *  in addition to the standard iType license. These options should only
 *  be enabled if your licensing agreement includes the premium feature.
 *  Premium features are identified with '*** Premium Feature ****' 
 *  in the macro header.
 *
 *  Copyright (C) 2001-2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef FS_CONFIG_H
#define FS_CONFIG_H


/****************************************************************************/
/*                          MODES OF OPERATION                              */
/****************************************************************************/


/*****************************************************************************
 *
 *  Enables memory-mapped font support
 *
 *  Description:
 *      Defining FS_MAPPED_FONTS enables the use of memory-mapped fonts
 *      in iType. Disk-based fonts will be memory mapped and accessed 
 *      via a memory pointer returned by the operating system.
 *
 *      This mode of operation uses system-dependent code in fs_mfont.c.
 *      You must implement the appropriate memory mapping code in
 *      that file. Several examples are provided in the source/port
 *      sub-directories.
 *      
 *  Notes:
 *      If FS_MAPPED_FONTS is not defined then fs_mfont.c
 *      can be stubbed out. See source/port/_stubs.
 *
 *  <GROUP fs-config>
 */
//#define FS_MAPPED_FONTS


/*****************************************************************************
 *
 *  Enables multi-process support
 *
 *  Description:
 *      Defining FS_MULTI_PROCESS enables data sharing across processes,
 *      and threads.
 *      
 *      In a multi-process scenario, iType clients share a single FS_SERVER,
 *      that is, they share a single set of a single set of loaded fonts,
 *      scaled fonts, glyph cache, and so forth. This occurs if the clients
 *      are in the same thread, different threads or different processes.
 * 
 *      Sharing a single FS_SERVER reduces memory requirements and improves
 *      speed compared to the equivalent non-shared case in which each process
 *      has its own copy of the internal iType data.
 *
 *      This option requires operating system support for accessing a
 *      shared memory space across processes. Linux, for example, supports
 *      this concept. Access to the data is synchronized  using a semaphore.
 *      The port directory must have valid implementations of fs_mutex.c,
 *      fs_shmem.c and fs_mfont.c.
 *      
 *      Defining this option also requires that FS_MULTI_THREAD and FS_INT_MEM 
 *      be defined.
 *
 *  Notes:
 *      Use of this option is not common. A more common approach to obtaining
 *      a similar result is to wrap iType within a font server using a 
 *      standard client-server protocol.
 *
 *      If FS_MULTI_PROCESS is not defined then fs_shmem.c and fs_mfont.c
 *      can be stubbed out. See source/port/_stubs.
 *
 *      FS_MULTI_PROCESS is not compatible with FS_CFFR
 *
 *  <GROUP fs-config>
 */
#undef FS_MULTI_PROCESS


/*****************************************************************************
 *
 *  Enables multi-threading support
 *
 *  Description:
 *      Defining FS_MULTI_THREAD enables thread-safe data sharing among 
 *      multiple threads.
 *      
 *      In a multi-threading scenario, iType clients in different threads may
 *      share a single FS_SERVER -- that is they share a single set of a single
 *      set of loaded fonts, scaled fonts, glyph cache, and so forth.
 *
 *      A "master" thread must call FS_init(), which initializes the first client
 *      and constructs an FS_SERVER. The other threads call FS_new_client to
 *      obtain a client that shares that FS_SERVER. Access to the shared data
 *      is synchronized using a mutex.
 *      
 *      If FS_MULTI_THREAD is defined, then the port directory should have a 
 *      valid implementation of fs_mutex.c. If the macro is left undefined
 *      then the stubbed version of fs_mutex.c should be used
 *      (see source/port/_stubs).
 *
 *  <GROUP fs-config>
 */
#undef FS_MULTI_THREAD


/*****************************************************************************
 *
 *  Indicates iType code is reentrant
 *
 *  Description:
 *      Beginning with iType 4.0, the code is always reentrant, that is
 *      multiple instances can be safely executed concurrently.
 *
 *      This macro has no effect on the iType build. The macro is defined
 *      for backward compatibility.
 *
 *  <GROUP fs-config>
 */
#define FS_REENTRANT    /* DO NOT CHANGE */


/*****************************************************************************
 *
 *  Enables internal memory management
 *
 *  Description:
 *      If FS_INT_MEM is defined then iType uses its internal memory manager,
 *      otherwise iType uses external memory management. 
 *
 *      EXTERNAL MEMORY MANAGMENT:
 *
 *      iType uses the SYS_MALLOC, SYS_REALLOC and SYS_FREE macros delared
 *      in fs_port.h. These macros should be set to the system
 *      memory functions, which are usually malloc, realloc and free.
 *
 *      iType tracks its total memory usage and ensures that it does not
 *      exceed the heap size value set when calling FS_init. Tracking usage is 
 *      more efficient if a FS_FREE_SIZE function is available. See FS_FREE_SIZE 
 *      and FREE_SIZE_DEFINED in fs_port.h for details.
 *
 *      INTERNAL MEMORY MANAGEMENT:
 *
 *      Upon construction, iType calls SYS_MALLOC to allocate a single heap
 *      buffer. The internal memory manager uses this buffer for all further 
 *      memory needs, until shutdown when iType calls SYS_FREE to free the
 *      buffer.
 *
 *      Internal memory management can improve performance by eliminating
 *      system malloc calls. The tradeoff is that there may be some loss of 
 *      effective heap size due to management overhead and memory fragmentation.
 *      
 *  <GROUP fs-config>
 */
#undef FS_INT_MEM



/****************************************************************************/
/*                          GLYPH RENDERERS                                 */
/****************************************************************************/

/*****************************************************************************
 *
 *  Enables embedded bitmap/embedded graymap support
 *
 *  Description:
 *      If FS_EMBEDDED_BITMAP is defined, then any TrueType font you use
 *      will be scanned for EBDT and EBLC data tables. If the font contains
 *      these tables, then, before generating a bitmap (graymap), iType will 
 *      check for a matching embedded bitmap (graymap). If an embedded bitmap
 *      (graymap) is found that matches (in terms of glyph index, scaling and 
 *      so forth), then that embedded bitmap (graymap) will be used and the 
 *      scaled bitmap (graymap) will not be constructed. Otherwise, if no 
 *      matching embedded bitmap (graymap) is found, iType will generate a 
 *      bitmap (graymap) from the scalable outline data, as it normally does.
 *
 *      If FS_EMBEDDED_BITMAP is the only renderer specified (this is not
 *      common), then iType will only return non-scaled, embedded bitmap
 *      or graymap glyphs. 
 *
 *      This option is usually defined. If none of your fonts have 
 *      embedded bitmaps then undefining this option will reduce code size
 *      and improve performance somewhat.
 *
 *  <GROUP fs-config>
 */
#define FS_EMBEDDED_BITMAP


/*****************************************************************************
 *
 *  Enables binary (1-bit) glyph rendering support
 *
 *  Description:
 *      If FS_BITMAPS is defined, then iType will return scaled bitmaps 
 *      using FS_get_bitmap or FS_get_glyphmap with FS_MAP_BITMAP specified
 *      as the glyph type.  
 *
 *      If you never need binary glyphs then disabling this option will
 *      reduce code size.
 *
 *  <GROUP fs-config>
 */
#define FS_BITMAPS


/*****************************************************************************
 *
 *  Enables 2, 4, or 8-bit glyph rendering support
 *
 *  Description:
 *      If FS_GRAYMAPS is defined, then iType will return scaled 
 *      4-bit graymaps using FS_get_graymap or 2, 4, or 8-bit graymaps using
 *      FS_get_glyphmap with FS_MAP_GRAYMAP specified as the glyph type.
 *
 *      If you never need grayscale glyphs then disabling this option
 *      will reduce code size. Disabling this option is not common.
 *
 *  <GROUP fs-config>
 */
#define FS_GRAYMAPS


/*****************************************************************************
 *
 *  Enables Edge(TM) hinting  ****Premium Feature****
 *
 *  Description:
 *      Edge(TM) Technology consists of Edge hinting and Edge Rendering of 
 *      graymaps. The two features are enabled separately and may be used 
 *      separately.
 *
 *      Defining this option enables Edge hinting support. Edge hinting may
 *      be used with regular iType graymap rendering or with Edge rendering.
 *      Edge hinting is not recommended for use with iType bitmap rendering.
 *
 *                         *****IMPORTANT****
 *      This a premium feature that requires special licensing
 *      terms with Monotype Imaging. Use Edge Technology support only
 *      if your licensing agreement includes Edge Technology.
 *
 *  <GROUP fs-config>
 */
#undef FS_EDGE_HINTS    /* Premium feature - requires special licensing */


/*****************************************************************************
 *
 *  Enables Edge(TM) rendering  *** Premium Feature ***
 *
 *  Description:
 *      Edge(TM) Technology consists of Edge hinting and Edge Rendering of 
 *      graymaps. The two features are enabled separately and may be used 
 *      separately.
 *
 *      Defining this option enables Edge grayscale rendering support.
 *
 *                         *****IMPORTANT****
 *      This a premium feature that requires special licensing
 *      terms with Monotype Imaging. Use Edge Technology support only
 *      if your licensing agreement includes Edge Technology.
 *
 *  <GROUP fs-config>
 */
#define FS_EDGE_RENDER   /* Premium feature - requires special licensing */


/****************************************************************************/
/*                          ADDITIONAL FEATURES                             */
/****************************************************************************/

/*****************************************************************************
 *
 *  Enables TrueType hinting
 *
 *  Description:
 *      Defining this option enables TrueType hinting in the build.
 *      TrueType hinting can be enabled/disabled at run-time using 
 *      FS_set_flags.
 *
 *      If undefined, only unhinted outlines and glyphs will be produced.
 *  
 *      This option is usually defined.
 *
 *  <GROUP fs-config>
 */
#define FS_HINTS


/*****************************************************************************
 *
 *  Enables support for linked fonts
 *
 *  Description:
 *      Define this option if you wish to use linked fonts.
 *
 *      If undefined, then an attempt to use a linked font using
 *      FS_add_font or FS_load_font will result in a 
 *      ERR_LINKED_FONTS_UNDEF error. 
 *
 *  <GROUP fs-config>
 */
#define FS_LINKED_FONTS

/*****************************************************************************
 *
 *  Enables support for CFF OpenType fonts
 *
 *  Description:
 *      Define this option if you wish to use CFF OpenType fonts.
 *
 *  Notes:
 *      FS_CFFR is not compatible with FS_MULTI_PROCESS
 *
 *  <GROUP fs-config>
 */
#define FS_CFFR

/*****************************************************************************
 *
 *  Enables support for stroke fonts
 *
 *  Description:
 *      Define this option if you wish to use stroke fonts.
 *
 *  Notes:
 *      If you want to maintain compatibility with iType 2.0 stroke-font data
 *      and do NOT want to use the more recent versions of the stroke-font
 *      format then define FS_STIK and undefine FS_CCC.
 *
 *  <GROUP fs-config>
 */
#define FS_STIK


/*****************************************************************************
 *
 *  Enables support for 'ccc' font compression
 *
 *  Description:
 *      Define this option if you wish to use 'ccc' compressed fonts.
 *
 *  <GROUP fs-config>
 */
#define FS_CCC


/*****************************************************************************
 *
 *  Enables support for 'ACT' font compression
 *
 *  Description:
 *      Define this option if you wish to use ACT compressed fonts.
 *
 *  <GROUP fs-config>
 */
#define FS_ACT3


/*****************************************************************************
 *
 *  Enables support for 'phased' graymap rendering
 *
 *  Description:
 *      Phased graymaps can be used to obtain the effect of character 
 *      positioning at ¼ pixel intervals, and stem weight resolution 
 *      to ¼ pixel. The tradeoff is that the hard edges produced
 *      by hinting are diminished. This approach can be useful for
 *      standard definition television displays. It is not suitable for
 *      most other displays.
 *
 *      Phased graymaps are obtained by calling FS_get_phased()
 *      with a build that has FS_PHASED defined.
 *
 *      This macro requires that FS_BITMAPS and FS_GRAYMAPS also be set.
 *
 *  <GROUP fs-config>
 */
#define FS_PHASED


/*****************************************************************************
 *
 *  Enables support for multi-colored icon glyphs
 *
 *  Description:
 *      Defining this option builds in code for handling multi-colored icons.
 *      Icons may be embedded within a font in the ICON table. The
 *      ICON table is a Monotype Imaging proprietary table.
 *
 *      The icons can be retrieved using the FS_get_glyphmap function
 *      with a FS_MAP_VECTOR_ICON or FS_MAP_RASTER_ICON type.
 *
 *  Notes:
 *      Only define this option if you need multi-colored icon support. 
 *      A performance penalty is associated with the use of this option.
 *
 *  <GROUP fs-config>
 */
#undef FS_ICONS


/*****************************************************************************
 *
 *  Enables support for pseudo-bold
 *
 *  Description:
 *      Define FS_PSUEDO_BOLD to enable pseudo-bold support.
 *
 *      Psuedo-bold is a technique that increases the boldness of glyphs
 *      using an algorithmic technique rather than in the font design itself.
 *
 *      At run-time, pseudo-boldness is achieved using the FS_set_bold_pct
 *      function.
 *
 *      This macro is usually defined.
 *
 *  <GROUP fs-config>
 */
#define FS_PSEUDO_BOLD


/*****************************************************************************
 *
 *  Several macros are used to control caching of glyphs, advance width, and 
 *  cmap entries
 *
 *  Description:
 *      You can separately specify caching for advance values, outlines, 
 *      bitmaps, graymaps, and phased graymaps, as well as for 
 *      'cmap' table entries.
 *
 *      Caching improves performance dramatically. Retrieving a glyph from
 *      cache is several hundred times faster than generating it from scratch.
 *      In general, you want to enable caching for items you will get from iType,
 *      and for which you do not have an external cache. Even then it might make
 *      sense, depending on how the external cache is organized. 
 *      
 *      <TABLE>             
 *      Macro                   Item                Function(s) that benefit from caching this item
 *      --------------------    --------------      --------------------------------------------------_
 *      FS_CACHE_ADVANCE        advance values      FS_get_advance
 *      FS_CACHE_OUTLINES       FS_OUTLINE          FS_get_outline
 *      FS_CACHE_BITMAPS        FS_BITMAP           FS_get_bitmap, FS_get_glyphmap
 *      FS_CACHE_GRAYMAPS       FS_GRAYMAP          FS_get_graymap, FS_get_glyphmap
 *      FS_CACHE_PHASED         FS_GRAYMAP          FS_get_phased
 *      FS_CACHE_EDGE_GLYPHS    FS_GRAYMAP          FS_get_glyphmap
 *      FS_CACHE_CMAP           cmap entry          FS_map_char, get glyph function, FS_get_outline
 *      </TABLE>
 *
 *  Notes:
 *      The glyph type returned by FS_get_glyphmap function depends on the 
 *      'type' argument, glyph rendering build options, and font contents. 
 *      But if you are specifying and getting a particular glyph type,
 *      then you probably want to cache that item - either within iType 
 *      or externally.
 *
 *      If you are caching bitmaps and graymaps, then caching outlines is usually
 *      not needed unless you also call FS_get_outline directly.
 *
 *      We recommend that you experiment with these settings and the initial 
 *      heap_size to  determine the best mix of size versus speed for your 
 *      particular environment.
 *
 *      As a rule of thumb though, a cache ‘hit rate’ of 90% will improve
 *      overall performance by a factor of 10 (compared to no caching).
 *
 *      OpenVG data glyphs (FS_VGGLYPH) can be cached. These glyphs are assembled using 
 *      underlying FS_GRAYMAP, FS_BITMAP, or FS_OUTLINE objects which may also be cached.
 *      In general, you only need to cache one or the other depending on what
 *      you use. If you access these types seperately, then you should cache
 *      both types separately.
 *
 *      Icon glyphs are not cached.
 *
 *  <GROUP fs-config>
 */
#define FS_CACHE_ADVANCE
#undef  FS_CACHE_OUTLINES       /* FS_OUTLINES must also be defined    */
#define FS_CACHE_BITMAPS        /* FS_BITMAPS must also be defined     */
#define FS_CACHE_GRAYMAPS       /* FS_GRAYMAPS must also be defined    */
#undef  FS_CACHE_PHASED         /* FS_PHASED must also be defined      */
#define FS_CACHE_EDGE_GLYPHS    /* FS_EDGE_RENDER must also be defined */
#undef  FS_CACHE_OPENVG         /* FS_OPENVG must also be defined      */
#define FS_CACHE_CMAP


/*****************************************************************************
 *
 *  Macro that defines the maximum cache size to be used for a single glyph
 *
 *  Description:
 *      When using glyph caching you must specify the maximum size, in bytes,
 *      of any single cache entry. This is to avoid the situation where 
 *      iType adds a very large character to the cache, and, in doing so, 
 *      clears out most of the existing cached characters in order to make
 *      enough room for the single large character.
 *
 *      For example, to cache nothing larger than a 50 line bitmap
 *          #define FS_CACHE_MAX_SIZE 350
 *          (an NxM bitmap is approximately 30 + (N*M)/8 bytes)
 *
 *      To cache nothing larger than a 50 line 4-bit graymap
 *          #define FS_CACHE_MAX_SIZE 1280
 *          (an NxM graymap is approximately 30 + (N*M)/2 bytes)
 *
 *      The default value (5120) handles up to 100 line 4-bit graymaps
 *      or 71 line 8-bit graymaps.
 *
 *      By looking at the data collected by defining FS_STATS, you can 
 *      arrive at proper settings for your environment.
 *
 *  <GROUP fs-config>
 */
#define FS_CACHE_MAX_SIZE 5120


/*****************************************************************************
 *
 *  Retains regular advance when emboldening monospace fonts
 *
 *  Description:
 *      Defining FS_MONOSPACE_PSEUDOBOLD_ADVANCE forces advance widths for
 *      monospace fonts to remain the same as regular advance widths.
 *
 *      Normally, the advance width for a proportional font is increased 
 *      proportionally to the emboldening amount. For monospace fonts, 
 *      the advance width is normally the same between regular and bold
 *      versions of the font. By default, applying pseudo-emboldening to glyphs
 *      will cause the advance width to increase. 
 *
 *      Defining FS_MONOSPACE_PSEUDOBOLD_ADVANCE forces advance widths for
 *      monospace fonts to remain the same as the regular advance widths. 
 *
 *      iType will then detect whether a font is considered to be a 
 *      monospace font by examining the fourth byte of the panose value
 *      in the OS/2 table, if present (this table is optional).
 *
 *  Notes:
 *      This flag affects all outlines generated from the font, regardless
 *      of rasterization type in effect. All glyphs in the font are considered
 *      either monospace or not.
 *
 *      This flag does not affect CJK bold (FLAGS_ADD_WEIGHT). It only affects
 *      outline emboldening.
 *      
 *  <GROUP fs-config>
 */
#define FS_MONOSPACE_PSEUDOBOLD_ADVANCE


/*****************************************************************************
 *
 *  Enables contour-winding support in the build
 *
 *  Description:
 *      Defining FS_CONTOUR_WINDING_DETECTION enables an algorithm that
 *      detects and corrects two problems that may occur when using pseudo-bold.
 *
 *      First, there are some TrueType fonts which do not comply with the TrueType
 *      specification for contour direction: they may be wound in the wrong 
 *      direction (ink on the left, rather than right), or they may be 
 *      inconsistently wound (contours in both directions in a single character).
 *
 *      iType will rasterize these fonts correctly if no emboldening
 *      (pseudo bold) is in effect; however, these fonts may produce incorrect
 *      output when emboldening is applied. For example, the characters could get
 *      thinner rather than bolder, or thinner in some parts and bolder
 *      in others.
 *
 *      The second problem occurs when the scale matrix is set so that the 
 *      character is mirrored about the y-axis and emboldening is in effect.
 *      You contour direction will be reversed and you may see problems with 
 *      thinning glyphs. This can occur even fonts that have correct winding.
 *
 *      If you use emboldening, and you expect to use either noncompliant 
 *      TrueType fonts in your target environment or glyph mirroring
 *      then you should enable FS_CONTOUR_WINDING_DETECTION.
 *      You must also turn on this feature at run time by making this call:
 *      FS_set_flag(FLAGS_CHECK_CONTOUR_WINDING_ON);
 *
 *      iType will then detect the incorrect winding and correctly apply
 *      pseudo-bold.
 *
 *  Notes:
 *      FS_BITMAPS must also be defined, even if not generating bitmaps.
 *      
 *      This define has no effect when using Edge Technology rendering.
 *      
 *  <GROUP fs-config>
 */
#define FS_CONTOUR_WINDING_DETECTION

 
 /*****************************************************************************
 *
 *  Define to use only even sizes in stroke fonts with SmartHint technology.
 *
 *  Description:
 *      Some SmartHint fonts only contain hints at even sizes to save space.
 *      If FS_STIK_EVENS_ONLY is defined and you are using a SmartHint font, 
 *      iType will automatically substitute the next lower even size when an 
 *      odd ppem scale size is set. 
 *
 *  <GROUP fs-config>
 */

#define FS_STIK_EVENS_ONLY


/*****************************************************************************
 *
 *  Enables OpenVG support  ****Premium feature****
 *
 *  Description:
 *      When this option is defined, iType can produce VGPath and VGImage 
 *      objects that are compatible with the OpenVG standard.
 *
 *                         *****IMPORTANT****
 *      This a premium feature that requires special licensing
 *      terms with Monotype Imaging. Define FS_OPENVG only
 *      if your licensing agreement includes OpenVG support.
 *
 *  <GROUP fs-config>
 */
#undef FS_OPENVG    /* Premium feature - requires special licensing */


/*****************************************************************************
 *
 *  Enables PFR font support  ****Premium Feature****
 *
 *  Description:
 *      When this option is defined, iType can be used to render outline 
 *      fonts encoded in PFR format, provided that such rendering is to be 
 *      used to achieve compliance with digital TV  standards developed by 
 *      DAVIC (ISO/IEC 16500), DVB and/or Digital TV Group.
 *
 *                         *****IMPORTANT****
 *      This a premium feature that requires special licensing
 *      terms with Monotype Imaging. Define FS_PFRR only
 *      if your licensing agreement includes PFR support.
 *
 *  <GROUP fs-config>
 */
#undef FS_PFRR      /* Premium feature - requires special licensing */



/****************************************************************************/
/*                      DEBUGGING TOOLS                                     */
/****************************************************************************/

/*****************************************************************************
 *
 *  Enables extra error checking, messages, etc.
 *
 *  Description:
 *      The FS_DEBUG facility enables extra error checking of the 
 *      application's implementation. Results are sent to stdout.
 *      
 *  Notes:
 *      FS_DEBUG should not be defined in final release builds or in builds
 *      that need to be thread safe because it declares global variables.
 *
 *  <GROUP fs-config>
 */
#undef FS_DEBUG


/*****************************************************************************
 *
 *  Enables extra FS_MULTI_PROCESS-specific messages
 *
 *  Description:
 *      
 *
 *  Notes:
 *      The FS_STATE structure contains a field, 'process_num'  that
 *      is conditionally compiled on FS_MP_DEBUG. If you enable
 *      FS_MP_DEBUG, you must set this field directly from your
 *      application. Otherwise, uninitialized-memory warnings may result.
 *      
 *      The 'testmp' program demonstrates how to set process_num correctly.
 *      
 *  <GROUP fs-config>
 */
#undef FS_MP_DEBUG


/*****************************************************************************
 *
 *   If defined, then memory management operations are logged to a file
 *      
 *   Description:
 *      This debugging option may be used to output a record of memory 
 *      allocations to a file named fs_mem_dbg.txt.
 *
 *      The name of this file is hard coded in the FSS_init function
 *      in fs_api.c.
 *
 *  <GROUP fs-config>
 */
#undef FS_MEM_DBG


/*****************************************************************************
 *
 *   Enables "dump" functions within the build
 *
 *   Description:
 *      If FS_DUMP is defined, then the build will include
 *      the functions: dump_bitmap(), dump_graymap(), dump_outline().
 *      These routines can then be called by test applications to dump the
 *      glyph metrics and an ASCII representation of the glyph to stdout.
 *
 *      <CODE>
 *      For example, the bitmap 'A' was generated using the albany__.ttf font:
 *
 *          lo_x=-1 hi_y=16 width=18 height=17 bpl=3
 *          i_dx=15 i_dy=0 dx=15.340820 dy=0.000000
 *             000000000000000000
 *             000000000001111111
 *             101234567890123456
 *          16 ------------------ 00 00 00
 *          15 -------***-------- 01 c0 00
 *          14 -------***-------- 01 c0 00
 *          13 ------**-**------- 03 60 00
 *          12 ------**-**------- 03 60 00
 *          11 ------**-**------- 03 60 00
 *          10 -----**---**------ 06 30 00
 *          09 -----**---**------ 06 30 00
 *          08 ----**-----**----- 0c 18 00
 *          07 ----**-----**----- 0c 18 00
 *          06 ---**********----- 1f fc 00
 *          05 ---**********----- 1f fc 00
 *          04 ---**-------**---- 18 0c 00
 *          03 --**---------**--- 30 06 00
 *          02 --**---------**--- 30 06 00
 *          01 -**-----------**-- 60 03 00
 *          00 -**-----------**-- 60 03 00
 *
 *          </CODE>
 *
 *      If you also define FS_ICON, the build will also include dump_icon.
 *      If FS_DUMP is not defined then these interfaces are not available.
 *   
 *  <GROUP fs-config>
 */
#define FS_DUMP


/*****************************************************************************
 *
 *   Enables reporting of glyph generation and caching statistics.
 *
 *   Description:
 *      If FS_STATS is defined, then iType collects statistics regarding
 *      glyph generation, embedded bitmaps and caching. These statistics
 *      are collected during the entire run-time until shutdown occurs,
 *      by calling FS_exit. At that time the results are reported to stdout.
 *
 *      The results can help you to determine proper configuration and 
 *      cache settings for your environment.
 *
 *      <CODE>
 *      This is an example of FS_STATS output for two embedded bitmap 
 *      characters iType processed using cache:
 *
 *      made_outl =0
 *      cached_outl =0
 *      made_bmap =0 
 *      cached_bmap =2
 *      embedded_bmap =2 
 *      made_gmap =0
 *      cached_gmap =0
 *      uncached_too_big=0
 *
 *      </CODE>
 *
 *  Notes:
 *      FS_STATS should not be defined in final release builds or in builds
 *      that need to be thread safe because it declares global variables.
 *
 *  <GROUP fs-config>
 */
#undef FS_STATS


#endif /* FS_CONFIG_H */
