/*****************************************************************************
 *
 *  tstypes.h - Defines the basic data types used throughout the code.
 *
 *  Copyright (C) 2002 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSTYPES_H
#define TSTYPES_H

#include "tsport.h"     /* Port-specific file */


 /****************************************************************************
 *
 *  Typedef for a one byte character.  This may be signed or unsigned
 *  depending on the system.  This is used for I/O and single-byte string functions.
 *
 *  <GROUP basictypes>
 */
typedef char TsChar;


/*****************************************************************************
 *
 *  Typedef for a signed short integer.
 *
 *  <GROUP basictypes>
 */
typedef signed short TsShort;


/*****************************************************************************
 *
 *  Typedef for a signed long integer.
 *
 *  <GROUP basictypes>
 */
typedef signed long TsLong;


/*****************************************************************************
 *
 *  Typedef for an unsigned byte.
 *
 *  <GROUP basictypes>
 */
typedef unsigned char TsByte;


/*****************************************************************************
 *
 *  Typedef for an unsigned short integer.
 *
 *  <GROUP basictypes>
 */
typedef unsigned short TsUShort;


/*****************************************************************************
 *
 *  Typedef for an unsigned long integer.
 *
 *  <GROUP basictypes>
 */
typedef unsigned long TsULong;


 /****************************************************************************
 *
 *  Typedef for a signed 8-bit integer.
 *
 *  <GROUP basictypes>
 */
typedef TsInt8_t TsInt8;


/*****************************************************************************
 *
 *  Typedef for a signed 16-bit integer.
 *
 *  <GROUP basictypes>
 */
typedef TsInt16_t TsInt16;


/*****************************************************************************
 *
 *  Typedef for a signed 32-bit integer.
 *
 *  <GROUP basictypes>
 */
typedef TsInt32_t TsInt32;


/*****************************************************************************
 *
 *  Typedef for an unsigned 8-bit integer.
 *
 *  <GROUP basictypes>
 */
typedef TsUInt8_t TsUInt8;


/*****************************************************************************
 *
 *  Typedef for an unsigned 16-bit integer.
 *
 *  <GROUP basictypes>
 */
typedef TsUInt16_t TsUInt16;


/*****************************************************************************
 *
 *  Typedef for an unsigned 32-bit integer.
 *
 *  <GROUP basictypes>
 */
typedef TsUInt32_t TsUInt32;


/*****************************************************************************
 *
 *  Typedef for a boolean value.
 *
 *  <GROUP basictypes>
 */
typedef TsByte TsBool;


/*****************************************************************************
 *
 *  Typedef used to express the length of something such as the length
 *  of a string in terms of characters.
 *
 *  <GROUP basictypes>
 */
typedef TsInt32 TsLength;


/*****************************************************************************
 *
 *  Typedef used to express the numerical index within an array, loop,
 *  list, or some other countable construct.
 *
 *  <GROUP basictypes>
 */
typedef TsInt32 TsIndex;


/*****************************************************************************
 *
 *  Typedef for the unsigned integral type returned by the sizeof operator,
 *  equivalent to the ANSI C 'size_t" type.  It is used to express memory
 *  block size, file size, or position.
 *
 *  <GROUP basictypes>
 */
typedef TsSize_t TsSize;


/*****************************************************************************
 *
 *  Typedef for the signed integral type used to express the distance between
 *  two pointers, equivalent to the ANSI C 'ptrdiff_t" type.
 *
 *  <GROUP basictypes>
 */
typedef TsPtrDiff_t TsPtrDiff;


/*****************************************************************************
 *
 *  Typedef for a wide character.
 *
 *  <GROUP basictypes>
 */
typedef TsWChar_t TsWChar;


/*****************************************************************************
 *
 *  Typedef used to define a general result or error condition.  A value of 0
 *  always indicates a success.
 *
 *  Note:
 *      This uses the built-in int type (an exception to the general rule to
 *      abstract all types), because this is often used as a return value,
 *      which is usually most efficient as an int.
 *
 *  <GROUP basictypes>
 */
typedef int TsResult;


/*****************************************************************************
 *
 *  Typedef used to define a UTF32 Unicode character.
 *
 *  <GROUP basictypes>
 */
typedef TsUInt32 TsUChar32;


/*****************************************************************************
 *
 *  Enumeration of endian settings.
 *
 *  <GROUP basictypes>
 */
typedef enum
{
    TS_ENDIAN_NOT_SPECIFIED = 0,        /* Not specified */
    TS_ENDIAN_BIG,                      /* Big endian */
    TS_ENDIAN_LITTLE                    /* Little endian */
} TsEndianness;





/**** Deprecated types ******************************************************/



typedef void TsTextSource;
typedef TsInt32 TsCoord;



#endif /* TSTYPES_H */
