
/* Copyright (C) 2001-2012 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/*  Encoding:   US_ASCII    Tab Size:   8   Indentation:    4        */

/* Copyright: 1987-1990 by Apple Computer, Inc., all rights reserved. */

/* fs_object.h */

#ifndef FS_OBJECT_H
#define FS_OBJECT_H

#include "fs_port.h"
#include "fs_bitio.h"
#include "fs_sfnt.h"
#include "fs_scratch_priv.h"

#ifdef FS_INT_MEM
#include "fs_bestfit.h"
#endif


/****************************************************************/
/* things users probably don't ever need to ever touch */
/****************************************************************/

/* only used by some MTI-internal font tools */
/* undefine in shipping product */
#undef EDIT_MODE

/* define to access QE test data in non-shipping directory */
/* undefine in shipping product */
#undef MTI_QE_TEST

/***************************/
/* various debugging flags */
/***************************/

/* define to get LOTS of information from each TrueType instruction */
#undef FS_TRACE    

/* define to get LOTS of information in tableptr.c and xmalloc.c */
#undef DEBUG_TABLE_PTR

/* define to get LOTS of information in fsglue.c */ 
#undef NC_DEBUG

/* define to get LOTS of information about autohinting */
#undef GRID_DEBUG    

/* define for direct draw debugging in stik.c */
#undef DEBUG_DIRECT

/* define for embedded-bitmap debugging in sbit.c */
#undef SBIT_DEBUG

/* set to 1 for pseudo-bold debugging in outline.c */
#define ALDUMP 0

/* define to perform debugging of INT_MEM heap in xmalloc.c */
/* do NOT enable in production code - will slow down the code severely */
#undef CHECK_HEAP        


/***********************************************************************/
/* The following enables an externally generated outline to be         */
/* rasterized by iType. The outline must be stored in                  */
/* STATE.user_outline prior to calling a get glyph map function.       */
/* The outline must conform to the FS_OUTLINE format.                  */
#undef FS_EXTERNAL_OUTLINE

/***********************************************************************/
/* The following enables trim_bitmap functionality for bitmap output   */
/* Trimmed bitmaps take less room in cache but are slower to produce   */
#define FS_TRIM_BITMAP

/***********************************************************************/
/* The following applies to loading fonts by a file path               */
/* Causes more tables to be loaded into memory to improve speed        */
#undef FS_SPEED_OVER_MEMORY

/* define to eliminate the memory lumping scheme */
#undef FS_NO_FS_LUMP

#define GASP_ADFH_CONTROL

/****************************************************************/
/* things users don't ever need to ever touch */
/****************************************************************/
typedef FS_LONG FS_FIXED;           /* 16.16 fixed point */
typedef FS_LONG FRACT;              /* 2.30 fixed point  */
typedef FS_LONG F26DOT6;            /* 26.6 fixed point  */
typedef FS_SHORT SHORTFRACT;        /* 2.14 fixed point  */

#ifdef FS_TRACE
#define FS_DEBUG 
extern int in_char;
extern int indent_is;
#endif /* FS_TRACE */

#ifdef FS_CACHE_CMAP
typedef struct
{
    FS_USHORT unicode;
    FS_USHORT index;
} FS_CMAP_CACHE;
#endif

typedef FS_BYTE FS_BOOLEAN;  /* true/false stored in a byte */
typedef FS_TINY FS_CHAR;     /* provided for compatibility with pre-2.3 iType releases */
 
/* Use the FILECHAR datatype where you need to match the system-defined "char" type
(i.e. to pass to system I/O or string functions) - if you use this datatype
rather than an unadorned "char", the semantics are clearer. */
typedef char FILECHAR;

#ifdef FS_EDGE_RENDER
#include "../adf/adftypesystem.h"
#endif

/* 2.14 UNIT VECTOR */
typedef struct {
    SHORTFRACT x,y;
    } SHORT_UNIT_VECTOR;

/* 2.30 UNIT VECTOR */
typedef struct {
    FRACT x,y;
    } UNIT_VECTOR;

typedef struct {
    F26DOT6 x,y;
    } F26DOT6_VECTOR;

typedef struct {
    FS_FIXED x,y;
    } FIXED_VECTOR;

typedef struct {
    FS_FIXED        transform[3][3];
    } transMatrix;

/* some useful macros -- watch out for side effects */
#ifndef ABS
#define ABS(x)      (((x)>0)?(x):-(x))
#endif

#ifndef MAX
#define MAX(a,b)    (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b)    (((a)<(b))?(a):(b))
#endif

/* byte swapping macros */ 
#if FS_BYTE_ORDER == FS_LITTLE_ENDIAN 
#define NEED_TO_SWAP 1 
/* Do not increment a pointer while passing it into these two macros.Ie do not make a call
   such as SWAPW(*p++), as the argument passed into SWAPW(x) is evaluated twice, causing 
   the pointer p to be incremented twice. 
*/
#define SWAPW(x) ( (((FS_USHORT)(x) >> 8) & 0x00FFU) | (((FS_USHORT)(x) << 8) & 0xFF00U) ) 
#define SWAPL(x) ( (((x) >> 24) & 0x000000FF) | (((x) >> 8) & 0x0000FF00) | (((x) << 8) & 0x00FF0000) | (((x) << 24) & 0xFF000000) ) 
#else /* BYTEORDER==FS_BIG_ENDIAN */
#define NEED_TO_SWAP 0
#define SWAPW(x) (x)
#define SWAPL(x) (x)
#endif /* BYTEORDER */

#ifdef FS_STIK
#define LATIN 1
#define CJK 2
#define GENERIC 3
#define OTHER 4

/* indices into the CVT for new STIK spacing algorithm */
#define NS_CAP_ROUND    0
#define NS_CAP_SQUARE   1
#define NS_X_SQUARE     2
#define NS_X_ROUND      3
#define NS_BASE_SQUARE  4
#define NS_BASE_ROUND   5
#define NS_O_LSB        6
#define NS_O_RSB        7        
#define NS_CVT_USED     12    /* number of indices used for Latin, GREEK, CYRILLIC */
#define ARABIC_UPPER_ACCENT_BOT            0
#define ARABIC_UPPER_ACCENT_TOP            1
#define ARABIC_LOWER_MEDIUM_ACCENT_BOT     2
#define ARABIC_UPPER_MEDIUM_ACCENT_BOT     3
#define ARABIC_UPPER_NUMERAL_TOP           4
#define ARABIC_UPPER_NUMERAL_BOT           5
#define ARABIC_LOWER_NUMERAL_TOP           6
#define ARABIC_LOWER_NUMERAL_BOT           7
#define ARABIC_UPPER_HIGH_TOP              8
#define ARABIC_UPPER_HIGH_BOT              9
#define ARABIC_LOWER_HIGH_BOT              10
#define ARABIC_LOWER_MEDIUM_HIGH_BOT       11
#define ARABIC_LOWER_MEDIUM_SHORT_BOT      12
#define ARABIC_LOWER_SHORT_HIGH_BOT        13
#define ARABIC_LOWER_SHORT_SHORT_BOT       14
#define ARABIC_CENTER_HIGH_TOP             15
#define ARABIC_CENTER_SHORT_TOP            16
#define ARABIC_UPPER_HIGH_1_DOT_TOP        17
#define ARABIC_UPPER_HIGH_2_DOT_TOP        18
#define ARABIC_UPPER_HIGH_3_DOT_TOP        19
#define ARABIC_UPPER_SHORT_1_DOT_TOP       20
#define ARABIC_UPPER_SHORT_2_DOT_TOP       21
#define ARABIC_UPPER_SHORT_3_DOT_TOP       22
#define ARABIC_SUPERSCRIPT_DAMMA_TOP       23
#define ARABIC_SUPERSCRIPT_DAMMA_BOT       24
#define ARABIC_SUPERSCRIPT_FATHA_TOP       25
#define ARABIC_SUPERSCRIPT_FATHA_BOT       26
#define ARABIC_SUBSCRIPT_KASRA_TOP         27
#define ARABIC_SUBSCRIPT_KASRA_BOT         28
#define ARABIC_SUPERSCRIPT_SHADDA_TOP      29
#define ARABIC_SUPERSCRIPT_SHADDA_BOT      30
#define ARABIC_SUPERSCRIPT_SUKUN_BOT       31
#define ARABIC_SUPERSCRIPT_SUKUN_TOP       32
#define ARABIC_ACCENT_SHADDA_KASRA_TOP     33
#define ARABIC_ACCENT_SHADDA_KASRA_BOT     34
#define ARABIC_ACCENT_SHADDA_FATHA_TOP     35
#define ARABIC_ACCENT_SHADDA_FATHA_BOT     36
#define ARABIC_SUPERSCRIPT_ALEF_TOP        37
#define ARABIC_SUPERSCRIPT_ALEF_BOT        38
#define ARABIC_SUBSCRIPT_ALEF_TOP          39
#define ARABIC_SUBSCRIPT_ALEF_BOT          40
#define ARABIC_UPPER_ACCENT_STEM_TOP       41
#define ARABIC_CVT_USED 42 /* number of indices used for ARABIC autohinting */

#endif /* FS_STIK */

/* move <p> to the next possible location of a (FS_VOID *) */
#define FS_ALIGN(p) p += (sizeof(FS_VOID *) - 1); p &= ~(sizeof(FS_VOID *) - 1)

#define GET_xWORD(a)  \
           ( ((FS_USHORT) *(FS_BYTE *)(a) << 8) |      \
              (FS_USHORT) *((FS_BYTE *)(a)+1) )

#define GET_xLONG(a)  \
           ( ((FS_ULONG) *(FS_BYTE *)(a) << 24)     |  \
             ((FS_ULONG) *((FS_BYTE *)(a)+1) << 16) |  \
             ((FS_ULONG) *((FS_BYTE *)(a)+2) << 8)  |  \
              (FS_ULONG) *((FS_BYTE *)(a)+3) ) 

#define GET_xUINT24(a)  \
           ( ((FS_ULONG) *(FS_BYTE *)(a) << 16)     |  \
             ((FS_ULONG) *((FS_BYTE *)(a)+1) << 8)  |  \
              (FS_ULONG) *((FS_BYTE *)(a)+2) ) 

#define FRAC2FIX(x) ((FS_FIXED)(x)<<2)


#ifndef FS_NO_FS_LUMP
typedef struct FS_MASTER_LUMP_
{
    FS_ULONG ref_count;
    struct FS_MASTER_LUMP_ *prev, *next;
} FS_MASTER_LUMP;

/*
 * allocate all fixed sized <things> in lumps to reduce heap fragmentation
 * also makes things their alloc/free much faster
 */

typedef struct FS_LUMP_
{
    FS_MASTER_LUMP  *master;    /* MUST BE FIRST IN STRUCTURE!!! */
    struct FS_LUMP_ *next;
} FS_LUMP;
#endif

/* font types */
#define UNK_TYPE 127
#define TTF_TYPE 0
#define TTC_TYPE 1
#define LTT_TYPE 2
#define PFR_TYPE 6
#define CFF_TYPE 7

/* guess at the number of contours and points in a CFF */
#define CFF_CONTOURS 64
#define CFF_POINTS   256

#define FNTSET_CHECKSUM_LENGTH 200

/* Font style definitions used by FS_get_available_fonts function. 
     <TABLE>
     STYLE_PLAIN
     STYLE_ITALIC
     STYLE_BOLD
     STYLE_BOLDITALIC
     STYLE_ANY
     </TABLE>
 
   <GROUP itypeenums>
 */
typedef enum
{
    STYLE_PLAIN,
    STYLE_ITALIC,
    STYLE_BOLD,
    STYLE_BOLDITALIC,
    STYLE_ANY
} FS_STYLE;

/* Font face type definitions used by FS_get_available_fonts function. 
     <TABLE>
     FACE_PROPORTIONAL,
     FACE_MONOSPACED,
     FACE_ANY
     </TABLE>
   <GROUP itypeenums>
 */
typedef enum
{
    FACE_PROPORTIONAL,
    FACE_MONOSPACED,
    FACE_ANY
} FS_FACE;

/* outline element types */
#define FS_MOVETO 0
#define FS_LINETO 1
#define FS_QUADTO 2
#define FS_CUBETO 3

#ifdef FS_STATS
extern FS_ULONG cache_calls;
extern FS_ULONG cache_probes;
extern FS_ULONG cached_advance;
extern FS_ULONG made_outl;
extern FS_ULONG cached_outl;
extern FS_ULONG made_bmap;
extern FS_ULONG cached_bmap;
extern FS_ULONG made_gmap;
extern FS_ULONG embedded_bmap;
extern FS_ULONG cached_gmap;
extern FS_ULONG made_openvg;
extern FS_ULONG cached_openvg;
extern FS_ULONG uncached_too_big;
extern FS_ULONG collisions;
extern FS_ULONG num_composite_hints;
extern FS_ULONG num_composite_chars;
extern FS_ULONG size_composite_char;
#endif

/********************* rasterization ****************************/
/* transition array node */
typedef struct {
    FS_FIXED value;      /* coordinate of this node in singley linked list */
    FS_LONG next;        /* index of next T_NODE in the singley linked list */
    } TNODE;

