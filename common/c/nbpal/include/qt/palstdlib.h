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

/*!--------------------------------------------------------------------------

    @file     palstdlib.h
    @date     03/17/2009
    @defgroup PALSTDLIB_H_LABEL stdlib API wrapper

    C stdlib API wrappers
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#ifndef PALSTDLIB_H
#define PALSTDLIB_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#endif

#include "pal.h"

#ifndef PACKED
#define PACKED
#endif

#define nsl_malloc malloc
#define nsl_realloc realloc
#define nsl_free free
#define nsl_memmove memmove
#define nsl_strcmp strcmp
#define nsl_strncmp strncmp
#define nsl_stricmp nsl_strcasecmp
#define nsl_strnicmp nsl_strncasecmp
#define nsl_strlen strlen
#define nsl_strcpy strcpy
#define nsl_strtod strtod
#define nsl_strtoul strtoul
#define nsl_strchr strchr
#define nsl_strrchr strrchr
#define nsl_memcpy memcpy
#define nsl_memset memset
#define nsl_memcmp memcmp
#define nsl_atoi atoi
#define nsl_atof atof
#define	nsl_sprintf sprintf
#define nsl_strncpy strncpy
#define nsl_strcat strcat
#define nsl_htonl htonl
#define nsl_htons htons
#define nsl_ntohl ntohl
#define nsl_ntohs ntohs
#define nsl_sscanf sscanf
#define nsl_assert(t) assert(t)
#define nsl_isalpha   isalpha
#define nsl_strempty(s) (!s || s[0] == 0)
#define nsl_qsort qsort
#define nsl_strstr strstr
#define nsl_atoll atoll
#define nsl_strtoull strtoull

/*! \brief This converts a double value to a string.

 @param v the value to convert
 @param buf A pointer to the buffer to store the string into
 @param size The size of the buffer in bytes
 @return \a >0 If successful <br>
 \a 0 If unsuccessful
 */

PAL_DEC int nsl_doubletostr(double v, char* buf, size_t size);

PAL_DEC size_t nsl_strlcat(char* dst, const char* src, size_t size);
PAL_DEC size_t nsl_strlcpy(char* dst, const char* src, size_t size);
PAL_DEC char*  nsl_strdup(const char* s);
/*! \brief Converts a uint64 value to a string.

 @param v the value to convert
 @param buf A pointer to the buffer to store the string into
 @param size The size of the buffer in bytes
 @return \a >0 If successful <br>
 \a 0 If unsuccessful
 */
PAL_DEC int nsl_uint64tostr(uint64 v, char* buf, size_t size);

/*! \brief Converts a int64 value to a string.

 @param v the value to convert
 @param buf A pointer to the buffer to store the string into
 @param size The size of the buffer in bytes
 @return \a >0 If successful <br>
 \a 0 If unsuccessful
 */
PAL_DEC int nsl_int64tostr(int64 v, char* buf, size_t size);

/*! Converts a string to an uint64 value

Converts a sequence of decimal numbers to a uint64 value. Leading spaces are ignored
and conversion stops at the first non-numeric character.

@param buf A pointer to the buffer to convert to a uint64
@param value On success, the uint64 equivalent of the string
@return Zero on failure; non-zero on success
*/
PAL_DEC uint64 nsl_strtouint64(const char* buf);

/*! Writes formatted data to string.

nsl_snprintf() write formatted output to a buffer specified by str. nsl_snprintf()
writes at most size bytes (including the trailing byte ('\0')) to str. The
null terminator '\0' is always appended to the output string.

@param str The stroage location of the output.
@param size The maximum number of bytes to store.
@param format The format control string, and see ANSI C standard liberary function
printf() for details.

@return the length of the formatted output string (not including the terminating
null) if the complete formatted output string and the terminating null are stored
in the buffer, -1 otherwise.
*/
PAL_DEC int nsl_snprintf(char* str, size_t size, const char *format, ...);

/*! Writes formatted data to string.

nsl_vsnprintf() write formatted output to a buffer specified by str. nsl_vsnprintf()
writes at most size bytes (including the trailing byte ('\0')) to str. The
null terminator '\0' is always appended to the output string.

@param str The stroage location of the output.
@param size The maximum number of bytes to store.
@param format The format control string, and see ANSI C standard liberary function
printf() for details.
@param ap Pointer to list of arguments

@return the length of the formatted output string (not including the terminating
null) if the complete formatted output string and the terminating null are stored
in the buffer, -1 otherwise.
*/
PAL_DEC int nsl_vsnprintf(char* str, size_t size, const char *format, va_list ap);

/*! \brief Try to make device unique string, otherwise returns hardcoded value.
 @return \a >Pointer to string. Caller should manage returned memory.<br>
 */
PAL_DEC char* nsl_getdeviceuniquestring();

PAL_DEC const char* nsl_stristr(const char* haystack, const char* needle);

/*!
 * \brief Compares the two strings s1 and s2, ignoring the case of the characters.
 * \param length Only compares the first n bytes of s1
 * \return an integer less than, equal to, or greater than zero if s1 is found,
 * respectively, to be less than, to match, or be greater than s2.
 */
PAL_DEC int nsl_strncasecmp(const char *s1, const char *s2, int length);

/*!
 * \brief Compares the two strings s1 and s2, ignoring the case of the characters.
 * \return an integer less than, equal to, or greater than zero if s1 is found,
 * respectively, to be less than, to match, or be greater than s2.
 */
PAL_DEC int nsl_strcasecmp(const char *s1, const char *s2);

/*!
 * \brief Parses a string into a sequence of tokens. On the first call to nsl_strtok_r()
 * the string to be parsed should be specified in str. In each subsequent call that
 * should parse the same string, str should be NULL.
 * \param str
 * \param delim
 * \param saveptr
 * \return A pointer to the next token, or NULL if there are no more tokens.
 */
PAL_DEC char* nsl_strtok_r(char *str, const char *delim, char **saveptr);

#endif
/*! @} */
