
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_sfnt.h */

#ifndef FS_SFNT_H
#define FS_SFNT_H


#define TTC_TAG           0
#define TTC_NUMFONTS      8
#define TTC_OFFSET        12

typedef FS_LONG sfnt_TableTag;

#define OFFSETTABLESIZE    12L
#define DirectoryEntrySize 16L

#define GET_sfnt_version( sfnt ) ( *((FS_ULONG *)((FS_BYTE *)(sfnt))) )    
#define GET_sfnt_numOffsets( sfnt ) ( *((FS_USHORT *)((FS_BYTE *)(sfnt) + 4)) )
#define GET_sfnt_table_tag( sfnt, i )                 ( *((sfnt_TableTag *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize)) )
#define GET_sfnt_table_checkSum( sfnt, i )            ( *((FS_ULONG *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize + 4)) )
#define SET_sfnt_table_checkSum( sfnt, i, value )    ( *((FS_ULONG *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize + 4)) = value )
#define GET_sfnt_table_offset( sfnt, i )            ( *((FS_ULONG *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize + 8)) )
#define SET_sfnt_table_offset( sfnt, i, value )        ( *((FS_ULONG *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize + 8)) = (value) )
#define GET_sfnt_table_length( sfnt, i )            ( *((FS_ULONG *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize + 12)) )
#define SET_sfnt_table_length( sfnt, i, value )        ( *((FS_ULONG *)((FS_BYTE *)(sfnt) + OFFSETTABLESIZE + (i) * DirectoryEntrySize + 12)) = (value) )

#define GET_ttc_font_offset( sfnt, i )    (( *((int *)((FS_BYTE *)(sfnt) + TTC_OFFSET + i*4)) ))
#define SET_ttc_font_offset( sfnt, i, value )    ( *((int *)((FS_BYTE *)(sfnt) +  TTC_OFFSET + i*4)) = (value) )

#define GET_maxp_numGlyphs( maxp ) ( *((FS_USHORT *)((FS_BYTE *)(maxp) + 4)) )
/*** Begin VDMX ***/

    typedef struct {
        FS_USHORT yPelHeight;
        FS_SHORT yMax;
        FS_SHORT yMin;
    } sfnt_vdmxTable;

    typedef struct {
        FS_BYTE bCharSet;
        FS_BYTE xRatio;
        FS_BYTE yStartRatio;
        FS_BYTE yEndRatio;
    } sfnt_vdmxRatio;

/*** End VDMX ***/


#endif /* FS_SFNT_H */