/* transition array */
typedef struct { 
    FS_SHORT max;        /* y coordinate of slot[0] */
    FS_SHORT num;        /* number of slots */
    FS_LONG stop_index;  /* 1+maximum index into <nodes> */
    FS_LONG next_index;  /* next available index into <nodes> */
    FS_LONG *indices;    /* starting index into <nodes> for each coordinate */
    FS_LONG hph_flag;
    FS_LONG hph_loopstartz;
    FS_LONG hph_prevz;
    FS_SHORT hph_loopstarty;
    FS_SHORT hph_prevy;
    FS_SHORT ref_count;  /* prevent get_some_back from recovering TLIST */
    TNODE *nodes;        /* the singly linked list of values */
    FS_LONG numIndices;
    FS_LONG numNodes;
    } TLIST;


/************* TrueType Fonts ***********************************/
#define STUBCONTROL 0x10000L
#define NODOCONTROL 0x20000L

/* unpacking constants */
#define ONCURVE              0x01
#define XSHORT               0x02
#define YSHORT               0x04
#define REPEAT_FLAGS         0x08
#define SHORT_X_IS_POS       0x10  /* flags & XSHORT */
#define NEXT_X_IS_ZERO       0x10
#define SHORT_Y_IS_POS       0x20  /* flags & YSHORT */
#define NEXT_Y_IS_ZERO       0x20


/* composite glyph constants */
#define ARG_1_AND_2_ARE_WORDS     0x0001  /* if set args are words otherwise they are bytes */
#define ARGS_ARE_XY_VALUES        0x0002  /* if set args are xy values, otherwise they are points */
#define ROUND_XY_TO_GRID          0x0004  /* for the xy values if above is true */
#define WE_HAVE_A_SCALE           0x0008  /* Sx = Sy, otherwise scale == 1.0 */
#define NON_OVERLAPPING           0x0010  /* set to same value for all components */
#define MORE_COMPONENTS           0x0020  /* indicates at least one more glyph after this one */
#define WE_HAVE_AN_X_AND_Y_SCALE  0x0040  /* Sx, Sy */
#define WE_HAVE_A_TWO_BY_TWO      0x0080  /* t00, t01, t10, t11 */
#define WE_HAVE_INSTRUCTIONS      0x0100  /* instructions follow */
#define USE_MY_METRICS            0x0200  /* apply these metrics to parent glyph */        

#define TTF_MAGIC 0x5F0F3CF5L

/* table tags in Big-Endian Byte order */
#define MAKE_TAG(a,b,c,d)    (((FS_ULONG)a<<24)|((FS_ULONG)b<<16)|((FS_ULONG)c<<8)|d)

/* if your compiler is decent these will become simple 32 bit constants */
#define TAG_ttcf    MAKE_TAG('t','t','c','f')
#define TAG_head    MAKE_TAG('h','e','a','d')
#define TAG_hhea    MAKE_TAG('h','h','e','a')
#define TAG_hmtx    MAKE_TAG('h','m','t','x')
#define TAG_maxp    MAKE_TAG('m','a','x','p')
#define TAG_loca    MAKE_TAG('l','o','c','a')
#define TAG_glyf    MAKE_TAG('g','l','y','f')
#define TAG_cmap    MAKE_TAG('c','m','a','p')
#define TAG_cvt     MAKE_TAG('c','v','t',' ')
#define TAG_fpgm    MAKE_TAG('f','p','g','m')
#define TAG_prep    MAKE_TAG('p','r','e','p')
#define TAG_EBLC    MAKE_TAG('E','B','L','C')
#define TAG_EBDT    MAKE_TAG('E','B','D','T')
#define TAG_kern    MAKE_TAG('k','e','r','n')
#define TAG_act3    MAKE_TAG('a','c','t','3')    
#define TAG_nstk    MAKE_TAG('n','s','t','k')    
#define TAG_name    MAKE_TAG('n','a','m','e')
#define TAG_post    MAKE_TAG('p','o','s','t')
#define TAG_OS2     MAKE_TAG('O','S','/','2')
#define TAG_EBSC    MAKE_TAG('E','B','S','C')
#define TAG_gasp    MAKE_TAG('g','a','s','p')
#define TAG_hdmx    MAKE_TAG('h','d','m','x')
#define TAG_LTSH    MAKE_TAG('L','T','S','H')
#define TAG_PCLT    MAKE_TAG('P','C','L','T')
#define TAG_VDMX    MAKE_TAG('V','D','M','X')
#define TAG_vhea    MAKE_TAG('v','h','e','a')
#define TAG_vmtx    MAKE_TAG('v','m','t','x')
#define TAG_GSUB    MAKE_TAG('G','S','U','B')
#define TAG_GPOS    MAKE_TAG('G','P','O','S')
#define TAG_BASE    MAKE_TAG('B','A','S','E')
#define TAG_JSTF    MAKE_TAG('J','S','T','F')
#define TAG_GDEF    MAKE_TAG('G','D','E','F')
#define TAG_VORG    MAKE_TAG('V','O','R','G')
#define TAG_Icon    MAKE_TAG('I','c','o','n')
#if defined(FS_EDGE_HINTS) || defined(FS_EDGE_RENDER)
#define TAG_adfh    MAKE_TAG('A','D','F','H')
#endif

#ifdef FS_CFFR
#define TAG_cff    MAKE_TAG('C','F','F',' ')
#endif

typedef struct {
    FS_ULONG        tag;            /* eg: 'cmap' */
    FS_ULONG        checkSum;       /* for table */
    FS_ULONG        offset;         /* offset from beginning of font */
    FS_ULONG        length;         /* length (bytes) */
    } TTF_DIR;

typedef struct {
    FS_FIXED        version;       /* 0x10000 (1.0)             */
    FS_USHORT       numTables;     /* number of tables          */
    FS_USHORT       searchRange;   /* (max2 <= numOffsets)*16   */
    FS_USHORT       entrySelector; /* log2(max2 <= numOffsets)  */
    FS_USHORT       rangeShift;    /* numOffsets*16-searchRange */
    TTF_DIR         tables[1];     /* table[numOffsets]         */
    } TTF_HEADER;

typedef struct {
    FS_ULONG        tag;           /* should be 'ttcf'            */
    FS_FIXED        version;       /* 0x10000 (1.0)               */
    FS_ULONG        numFonts;      /* number of fonts in TTC file */
    FS_ULONG        offsets[1];    /* offsets from beginning of file to table directories */
    } TTC_HEADER;

typedef struct {
    FS_FIXED    version;             /* for this table, set to 1.0 */
    FS_FIXED    fontRevision;        /* For Font Manufacturer */
    FS_ULONG    checkSumAdjustment;
    FS_ULONG    magicNumber;         /* signature, should always be 0x5F0F3CF5  == MAGIC */
    FS_USHORT   flags;
    FS_USHORT   unitsPerEm;          /* Specifies how many in Font Units we have per EM */
    FS_ULONG    created[2];
    FS_ULONG    modified[2];
    FS_SHORT    xMin;                /* fontwide bbox */
    FS_SHORT    yMin;
    FS_SHORT    xMax;
    FS_SHORT    yMax;
    FS_USHORT   macStyle;            /* macintosh style word */
    FS_USHORT   lowestRecPPEM;       /* lowest recommended pixels per Em */
    FS_SHORT    fontDirectionHint;   /* 0==mixed, 1==L-R or T-B, -1==R-L or B-T , 2==like 1 + neutrals, -2 like (-1) + neutrals */
    FS_SHORT    indexToLocFormat;    /* 0== short, 1==long */
    FS_SHORT    glyphDataFormat;     /* better be 0 */
    } TTF_HEAD;

#define sizeof_TTF_HEAD_    54  /* ttf size not match of struct size (56) */

typedef struct {
    FS_FIXED    version;             /* for this table, set to 1.0 */
    FS_SHORT    yAscender;
    FS_SHORT    yDescender;
    FS_SHORT    yLineGap;            /* Recommended linespacing = ascender - descender + linegap */
    FS_USHORT   advanceWidthMax;    
    FS_SHORT    minLeftSideBearing;
    FS_SHORT    minRightSideBearing;
    FS_SHORT    xMaxExtent;          /* Max of ( LSBi + (XMAXi - XMINi) ), i loops through all glyphs */
    FS_SHORT    horizontalCaretSlopeNumerator;
    FS_SHORT    horizontalCaretSlopeDenominator;
    FS_SHORT    caretOffset;
    FS_USHORT   reserved1;
    FS_USHORT   reserved2;
    FS_USHORT   reserved3;
    FS_USHORT   reserved4;
    FS_SHORT    metricDataFormat;           /* set to 0 for current format */
    FS_USHORT   numberOf_LongHorMetrics;    /* if format == 0 */
    } TTF_HHEA;

typedef struct {
    FS_FIXED    version;                /* for this table, set to 1.0 */
    FS_SHORT    yAscender;
    FS_SHORT    yDescender;
    FS_SHORT    yLineGap;            
    FS_USHORT   advanceHeightMax;    
    FS_SHORT    minTopSideBearing;
    FS_SHORT    minBottomSideBearing;
    FS_SHORT    yMaxExtent;                
    FS_SHORT    horizontalCaretSlopeNumerator;
    FS_SHORT    horizontalCaretSlopeDenominator;
    FS_SHORT    caretOffset;
    FS_USHORT   reserved1;
    FS_USHORT   reserved2;
    FS_USHORT   reserved3;
    FS_USHORT   reserved4;
    FS_SHORT    metricDataFormat;           /* set to 0 for current format */
    FS_USHORT   numberOf_LongVerMetrics;    /* if format == 0 */
    } TTF_VHEA;

typedef struct {
    FS_FIXED    version;                /* for this table, set to 1.0 */
    FS_USHORT   numGlyphs;              /* number of glyphs in the file ... loca is one larger */
    FS_USHORT   maxPoints;              /* in an individual glyph */
    FS_USHORT   maxContours;            /* in an individual glyph */
    FS_USHORT   maxCompositePoints;     /* in an composite glyph */
    FS_USHORT   maxCompositeContours;   /* in an composite glyph */
    FS_USHORT   maxElements;            /* set to 2, or 1 if no twilightzone points */
    FS_USHORT   maxTwilightPoints;      /* max points in element zero */
    FS_USHORT   maxStorage;             /* max number of storage locations */
    FS_USHORT   maxFunctionDefs;        /* max number of FDEFs in any preprogram */
    FS_USHORT   maxInstructionDefs;     /* max number of IDEFs in any preprogram */
    FS_USHORT   maxStackElements;       /* max number of stack elements for any individual glyph */
    FS_USHORT   maxSizeOfInstructions;  /* max size in bytes for any individual glyph */
    FS_USHORT   maxComponentElements;   /* number of glyphs referenced at top level */
    FS_USHORT   maxComponentDepth;      /* levels of recursion, 1 for simple components */
    } TTF_MAXP;

/*****************************************************************************
 *
 *  Structure that describes the cmap platform and encoding.
 *
 *  Description:
 *      The FONT_METRICS structure returned by FS_font_metrics() contains
 *      an array of these structures.
 *
 *      The platform ID specifies a particular platform which this cmap
 *      complies.  Each platform supports a list of encodings.  For
 *      example the Windows platform = 3.  On that platform the Unicode
 *      encoding is specified as 1.
 *
 *      For more information, see the TrueType specification.
 *
 *  See Also:
 *      FONT_METRICS 
 *
 *  <GROUP itypestructs>
 */
typedef struct 
{
    FS_USHORT   platform;       /* platform ID */
    FS_USHORT   encoding;       /* encoding ID */
    FS_ULONG    offset;         /* offset to data (private) */
} CMAP_TAB;


typedef struct {
    FS_USHORT version;
    FS_USHORT number;
    CMAP_TAB tables[1];                /* really tables[number] */
    /* the subtable data follows here */
    } TTF_CMAP;

#ifdef FS_CFFR 
typedef struct {
#if 0 /* future expansion */
    int numbluevalues;
    int bluevalues[14];
    int numotherblues;
    int otherblues[10];
    int numfamilyblues;
    int familyblues[14];
    int numfamilyotherblues;
    int familyotherblues[10];
    FS_FIXED bluescale;
    int blueshift;
    int bluefuzz;
    FS_FIXED stdHW;
    FS_FIXED stdVW;
    int numsnapstemH;
    FS_FIXED snapstemH[12];
    int numsnapstemV;
    FS_FIXED snapstemV[12];
    int forceBold;
    int languageGroup;
    FS_FIXED expansionFactor;
    int initialRandomSeed;
#endif /* if 0 future expansion */
    int subrsoff;
    int defaultwidthX;
    int nominalwidthX;
} PRIVATEDICT;

# define FS_CFF_FUTURE_USE 0

