
/* Copyright (C) 2009 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_bitmap.h   */
/* private header */

#ifndef FS_BITMAP_H
#define FS_BITMAP_H

#ifdef __cplusplus
extern "C" {
#endif


/* exports from fs_line.c */
#if defined(FS_BITMAPS)
FS_VOID line_simple(_DS_ FS_FIXED x0, FS_FIXED y0, FS_FIXED x1, FS_FIXED y1);
FS_VOID line_dropout(_DS_ FS_FIXED x0, FS_FIXED y0, FS_FIXED x1, FS_FIXED y1);
FS_VOID FS_quad(_DS_ FS_FIXED x0, FS_FIXED y0, FS_FIXED x1, FS_FIXED y1, FS_FIXED x2, FS_FIXED y2);
FS_VOID FS_cube(_DS_ FS_FIXED x0, FS_FIXED y0, FS_FIXED x1, FS_FIXED y1, FS_FIXED x2, FS_FIXED y2,FS_FIXED x3, FS_FIXED y3);
#endif /* FS_BITMAPS */

/* exports from fs_tlist.c */
#if defined(FS_BITMAPS)
TLIST *new_tlist(_DS_ TLIST *olist, FS_SHORT max, FS_SHORT num);
FS_VOID delete_tlist(_DS_ TLIST *list);
FS_VOID append_tlist(_DS_ FS_FIXED x, FS_SHORT y, TLIST *list);
FS_VOID remove_tran(_DS_ FS_FIXED x, FS_SHORT y, TLIST *list);
FS_VOID dump_tlist(FILECHAR *s, TLIST *list);
#endif /* FS_BITMAPS */

/* exports from "fs_bitmap.c" */
#if defined(FS_BITMAPS)
FS_BITMAP *new_bitmap(_DS_ FS_OUTLINE *outl);
FS_BITMAP *make_bitmap(_DS_ FS_OUTLINE *p);
FS_VOID nzw_runs(_DS_ FS_BITMAP *bitmap, TLIST *list);
#endif /* FS_BITMAPS */

#if defined(FS_BITMAPS) || defined(FS_EMBEDDED_BITMAP)
FS_BITMAP *get_bitmap(_DS_ FS_ULONG id, FS_USHORT index);
FS_BITMAP *trim_bitmap(_DS_ FS_BITMAP *bmap);
#endif

#ifdef __cplusplus
    }
#endif

#endif /* FS_BITMAP_H */
