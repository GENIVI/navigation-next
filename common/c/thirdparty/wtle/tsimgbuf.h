/*****************************************************************************
 *
 *  tsimgbuf.h  - Declares the interface to an image buffer representation
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSIMGBUF_H
#define TSIMGBUF_H


#include "ts2d.h"
#include "tsresult.h"

TS_BEGIN_HEADER

#define TS_IMGBUF_BYTES_PER_PIXEL 3    /* assume three bytes (RGB) per pixel */


/*****************************************************************************
 *
 *  A structure that describes a simple color image.
 *
 *  <GROUP imagebuf>
 */
typedef struct TsImageBuffer_
{
    TsLength xSize;     /* number of columns or pixels per scanline */
    TsLength ySize;     /* number of rows or scanlines */
    TsByte **rows;      /* image data */
#ifdef DEBUG_PIXEL_EXTREMES
    TsInt32 minx;
    TsInt32 miny;
    TsInt32 maxx;
    TsInt32 maxy;
#endif
} TsImageBuffer;


/*****************************************************************************
 *
 *  Initializes an image buffer.
 *
 *  Description:
 *      This function initializes the values of an image buffer
 *      structure.  It also allocates space for the image data,
 *      given the width and height arguments. Note that as this
 *      object is an example implementation of a derived class
 *      that a WTLE client would typically create, memory used
 *      by the object is dealt with by using the TS_MALLOC and
 *      TS_FREE macros instead of TsMemMgr calls. The effect of
 *      this is that when TS_MEM_DBG is defined, the memory used
 *      by this object is not reported.
 *
 *      When finished the user should call the TsImageBuffer_delete
 *      function to free the image data memory.
 *
 *  Parameters:
 *      pD              - [in] pointer to existing structure
 *      width           - [in] image width in pixels
 *      height          - [in] image height in pixels
 *      bgColor         - [in] image background color
 *
 *  Return value:
 *      TS_OK or error code.
 *
 *  <GROUP imagebuffer>
 */
TS_EXPORT(TsResult)
TsImageBuffer_init(TsImageBuffer *pD, TsLength width, TsLength height,
                   TsUInt8 bgColor);


/*****************************************************************************
 *
 *  Finishes an image buffer.
 *
 *  Description:
 *      This function cleans up an image buffer structure.  Any
 *      memory owned by the structure is freed.
 *
 *      It is an unchecked runtime error to call this function
 *      without having first initialized the structure by
 *      calling TsImageBuffer_init.
 *
 *  Parameters:
 *      pD              - [in] pointer to existing structure
 *
 *  Return value:
 *      TS_OK or error code.
 *
 *  <GROUP imagebuffer>
 */
TS_EXPORT(void)
TsImageBuffer_done(TsImageBuffer *pD);


TS_END_HEADER

#endif /* TSIMGBUF_H */