typedef struct {
#if FS_CFF_FUTURE_USE
    FS_SHORT version;
    FS_SHORT notice;
    FS_SHORT copyright;
    FS_SHORT fullname;
    FS_SHORT familyname;
    FS_SHORT weight;
    FS_BYTE isFixedPitch;
    FS_LONG italicAngle;
    FS_LONG painttype;
    FS_LONG uniqueid;
    FS_LONG strokewidth;
    FS_LONG syntheticbaseindex;
    FS_SHORT psembeddedlanguagecode;
    FS_SHORT basefontname;
    FS_LONG basefontblend;
    FS_SHORT ordering;
    FS_LONG supplement;
    FS_LONG CIDversion;
    FS_LONG CIDrevision;
    FS_LONG CIDfonttype;
    FS_LONG UIDbase;
    FS_SHORT fontname;
    FS_CHAR **nameptrs;
    FS_CHAR **glyphptrs;
#endif
    FS_SHORT registry;
    FS_LONG CIDcount;
    FS_LONG FDarrayoffset;
    FS_LONG FDselectoffset;
    FS_LONG FDselect;
    FS_LONG underlineposition;
    FS_LONG underlinethickness;
    FS_LONG charstringtype;
    FRACT fontmatrix[6];
    FS_LONG fontbbox[4];
    FS_LONG charsetoffset;
    FS_LONG encodingoffset;
    FS_LONG charstringsoffset;
    char * charstringsINDEXptr;
    FS_LONG privatedictoffset;
    FS_LONG privatedictsize;
    FS_SHORT numglyphs;
    FS_USHORT *charset;
    PRIVATEDICT privatedict;    
} CFFTOPDICT;

typedef struct {
  FS_SHORT max_points;
  FS_SHORT max_contours;
  FS_LONG nameINDEXoffset;
  FS_BYTE *topdictINDEXptr;
  FS_LONG topdictINDEXoffset;
  FS_LONG stringINDEXoffset;
  FS_BYTE *globalsubrINDEXptr;
  FS_LONG globalsubrINDEXoffset;
  char *charstringINDEXptr;
  FS_LONG charstringINDEXoffset;
  CFFTOPDICT topdict;
} CFF_TAB;
#endif /* FS_CFFR */

/* a structure for passing metrics around */
typedef struct {
    F26DOT6 lsb_x;
    F26DOT6 lsb_y;
    F26DOT6 rsb_x;
    F26DOT6 rsb_y;
    FS_SHORT lsb;    /* unscaled */
    FS_SHORT aw;     /* unscaled */

    F26DOT6 tsb_x;
    F26DOT6 tsb_y;
    F26DOT6 bsb_x;
    F26DOT6 bsb_y;
    FS_SHORT tsb;    /* unscaled */
    FS_SHORT ah;     /* unscaled */

    FS_BOOLEAN useMyMetrics;
} fsg_Metrics;

/* buffer size limits for TTF_CMAP and TTF_NAME structs */
#define MAX_MAPPINGS_LEN            20
#define MAX_FONT_NAME_LEN           50
#define MAX_FONT_FAMILY_NAME_LEN    50
#define MAX_COPYRIGHT_LEN           2048

/* "name" table offsets */
#define NAME_NUM_REC        2
#define NAME_OFF_TO_STRS    4
#define NAME_NAMERECS       6
#define NAME_SIZE_NAMEREC   12
#define NAME_TAB_PLATID     0
#define NAME_TAB_SPECID     2
#define NAME_TAB_LANGID     4
#define NAME_TAB_NAMEID     6
#define NAME_TAB_STRLEN     8
#define NAME_TAB_STROFF     10

#define MSFT_ENC           3         /* MSFT format platform ID */
#define MSFT_US_ENGL       0x0409    /* MSFT format language ID in name table */
#define MAC_ENC            1         /* MAC format platform ID */
#define MAC_US_ENGL        0         /* MAC format language ID in name table */

#ifdef FS_HINTS
/* script numbers */
#define SCRIPT_OTHER            0
#define SCRIPT_LATIN            1
#define SCRIPT_GREEK            2
#define SCRIPT_CYRILLIC         3
#define SCRIPT_HEBREW           4
#define SCRIPT_ARABIC           5
#define SCRIPT_HANGUL           6
#define SCRIPT_CJK              7
#define SCRIPT_DEVANAGARI       8
#define SCRIPT_BENGALI          9
#define SCRIPT_GURMUHKI         10
#define SCRIPT_TELUGU           11
#define SCRIPT_KANNADA          12
#define SCRIPT_MALAYALAM        13
#define SCRIPT_TAMIL            14
#define SCRIPT_ORIYA            15
#define SCRIPT_SINHALA          16
#define SCRIPT_THAI             17
#define SCRIPT_KHMER            18
#define SCRIPT_GUJARATI         19

/* rtgah reflines */
#define RTGA_CAP_SQUARE         0
#define RTGA_CAP_ROUND          1
#define RTGA_LC_SQUARE          2
#define RTGA_LC_ROUND           3
#define RTGA_BASE_SQUARE        4
#define RTGA_BASE_ROUND         5
#define RTGA_l_LSB              6
#define RTGA_l_RSB              7
#define RTGA_CJK_TOP            8
#define RTGA_CJK_BOT            9
#define RTGA_CJK_PTOP           10
#define RTGA_CJK_PBOT           11
#define RTGA_HANGUL_TOP         12
#define RTGA_HANGUL_BOT         13
#define RTGA_HANGUL_PTOP        14
#define RTGA_HANGUL_PBOT        15
#define RTGA_DEVANAGARI_TOP     16
#define RTGA_DEVANAGARI_BOT     17
#define RTGA_BENGALI_TOP        18
#define RTGA_BENGALI_BOT        19
#define RTGA_GURMUKHI_TOP       20
#define RTGA_GURMUKHI_BOT       21

#define RTGA_TELUGU_TOP         22
#define RTGA_TELUGU_BOT         23

#define RTGA_MALAYALAM_CAP_ROUND   24
#define RTGA_MALAYALAM_CAP_SQUARE  25
#define RTGA_MALAYALAM_BASE_ROUND  26
#define RTGA_MALAYALAM_BASE_SQUARE 27

#define RTGA_TAMIL_CAP_ROUND       28
#define RTGA_TAMIL_CAP_SQUARE      29
#define RTGA_TAMIL_BASE_ROUND      30
#define RTGA_TAMIL_BASE_SQUARE     31

#define RTGA_ORIYA_CAP_ROUND       32
#define RTGA_ORIYA_CAP_SQUARE      33
#define RTGA_ORIYA_BASE_ROUND      34
#define RTGA_ORIYA_BASE_SQUARE     35

#define RTGA_SINHALA_CAP_ROUND     36
#define RTGA_SINHALA_CAP_SQUARE    37
#define RTGA_SINHALA_BASE_ROUND    38
#define RTGA_SINHALA_BASE_SQUARE   39

#define RTGA_THAI_CAP_ROUND        40
#define RTGA_THAI_CAP_SQUARE       41
#define RTGA_THAI_BASE_ROUND       42
#define RTGA_THAI_BASE_SQUARE      43

#define RTGA_KHMER_CAP_ROUND       44
#define RTGA_KHMER_BASE_ROUND      45
#define RTGA_KHMER_BASE_SQUARE     46

#define RTGA_GUJARATI_CAP_ROUND    47
#define RTGA_GUJARATI_CAP_SQUARE   48
#define RTGA_GUJARATI_BASE_ROUND   49
#define RTGA_GUJARATI_BASE_SQUARE  50

#define RTGAH_ARABIC_BOT           51
#define RTGA_BEEN_THERE            52
#define RTGA_COUNT                 56

#endif

typedef enum {
    name_Copyright,
    name_Family,
    name_Subfamily,
    name_UniqueName,
    name_FullName,
    name_Version,
    name_Postscript,
    name_Trademark
    } sfnt_NameIndex;

typedef struct {
    FILECHAR font_name[MAX_FONT_NAME_LEN];
    FS_BOOLEAN font_name_too_long;
    FILECHAR font_family_name[MAX_FONT_FAMILY_NAME_LEN];
    FS_BOOLEAN font_family_name_too_long;
    FILECHAR unique_name[MAX_FONT_NAME_LEN];
    FS_BOOLEAN unique_name_too_long;
    FILECHAR copyright[MAX_COPYRIGHT_LEN];
    FS_BOOLEAN copyright_too_long;
    } TTF_NAME;        /* abbreviated contents */

typedef struct {

    FS_FIXED version;
    FS_FIXED italicAngle;
    FS_SHORT underlinePosition;     /* in design units */
    FS_SHORT underlineThickness;    /* in design units */
    FS_ULONG isFixedPitch;
    FS_ULONG minMemType42;
    FS_ULONG maxMemType42;
    FS_ULONG minMemType1;
    FS_ULONG maxMemType1;

    /* versions 1.0 and 3.0 tables end here. */

    /* version 2.0 adds the following to the 1.0/3.0 version:

        FS_USHORT numberOfGlyphs;
        FS_USHORT glyphNameIndex[numGlyphs];
        FILECHAR  names[numberNewGlyphs];
     */

    /* version 2.5 (deprecated) adds the following to the 1.0/3.0 version:

        FS_USHORT numberOfGlyphs;
        FILECHAR  offset[numGlyphs];
     */

} TTF_POST;

typedef struct {
    FS_USHORT   version;
    FS_SHORT    xAvgCharWidth;
    FS_USHORT   usWeightClass;
    FS_USHORT   usWidthClass;
    FS_SHORT    fsType;
    FS_SHORT    ySubscriptXSize;
    FS_SHORT    ySubscriptYSize;
    FS_SHORT    ySubscriptXOffset;
    FS_SHORT    ySubscriptYOffset;
    FS_SHORT    ySuperscriptXSize;
    FS_SHORT    ySuperscriptYSize;
    FS_SHORT    ySuperscriptXOffset;
    FS_SHORT    ySuperscriptYOffset;
    FS_SHORT    yStrikeoutSize;
    FS_SHORT    yStrikeoutPosition;
    FS_SHORT    sFamilyClass;
    FS_BYTE     panose[10];
    FS_ULONG    ulUnicodeRange1;    /* Bits 0-31 */
    FS_ULONG    ulUnicodeRange2;    /* Bits 32-63 */
    FS_ULONG    ulUnicodeRange3;    /* Bits 64-95 */
    FS_ULONG    ulUnicodeRange4;    /* Bits 96-127 */
    FILECHAR    achVendID[4];
    FS_USHORT   fsSelection;
    FS_USHORT   usFirstCharIndex;
    FS_USHORT   usLastCharIndex;
    FS_SHORT    sTypoAscender;
    FS_SHORT    sTypoDescender;
    FS_SHORT    sTypoLineGap;
    FS_USHORT   usWinAscent;
    FS_USHORT   usWinDescent;
    /* version 1 additions */
    FS_ULONG    ulCodePageRange1;    /* Bits 0-31 */
    FS_ULONG    ulCodePageRange2;    /* Bits 32-63 */
    /* version 2 additions */
    FS_SHORT    sxHeight;
    FS_SHORT    sCapHeight; 
    FS_USHORT   usDefaultChar;
    FS_USHORT   usBreakChar;
    FS_USHORT   usMaxContext; 
    } TTF_OS2;

#ifdef FS_CCC
typedef struct {
    FS_SHORT numBits_LOCA;  /* number of bits used for LOCA */
    FS_SHORT minVal_LSB, numBits_LSB;    /* mimium value for LSB and number of bits for LSB */
    FS_SHORT minVal_AW, numBits_AW;    /* mimium value for AW and number of bits for AW */
    FS_SHORT minVal_TSB, numBits_TSB;    /* mimium value for TSB and number of bits for TSB */
    FS_SHORT minVal_AH, numBits_AH;    /* mimium value for AW and number of bits for AH */
    FS_ULONG numBits_GLYF;                /* number of parts for GLYF */
    FS_BOOLEAN cccVersion7;            /* true if cccVersion 7 */
    FS_BOOLEAN cccVersion8;            /* true if cccVersion 8 */
    FS_SHORT   numBits_SF;    /* number of bits used for scale factors, 10 for v7, 16 for v8 */
    } CCC_INFO;
#endif


#ifdef FS_EMBEDDED_BITMAP
typedef struct {
    FS_TINY    ascender;
    FS_TINY    descender;
    FS_BYTE    widthMax;
    FS_TINY    caretSlopeNumerator;
    FS_TINY    caretSlopeDenominator;
    FS_TINY    charetOffset;
    FS_TINY    minOriginSB;
    FS_TINY    minAdvanceSB;
    FS_TINY    maxBeforeBL;
    FS_TINY    minAfterBL;
    FS_TINY    pad1;
    FS_TINY    pad2;
    } SBIT_LINE_METRICS;    /* Moved from SBIT.C */

typedef struct {
    FS_ULONG    indexSubTableArrayOffset;
    FS_ULONG    indexTableSize;
    FS_ULONG    numberOfIndexSubTables;
    FS_ULONG    colorRef;
    SBIT_LINE_METRICS hori;
    SBIT_LINE_METRICS vert;
    FS_USHORT    startGlyphIndex;
    FS_USHORT    endGlyphIndex;
    FS_BYTE      ppmX;
    FS_BYTE      ppmY;
    FS_BYTE      bitDepth;        /* we use 1 and 4 */
    FS_BYTE      flags;
    } BITMAP_SIZE_TABLE;    /* Moved from SBIT.C */

typedef struct {
    FS_USHORT    firstGlyphIndex;
    FS_USHORT    lastGlyphIndex;
    FS_ULONG additionalOffsetToIndexSubTable;
    } INDEX_SUB_TABLE;    /* Moved from SBIT.C */

