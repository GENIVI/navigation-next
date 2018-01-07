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
#include <QtGlobal>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <string>
#include <algorithm>
#include "palstdlib.h"
#include "paltypes.h"

#define UNIQUE_DEVICE_STRING_MAX_LENGTH 50

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

    return(dlen + (s - src));   /* count does not include NUL */
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
            *d = '\0';      /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1);    /* count does not include NUL */
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

PAL_DEF char* nsl_getdeviceuniquestring()
{
    char *uid = NULL;
    return uid;
}

PAL_DEF const char* nsl_stristr(const char* haystack, const char* needle)
{
    std::string str1(haystack);
    std::transform(str1.begin(), str1.end(), str1.begin(), toupper);

    std::string str2(needle);
    std::transform(str2.begin(), str2.end(), str2.begin(), toupper);

    size_t ret = str1.find(str2, 0);
    const char* result = NULL;
    if (ret != std::string::npos)
    {
        result = haystack + ret;
    }

    return result;
}

/*
 * This array is designed for mapping upper and lower case letter
 * together for a case independent comparison.  The mappings are
 * based upon ascii character sequences.
 */
static const unsigned char charmap[] = {
    '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
    '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
    '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
    '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
    '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
    '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
    '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
    '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
    '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
    '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
    '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
    '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
    '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215,
    216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231,
    232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247,
    248, 249, 250, 251, 252, 253, 254, 255

//    '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
//    '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
//    '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
//    '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
//    '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
//    '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
//    '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
//    '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
//    '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
//    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
//    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
//    '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
//    '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
//    '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
//    '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
//    '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377'
};

PAL_DEF int
nsl_strncasecmp(const char *s1, const char *s2, int n)
{
    register unsigned char c1, c2;
    int  v = 0;
    while ((v == 0) && (n > 0) )
    {
        c1 = (unsigned char)*s1++;
        c2 = (unsigned char)*s2++;
        /* The casts are necessary when pStr1 is shorter & char is signed */
        v = charmap[c1] - charmap[c2];
        n--;
    }

    return v;
}

PAL_DEF int
nsl_strcasecmp(const char *s1, const char *s2)
{
    register unsigned char c1, c2;
    int  v;
    do
    {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        /* The casts are necessary when pStr1 is shorter & char is signed */
        v = charmap[c1] - charmap[c2];
    } while ((v == 0) && (c1 != '\0') && (c2 != '\0') );

    return v;
}

PAL_DEF char*
nsl_strtok_r(char *s, const char *delim, char **save_ptr)
{
    char *token;

    if (s == NULL)
        s = *save_ptr;

    /* Scan leading delimiters. */
    s += strspn (s, delim);
    if (*s == '\0')
    {
        *save_ptr = s;
        return NULL;
    }

    /* Find the end of the token. */
    token = s;
    s = strpbrk (token, delim);
    if (s == NULL)
    {
        /* This token finishes the string. */
        *save_ptr = strchr(token, '\0');
    }
    else
    {
        /* Terminate the token and make *SAVE_PTR point past it. */
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}
