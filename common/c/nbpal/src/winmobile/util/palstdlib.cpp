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

#include "windows.h"
#include "palstdlib.h"
#include "paltypes.h"
#include "winsock2.h"

uint32 SecondFromGPSTime(SYSTEMTIME currentST);

int
nsl_doubletostr(double v, char* buf, size_t size)
{
	return nsl_snprintf(buf, size, "%lf", v) > 0;
}

unsigned int
nsl_gettimeseconds()
{
	SYSTEMTIME systemTime ={0};
	GetLocalTime( &systemTime );
	return SecondFromGPSTime(systemTime);
}


uint32
SecondFromGPSTime(SYSTEMTIME currentST)
{
	SYSTEMTIME GPSEpochST;
	FILETIME GPSEpochFT;
	ULARGE_INTEGER GPSEpochULI;

	FILETIME currentFT;
	ULARGE_INTEGER currentULI;

	ULARGE_INTEGER difference;

	GPSEpochST.wYear = 1980;
	GPSEpochST.wMonth = 1;
	GPSEpochST.wDay = 6;
	GPSEpochST.wHour = 0;
	GPSEpochST.wMinute = 0;
	GPSEpochST.wSecond = 0;
	GPSEpochST.wMilliseconds = 0;
	GPSEpochST.wDayOfWeek = 0;

	SystemTimeToFileTime(&GPSEpochST, &GPSEpochFT);

	GPSEpochULI.HighPart = GPSEpochFT.dwHighDateTime;
	GPSEpochULI.LowPart = GPSEpochFT.dwLowDateTime;

	SystemTimeToFileTime(&currentST, &currentFT);

	currentULI.HighPart = currentFT.dwHighDateTime;
	currentULI.LowPart = currentFT.dwLowDateTime;

	difference.QuadPart = currentULI.QuadPart - GPSEpochULI.QuadPart;
	difference.QuadPart = difference.QuadPart / 10000000; // convert from 100 ns blocks to seconds

	return difference.LowPart;
}

unsigned long nsl_htonl(unsigned long hostlong)
{
    return htonl(hostlong);
}

unsigned long nsl_ntohl(unsigned long netlong)
{
    return ntohl(netlong);
}

unsigned short nsl_htons(unsigned short hostshort)
{
    return htons(hostshort);
}

unsigned short nsl_ntohs(unsigned short netshort)
{
    return ntohs(netshort);
}

size_t nbi_strlcat(char *dst, const char *src, size_t size)
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

size_t nbi_strlcpy(char *dst, const char *src, size_t size)
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

char* nsl_stristr(const char* str, const char* strSearch)
{
    if ((str != NULL) && (strSearch != NULL))
    {
        unsigned int strLen = nsl_strlen(str);
        unsigned int strSearchLen = nsl_strlen(strSearch);

        if (strLen < strSearchLen)
        {
            return NULL;
        }

        for(unsigned int i=0; i <= (strLen - strSearchLen); i++)
        {
            if (nsl_strnicmp((str + i), strSearch, strSearchLen) == 0)
            {
                return (char *)(str + i);
            }
        }
    }

    return NULL;
}

int nsl_snprintf(char* dst, size_t size, const char *format, ...)
{
    va_list arg;
    int result = 0;

    va_start(arg, format);
    result = nsl_vsnprintf(dst, size, format, arg);
    va_end(arg);

    return result;
}

int nsl_vsnprintf(char* dst, size_t size, const char *format, va_list ap)
{
    int result = _vsnprintf(dst, size, format, ap);

    if (result == -1)
    {
        if ((dst != NULL) && (size > 0))
        {
            dst[size - 1] = '\0'; /* always terminate */
        }
        return -1;
    }
    else if ((result == (int)size) && (dst[size - 1] != '\0'))
    {
        dst[size - 1] = '\0'; /* always terminate */
        return -1;
    }
    else if ((dst == NULL) || (size == 0))
    {
        return -1;
    }
    else
    {
        return result;
    }
}

int nsl_uint64tostr(uint64 v, char* buf, size_t size)
{
    return (nsl_snprintf(buf, size, "%I64u", v) > 0);
}

int nsl_int64tostr(int64 v, char* buf, size_t size)
{
    return (nsl_snprintf(buf, size, "%I64d", v) > 0);
}

char* nsl_getdeviceuniquestring()
{
    return nsl_strdup("F=3Kdf_(qw>Wq@#u");
}
/*
    Implement malloc/realloc/free here so that an application can
    mix and match Debug and Release versions of all of the core
    dlls and anything else that uses them
*/
PAL_DEF void* nsl_malloc_impl(nb_usize size, const char* filename, int32 line)
{
#if defined(_DEBUG) && !defined(_WIN32_WCE)
    if (filename && line)
    {
        return _malloc_dbg(size, _NORMAL_BLOCK, filename, line);
    }
    else
    {
        return _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
    }
#else
    return malloc(size);
#endif
}

PAL_DEF void* nsl_realloc_impl(void* memory, nb_usize size, const char* filename, int32 line)
{
#if defined(_DEBUG) && !defined(_WIN32_WCE)
    if (filename && line)
    {
        return _realloc_dbg(memory, size, _NORMAL_BLOCK, filename, line);
    }
    else
    {
        return _realloc_dbg(memory, size, _NORMAL_BLOCK, __FILE__, __LINE__);
    }
#else
    return realloc(memory, size);
#endif
}

PAL_DEF void nsl_free_impl(void* memory)
{
#if defined(_DEBUG) && !defined(_WIN32_WCE)
    _free_dbg(memory, _NORMAL_BLOCK);
#else
    free(memory);
#endif
}
