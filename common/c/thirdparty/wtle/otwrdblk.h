/*****************************************************************************
 *
 *  otwrdblk.h - Defines interface to TsOtWordBlock class.
 *
 *  Copyright (C) 2005-2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 ****************************************************************************/

/*****************************************************************************
 *  Note: this file is for WT Shaper's internal use only, these objects and
 *        methods are not intended for use by client code.
 ****************************************************************************/

#ifndef OTWRDBLK_H
#define OTWRDBLK_H

#include "tsglypos.h"
#include "tstypes.h"

TS_BEGIN_HEADER


typedef struct TsOtWordBlockFuncs_
{
    /* number of valid elements in collection */
    TsLength (*length)(void *derived);

    /* grow collection by "count" new elements following position "index" */
    TsResult (*append)(void *derived, TsIndex index, TsLength count);

    TsUInt32 (*getUnicode)(void *derived, TsIndex index); /* debug only */

    TsUInt16 (*getGlyphID)(void *derived, TsIndex index);
    void     (*setGlyphID)(void *derived, TsIndex index, TsUInt16 gID);

    TsContextPosition (*getContext)(void *derived, TsIndex index);
    void              (*setContext)(void *derived, TsIndex index, TsContextPosition cp);

    /* this used for "get" AND "set" */
    TsPositionAdjust *(*positionAdjust)(void *derived, TsIndex index);

} TsOtWordBlockFuncs;


typedef struct TsOtWordBlock_
{
    void *derived;

    const TsOtWordBlockFuncs *funcs;  /* pointer to functions that implement this object */
#ifdef TS_USE_DYNAMIC_FUNCTION_POINTER_TABLES
    TsOtWordBlockFuncs funcTable;              /* function pointer table */
#endif

} TsOtWordBlock;


TS_EXPORT(void)
TsOtWordBlock_init(TsOtWordBlock *wb, void *data, const TsOtWordBlockFuncs *funcs);


/*
TS_EXPORT(TsLength)
TsOtWordBlock_length(TsOtWordBlock *wb);
*/
#define TsOtWordBlock_length(wb) (wb->funcs->length(wb->derived))


TS_EXPORT(TsResult)
TsOtWordBlock_append(TsOtWordBlock *wb, TsIndex index, TsLength count);


TS_EXPORT(TsUInt32)
TsOtWordBlock_getUnicode(TsOtWordBlock *wb, TsIndex index);


/*
TS_EXPORT(TsUInt16)
TsOtWordBlock_getGlyphID(TsOtWordBlock *wb, TsIndex index);
*/
#define TsOtWordBlock_getGlyphID(wb, i) (wb)->funcs->getGlyphID((wb)->derived, i)

TS_EXPORT(void)
TsOtWordBlock_setGlyphID(TsOtWordBlock *wb, TsIndex index, TsUInt16 gID);


TS_EXPORT(TsContextPosition)
TsOtWordBlock_getContext(TsOtWordBlock *wb, TsIndex index);


TS_EXPORT(void)
TsOtWordBlock_setContext(TsOtWordBlock *wb, TsIndex index, TsContextPosition cp);


TS_EXPORT(TsPositionAdjust *)
TsOtWordBlock_positionAdjust(TsOtWordBlock *wb, TsIndex index);


TS_END_HEADER

#endif /* OTWRDBLK_H */
