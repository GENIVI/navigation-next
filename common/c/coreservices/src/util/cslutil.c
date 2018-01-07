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
 * util.c: created 2004/05/01 by Dima Dorfman.
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

/*
 * Application- and BREW-independent utility functions. These must not depend on any part of the BREW API
 * for which there isn't an ISO C equivalent.
 */

#include "paltypes.h"
#include "palstdlib.h"
#include "palmath.h"
#include "palclock.h"
#include "cslutil.h"
#include "sha1.h"
#include "csltypes.h"


#define GPS_TO_UNIX_OFFSET 315964800

#define	LONGEST_DEBUGF_STRING		500

#define MAX_VALUE_CHECK(a, b) \
    if (a > b) {            \
    	return -1;}


#define INVALID_CHECK(a, b) \
    if (a == b) {            \
    	return -1;}

// See header file for description
CSL_DEF void
hexlify(size_t len, const char *bin, char *txt)
{
	const char *binlim;

	for (binlim = bin + len; bin < binlim; ++bin) {
		*txt++ = "0123456789abcdef"[(*bin & 0xf0) >> 4];
		*txt++ = "0123456789abcdef"[*bin & 0xf];
	}
}

static int
hexchar(char hex)
{
	const char *hexletters = "abcdefABCDEF";
	char *cp;

	if (hex >= '0' && hex <= '9')
		return (hex - '0');
	cp = nsl_strchr(hexletters, hex);
	if (cp == NULL)
		return (-1);
	return ((cp - hexletters) % 6 + 10);
}

// See header file for description
CSL_DEF int
unhexlify(size_t len, const char *txt, char *bin)
{
	char *binlim;
	int a, b;

	for (binlim = bin + len; bin < binlim; txt += 2) {
		a = hexchar(txt[0]);
		b = hexchar(txt[1]);
		if (a < 0 || b < 0)
			return (0);
		*bin++ = (char) ((a << 4) | b);
	}
	return (1);
}

// See header file for description
CSL_DEF int
decode_base64(const char* in, char* out)
{
	byte* pSrc = (byte*)in;
	byte* pDest = (byte*)out;

	nb_size srcSize = 0;
	nb_size destSize = 0;

	uint16 BASE64_INPUT = 4;
	uint16 BASE64_OUTPUT = 3;

	byte BASE64_TABLE[ 0x80 ] = {
	    /*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	    /*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	    /*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	    /*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	    /*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	    /*28-2f*/ 0xFF, 0xFF, 0xFF, 0x3e, 0xFF, 0xFF, 0xFF, 0x3f, //2 = '+' and '/'
	    /*30-37*/ 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, //8 = '0'-'7'
	    /*38-3f*/ 0x3c, 0x3d, 0xFF, 0xFF, 0xFF, 0x40, 0xFF, 0xFF, //2 = '8'-'9' and '='
	    /*40-47*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //7 = 'A'-'G'
	    /*48-4f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, //8 = 'H'-'O'
	    /*50-57*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, //8 = 'P'-'W'
	    /*58-5f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //3 = 'X'-'Z'
	    /*60-67*/ 0xFF, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, //7 = 'a'-'g'
	    /*68-6f*/ 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, //8 = 'h'-'o'
	    /*70-77*/ 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, //8 = 'p'-'w'
	    /*78-7f*/ 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  //3 = 'x'-'z'
	};

    if (in == NULL || out == NULL)
    {
        return -1;
    }
    srcSize = (int)nsl_strlen(in);

    while (srcSize >= 1)
    {
        // 4 inputs...
        byte in1 = *pSrc++;
        byte in2 = *pSrc++;
        byte in3 = *pSrc++;
        byte in4 = *pSrc++;
        srcSize -= BASE64_INPUT; //4

        // Validate ascii...
        MAX_VALUE_CHECK(in1, 0x7f)
        MAX_VALUE_CHECK(in2, 0x7f)
        MAX_VALUE_CHECK(in3, 0x7f)
        MAX_VALUE_CHECK(in4, 0x7f)



        // Convert ascii to base16...
        in1 = BASE64_TABLE[ in1 ];
        in2 = BASE64_TABLE[ in2 ];
        in3 = BASE64_TABLE[ in3 ];
        in4 = BASE64_TABLE[ in4 ];

        // Validate base16...
        INVALID_CHECK( in1, 0xff)
        INVALID_CHECK( in2, 0xff)
        INVALID_CHECK( in3, 0xff)
        INVALID_CHECK( in4, 0xff)
        MAX_VALUE_CHECK(in1, 63)
        MAX_VALUE_CHECK(in2, 63)
        MAX_VALUE_CHECK(in3, 64)
        MAX_VALUE_CHECK(in4, 64)


        // 3 outputs...
        *pDest++ = ((in1 & 0x3f) << 2) | ((in2 & 0x30) >> 4);
        *pDest++ = ((in2 & 0x0f) << 4) | ((in3 & 0x3c) >> 2);
        *pDest++ = ((in3 & 0x03) << 6) | (in4 & 0x3f);
        destSize += BASE64_OUTPUT; //3

        // Padding...
        if (in4 == 64)
        {
            --destSize;
            if (in3 == 64)
            {
                --destSize;
            }
        }
    }

	return destSize;
}

