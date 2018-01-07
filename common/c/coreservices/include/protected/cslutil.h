/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * util.h: created 2004/05/01 by Dima Dorfman.
 *
 * Copyright (c) 2004 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 */

#ifndef CSLUTIL_H
#define	CSLUTIL_H

#include "paltypes.h"
#include "palstdlib.h"
#include "nberror.h"
#include "cslexp.h"

typedef nb_boolean boolean;

/*
 * Prototypes for utility functions implemented in util.c.
 */

/*! Hexify data.

    Hexadecimal representation of binary data. The len argument specifies the number
    of binary bytes to process. bin must be valid up to len bytes, and txt must be
    valid up to (len * 2) bytes. 
*/
CSL_DEC void 
hexlify(size_t len, const char *bin, char *txt);

/*! Un-hexify data.

    If unhexlify returns false, the text stream was
    not a valid hexadecimal representation, and the contents of the binary stream
    is unspecified.
*/
CSL_DEC int 
unhexlify(size_t len, const char *txt, char *bin);

/*! Base64 decode data.

    @return Length of decoded value. '-1' on error.
*/
CSL_DEC int 
decode_base64(const char* in,       /*!< Base64 data to decode */
              char* out             /*!< On return the decoded string. Buffer has to be valid, not sure what the size requirements are */
              );

/*! Base64 encode data.

    nsl_free() has to be called on the returned string.

    @return NE_OK on success.
    @see decode_base64
*/
CSL_DEC NB_Error 
encode_base64(const char* in,  /*!< Data to encode */
              nb_size inLen,   /*!< Length in bytes of data to encode */
              char** out       /*!< On return encoded string. Call nsl_free() to free data */
              );

CSL_DEC void hmac(const byte *msg, nb_size msglen, const byte *key, nb_size keylen, byte *digest);

CSL_DEC uint32 gpstime(void);
CSL_DEC nb_unixTime unixtime(void);
CSL_DEC nb_unixTime gps2unixtime(uint32 gpstime);
CSL_DEC uint32 unixtime2gps(nb_unixTime unixtime);

CSL_DEC int str_cmpx(const char *s1, const char *s2);
CSL_DEC int str_cmpxi(const char *s1, const char *s2);
CSL_DEC int str_match_simple_pattern(const char* pat, const char* str);
CSL_DEC void str_rev_into(void *out, const void *in, size_t len);
CSL_DEC void str_rev(char* str);

CSL_DEC NB_Error get_sha_hash(const void* pdata, int nsize, byte* psum);

/*
 * Prototype for crc32, implemented in crc32.c.
 */
CSL_DEC uint32 crc32(uint32 crc, const byte *buf, size_t len);

/*
 * Prototype for heapsort, implemented in heapsort.c
 */
/*
 * dmcpherson comment out for now ... need to rationalize with BSD version
 */
CSL_DEC void nim_heapsort(void *vbase, size_t nmemb, size_t size, void *k, int (*compar)(const void *, const void *));


CSL_DEC void obfuscate_buffer(byte* data, uint32 size, const byte* key_data, uint32 key_size);

CSL_DEC char nimtoupper(char c);
CSL_DEC char nimtolower(char c);

CSL_DEC boolean nimisupper(char c);
CSL_DEC boolean nimislower(char c);

CSL_DEC uint32 countbits(uint32 v);

/*
 * name is 2 character string (e.g, "en", "es", ...)
 */
CSL_DEC uint32 tolangid(const char* name);

/*
The strncpyt() is simular to strncpy and function copies not more than n bytes (bytes that follow a null byte are not copied) from the array pointed to by s2 to the array pointed to by s1. If copying takes place between objects that overlap, the behaviour is undefined.
If the array pointed to by s2 is a string that is longer than n bytes, the last n bytes of s2 are copied.
If the array pointed to by s2 is a string that is shorter than n bytes, null bytes are appended to the copy in the array pointed to by s1, until n bytes in all are written.
 */
CSL_DEC char* nim_strncpyt(char *s1, const char *s2, size_t n);
#endif
