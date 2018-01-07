/**
 *-----------------------------------------------------------------------------------
 *    Filename: ADFGenerate.h
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Copyright 2004-2007 Mitsubishi Electric Research Laboratories (MERL)
 *    The ADF data structure and general purpose ADF generation utilities
 *    Ronald Perry and Sarah Frisken
 *-----------------------------------------------------------------------------------
 */


/**
 *-----------------------------------------------------------------------------------
 *    To avoid multiple inclusion of header files
 *-----------------------------------------------------------------------------------
 */
#ifndef ADFGENERATE_H
#define ADFGENERATE_H


/**
 *-----------------------------------------------------------------------------------
 *    To make functions accessible from C++ code
 *-----------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif


/**
 *-----------------------------------------------------------------------------------
 *    VERSION NUMBERS FOR ADF DATA STRUCTURES
 *-----------------------------------------------------------------------------------
 */
#define ADF_EXPLICIT_VERSION_NUMBER 0x41444631  /* 'ADF1' */
#define ADF_IMPLICIT_VERSION_NUMBER 0x494D5031  /* 'IMP1' */


/**
 *-----------------------------------------------------------------------------------
 *    ADF GLYPH DATA STRUCTURE
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Saffron supports two types of ADFs for representing glyphs: explicit ADFs and
 *    implicit ADFs. A compile time Boolean flag (ADF_USE_IMPLICIT_ADFS) controls the
 *    selection of which ADF type to use throughout the system. Note that outline-based
 *    glyphs (e.g., from TrueType fonts) can be represented by both explicit and
 *    implicit ADFs while stroke-based glyphs can only be represented by implicit ADFs.
 *
 *    Explicit ADF generation uses top-down spatial subdivision to generate a spatial 
 *    hierarchy of explicit ADF cells (i.e., a two-dimensional quadtree of explicit ADF 
 *    cells), where each explicit ADF cell contains a set of sampled distance values 
 *    and a reconstruction method; explicit ADF rendering reconstructs the distance 
 *    field within each explicit ADF cell using its reconstruction method and then maps 
 *    the reconstructed distances to density values.
 *
 *    In contrast, implicit ADF cells are not generated during generation (they are 
 *    generated on-demand during rendering). More specifically, implicit ADF generation
 *    preprocesses an ADFPath (which represents a glyph); implicit ADF rendering 
 *    generates implicit ADF cells from the preprocessed ADFPath and renders each 
 *    implicit ADF cell by 1) reconstructing the distance field within the implicit ADF 
 *    cell using its reconstruction method and 2) mapping the reconstructed distances 
 *    to density values.
 *
 *    An ADFGlyph is used to represent both explicit and implicit ADFs.
 *
 *    The cells of an explicit ADF quadtree are stored in a contiguous block of memory
 *    immediately following the ADF header (i.e., the ADFGlyph data structure), with 
 *    rootOffset specifying the offset in bytes from the start of the header to the 
 *    root cell of the explicit ADF quadtree. A contiguous block of distance values 
 *    (for representing the distance field within the explicit ADF cells) immediately 
 *    follows the block of explicit ADF cells.
 *
 *    Implicit ADF generation transforms pen commands of an ADFPath from font units to 
 *    the [0.0,1.0] x [0.0,1.0] ADF coordinate system and stores the transformed pen 
 *    commands in a contiguous array immediately following the ADF header (i.e., the 
 *    ADFGlyph data structure). ADFGlyph.numCells is used to store the number of 
 *    transformed pen commands and ADFGlyph.rootOffset is used to store the offset in 
 *    bytes from the start of the ADF header to the array of transformed pen commands.
 *
 *    The ADFGlyph data structure consists of the following elements:
 *
 *    version: The version number of the ADF data structure (i.e.,
 *    ADF_EXPLICIT_VERSION_NUMBER or ADF_IMPLICIT_VERSION_NUMBER).
 *
 *    totalSizeBytes: The total size in bytes of the ADF, including the ADF header.
 *
 *    rootOffset: If using explicit ADFs, the offset in bytes from the start of the ADF 
 *    header to the root cell of the explicit ADF quadtree. If using implicit ADFs, the 
 *    offset in bytes from the start of the ADF header to the array of transformed pen 
 *    commands.
 *
 *    numCells: If using explicit ADFs, the total number of explicit cells in the 
 *    explicit ADF. If using implicit ADFs, the number of transformed pen commands.
 *
 *    charCode: The Unicode character code of the glyph represented by the ADF.
 *
 *    FUToADFScale: The scale factor from font units to ADF coordinates for this glyph. 
 *    Points that define a glyph (e.g., control points of an outline-based glyph), 
 *    originally specified in font units, are scaled to fit optimally within the ADF's 
 *    [0.0, 1.0] x [0.0, 1.0] floating point coordinate system. 
 *
 *    ADFUnitsPerEM: The em box size, in floating point ADF units, of the typeface of 
 *    the glyph represented by the ADF (i.e., ADF units per em).
 *
 *    pathType: If using implicit ADFs, indicates whether the ADF represents an 
 *    outline-based glyph (ADF_OUTLINE_PATH) or a uniform-width stroke-based glyph 
 *    (ADF_UNIFORM_STROKE_PATH).
 *
 *    pathWidth: If using implicit ADFs and pathType is ADF_UNIFORM_STROKE_PATH, the 
 *    stroke width of the uniform-width stroke-based glyph in ADF coordinates.
 *
 *    glyphOriginX, glyphOriginY: The glyph origin (i.e., the reference point for 
 *    typesetting) in ADF coordinates.
 *    
 *    glyphMinX, glyphMinY, glyphMaxX, glyphMaxY: The exact bounding box (in ADF
 *    coordinates) of the glyph represented by the ADF. If pathType is
 *    ADF_UNIFORM_STROKE_PATH, then the bounding box accounts for the stroke width
 *    (i.e., pathWidth) of the uniform-width stroke-based glyph.
 *
 *
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
#define ADF_X_ALGN_ZONE_BIT     0x0001     /* Bit indicating that x alignment zones exist */
#define ADF_Y_ALGN_ZONE_BIT     0x0002     /* Bit indicating that y alignment zones exist  */
/**
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 */
typedef struct {
    ADF_U32 version;         /* Version number of this ADFGlyph data structure */
    ADF_U32 totalSizeBytes;  /* Total size of ADF, including header */
    ADF_U32 rootOffset;      /* Byte offset from start of ADF header to ADF data */
    ADF_U32 numCells;        /* Total number of cells or pen commands in the ADF */
    ADF_U32 charCode;        /* Unicode value of the glyph represented by the ADF */
    ADF_F32 FUToADFScale;    /* Scale from font units to ADF coordinates */
    ADF_F32 ADFUnitsPerEM;   /* Em box size of typeface of glyph represented by ADF */
    ADF_U32 pathType;        /* ADF_OUTLINE_PATH or ADF_UNIFORM_STROKE_PATH */
    ADF_F32 pathWidth;       /* The stroke width of an ADF_UNIFORM_STROKE_PATH */
    ADF_F32 glyphOriginX;    /* x-coordinate of the glyph origin in ADF coordinates */
    ADF_F32 glyphOriginY;    /* y-coordinate of the glyph origin in ADF coordinates */
    ADF_F32 glyphMinX;       /* The minimum x-coord of the glyph's bBox in ADF coords */
    ADF_F32 glyphMinY;       /* The minimum y-coord of the glyph's bBox in ADF coords */
    ADF_F32 glyphMaxX;       /* The maximum x-coord of the glyph's bBox in ADF coords */
    ADF_F32 glyphMaxY;       /* The maximum y-coord of the glyph's bBox in ADF coords */
}    ADFGlyph;


