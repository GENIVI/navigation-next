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

    @file     teststdlib.c
    @date     03/17/2009
    @defgroup TESTSTDLIB_H_LABEL stdlib PAL unit tests

    Unit tests cases for stdlib PAL

    This file contains all unit tests for the stdlib PAL component
*/
/*
    See file description in header file.

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

/* all the file's code goes here */
#include "teststdlib.h"
#include "main.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "pal.h"
#include "paldebuglog.h"

// Disable "warning C4127: conditional expression is constant" because of macros
#pragma warning(disable:4127)

const char ALL_ZERO=0;
const char ALL_ONE=0xff;
const double PRECISION=0.000001;

#define BUF_SIZE    32

// All tests. Add all tests to the TestFile_AddAllTests function

static int IsMemoryWritable(char *ptr, uint32 size)
{
    uint32 i = 0;

    CU_ASSERT_FATAL(ptr != NULL);
    CU_ASSERT_FATAL(size != 0);

    for(i=0; i<size; i++)
    {
        *(ptr+i) = ALL_ZERO;
    }
    for(i=0; i<size; i++)
    {
        if (*(ptr+i) != ALL_ZERO)
        {
            return FALSE;
        }
    }

    for(i=0; i<size; i++)
    {
        *(ptr+i) = ALL_ONE;
    }
    for(i=0; i<size; i++)
    {
        if (*(ptr+i) != ALL_ONE)
        {
            return FALSE;
        }
    }

    return TRUE;
}


static void TestSimulatorRunning(void)
{
    CU_ASSERT_EQUAL(PAL_IsSimulator(NULL), TRUE);
}

static void TestTypes(void)
{
    CU_ASSERT_EQUAL(sizeof(byte), 1);

    CU_ASSERT_EQUAL(sizeof(uint8), 1);
    CU_ASSERT_EQUAL(sizeof(int8), 1);

    CU_ASSERT_EQUAL(sizeof(uint16), 2);
    CU_ASSERT_EQUAL(sizeof(int16), 2);

    CU_ASSERT_EQUAL(sizeof(uint32), 4);
    CU_ASSERT_EQUAL(sizeof(int32), 4);

    CU_ASSERT_EQUAL(sizeof(uint64), 8);
    CU_ASSERT_EQUAL(sizeof(int64), 8);
}


static void TestMemset(void)
{
    char buf[BUF_SIZE];
    int allZeroResult = TRUE;
    int allOneResult = TRUE;
    int i = 0;

    buf[0] = ALL_ONE;
    buf[BUF_SIZE-1] = ALL_ONE;
    CU_ASSERT(nsl_memset(buf+1, ALL_ZERO, BUF_SIZE-2) == (buf+1));
    CU_ASSERT(buf[0] == ALL_ONE);
    CU_ASSERT(buf[BUF_SIZE-1] == ALL_ONE);

    for(i=1; i<BUF_SIZE-1; i++)
    {
        if (buf[i] != ALL_ZERO)
        {
            allZeroResult = FALSE;
            break;
        }
    }
    CU_ASSERT(allZeroResult == TRUE);

    buf[0] = ALL_ZERO;
    buf[BUF_SIZE-1] = ALL_ZERO;
    CU_ASSERT(nsl_memset(buf+1, ALL_ONE, BUF_SIZE-2) == (buf+1));
    CU_ASSERT(buf[0] == ALL_ZERO);
    CU_ASSERT(buf[BUF_SIZE-1] == ALL_ZERO);

    for(i=1; i<BUF_SIZE-1; i++)
    {
        if (buf[i] != ALL_ONE)
        {
            allOneResult = FALSE;
            break;
        }
    }
    CU_ASSERT(allOneResult == TRUE);
}

static void TestMemcmp(void)
{
    char srcBuf[BUF_SIZE];
    char dstBuf[BUF_SIZE];

    nsl_memset(srcBuf, ALL_ZERO, sizeof(srcBuf));
    nsl_memset(dstBuf, ALL_ZERO, sizeof(dstBuf));

    CU_ASSERT(nsl_memcmp(srcBuf, dstBuf, BUF_SIZE) == 0);

    dstBuf[BUF_SIZE-1] = 1;
    CU_ASSERT(nsl_memcmp(srcBuf, dstBuf, BUF_SIZE) < 0);

    srcBuf[0] = 1;
    CU_ASSERT(nsl_memcmp(srcBuf, dstBuf, BUF_SIZE) > 0);
}

static void TestMemcpy(void)
{
    char srcBuf[BUF_SIZE];
    char dstBuf[BUF_SIZE];
    int memcpyResult = TRUE;
    int i = 0;

    nsl_memset(srcBuf, ALL_ZERO, sizeof(srcBuf));
    nsl_memset(dstBuf, ALL_ONE, sizeof(dstBuf));

    CU_ASSERT(nsl_memcpy(dstBuf+1, srcBuf, sizeof(dstBuf)-2) == (dstBuf+1));
    CU_ASSERT(dstBuf[0] == ALL_ONE);
    CU_ASSERT(dstBuf[BUF_SIZE-1] == ALL_ONE);
    for(i=1; i<BUF_SIZE-1; i++)
    {
        if (dstBuf[i] != ALL_ZERO)
        {
            memcpyResult = FALSE;
            break;
        }
    }
    CU_ASSERT(memcpyResult == TRUE);
}