typedef struct {
    BITMAP_SIZE_TABLE bst;    /* bitmapSizeTable */
    INDEX_SUB_TABLE *ist;     /* corresponding indexSubTable[] */
    } BITMAP_SIZE_TABLE_ARRAY;

    /* The following describes only a portion of the EBLC table */ 
typedef struct {
    FS_FIXED                version;    /* 0x20000 (2.0) */
    FS_ULONG                numSizes;   /* number of bitmapSizeTables */
    BITMAP_SIZE_TABLE_ARRAY *bsta;      /* BITMAP_SIZE_TABLE_ARRAY[numSizes] */
    } TTF_EBLC_PARTIAL;
#endif    /* FS_EMBEDDED_BITMAP */

typedef struct {
    FS_LONG start;       /* offset to first instruction */
    FS_USHORT length;    /* number of bytes to execute <4> */
    FS_USHORT pgmIndex;  /* index to appropriate preprogram for this func (0..1) */
} fnt_funcDef;

typedef struct {
    FS_LONG start;
    FS_USHORT length;
    FS_BYTE  pgmIndex;
    FS_BYTE  opCode;
} fnt_instrDef;

typedef struct {
    FS_USHORT startglyphid;
    FS_USHORT endglyphid;
    FS_USHORT index;
} RANGEINFO;

typedef struct {
    FS_USHORT cvtindex;
    FS_SHORT adjustment;
} DELTAINFO;


typedef struct {
    FILECHAR        *path;                  /* if a disk file .. the path */
    FS_BYTE         *memptr;                /* else ROM/RAM location */
    FS_FILE         *fp;                    /* fp = FS_open(path) for disk file */
    TTC_HEADER      *ttc_header;            /* NULL if vanilla TTF */
    FS_ULONG        ttf_offset;             /* offset to TTF in TTC */
    TTF_HEADER      *ttf_header;            /* for current (sub) font */
    TTF_HEAD        *head;                  /* 'head' table pointer */
    TTF_HHEA        *hhea;                  /* 'hhea' table pointer */
    TTF_VHEA        *vhea;                  /* 'vhea' table pointer */
    TTF_MAXP        *maxp;                  /* 'maxp' table pointer */
    TTF_CMAP        *cmap;                  /* 'cmap' table pointer - MOTOROLA BYTE ORDER */
    FS_ULONG        own_cmap;               /* flag to indicate whether cmap has been allocated */
    FS_BYTE         *prep;                  /* 'prep' table  pointer */
    FS_LONG         num_prep;
    FS_BYTE         *fpgm;                  /* 'fpgm' table pointer */
    FS_LONG         num_fpgm;
    FS_SHORT        *cvt;                   /* 'cvt' table pointer */
    FS_LONG         num_cvt;
    FS_ULONG        *loca;                  /* 'loca' table pointer */
    FS_ULONG        loca_offset;
#ifdef FS_SPEED_OVER_MEMORY
    FS_ULONG        *hmtx;                  /* 'hmtx' table pointer */
    FS_ULONG        *vmtx;                  /* 'vmtx' table pointer */
#endif
    FS_ULONG        glyf_size;
    FS_ULONG        glyf_offset;            /* offset to "glyf" table */
    FS_ULONG        eblc_offset;            /* offset to "EBLC" table */
    FS_ULONG        kern_offset;            /* offset to 'kern' table */
    FS_ULONG        hmtx_offset;            /* offset to "hmtx" table */
    FS_ULONG        vmtx_offset;            /* offset to "vmtx" table */
    FS_ULONG        icon_offset;            /* offset to "Icon" table */
    TTF_OS2         *os2;                   /* for current (sub) font */
    TTF_NAME        *name;                  /* for current (sub) font - (abbreviated) */
    FS_VOID         *decomp;                /* (for ACT3 fonts) MTX_RA_TT_Decomp */
    FS_USHORT       *nstk;                  /* array of non-stik indices */
#ifdef FS_CCC
    CCC_INFO        ccc_info;
#endif
#ifdef FS_EMBEDDED_BITMAP
    TTF_EBLC_PARTIAL    *ttf_EBLC_partial;  /* 'EBLC' table pointer */
#endif    /* FS_EMBEDDED_BITMAP */
    FS_ULONG        hdmx_offset;
    FS_ULONG        vdmx_offset;
    FS_ULONG        data_offset;            /* offset (in bytes) from start of file to font */
    FS_ULONG        data_length;            /* size (in bytes) of font within file */
    fnt_funcDef     *functionDefs;          /* offsets into the Font ProGraM */
    fnt_instrDef    *instructionDefs;
#ifdef FS_ICONS
    FS_ULONG        num_icons;              /* number of icons in the font              */
    FS_ULONG        icon_first;             /* first index of icons                     */
    FS_ULONG        icon_last;              /* last index of icons                      */ 
#endif /* FS_ICONS */
#if defined(FS_EDGE_HINTS) || defined(FS_EDGE_RENDER)
    FS_BYTE         *adfh;                  /* 'ADFH' table pointer */
#endif /* FS_EDGE_HINTS or FS_EDGE_RENDER */
#ifdef GASP_ADFH_CONTROL
    FS_BYTE        *gasp;
#endif    

#ifdef FS_CFFR
    CFF_TAB         *cff;
    FS_ULONG        cff_offset;
#endif /* FS_CFFR */
    } TTF;

typedef FS_BYTE KSTf0_data;                 /* Kern Sub-Table format 0 data */

/*************** internal font flags *****************************/
#define FONTFLAG_STIK              (FS_ULONG)0x00000002
#define FONTFLAG_ACT3              (FS_ULONG)0x00000004
#define FONTFLAG_CCC               (FS_ULONG)0x00000008
#define FONTFLAG_DDD               (FS_ULONG)0x00000010
#define FONTFLAG_NEW_AA_ON         (FS_ULONG)0x00000040
#define FONTFLAG_DIRECT_DRAW_ON    (FS_ULONG)0x00000080
#define FONTFLAG_EDGE              (FS_ULONG)0x00000100
#define FONTFLAG_IS_ITALIC         (FS_ULONG)0x00000200

#define RTGAH_NOPE   0 /* indicates not suitable for RTGAH (in sfnt)  */
#define RTGAH_YES    1 /* indicates font suitable for RTGAH (in sfnt) */

/************* loaded/available fonts ***************************/
typedef struct lfnt_ {
    FILECHAR   *name;               /* the proper name of the font              */
    FILECHAR   *path;               /* if on DISK ... the complete path name    */
    FS_BYTE    *memptr;             /* else if in RAM/ROM ... the address       */
    FS_ULONG    index;              /* index of <name> in a TT collection       */
    FS_ULONG    data_offset;        /* offset in bytes to font data in file     */
    FS_ULONG    data_length;        /* size in bytes of font data in file       */
    FS_ULONG    fontflags;          /* font specific control flags              */
#ifdef FS_CACHE_CMAP
    FS_CMAP_CACHE *cmap_cache;      /* cmap cache unicode & glyph index values  */
    FS_USHORT      cmap_cache_mod;
#endif
    FS_ULONG    fntset_refs;        /* number of fontsets referencing this font */
    FS_VOID    *fnt;                /* if loaded ... here's the essential data  */
    FS_BOOLEAN  loading;            /* boolean -- protect from get_some_back    */
    FS_BYTE     fnt_type;           /* specifies ttf or pfr etc.                */
    KSTf0_data *kerning;            /* the address to the kerning data          */
                                    /* (if FS_get_kerning() is called)          */
    struct lfnt_ *next;             /* linked list goo                          */
} LFNT;

#if defined(FS_MULTI_PROCESS) || defined(FS_MAPPED_FONTS)
/************* loaded/available file-mapped fonts ***************************/
typedef struct mappedfnt_ {
    FILECHAR    *path;              /* on DISK (by definition) ... the complete path name */
    FS_BYTE     *memptr;            /* starting address of the mapped file */
    FS_VOID     *hfnt;              /* handle to mapped file */
    FS_ULONG    mapped_length;      /* size in bytes of the mapping */
    struct mappedfnt_ *next;        /* linked list goo */
    } MAPPEDFNT;
#endif    /* FS_MULTI_PROCESS */

/************* scaled font enviromnent **************************/
typedef struct senv_ {
    FS_USHORT xppm;            /* rotationally invariant x size        */
    FS_USHORT yppm;            /* rotationally invariant y size        */ 
    FS_BOOLEAN vanilla;        /* ? no rotation, skewing, or mirroring */
    FS_ULONG hdmx_group_offset; /* hdmx group offset for current xppm(for vanilla case) */
#ifdef GASP_ADFH_CONTROL
    FS_BYTE render_behavior; 
#endif
#ifdef FS_STIK
    FS_SHORT stroke_width;
    /*  following unused with AA autohinter */
    FS_SHORT language;    
    FS_FIXED cap_round;        /* for stik font autohinter */
    FS_FIXED cap_square;
    FS_FIXED x_round;
    FS_FIXED x_square;
    FS_FIXED base_round;
    FS_FIXED base_square; 
    FS_FIXED lsb;
    FS_FIXED rsb;
#endif
#ifdef FS_PSEUDO_BOLD
    FS_SHORT bold_width;
#endif

    FS_VOID *ttkey;            /* the TT scale information */
                               /* (fsg_SplineKey *)        */
    } SENV;


/************* scaled font instance *****************************/
/* so fonts with different SCALE's can share the same LFNT      */
typedef struct sfnt_ {
    LFNT *lfnt;             /* the loaded font for this sfnt          */
    FS_FIXED user_scale[4]; /* scale matrix in 1/65536-the of PIXELS  */
    FS_SHORT vertical_shift;/* component font baseline shift          */
    FS_FIXED stroke_pct;    /* distinguishes sfnt for caching         */
    FS_FIXED bold_pct;      /* distinguishes sfnt for caching         */
    SENV *senv;             /* scale enviromnent (guts)               */
#ifdef FS_HINTS
    FS_USHORT rtgah_suitable;     /* flag to indicate RTGAH suitability     */
    F26DOT6 ref_lines[RTGA_COUNT]; /* RTGAH ref lines (never gutted)  */
#endif
    FS_ULONG ref_count;     /* number of clients with this as current */
    FS_ULONG cache_count;   /* number of things in cache              */
    FS_ULONG active_count;  /* number of ACTIVE things in cache       */
    struct sfnt_ *next;     /* linked list goo                        */
    } SFNT;

typedef struct {
    FS_ULONG range1;       /* mask indicating Unicode ranges which  */
    FS_ULONG range2;       /* are potentially available in the font */
    } UniRange;

/************* component font properties *************************/
/* so different fonts sets can share a loaded font but have      */
/* different properties for the shared component font            */
typedef struct cfnt_ {
    FILECHAR   *name;
    UniRange    bmp;
    UniRange    smp;
    UniRange    sip;
    UniRange    ssp;
    FS_USHORT   start_index;  /* glyph index start for component  */
    FS_BOOLEAN  embolden;     /* flag to control pseudo-embolden  */
    FS_BOOLEAN  italicize;    /* flag to control pseudo-italics   */
    FS_FIXED    bold_pct;     /* component bold percentage        */
    FS_FIXED    italic_angle; /* component pseudo-italics angle   */
    LFNT       *lfnt;         /* loaded font for this component   */
    FS_ULONG    adjOffset;    /* offset into LTT of scale/shift adjustments */
    FS_USHORT   numAdjust;    /* number of adjustment triplets */
    } CFNT;

/**************** font set ot table ******************************/
typedef struct {
        FS_ULONG offset;
        FS_ULONG size;
        } FNTSET_OT_TABLE;

/**************** font set instance ******************************/
typedef struct fntset_ {
    FILECHAR   *name;        /* the name of the font set              */
    FILECHAR   *path;        /* if on DISK ... the complete path name */
    FS_BYTE    *memptr;      /* else if in RAM/ROM ... the address    */
    FS_ULONG    data_offset; /* data offset within resource           */
    FS_ULONG    data_length; /* data size within resource             */
    FS_USHORT   num_fonts;   /* number of fonts in set                */
    CFNT       *cfnt;        /* array of component fonts              */
    LFNT       *metric_font; /* lfnt of metric font                   */

    FS_ULONG    name_offset;
    FS_ULONG    name_size;
    FS_ULONG    maxp_offset;
    FS_ULONG    os_2_offset;
    FS_ULONG    head_offset;
    FS_ULONG    hhea_offset;
    FS_ULONG    vhea_offset;
    FS_ULONG    post_offset;

    FNTSET_OT_TABLE *cmap;

    FNTSET_OT_TABLE *gdef;
    FNTSET_OT_TABLE *gsub;
    FNTSET_OT_TABLE *gpos;

    FS_ULONG    ref_count;   /* number of clients who have this FNTSET as current font set */
    FS_ULONG    count_added; /* number of clients who have loaded/added this fontset */

    FS_ULONG    checksum;    /* used to check whether a memptr points to this fontset */

    struct fntset_ *next;    /* linked list goo                       */
    } FNTSET;

