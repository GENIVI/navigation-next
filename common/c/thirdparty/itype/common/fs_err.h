
/* Copyright (C) 2001-2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential information of Monotype Imaging Inc. */

/* fs_err.h */


#ifndef FS_ERR_H
#define FS_ERR_H


/* no error */
#define SUCCESS                      0
#define NO_ERR                       0


/* file errors */
#define ERR_FILE_CREATE              101
#define ERR_FILE_OPEN                102
#define ERR_FILE_CLOSE               103
#define ERR_FILE_SEEK                104
#define ERR_FILE_TELL                105
#define ERR_FILE_READ                106
#define ERR_FILE_WRITE               107

/* memory errors */
#define ERR_MALLOC_FAIL              201
#define ERR_REALLOC_FAIL             202
#define ERR_FREE_BAD_PTR             204
#define ERR_RESIZE_FAIL              206
#define ERR_BLOCK_GRANULARITY        207
#define ERR_BLOCK_SIZE               208
#define ERR_BLOCK_BEFORE_FIRST       209
#define ERR_BLOCK_AFTER_LAST         210
#define ERR_BLOCK_DAMAGED            211
#define ERR_BLOCK_IS_FREE            212
#define ERR_BLOCK_IN_USE             213
#define ERR_FREE_LIST_DAMAGED        214

/* SFNT/LFNT ERRORS */
#define ERR_FONT_NOT_FOUND           301
#define ERR_NO_CURRENT_SFNT          302
#define ERR_NO_CURRENT_LFNT          303
#define ERR_BAD_LFNT                 304
#define ERR_NO_CURRENT_FNTSET        305
#define ERR_DUPLICATED_FONT_NAME     306
#define ERR_FONT_BUFFER_TOO_SMALL    310
#define ERR_FONT_NAME_NOT_UNIQUE     311
#define ERR_FONT_NAME_NOT_FOUND      312
#define ERR_DELETE_FONT              313
#define ERR_FONT_NAME_IN_USE         314
#define ERR_BAD_TYPESET              315
#define ERR_BAD_FNTSET               316
#define ERR_BAD_SFNT                 317

/* TT errors */
#define ERR_CMAP_UNSUPPORTED         402
#define ERR_NOT_A_TTF                403
#define ERR_BAD_TTC_INDEX            404
#define ERR_TABLE_NOT_FOUND          405
#define ERR_BAD_GLYF_INDEX           406
#define ERR_BAD_GLYF_FORMAT          407
#define OUT_OFF_SEQUENCE_CALL_ERR    412
#define UNDEFINED_INSTRUCTION_ERR    416
#define TRASHED_MEM_ERR              417
#define POINTS_DATA_ERR              418
#define CONTOUR_DATA_ERR             419
#define EBLC_VERSION                 426
#define EBLC_NO_METRICS              427
#define EBLC_COMP_SIZE               428
#define ERR_INDEX_FORMAT             429
#define ERR_ACT3_DISK                430        /* only support RAM/ROM this time */
#define ERR_ACT3_UNDEF               431        /* go define FS_ACT3 ... */
#define ERR_STIK_UNDEF               432        /* go define FS_STIK ... */
#define ERR_CCC_UNDEF                433        /* go define FS_CCC  ... */
#define ERR_PHASED_UNDEF             435        /* go define FS_PHASED ... */
#define ERR_PFR_UNDEF                436        /* go define PFR ...*/
#define ERR_BOLD_UNDEF               437        /* go define FS_PSEUDO_BOLD... */
#define ERR_CONTOURCHECK_UNDEF       438        /* go define FS_CONTOUR_WINDING_DETECTION... */
#define ERR_FS_EDGE_HINTS_UNDEF      439        /* go define FS_EDGE_HINTS */
#define ERR_FS_EDGE_RENDER_UNDEF     440        /* go define FS_EDGE_RENDER */
#define ERR_TABLE_UNSUPPORTED        441
#define ERR_NOT_A_LTT                442
#define ERR_BAD_LTT_NUM_FONTS        443
#define ERR_BAD_LTT_METRIC_FONT      444
#define ERR_BAD_LTT_INDEX            445
#define ERR_BAD_FONT_TYPE            446
#define ERR_INVALID_FILE_SIZE        447
#define ERR_BAD_TABLE_DIR            448
#define ERR_BAD_TABLE_CHECKSUM       449
#define ERR_LINKED_FONTS_UNDEF       450
#define ERR_BAD_LTT_COMPONENT        451
#define ERR_CFF_UNDEF                452        /* go define FS_CFFR */
#define ERR_CFF_MAXP                 453
#define ERR_CFF_BAD_OPCODE           456
#define ERR_CFF_CID                  457
#define ERR_CFF_MULTIPLE_FONTS       458
#define ERR_CFF_CHARSTRINGTYPE       459
#define ERR_CFF_BAD_SUBRSPTR         460
#define ERR_CFF_BAD_OFFSET_SIZE      461

/* random errors */
#define ERR_SCALE_LIMIT              502
#define ERR_SCALE_DEGENERATE         503
#define ERR_NO_MATCHING              505
#define ERR_BAD_REF_COUNT            508
#define ERR_POSIX_THREADS_NOT_SUPP   512        /* POSIX threads not supported on this UNIX system */
#define ERR_NO_ICONS                 513
#define ERR_BAD_ICON_INDEX           514
#define ERR_NOT_VANILLA              516
#define ERR_VDMX_RATIO               517
#define ERR_yPelHeight_NOT_FOUND     518
#define ERR_NOT_SUPPORTED            519
#define ERR_INVALID_CMAP             520
#define ERR_INVALID_ADDRESS          521
#define ERR_xPelHeight_NOT_FOUND     522
#define ERR_BAD_EXTERNAL_HEAP        523
#define ERR_RASTER_RESOURCE_LIMIT    524



#define ERR_TINY_TYPE                601
#define ERR_BYTE_TYPE                602
#define ERR_SHORT_TYPE               603
#define ERR_USHORT_TYPE              604
#define ERR_LONG_TYPE                605
#define ERR_ULONG_TYPE               606
#define ERR_SIZEOF_LONG              607
#define ERR_BYTE_ORDER               608
#define ERR_INT_64                   609
#define ERR_FONT_IN_USE              610
#define ERR_BAD_EFFECT               612
#define ERR_BAD_PERCENT              613
#define ERR_BAD_OUTLINE_WIDTH        614    
#define ERR_BAD_OUTLINE_OPACITY      615    

/* table pointer errors */
#define ERR_CCC_BAD_TABLE            701
#define ERR_FREE_BAD_COUNT           702

/* mutex */
#define ERR_MUTEX_CREATE             801
#define ERR_MUTEX_GONE               802
#define ERR_MUTEX_TIMEOUT            803
#define ERR_MUTEX_RELEASE            804

/* shared memory */
#define ERR_SHMEM_CREATE             821
#define ERR_SHMEM_ATTACH             822
#define ERR_SHMEM_DETACH             823
#define ERR_SHMEM_DELETE             824
#define ERR_SHMEM_OPEN               825

/* file mapping */
#define ERR_FILE_UNMAP               831
#define ERR_CREATE_FILE_MAPPING      832
#define ERR_MAP_FILE_VIEW            833

#define ERR_bad_token_code          1030

#define NOT_DEFINED                 (FS_SHORT)0x7FFF

#endif /* FS_ERR_H */