static void TestMemmove(void)
{
    char srcBuf[BUF_SIZE];
    char dstBuf[BUF_SIZE];
    int i = 0;
    int memmoveResult = TRUE;
    char largeBuf[BUF_SIZE*2];

    nsl_memset(srcBuf, ALL_ZERO, sizeof(srcBuf));
    nsl_memset(dstBuf, ALL_ONE, sizeof(dstBuf));

    CU_ASSERT(nsl_memmove(dstBuf+1, srcBuf, BUF_SIZE-2) == (dstBuf+1));
    CU_ASSERT(dstBuf[0] == ALL_ONE);
    CU_ASSERT(dstBuf[BUF_SIZE-1] == ALL_ONE);
    for(i=1; i<BUF_SIZE-1; i++)
    {
        if (dstBuf[i] != ALL_ZERO)
        {
            memmoveResult = FALSE;
            break;
        }
    }
    CU_ASSERT(memmoveResult == TRUE);

    /* overlapping memory move test*/
    for(i=0; i<sizeof(largeBuf); i++)
    {
        largeBuf[i] = (char)i;
    }
    CU_ASSERT(nsl_memmove(largeBuf+BUF_SIZE-1, largeBuf, BUF_SIZE)
            == (largeBuf+BUF_SIZE-1));

    memmoveResult = TRUE;
    for(i=0; i<BUF_SIZE; i++)
    {
        if (largeBuf[i+BUF_SIZE-1] != i)
        {
            memmoveResult = FALSE;
            break;
        }
    }
    CU_ASSERT(memmoveResult = TRUE);
    CU_ASSERT(largeBuf[2*BUF_SIZE-1] == (2*BUF_SIZE-1));
}

static void TestMalloc(void)
{
    char *pFirstMalloc = NULL;
    char *pSecondMalloc = NULL;

    pFirstMalloc = nsl_malloc(BUF_SIZE);
    CU_ASSERT(pFirstMalloc != NULL);
    CU_ASSERT(IsMemoryWritable(pFirstMalloc, BUF_SIZE) == TRUE);

    pSecondMalloc = nsl_malloc(BUF_SIZE);
    CU_ASSERT(pSecondMalloc != NULL);
    CU_ASSERT(IsMemoryWritable(pSecondMalloc, BUF_SIZE) == TRUE);
    CU_ASSERT(pFirstMalloc != pSecondMalloc);

    nsl_free(pFirstMalloc);
    nsl_free(pSecondMalloc);
}

static void TestRealloc(void)
{
    char *pFirstMalloc = NULL;
    char *pRealloc= NULL;
    const int NEW_BUF_SIZE = BUF_SIZE * 2;
    int i=0;
    int unchanged = TRUE;

    pFirstMalloc = nsl_malloc(BUF_SIZE);
    CU_ASSERT(pFirstMalloc != NULL);
    CU_ASSERT(IsMemoryWritable(pFirstMalloc, BUF_SIZE) == TRUE);

    for(i=0; i<BUF_SIZE; i++)
    {
        *(pFirstMalloc+i) = (char)i;
    }

    pRealloc= nsl_realloc(pFirstMalloc, NEW_BUF_SIZE);
    CU_ASSERT(pRealloc != NULL);
    for(i=0; i<BUF_SIZE; i++)
    {
        if (*(pRealloc+i) != i)
        {
            unchanged = FALSE;
            break;
        }
    }
    CU_ASSERT(unchanged == TRUE);
    CU_ASSERT(IsMemoryWritable(pRealloc, NEW_BUF_SIZE) == TRUE);

    nsl_free(pRealloc);
}

static void TestAtoi(void)
{
    CU_ASSERT_EQUAL(nsl_atoi("0"), 0);
    CU_ASSERT_EQUAL(nsl_atoi("+0"), 0);
    CU_ASSERT_EQUAL(nsl_atoi("-0"), 0);
    CU_ASSERT_EQUAL(nsl_atoi("123"), 123);
    CU_ASSERT_EQUAL(nsl_atoi("123 "), 123);
    CU_ASSERT_EQUAL(nsl_atoi(" 123"), 123);
    CU_ASSERT_EQUAL(nsl_atoi(" 123 "), 123);
    CU_ASSERT_EQUAL(nsl_atoi("+123 "), 123);
    CU_ASSERT_EQUAL(nsl_atoi("-123"), -123);
    CU_ASSERT_EQUAL(nsl_atoi("       -123          "), -123);

    CU_ASSERT_EQUAL(nsl_atoi("2147483647"), 2147483647);
    CU_ASSERT_EQUAL(nsl_atoi("-2147483647"), -2147483647);
}

static void TestAtof(void)
{
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("12.3456"), 12.3456, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("-12.3456"), -12.3456, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("12.3456e19"), 12.3456e19, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("-.1e+9"), -.1e+9, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("1e20"), 1e20, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof(".125"), .125, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("0e-19"), 0, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("4\0ab"), 4, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_atof("5.9e-76"), 5.9e-76, PRECISION);
}

