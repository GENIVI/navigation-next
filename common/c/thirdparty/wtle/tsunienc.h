/*****************************************************************************
 *
 *  tsunienc.h - Unicode encoding/decoding utilities
 *
 *  Copyright (C) 2003 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSUNIENC_H
#define TSUNIENC_H

#include "tstypes.h"

TS_BEGIN_HEADER


/*****************************************************************************
 *
 *  Enumeration of text encodings.
 *
 *  <GROUP unienc>
 */
typedef enum
{
    TS_ENCODING_NOT_SPECIFIED = 0,
    TS_ASCII,           /* plain ASCII text */
    TS_UTF8,            /* text encoded in UTF-8 */
    TS_UTF16,           /* text encoded in UTF-16 */
    TS_UTF32,           /* text encoded in UTF-32 */
    TS_WCP_1250,        /* text encoded in Windows Code Page 1250 (Central Europe) */
    TS_WCP_1251,        /* text encoded in Windows Code Page 1251 (Cyrillic) */
    TS_WCP_1252,        /* text encoded in Windows Code Page 1252 (Western Europe / US) */
    TS_WCP_1253,        /* text encoded in Windows Code Page 1253 (Greek) */
    TS_WCP_1254,        /* text encoded in Windows Code Page 1254 (Turkish) */
    TS_WCP_1257         /* text encoded in Windows Code Page 1257 (Baltic) */
} TsEncoding;


/*****************************************************************************
 *
 *  Macro for the replacement character.
 *
 *  <GROUP unienc>
 */
#define TS_REPLACEMENT_CHARACTER 0xfffd

/*****************************************************************************
 *
 *  Returns the next Unicode character that is encoded in a UTF8 text array.
 *
 *  Description:
 *      This function returns a single UTF32 Unicode value that has been
 *      decoded from the UTF8 array, starting at source.
 *
 *      Each successful call will consume 1 to 4 code units (bytes)
 *      depending on the character that is encoded.
 *
 *      If srclen is too short to decode the next character then
 *      the function returns the value FFFD, which is the REPLACEMENT CHARACTER
 *      and consumed returns the number of bytes consumed to that point.
 *
 *  Parameters:
 *      source      - [in] pointer to UTF8 text
 *      srclen      - [in] length of source in code units
 *      consumed    - [out] returns the number of code units that are consumed
 *                          in getting this character
 *
 *  Return value:
 *      UTF32 Unicode character.
 *
 *  <GROUP unienc>
 */
TS_EXPORT(TsUInt32)
TsUniEnc_decodeUTF8(TsUInt8 *source, TsLength srclen, TsLength *consumed);


/*****************************************************************************
 *
 *  Returns the next Unicode character that is encoded in a UTF16 text array.
 *
 *  Description:
 *      This function returns a single UTF32 Unicode value that has been
 *      decoded from the UTF16 array, starting at source.
 *
 *      Each successful call will consume 1 to 2 code units (UInt16)
 *      depending on the character that is encoded.
 *
 *      If srclen is too short to decode the next character,
 *      the function returns the value FFFD, which is the REPLACEMENT CHARACTER
 *      and 'consumed' is returned as 1.
 *
 *      If the first character indicates that the character is a surrogate
 *      then the second code unit is verified. If it is an invalid low
 *      surrogate value then the function returns the REPLACEMENT_CHARACTER
 *      and 'consumed' is returned as 1.
 *
 *  Parameters:
 *      source      - [in] pointer to UTF16 text
 *      srclen      - [in] length of source in code units
 *      consumed    - [out] returns the number of code units that are consumed
 *                          in getting this character
 *
 *  Return value:
 *      UTF32 Unicode character.
 *
 *  <GROUP unienc>
 */
TS_EXPORT(TsUInt32)
TsUniEnc_decodeUTF16(TsUInt16 *source, TsLength srclen, TsLength *consumed);






TS_EXPORT(TsLength)
TsUniEnc_toUTF32(TsEncoding enc, void *run, TsLength numread, void *dest, TsLength size);


TS_END_HEADER

#endif /* TSUNIENC_H */
