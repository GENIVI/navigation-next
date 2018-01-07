
/* Copyright (C) 2007 Monotype Imaging Inc. All rights reserved. */

/* Confidential Information of Monotype Imaging Inc. */

/* fs_openvg.h */

#ifndef __FS_OPENVG_H__
#define __FS_OPENVG_H__

#ifdef FS_OPENVG

#if defined(FS_MULTI_PROCESS)
#error FS_OPENVG is not compatible with FS_MULTI_PROCESS
#endif

typedef float FS_FLOAT;   /* only used in OpenVG code */

/**
 * OpenVG glyph types
 */
#define FS_VGPATHSCALED     0x0001
#define FS_VGPATHUNSCALED   0x0002
#define FS_VGBITMAP         0x0004
#define FS_VGGRAYMAP        0x0008
#define FS_VGEDGEMAP        0x0010

/**
 *  Structure that describes a glyph as an OpenVG path or image
 *
 *  Description:
 *      This structure contains glyph metrics and image data.
 *      Image data may either be a VGPath or VGImage object depending on the 
 *      specified type.
 *
 *  <GROUP itypestructs>
 */
 typedef struct
 {
    CACHE_ENTRY *cache_ptr;     /* cache-info structure (private)                   */
    FS_LONG size;               /* size of structure in bytes                       */
    FS_FIXED lo_x;              /* smallest x coordinate                            */
    FS_FIXED hi_x;              /* largest x coordinate                             */
    FS_FIXED lo_y;              /* smallest y coordinate                            */
    FS_FIXED hi_y;              /* largest y coordinate                             */
    FS_SHORT i_dx;              /* x-advance in pixels - 0 if rotated or skewed     */
    FS_SHORT i_dy;              /* y-advance in pixels - 0 if rotated or skewed     */
    FS_FIXED dx;                /* x-advance in fractional pixels (16.16 format)    */
    FS_FIXED dy;                /* y-advance in fractional pixels (16.16 format)    */
    FS_ULONG type;              /* type of OpenVG object                            */
    FS_BYTE vgdata[1];          /* data array for FS_VGPATHDATA or FS_VGIMAGEDATA   */
 } FS_VGGLYPH;

/**
 *  Structure that describes path data in a FS_VGGLYPH of a path type
 *
 *  Description:
 *      This structure assists you in extracting information from the vgdata pointer
 *      of a FS_VGGLYPH when the type is either FS_VGPATHSCALED or FS_VGPATHUNSCALED.
 *      This information may be used to contruct and fill a VGPath object.
 *
 *      Note that memory for this object is allocated as part of the FS_VGGLYPH
 *      and should not be allocated or deallocated separately.
 *
 *  <GROUP itypestructs>
 */
 typedef struct
 {
    FS_FLOAT  scale;            /* scale factor for data                            */
    FS_USHORT numSegments;      /* number of path segments                          */
    FS_USHORT numCoord;         /* number of path coordinate pairs                  */
    FS_BYTE  *pathSegments;     /* pointer to path segment data                     */
    FS_FLOAT *pathData;         /* pointer to path coordinate data                  */
 } FS_VGPATHDATA;

 /**
 *  Structure that describes image data in a FS_VGGLYPH of an image type
 *
 *  Description:
 *      This structure assists you in extracting information from the vgdata pointer
 *      of a FS_VGGLYPH when the type is either FS_VGPATHSCALED or FS_VGPATHUNSCALED.
 *      This information may be used to contruct and fill a VGPath object.
 *
 *      Note that memory for this object is allocated as part of the FS_VGGLYPH
 *      and should not be allocated or deallocated separately.
 *
 *  <GROUP itypestructs>
 */
 typedef struct
 {
    FS_USHORT width;       /* width of image            */
    FS_USHORT height;      /* height of image           */
    FS_USHORT stride;      /* data stride               */
    FS_BYTE   pixels[1];   /* image data array (data    */
 } FS_VGIMAGEDATA;


/******************************/
/* Public Functions           */
/******************************/
/* For those API functions that require mutex protection when FS_MULTI_THREAD   */
/* is defined, two versions of the function exist - an FSS_ version without     */
/* mutex protection and an FS_ version with mutex protection. The FS_ version   */
/* calls the FSS_ version after setting the mutex lock.                         */
/* When the code is built without FS_MULTI_THREAD defined, the FS_ version is   */
/* defined to be the FSS_ version since no mutex lock is required.              */

#ifdef FS_MULTI_THREAD
/*****************************************************************************
 *
 *  Returns an OpenVG object.
 *
 *  Description:
 *      This function returns an OpenVG glyph which can be a scaled or unscaled
 *      VGPath or a VGImage (bitmap or graymap). This feature supports systems
 *      that use OpenVG hardware acceleration. 
 *
 *      The glyph returned is specified by an id and type. The id value is either
 *      a character value or glyph index, depending on whether FLAGS_CMAP_ON is set.
 *      If FLAGS_CMAP_ON is set, the id represents a character value. This value is
 *      a Unicode value when a Unicode CMAP is used (platform = 3, encoding = 1 or 10).
 *      Otherwise, the id is a character value in the CMAP table selected.
 *      If FLAGS_CMAP_OFF is set, the id represents a glyph index. 
 *
 *      The type parameter specifies the type of OpenVG glyph to return. Valid types are:
 *      <TABLE>
 *      VGGLYPH Type           Description
 *      -------------------    --------------------------------------------------
 *      FS_VGPATHSCALED        scaled VGPath object
 *      FS_VGPATHUNSCALED      unscaled VGPath object
 *      FS_VGBITMAP            VGImage of type VG_BW_1 (1-bit monochrome bitmap)
 *      FS_VGGRAYMAP           VGImage of type VG_A_8 (8-bit alpha grayscale) 
 *      </TABLE>
 *
 *      Only one type should be specified in the call. The OpenVG object is returned
 *      as data within a FS_VGGLYPH object which also contains metric information.
 *
 *      FS_free_char() should be used to free the FS_VGGLYPH glyph that is returned
 *      by this function.
 *
 *  Parameters:
 *      state       - [in] pointer to FS_STATE structure
 *      id          - [in] character value or glyph index
 *      type        - [in] type of OpenVG glyph to create
 *
 *  Return Value:
 *      Pointer to FS_VGGLYPH structure, or NULL upon error.
 *
 *  See Also:
 *      FS_get_glyphmap, FS_free_char()
 *
 *  Version:
 *      Introduced in Version 4.0
 *
 *  <GROUP getglyphs>
 */ 
ITYPE_API FS_VGGLYPH *FS_get_VGglyph(_DS_ FS_ULONG id, FS_USHORT type);

/* non-mutex protected version is private and not exported */
FS_VGGLYPH *FSS_get_VGglyph(_DS_ FS_ULONG id, FS_USHORT type);

#else  /* single thread */

/* non-mutex protected version is public and exported */
ITYPE_API FS_VGGLYPH *FSS_get_VGglyph(_DS_ FS_ULONG id, FS_USHORT type);
#define FS_get_VGglyph FSS_get_VGglyph

#endif /* FS_MULTI_THREAD */


/******************************/
/* Private Functions          */
/******************************/
FS_VGGLYPH* FSS_get_VGPath(_DS_ FS_ULONG id, FS_USHORT type);
FS_VGGLYPH* FSS_get_VGImage_graymap(_DS_ FS_ULONG id);
FS_VGGLYPH* FSS_get_VGImage_bitmap(_DS_ FS_ULONG id);

#endif /* FS_OPENVG */

#endif /* __FS_OPENVG_H__ */

