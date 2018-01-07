
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_ttf_conv.h */

#ifndef FS_TTF_CONV_H
#define FS_TTF_CONV_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FS_BYTE *ctf1; /* The ctf data  */
    FS_LONG ctfSize1;
    } ctfDescriptor;


typedef struct {
    FS_BYTE *ttf;               /* The TrueType data */
    FS_LONG ttfSize;            /* The TrueType size */

    FS_BYTE *tableStart;        /* The start of the input table */
    FS_BYTE *tableEnd;          /* Points at the first byte past the input table */
    
    ctfDescriptor ctfData;      /* ctf data descriptor */
    
    FS_SHORT convertFormat;
    FS_SHORT indexToLocFormat;
    } TTF_Converter;

typedef struct {
    FS_SHORT            byteCount;
    FS_SHORT            xBits;
    FS_SHORT            yBits;
    FS_SHORT            dxPlus;
    FS_SHORT            dyPlus;
    FS_BYTE             xIsNegative; /* No meaning if xBits == 0 */
    FS_BYTE             yIsNegative; /* No meaning if yBits == 0 */
} coordEncodingType;

FS_SHORT READWORD_INC( FS_BYTE * *pRef );
FS_ULONG READLONG_INC( FS_BYTE * *pRef );

#ifdef __cplusplus
}
#endif

#endif /* FS_TTF_CONV_H */
