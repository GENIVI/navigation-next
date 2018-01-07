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

    @defgroup TestUtil_c Unit Tests for CSL Logging

    Unit Tests for CSL Util

    This file contains all unit tests for the Core logging component
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

#include "testutil.h"
#include "main.h"
#include "platformutil.h"
#ifdef WINCE
#include "winbase.h"
#endif
#include "logimpl.h"
#include "fileutil.h"

#include "bq.h"
#include "dynbuf.h"
#include "csdict.h"
#include "Util.h"
#include "fsalloc.h"
#include "ht.h"
#include "vec.h"
#include "cslquadkey.h"
#include "palmath.h"
#include "cslaes.h"


#define TEST_DIR_NAME   "FileUtilTest"
#define TEST_FILENAME   "FileUtil.tst"
#define TEST_FILENAME2  "FileUtil2.tst"
#define TEST_FILENAME3  "FileUtil3.tst"

static const char testutil_data[] = {

	82, 220, 158, 174, 49, 227, 106, 101, 96, 221, 94, 65, 7, 90, 
	90, 167, 67, 113, 30, 250, 0, 0, 0, 2, 116, 101, 115, 116, 99, 
	97, 112, 49, 0, 121, 101, 115, 0, 116, 101, 115, 116, 99, 97, 
	112, 50, 0, 110, 111, 0, 0, 0, 0, 13, 116, 101, 115, 116, 45, 
	101, 108, 101, 109, 101, 110, 116, 0, 0, 0, 0, 18, 101, 120, 
	97, 109, 112, 108, 101, 45, 105, 116, 101, 109, 0, 110, 97, 
	109, 101, 0, 0, 0, 0, 24, 101, 120, 97, 109, 112, 108, 101, 45,
	105, 116, 101, 109, 0, 110, 97, 109, 101, 0, 118, 97, 108, 117, 
	101, 0, 0, 0, 0, 18, 100, 101, 116, 97, 105, 108, 115, 0, 97, 0, 
	98, 0, 99, 0, 100, 0, 101, 0, 0, 0, 0, 16, 100, 101, 116, 97, 105, 
	108, 115, 0, 97, 0, 98, 0, 99, 0, 100, 0
};

// Structure used for various test
struct UtilTestData
{
    nb_unixTime time;
    uint32 x;
    uint32 y;
    uint32 z;
    char   name[16];
};


// Local Functions ...............................................................................

// Helper functions 
static void _base64_encode_triple(unsigned char triple[3], char result[4]);
static int TestUtilEncodeBase64(unsigned char *source, size_t sourcelen, char *target, size_t targetlen);
static void TestQuadkey(uint32 x, uint32 y, uint8 zoom, const char* expectedResult);

// Test functions
static void TestUtilFileUtil(void);
static void TestUtilBQ(void);
static void TestUtilCSDict(void);
static void TestUtilDynBuf(void);
static void TestUtilFileUtil(void);
static void TestUtilFS(void);
static void TestUtilHashTable(void);
static void TestUtilResource(void);
static void TestUtilMisc(void);
static void TestUtilXYZToQuadkey(void);
static void TestUtilAES(void);

// Helper functions for vector tests
static void FillVector(CSL_Vector* vector, const int* testArray, int size);
static void TestDifference(CSL_Vector* input1,
                           CSL_Vector* input2,
                           CSL_Vector* output,
                           const int* testArray1,
                           const int* testArray2,
                           const int* expectedArray,
                           int size1,
                           int size2,
                           int expectedSize);

static void RemoveDuplicate(int* inputArray, int inputSize, int* outputArray, int outputSize);
static int VectorCompareFunction(const void* left, const void* right);
static int VectorForEachCallback(void* userData, void* itemPointer);


// Vector test functions
static void TestUtilVector(void);
static void TestUtilVectorDifference1(void);
static void TestUtilVectorDifference2(void);
static void TestUtilVectorRemove(void);
static void TestUtilVectorRemoveDuplicate(void);
static void TestUtilVectorForEach(void);


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestUtil_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestUtilFileUtil",             &TestUtilFileUtil);
    CU_add_test(pTestSuite, "TestUtilBQ",                   &TestUtilBQ);
    CU_add_test(pTestSuite, "TestUtilCSDict",               &TestUtilCSDict);
    CU_add_test(pTestSuite, "TestUtilDynBuf",               &TestUtilDynBuf);
    CU_add_test(pTestSuite, "TestUtilFS",                   &TestUtilFS);
    CU_add_test(pTestSuite, "TestUtilHashTable",            &TestUtilHashTable);
    CU_add_test(pTestSuite, "TestUtilResource",             &TestUtilResource);
    CU_add_test(pTestSuite, "TestUtilMisc",                 &TestUtilMisc);
    CU_add_test(pTestSuite, "TestUtilXYZToQuadkey",         &TestUtilXYZToQuadkey);
    CU_add_test(pTestSuite, "TestUtilAES",                  &TestUtilAES);

    CU_add_test(pTestSuite, "TestUtilVector",               &TestUtilVector);
    CU_add_test(pTestSuite, "TestUtilVectorDifference1",    &TestUtilVectorDifference1);
    CU_add_test(pTestSuite, "TestUtilVectorDifference2",    &TestUtilVectorDifference2);
    CU_add_test(pTestSuite, "TestUtilVectorRemove",         &TestUtilVectorRemove);
    CU_add_test(pTestSuite, "TestUtilVectorRemoveDuplicate",&TestUtilVectorRemoveDuplicate);
    CU_add_test(pTestSuite, "TestUtilVectorForEach",        &TestUtilVectorForEach);
};


/*! Add common initialization code here.

    @return 0

    @see TestNetwork_SuiteCleanup
*/
int
TestUtil_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

    @return 0

    @see TestUtil_SuiteSetup
*/
int 
TestUtil_SuiteCleanup()
{
    return 0;
}


// Local Functions ...............................................................................

static void 
_base64_encode_triple(unsigned char triple[3], char result[4])
 {
    /*
    ** Translation Table as described in RFC1113
    */
    static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int tripleValue, i;

    tripleValue = triple[0];
    tripleValue *= 256;
    tripleValue += triple[1];
    tripleValue *= 256;
    tripleValue += triple[2];

    for (i=0; i<4; i++)
    {
	result[3-i] = cb64[tripleValue%64];
	tripleValue /= 64;
    }
} 

