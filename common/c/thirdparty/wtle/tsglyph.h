/*****************************************************************************
 *
 *  tsglyph.h - Interface to TsGlyph class.
 *
 *  Copyright (C) 2003, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSGLYPH_H
#define TSGLYPH_H

#include "tsport.h"
#include "ts2d.h"
#include "tsimage.h"
#include "tsmem.h"

TS_BEGIN_HEADER


struct TsGlyph_;


/******************************************************************************
 *
 *  The glyph object.
 *
 *  Description:
 *      This object provides an interface to a glyph.  The TsGlyph class is
 *      an abstract class using the C++ definition, and must be derived
 *      from for concrete glyph services.
 *
 *  <GROUP glyph>
 *
 */
typedef struct TsGlyph_ TsGlyph;


/******************************************************************************
 *
 *  Glyph metrics such as bounding box and advance width.
 *
 *  <GROUP glyph>
 *
 */
typedef struct TsGlyphMetrics_
{
    TsPoint advance;        /* linear advance width */
    TsPoint pixelAdvance;   /* non-linear advance width */
    TsRect boundingBox;     /* bounding box */
} TsGlyphMetrics;



#define TS_GLYPH_TEXT_LAYER             (0x01)
#define TS_GLYPH_EDGE_LAYER             (0x02)
#define TS_GLYPH_SHADOW_LAYER           (0x04)
#define TS_GLYPH_ALL_LAYERS             (0x0F)

#define TS_GLYPH_VGPATH_SCALED          (0x10)
#define TS_GLYPH_VGPATH_UNSCALED        (0x20)
#define TS_GLYPH_VGBITMAP               (0x40)
#define TS_GLYPH_VGGRAYMAP              (0x80)

#define TS_GLYPH_OPENVG                 (0xF0)


/******************************************************************************
 *
 *  Typedef for a bitfield defining glyph layers.
 *
 *  Description:
 *
 *
 *  <GROUP glyph>
 *
 */
typedef int TsGlyphLayer;



/******************************************************************************
 *
 *  The glyph virtual function table.
 *
 *  Description:
 *      This structure provides the "virtual function" pointers that are
 *      necessary for the "abstract class" TsGlyph to provide glyph
 *      services.  This structure must be properly initialized and passed
 *      to the TsGlyph constructor via TsGlyph_new.
 *
 *  <GROUP glyph>
 *
 */
typedef struct TsGlyphFuncs_
{
    void (*releaseExternal) (void *data);   /* release references to font engine data */
    void (*deleteIt)(void *data);   /* delete the derived class object */
    TsPoint  (*getAdvance)  (void *data); /* get glyph's linear advance width */
    TsPoint  (*getPixelAdvance)  (void *data); /* get the glyph's non-linear advance width in pixels */
    TsRect   (*getBoundingBox)(void *data); /* get the glyph's bounding box */
    TsResult (*getImage)(void *data, TsGlyphLayer layer, TsGlyphImage *image); /* return the glyph's contents via an image pointer */
} TsGlyphFuncs;


/******************************************************************************
 *
 *  Creates and initializes a new TsGlyph object.
 *
 *  Parameters:
 *      funcs    - [in] a properly initialized TsGlyphFuncs structure.
 *      data     - [in] the derived class' private data.  This void pointer
 *                      will be passed as the first argument to each of the
 *                      functions defined in TsGlyphFuncs.
 *
 *  Return value:
 *      Pointer to new TsGlyph object or NULL if error.
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsGlyph *)
TsGlyph_new(const TsGlyphFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Creates and initializes a new memory managed TsGlyph object.
 *
 *  Parameters:
 *      memMgr   - [in] pointer to a memory manager object or NULL
 *      funcs    - [in] a properly initialized TsGlyphFuncs structure.
 *      data     - [in] the derived class' private data.  This void pointer
 *                      will be passed as the first argument to each of the
 *                      functions defined in TsGlyphFuncs.
 *
 *  Return value:
 *      Pointer to new TsGlyph object or NULL if error.
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsGlyph *)
TsGlyph_newex(TsMemMgr *memMgr, const TsGlyphFuncs *funcs, void *data);


/******************************************************************************
 *
 *  Deletes a TsGlyph object.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP glyph>
 */
TS_EXPORT(void)
TsGlyph_delete(TsGlyph *glyph);


/******************************************************************************
 *
 *  Releases the hold on font engine-specific external data.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP glyph>
 */
TS_EXPORT(void)
TsGlyph_releaseExternalData(TsGlyph *glyph);


/*****************************************************************************
 *
 *  Increments the reference counter of the TsGlyph object and
 *  returns the pointer to the same object.
 *
 *  Description:
 *      This function ensures proper reference counting so that the copy
 *      can be safely assigned.  Each call to this function must be matched
 *      with a subsequent call to TsGlyph_releaseHandle.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      Pointer to glyph object.
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsGlyph *)
TsGlyph_copyHandle(TsGlyph *glyph);


/*****************************************************************************
 *
 *  Decrements the reference counter of the TsGlyph object.
 *
 *  Description:
 *      This function reverses the action of TsGlyph_copyHandle.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP glyph>
 */
TS_EXPORT(void)
TsGlyph_releaseHandle(TsGlyph *glyph);


/******************************************************************************
 *
 *  Get TsGlyph advance or escapement scaled to pixels as
 *  a 16.16 fixed-point number. This advance should be used to determine
 *  the optimum advance without regard to the pixel grid. It is useful
 *  when determining the length of a line that would occur if glyphs could
 *  be generated and positioned at very high resolution.
 *
 *  This function should not be used to determine actual inter-glyph spacing.
 *  Instead, see TsGlyph_getPixelAdvance(), which takes into account
 *  the true pixel grid of the characters.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      x,y advance width as a TsPoint
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsPoint)
TsGlyph_getAdvance(TsGlyph *glyph);


/******************************************************************************
 *
 *  Get TsGlyph advance or escapement scaled to pixels as
 *  a 16.16 fixed-point number. This advance should be used to determine
 *  the correct inter-character spacing taking into account the true character
 *  pixel grid. Use this function to determine the actual inter-character
 *  spacing.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      x,y integer pixel advance as a TsPoint
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsPoint)
TsGlyph_getPixelAdvance(TsGlyph *glyph);


/******************************************************************************
 *
 *  Get the glyph bounding box relative to its origin.
 *
 *  Description:
 *      The bounding box is a rectangle that encloses all of the pixels
 *      in the glyph.  The bounding box accounts for all font effects that
 *      may affect the bounding box size or location, relative to the origin.
 *      This includes the size, width, edge effects, rendering style,
 *      superscript, subscript, and rotation.
 *
 *  Parameters:
 *      glyph       - [in] this
 *
 *  Return value:
 *      bounding box as a TsRect
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsRect)
TsGlyph_getBoundingBox(TsGlyph *glyph);


/******************************************************************************
 *
 *  Gets an image representation of a single layer of the glyph
 *
 *  Parameters:
 *      glyph       - [in] this
 *      layer       - [in] which layer
 *      image       - [out] returned image
 *
 *  Return value:
 *      TsResult
 *
 *  <GROUP glyph>
 */
TS_EXPORT(TsResult)
TsGlyph_getImage(TsGlyph *glyph, TsGlyphLayer layer, TsGlyphImage *img);


TS_END_HEADER

#endif /* TSGLYPH_H */
