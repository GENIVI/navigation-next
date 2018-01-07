/*****************************************************************************
 *
 *  tsstubdc.h  - Defines interface to a do-nothing device context.
 *                This implementation of a device context is used
 *                for performance evaluation of WTLE dispaly code.
 *
 *  Copyright (C) 2009 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSSTUBDC_H
#define TSSTUBDC_H

#include "tsdc.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  A typedef for object.
 *
 *  Description:
 *      The TsStubDC class is derived from the TsDC base class.
 *
 *  <GROUP tsstubdc>
 */
typedef struct TsStubDC_
{
    TsDC base;
} TsStubDC;


/*****************************************************************************
 *
 *  Returns a pointer to new stub device context object.
 *
 *  Description:
 *      Creates and initializes a new TsStubDC object.
 *
 *      The stub device context is used for testing the performance of display
 *      code.
 *
 *      Each call to this function must be matched with a subsequent call
 *      to TsStubDCC_delete, when finished.
 *
 *  Parameters:
 *      memMgr - [in]   memory manager object (can be NULL if not using memory manager)
 *
 *  Return value:
 *      Pointer to valid TsStubDC object (cast to TsDC *), or NULL on error.
 *
 *  <GROUP tsstubdc>
 */
TS_EXPORT(TsDC *)
TsStubDC_new(TsMemMgr *memMgr);


/*****************************************************************************
 *
 *  Deletes a TsStubDC object.
 *
 *  Remarks:
 *      Each call to this function must be matched with a prior call to
 *      TsStubDC_new.
 *
 *  Parameters:
 *      dc -        [in] this
 *
 *  Return value:
 *      None
 *
 *  <GROUP tsstubdc>
 */
TS_EXPORT(void)
TsStubDC_delete(TsDC *dc);



TS_END_HEADER

#endif /* TSSTUBDC_H */