/************* scaled component font properties ******************/
/* this object is the client's view of a scaled component font   */
typedef struct tfnt_ {
    /* following CFNT pointer makes this object more indepedent  */
    CFNT       *cfnt;         /* copy of FNTSET's cfnt pointer   */
    SFNT       *sfnt;         /* scaled component font           */
    FS_ULONG    cmap_offset;  /* offset to fonts cmap for speed  */
#ifdef FS_EDGE_RENDER
    ADFRenderAttrs adfRenderAttrs;/* Edge(TM) rendering attributes*/
#endif
    } TFNT;

/************* typeset properties ***********************************/
/* this object tracks the font, cmap, and scaled font set by        */
/* the client. It is an array to handle linked fonts.               */
/* Since Edge(TM) rendering attributes are font and scale dependent */
/* these attributes are stored for each font here.                  */
typedef struct typeset_ {
    FNTSET      *fntset;      /* associated font set                */
    FS_USHORT    capacity;    /* size of TFNT array                 */
    TFNT       * tfntarray;   /* array (fntset->num_fonts valid)    */
    } TYPESET;

/*************** cache ******************************************/
/* if cache is VERY large, you may wish to increase this to
* improve cache performance. The value must be a prime number
* less than 65536
*/
#define CACHE_MOD 1021

/* the largest N prime numbers < 32768  (Knuth V2 p390) */
#define CACHE_OUTLINE        (FS_USHORT)(32768L - 19)
#define CACHE_GRAYMAP        (FS_USHORT)(32768L - 49)
#define CACHE_GRAYMAP2       (FS_USHORT)(32768L - 51)
#define CACHE_GRAYMAP8       (FS_USHORT)(32768L - 55)
#define CACHE_BITMAP         (FS_USHORT)(32768L - 61)
#define CACHE_PHASE0         (FS_USHORT)(32768L - 75)
#define CACHE_PHASE1         (FS_USHORT)(32768L - 81)
#define CACHE_PHASE2         (FS_USHORT)(32768L - 115)
#define CACHE_PHASE3         (FS_USHORT)(32768L - 121)
#define CACHE_ICON           (FS_USHORT)(32768L - 135)
#define CACHE_ADF_GRAYMAP    (FS_USHORT)(32768L - 147)
#define CACHE_ADF_GRAYMAP2   (FS_USHORT)(32768L - 157)
#define CACHE_ADF_GRAYMAP8   (FS_USHORT)(32768L - 159)
#define CACHE_ADVANCE_GMAP2  (FS_USHORT)(32768L - 165)
#define CACHE_ADVANCE_GMAP4  (FS_USHORT)(32768L - 181)
#define CACHE_ADVANCE_GMAP8  (FS_USHORT)(32768L - 189)
#define CACHE_ADVANCE_BMAP   (FS_USHORT)(32768L - 195)
#define CACHE_ADVANCE_ADF2   (FS_USHORT)(32768L - 199)
#define CACHE_ADVANCE_ADF4   (FS_USHORT)(32768L - 205)
#define CACHE_ADVANCE_ADF8   (FS_USHORT)(32768L - 207)
#define CACHE_VGPATHUNSCALED (FS_USHORT)(32768L - 231)
#define CACHE_VGPATHSCALED   (FS_USHORT)(32768L - 235)
#define CACHE_VGBITMAP       (FS_USHORT)(32768L - 237)
#define CACHE_VGGRAYMAP      (FS_USHORT)(32768L - 261)
#define CACHE_VGEDGEMAP      (FS_USHORT)(32768L - 265)

typedef struct cache_entry_ {
#ifndef FS_NO_FS_LUMP
    FS_MASTER_LUMP *master;      /* MUST BE FIRST IN STRUCTURE!!! */
#endif

    FS_USHORT       type;        /* CACHE_OUTLINE, CACHE_BITMAP, CACHE_GRAYMAP, etc */
    FS_USHORT       index;       /* glyph index of the <data> */
    FS_ULONG        flags;       /* user accessible flags */
    FS_BYTE        *data;        /* the FS_OUTLINE*, FS_BITMAP*, or FS_GRAYMAP* */
    SFNT           *sfnt;        /* the scaled font it came from */
    FS_FIXED        outline_opacity;
    FS_USHORT       outline_width;
#ifdef FS_STIK
    FS_SHORT        stroke_width;
#endif /* FS_STIK */

#ifdef FS_PSEUDO_BOLD
    FS_SHORT        bold_width;
#endif /* FS_PSEUDO_BOLD */

#ifdef FS_EDGE_RENDER
    ADF_F32        outsideCutoff;   /* Outside cutoff value for Edge(TM) CSM */
    ADF_F32        insideCutoff;    /* Inside cutoff value for Edge(TM) CSM  */
    ADF_F32        gamma;           /* Gamma curve exponent for Edge(TM) CSM */
#endif

    struct cache_entry_ *prev,*next;
    FS_ULONG ref_counter;
    FS_BYTE any_hints; /* added for reporting hints info. */
    } CACHE_ENTRY;


/************* user accessible flags *********************************/

/* user accessible interpreter flags */
#define FLAG_DO_DROPOUTS    (FS_ULONG)0x00000001
#define FLAGS_DROPOUTS_ON   FLAG_DO_DROPOUTS
#define FLAGS_DROPOUTS_OFF  (FS_ULONG)(~FLAGS_DROPOUTS_ON)
#define FLAGS_NO_HINTS      (FS_ULONG)0x00000002
#define FLAGS_HINTS_OFF     FLAGS_NO_HINTS
#define FLAGS_HINTS_ON      (FS_ULONG)(~FLAGS_HINTS_OFF)

/* user accessible bitmap/graymap flags */
#define FLAGS_EMBOSSED           (FS_ULONG)0x00000004
#define FLAGS_ENGRAVED           (FS_ULONG)0x00000008
#define FLAGS_OUTLINED           (FS_ULONG)0x00000010
#define FLAGS_OUTLINED_1PIXEL    (FS_ULONG)0x00000010    /* synonym, for backwards compatibility */
#define FLAGS_OUTLINED_2PIXEL    (FS_ULONG)0x00000020
#define FLAGS_OUTLINED_FILLED    (FS_ULONG)0x00000040
#define FLAGS_OUTLINED_UNFILLED  (FS_ULONG)0x00000080
#define FLAGS_OUTLINED_SOFT      (FS_ULONG)0x00000100
#define FLAGS_ADD_WEIGHT         (FS_ULONG)0x00000200
#define FLAGS_ADD_WEIGHT_ON      FLAGS_ADD_WEIGHT
#define FLAGS_ADD_WEIGHT_OFF     (FS_ULONG)(~FLAGS_ADD_WEIGHT_ON)
#define FLAGS_REGULARBOLD        (FS_ULONG)0x00000400
#define FLAGS_REGULARBOLD_ON     FLAGS_REGULARBOLD
#define FLAGS_REGULARBOLD_OFF    (FS_ULONG)(~FLAGS_REGULARBOLD)
#define FLAGS_FRACTBOLD          (FS_ULONG)0x00000800
#define FLAGS_FRACTBOLD_ON       FLAGS_FRACTBOLD
#define FLAGS_FRACTBOLD_OFF      (FS_ULONG)(~FLAGS_FRACTBOLD)
#define FLAGS_OUTLINEBOLD        (FS_ULONG)(FLAGS_REGULARBOLD | FLAGS_FRACTBOLD)
#define FLAGS_NO_EFFECT          (FS_ULONG)(~( FLAGS_EMBOSSED | FLAGS_ENGRAVED \
                                         | FLAGS_OUTLINED_1PIXEL | FLAGS_OUTLINED_2PIXEL \
                                         | FLAGS_OUTLINED_UNFILLED | FLAGS_OUTLINED_FILLED \
                                         | FLAGS_OUTLINED_SOFT))
/* user accessible graymap flags */
#define FLAGS_SOFTENED      (FS_ULONG)0x00001000
#define FLAGS_SOFTENED_ON   FLAGS_SOFTENED
#define FLAGS_SOFTENED_OFF  (FS_ULONG)(~FLAGS_SOFTENED_ON)

#define FLAGS_PHASED        (FS_ULONG)0x00002000
#define FLAGS_PHASED_ON     FLAGS_PHASED
#define FLAGS_PHASED_OFF    (FS_ULONG)(~FLAGS_PHASED_ON)
#define FLAGS_NO_GRAYMAP_EFFECT (FS_ULONG)(~(FLAGS_SOFTENED_ON | FLAGS_PHASED_ON))

/* user accessible CMAP control flags */
#define FLAGS_CMAP_OFF    (FS_ULONG)0x00004000
#define FLAGS_CMAP_ON     (FS_ULONG)(~FLAGS_CMAP_OFF)

/* user accessible check contour winding flags */
#define FLAGS_CHECK_CONTOUR_WINDING      (FS_ULONG)0x00008000
#define FLAGS_CHECK_CONTOUR_WINDING_ON   FLAGS_CHECK_CONTOUR_WINDING
#define FLAGS_CHECK_CONTOUR_WINDING_OFF  (FS_ULONG)(~FLAGS_CHECK_CONTOUR_WINDING_ON)

/* user accessible vertical writing flags */
#define FLAGS_VERTICAL_ON     (FS_ULONG)0x00010000
#define FLAGS_VERTICAL_OFF    (FS_ULONG)(~FLAGS_VERTICAL_ON)

/* user accessible vertical writing flag */
#define FLAGS_VERTICAL_ROTATE_RIGHT_ON    (FS_ULONG)0x00020000
#define FLAGS_VERTICAL_ROTATE_RIGHT_OFF   (FS_ULONG)(~FLAGS_VERTICAL_ROTATE_RIGHT_ON)

/* user accessible vertical writing flag */
#define FLAGS_VERTICAL_ROTATE_LEFT_ON     (FS_ULONG)0x00040000
#define FLAGS_VERTICAL_ROTATE_LEFT_OFF    (FS_ULONG)(~FLAGS_VERTICAL_ROTATE_LEFT_ON)

/* user accessible ADF flags */
#define FLAGS_DEFAULT_CSM_ON  (FS_ULONG)0x00080000
#define FLAGS_DEFAULT_CSM_OFF (FS_ULONG)(~FLAGS_DEFAULT_CSM_ON)

#define FLAGS_MAZ_ON          (FS_ULONG)0x00100000
#define FLAGS_MAZ_OFF         (FS_ULONG)(~FLAGS_MAZ_ON)

#define FLAGS_ARABIC_AUTOHINT_ON    (FS_ULONG)0x00200000
#define FLAGS_ARABIC_AUTOHINT_OFF   (FS_ULONG)(~FLAGS_ARABIC_AUTOHINT_ON)

/* user accessible grayscale rendering flag */
#define FLAGS_GRAYSCALE       (FS_ULONG)0x00400000
#define FLAGS_GRAYSCALE_ON    FLAGS_GRAYSCALE
#define FLAGS_GRAYSCALE_OFF   (FS_ULONG)(~FLAGS_GRAYSCALE_ON)

/* user accessible autohinting flags */
#define FLAGS_AUTOHINT_OFF   (FS_ULONG) 0x00800000
#define FLAGS_AUTOHINT_ON    (FS_ULONG)(~FLAGS_AUTOHINT_OFF)
#define FLAGS_FORCE_AUTOHINT (FS_ULONG) 0x01000000
#define FLAGS_FORCE_AUTOHINT_OFF (FS_ULONG)(~FLAGS_FORCE_AUTOHINT)
#define FLAGS_AUTOHINT_YONLY_ON  (FS_ULONG) 0x02000000
#define FLAGS_AUTOHINT_YONLY_OFF (FS_ULONG)(~FLAGS_AUTOHINT_YONLY_ON)

/* internal flags */
#define FLAGS_DO_PHASE        (FS_ULONG)0x04000000
#define FLAGS_NO_PHASE        (FS_ULONG)(~FLAGS_DO_PHASE)

#define FLAGS_GRAY_LIMIT      (FS_ULONG)0x08000000
#define FLAGS_FIX_DROPOUTS    (FS_ULONG)0x10000000
#define FLAGS_RTGAH_REF       (FS_ULONG)0x20000000


/* Major and Minor versions: note that the MINOR_VERSION value is 
   (released minor version * 100 + patch release number), 
   to allow for numbering of interim patch releases */
/* i.e. this version is 5.0.0 */
#define MAJOR_VERSION 5
#define MINOR_VERSION 100
/* Version in string form */
#define FS_VERSION_STRING "5.1.0"

/* different private values for ttf->head->glyphDataFormat */
#define STIK_FORMAT (FS_SHORT)(0x9654)

#define OFFLINE_STIK_FORMAT (FS_SHORT)(0x9655)
#define OFFLINE_CCC_FORMAT  (FS_SHORT)(0x0602)

/* version8 CCC format: legacy 0x020x, offlined 0x060x */
#define STIK_FORMAT_CCC(a)    ( ( ( a >= (FS_SHORT)(0x0200) ) &&  ( a <= (FS_SHORT)(0x02FF) ) ) \
                                || \
                                ( ( a >= (FS_SHORT)(0x0602) ) &&  ( a <= (FS_SHORT)(0x06FF) ) ) )
