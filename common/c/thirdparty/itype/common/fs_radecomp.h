
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_radecomp.h */

#ifndef FS_RADECOMP_H
#define FS_RADECOMP_H


/* VL - 04/09/2003 */
/* BYTE_CACHE_INTERVAL is encoded as a variable parameter in */
/* the new ACT3-compressed fonts and is assumed to always be */
/* equal to 128 bytes in legacy ACT3-compressed fonts. This  */
/* should now be undefined in order to have new, variable    */
/* interval processing be enabled */
/* #define BYTE_CACHE_INTERVAL        128L */

#define CURRENT_VERSION_NUMBER    3

#define MAGIC_NUMBER            0x5a

/* how many bits to use in the table -- 10 seems about right */
#define TABLE_BITS 10
#define TABLE_SIZE (1<<TABLE_BITS)
#define NEXT_MASK   ((1<<TABLE_BITS)-1)

typedef struct {
    BITIO *bio;
    FS_LONG unCompressedByteCount;    /* original file size */

    /* dictionary stuff */
    FS_LONG num_tokens;    /* number of tokens in dictionary */
    FS_LONG string_len;    /* length of <string> */
    FS_BYTE *string;       /* BYTES for all tokens in dictionary */
    FS_ULONG *offsets;     /* token <i> begins at starts + offsets[i] */
    FS_BYTE *lengths;      /* token <i> is lengths[i] bytes FS_LONG */

    /* canonical huffman code stuff */
    FS_LONG num_groups;    /* number of groups (codes with same number of bits) */
    FS_ULONG *firsts;      /* first huffman code in each group - left justified */
    FS_ULONG *lasts;       /* last huffman code in each group - left justified */
    FS_BYTE *bits;         /* number of significant bits in each group */
    FS_ULONG *tokens;      /* first token number in each group */
    FS_ULONG next;         /* next group of bits ... maintained by <get_next_token> */
    FS_ULONG next_mask;    /* to toss excess bits on left */
    FS_LONG  slowGroup;    /* first group not completely in table */
    
    /* navigation */
    FS_BYTE *bitMarks;     /* point to ROM and decode the ULONGs on the fly */
    
    FS_BYTE *deltaBytes;   /* error in the anchor position compared to the ideal position */
    FS_ULONG token_offset; /* position of first token ... ie: the encoded data */
    /* a table of frequently used huffman codes for quick decoding */
    FS_LONG token_num_table[TABLE_SIZE];
    FS_BYTE token_bits_table[TABLE_SIZE];
    } MTX_RA_DECOMP;


/* prototypes */
MTX_RA_DECOMP *MTX_RA_DECOMP_Create( _DS_ FS_BYTE *data );
#ifdef BYTE_CACHE_INTERVAL /* VL - 4/09/03 */
FS_BYTE *MTX_RA_GetFragmentPtr(_DS_ MTX_RA_DECOMP *t, FS_ULONG position, FS_ULONG *byteCountPtr );
#else
FS_BYTE *MTX_RA_GetFragmentPtr(_DS_ MTX_RA_DECOMP *t, FS_ULONG position, FS_ULONG *byteCountPtr, FS_USHORT byteCacheInterval );
#endif /* BYTE_CACHE_INTERVAL */
FS_BYTE *MTX_TTC_GetTTFGlyphMemory( _DS_ FS_BYTE *spline, FS_ULONG oldLength, FS_ULONG *newLength);
FS_VOID MTX_RA_DECOMP_Destroy(_DS_ MTX_RA_DECOMP *t);
FS_BYTE *ReadSymbol(_DS_ MTX_RA_DECOMP *t, FS_ULONG *len );


#endif /* FS_RADECOMP_H */
