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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

#include "palstdlib.h"
#include "paltypes.h"

PAL_DEF int
nsl_doubletostr(double v, char* buf, size_t size)
{
    return nsl_snprintf(buf, size, "%lf", v) > 0;
}

PAL_DEF size_t
nsl_strlcat(char *dst, const char *src, size_t size)
{
    char *d = dst;
    const char *s = src;
    size_t n = size;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
    {
        d++;
    }

    dlen = d - dst;
    n = size - dlen;

    if (n == 0)
    {
        return(dlen + nsl_strlen(s));
    }

    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return(dlen + (s - src));	/* count does not include NUL */
}

PAL_DEF size_t
nsl_strlcpy(char *dst, const char *src, size_t size)
{
    char *d = dst;
    const char *s = src;
    size_t n = size;

    /* Copy as many bytes as will fit */
    if (n != 0)
    {
        while (--n != 0)
        {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (size != 0)
            *d = '\0';		/* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1);	/* count does not include NUL */
}

PAL_DEF int
nsl_snprintf(char* dst, size_t size, const char *format, ...)
{
    va_list arg;
    int result = 0;

    va_start(arg, format);
    result = nsl_vsnprintf(dst, size, format, arg);
    va_end(arg);

    return result;
}

PAL_DEF int
nsl_vsnprintf(char* dst, size_t size, const char *format, va_list ap)
{
    int result = vsnprintf(dst, size, format, ap);
    if (result > size)
    {
        return -1;
    }
    else
    {
        return result;
    }
}

PAL_DEF int
nsl_uint64tostr(uint64 v, char* buf, size_t size)
{
    return (nsl_snprintf(buf, size, "%llu", v) > 0);
}

PAL_DEF int
nsl_int64tostr(int64 v, char* buf, size_t size)
{
    return (nsl_snprintf(buf, size, "%lld", v) > 0);
}

PAL_DEF char*
nsl_strdup(const char* s)
{
    if (s)
    {
        return strdup(s);
    }
    else
    {
        return NULL;
    }
}

PAL_DEF char*
nsl_getdeviceuniquestring()
{
    return "NOT-Implemented!";
}