#define TTF_FORMAT_CCC(a)   ( ( a >= (FS_SHORT)(0x0400) ) &&  ( a <= (FS_SHORT)(0x04FF) ) )
#define TTF_FORMAT_DDD(a)   ( ( a >= (FS_SHORT)(0x0500) ) &&  ( a <= (FS_SHORT)(0x05FF) ) )
#define STIK_FORMAT_DDD(a)   ( ( a >= (FS_SHORT)(0x0700) ) &&  ( a <= (FS_SHORT)(0x07FF) ) )

/* FS_GLYPHMAP types
*  Used as flags that can be OR'd
*/
#define FS_MAP_BITMAP         0x0001
#define FS_MAP_GRAYMAP2       0x0002
#define FS_MAP_GRAYMAP4       0x0004
#define FS_MAP_GRAYMAP8       0x0008
#define FS_MAP_GRAYMAP        FS_MAP_GRAYMAP4
#define FS_MAP_EDGE_GRAYMAP2  0x0200
#define FS_MAP_EDGE_GRAYMAP4  0x0400
#define FS_MAP_EDGE_GRAYMAP8  0x0800
#define FS_MAP_EDGE_TECH      FS_MAP_EDGE_GRAYMAP4
#define FS_MAP_RASTER_ICON    0x0020
#define FS_MAP_VECTOR_ICON    0x0040
#define FS_MAP_DISTANCEFIELD  0x0080

#ifdef GASP_ADFH_CONTROL
#define GRIDFIT 0x0001                    /* off when GASP_GRIDFIT in gasp is off */
#define DOGRAY 0x0002                    /* off when GASP_DOGRAY in gasp is off */
#define DOEDGE 0x0004                    
#endif
#define FS_MAP_ANY_GRAYMAP (FS_MAP_GRAYMAP2|FS_MAP_GRAYMAP4|FS_MAP_GRAYMAP8)
#define FS_MAP_ANY_EDGE_GRAYMAP (FS_MAP_EDGE_GRAYMAP2|FS_MAP_EDGE_GRAYMAP4|FS_MAP_EDGE_GRAYMAP8)
#define FS_MAP_ANY_ICON (FS_MAP_RASTER_ICON | FS_MAP_VECTOR_ICON)

/* When we're processing a STIK font, we use bit 7 of the
* first TrueType glyph flag byte to say that this character
* REALLY is an outline character. It also appears in the byte
* FS_OUTLINE->type[0] with the same meaning */
#define OUTLINE_CHAR        0x80

/* "normal" stroke percent for STIK fonts as fixed number (changed from 3% to 5%) */
#define FS_NORMAL_STROKE_PCT    3277  /* 5% in 16.16 */

/* this could be defined by user in fs_config.h */
#ifndef FS_DEFAULT_STROKE_PCT
#define FS_DEFAULT_STROKE_PCT FS_NORMAL_STROKE_PCT
#endif

/* deprecated */
#define DEFAULT_STROKE_PCT FS_DEFAULT_STROKE_PCT

/*****************************************************************************/
/* Basic data structures returned to the user, including:                    */
/* FS_OUTLINE, FS_BITMAP, FS_GRAYMAP, and FS_GLYPHMAP.                       */
/* FS_BITMAP, FS_GRAYMAP, and FS_GLYPHMAP have the same basic structure.     */
/* The only thing that differs is the number of bits per pixel.              */
/* and therefore the number of bytes per line <bpl> in each.                 */
/* FS_GLYPHMAP is a generalization of FS_BITMAP and FS_GRAYMAP to support    */
/* additional glyph types such as icons.                                     */
/*****************************************************************************/

/*****************************************************************************
 *
 *  Structure that describes a scaled and rasterized glyph as a
 *  1-bit-per-pixel bitmap.
 *   
 *  Description:
 *      This structure contains glyph metrics and image data.
 *  
 *      The metrics define glyph size and position, and the relative
 *      position of the next glyph.
 *  
 *      The lo_x and hi_y values define the upper left coordinate of
 *      the glyph relative to the <I>origin</I>, which is the point,
 *      generally along the baseline, that the glyph is to be
 *      positioned. These values are in terms of integer pixels.
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
 *      The image data is stored in the bits[] array. It is a simple
 *      bit-mapped representation of a glyph: each bit represents a
 *      pixel, high-order bit comes first, bits are mapped
 *      left-to-right and then top-to-bottom. Rows are padded, if
 *      necessary, so that each ends on a byte boundary. A bit value
 *      of "1" means that the pixel is on or "inked".
 *  
 *      The number of bytes in a row is usually, but not necessarily,
 *      equal to (7 + width)/8. The actual length of the bits array
 *      is height * bpl.
 *
 *      For example consider a glyph of simple representation of the
 *      character "F":
 *      <CODE>
 *
 *
 *                       ****
 *                       *
 *                       ***
 *                       *
 *      baseline--->     *    * <---next character's origin
 * 
 *             0123456789012345678901234567890
 *
 *      </CODE>
 *      The FS_BITMAP would have these values (i_dx and dx are just example values):
 *
 *      <TABLE>
 *          lo_x     0 
 *          hi_y     4
 *          i_dx     5
 *          i_dy     0
 *          dx       340787 (5.2 as 16.16)
 *          dy       0 (0.0 as 16.16 format)
 *          width    4
 *          height   5
 *          bpl      1
 *          bits{}   {F0, 80, E0, 80, 80}
 *      </TABLE>
 *
 *  See Also:
 *      FS_get_bitmap
 *
 *  <GROUP itypestructs>
 */
typedef struct {
    CACHE_ENTRY *cache_ptr; /* cache-info structure (private) */
    FS_LONG size;           /* size of structure in bytes, including bits array */
    FS_SHORT lo_x;          /* pixel coordinate of left column, relative to origin */
    FS_SHORT hi_y;          /* pixel coordinate of upper row, relative to origin */
    FS_SHORT i_dx;          /* x-advance in pixels - 0 if rotated or skewed */
    FS_SHORT i_dy;          /* y-advance in pixels - 0 if rotated or skewed */
    FS_FIXED dx;            /* x-advance in fractional pixels (16.16 format) */
    FS_FIXED dy;            /* y-advance in fractional pixels (16.16 format) */
    FS_SHORT width;         /* width of the bitmap in pixels */
    FS_SHORT height;        /* height of the bitmap in pixels */
    FS_SHORT bpl;           /* bytes per line (row) of bitmap data */
    FS_BOOLEAN embedded;    /* was this taken from an embedded bitmap? */
    FS_SHORT bitsPerPixel;  /* bits per pixel of bitmap data (always 1)                 */
    FS_USHORT type;         /* type of glyph data                                       */
    FS_BYTE  bits[1];       /* bitmap data packed in left-to-right, top-to-bottom order */
    } FS_BITMAP;


/*****************************************************************************
 *
 *  Structure that describes a scaled and rasterized glyph
 *  as a 4-bit-per-pixel graymap.
 *
 *  Description:
 *      This structure contains glyph metrics and image data.
 *
 *      The metrics define glyph size and position, and the relative
 *      position of the next glyph.
 *      
 *      The lo_x and hi_y values define the upper left coordinate of
 *      the glyph relative to the <I>origin</I>, which is the point,
 *      generally along the baseline, that the glyph is to be
 *      positioned. These values are in terms of integer pixels.
 *
 *      The <I>advance</I> is the distance from the current origin to that of
 *      the next character.  This structure supplies the advance in two forms:
 *      (1) integer and (2) fractional pixels.  Most of the time, the integer
 *      pixel advance will give the best intercharacter spacing results.  
 *      You must use the fractional pixel advance whenever both i_dx and i_dy
 *      are both zero as happens when the text is rotated or skewed.  See the
 *      iType User Guide for a detailed explanation of text layout issues.
 *
 *      The image data is stored in the bits[] array. It is a simple gray-mapped
 *      representation of a glyph: each nibble represents a pixel, high-order nibble
 *      comes first, nibbles are mapped left-to-right and then top-to-bottom.
 *      Rows are padded, if necessary, so that each ends on a byte boundary.
 *
 *      The gray level values (0 to 15) represent the relative intensities of the
 *      text display color.  You will probably need to use different color values when 
 *      writing to your frame buffer.  See the discussion in the User Guide.
 *
 *      The number of bytes in a row is usually, but not necessarily, equal
 *      to (1 + width)/2.  The actual length of the bits array is height * bpl.
 *
 *      For example consider a glyph of simple representation of the character "F":
 *
 *      <CODE>
 *                       ****
 *                       *
 *                       ***
 *                       *
 *      baseline--->     *    * <---next character's origin
 * 
 *             0123456789012345678901234567890
 *      </CODE>
 *
 *      The FS_GRAYMAP would have these values (i_dx and dx are just example values):
 *      <TABLE>
 *          lo_x        0
 *          hi_y        4
 *          i_dx        5
 *          i_dy        0
 *          dx          340787 (5.2 as 16.16)
 *          dy          0  (0.0 as 16.16 format)
 *          width       4
 *          height      5
 *          bpl         2
 *          bits{}      {FF, FF, F0, 00, FF, F0, F0, 00, F0, 00}
 *      </TABLE>
 *
 *  See also:
 *      FS_get_graymap, FS_get_phased
 *
 *  <GROUP itypestructs>
 */
typedef struct {
    CACHE_ENTRY *cache_ptr; /* cache-info structure (private) */
    FS_LONG size;           /* size of structure in bytes, including bits array */
    FS_SHORT lo_x;          /* pixel coordinate of left column, relative to origin */
    FS_SHORT hi_y;          /* pixel coordinate of upper row, relative to origin */
    FS_SHORT i_dx;          /* x-advance in pixels - 0 if rotated or skewed */
    FS_SHORT i_dy;          /* y-advance in pixels - 0 if rotated or skewed */
    FS_FIXED dx;            /* x-advance in fractional pixels (16.16 format) */
    FS_FIXED dy;            /* y-advance in fractional pixels (16.16 format) */
    FS_SHORT width;         /* width of the graymap in pixels */
    FS_SHORT height;        /* height of the graymap in pixels */
    FS_SHORT bpl;           /* bytes per line (row) of graymap data */
    FS_BOOLEAN embedded;    /* was this taken from an embedded graymap? */
    FS_SHORT bitsPerPixel;  /* bits per pixel of graymap data (2,4,8)                    */
    FS_USHORT type;         /* type of glyph data                                        */
    FS_BYTE bits[1];        /* graymap data packed in left-to-right, top-to-bottom order */
    } FS_GRAYMAP;

/*****************************************************************************
 *
 *  Generalized structure that describes a scaled and rasterized glyph.
 *  The glyph could be a bitmap, graymap, or icon.
 *  Various bit depths are supported for graymaps. 
 *
 *  Description:
 *      This structure contains glyph metrics and image data.
 *
 *      The metrics define glyph size and position, and the relative
 *      position of the next glyph.
 *      
 *      The lo_x and hi_y values define the upper left coordinate of
 *      the glyph relative to the <I>origin</I>, which is the point,
 *      generally along the baseline, that the glyph is to be
 *      positioned. These values are in terms of integer pixels.
 *
 *      The <I>advance</I> is the distance from the current origin to that of
 *      the next character.  This structure supplies the advance in two forms:
 *      (1) integer and (2) fractional pixels.  Most of the time, the integer
 *      pixel advance will give the best intercharacter spacing results.  
 *      You must use the fractional pixel advance whenever both i_dx and i_dy
 *      are both zero as happens when the text is rotated or skewed.  See the
 *      iType User Guide for a detailed explanation of text layout issues.
 *
 *      The image data is stored in the bits[] array. The interpretation of the 
 *      data depends on the glyph map type. The data may represent an outline, a 1-bit 
 *      per pixel bitmap, or a 2,4, or 8-bit graymap. It may also represent a raster 
 *      icon or a vector icon. It could also represent an OpenVG path or raster image. 
 *      
 *      The following glyphmap types are supported:
 *      <TABLE>
 *      Glyphmap Type          Description
 *      -------------------    ----------------------------------
 *      FS_MAP_BITMAP          (1 bit per pixel)
 *      FS_MAP_GRAYMAP         (4 bits per pixel)
 *      FS_MAP_GRAYMAP2        (2 bits per pixel)
 *      FS_MAP_GRAYMAP4        (4 bits per pixel)
 *      FS_MAP_GRAYMAP8        (8 bits per pixel)
 *      FS_MAP_EDGE_TECH       (4 bits per pixel Edge graymap)
 *      FS_MAP_EDGE_GRAYMAP2   (2 bits per pixel Edge graymap)
 *      FS_MAP_EDGE_GRAYMAP4   (4 bits per pixel Edge graymap)
 *      FS_MAP_EDGE_GRAYMAP8   (8 bits per pixel Edge graymap)
 *      FS_MAP_RASTER_ICON     (scaled raster-based icon data)
 *      FS_MAP_VECTOR_ICON     (scalable vector-based icon data)
 *      </TABLE>
 *
 *      The data for the FS_MAP_BITMAP type is identical to
 *      that in FS_BITMAP. The data format for type FS_MAP_GRAYMAP is identical to
 *      that of FS_GRAYMAP except that it depends on the bitPerPixel value. 
 *      Rows are padded, if necessary, so that each ends on a byte boundary. 
 *      Edge graymaps are in the same format as FS_GRAYMAP.
 *      See discussion in the <I>iType User Guide</I>.
 *
 *      For bitsPerPixel = 2, the gray level values range from 0 to 3. Each
 *      byte represents 4 pixels.
 *
 *      For bitsPerPixel = 4, the gray level values range from 0 to 15. Each nibble
 *      represents a pixel.
 *
 *      For bitsPerPixel = 8, the gray level values range from 0 to 255. Each byte
 *      represents a pixel. 
 *
 *      The icon data is actual image data in a self-tagged raster or vector format. 
 *      Size and offset information is provided that allows the icon to be inserted
 *      into the text stream by the user's application, allowing the icon to 
 *      flow with the regular text. See the discussion in the <I>iType User Guide</I>.
 *
 *  <GROUP itypestructs>
 */