// See header file for description
CSL_DEF NB_Error
encode_base64(const char* in, nb_size inLen, char** out)
{
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* temp = 0;
    int i = 0;

    if (!in || !out)
    {
        return NE_INVAL;
    }

    // Allocate a buffer big enough to handle the encoded output, including null character
    temp = nsl_malloc(((inLen / 3) * 4) + 5);
    if (!temp)
    {
        return NE_NOMEM;
    }
    *out = temp;

    for (i = 0; i < inLen; i += 3)
    {
        int idx = (in[i] & 0xfc) >> 2;
        *temp++ = alphabet[idx];

        idx = (in[i] & 0x03) << 4;

        if (i + 1 < inLen)
        {
            idx |= ((in[i + 1] & 0xf0) >> 4);
            *temp++ = alphabet[idx];
            idx = (in[i + 1] & 0x0f) << 2;

            if (i + 2 < inLen)
            {
                idx |= ((in[i + 2] & 0xc0) >> 6);
                *temp++ = alphabet[idx];
                idx = in[i + 2] & 0x3f;
                *temp++ = alphabet[idx];
            }
            else
            {
                *temp++ = alphabet[idx];
                *temp++ = '=';
            }
        }
        else
        {
            *temp++ = alphabet[idx];
            *temp++ = '=';
            *temp++ = '=';
        }
    }
    *temp = '\0';

    return NE_OK;
}

/*
 * Calculate HMAC(key, msg) and store the result in digest. The caller is responsible for
 * allocating a buffer big enough to hold a digest produced by that hash function.
 */
CSL_DEF void
hmac(const byte *msg, nb_size msglen, const byte *key, nb_size keylen, byte *digest)
{
#define	B	64
    byte ipad[B], opad[B];
    nb_size ki;

    SHA_CTX sha1;

    for (ki = 0; ki < B; ++ki) {
        if (ki < keylen) {
            ipad[ki] = key[ki] ^ 0x36;
            opad[ki] = key[ki] ^ 0x5c;
        } else {
            ipad[ki] = 0x36;
            opad[ki] = 0x5c;
        }
    }

    /* Inner hash. */
    SHAInit(&sha1);
    SHAUpdate(&sha1, ipad, B );
    SHAUpdate(&sha1, (byte *)msg, msglen);
    SHAFinal(digest, &sha1);

    /* Outer hash. */
    SHAInit(&sha1);
    SHAUpdate(&sha1, opad, B);
    SHAUpdate(&sha1, digest, SHS_DIGESTSIZE);
    SHAFinal(digest, &sha1);
#undef B
}

/*
 * Time operations. These are mostly BREW-specific because I don't want to
 * import a full time service implementation most of which won't be needed.
 */

CSL_DEF uint32
gpstime(void)
{
    return PAL_ClockGetGPSTime();
}

CSL_DEF nb_unixTime
unixtime(void)
{
	return (gpstime() + GPS_TO_UNIX_OFFSET);
}

CSL_DEF nb_unixTime
gps2unixtime(uint32 gpstime)
{

	return (gpstime + GPS_TO_UNIX_OFFSET);
}

CSL_DEF uint32
unixtime2gps(nb_unixTime unixtime)
{

	return (uint32)(unixtime - GPS_TO_UNIX_OFFSET);
}

/*
 * String functions.
 */

CSL_DEF int
str_cmpx(const char *s1, const char *s2)
{

	if (s1 == NULL && s2 == NULL)
		return (0);
	else if (s1 == NULL && s2 != NULL)
		return (-1);
	else if (s1 != NULL && s2 == NULL)
		return (1);
	else
		return (nsl_strcmp(s1, s2));
}

CSL_DEF int
str_cmpxi(const char *s1, const char *s2)
{
	if (s1 == NULL && s2 == NULL)
		return (0);
	else if (s1 == NULL && s2 != NULL)
		return (-1);
	else if (s1 != NULL && s2 == NULL)
		return (1);
	else
		return (nsl_stricmp(s1, s2));
}

/*	Simple String Pattern Matching
 *
 *	Parameters:
 *	pat: the pattern to be matched
 *	str: the string, can be NULL
 *	len: number of characters to match, -1 to match all characters
 *
 *	Return Value:
 *	0: no match
 *	1: wildcard (*) match
 *  2: choice (|) match or negative (!) match
 *	3: exact match
 *
 *	Implements patterns of the following forms
 *
 *	1)	Strings which do not contain '!' or '|' or '*': exact match
 *	2)	String which begins with '!': call recursively and negate
 *	3)	Multiple strings separated by '|': TRUE is recursive call
 *		on any of the strings is true
 *	4)	*: any string
 *	5)	NULL str matches empty or NULL pattern or wild card
 */