// Helper function for test TestUtilMisc()
static int 
TestUtilEncodeBase64(unsigned char *source, size_t sourcelen, char *target, size_t targetlen)
 { 
    /* check if the result will fit in the target buffer */
    if ((sourcelen+2)/3*4 > targetlen-1)
    {
	    return 0;
    }

    /* encode all full triples */
    while (sourcelen >= 3)
    {
        _base64_encode_triple(source, target);
        sourcelen -= 3;
        source += 3;
        target += 4;
    }

    /* encode the last one or two characters */
    if (sourcelen > 0)
    {
	    unsigned char temp[3];
	    memset(temp, 0, sizeof(temp));
	    memcpy(temp, source, sourcelen);
	    _base64_encode_triple(temp, target);
	    target[3] = '=';

	    if (sourcelen == 1)
        {
	        target[2] = '=';
        }

	    target += 4;
    }

    /* terminate the string */
    target[0] = 0;

    return 1;
}

/*! Helper function to test quadkey conversions.

    @return None, Assert on failure.
*/
static void 
TestQuadkey(uint32 x, uint32 y, uint8 zoom, const char* expectedResult)
{
    char quadkey[CSL_MAXIMUM_QUADKEY_LENGTH] = {0};

    NB_Error result = CSL_QuadkeyConvertFromXYZ(x, y, zoom, quadkey);

    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(quadkey, expectedResult);
}


// Test functions ....................................................................................................

static void 
TestUtilFileUtil(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    // Ensure that our test files are deleted
    if(fileexists(pal, TEST_DIR_NAME))
    {
        CU_ASSERT(removefile(pal, TEST_DIR_NAME) == TRUE);
    }

    // First lets create a directory
   CU_ASSERT_EQUAL(createdirectory(pal, TEST_DIR_NAME), TRUE);

     // Create a test file
    CU_ASSERT_EQUAL(savefile(pal, PFM_Create, TEST_FILENAME, (uint8 *)testutil_data, sizeof(testutil_data)), 1);

    if(fileexists(pal, TEST_FILENAME))
    {
        // Ensure that it is the correct size
        CU_ASSERT_EQUAL(filesize(pal, TEST_FILENAME), sizeof(testutil_data));

        // Append more to the end of the file
        CU_ASSERT_EQUAL(appendfile(pal, TEST_FILENAME, (uint8 *)testutil_data, sizeof(testutil_data)), 1);

        // Ensure that it is the correct size
        CU_ASSERT_EQUAL(filesize(pal, TEST_FILENAME), sizeof(testutil_data) * 2);

        // rename the file
        CU_ASSERT_EQUAL(renamefile(pal, TEST_FILENAME, TEST_FILENAME2), 1);

        if(fileexists(pal, TEST_FILENAME2))
        {
            // Remove the testfile
            CU_ASSERT_EQUAL(removefile(pal, TEST_FILENAME2), 1);
        }
    }

    // test appendfile() in case of absent file
    if(!fileexists(pal, TEST_FILENAME3))
    {
        CU_ASSERT(removefile(pal, TEST_FILENAME3) == TRUE);
    }
    CU_ASSERT_EQUAL(appendfile(pal, TEST_FILENAME3, (uint8 *)testutil_data, sizeof(testutil_data)), 1);
    CU_ASSERT(fileexists(pal, TEST_FILENAME3) == TRUE);
    CU_ASSERT(removefile(pal, TEST_FILENAME3) == TRUE);

    // Remove our test directory if it was created
    if(fileexists(pal, TEST_DIR_NAME))
    {
        CU_ASSERT_EQUAL(removedir(pal, TEST_DIR_NAME), 0);
    }

    PAL_Destroy(pal);
}

static void 
TestUtilBQ(void)
{
    int ret = 0;
    struct bq bq = {0};
    PAL_Instance *pal = PAL_CreateInstance();
    int i = 0;

    // Initialize the BQ structure
    csl_bqinit(&bq);

    // Do a series of csl_bqpreps to allow alloc and resize functions to be tested
    for(i = 1; i <= 32; i++)
    {
        ret = csl_bqprep(&bq, 1024*i);
        CU_ASSERT_EQUAL(ret, 1);
        CU_ASSERT_PTR_NOT_NULL(bq.p);
        CU_ASSERT_PTR_NOT_NULL(bq.r);
        CU_ASSERT_PTR_NOT_NULL(bq.w);
        CU_ASSERT_PTR_NOT_NULL(bq.x);
    }

    // Shrink it
    csl_bqshrink(&bq);

    CU_ASSERT_PTR_NULL(bq.p);
    CU_ASSERT_PTR_NULL(bq.r);
    CU_ASSERT_PTR_NULL(bq.w);
    CU_ASSERT_PTR_NULL(bq.x);

    // Free up the memory
    csl_bqfree(&bq);

    PAL_Destroy(pal);
}

static void 
TestUtilCSDictDelFunc(void* userdata, const char* key, const char *value, size_t vlen)
{
    int *iData = (int *)userdata;

    CU_ASSERT_PTR_NOT_NULL(iData);

    // Ensure the user data is correct
    CU_ASSERT_EQUAL(*iData, 1024);
}