static void TestDoubleToStr(void)
{
    char buf[BUF_SIZE];
    char *pStart = buf + 1;

    nsl_memset(buf, ALL_ONE, sizeof(buf));

#define TEST_DOUBLE_TO_STR(d) do \
    {\
        CU_ASSERT(nsl_doubletostr(d, pStart, BUF_SIZE-2) > 0);\
        CU_ASSERT_EQUAL(buf[0], ALL_ONE);\
        CU_ASSERT_EQUAL(buf[BUF_SIZE-1], ALL_ONE);\
        CU_ASSERT_DOUBLE_EQUAL(nsl_atof(pStart), d, PRECISION);\
    } while (0)

    TEST_DOUBLE_TO_STR(-12.3456);
    TEST_DOUBLE_TO_STR(12.3456e19);
    TEST_DOUBLE_TO_STR(-.1e+9);
    TEST_DOUBLE_TO_STR(1e20);
    TEST_DOUBLE_TO_STR(.125);
    TEST_DOUBLE_TO_STR(0e-19);
    TEST_DOUBLE_TO_STR(4);
    TEST_DOUBLE_TO_STR(5.9e-76);

#undef TEST_DOUBLE_TO_STR
}

static void TestHtonl(void)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    /* little endian host */
    CU_ASSERT_EQUAL(nsl_htonl(0x12345678), 0x78563412);
    CU_ASSERT_EQUAL(nsl_htonl(0), 0);
#else
    /* big endian host */
    CU_ASSERT_EQUAL(nsl_htonl(0x12345678), 0x12345678);
    CU_ASSERT_EQUAL(nsl_htonl(0), 0);
#endif
}

static void TestNtohl(void)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    /* little endian host */
    CU_ASSERT_EQUAL(nsl_ntohl(0x12345678), 0x78563412);
    CU_ASSERT_EQUAL(nsl_ntohl(0), 0);
#else
    /* big endian host */
    CU_ASSERT_EQUAL(nsl_ntohl(0x12345678), 0x12345678);
    CU_ASSERT_EQUAL(nsl_ntohl(0), 0);
#endif
}

static void TestHtons(void)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    /* little endian host */
    CU_ASSERT_EQUAL(nsl_htons(0x1234), 0x3412);
    CU_ASSERT_EQUAL(nsl_htons(0), 0);
#else
    /* big endian host */
    CU_ASSERT_EQUAL(nsl_htons(0x1234), 0x1234);
    CU_ASSERT_EQUAL(nsl_htons(0), 0);
#endif
}

static void TestNtohs(void)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    /* little endian host */
    CU_ASSERT_EQUAL(nsl_ntohs(0x1234), 0x3412);
    CU_ASSERT_EQUAL(nsl_ntohs(0), 0);
#else
    /* big endian host */
    CU_ASSERT_EQUAL(nsl_ntohs(0x1234), 0x1234);
    CU_ASSERT_EQUAL(nsl_ntohs(0), 0);
#endif
}

static void TestStrlen(void)
{
    CU_ASSERT_EQUAL(nsl_strlen(""), 0);
    CU_ASSERT_EQUAL(nsl_strlen("1"), 1);
    CU_ASSERT_EQUAL(nsl_strlen("1234567890"), 10);
    CU_ASSERT_EQUAL(nsl_strlen("1234\0 567890"), 4);
}

static void TestStrcpy(void)
{
    char dstBuf[BUF_SIZE];
    char srcBuf[BUF_SIZE];

    CU_ASSERT_EQUAL(nsl_strcpy(dstBuf, ""), dstBuf);
    CU_ASSERT_STRING_EQUAL(dstBuf, "");

    CU_ASSERT_EQUAL(nsl_strcpy(dstBuf, "0123456789"), dstBuf);
    CU_ASSERT_STRING_EQUAL(dstBuf, "0123456789");

    CU_ASSERT_EQUAL(nsl_strcpy(dstBuf, "abcd"), dstBuf);
    CU_ASSERT_STRING_EQUAL(dstBuf, "abcd");
    CU_ASSERT_EQUAL(nsl_strcpy(dstBuf, "x"), dstBuf);
    CU_ASSERT_STRING_EQUAL(dstBuf, "x");
    CU_ASSERT_STRING_EQUAL(dstBuf+2, "cd");

    CU_ASSERT_EQUAL(nsl_strcpy(srcBuf, "hello world!"), srcBuf);
    CU_ASSERT_EQUAL(nsl_strcpy(dstBuf, srcBuf), dstBuf);
    CU_ASSERT_STRING_EQUAL(dstBuf, "hello world!");
    CU_ASSERT_STRING_EQUAL(srcBuf, "hello world!");
}

static void TestStrncpy(void)
{
    char dstBuf[BUF_SIZE];
    const char* srcBuf = "0123456789ABCDEF";

    CU_ASSERT_STRING_EQUAL(nsl_strncpy(dstBuf, "", BUF_SIZE), "");
    CU_ASSERT_STRING_EQUAL(nsl_strncpy(dstBuf, "0123456789", BUF_SIZE),
            "0123456789");

    nsl_memset(dstBuf, ALL_ONE, BUF_SIZE);
    CU_ASSERT_STRING_NOT_EQUAL(nsl_strncpy(dstBuf, srcBuf, nsl_strlen(srcBuf)),
            srcBuf);
    CU_ASSERT_EQUAL(dstBuf[nsl_strlen(srcBuf)], ALL_ONE);

    CU_ASSERT_EQUAL(nsl_memcmp(dstBuf, srcBuf, nsl_strlen(srcBuf)), 0);
}

