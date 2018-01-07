/*****************************************************************************
 *
 *  tsfntref.h - Interface to TsFontSpec class.
 *
 *  Copyright (C) 2003-2005 Monotype Imaging Inc. All rights reserved.
 *
 *  Monotype Imaging Confidential.
 *
 ****************************************************************************/

#ifndef TSFNTREF_H
#define TSFNTREF_H

#include "tsrsrc.h"

TS_BEGIN_HEADER

struct TsFont_;

/*****************************************************************************
 *
 *  The font object.
 *
 *  Description:
 *      This object provides an interface to a font which primarily
 *      provides TsFontStyle objects.  The TsFont class is an abstract class
 *      using the C++ definition, and must be derived from for concrete
 *      font services.
 *
 *  <GROUP font>
 */
typedef struct TsFont_ TsFont;

/* ========================================================================= */

/*****************************************************************************
 *
 *  The font specification object.
 *
 *  Description:
 *      This object provides the required specification to indicate the
 *      font file to be opened and to be utilized for the creation of a
 *      TsFont object.  The TsResource provides the font file name or
 *      the memory address of the font file.  The index indicates the
 *      specific font entity within a multi-font font file.
 *
 *  <GROUP fontspec>
 */
typedef struct TsFontSpec_
{
    TsResource resource;    /* font file location */
    TsIndex index;          /* entity within multi-font font file */
} TsFontSpec;


/*****************************************************************************
 *
 *  Compares two TsFontSpec objects for equality.
 *
 *  Description:
 *      This function will compare two TsFontSpec objects and return an
 *      indication as to whether they both refer to the same font file.
 *
 *  Parameters:
 *      spec1     - [in] pointer to first TsFontSpec object
 *      spec2     - [in] pointer to second TsFontSpec object
 *
 *  Return value:
 *      TsBool value, TRUE if TsFontSpec objects are equal, FALSE otherwise.
 *
 *  <GROUP fontspec>
 */
TS_EXPORT(TsBool)
TsFontSpec_isEqual(TsFontSpec *spec1, TsFontSpec *spec2);


TS_END_HEADER

#endif /* TSFNTREF_H */