static void 
TestUtilCSDict(void)
{
    struct UtilTestData csTest;
    struct UtilTestData *pcsTest;
    struct CSL_Dictionary * pDict;
    int ret = 0, i = 0;
    size_t size = 0;
    int userData = 1024;
    char *pKey, *pVal;
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    pDict = CSL_DictionaryAlloc(sizeof(struct UtilTestData));
    CU_ASSERT(pDict != NULL);

    for(i = 0; i < 10; i++)
    {
        // Set up test keys
        csTest.time = unixtime();
        csTest.x = 1;
        csTest.y = 2;
        csTest.z = 3;

        nsl_sprintf(csTest.name, "Test Key %d", i+1);
        ret = CSL_DictionarySet(pDict, csTest.name, (const char *)&csTest, sizeof(csTest));
        CU_ASSERT(ret != 0);

        // Check the test keys to ensure that we got the correct values
        pcsTest = (struct UtilTestData *)CSL_DictionaryGet(pDict, csTest.name, &size);
        CU_ASSERT(pcsTest != NULL);

        // Check the expected values
        CU_ASSERT_EQUAL(pcsTest->x, csTest.x);
        CU_ASSERT_EQUAL(pcsTest->y, csTest.y);
        CU_ASSERT_EQUAL(pcsTest->z, csTest.z);
        CU_ASSERT_EQUAL(pcsTest->time, csTest.time);
        CU_ASSERT_STRING_EQUAL(pcsTest->name, csTest.name);

        // Check the len function
        CU_ASSERT(CSL_DictionaryLength(pDict) > 0);
    }

    // Try accessing the dictionary using next function
    i = 0;
    do 
    {
        ret = CSL_DictionaryNext(pDict, &i, (const char **)&pKey, &pVal, &size);
        if(ret == 1)
        {
            pcsTest = (struct UtilTestData *)pVal;
            CU_ASSERT_PTR_NOT_NULL(pKey);
            CU_ASSERT_PTR_NOT_NULL(pcsTest);
            CU_ASSERT_EQUAL(size, sizeof(csTest));
            CU_ASSERT_EQUAL(pcsTest->x, 1);
            CU_ASSERT_EQUAL(pcsTest->y, 2);
            CU_ASSERT_EQUAL(pcsTest->z, 3);
            CU_ASSERT_STRING_EQUAL(pcsTest->name, pKey);
        }

    }while(ret);


    CSL_DictionaryDealloc(pDict);

    pDict = CSL_DictionaryAllocEx(sizeof(struct UtilTestData), TestUtilCSDictDelFunc, &userData);
    CU_ASSERT(pDict != NULL);

    // Set up test keys
    csTest.time = unixtime();
    csTest.x = 1;
    csTest.y = 2;
    csTest.z = 3;

    nsl_strcpy(csTest.name, "Test Key EX");
    ret = CSL_DictionarySet(pDict, csTest.name, (const char *)&csTest, sizeof(csTest));
    CU_ASSERT(ret != 0);

    // Check the test keys to ensure that we got the correct values
    pcsTest = (struct UtilTestData *)CSL_DictionaryGet(pDict, csTest.name, &size);
    CU_ASSERT(pcsTest != NULL);

    // Check the expected values
    CU_ASSERT_EQUAL(pcsTest->x, csTest.x);
    CU_ASSERT_EQUAL(pcsTest->y, csTest.y);
    CU_ASSERT_EQUAL(pcsTest->z, csTest.z);
    CU_ASSERT_EQUAL(pcsTest->time, csTest.time);
    CU_ASSERT_STRING_EQUAL(pcsTest->name, csTest.name);

    // Check the len function
    CU_ASSERT(CSL_DictionaryLength(pDict) > 0);

    // Delete our key and ensure that it has been removed
    CSL_DictionaryDelete(pDict, csTest.name);
    pcsTest = (struct UtilTestData *)CSL_DictionaryGet(pDict, csTest.name, &size);
    CU_ASSERT_PTR_NULL(pcsTest);

    CSL_DictionaryDealloc(pDict);   

    PAL_Destroy(pal);
}

static void 
TestUtilDynBuf(void)
{
    NB_Error err;
    struct dynbuf dbuf;
    int i = 0;
    int len = 0;
    int nlen = 0;
    char ch;
    byte *pch;
    double dVal;
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);
    
    // Create a new buffer
    err = dbufnew(&dbuf, sizeof(testutil_data)-10);
    CU_ASSERT_EQUAL(err, 0);

    // Concatenate the data to the dbuf
    dbufcat(&dbuf, (const byte *)testutil_data, sizeof(testutil_data));

    // Check the length
    len = dbuflen(&dbuf);
    CU_ASSERT(len >= sizeof(testutil_data));

    // Check the data to ensure that it is correct using the dbufgetc
    for(i = 0; i < sizeof(testutil_data); i++)
    {
        ch = dbufgetc(&dbuf, i);
        CU_ASSERT_EQUAL(ch, testutil_data[i]);
    }

    // Check the dbufget function
    pch = (byte *)dbufget(&dbuf);
    for(i = 0; i < sizeof(testutil_data); i++)
    {
        ch = (char)*pch++;
        CU_ASSERT_EQUAL(ch, testutil_data[i]);
    }

    // Append a float and check the value
    dbuffmtd(&dbuf, 21936517.523);
    dbufnuls(&dbuf);
    len = dbuflen(&dbuf);
    nlen = (int)(sizeof(testutil_data) + sizeof(double));
    CU_ASSERT(len >= nlen);
    pch = (byte *)dbufget(&dbuf);
    pch += sizeof(testutil_data);
    dVal = nsl_atof((const char *)pch);
    CU_ASSERT_DOUBLE_EQUAL(dVal, 21936517.523, 0.001);

    // Check the dbufcats
    len = dbuflen(&dbuf);
    dbufcats(&dbuf, "Additional string");
    nlen = dbuflen(&dbuf);
    CU_ASSERT(nlen == (len + 17));

    dbufdel(&dbuf);

    PAL_Destroy(pal);
}

static void 
TestUtilFS(void)
{
    struct fsaheap heap;
    void *pBuf = NULL;
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    // Initialize our heap
    CU_ASSERT_EQUAL(fsaheap_init(&heap, 1024, 1024*32), 1);

    pBuf = fsaheap_alloc(&heap);
    CU_ASSERT(pBuf != NULL);

    // Free our buffer
    fsaheap_free(&heap, pBuf);

    // Free our heap
    fsaheap_destroy(&heap);

    PAL_Destroy(pal);
}

static int
TestUtilCompare(void *a, void *b)
{
    if(a == b)
        return(1);

	return (0);
}

static long
TestUtilByteHash(const char *s, size_t size)
{
	nb_size len;
	long x;

	len = size;
	x = *s << 7;
	while (--len >= 0)
		x = (1000003 * x) ^ *s++;
	x ^= size;
	return (x);
}