static void TestStrlcpy(void)
{
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE*2];

    size_t buf1size = sizeof(buf1);
    size_t ret = 0;

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_memset(buf2, ALL_ZERO, sizeof(buf2));
    nsl_strcpy(buf2,  "abcdef");
    ret = nsl_strlcpy(buf1, buf2, buf1size);
    CU_ASSERT_EQUAL(ret, nsl_strlen(buf2));
    CU_ASSERT_STRING_EQUAL(buf1, buf2);
    CU_ASSERT_EQUAL(nsl_memcmp(buf1, buf2, nsl_strlen(buf2)), 0);

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_strcpy(buf2,  "abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz");
    ret = nsl_strlcpy(buf1, buf2, buf1size);
    CU_ASSERT_EQUAL(ret, nsl_strlen(buf2)); //number of bytes needed to store this string
    CU_ASSERT_STRING_NOT_EQUAL(buf1, buf2);
    CU_ASSERT_NOT_EQUAL(nsl_memcmp(buf1, buf2, nsl_strlen(buf2)), 0);

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_memset(buf2, 'A', sizeof(buf2));
    buf2[sizeof(buf2) - 1] = '\0';
    ret = nsl_strlcpy(buf1, buf2, sizeof(buf1));
    CU_ASSERT_EQUAL(ret, nsl_strlen(buf2));
    CU_ASSERT_EQUAL(strlen(buf1), (sizeof(buf1) - 1));
    CU_ASSERT_EQUAL(nsl_memcmp(buf1, buf2, (sizeof(buf1) - 1)), 0);
}

static void TestStrdup(void)
{
    char *strdupResult = NULL;
    const char *srcStr = "1234";
    const char *emptyStr = "";

    strdupResult = nsl_strdup(srcStr);
    CU_ASSERT_NOT_EQUAL(strdupResult, NULL);
    CU_ASSERT_NOT_EQUAL(strdupResult, srcStr);
    CU_ASSERT_STRING_EQUAL(strdupResult, srcStr);
    nsl_free(strdupResult);

    strdupResult = nsl_strdup(emptyStr);
    CU_ASSERT_NOT_EQUAL(strdupResult, NULL);
    CU_ASSERT_NOT_EQUAL(strdupResult, emptyStr);
    CU_ASSERT_STRING_EQUAL(strdupResult, emptyStr);
    nsl_free(strdupResult);
}

static void TestStrcmp(void)
{
    CU_ASSERT(nsl_strcmp("a", "a") == 0);
    CU_ASSERT(nsl_strcmp("a", "b") < 0);
    CU_ASSERT(nsl_strcmp("b", "a") > 0);

    CU_ASSERT(nsl_strcmp("ABCD", "ABCD ") < 0);
    CU_ASSERT(nsl_strcmp(" ABCD", "ABCD") < 0);
    CU_ASSERT(nsl_strcmp("ABCD", "") > 0);
}

static void TestStrncmp(void)
{
    CU_ASSERT(nsl_strncmp("a", "a", 1) == 0);
    CU_ASSERT(nsl_strncmp("a", "b", 1) < 0);
    CU_ASSERT(nsl_strncmp("b", "a", 1) > 0);

    CU_ASSERT(nsl_strncmp("a", "a", 2) == 0);
    CU_ASSERT(nsl_strncmp("a", "b", 2) < 0);
    CU_ASSERT(nsl_strncmp("b", "a", 2) > 0);

    CU_ASSERT(nsl_strncmp("ABCD", "ABCD ", 5) < 0);
    CU_ASSERT(nsl_strncmp("ABCD", "ABCD ", 4) == 0);

    CU_ASSERT(nsl_strncmp(" ABCD", "ABCD", 5) < 0);
    CU_ASSERT(nsl_strncmp("ABCD", "", 4) > 0);
}

static void TestStricmp(void)
{
    CU_ASSERT(nsl_stricmp("a", "a") == 0);
    CU_ASSERT(nsl_stricmp("a", "A") == 0);

    CU_ASSERT(nsl_stricmp("a", "b") < 0);
    CU_ASSERT(nsl_stricmp("a", "B") < 0);
    CU_ASSERT(nsl_stricmp("A", "b") < 0);

    CU_ASSERT(nsl_stricmp("b", "a") > 0);
    CU_ASSERT(nsl_stricmp("b", "A") > 0);
    CU_ASSERT(nsl_stricmp("B", "a") > 0);

    CU_ASSERT(nsl_stricmp("ABCD", "ABCD ") < 0);
    CU_ASSERT(nsl_stricmp("ABCD", "abcd ") < 0);
    CU_ASSERT(nsl_stricmp("abcd", "ABCD ") < 0);
    CU_ASSERT(nsl_stricmp("abcd", "AbCD ") < 0);

    CU_ASSERT(nsl_stricmp(" ABCD", "ABCD") < 0);
    CU_ASSERT(nsl_stricmp(" ABCD", "abcd") < 0);
    CU_ASSERT(nsl_stricmp(" abcd", "ABCD") < 0);
    CU_ASSERT(nsl_stricmp(" abcd", "AbCD") < 0);

    CU_ASSERT(nsl_stricmp("ABCD", "") > 0);
    CU_ASSERT(nsl_stricmp("abcd", "") > 0);
}

