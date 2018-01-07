
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_ra_ttf.h */


/*
 * converts between TTF and CTF formats
 */


#ifndef FS_RA_TTF_H
#define FS_RA_TTF_H

typedef struct {
    FS_ULONG glyf_OffsetStart;       /* Offset to first byte. */
    FS_ULONG glyf_OffsetEnd;         /* One past the last byte .*/
    FS_SHORT headerLength;
    FS_ULONG uncmpSize;
    FS_ULONG cmpSize;
    MTX_TABLE_COMPRESS *tc;
    FS_BYTE *cmpStart;               /* Compressed font start (after the header) */
    FS_BYTE *cmpEnd;                 /* just past the last byte of compressed data */
    } MTX_RA_TT_Decomp;

/* prototypes */
MTX_RA_TT_Decomp *MTX_RA_TT_Create( _DS_ FS_BYTE *compressedData);
MTX_RA_TT_Decomp *MTX_RA_TT_ReadRom( _DS_ MTX_RA_TT_Decomp *t, FS_BYTE* data );
FS_ULONG MTX_RA_TT_getUncompressedFileSize(FS_VOID * t);    
FS_BYTE *MTX_RA_Get_TTF_FragmentPtr(_DS_ MTX_RA_TT_Decomp *t, FS_ULONG position, FS_ULONG *byteCountPtr );
FS_VOID MTX_RA_ReleaseTTF_Fragment(_DS_ MTX_RA_TT_Decomp *t, FS_VOID * data );
FS_VOID MTX_RA_TT_Destroy(_DS_ MTX_RA_TT_Decomp *t);

#endif /* FS_RA_TTF_H */
