
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_bitio.h */


#ifndef FS_BITIO_H
#define FS_BITIO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    FS_BYTE *array;    /* the array of FS_BYTE */
    FS_LONG index;     /* next available bit index */
    FS_LONG max;       /* maximum bit index */
    FS_LONG high;      /* high water mark - for written arrays */
    } BITIO;

#define MTX_BITIO_ftell(t) (t)->index
#define MTX_BITIO_fseek(t,pos) ((t)->index = (pos))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FS_BITIO_H */
