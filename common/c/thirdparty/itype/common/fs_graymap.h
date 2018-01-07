
/* Copyright (C) 2009 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_graymap.h   */
/* private header */

#ifndef FS_GRAYMAP_H
#define FS_GRAYMAP_H

#ifdef __cplusplus
extern "C" {
#endif

/* exports from "fs_graymap.c" */
#ifdef FS_GRAYMAPS
FS_GRAYMAP *make_graymap(_DS_ FS_OUTLINE *outl, FS_ULONG index, FS_USHORT type, 
                         FS_BOOLEAN cache_ok);
#endif
#if defined(FS_GRAYMAPS) || defined(FS_EMBEDDED_BITMAP)
FS_GRAYMAP *get_graymap(_DS_ FS_ULONG id, FS_USHORT index, FS_USHORT type);
#endif

#if defined(FS_GRAYMAPS) && defined(FS_BITMAPS) 
FS_GRAYMAP *get_phased_graymap(_DS_ FS_ULONG id, FS_SHORT csp);
#endif

#if defined(FS_GRAYMAPS) || defined(FS_EDGE_RENDER)
FS_GRAYMAP *trim_convert(_DS_ FS_GRAYMAP *gmap, FS_USHORT type);
#endif


#ifdef __cplusplus
    }
#endif

#endif /* FS_GRAYMAP_H */
