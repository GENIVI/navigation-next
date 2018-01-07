/*****************************************************************************
 *
 *  tstag.h - TsTag type definition and method declarations.
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSTAG_H
#define TSTAG_H

#include "tstypes.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  Represents the four byte TrueType "Tag" data type.
 *
 *  All tags are 4-byte character strings composed of a limited set of
 *  ASCII characters in the 0x20-0x7E range. If a tag consists of three
 *  or less valid ASCII characters, the letters are followed by the requisite
 *  number of spaces (0x20), each consisting of a single byte.
 *
 *  <GROUP tstag>
 */
typedef TsUInt32 TsTag;


/******************************************************************************
 *
 *  Given four character arguments, make a TsTag value.
 *
 *  <GROUP tstag>
 */
#define TsMakeTag(a,b,c,d) (TsTag)((a << 24) | (b << 16) | (c << 8) | d)


/******************************************************************************
 *
 *  Convert TsTag to a TsChar * string.  Result will be terminated
 *  properly with a NULL character.  TsChar * array is assumed to be
 *  at least five (5) characters long.
 *
 *  Parameters:
 *      tag         - [in] tag to convert to string
 *      string      - [out] array of at least five (5) characters for result.
 *
 *  Return value:
 *      TsChar * string pointer to provided, passed-in string.
 *
 *  <GROUP tstag>
 */
TS_EXPORT(TsChar *)
TsTag_toString(TsTag tag, TsChar *string);


/******************************************************************************
 *
 *  Convert TsChar * string to a TsTag.
 *
 *  Parameters:
 *      string      - [in] string to convert to tag
 *
 *  Return value:
 *      TsTag tag
 *
 *  <GROUP tstag>
 */
TS_EXPORT(TsTag)
TsTag_fromString(const TsChar *string);


TS_END_HEADER

#endif /* TSTAG_H */
