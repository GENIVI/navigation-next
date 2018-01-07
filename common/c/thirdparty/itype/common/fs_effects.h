
/* Copyright (C) 2001-2009 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_effects.h */

#ifndef FS_EFFECTS_H
#define FS_EFFECTS_H

/*****************************************************************************
 *
 *   Enables more square looking N-pixel outlines for graymaps
 *
 *   Description:
 *      Define FS_SQUARE_OUTLINE if you prefer a more square looking outline
 *      effect for graymaps. The normal iType default is to round off the corners
 *      of the outline and produce more round looking outlines.
 *
 *  <GROUP fs-config>
 */
#undef FS_SQUARE_OUTLINE

#ifdef __cplusplus
extern "C" {
#endif

#define SPECIALEFFECTS (FLAGS_EMBOSSED | FLAGS_ENGRAVED | FLAGS_OUTLINED | FLAGS_OUTLINED_2PIXEL | \
                        FLAGS_OUTLINED_UNFILLED | FLAGS_OUTLINED_FILLED | FLAGS_OUTLINED_SOFT)

/* Bitmap Special Effects */
#if defined(FS_BITMAPS) || defined(FS_EMBEDDED_BITMAP)

FS_BITMAP *engrave_bitmap(FS_BITMAP *bmap);
FS_BITMAP *emboss_bitmap(FS_BITMAP *bmap);
FS_BITMAP *outline_bitmap(_DS_ FS_BITMAP *src, FS_USHORT n, FS_USHORT filled);

#ifdef FS_PSEUDO_BOLD
FS_BITMAP *pixelbold_bitmap(_DS_ FS_BITMAP *bmap);
FS_BITMAP *pixelbold_embedded_bitmap(_DS_ FS_BITMAP *bmap);
#endif

#endif /* FS_BITMAPS */

/* Graymap Special Effects */
#if defined(FS_GRAYMAPS) || defined(FS_EMBEDDED_BITMAP) || defined(FS_EDGE_RENDER)

FS_GRAYMAP *engrave_graymap(FS_GRAYMAP *gmap);
FS_GRAYMAP *emboss_graymap(FS_GRAYMAP *gmap);
FS_GRAYMAP *outline_graymap(_DS_ FS_GRAYMAP *src, FS_USHORT width, FS_FIXED opacity );
FS_GRAYMAP *soft_outline_graymap(_DS_ FS_GRAYMAP *gmap, 
                                 FS_USHORT width, FS_FIXED opacity );
FS_GRAYMAP *Soften(_DS_ int fx, int fy, FS_GRAYMAP *old );
FS_GRAYMAP *copy_graymap(_DS_ FS_GRAYMAP *p);

#ifdef FS_PSEUDO_BOLD
FS_GRAYMAP *pixelbold_graymap2(_DS_ FS_GRAYMAP *gmap);
FS_GRAYMAP *pixelbold_graymap4(_DS_ FS_GRAYMAP *gmap);
FS_GRAYMAP *pixelbold_graymap8(_DS_ FS_GRAYMAP *gmap);
FS_GRAYMAP *pixelbold_embedded_graymap(_DS_ FS_GRAYMAP *gmap);
#endif

#endif /* FS_GRAYMAPS or FS_EMBEDDED_BITMAP or FS_EDGE_RENDER */

#ifdef __cplusplus
    }
#endif

#endif /* FS_EFFECTS_H */