static void 
TestUtilHashTable(void)
{
    struct ht* pht = NULL;
    struct UtilTestData test;
    int len = 0;
	long hash;
	void **slot;
    void **buf;
    char *pName = "Test Name";
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    pht = htalloc(1024);
    CU_ASSERT(pht != NULL);


    test.x = 4;
    test.y = 5;
    test.z = 6;
    test.time = unixtime();
    nsl_strcpy(test.name, pName);
    hash = TestUtilByteHash(test.name, nsl_strlen(test.name));

    // Look for the entry as it should not exist
	slot = htlook(pht, hash, TestUtilCompare, &test);
    CU_ASSERT(*slot == NULL);

    // Insert a new entry
	buf = htins(pht, slot, hash, &test);
    CU_ASSERT(*buf != NULL);

    // Check the length
    len = htlen(pht);
    CU_ASSERT(len == 1);

    // Search for the entry
	slot = htlook(pht, hash, TestUtilCompare, &test);
    CU_ASSERT_PTR_NOT_NULL(*slot);

    htdealloc(pht);

    PAL_Destroy(pal);
}

static void 
TestUtilResource(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    // TODO: Test TBA

    PAL_Destroy(pal);
}

static void 
TestUtilMisc(void)
{
    int result;
    int ret, i;
    uint32 ui32;
    char ch;
    char data[64]; 
    char buffer[64];
    nb_unixTime palTime, gpsTime, convTime;
    unsigned char hmacExpected[] = {0x6d, 0x50, 0xa2, 0x47, 0xd5, 0x4c, 0x02, 0x62, 0x7d, 0x9d, 
                                    0x02, 0xb8, 0xb2, 0xb3, 0xc9, 0x02, 0x57, 0x40, 0x6a, 0x54};
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    result = str_cmpx("Match this", "Match this");
    CU_ASSERT_EQUAL(result, 0);

    result = str_cmpx("Match that", "Match this");
    CU_ASSERT_EQUAL(result, -1);

    result = str_cmpxi("Match THIS", "match this");
    CU_ASSERT_EQUAL(result, 0);

    result = str_cmpxi("Match that", "match this");
    CU_ASSERT_EQUAL(result, -8);

    nsl_strcpy(data, "ABCDEFG");
    nsl_memset(buffer, 0, 64);
    str_rev_into(buffer, data, strlen(data));
    CU_ASSERT_STRING_EQUAL(buffer, "GFEDCBA");

    ui32 = crc32(0, (const byte *)testutil_data, sizeof(testutil_data));
    CU_ASSERT_EQUAL(ui32, 2193651707UL);

    ch = nimtoupper('l');
    CU_ASSERT_EQUAL(ch, 'L');

    ch = nimtolower('Z');
    CU_ASSERT_EQUAL(ch, 'z');

    CU_ASSERT_TRUE(nimisupper('B'));
    CU_ASSERT_TRUE(nimislower('c'));

    CU_ASSERT_FALSE(nimisupper('b'));
    CU_ASSERT_FALSE(nimisupper('x'));

#if 0   // These functions are not supported in the coreservices util
    CU_ASSERT_TRUE(alldigits("1234567890"));
    CU_ASSERT_FALSE(alldigits("1234567A90"));

    CU_ASSERT_TRUE(allupper("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    CU_ASSERT_FALSE(allupper("ABCDEFGHIJKLMNOPqRSTUVWXYZ"));
    CU_ASSERT_TRUE(alluppern("ABCDE", 5));
    CU_ASSERT_FALSE(alluppern("ABCdE", 5));
#endif

    ui32 = tolangid("en-US");
    CU_ASSERT_EQUAL(ui32, 0x20206e65);

    // Check hexlify/unhexlify
    nsl_memset(data, 0, 64);
    nsl_memset(buffer, 0, 64);
    nsl_strcpy(data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    hexlify(26, data, buffer);
    CU_ASSERT_EQUAL(nsl_stricmp(buffer, "4142434445464748494a4b4c4d4e4f505152535455565758595a"), 0);
    nsl_memset(data, 0, 64);
    unhexlify(26, buffer, data);
    CU_ASSERT_EQUAL(nsl_strcmp(data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 0);

    nsl_memset(buffer, 0, 64);
    nsl_memset(data, 0, 64);
    nsl_strcpy(buffer, "This is a test of base 64");
    TestUtilEncodeBase64((unsigned char *)buffer, nsl_strlen(buffer), data, 64);

    nsl_memset(buffer, 0, 64);
    ret = decode_base64(data, buffer);
    CU_ASSERT_NOT_EQUAL(ret, -1);
    CU_ASSERT_STRING_EQUAL(buffer, "This is a test of base 64");

    // Test the hmac function
    nsl_memset(buffer, 0, 64);
    nsl_memset(data, 0, 64);
    nsl_strcpy(buffer, "TestOfHMAC");
    hmac((const byte *)buffer, nsl_strlen(buffer), (const byte *)"MyKey", 5, (byte *)data);
    for(i = 0; i < 20; i++)
    {
        CU_ASSERT_EQUAL((unsigned char)data[i], hmacExpected[i]);
    }

    // Check time functions
    palTime = unixtime();
    CU_ASSERT(palTime);
    gpsTime = gpstime();
    CU_ASSERT(gpsTime);
    convTime = gps2unixtime(gpsTime);
    CU_ASSERT(convTime == palTime);

    PAL_Destroy(pal);
}

/*! Test XYZ to Quadkey conversion.

    @return None. Calls CU-Assert on error.
*/
static void 
TestUtilXYZToQuadkey(void)
{
    char quadkey[CSL_MAXIMUM_QUADKEY_LENGTH] = {0};

    // Zoom level 1
    TestQuadkey(0, 0, 1, "0");
    TestQuadkey(1, 0, 1, "1");
    TestQuadkey(0, 1, 1, "2");
    TestQuadkey(1, 1, 1, "3");

    // Zoom level 2
    TestQuadkey(0, 0, 2, "00");
    TestQuadkey(1, 1, 2, "03");
    TestQuadkey(2, 0, 2, "10");
    TestQuadkey(3, 3, 2, "33");

    // Zoom level 3
    TestQuadkey(0, 0, 3, "000");
    TestQuadkey(1, 1, 3, "003");
    TestQuadkey(2, 2, 3, "030");
    TestQuadkey(3, 3, 3, "033");
    TestQuadkey(7, 7, 3, "333");

    TestQuadkey(5, 1, 3, "103");
    TestQuadkey(1, 6, 3, "221");
    TestQuadkey(4, 3, 3, "122");

    // Random sampling of quadkeys
    TestQuadkey(35, 64, 7, "2100011");
    TestQuadkey(284, 516, 10, "2100011300");
    TestQuadkey(145785, 264363, 19, "2100011300121313023");
    TestQuadkey(9759, 5468, 14, "12031202033311");
    TestQuadkey(8, 5, 4, "1202");

    // Test invalid zoom
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(0, 0, 0, quadkey), NE_INVAL);
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(0, 0, 33, quadkey), NE_INVAL);

    // Test some invalid parameters
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(2, 0, 1, quadkey), NE_INVAL);
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(0, 2, 1, quadkey), NE_INVAL);
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(4, 4, 2, quadkey), NE_INVAL);
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(8, 8, 3, quadkey), NE_INVAL);

    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(1024, 0, 10, quadkey), NE_INVAL);
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(1023, 1023, 10, quadkey), NE_OK);

    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(8388608, 0, 23, quadkey), NE_INVAL);
    CU_ASSERT_EQUAL(CSL_QuadkeyConvertFromXYZ(8388607, 8388607, 23, quadkey), NE_OK);
}