typedef struct {
    CACHE_ENTRY *cache_ptr; /* cache-info structure (private)                          */
    FS_LONG size;           /* size of structure in bytes, including bits array        */
    FS_SHORT lo_x;          /* pixel coordinate of left column, relative to origin     */
    FS_SHORT hi_y;          /* pixel coordinate of upper row, relative to origin       */
    FS_SHORT i_dx;          /* x-advance in pixels - 0 if rotated or skewed            */
    FS_SHORT i_dy;          /* y-advance in pixels - 0 if rotated or skewed            */
    FS_FIXED dx;            /* x-advance in fractional pixels (16.16 format)           */
    FS_FIXED dy;            /* y-advance in fractional pixels (16.16 format)           */
    FS_SHORT width;         /* width of the graymap in pixels                          */
    FS_SHORT height;        /* height of the graymap in pixels                         */
    FS_SHORT bpl;           /* bytes per line (row) of glyphmap data                   */
    FS_BOOLEAN embedded;    /* was this an embedded glyph?                             */
    FS_SHORT bitsPerPixel;  /* bits per pixel of glyphmap data                         */
    FS_USHORT type;         /* type of glyph data                                      */
    FS_BYTE bits[1];        /* glpyh data                                              */
    } FS_GLYPHMAP;


/* bit values for "outl_flag" field */
#define OUTL_FLAGS_ANYHINTS             0x0001
#define OUTL_FLAGS_SCANCTRL             0x0002
#define OUTL_FLAGS_GRIDFITTYPE_BAZ      0x0004
#define OUTL_FLAGS_GRIDFITTYPE_MAZ      0x0008
#define OUTL_FLAGS_RTGAH                0x0010

/*****************************************************************************
 *
 *  Structure that describes a glyph as quadratic outlines.
 *
 *  Description:
 *      This structure contains glyph metrics and image data.
 *
 *      The lo_x and hi_x values define the left and right extents
 *      of the outline and the lo_y and hi_y define the lower and 
 *      upper bounds of the outline. These values are fixed point 
 *      values relative to the origin, which is the point, generally 
 *      along the baseline, that the glyph is to be positioned.
 *
 *      Note that the interpretation of the lo_x, hi_x, lo_y, hi_y
 *      values is somewhat different from those in a glyph map.
 *      For glyph maps, those values are in integer pixels relative
 *      to the origin. For outlines, they refer to the bounds of
 *      the outline in fixed point coordinates that have units of 
 *      pixels. For example, if the lo_x value is 0.0 and the hi_x 
 *      value is 11.0, that means that the leftmost extent of the
 *      outline starts at the origin and the rightmost extent is at
 *      the boundary between the 10th and 11th pixel from the origin, 
 *      counting from zero. The total width of the outline is 11.0 
 *      pixels wide in this case. 
 *
 *      The outline is represented as a set of elements represented by
 *      a type and a set of points. Point types can consist of:
 *      FS_MOVETO, FS_LINETO, FS_QUADTO, FS_CUBETO
 *      Correponding x,y values are stored in 16.16 fixed point arrays.
 *      The number of points differs from the number of types since
 *      a moveto and lineto require 1 point, a quadto requires 2 points,
 *      and a cubeto requires 3 points.
 *
 *      For example, the character "F" could be represented as:
 *      <TABLE>
 *          num         11
 *          type[]      {FS_MOVETO, FS_LINETO, ..., FS_LINETO}
 *          x[]         {0, 0, 196608, 196608, 65536, 65536, 131072, 131072, 65536, 65536, 0}
 *          y[]         {0, 327680, 327680, 262144, 262144, 196608, 196608, 131072, 131072, 0, 0}
 *      </TABLE>
 *
 *      Even though FS_OUTLINE contains pointers to structures, it is allocated
 *      as one piece of contiguous memory.  Any FS_OUTLINE structure
 *      returned by FS_get_outline() should be freed using FS_free_char().
 *
 *  See Also:
 *      FS_get_outline()
 *
 *  <GROUP itypestructs>
 */ 
typedef struct {
    CACHE_ENTRY *cache_ptr; /* cache-info structure (private) */
    FS_LONG  size;          /* size of structure in bytes */
    FS_FIXED lo_x;          /* smallest x coordinate */
    FS_FIXED hi_x;          /* largest x coordinate */
    FS_FIXED lo_y;          /* smallest y coordinate */
    FS_FIXED hi_y;          /* largest y coordinate */
    FS_SHORT i_dx;          /* x-advance in pixels - 0 if rotated or skewed */
    FS_SHORT i_dy;          /* y-advance in pixels - 0 if rotated or skewed */
    FS_FIXED dx;            /* x-advance in fractional pixels (16.16 format) */
    FS_FIXED dy;            /* y-advance in fractional pixels (16.16 format) */
    FS_SHORT nc;            /* number of contours */
    FS_SHORT np;            /* number of curve points (x,y) */
    FS_SHORT num;           /* number of curve types (moves, lines, or quadratics) */
    FS_SHORT polarity;      /* which side is "ink" on? (for emboldening) */
    FS_BYTE outl_flag;      /* bit field describe: any hints, scan control, gridFitType for EDGE of the outline*/
    FS_BYTE *type;          /* respective type of curve elements (one of FS_MOVETO, FS_LINETO, FS_QUADTO) */
    FS_FIXED *x;            /* x-coordinates of the elements in fractional pixels (16.16 format) */
    FS_FIXED *y;            /* y-coordinates of the elements in fractional pixels (16.16 format) */
    } FS_OUTLINE;

    /* note: OUTLINEs are allocated in one contiguous piece, so you can delete
    * them with <FS_free_char>. The pointers are set in creation or by <copy_outline>
    */

/*****************************************************************************
 *
 *  Structure that describes a glyph advance.
 *
 *  Description:
 *      This structure contains fixed point advance data for a glyph.
 *
 *      The dx and dy values are the same as would be returned in a 
 *      glyph map of the corresponding type.
 *
 *
 *  See Also:
 *      FS_get_advance()
 *
 *  This is a private data structure and should not be used by clients
 */ 
typedef struct {
    CACHE_ENTRY *cache_ptr; /* cache-info structure (private) */
    FS_LONG  size;          /* size of structure in bytes */
    FS_SHORT i_dx;          /* x-advance in integer form */
    FS_SHORT i_dy;          /* y-advance in integer form */
    FS_FIXED dx;            /* x-advance in fractional pixels (16.16 format) */
    FS_FIXED dy;            /* y-advance in fractional pixels (16.16 format) */
    } FS_ADVANCE;

/********* structures used for grayscale rendering *****************/
#ifdef FS_GRAYMAPS
/*
** reserve room for 512 thangs - 8K bytes
**
** assuming an average of 8 transitions per raster line
** that would handle 512/8 = 64 ppm
**
** easy characters have 2 transitions per raster line
** that would handle 512/2 = 256 ppm
**
** nasty characters have 16 transitions per raster line
** that would handle 512/16 = 32 ppm
**
** if a character needs more thangs than that ... use multiple bands
*/
#define NUM_THANGS 512
#define NUM_ROWS (NUM_THANGS/2)    
#define STACK_COUNT 48    /* must be an integer multiple of 6 (quads) and 8 (cubes) -- and 24 is too small */
#define STACK_FULL ((FS_LONG)(stack-stack_base) >= STACK_COUNT)

typedef struct _THANG 
    {
    FS_SHORT x;
    FS_SHORT y;
    FS_FIXED a;
    FS_FIXED f;
    struct _THANG *next;
    } THANG;

typedef struct {
    THANG thangs[NUM_THANGS];            /* all available thangs                       */
    THANG *rows[NUM_ROWS];               /* rows of thangs for each raster of the band */
    int next_thang;
    FS_FIXED *areas,*nexts;              /* allocate these based on gmap->width        */
    int areasize;                        /* number of areas allocated                  */
    int num_bands;                       /* number of bands required to cover graymap  */
    FS_FIXED band_ymin;                  /* bottom one has this y coordinate           */
    FS_FIXED band_ymax;                  /* top one has this y coordinate              */
    int top;                             /* band_ymax >> 16                            */
    int dir;                             /* line direction +/- 1                       */
    int ref_count;                       /* prevent get_some_back from recovering mem  */
    FS_GRAYMAP *gmap;                    /* the graymap we are building                */
    FS_FIXED stack_base[STACK_COUNT];    /* the quad/cube subdivision stack            */
    FS_FIXED *stack;                     /* initialized to stack_base                  */
    } RASTER;

#endif /* FS_GRAYMAPS */

/*** table pointers ***/
typedef struct table_ptr_rec {
    LFNT *lfnt;          /* the LFNT the table was read from */
    FS_BYTE *ptr;        /* pointer to the data (internal or external) */
    FS_ULONG tag;        /* the table tag we read */
    FS_ULONG ref_count;  /* number of 'users' of the pointer */
    FS_BOOLEAN external; /* if external==true, we can 'free' the pointer */
    struct table_ptr_rec *next;
    } TABLE_PTR_REC;


/*** the SERVER -- iType guts ***/
typedef void*  FS_MUTEX;
struct FS_STATE;

typedef struct  {
    FNTSET *font_sets;      /* linked list - all font sets */
    SFNT *scaled_fonts;     /* linked list - all scaled fonts */
    LFNT *loaded_fonts;     /* linked list - all available/loaded fonts */

    TLIST *tlist;           /* the current transition list -- for the rasterizer */
    TLIST *drop;            /* the current dropout list -- for the rasterizer */
    
#ifdef FS_STIK
    FS_BITMAP *bmap;        /* for direct stik font rendering */
#endif /* FS_STIK */
    
#ifdef FS_CONTOUR_WINDING_DETECTION
    /* Array of TNODE indices of the start of loops 1 through outl->nnc */
    /* The array is indexed by contour number */
    /* nextLoopTX[0] is the starting TNODE index of the contour 1 */
    /* nextLoopTX[number of contours - 1] is the TNODE index just after the last */
    /* TNODE in the last contour */
    FS_LONG *nextLoopTX;
#endif
    
    FS_VOID (*line)(struct FS_STATE*, FS_FIXED,FS_FIXED,FS_FIXED,FS_FIXED);
    
    CACHE_ENTRY *cache[CACHE_MOD];     /* cached things for all SFNT's */

    FS_ULONG heap_size;            /* amount of memory we can use */
    FS_ULONG allocated;            /* high water mark of memory allocation */

#ifdef FS_INT_MEM
    /* we either malloc(heapsize) or use external lump, then work within that lump */
    /* Why?  it keeps from fragmenting the system heap */
    _heap heap;
    FS_BOOLEAN iTypeOwnsHeap;   /* True if heap block was allocated by iType, False if client passed it in */
#endif /* FS_INT_MEM */

    TABLE_PTR_REC *table_ptrs;    /* for tables by reference */

#ifndef FS_NO_FS_LUMP
    FS_MASTER_LUMP *list_of_lumps;
    void *CACHE_ENTRY_chain;
#endif

    FsScratchSpace scratch;       /* general usage scratch space */

    FS_LONG workspaceSize;        /* temporary workspace for instruction stack, stack */
    FS_BYTE *workspace;           /* and outline for hint processing */
    SFNT    *workspace_sfnt;      /* pointer to sfnt currently using workspace */

    FS_ULONG version;             /* Major and Minor version  */

    FS_ULONG client_count;        /* number of active clients ... don't call FS_exit() until it's 0 */
#ifdef FS_MULTI_PROCESS
    FS_ULONG total_size;
    FS_LONG proc_count;           /* number of clients sharing heap */
#endif

#ifdef FS_GRAYMAPS
    RASTER *raster;               /* raster data heavily used by grayscale renderer */
#endif

    } FS_SERVER;

/************* loaded/available file-mapped fonts ***************************/
typedef struct addedfnt_
{
    FILECHAR *name;
    FNTSET   *set;
    FS_ULONG  add_count;
    struct addedfnt_ *next;
} ADDEDFNT;

/*****************************************************************************
 * the iType Engine STATE (client info) 
 *  <GROUP itypestructs>
 */