/**
 *-----------------------------------------------------------------------------------
 *    SCALING AND OFFSETTING AN ADF PATH FROM FONT UNITS TO ADF COORDINATES
 *-----------------------------------------------------------------------------------
 *-----------------------------------------------------------------------------------
 *    Determine the scale and offset for transforming a glyph represented by a 
 *    specified ADFPath from font units to the [0.0,1.0] x [0.0,1.0] ADF coordinate 
 *    system and compute the EM box size of the typeface of the glyph in ADF units.
 *    ADFSetGlyphScaleAndOffset() allows the identical (i.e., bit-exact) computation 
 *    of its output attributes during ADFGenerateADF() and ADFRenderSetupFromPath().
 *-----------------------------------------------------------------------------------
 */
ADF_Void ADFSetGlyphScaleAndOffset (ADFPath *path, ADF_F32 *FUToADFScale, 
                                    ADF_F32 *glyphOffsetX, ADF_F32 *glyphOffsetY, 
                                    ADF_F32 *ADFUnitsPerEM);


/**
 *-----------------------------------------------------------------------------------
 *    End of C++ wrapper
 *-----------------------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif


/**
 *-----------------------------------------------------------------------------------
 *    End of ADFGENERATE_H
 *-----------------------------------------------------------------------------------
 */
#endif