static void TestStrnicmp(void)
{
    CU_ASSERT(nsl_strnicmp("a", "a", 1) == 0);
    CU_ASSERT(nsl_strnicmp("a", "A", 1) == 0);
    CU_ASSERT(nsl_strnicmp("A", "a", 1) == 0);

    CU_ASSERT(nsl_strnicmp("a", "b", 1) < 0);
    CU_ASSERT(nsl_strnicmp("a", "B", 1) < 0);
    CU_ASSERT(nsl_strnicmp("A", "b", 1) < 0);

    CU_ASSERT(nsl_strnicmp("b", "a", 1) > 0);
    CU_ASSERT(nsl_strnicmp("b", "A", 1) > 0);
    CU_ASSERT(nsl_strnicmp("B", "a", 1) > 0);

    CU_ASSERT(nsl_strnicmp("a", "a", 2) == 0);
    CU_ASSERT(nsl_strnicmp("a", "A", 2) == 0);
    CU_ASSERT(nsl_strnicmp("A", "a", 2) == 0);

    CU_ASSERT(nsl_strnicmp("a", "b", 2) < 0);
    CU_ASSERT(nsl_strnicmp("a", "B", 2) < 0);
    CU_ASSERT(nsl_strnicmp("A", "b", 2) < 0);

    CU_ASSERT(nsl_strnicmp("b", "a", 2) > 0);
    CU_ASSERT(nsl_strnicmp("B", "a", 2) > 0);
    CU_ASSERT(nsl_strnicmp("b", "A", 2) > 0);

    CU_ASSERT(nsl_strnicmp("ABCD", "ABCD ", 5) < 0);
    CU_ASSERT(nsl_strnicmp("ABCD", "abcd ", 5) < 0);
    CU_ASSERT(nsl_strnicmp("abcd", "ABCD ", 5) < 0);

    CU_ASSERT(nsl_strnicmp("ABCD", "ABCD ", 4) == 0);
    CU_ASSERT(nsl_strnicmp("ABCD", "abcd ", 4) == 0);
    CU_ASSERT(nsl_strnicmp("abcd", "ABCD ", 4) == 0);

    CU_ASSERT(nsl_strnicmp(" ABCD", "ABCD", 5) < 0);
    CU_ASSERT(nsl_strnicmp(" abcd", "ABCD", 5) < 0);
    CU_ASSERT(nsl_strnicmp(" ABCD", "abcd", 5) < 0);

    CU_ASSERT(nsl_strnicmp("ABCD", "", 4) > 0);
    CU_ASSERT(nsl_strnicmp("abcd", "", 4) > 0);
}

static void TestStristr(void)
{
    const char *haystack = "0123456789abcdef";

    CU_ASSERT(nsl_stristr(haystack, "Z") == NULL);
    CU_ASSERT(nsl_stristr(haystack, "a") != NULL);
    /* CU_ASSERT_EQUAL(nsl_stristr(haystack, "a") - haystack, 0x0a); */

    CU_ASSERT(nsl_stristr(haystack, "A") != NULL);
    /* CU_ASSERT_EQUAL(nsl_stristr(haystack, "A") - haystack, 0x0a); */

    CU_ASSERT(nsl_stristr(haystack, "012A") == NULL);
    CU_ASSERT(nsl_stristr(haystack, "012") == haystack);
    CU_ASSERT(nsl_stristr(haystack, "f0") == NULL);

    CU_ASSERT(nsl_stristr(haystack, haystack) == haystack);
}

static void TestStrchr(void)
{
    const char *haystack = "0123456789abcdef";

    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'G'), NULL);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '0'), haystack+0);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '1'), haystack+1);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '2'), haystack+2);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '3'), haystack+3);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '4'), haystack+4);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '5'), haystack+5);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '6'), haystack+6);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '7'), haystack+7);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '8'), haystack+8);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '9'), haystack+9);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'a'), haystack+0x0a);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'b'), haystack+0x0b);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'c'), haystack+0x0c);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'd'), haystack+0x0d);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'e'), haystack+0x0e);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, 'f'), haystack+0x0f);
    CU_ASSERT_EQUAL(nsl_strchr(haystack, '\0'), haystack+nsl_strlen(haystack));
}

static void TestStrempty(void)
{
    const char *nullPtr = NULL;
    CU_ASSERT_FALSE(nsl_strempty("a"));
    CU_ASSERT_FALSE(nsl_strempty(" "));
    CU_ASSERT(nsl_strempty(""));
    CU_ASSERT(nsl_strempty(nullPtr));
}

