
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_tablecmp.h */

#ifndef FS_TABLECMP_H
#define FS_TABLECMP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
    {
    FS_ULONG           start;         /* uncompressed "start"                */
    FS_ULONG           length;        /* uncompressed length                 */
    FS_ULONG           cmpStart;      /* actual compressed start             */
    FS_SHORT           isCompressed;  /* non zero if rom block is compressed */
    MTX_RA_DECOMP     *decomp;        /* decompressor if block is compressed */
    } MTX_ROM_BLOCK;

typedef struct
    {
    int numTags;
    sfnt_TableTag *tagsToCompress;
    FS_LONG glyf_OffsetStart;        /* Offset to first byte. */
    FS_LONG glyf_OffsetEnd;          /* One past the last byte .*/
    FS_BYTE *sfnt0;
    FS_LONG sizeIn;
    FS_BYTE *sfnt1;
    FS_LONG sizeOut;
    FS_SHORT numSlots;    /* total number of slots in rb[] */
    FS_SHORT numEntries;  /* number of slots used in rb[]  */
    MTX_ROM_BLOCK *rb;
    } MTX_TABLE_COMPRESS;


FS_VOID    MTX_TABLE_COMPRESS_Destroy          (_DS_ MTX_TABLE_COMPRESS *t);
MTX_TABLE_COMPRESS *MTX_TABLE_COMPRESS_Create( _DS0_ );

#ifdef __cplusplus
}
#endif

#endif /* FS_TABLECMP_H */