#define MATCH_EXACT			3
#define MATCH_CHOICE		2
#define MATCH_NEGATIVE		2
#define MATCH_WILDCARD		1
#define MATCH_NONE			0

static int
str_match_simple_pattern_impl(const char* pat, size_t plen, const char* str);

CSL_DEF int
str_match_simple_pattern(const char* pat, const char* str)
{
	return str_match_simple_pattern_impl(pat, pat == NULL ? 0 : nsl_strlen(pat), str);
}

static int
str_match_simple_pattern_impl(const char* pat, size_t plen, const char* str)
{
	const char* e;
	size_t l;

	if (pat == NULL)
		return (str == NULL || str[0] == 0) ? MATCH_EXACT : MATCH_NONE;

	if (pat[0] == '*' && pat[1] == 0)
		return MATCH_WILDCARD;

	e = nsl_strchr(pat, '|');

	if (e != NULL && (e-pat) >= (int) plen)
		e = NULL;

	if (pat[0] == '!') {
		if (plen > 0)
			return str_match_simple_pattern_impl(pat+1, plen-1, str) == 0 ? MATCH_NEGATIVE : MATCH_NONE;
		else
			return MATCH_NONE;
	}
	else if (e != NULL) {
		return str_match_simple_pattern_impl(pat, e-pat, str) ||
			   str_match_simple_pattern_impl(e+1, plen-(e-pat)-1, str) ? MATCH_CHOICE : MATCH_NONE;
	}
	else if (pat[0] == 0 && (str == NULL || str[0] == 0))
		return MATCH_WILDCARD;
	else if (str == NULL)
		return MATCH_NONE;
	else if (nsl_strlen(pat) == nsl_strlen(str))
		return (nsl_strncmp(pat,str,plen) == 0) ? MATCH_EXACT : MATCH_NONE;
	else if (nsl_strlen(pat) > (l=nsl_strlen(str)) && pat[l] == '|')
		return (nsl_strncmp(pat,str,plen) == 0) ? MATCH_CHOICE : MATCH_NONE;
	else
		return MATCH_NONE;
}


CSL_DEF void
str_rev_into(void *out, const void *in, size_t len)
{
	const char *cin = in;
	char *cout = out;
	const char *p;

	p = cin + len;
	while (p > cin)
		*cout++ = *--p;
}

CSL_DEF void
str_rev(char* str)
{
	int l = (int)nsl_strlen(str);
	int n;
	char c;

	for (n=0;n<l/2;n++) {

		c = str[n];
		str[n] = str[l-n-1];
		str[l-n-1] = c;
	}
}

/*
 * Math functions.
 */

CSL_DEF NB_Error
get_sha_hash(const void* pdata, int nsize, byte* psum)
{
	SHA_CTX	ctx;
	SHAInit(&ctx);
	SHAUpdate(&ctx, pdata, nsize);
	SHAFinal(psum, &ctx);
	return NE_OK;
}

CSL_DEF void
obfuscate_buffer(byte* data, uint32 size, const byte* key_data, uint32 key_size)
{
	uint32 p;

	for (p = 0; p < size; p++)
		data[p] ^= key_data[p % key_size];
}

CSL_DEF char
nimtoupper(char c)
{
	return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

CSL_DEF char
nimtolower(char c)
{
	return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

CSL_DEF boolean
nimisupper(char c)
{
	return c >= 'A' && c <= 'Z' ? TRUE : FALSE;
}

CSL_DEF boolean
nimislower(char c)
{
	return c >= 'a' && c <= 'z' ? TRUE : FALSE;
}

CSL_DEF boolean
nimisascii(char c)
{
    return ((c >= 0x00) && ((byte)c <= 0x7f)) ? TRUE : FALSE;
}

CSL_DEF uint32
countbits(uint32 v)
{
	uint32 c; // c accumulates the total bits set in v
	for (c = 0; v; c++)
	{
		v &= v - 1; // clear the least significant bit set
	}

	return c;
}

CSL_DEF uint32
tolangid(const char* name)
{
	uint32 langid = 0x20200000;
	langid = NB_SET_BYTE_0(langid, name[0]);
	langid = NB_SET_BYTE_1(langid, name[1]);
	return langid;
}



CSL_DEF char *
nim_strncpyt(char *s1, const char *s2, size_t n)
{
    size_t len = nsl_strlen(s2);
    size_t i = 0;

    if (len > n)
        i = len - n;

    nsl_strncpy(s1, s2 + i, n);

    return s1;
}