typedef struct  FS_STATE {
    struct FS_STATE *parent;

    ADDEDFNT *name_map;

    TYPESET cur_typeset;       /* the current type set                               */
    FS_USHORT cur_font;        /* the current type set element selected              */
    LFNT *cur_lfnt;            /* the current LFNT being used within the current set */
    SFNT *cur_sfnt;            /* the current scaled font = LFNT + SCALE             */
    FS_USHORT platform;        /* platform and encoding are needed                   */
    FS_USHORT encoding;        /* to restore current cmap                            */
    FS_USHORT varPlatform;     /* platform for getting Ideographic Variations        */
    FS_USHORT varEncoding;     /* encoding gor getting Ideographic Variations        */
    FS_ULONG error;            /* the most recent error code                         */
    FS_ULONG flags;            /* user accessible (FS_set_flags) state flags         */
    FS_FIXED scale[4];         /* needed to rebuild SFNT from LFNT                   */
    FS_FIXED stroke_pct;       /* stroke-width percentage                            */
    FS_FIXED bold_pct;         /* emboldening percentage                             */
    FS_ULONG heap_size;        /* the additional memory requested by this client     */
    FS_BYTE level;             /* nesting level for composites                       */
    FS_BYTE any_hints;         /* does the glyph have any hints?                     */
    FS_BYTE outl_char;         /* this char in STIK font is really an outline        */
    FS_USHORT lpm;             /* lines per em                                       */
    FS_USHORT outline_width;   /* N-pixel outline width                              */
    FS_FIXED outline_opacity;  /* N-pixel outline opacity                            */
    FS_ULONG ref_count;
    FS_SERVER *server;         /* pointer to the guts of iType                       */
#ifdef FS_STIK
    FS_BOOLEAN use_reflines;   /* used in autohint_stik                              */
#endif
#ifdef FS_EXTERNAL_OUTLINE
    FS_OUTLINE *user_outline;
    FS_FIXED user_outline_scale[4];
    FS_BOOLEAN user_outline_scale_vanilla;
#endif
#if defined(FS_EDGE_HINTS) || defined(FS_EDGE_RENDER)
    void*  libInst;                   /* ADF instance data structure                    */
#endif
#ifdef FS_EDGE_RENDER
    ADF_U32        adfGridFitType;    /* current Edge(TM) grid fit type                 */
    ADF_F32        outsideCutoff;     /* default Outside cutoff value for Edge(TM) CSM  */
    ADF_F32        insideCutoff;      /* default Inside cutoff value for Edge(TM) CSM   */
    ADF_F32        gamma;             /* default Gamma curve exponent for Edge(TM) CSM  */
    ADF_F32        outsideCutoffAdj;  /* current outside cutoff adjustment at STATE.lpm */
    ADF_F32        insideCutoffAdj;   /* current inside cutoff adjustment at STATE.lpm  */
    ADF_F32        sharpnessOffset;   /* current sharpness adjustment offset            */
    ADF_F32        sharpnessSlope;    /* current sharpness adjustment slope (vs ppem)   */
    ADF_F32        thicknessOffset;   /* current thickness adjustment offset            */
    ADF_F32        thicknessSlope;    /* current thickness adjustment slope (vs ppem)   */
#endif
#if defined(FS_EDGE_HINTS) && defined(FS_GRIDVIEWER)
    FS_BYTE        gridViewerFlag;
    FS_BYTE        noDeltas;
#endif

#if defined(FS_MULTI_PROCESS) || defined(FS_MAPPED_FONTS)
    MAPPEDFNT *mappedfnts;        /* process's file-mapped list          */
#endif
#ifdef FS_MULTI_PROCESS
    /* File-mapped and shared memory data can potentially be mapped to a */
    /* different location in memory for each process. As a result, each  */
    /* process needs its own LFNT list and shared memory pointer.        */
    FS_BYTE *shared_mem;          /* process's pointer to shared memory  */
    FS_LONG hshared_mem;          /* process's handle to shared memory   */
    FS_BOOLEAN my_shared_mem;     /* TRUE if shared mem seg was created by this process */
#endif
#if defined(FS_MULTI_PROCESS) && defined(FS_MP_DEBUG)
    FS_LONG process_num;        /* process number associated with this state */
#endif

#ifdef FS_MEM_DBG
    FS_FILE *memdbgfp;
    char *memdbgid;
#endif

    /* NOTE: The "FS_mutex_ptr" field must be the last field of the */
    /* FS_STATE structure in order for multi-threading and */
    /* multi-processing to work properly! */
#ifdef FS_MULTI_THREAD
    FS_MUTEX FS_mutex_ptr;    /* the mutex used for multi-threading. this must be */
                              /* the last variable in the structure, see FS_init in multi.c */
#endif
    } FS_STATE;


/*****************************************************************************
 *
 *  Structure that describes a bounding box.
 *
 *  Description:
 *      A bounding box describes the extent of a character or character set.
 *
 *      The y-coordinate increases from bottom-to-top.
 *
 *  See Also:
 *      FONT_METRICS 
 *
 *  <GROUP itypestructs>
 */
typedef struct 
{
    FS_SHORT xMin;              /* left */
    FS_SHORT yMin;              /* bottom */
    FS_SHORT xMax;              /* right */
    FS_SHORT yMax;              /* top */
} FS_BBOX;


/* bit values for "font_type" field */
#define FM_FLAGS_TTF    0x0001
#define FM_FLAGS_STIK   0x0002
#define FM_FLAGS_CCC    0x0004
#define FM_FLAGS_ACT3   0x0008
#define FM_FLAGS_TTC    0x0010
#define FM_FLAGS_PFR    0x0020
#define FM_FLAGS_DDD    0x0040
#define FM_FLAGS_CFF    0x0080


/*****************************************************************************
 *
 *  Structure that describes metrics that apply to entire font.
 *
 *  Description:
 *      In general, if a numeric field or table is not present in the font,
 *      the corresponding field will be set to NOT_DEFINED.
 *
 *      There are three sets of ascender/descender/leading values.  Refer
 *      to the <I>User Guide</I> for discussion of how to use these.
 *      There is no os2_win_leading value.
 *
 *      For PFR fonts (which do not have TrueType tables), the same set of 
 *      values is used for all three sets of ascender/descender/leading values,
 *      and the cmap values will be null. The metrics_resolution value is 
 *      used only for PFR fonts - it will be null for all TrueType fonts.
 *
 *      The font_type bit field bits settings are:
 *      <TABLE>
 *      <B>Bit</>   <B>Type</B>
 *      0           TTF TrueType font
 *      1           STF (Stroke font)
 *      2           CCC compressed font
 *      3           ACT compressed font
 *      4           TTC TrueType collection
 *      5           PFR Format 0 font
 *      6           DDD compressed font
 *      7           CFF OpenType font
 *      8-15        Reserved 
 *      </TABLE>
 *
 *      The <I>mappings</I> array describes the set of available cmaps.  Each
 *      array entry is a CMAP_TAB structure, which stores cmap platform
 *      and encoding information.  The number of valid mappings[] entries
 *      is given by num_cmap_tables.
 *
 *  See Also:
 *      FS_font_metrics(), CMAP_TAB, FS_BBOX
 *
 *  <GROUP itypestructs>
 */
typedef struct {
    FS_USHORT unitsPerEm;       /* design units from 'head' table -> unitsPerEm */
    FS_USHORT head_macStyle;    /* font style from 'head' table-> macStyle   */ 
    FS_SHORT os2_win_ascent;    /* ascender from 'OS/2' table -> usWinAscent */
    FS_SHORT os2_win_descent;   /* descender from 'OS/2' table -> usWinDescent */
    FS_SHORT os2_ascent;        /* ascender from 'OS/2' table -> sTypoAscender */
    FS_SHORT os2_descent;       /* descender from 'OS/2' table -> sTypoDescender */
    FS_USHORT os2_fsSelection;  /* font selection flags from 'OS/2' table-> fsSelection */
    FS_USHORT os2_leading;      /* line gap from 'OS/2' table -> sTypoLineGap */
    FS_SHORT hhea_ascent;       /* ascender from 'hhea' table -> Ascender */
    FS_SHORT hhea_descent;      /* descender from 'hhea' table -> Descender */
    FS_USHORT hhea_leading;     /* line gap from 'hhea' table -> LineGap */
    FS_USHORT embedding_bits;   /* font embedding licensing rights from 'OS/2' table -> fsType */
    FS_BBOX font_bbox;          /* exact value of font bounding box */
    FS_USHORT font_type;        /* bit field that describes type */
    FS_USHORT num_cmap_tables;  /* number of valid entries in the <I>mappings</I> array (from "cmap" table) */
    CMAP_TAB mappings[MAX_MAPPINGS_LEN]; /* array of cmap descriptions (from "cmap" table) */
    FS_ULONG numFontsInTTC;     /* number of fonts in file (valid only if file is a TrueType collection) */
    FS_ULONG indexInTTC;        /* index in TTC file of current font (valid only if file is a TrueType collection) */
    FILECHAR font_name[MAX_FONT_NAME_LEN];        /* font name */
    FILECHAR font_family_name[MAX_FONT_FAMILY_NAME_LEN];  /* font family name (from 'name' table) */
    FILECHAR copyright[MAX_COPYRIGHT_LEN];        /* font copyright (from 'name' table) */
    FS_USHORT metricsResolution;  /* pfr only */
    FS_ULONG numGlyphs;           /* number of glyphs in the font (not icons) */
    FS_ULONG numIcons;            /* number of icons in the font (not glyphs) */
    } FONT_METRICS;


#define FS_MP_SEM_NAME        "iTypeSemaphore"
/* To convert a pointer to shared memory to an offset */
#ifdef FS_MULTI_PROCESS
#define MP_memOFF(a) ((!(a)) ? NULL : (FS_VOID *)((FS_BYTE *)(a) - (FS_BYTE *)ttf->memptr) )
#define MP_OFF(a) ((!(a)) ? NULL : (FS_VOID *)((FS_BYTE *)(a) - (FS_BYTE *)STATE.server) )
#define MPsp_OFF(a) ((!(a)) ? NULL : (FS_VOID *)((FS_BYTE *)(a) - (FS_BYTE *)sp->server) )
#define MPgs_OFF(a) ((!(a)) ? NULL : (FS_VOID *)((FS_BYTE *)(a) - (FS_BYTE *)gs->server) )
#else
#define MP_memOFF(a) a
#define MP_OFF(a) a
#define MPsp_OFF(a) a
#define MPgs_OFF(a) a
#endif
 
/* To convert an offset to a pointer to shared memory */
#ifdef FS_MULTI_PROCESS
/* DO NOT CHANGE "(long)" CAST BELOW - NEEDED FOR ALPHA */
#define MP_memPTR(type,a) ((!(a)) ? NULL : (type *)((long)(a) + (FS_BYTE *)ttf->memptr) )
#define MP_PTR(type,a) ((!(a)) ? NULL : (type *)((long)(a) + (FS_BYTE *)STATE.server) )
#define MPsp_PTR(type,a) ((!(a)) ? NULL : (type *)((long)(a) + (FS_BYTE *)sp->server) )
#define MPgs_PTR(type,a) ((!(a)) ? NULL : (type *)((long)(a) + (FS_BYTE *)gs->server) )
#else
#define MP_memPTR(type,a) a    
#define MP_PTR(type,a) a
#define MPsp_PTR(type,a) a
#define MPgs_PTR(type,a) a
#endif

/******************************************************************************
 *
 * If the calling program uses the _PS_ or _PS0_ macros to pass the
 * FS_STATE pointer to functions, it must
 * declare and allocate the state pointer using the    
 * name "FS_state_ptr".  Then this pointer will be
 * passed by the _PS_ and _PS0_ macros.  For example:
 *
 *  main()
 *    {
 *        /- other local variables -/
 *        FS_STATE *FS_state_ptr;
 *        FS_state_ptr = (FS_STATE *)malloc(sizeof(FS_STATE));
 *    
 *        FS_init(_PS_ heap_SIZE);
 *        /- other code -/
 *
 *      free(FS_state_ptr);
 *  }
 *
 * Or:
 *
 *  main()
 *    {
 *        /- other local variables -/
 *        FS_STATE my_FS_state, *FS_state_ptr;
 *        FS_state_ptr = &my_FS_state;
 *    
 *        FS_init(_PS_ heap_SIZE);
 *        /- other code -/
 *  }
 *
 * But, keep in mind, this second example will not have a valid "my_FS_state"
 * when "main()" returns due to the object having been allocated on the stack.
 *
 */

/*lint -esym(773,_DS0_) */ /* This macro is used in parameter lists and thus should not be parenthesized. */

#define STATE (*FS_state_ptr)           /* dereference passed pointer */
#define _DS0_ FS_STATE *FS_state_ptr    /* define state pointer as the only argument */
#define _DS_  FS_STATE *FS_state_ptr,   /* define state pointer as the first argument */
#define _PS0_ FS_state_ptr              /* pass state pointer as the only argument */
#define _PS_  FS_state_ptr,             /* pass state pointer as the first argument */

#ifdef FS_ACT3
#include "fs_radecomp.h"
#include "fs_tablecmp.h"
#include "fs_ttf_conv.h"
#include "fs_ra_ttf.h"
#endif

/* default case...empty definition */
#ifndef ITYPE_API
#define ITYPE_API
#endif

#ifdef FS_OPENVG
#include "../openvg/fs_openvg.h"
#endif

#endif /* FS_OBJECT_H */