/*! Test AES encypting/decrypting.

    @return None. Calls CU-Assert on error.
*/
static void
TestUtilAES(void)
{
    NB_Error result = NE_OK;
    const char *invalid_string = "";
    const char *basestring11 = "abababababa";
    const char *basestring16 = "abababababababab";
    const char *basestring24 = "abababababababababababab";
    const char *basestring32 = "abababababababababababababababab";
    const char *basestring35 = "abababababababababababababababababa";
    const char *basestring48 = "abababababababababababababababababababababababab";
    const char *invalid_key = "1234567";
    const char *key16_1 = "1234567812345678";  //128 bits
    const char *key16_2 = "1111111111111111";
    const char *key24_1 = "123456781234567812345678";  //192 bits
    const char *key24_2 = "111111111111111111111111";
    const char *key32_1 = "12345678123456781234567812345678";  //256 bits
    const char *key32_2 = "11111111111111111111111111111111";
    char encrypted[64] = {0};
    char decrypted[64] = {0};
    int size = 0;

/*---Check for invalid input parametres:---*/
    //Invalid string with valid key
    size = nsl_strlen(invalid_string);
    result = CSL_AesEncrypt(invalid_string, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_INVAL);
    //Valid string with invalid key
    size = nsl_strlen(basestring16);
    result = CSL_AesEncrypt(basestring16, encrypted, &size, invalid_key, nsl_strlen(invalid_key));
    CU_ASSERT_EQUAL(result, NE_INVAL);
    //Invalid string with invalid key
    size = nsl_strlen(invalid_string);
    result = CSL_AesEncrypt(invalid_string, encrypted, &size, invalid_key, nsl_strlen(invalid_key));
    CU_ASSERT_EQUAL(result, NE_INVAL);

/*---Check for encrypting/decrypting---*/

    //11 byte string with 16 byte key
    size = nsl_strlen(basestring11);
    result = CSL_AesEncrypt(basestring11, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
        //Decrypt with invalid key
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_2, nsl_strlen(key16_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring11);
        //Decrypt with valid key
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring11);

    //11 byte string with 24 byte key
    size = nsl_strlen(basestring11);
    result = CSL_AesEncrypt(basestring11, encrypted, &size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_2, nsl_strlen(key24_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring11);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring11);

    //11 byte string with 32 byte key
    size = nsl_strlen(basestring11);
    result = CSL_AesEncrypt(basestring11, encrypted, &size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_2, nsl_strlen(key32_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring11);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring11);

    //16 byte string with 16 byte key
    size = nsl_strlen(basestring16);
    result = CSL_AesEncrypt(basestring16, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_2, nsl_strlen(key16_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring16);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring16);

    //16 byte string with 24 byte key
    size = nsl_strlen(basestring16);
    result = CSL_AesEncrypt(basestring16, encrypted, &size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_2, nsl_strlen(key24_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring16);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring16);

    //16 byte string with 32 byte key
    size = nsl_strlen(basestring16);
    result = CSL_AesEncrypt(basestring16, encrypted, &size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_2, nsl_strlen(key32_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring16);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring16);

    //24 byte string with 16 byte key
    size = nsl_strlen(basestring24);
    result = CSL_AesEncrypt(basestring24, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_2, nsl_strlen(key16_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring24);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring24);

    //24 byte string with 24 byte key
    size = nsl_strlen(basestring24);
    result = CSL_AesEncrypt(basestring24, encrypted, &size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_2, nsl_strlen(key24_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring24);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring24);

    //24 byte string with 32 byte key
    size = nsl_strlen(basestring24);
    result = CSL_AesEncrypt(basestring24, encrypted, &size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_2, nsl_strlen(key32_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring24);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring24);

    //32 byte string with 16 byte key
    size = nsl_strlen(basestring32);
    result = CSL_AesEncrypt(basestring32, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_2, nsl_strlen(key16_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring32);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring32);

    //32 byte string with 24 byte key
    size = nsl_strlen(basestring32);
    result = CSL_AesEncrypt(basestring32, encrypted, &size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_2, nsl_strlen(key24_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring32);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring32);

    //32 byte string with 32 byte key
    size = nsl_strlen(basestring32);
    result = CSL_AesEncrypt(basestring32, encrypted, &size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_2, nsl_strlen(key32_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring32);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring32);

    //35 byte string with 16 byte key
    size = nsl_strlen(basestring35);
    result = CSL_AesEncrypt(basestring35, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_2, nsl_strlen(key16_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring35);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring35);

    //35 byte string with 24 byte key
    size = nsl_strlen(basestring35);
    result = CSL_AesEncrypt(basestring35, encrypted, &size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_2, nsl_strlen(key24_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring35);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring35);

    //35 byte string with 32 byte key
    size = nsl_strlen(basestring35);
    result = CSL_AesEncrypt(basestring35, encrypted, &size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_2, nsl_strlen(key32_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring35);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring35);

    //48 byte string with 16 byte key
    size = nsl_strlen(basestring48);
    result = CSL_AesEncrypt(basestring48, encrypted, &size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_2, nsl_strlen(key16_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring48);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key16_1, nsl_strlen(key16_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring48);

    //48 byte string with 24 byte key
    size = nsl_strlen(basestring48);
    result = CSL_AesEncrypt(basestring48, encrypted, &size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_2, nsl_strlen(key24_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring48);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key24_1, nsl_strlen(key24_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring48);

    //48 byte string with 32 byte key
    size = nsl_strlen(basestring48);
    result = CSL_AesEncrypt(basestring48, encrypted, &size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_2, nsl_strlen(key32_2));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_NOT_EQUAL(decrypted, basestring48);
    result = CSL_AesDecrypt(encrypted, decrypted, size, key32_1, nsl_strlen(key32_1));
    CU_ASSERT_EQUAL(result, NE_OK);
    CU_ASSERT_STRING_EQUAL(decrypted, basestring48);
}

// Vector test functions .............................................................................................

/*! Fill vector with integer array 

    Vector has to be valid
*/
static void 
FillVector(CSL_Vector* vector, const int* testArray, int size)
{
    int* start = NULL;

    CU_ASSERT_PTR_NOT_NULL(vector);
    CSL_VectorRemoveAll(vector);

    CSL_VectorSetSize(vector, size);
    start = CSL_VectorGetPointer(vector, 0);
    nsl_memcpy(start, testArray, size * sizeof(int));
}

/*! Test function to test CSL_VectorDifference(). 

    Takes two input arrays and calls CSL_VectorDifference(). It then compares the returned output vector with the
    given expected array.
*/
static void
TestDifference(CSL_Vector* input1,      /*!< Input vector 1, has to be valid */
               CSL_Vector* input2,      /*!< Input vector 2, has to be valid */
               CSL_Vector* output,      /*!< output vector, has to be valid */
               const int* testArray1,          /*!< Test array for input vector 1 */
               const int* testArray2,          /*!< Test array for input vector 2 */
               const int* expectedArray,       /*!< Expected output vector */
               int size1,                      /*!< Size of input array 1 */
               int size2,                      /*!< Size of input array 2 */
               int expectedSize                /*!< Size of expected output array */
               )
{
    int i = 0;

    FillVector(input1, testArray1, size1);
    FillVector(input2, testArray2, size2);

    CU_ASSERT(CSL_VectorDifference(input1, input2, output, &VectorCompareFunction));
    CU_ASSERT_EQUAL(CSL_VectorGetLength(output), expectedSize);

    // Check result in output vector
    for (i = 0; i < expectedSize; ++i)
    {
        int value = 0;
        CSL_VectorGet(output, i, &value);
        CU_ASSERT_EQUAL(value, expectedArray[i]);
    }

    // Clear all vectors for next test
    CSL_VectorRemoveAll(input1);
    CSL_VectorRemoveAll(input2);
    CSL_VectorRemoveAll(output);
}

/*! Helper function of TestUtilVectorRemoveDuplicate.

    @see TestUtilVectorRemoveDuplicate
*/
void
RemoveDuplicate(int* inputArray, int inputSize, int* outputArray, int outputSize)
{
    int i = 0;
    int count = 0;
    int actualOutputSize = 0;

    CSL_Vector* vector = CSL_VectorAlloc(sizeof(int));
    CU_ASSERT_PTR_NOT_NULL(vector);
    if (vector)
    {
        FillVector(vector, inputArray, inputSize);

        CSL_VectorSort(vector, &VectorCompareFunction);
        CSL_VectorRemoveDuplicates(vector, &VectorCompareFunction);

        /*
            Validate result. It has to match the output vector
        */

        actualOutputSize = CSL_VectorGetLength(vector);
        CU_ASSERT_EQUAL(outputSize, actualOutputSize);

        count = MIN(outputSize, actualOutputSize);
        for (i = 0; i < count; ++i)
        {
            int* item = (int*)CSL_VectorGetPointer(vector, i);
            CU_ASSERT_EQUAL(*item, outputArray[i]);
        }

        CSL_VectorDealloc(vector);
    }
}


/*! Compare function for vector tests

    The entries are 'int' values.
*/
static int 
VectorCompareFunction(const void* left, const void* right)
{
    if (*((int*)left) < *((int*)right))
    {
        return -1;
    }
    if (*((int*)left) > *((int*)right))
    {
        return 1;
    }
    return 0;
}

/*! Callback function for for-each test.

    @see TestUtilVectorForEach
    @see CSL_VectorForEachCallback
*/
int 
VectorForEachCallback(void* userData, void* itemPointer)
{
    static int count = 0;

    int* array1 = (int*)userData;
    
    int actualValue = *(int*)itemPointer;
    int expectedValue = *(array1 + (count++));

    CU_ASSERT_PTR_NOT_NULL(userData);
    CU_ASSERT_PTR_NOT_NULL(itemPointer);
    CU_ASSERT_EQUAL(actualValue, expectedValue);

    // Continue loop
    return 1;
}

static void 
TestUtilVector(void)
{
    int length = 0;
    int index, ret;
    struct UtilTestData* pTest = NULL;
    struct UtilTestData* pAnother = NULL;
    struct UtilTestData* pTestCheck = NULL;
    CSL_Vector* pVec = NULL, *pVecCopy = NULL;
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);

    pVec = CSL_VectorAlloc(sizeof(void *));
    CU_ASSERT(pVec != NULL);

    pTest = nsl_malloc(sizeof(struct UtilTestData));
    CU_ASSERT_PTR_NOT_NULL(pTest);

    CSL_VectorAppend(pVec, &pTest);

    length = CSL_VectorGetLength(pVec);
    CU_ASSERT_EQUAL(length, 1);

    // Get our vector back and check that it is correct
    CSL_VectorGet(pVec, 0, &pTestCheck);
    CU_ASSERT(pTest == pTestCheck);

    // We should just have one pointer
    length = CSL_VectorGetItemSize(pVec);
    CU_ASSERT_EQUAL(length, 4);

    pTestCheck = (struct UtilTestData *)CSL_VectorGetPointer(pVec, 0);
    CU_ASSERT_PTR_NOT_NULL(pTestCheck);

    // Append another vector
    pAnother = nsl_malloc(sizeof(struct UtilTestData));
    CSL_VectorAppend(pVec, &pAnother);

    // We should have two now
    length = CSL_VectorGetLength(pVec);
    CU_ASSERT_EQUAL(length, 2);

    // Check the copy function
    ret = CSL_VectorCopy(&pVecCopy, pVec, NULL, NULL);
    CU_ASSERT_EQUAL(ret, 1);
    length = CSL_VectorGetLength(pVecCopy);
    CU_ASSERT_EQUAL(length, 2);
    CSL_VectorRemoveAll(pVecCopy);
    length = CSL_VectorGetLength(pVecCopy);
    CU_ASSERT_EQUAL(length, 0);
    CSL_VectorDealloc(pVecCopy);

    CSL_VectorRemove(pVec, 0);
    length = CSL_VectorGetLength(pVec);
    CU_ASSERT_EQUAL(length, 1);

    CSL_VectorPop(pVec, &index);
    length = CSL_VectorGetLength(pVec);
    CU_ASSERT_EQUAL(length, 0);

    // Free our memory
    nsl_free(pTest);
    nsl_free(pAnother);
    CSL_VectorDealloc(pVec);

    PAL_Destroy(pal);
}

/*! Test CSL_VectorDifference() function. */
void 
TestUtilVectorDifference1(void)
{
    int testArray1[] = {1,2,3,4,5,6,7,8,9,10};

    CSL_Vector* input1 = CSL_VectorAlloc(sizeof(int));
    CSL_Vector* input2 = CSL_VectorAlloc(sizeof(int));
    CSL_Vector* output = CSL_VectorAlloc(sizeof(int));

    if ((!input1) || (!input2) || (!output))
    {
        CU_ASSERT(FALSE);
        return;
    }

    // First test empty vectors. This should just result in an empty output vector
    CU_ASSERT(CSL_VectorDifference(input1, input2, output, &VectorCompareFunction));
    CU_ASSERT_EQUAL(CSL_VectorGetLength(output), 0);

    // Fill second vector
    FillVector(input2, testArray1, sizeof(testArray1) / sizeof(int));

    // Compare again, the output vector should still be empty after the call
    CU_ASSERT(CSL_VectorDifference(input1, input2, output, &VectorCompareFunction));
    CU_ASSERT_EQUAL(CSL_VectorGetLength(output), 0);

    // Fill the first vector with the same array
    FillVector(input1, testArray1, sizeof(testArray1) / sizeof(int));

    // Compare again, the output vector should still be empty after the call
    CU_ASSERT(CSL_VectorDifference(input1, input2, output, &VectorCompareFunction));
    CU_ASSERT_EQUAL(CSL_VectorGetLength(output), 0);

    {
        int testValue = 11;
        int returnValue = 0;

        // Add one more item to vector one. It should result in the output vector.
        CSL_VectorAppend(input1, &testValue);
        CU_ASSERT(CSL_VectorDifference(input1, input2, output, &VectorCompareFunction));
        CU_ASSERT_EQUAL(CSL_VectorGetLength(output), 1);

        // Check result in output vector
        CSL_VectorGet(output, 0, &returnValue);
        CU_ASSERT_EQUAL(returnValue, testValue);
    }    

    // Another test
    {
        int i = 0;
        int count = sizeof(testArray1) / sizeof(int);

        CSL_VectorRemoveAll(input1);
        CSL_VectorRemoveAll(input2);
        CSL_VectorRemoveAll(output);

        // Now just fill the first vector but leave the second empty. It should copy the entire vector to the output vector.
        FillVector(input1, testArray1, count);
        CU_ASSERT(CSL_VectorDifference(input1, input2, output, &VectorCompareFunction));
        CU_ASSERT_EQUAL(CSL_VectorGetLength(output), count);

        // Validate result
        count = (CSL_VectorGetLength(output) < count) ? CSL_VectorGetLength(output) : count;
        for (i = 0; i < count; ++i)
        {
            int value = 0;
            CSL_VectorGet(output, i, &value);
            CU_ASSERT_EQUAL(value, testArray1[i]);
        }
    }

    CSL_VectorDealloc(input1);
    CSL_VectorDealloc(input2);
    CSL_VectorDealloc(output);
}

/*! Test CSL_VectorDifference() function. */
void 
TestUtilVectorDifference2(void)
{
    CSL_Vector* input1 = CSL_VectorAlloc(sizeof(int));
    CSL_Vector* input2 = CSL_VectorAlloc(sizeof(int));
    CSL_Vector* output = CSL_VectorAlloc(sizeof(int));

    if ((!input1) || (!input2) || (!output))
    {
        CU_ASSERT(FALSE);
        return;
    }

    // Test 1
    {
        int testArray1[] = {1,3,5,7,9};
        int testArray2[] = {2,4,6,8,10};
        int expectedArray[] = {1,3,5,7,9};

        TestDifference(input1, input2, output, testArray1, testArray2, expectedArray, sizeof(testArray1)/sizeof(int), sizeof(testArray2)/sizeof(int), sizeof(expectedArray)/sizeof(int));
    }

    // Test 2
    {
        int testArray1[] = {2,4,6,8,10};
        int testArray2[] = {1,3,5,7,9};
        int expectedArray[] = {2,4,6,8,10};

        TestDifference(input1, input2, output, testArray1, testArray2, expectedArray, sizeof(testArray1)/sizeof(int), sizeof(testArray2)/sizeof(int), sizeof(expectedArray)/sizeof(int));
    }

    // Test 3
    {
        int testArray1[] = {1,2,3,4,5,6,7,8,9,10};
        int testArray2[] = {1,3,5,7,9};
        int expectedArray[] = {2,4,6,8,10};

        TestDifference(input1, input2, output, testArray1, testArray2, expectedArray, sizeof(testArray1)/sizeof(int), sizeof(testArray2)/sizeof(int), sizeof(expectedArray)/sizeof(int));
    }

    // Test 4
    {
        int testArray1[] = {1,1,1,1,1,5,5,5,5,5,8,8,8,8,8};
        int testArray2[] = {0,0,0,3,7,10,20,30};
        int expectedArray[] = {1,1,1,1,1,5,5,5,5,5,8,8,8,8,8};

        TestDifference(input1, input2, output, testArray1, testArray2, expectedArray, sizeof(testArray1)/sizeof(int), sizeof(testArray2)/sizeof(int), sizeof(expectedArray)/sizeof(int));
    }

    // Test 5
    {
        int testArray1[] = {0,1,2,3,4,5,20,21,22,23,24,25};
        int testArray2[] = {0,1,1,2,2,3,3,4,4,5,5,6,7,8,9,10};
        int expectedArray[] = {20,21,22,23,24,25};

        TestDifference(input1, input2, output, testArray1, testArray2, expectedArray, sizeof(testArray1)/sizeof(int), sizeof(testArray2)/sizeof(int), sizeof(expectedArray)/sizeof(int));
    }

    CSL_VectorDealloc(input1);
    CSL_VectorDealloc(input2);
    CSL_VectorDealloc(output);
}

/*! Test the VectorRemove function.

    The remove function makes a block copy which I want to verify with this unit test.
*/
void 
TestUtilVectorRemove(void)
{
    int i = 0; 
    const int COUNT = 20;
    const int MIDDLE_INDEX = 7;
    int testArray[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

    CSL_Vector* vector = CSL_VectorAlloc(sizeof(int));
    CU_ASSERT_PTR_NOT_NULL(vector);

    /*
        Test removing the first element
    */

    FillVector(vector, testArray, COUNT);

    for (i = 0; i < COUNT; ++i)
    {
        int j = 0;
        int size = 0;

        // Remove first element. This should do a block copy of the remaining items.
        CSL_VectorRemove(vector, 0);

        // Validate all remaining items
        for (j = 0; j < (COUNT - i - 1); ++j)
        {
            int* item = (int*)CSL_VectorGetPointer(vector, j);
            CU_ASSERT_EQUAL(*item, testArray[j + i + 1]);
        }

        size = CSL_VectorGetLength(vector);
        CU_ASSERT_EQUAL(size, COUNT - i - 1);
    }

    /*
        Test removing element in the middle
    */

    FillVector(vector, testArray, COUNT);

    for (i = 0; i < COUNT - MIDDLE_INDEX; ++i)
    {
        int j = 0;
        int size = 0;

        // Remove element in the middle
        CSL_VectorRemove(vector, MIDDLE_INDEX);

        // Validate all remaining items
        for (j = 0; j < (COUNT - i - 1); ++j)
        {
            int* item = (int*)CSL_VectorGetPointer(vector, j);

            if (j < MIDDLE_INDEX)
            {
                CU_ASSERT_EQUAL(*item, testArray[j]);
            }
            else
            {
                CU_ASSERT_EQUAL(*item, testArray[j + i + 1]);
            }
        }

        size = CSL_VectorGetLength(vector);
        CU_ASSERT_EQUAL(size, COUNT - i - 1);
    }

    /*
        Test removing last element
    */

    FillVector(vector, testArray, COUNT);

    for (i = 0; i < COUNT; ++i)
    {
        int j = 0;
        int size = 0;

        // Remove last element
        CSL_VectorRemove(vector, COUNT - i - 1);

        // Validate all remaining items
        for (j = 0; j < (COUNT - i - 1); ++j)
        {
            int* item = (int*)CSL_VectorGetPointer(vector, j);
            CU_ASSERT_EQUAL(*item, testArray[j]);
        }

        size = CSL_VectorGetLength(vector);
        CU_ASSERT_EQUAL(size, COUNT - i - 1);
    }

    CSL_VectorDealloc(vector);
}

/*! Test vector remove duplicate function.
*/
void 
TestUtilVectorRemoveDuplicate(void)
{
    int input1[]    = {9,5,6,3,1,1,2,2,2,4,7,8,5,4,2,3,6,9,5,1,2,4,5,9,6,8,6,5,9,7,4,2,3,1,5,2,6,3,4,9,8,7};
    int output1[]   = {1,2,3,4,5,6,7,8,9};

    int input2[]    = {1,1,1,2,2,2,3,3,3,4,4,4,5,5,5,6,6,7,7,8,9};
    int output2[]   = {1,2,3,4,5,6,7,8,9};

    int input3[]    = {1,4,7};
    int output3[]   = {1,4,7};

    int input4[]    = {4,4};
    int output4[]   = {4};

    int input5[]    = {4};
    int output5[]   = {4};

    RemoveDuplicate(input1, sizeof(input1)/sizeof(int), output1, sizeof(output1)/sizeof(int));
    RemoveDuplicate(input2, sizeof(input2)/sizeof(int), output2, sizeof(output2)/sizeof(int));
    RemoveDuplicate(input3, sizeof(input3)/sizeof(int), output3, sizeof(output3)/sizeof(int));
    RemoveDuplicate(input4, sizeof(input4)/sizeof(int), output4, sizeof(output4)/sizeof(int));
    RemoveDuplicate(input5, sizeof(input5)/sizeof(int), output5, sizeof(output5)/sizeof(int));
}

/*! Test vector for-each function.
*/
void 
TestUtilVectorForEach(void)
{
    int array1[]   = {1,2,3,4,5,6,7,8,9};

    CSL_Vector* vector = CSL_VectorAlloc(sizeof(int));
    CU_ASSERT_PTR_NOT_NULL(vector);

    FillVector(vector, array1, sizeof(array1)/sizeof(int));

    CSL_VectorForEach(vector, &VectorForEachCallback, array1);

    CSL_VectorDealloc(vector);
}


/*! @} */

