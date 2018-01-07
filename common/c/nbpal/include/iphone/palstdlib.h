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
 * nimstdlib.h: created 2008/10/01 by Mark Goddard.
 *
 * Copyright (c) 2008 Networks In Motion, Inc.
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

#ifndef _PAL_STDLIB_H_
#define _PAL_STDLIB_H_

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "pal.h"

#include <stdarg.h>

#define nsl_atof      atof
#define nsl_atoi      atoi
#define nsl_free      free
#define nsl_htonl     htonl
#define nsl_htons     htons
#define nsl_isalpha   isalpha
#define nsl_malloc    malloc
#define nsl_memchr    memchr
#define nsl_memcmp    memcmp
#define nsl_memcpy    memcpy
#define nsl_memmove   memmove
#define nsl_memset    memset
#define nsl_ntohl     ntohl
#define nsl_ntohs     ntohs
#define nsl_qsort     qsort
#define nsl_realloc   realloc
#define nsl_snprintf  snprintf
#define nsl_sprintf   sprintf
#define nsl_sscanf    sscanf
#define nsl_strcat    strcat
#define nsl_strchr    strchr
#define nsl_strcmp    strcmp
#define nsl_strcpy    strcpy
#define nsl_strdup    strdup
#define nsl_stricmp   strcasecmp
#define nsl_stristr   strcasestr
#define nsl_strlcat   strlcat
#define nsl_strlcpy   strlcpy
#define nsl_strlen    strlen
#define nsl_strncmp   strncmp
#define nsl_strncpy   strncpy
#define nsl_strnicmp  strncasecmp
#define nsl_strrchr   strrchr
#define nsl_strstr    strstr
#define nsl_strtod    strtod
#define nsl_strtoul   strtoul
#define nsl_vsnprintf vsnprintf
#define nsl_tolower   tolower
#define nsl_toupper   toupper
#define nsl_snprintf snprintf
#define nsl_sprintf sprintf
#define nsl_strlcpy strlcpy
#define nsl_strncpy strncpy
#define nsl_strcat strcat
#define nsl_strtok_r strtok_r

#define nsl_assert(t) assert(t)
#define nsl_qsort qsort
#define nsl_socket  socket
#define nsl_closesocket close
#define nsl_select select

#define nsl_strempty(s) (!s || s[0] == 0)
#define nsl_atoll atoll
#define nsl_strtoull strtoull
#define PACKED
/*! \brief This converts a double value to a string.

 @param v the value to convert
 @param buf A pointer to the buffer to store the string into
 @param size The size of the buffer in bytes
 @return \a >0 If successful <br>
 \a 0 If unsuccessful
 */

PAL_DEC int nsl_doubletostr(double v, char* buf, size_t size);

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

/*! \brief Try to make device unique string, otherwise returns hardcoded value.
 @return \a >Pointer to string. Caller should manage returned memory.<br>
 */
PAL_DEC char* nsl_getdeviceuniquestring();


#endif // _PAL_STDLIB_H_
