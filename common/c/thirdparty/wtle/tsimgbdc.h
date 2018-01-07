/*****************************************************************************
 *
 *  tsimgbdc.h  - Defines interface to image buffer-specific device context.
 *
 *  Copyright (C) 2003, 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSIMGBUFDC_H
#define TSIMGBUFDC_H

#include "tsdc.h"
#include "tsimgbuf.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  A typedef for a simple RGB image buffer object.
 *
 *  Description:
 *      The TsImageBufferDC class is derived from the TsDC base class.
 *
 *  <GROUP imagebufferdc>
 */
typedef struct TsImageBufferDC_
{
    TsDC base;
    TsImageBuffer *imgBuf;
} TsImageBufferDC;


/*****************************************************************************
 *
 *  Returns a pointer to new image buffer device context object.
 *
 *  Description:
 *      Creates and initializes a new TsImageBufferDC object.
 *
 *      The image buffer device context treats a simple TsImageBuffer
 *      as a device for drawing purposes. The rendering code will
 *      draw directly to the RGB buffer. This is useful for testing.
 *
 *      Each call to this function must be matched with a subsequent call
 *      to TsImageBufferDC_delete, when finished.
 *
 *  Parameters:
 *      memMgr - [in]   memory manager object (can be NULL if not using memory manager)
 *      imgBuf - [in]   pointer to a TsImageBuffer object, which encapulates the raw image buffer
 *
 *  Return value:
 *      Pointer to valid TsImageBufferDC object (cast to TsDC *), or NULL on error.
 *
 *  <GROUP imagebufferdc>
 */
TS_EXPORT(TsDC *)
TsImageBufferDC_new(TsMemMgr *memMgr, TsImageBuffer *imgBuf);


/*****************************************************************************
 *
 *  Deletes a TsImageBufferDC object.
 *
 *  Remarks:
 *      Each call to this function must be matched with a prior call to
 *      TsImageBufferDC_new.
 *
 *  Parameters:
 *      dc -        [in] this
 *
 *  Return value:
 *      None
 *
 *  <GROUP imagebufferdc>
 */
TS_EXPORT(void)
TsImageBufferDC_delete(TsDC *dc);



TS_END_HEADER

#endif /* TSIMGBUFDC_H */
