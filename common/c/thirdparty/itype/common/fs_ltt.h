/******************************************************************************
 *
 *  fs_ltt.h - Declares the internal structure of an .ltt file.
 *
 *  Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential information of Monotype Imaging Inc.
 *
 *****************************************************************************/


#ifndef FS_LTT_H
#define FS_LTT_H

#include "fs_err.h"
#include "fs_port.h"
#include "fs_object.h"

#include "fs_ltt_api.h"

#define PRE_TARGETOFF_LTT_VERSION 2
#define PRE_NUMICONS_LTT_VERSION 3
#define PRE_CALLBACK_LTT_VERSION 4
#define PRE_FOUR_ONE_LTT_VERSION 5
#define PRE_HEAD_SIZE_INC_VERSION 6
#define CURRENT_LTT_VERSION 7


#define LTT_STATE_PTR   FS_STATE *FS_state_ptr; FS_state_ptr = ltt->fsState

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 *
 *  Structure that describes a Unicode coverage plane.
 *
 *  Description:
 *      This data structure represents a set of coverage 
 *      ranges for a font.
 *
 */
typedef struct FsUnicodePlaneCoverage_
{
    FS_ULONG  range1; /* first 32 bit range */
    FS_ULONG  range2; /* second 32 bit range */

} FsUnicodePlaneCoverage;

/*****************************************************************************
 *
 *  Structure that describes a linked font component.
 *
 *  Description:
 *      This data structure represents a single component font
 *      within a linked font.
 *
 */
struct FsLttComponent_
{
    FILECHAR *fontName;          /* component font name */
    FILECHAR *fileName;          /* component file path */

    FsUnicodePlaneCoverage bmp;  /* bmp coverage range */

    FS_USHORT numGlyphs;         /* number of glyphs in component font */
    FS_USHORT numIcons;          /* number of icons in component font */
        
    FS_USHORT ttcIndex;          /* TrueType collection index if applicable */

    FILECHAR *targetCId;         /* ID in target system */
    FILECHAR *targetDir;         /* font directory in target system */
    FS_ULONG  targetPtr;         /* memory pointer in target system */
    FS_ULONG  targetOff;         /* font offset in target system */
    FS_ULONG  targetLen;         /* font length in target system */

    FS_ULONG  fontOffset;        /* component font offset */
    FS_ULONG  fontSize;          /* component font size */

    FS_BOOLEAN bold;             /* apply pseudo-emboldening ? */
    FS_FIXED   boldPercent;      /* pseudobold percentage */

    FS_BOOLEAN italic;           /* apply italicizing ? */
    FS_FIXED   italicAngle;      /* italics angle in degrees */

#ifdef STROKE_PERCENT_PER_COMPONENT_IS_DESIRED
    FS_FIXED   strokePercent;    /* component font stroke percentage */
#endif

    FsUnicodePlaneCoverage smp;  /* SMP Unicode coverage */
    FsUnicodePlaneCoverage sip;  /* SIP Unicode coverage */
    FsUnicodePlaneCoverage ssp;  /* SSP Unicode coverage */

    FS_ULONG      adjustmentsOffset;   /* offset to adjustments */
    FS_USHORT     numAdjustments;      /* number of adjustments */
    FsPPemAdjust *adjustments;         /* adjustments array */
    FS_USHORT     adjustmentsCapacity; /* size of adjustments array */

    FS_USHORT group;             /* scale/shift group */

    /* below not represented in ltt file */

    FILECHAR *winPath;          /* path of font file used by FontLinker */

    FsLtt *ltt;                 /* parent FsLtt */
};

/*****************************************************************************
 *
 *  Structure that describes a table location within a linked font.
 *
 *  Description:
 *      This data structure stores the offset and size of a merged
 *      table with in a linked font header.
 *
 */
typedef struct FsLttOtTable_
{
    FS_ULONG offset;  /* 0 means can get from component font 0, if it exists */
    FS_ULONG size;    /* table size */

} FsLttOtTable;


typedef struct FsGroupType_
{
    FS_USHORT group;
    FS_USHORT type;

} FsGroupType;

#define FsLTT_GROUPTYPE_0 0


/*****************************************************************************
 *
 *  Structure that describes a linked font.
 *
 *  Description:
 *      This data structure represents a linked font.
 *
 */
struct FsLtt_
{
    FS_USHORT       version;        /* version of linked font header */

    FS_USHORT       numLegal;       /* size of legal name array */

    FILECHAR      **legal;          /* array of legal names */

    FILECHAR       *name;           /* family name */
    FILECHAR       *subfamilyName;  /* subfamily name */

    FILECHAR       *fileversion;    /* user file version string */

    FS_USHORT       numComponents;  /* number of component fonts */
    FS_USHORT       mtrxComponent;  /* index of metric component font */
    FsLttComponent *components;     /* component font array */

    /* *** Notes on tables:
        TTF_NAME is an abbreviated form of full TTF-spec'd table
            ttf_load requires full table
            abbreviated form returned by get_table_structure
            is not a "fixed size" table
        TTF_MAXP
            ttf_load requires it
            returned by get_table_structure
        TTF_OS2
            ttf_load requires it (when not "EDIT_MODE")
            returned by get_table_structure
        TTF_HHEA
            ttf_load requires it
            returned by get_table_structure
        TTF_VHEA
            ttf_load requires it only if VMTX exists
            can be returned by get_table_structure if it exists
        [TTF_POST] does not currently have a declaration
            is not necessarily a "fixed size" table, although
            we will use version 3.0 which IS "fixed size"
            ttf_load DOES NOT query for this
            get_table_structure does NOT handle this table
    *** */

    FsLttOtTable    ttf_name;  /* merged name table */

    FS_ULONG        maxp_off;  /* offset to merged maxp table */

    FS_ULONG        os_2_off;  /* offset to merged OS/2 table */

    FS_ULONG        head_off;  /* offset to merged head table */

    FS_ULONG        hhea_off;  /* offset to merged hhea table */

    FS_ULONG        vhea_off;  /* offset to merged vhea table */

    FS_ULONG        post_off;  /* offset to merged post table */

    FsLttOtTable    cmap;      /* merged cmap table */

    FsLttOtTable    gdef;      /* merged GDEF OpenType table */
    FsLttOtTable    gsub;      /* merged GSUB OpenType table */
    FsLttOtTable    gpos;      /* merged GPOS OpenType table */

    FS_ULONG     groupTypesOffset;   /* offset to group types */
    FS_USHORT    numGroupTypes;      /* number of group types */
    FsGroupType *groupTypes;         /* group type array */
    FS_USHORT    groupTypesCapacity; /* size of group type array */

    /* below not represented in ltt file */

    FS_USHORT capacityComponents; /* size of component array */

    FS_STATE *fsState;            /* iType STATE pointer */
};



#define FSLTT_PRE7_HEAD_SIZE  512
#define FSLTT_FILE_HEAD_SIZE 1024
#define FSLTT_COMP_DESC_SIZE  512


#ifdef __cplusplus
}
#endif

#endif /* FS_LTT_H */
