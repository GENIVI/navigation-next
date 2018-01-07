/*****************************************************************************
 *
 *  otlayout.h - OpenType layout.
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 * Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifndef OTLAYOUT_H
#define OTLAYOUT_H

#include "otfont.h"
#include "otwrdblk.h"

TS_BEGIN_HEADER

struct TsOtLayout_;

/******************************************************************************
 *
 *  Summary:
 *      Implementation object for OpenType layout shaping engine.
 *
 *  Description:
 *      Intended to be an object managed by TsOtLayoutManager but referenced
 *      for the sole purpose of shaping by TsShaper which uses the only
 *      "public" method, TsOtLayout_shape().
 *
 *      This object implements the OT Layout shaping engine, calling
 *      appropriate object methods for available layout tables, such as
 *      the TsOtGSUBAccess and TsOtGPOSAccess object methods.
 *
 *  <GROUP opentype>
 */
typedef struct TsOtLayout_ TsOtLayout;

/******************************************************************************
 *
 *  Provide OT layout shaping (substitution/positioning) on glyph array.
 *
 *  Parameters:
 *      l           - [in] this
 *      word        - [in/out] input/output glyph array and positions
 *      scaledFont  - [in] scaled font object used for some GPOS operations
 *
 *  Return value:
 *      Appropriate error return or 0 on success.
 *
 *  Notes:
 *      Intended to be used by TsShaper.
 *      On invocation, the TsWordBlock is expected to contain a valid array
 *      of OT font glyph IDs to which the OT layout tables are to be applied.
 *      The glyph array within the TsWordBlock may get altered by the
 *      operations specified in the GSUB table of the OT layout tables, if
 *      defined.  The X and Y positioning arrays within the TsWordBlock may
 *      get altered by the operations specified in the GPOS table of the OT
 *      layout tables, if defined.  Return is a relevant TsResult,
 *
 *  <GROUP opentype>
 */
TS_EXPORT(TsResult)
TsOtLayout_shape(TsOtLayout *l, TsOtWordBlock *word, TsOtScaledFont scaledFont);

TS_EXPORT(TsBool)
TsOtLayout_hasGSUB(TsOtLayout *l);

TS_EXPORT(TsBool)
TsOtLayout_hasGPOS(TsOtLayout *l);

/******************************************************************************
 *
 *  Dump or print all information known by the TsOtLayout object.
 *
 *  Parameters:
 *      l           - [in] this
 *
 *  Return value:
 *      void
 *
 *  Notes:
 *      Dumps all information from the OT layout tables...including all
 *      scripts and language systems defined in the tables.  The information
 *      is printed using the TS_PRINT construct, as ported.
 *
 *      The guts of this function are only defined when TS_OT_DUMP is defined
 *      as a preprocessor macro.  TS_OT_DUMP is currently defined automatically
 *      when TS_NDEBUG is NOT defined, and not defined otherwise. (see ottypes.h)
 *
 *  <GROUP opentype>
 *
 */
TS_EXPORT(void)
TsOtLayout_dump(TsOtLayout *l);

TS_END_HEADER

#endif /* OTLAYOUT_H */