static void TestStrcat(void)
{
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_memset(buf2, ALL_ZERO, sizeof(buf2));

    CU_ASSERT_EQUAL(nsl_strcat(buf1, "abc"), buf1);
    CU_ASSERT_STRING_EQUAL(buf1, "abc");

    nsl_strcpy(buf1, "x");
    CU_ASSERT_EQUAL(nsl_strcat(buf1, "yz"), buf1);
    CU_ASSERT_STRING_EQUAL(buf1, "xyz");

    nsl_strcpy(buf1, "ab");
    nsl_strcpy(buf2, "cd");
    CU_ASSERT_EQUAL(nsl_strcat(buf1, buf2), buf1);
    CU_ASSERT_STRING_EQUAL(buf1, "abcd");
    CU_ASSERT_STRING_EQUAL(buf2, "cd");

    nsl_strcpy(buf1, "");
    CU_ASSERT_EQUAL(nsl_strcat(buf1, ""), buf1);
    CU_ASSERT_STRING_EQUAL(buf1, "");

    nsl_strcpy(buf1, "");
    CU_ASSERT_EQUAL(nsl_strcat(buf1, "ab"), buf1);
    CU_ASSERT_STRING_EQUAL(buf1, "ab");

    nsl_strcpy(buf1, "ab");
    CU_ASSERT_EQUAL(nsl_strcat(buf1, ""), buf1);
    CU_ASSERT_STRING_EQUAL(buf1, "ab");
}

static void TestStrlcat(void)
{
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE*2];

    size_t buf1size = sizeof(buf1);
    int buf1len = 0;
    size_t ret = 0;

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_memset(buf2, ALL_ZERO, sizeof(buf2));
    nsl_strcpy(buf2,  "abc");
    ret = nsl_strlcat(buf1, buf2, buf1size);
    CU_ASSERT_EQUAL(ret, nsl_strlen(buf2));
    CU_ASSERT_STRING_EQUAL(buf1, "abc");

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_strcpy(buf1,  "123");
    buf1len = nsl_strlen(buf1);
    ret = nsl_strlcat(buf1, "abc", buf1size);
    CU_ASSERT_EQUAL(ret, buf1len + nsl_strlen(buf2));
    CU_ASSERT_STRING_EQUAL(buf1, "123abc");

    nsl_memset(buf1, ALL_ZERO, sizeof(buf1));
    nsl_strcpy(buf2,  "abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz");
    ret = nsl_strlcat(buf1, buf2, buf1size);
    CU_ASSERT_EQUAL(ret, nsl_strlen(buf2)); //number of bytes needed to store this string

    nsl_memset(buf1, 'A', sizeof(buf1));
    buf1[sizeof(buf1) - 1] = '\0';

    nsl_memset(buf2, 'B', sizeof(buf2));
    buf2[sizeof(buf2) - 1] = '\0';
    ret = nsl_strlcat(buf1, buf2, sizeof(buf1));
    CU_ASSERT_EQUAL(ret, sizeof(buf1) + sizeof(buf2) -2);
    CU_ASSERT_EQUAL(strlen(buf1), (sizeof(buf1) - 1));
    nsl_memset(buf2, 'A', sizeof(buf2));
    buf2[sizeof(buf2) - 1] = '\0';
    CU_ASSERT_EQUAL(nsl_memcmp(buf1, buf2, sizeof(buf1) -1), 0);
}

static void TestSprintf(void)
{
    char buf[BUF_SIZE];

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%d", 0), 1);
    CU_ASSERT_STRING_EQUAL(buf, "0");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, " %d ", 0), 3);
    CU_ASSERT_STRING_EQUAL(buf, " 0 ");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%d", 123456789), 9);
    CU_ASSERT_STRING_EQUAL(buf, "123456789");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%d", -123456789), 10);
    CU_ASSERT_STRING_EQUAL(buf, "-123456789");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%x", 0x1234abcd), 8);
    CU_ASSERT_STRING_EQUAL(buf, "1234abcd");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%X", 0x1234abcd), 8);
    CU_ASSERT_STRING_EQUAL(buf, "1234ABCD");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%X", 0x1234abcd), 8);
    CU_ASSERT_STRING_EQUAL(buf, "1234ABCD");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%0.5f", 3.14159), 7);
    CU_ASSERT_STRING_EQUAL(buf, "3.14159");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%0.5f", -3.14159), 8);
    CU_ASSERT_STRING_EQUAL(buf, "-3.14159");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%s", ""), 0);
    CU_ASSERT_STRING_EQUAL(buf, "");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%s", "123"), 3);
    CU_ASSERT_STRING_EQUAL(buf, "123");

    nsl_memset(buf, ALL_ONE, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_sprintf(buf, "%s %s", "123", "456"), 7);
    CU_ASSERT_STRING_EQUAL(buf, "123 456");
}

static void TestSnprintf(void)
{
    char buf[BUF_SIZE];
	
    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%d", 0), 1);
    CU_ASSERT_STRING_EQUAL(buf, "0");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), " %d ", 0), 3);
    CU_ASSERT_STRING_EQUAL(buf, " 0 ");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%d", 123456789), 9);
    CU_ASSERT_STRING_EQUAL(buf, "123456789");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));		
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 3, "%d", 123456789), -1);
    CU_ASSERT_STRING_EQUAL(buf, "12");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%d", -123456789), 10);
    CU_ASSERT_STRING_EQUAL(buf, "-123456789");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 3, "%d", -123456789), -1);
    CU_ASSERT_STRING_EQUAL(buf, "-1");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%x", 0x1234abcd), 8);
    CU_ASSERT_STRING_EQUAL(buf, "1234abcd");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 5, "%x", 0x1234abcd), -1);
    CU_ASSERT_STRING_EQUAL(buf, "1234");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%X", 0x1234abcd), 8);
    CU_ASSERT_STRING_EQUAL(buf, "1234ABCD");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 7, "%X", 0x1234abcd), -1);
    CU_ASSERT_STRING_EQUAL(buf, "1234AB");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%X", 0x1234abcd), 8);
    CU_ASSERT_STRING_EQUAL(buf, "1234ABCD");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 7, "%X", 0x1234abcd), -1);
    CU_ASSERT_STRING_EQUAL(buf, "1234AB");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%0.5f", 3.14159), 7);
    CU_ASSERT_STRING_EQUAL(buf, "3.14159");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 6, "%0.5f", 3.14159), -1);
    CU_ASSERT_STRING_EQUAL(buf, "3.141");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%0.5f", -3.14159), 8);
    CU_ASSERT_STRING_EQUAL(buf, "-3.14159");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 2, "%0.5f", -3.14159), -1);
    CU_ASSERT_STRING_EQUAL(buf, "-");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 4, "%0.6e", 5e-20), -1);
    CU_ASSERT_STRING_EQUAL(buf, "5.0");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 1, "%0.6E", 5e-20), -1);
    CU_ASSERT_STRING_EQUAL(buf, "");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%s", ""), 0);
    CU_ASSERT_STRING_EQUAL(buf, "");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%s", "123"), 3);
    CU_ASSERT_STRING_EQUAL(buf, "123");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, sizeof(buf), "%s %s", "123", "456"), 7);
    CU_ASSERT_STRING_EQUAL(buf, "123 456");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 4, "%s %s", "123", "456"), -1);
    CU_ASSERT_STRING_EQUAL(buf, "123");

    nsl_memset(buf, ALL_ZERO, sizeof(buf));
    CU_ASSERT_EQUAL(nsl_snprintf(NULL, 0, "This is a test."), -1);
    CU_ASSERT_EQUAL(nsl_snprintf(buf, 0, "This is a test."), -1);
}

static void TestStrtod(void)
{
    char buf[BUF_SIZE];
    char *endPtr = NULL;

    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("12.3456", NULL), 12.3456, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("-12.3456", NULL), -12.3456, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("12.3456e19", NULL), 12.3456e19, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("-.1e+9", NULL), -.1e+9, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("1e20", NULL), 1e20, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod(".125", NULL), .125, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("0e-19", NULL), 0, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("4\0ab", NULL), 4, PRECISION);
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod("5.9e-76", NULL), 5.9e-76, PRECISION);

    nsl_strcpy(buf, "12.3456 abc");
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod(buf, &endPtr), 12.3456, PRECISION);
    CU_ASSERT_STRING_EQUAL(endPtr, " abc");

    nsl_strcpy(buf, "not a  number");
    CU_ASSERT_DOUBLE_EQUAL(nsl_strtod(buf, &endPtr), 0, PRECISION);
    CU_ASSERT_EQUAL(buf, endPtr);
}

static void TestStrtoul(void)
{
    char buf[BUF_SIZE];
    char *endPtr = NULL;

    CU_ASSERT_EQUAL(nsl_strtoul("0", NULL, 0), 0);
    CU_ASSERT_EQUAL(nsl_strtoul("+0", NULL, 0), 0);
    CU_ASSERT_EQUAL(nsl_strtoul("-0", NULL, 0), 0);
    CU_ASSERT_EQUAL(nsl_strtoul("123", NULL, 0), 123);
    CU_ASSERT_EQUAL(nsl_strtoul("123 ", NULL, 0), 123);
    CU_ASSERT_EQUAL(nsl_strtoul(" 123", NULL, 0), 123);
    CU_ASSERT_EQUAL(nsl_strtoul(" 123 ", NULL, 0), 123);
    CU_ASSERT_EQUAL(nsl_strtoul("+123 ", NULL, 0), 123);
    CU_ASSERT_EQUAL(nsl_strtoul("-123", NULL, 0), -123);
    CU_ASSERT_EQUAL(nsl_strtoul("       -123          ", NULL, 0), -123);

    CU_ASSERT_EQUAL(nsl_strtoul("2147483647", NULL, 0), 2147483647);
    CU_ASSERT_EQUAL(nsl_strtoul("-2147483647", NULL, 0), -2147483647);

    CU_ASSERT_EQUAL(nsl_strtoul("123", NULL, 10), 123);
    CU_ASSERT_EQUAL(nsl_strtoul("123", NULL, 8), 0123);
    CU_ASSERT_EQUAL(nsl_strtoul("123", NULL, 4), (1*4*4 + 2*4 + 3));

    nsl_strcpy(buf, "1234 abcd");
    CU_ASSERT_EQUAL(nsl_strtoul(buf, &endPtr, 10), 1234);
    CU_ASSERT_EQUAL(endPtr, buf+4);

    nsl_strcpy(buf, "1234abcd");
    CU_ASSERT_EQUAL(nsl_strtoul(buf, &endPtr, 10), 1234);
    CU_ASSERT_EQUAL(endPtr, buf+4);

    nsl_strcpy(buf, "1234abcd");
    CU_ASSERT_EQUAL(nsl_strtoul(buf, &endPtr, 16), 0x1234abcd);
    CU_ASSERT_EQUAL(endPtr, buf+8);

    nsl_strcpy(buf, "0x1234abcd");
    CU_ASSERT_EQUAL(nsl_strtoul(buf, &endPtr, 0), 0x1234abcd);
    CU_ASSERT_EQUAL(endPtr, buf+nsl_strlen(buf));
}

static void TestUint64tostr(void)
{
    char buf[BUF_SIZE];

    CU_ASSERT(nsl_uint64tostr(9493790319ULL, buf, sizeof(buf)) > 0);
    CU_ASSERT_STRING_EQUAL(buf, "9493790319");

    CU_ASSERT(nsl_uint64tostr(0ULL, buf, sizeof(buf)) > 0);
    CU_ASSERT_STRING_EQUAL(buf, "0");
}

static void TestInt64tostr(void)
{
    char buf[BUF_SIZE];

    CU_ASSERT(nsl_int64tostr(9493790319LL, buf, sizeof(buf)) > 0);
    CU_ASSERT_STRING_EQUAL(buf, "9493790319");

    CU_ASSERT(nsl_int64tostr(-9493790319LL, buf, sizeof(buf)) > 0);
    CU_ASSERT_STRING_EQUAL(buf, "-9493790319");

    CU_ASSERT(nsl_int64tostr(0LL, buf, sizeof(buf)) > 0);
    CU_ASSERT_STRING_EQUAL(buf, "0");

    CU_ASSERT(nsl_int64tostr(-1LL, buf, sizeof(buf)) > 0);
    CU_ASSERT_STRING_EQUAL(buf, "-1");
}

/*! Add all your test functions here

@return None
*/
void TestStdlib_AddAllTests( CU_pSuite pTestSuite )
{
    // ! Add all your function names here !

    CU_add_test(pTestSuite, "TestSimulatorRunning", &TestSimulatorRunning);
    CU_add_test(pTestSuite, "TestTypes", &TestTypes);

    CU_add_test(pTestSuite, "TestMemset", &TestMemset);
    CU_add_test(pTestSuite, "TestMemcpy", &TestMemcpy);
    CU_add_test(pTestSuite, "TestMemcmp", &TestMemcmp);
    CU_add_test(pTestSuite, "TestMemmove", &TestMemmove);

    CU_add_test(pTestSuite, "TestMalloc", &TestMalloc);
    CU_add_test(pTestSuite, "TestRealloc", &TestRealloc);

    CU_add_test(pTestSuite, "TestAtoi", &TestAtoi);
    CU_add_test(pTestSuite, "TestAtof", &TestAtof);
    CU_add_test(pTestSuite, "TestDoubleToStr", &TestDoubleToStr);

    CU_add_test(pTestSuite, "TestHtonl", &TestHtonl);
    CU_add_test(pTestSuite, "TestNtohl", &TestNtohl);

    CU_add_test(pTestSuite, "TestHtons", &TestHtons);
    CU_add_test(pTestSuite, "TestNtohs", &TestNtohs);

    CU_add_test(pTestSuite, "TestStrlen", &TestStrlen);
    CU_add_test(pTestSuite, "TestStrcpy", &TestStrcpy);
    CU_add_test(pTestSuite, "TestStrncpy", &TestStrncpy);
    CU_add_test(pTestSuite, "TestStrlcpy", &TestStrlcpy);
    CU_add_test(pTestSuite, "TestStrdup", &TestStrdup);
    CU_add_test(pTestSuite, "TestStrcmp", &TestStrcmp);
    CU_add_test(pTestSuite, "TestStrncmp", &TestStrncmp);
    CU_add_test(pTestSuite, "TestStricmp", &TestStricmp);
    CU_add_test(pTestSuite, "TestStrnicmp", &TestStrnicmp);
    CU_add_test(pTestSuite, "TestStrchr", &TestStrchr);
    CU_add_test(pTestSuite, "TestStristr", &TestStristr);
    CU_add_test(pTestSuite, "TestStrempty", &TestStrempty);
    CU_add_test(pTestSuite, "TestStrcat", &TestStrcat);
    CU_add_test(pTestSuite, "TestStrlcat", &TestStrlcat);
    CU_add_test(pTestSuite, "TestSprintf", &TestSprintf);
    CU_add_test(pTestSuite, "TestSnprintf", &TestSnprintf);
    CU_add_test(pTestSuite, "TestStrtod", &TestStrtod);
    CU_add_test(pTestSuite, "TestStrtoul", &TestStrtoul);

    CU_add_test(pTestSuite, "TestUint64tostr", &TestUint64tostr);
    CU_add_test(pTestSuite, "TestInt64tostr", &TestInt64tostr);
};


/*! Add common initialization code here.

@return 0

@see TestFile_SuiteCleanup
*/
int TestStdlib_SuiteSetup()
{
    TEST_LOGGING("TestStdlib_SuiteSetup: %s", "teststdlib.c");

    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestFile_SuiteSetup
*/
int TestStdlib_SuiteCleanup()
{
    return 0;
}

/*! @} */
