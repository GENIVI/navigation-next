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

    @defgroup TestFile_h Unit Tests for PAL File

    Unit Tests for PAL file

    This file contains all unit tests for the PAL file component
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

#include "testcache.h"
#include "main.h"
#include "platformutil.h"
#include "palclock.h"

#ifdef WINCE
#include "winbase.h"
#endif
#include "palfile.h"

#include "cslcache.h"

#ifdef WINCE
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR "/"
#endif


// Local Constants ...............................................................................

static const byte OBFUSCATE[] = {
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};

#define CACHE_NAME "TESTCACHE"

#define CACHE_MAX_ITEMS_SMALL           5
#define CACHE_MAX_ITEMS_MEDIUM          50
#define CACHE_MAX_ITEMS_EXTRA_LARGE     1000

#define CACHE_ENTRY_SIZE_LARGE          1000


/*! Start pattern to fill cache data */
const byte START_PATTERN = 0x31;


// Local macros ..................................................................................

#define TEST_CACHE_FIND(cache, name, namelen, test) do {                                \
    byte* pdata=NULL;                                                                   \
    size_t pdataSize=0;												                    \
    CSL_CacheFind(cache, name, namelen, &pdata, &pdataSize, FALSE, TRUE, FALSE);        \
    CU_ASSERT(pdata != NULL);                                                           \
    if (pdata) {                                                                        \
    CU_ASSERT_EQUAL( (int)pdataSize, test);                                             \
    nsl_free(pdata); }                                                                  \
    }while(0);

#define TEST_CACHE_NOT_FIND(cache, name, namelen) do {                                  \
    byte* pdata=NULL;                                                                   \
    size_t pdataSize=0;                                                                 \
    NB_Error err = NE_OK;                                                               \
    err = CSL_CacheFind(cache, name, namelen, &pdata, &pdataSize, FALSE, TRUE, FALSE);  \
    CU_ASSERT(err != NE_OK);                                                            \
    CU_ASSERT(pdata == NULL);                                                           \
    CU_ASSERT_EQUAL(pdataSize, 0);                                                      \
    nsl_free(pdata);                                                                    \
    }while(0);



// Local Functions ...............................................................................

static void TestPalCreate(void);
static void TestCacheCreateDestroy(void);
static void TestCacheSetSize(void);
static void CacheUserFunc (uint32 ts, byte* name, size_t namelen, byte* data, size_t datalen, void* puser);
static void TestCacheAdd(void);
static void TestCacheSaveLoad(void);
static void TestCacheNewLoad(void);
static void TestCacheLarge(void);
static void TestCacheUserData(void);

// Tests for memory cache
static void TestVerifyTestData(void);
static void TestCacheAddCopy(void);
static void TestCacheAddNonCopy(void);
static void TestCacheDrop(void);
static void TestCacheAccessAndDrop(void);
static void TestCacheProtect(void);
static void TestCacheProtectAndFail(void);
static void TestCacheFindAndProtect(void);
static void TestCacheReplaceEntry(void);
static void TestCacheRemove(void);

// Tests for persistent/file cache
static void TestCacheSaveAccessed(void);
static void TestCacheSaveDirty(void);
static void TestCacheSaveAndDrop(void);
static void TestCacheRemoveOrphanedItems(void);
static void TestCachePersistentOrder(void);
static void TestCacheNotifications(void);

static void TestCacheStressTest(void);
static void TestCachePerformance(void);
static void TestCacheResizeCache(void);

// Test persistent cache using non-obfuscated files
static void TestCacheNonObfuscatedFiles(void);
static void TestCacheNonObfuscatedFilesReload(void);

// Tests to clear cache
static void TestCacheClearAll(void);
static void TestCacheClearMemoryOnly(void);
static void TestCacheClearPersistentOnly(void);
static void TestCacheClearCheckProtected(void);

// Functions for test data buffers
static boolean AllocateTestData(byte** data, int entrySize, int entryCount);
static void FreeTestData(byte** data, int entryCount);
static void VerifyTestEntry(byte* entry, int entrySize, int index);

// Helper functions
static void CleanupTestFiles(PAL_Instance* pal);
static int GetElapsedMilliseconds(const PAL_ClockDateTime* time1, const PAL_ClockDateTime* time2);
static void RemoveNotificationFunction(CSL_Cache* cache, 
                                       const byte* name,
                                       size_t namelen,
                                       void* userData);


// Public Functions ..............................................................................

/*! Add all your test functions here

@return None
*/
void
TestCache_AddAllTests( CU_pSuite pTestSuite )
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestPalCreate",                &TestPalCreate);
    CU_add_test(pTestSuite, "TestCacheCreateDestroy",       &TestCacheCreateDestroy);
    CU_add_test(pTestSuite, "TestCacheSetGetSize",          &TestCacheSetSize);
    CU_add_test(pTestSuite, "TestCacheAdd",                 &TestCacheAdd);
    CU_add_test(pTestSuite, "TestCacheSaveLoad",            &TestCacheSaveLoad);
    CU_add_test(pTestSuite, "TestCacheNewLoad",             &TestCacheNewLoad);
    CU_add_test(pTestSuite, "TestCacheLarge",               &TestCacheLarge);
    CU_add_test(pTestSuite, "TestCacheUserData",            &TestCacheUserData);

    // Test memory cache
    CU_add_test(pTestSuite, "TestVerifyTestData",           &TestVerifyTestData);
    CU_add_test(pTestSuite, "TestCacheAddCopy",             &TestCacheAddCopy);
    CU_add_test(pTestSuite, "TestCacheAddNonCopy",          &TestCacheAddNonCopy);
    CU_add_test(pTestSuite, "TestCacheDrop",                &TestCacheDrop);
    CU_add_test(pTestSuite, "TestCacheAccessAndDrop",       &TestCacheAccessAndDrop);
    CU_add_test(pTestSuite, "TestCacheProtect",             &TestCacheProtect);
    CU_add_test(pTestSuite, "TestCacheProtectAndFail",      &TestCacheProtectAndFail);
    CU_add_test(pTestSuite, "TestCacheFindAndProtect",      &TestCacheFindAndProtect);
    CU_add_test(pTestSuite, "TestCacheReplaceEntry",        &TestCacheReplaceEntry);
    CU_add_test(pTestSuite, "TestCacheRemove",              &TestCacheRemove);

    // Test persistent cache
    CU_add_test(pTestSuite, "TestCacheSaveAccessed",        &TestCacheSaveAccessed);
    CU_add_test(pTestSuite, "TestCacheSaveDirty",           &TestCacheSaveDirty);
    CU_add_test(pTestSuite, "TestCacheSaveAndDrop",         &TestCacheSaveAndDrop);
    CU_add_test(pTestSuite, "TestCacheRemoveOrphanedItems", &TestCacheRemoveOrphanedItems);
    CU_add_test(pTestSuite, "TestCachePersistentOrder",     &TestCachePersistentOrder);
    CU_add_test(pTestSuite, "TestCacheNotifications",       &TestCacheNotifications);

    CU_add_test(pTestSuite, "TestCacheStressTest",          &TestCacheStressTest);
    CU_add_test(pTestSuite, "TestCacheResizeCache",         &TestCacheResizeCache);
   
    // Don't run by default (takes a while)
//    CU_add_test(pTestSuite, "TestCachePerformance",         &TestCachePerformance);

    CU_add_test(pTestSuite, "TestCacheNonObfuscatedFiles",  &TestCacheNonObfuscatedFiles);
    CU_add_test(pTestSuite, "TestCacheNonObfuscatedFilesReload",  &TestCacheNonObfuscatedFilesReload);

    // Tests to clear cache
    CU_add_test(pTestSuite, "TestCacheClearAll",            &TestCacheClearAll);
    CU_add_test(pTestSuite, "TestCacheClearMemoryOnly",     &TestCacheClearMemoryOnly);
    CU_add_test(pTestSuite, "TestCacheClearCheckProtected", &TestCacheClearCheckProtected);
};


/*! Add common initialization code here.

@return 0

@see TestFile_SuiteCleanup
*/
int
TestCache_SuiteSetup()
{
    PAL_Instance* pal = PAL_CreateInstance();
    if (pal)
    {
        CleanupTestFiles(pal);
        PAL_Destroy(pal);
    }
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestFile_SuiteSetup
*/
int 
TestCache_SuiteCleanup()
{
    PAL_Instance* pal = PAL_CreateInstance();
    if (pal)
    {
        CleanupTestFiles(pal);
        PAL_Destroy(pal);
    }
    return 0;
}


// Local Functions ...............................................................................

static void TestPalCreate(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CU_ASSERT(pal != NULL);
    PAL_Destroy(pal);
}

static void TestCacheCreateDestroy(void)
{
    PAL_Instance *pal = PAL_CreateInstance();

    CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);

    CU_ASSERT(cache != NULL);
    
    CSL_CacheDealloc(cache);

    CU_ASSERT(cache != NULL);
    
    PAL_Destroy(pal);
}

static void TestCacheSetSize(void)
{
    PAL_Instance *pal = PAL_CreateInstance();

    int maxsizeMemory = 0;
    int maxsizePersistent = 0;

    CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, 200, 300, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
    CU_ASSERT_PTR_NOT_NULL(cache);
    if(!cache)
    {
        return;
    }
    CSL_CacheGetStats(cache, &maxsizeMemory, &maxsizePersistent, NULL, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(maxsizeMemory, 200);
    CU_ASSERT_EQUAL(maxsizePersistent, 300);

    CSL_CacheDealloc(cache);

    PAL_Destroy(pal);
}

static void CacheUserFunc (uint32 ts, byte* name, size_t namelen, byte* data, size_t datalen, void* puser)
{
    *data = '9';
}

static void TestCacheAdd(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CSL_Cache* cache = NULL;
    NB_Error err = NE_OK;
    
    int size = 0;
    int protectedItems = 0;
    int i = 0;
    int namelen = sizeof(byte);
    int datalen = namelen;

    byte data[CACHE_MAX_ITEMS_SMALL] = {'1', '2', '3', '4', '5'};
    byte name[CACHE_MAX_ITEMS_SMALL] = {'A', 'B', 'C', 'D', 'E'};
    
    cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
    CU_ASSERT(cache != NULL);
    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 0);

    for (i=0; i<CACHE_MAX_ITEMS_SMALL; i++)
    {
        err = CSL_CacheAdd(cache, &name[i], sizeof(byte),  &data[i], sizeof (byte), FALSE, TRUE, 0);
        CU_ASSERT_EQUAL(err, NE_OK);
        CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
        CU_ASSERT_EQUAL(size, i+1);
    }

    CSL_CacheGetStats(cache, NULL, NULL, NULL, NULL, NULL, &protectedItems);
    CU_ASSERT_EQUAL(protectedItems, 0);

    TEST_CACHE_FIND(cache, &name[0], namelen, datalen);
    TEST_CACHE_FIND(cache, &name[1], namelen, datalen);
    TEST_CACHE_FIND(cache, &name[2], namelen, datalen);
    TEST_CACHE_FIND(cache, &name[3], namelen, datalen);
    TEST_CACHE_FIND(cache, &name[4], namelen, datalen);

    CSL_CacheProtect(cache, &name[3], sizeof(byte));
    CSL_CacheGetStats(cache, NULL, NULL, NULL, NULL, NULL, &protectedItems);
    CU_ASSERT_EQUAL(protectedItems, 1);

    CSL_CacheProtect(cache, &name[4], sizeof(byte));
    CSL_CacheGetStats(cache, NULL, NULL, NULL, NULL, NULL, &protectedItems);
    CU_ASSERT_EQUAL(protectedItems, 2);

    CSL_CacheClearProtect(cache);
    CSL_CacheGetStats(cache, NULL, NULL, NULL, NULL, NULL, &protectedItems);
    CU_ASSERT_EQUAL(protectedItems, 0);

    CSL_CacheForEach(cache, CacheUserFunc, NULL, FALSE);
    CSL_CacheGetStats(cache, NULL, NULL, NULL, NULL, NULL, &protectedItems);
    CU_ASSERT_EQUAL(protectedItems, 0);

    for (i=0; i<CACHE_MAX_ITEMS_SMALL; i++)
    {
        TEST_CACHE_FIND(cache, &name[i], namelen, datalen);	
    }

    CSL_CacheDealloc(cache);

    // Allocate cache with one memory and one persistent entry
    cache = CSL_CacheAlloc(pal, CACHE_NAME, 1, 1, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
    CU_ASSERT(cache != NULL);
    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 0);

    for (i=0; i<CACHE_MAX_ITEMS_SMALL; i++)
    {
        // This will replace the existing entry (since we only allow one entry)
        err = CSL_CacheAdd(cache, &name[i], sizeof(byte),  &data[i], sizeof (byte), FALSE, TRUE, 0);
        CU_ASSERT_EQUAL(err, NE_OK);		
    }

    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 1);

    TEST_CACHE_NOT_FIND(cache, &name[0], namelen);
    TEST_CACHE_NOT_FIND(cache, &name[1], namelen);
    TEST_CACHE_NOT_FIND(cache, &name[2], namelen);
    TEST_CACHE_NOT_FIND(cache, &name[3], namelen);
    TEST_CACHE_FIND(cache, &name[4], namelen, datalen);

    CSL_CacheDealloc(cache);

    PAL_Destroy(pal);
}

static void TestCacheSaveLoad(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CSL_Cache* cache = NULL;
    NB_Error err = NE_OK;

    int size=0;
    int i=0;
    int namelen = sizeof(byte);
    int datalen = namelen;

    byte data[CACHE_MAX_ITEMS_SMALL] = {'1', '2', '3', '4', '5'};
    byte name[CACHE_MAX_ITEMS_SMALL] = {'A', 'B', 'C', 'D', 'E'};

    cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
    CU_ASSERT(cache != NULL);
    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 0);

    for (i=0; i<CACHE_MAX_ITEMS_SMALL; i++)
    {
        err = CSL_CacheAdd(cache, &name[i], sizeof(byte),  &data[i], sizeof (byte), FALSE, TRUE, 0);
        CU_ASSERT_EQUAL(err, NE_OK);
        CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
        CU_ASSERT_EQUAL(size, i+1);
    }

    TEST_CACHE_FIND(cache, &name[0], namelen, datalen); //Already In memory
    TEST_CACHE_FIND(cache, &name[2], namelen, datalen); //Already In memory
    TEST_CACHE_FIND(cache, &name[3], namelen, datalen); //Already In memory

    // Save the just accessed entries to the file cache. Set the 'removeFromMemory' flag to remove them from the memory cache.
    err = CSL_CacheSaveAccessed(cache, -1, NULL, TRUE);
    CU_ASSERT_EQUAL(err, NE_OK);

    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_ErrFileNotExist);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_Ok);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_ErrFileNotExist);

    // This will load the item back to the memory cache
    TEST_CACHE_FIND(cache, &name[3], namelen, datalen);

    err = CSL_CacheSaveIndex(cache);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000000"), PAL_Ok);

    // Now we should have 3 items in the memory cache. The two original one we just loaded again from the file-cache.
    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 3);

    TEST_CACHE_FIND(cache, &name[0], namelen, datalen); //Should load this from the file
    TEST_CACHE_FIND(cache, &name[1], namelen, datalen); //Already In memory
    
    // Now we should have 4 entries in the memory cache
    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 4);

    err = CSL_CacheSaveAccessed(cache, -1, NULL, TRUE);
    CU_ASSERT_EQUAL(err, NE_OK);
    err = CSL_CacheSaveIndex(cache);
    CU_ASSERT_EQUAL(err, NE_OK);

    TEST_CACHE_FIND(cache, &name[3], namelen, datalen);  //Should load this from the file

    err = CSL_CacheSaveAccessed(cache, -1, NULL, TRUE);
    CU_ASSERT_EQUAL(err, NE_OK);
    err = CSL_CacheSaveIndex(cache);
    CU_ASSERT_EQUAL(err, NE_OK);

    CSL_CacheDealloc(cache);
    
    PAL_Destroy(pal);
}

static void TestCacheNewLoad(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CSL_Cache* c_loaded = NULL;
    NB_Error err = NE_OK;
    int size=0;
    int namelen = sizeof(byte);
    int datalen = namelen;

    byte name[CACHE_MAX_ITEMS_SMALL] = {'A', 'B', 'C'};
    
    // This test reloads the cache index file from the previous test. Make sure when changing the previous test to also update this test.
    c_loaded = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
    CU_ASSERT_PTR_NOT_NULL(c_loaded);
    if(!c_loaded)
    {
        return;
    }
    err = CSL_CacheLoadIndex(c_loaded);
    CU_ASSERT_EQUAL(err, NE_OK);

    // At this time only the index-file is loaded but non of the entries are in memory, yet.
    CSL_CacheGetStats(c_loaded, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 0);

    // This will load the entries into memory
    TEST_CACHE_FIND(c_loaded, &name[0], namelen, datalen);
    TEST_CACHE_FIND(c_loaded, &name[1], namelen, datalen);
    TEST_CACHE_FIND(c_loaded, &name[2], namelen, datalen);

    // Now we should have three entries in memory
    CSL_CacheGetStats(c_loaded, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 3);

    CSL_CacheDealloc(c_loaded);
    
    PAL_FileRemoveDirectory(pal, "TESTCACHE", TRUE);

    PAL_Destroy(pal);
}

static void TestCacheLarge(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    CSL_Cache* cache = NULL;
    NB_Error err = NE_OK;
    int size=0;
    int i=0;
    char name[10];
    char data[10];

    cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_EXTRA_LARGE, CACHE_MAX_ITEMS_EXTRA_LARGE, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
    CU_ASSERT(cache != NULL);
    CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
    CU_ASSERT_EQUAL(size, 0);

    for (i=0; i<CACHE_MAX_ITEMS_EXTRA_LARGE; i++)
    {
        nsl_sprintf (name, "%s%d", "cache", i);
        nsl_sprintf (data, "%s%d", "c", i);		
        err = CSL_CacheAdd(cache, (byte*) name, nsl_strlen(name),  (byte*) data, nsl_strlen(data), FALSE, TRUE, 0);
        CU_ASSERT_EQUAL(err, NE_OK);	
        CSL_CacheGetStats(cache, NULL, NULL, NULL, &size, NULL, NULL);
        CU_ASSERT_EQUAL(size, i+1);
    }

    for (i=0; i<50; i++)
    {
        int r = rand() % CACHE_MAX_ITEMS_EXTRA_LARGE;
        nsl_sprintf (name, "%s%d", "cache", r);
        nsl_sprintf (data, "%s%d", "c", r);				
        TEST_CACHE_FIND(cache, (byte*) name, nsl_strlen(name), nsl_strlen(data));
    }

    err = CSL_CacheSaveAccessed(cache, -1, NULL, TRUE);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = CSL_CacheSaveIndex(cache);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000000"), PAL_Ok);

    nsl_sprintf (name, "%s%d", "cache", 143);
    nsl_sprintf (data, "%s%d", "c", 143);				
    TEST_CACHE_FIND(cache, (byte*) name, nsl_strlen(name), nsl_strlen(data));

    nsl_sprintf (name, "%s%d", "cache", 380);
    nsl_sprintf (data, "%s%d", "c", 380);				
    TEST_CACHE_FIND(cache, (byte*) name, nsl_strlen(name), nsl_strlen(data));

    CSL_CacheDealloc(cache);

    // This creates a much smaller cache
    cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_MEDIUM, CACHE_MAX_ITEMS_MEDIUM, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);

    for (i=0; i<CACHE_MAX_ITEMS_EXTRA_LARGE; i++)
    {
        // Since the cache is much smaller most entries get thrown out 
        nsl_sprintf (name, "%s%d", "cache", i);
        nsl_sprintf (data, "%s%d", "c", i);		
        err = CSL_CacheAdd(cache, (byte*) name, nsl_strlen(name),  (byte*) data, nsl_strlen(data), FALSE, TRUE, 0);
        CU_ASSERT_EQUAL(err, NE_OK);			
    }
    
    nsl_sprintf (name, "%s%d", "cache", 999);
    nsl_sprintf (data, "%s%d", "c", 999);				
    TEST_CACHE_FIND(cache, (byte*) name, nsl_strlen(name), nsl_strlen(data));

    nsl_sprintf (name, "%s%d", "cache", 995);
    nsl_sprintf (data, "%s%d", "c", 995);				
    TEST_CACHE_FIND(cache, (byte*) name, nsl_strlen(name), nsl_strlen(data));

    nsl_sprintf (name, "%s%d", "cache", 1);
    nsl_sprintf (data, "%s%d", "c", 1);				
    TEST_CACHE_NOT_FIND(cache, (byte*) name, nsl_strlen(name));

    CSL_CacheDealloc(cache);

    PAL_FileRemoveDirectory(pal, "TESTCACHE", TRUE);

    PAL_Destroy(pal);
}

/*! Test saving/retrieving user data from index-file.

    @return None, assert on failures.
*/
void 
TestCacheUserData(void)
{
    PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        const char* USER_DATA_1 = "Test user Data. 88888 99999";
        const char* USER_DATA_2 = "Some more user Data. 44444 22222";
        const char* USER_DATA_3 = "And yet more user Data. 55555 00000";

        /*
            Create cache once.
        */
        CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            byte* data = NULL;
            size_t size = 0;

            // Set user data including NULL termination
            NB_Error result = CSL_CacheSetUserData(cache, (byte*)USER_DATA_1, nsl_strlen(USER_DATA_1) + 1);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Validate data
            result = CSL_CacheGetUserData(cache, &data, &size);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(size, nsl_strlen(USER_DATA_1) + 1);
            CU_ASSERT_STRING_EQUAL(data, USER_DATA_1);

            // Reset-user data.
            result = CSL_CacheSetUserData(cache, NULL, 0);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Validate data
            result = CSL_CacheGetUserData(cache, &data, &size);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(size, 0);
            CU_ASSERT_PTR_NULL(data);

            // Set to second data
            result = CSL_CacheSetUserData(cache, (byte*)USER_DATA_2, nsl_strlen(USER_DATA_2) + 1);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Validate data
            result = CSL_CacheGetUserData(cache, &data, &size);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(size, nsl_strlen(USER_DATA_2) + 1);
            CU_ASSERT_STRING_EQUAL(data, USER_DATA_2);

            // Set to third data
            result = CSL_CacheSetUserData(cache, (byte*)USER_DATA_3, nsl_strlen(USER_DATA_3) + 1);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Validate data
            result = CSL_CacheGetUserData(cache, &data, &size);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(size, nsl_strlen(USER_DATA_3) + 1);
            CU_ASSERT_STRING_EQUAL(data, USER_DATA_3);

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        /*
            Reload cache again
        */
        cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            byte* data = NULL;
            size_t size = 0;

            NB_Error result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Validate data
            result = CSL_CacheGetUserData(cache, &data, &size);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(size, nsl_strlen(USER_DATA_3) + 1);
            CU_ASSERT_PTR_NOT_NULL(data);
            if (data)
            {
                CU_ASSERT_STRING_EQUAL(data, USER_DATA_3);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }
}

/*! Simple test to verify that our test data generation works.

    @return None, assert on failures.
*/
void
TestVerifyTestData(void)
{
    // Array to hold the pointers for the entries
    byte* testDataArray[CACHE_MAX_ITEMS_MEDIUM] = {0};

    boolean result = AllocateTestData(testDataArray, CACHE_ENTRY_SIZE_LARGE, CACHE_MAX_ITEMS_MEDIUM);
    CU_ASSERT_EQUAL(result, TRUE);
    if (result)
    {
        // Verify the pattern of the entries
        int index = 0;
        for (index = 0; index < CACHE_MAX_ITEMS_MEDIUM; ++index)
        {
            VerifyTestEntry(testDataArray[index], CACHE_ENTRY_SIZE_LARGE, index);
        }

        FreeTestData(testDataArray, CACHE_MAX_ITEMS_MEDIUM);
    }
}

/*! Test adding entries to the cache. Let the cache make a copy of the added data.

    @return None, assert on failures.
*/
void 
TestCacheAddCopy(void)
{
    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[CACHE_MAX_ITEMS_SMALL] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[CACHE_MAX_ITEMS_SMALL] = {0x00, 0x01, 0x02, 0x03, 0x04};

    if (!AllocateTestData(testDataArray, CACHE_ENTRY_SIZE_LARGE, CACHE_MAX_ITEMS_SMALL))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;

            // Add entries, set the 'makeCopy' flag
            for (i = 0; i < CACHE_MAX_ITEMS_SMALL; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], CACHE_ENTRY_SIZE_LARGE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Now we can free the test data it is no longer needed.
            FreeTestData(testDataArray, CACHE_MAX_ITEMS_SMALL);

            // Get entries
            for (i = 0; i < CACHE_MAX_ITEMS_SMALL; ++i)
            {
                byte* entry = NULL;
                size_t entrySize = 0;

                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, CACHE_ENTRY_SIZE_LARGE);

                // Verify data
                VerifyTestEntry(entry, CACHE_ENTRY_SIZE_LARGE, i);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }
}

/*! Test adding entries to the cache keeping the original allocated data.

    @return None, assert on failures.
*/
void 
TestCacheAddNonCopy(void)
{
    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[CACHE_MAX_ITEMS_SMALL] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[CACHE_MAX_ITEMS_SMALL] = {0x00, 0x01, 0x02, 0x03, 0x04};

    if (!AllocateTestData(testDataArray, CACHE_ENTRY_SIZE_LARGE, CACHE_MAX_ITEMS_SMALL))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;

            // Add entries, set the 'makeCopy' flag to 'FALSE'
            for (i = 0; i < CACHE_MAX_ITEMS_SMALL; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], CACHE_ENTRY_SIZE_LARGE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // You can uncomment this line in order to verify that the data was indeed not copied. If you 
            // uncomment this line then CSL_CacheDealloc() will crash or the test will fail due to duplicate freeing.
//            FreeTestData(testDataArray, CACHE_MAX_ITEMS_SMALL);

            // Get entries
            for (i = 0; i < CACHE_MAX_ITEMS_SMALL; ++i)
            {
                byte* entry = NULL;
                size_t entrySize = 0;

                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, CACHE_ENTRY_SIZE_LARGE);

                // Verify data
                VerifyTestEntry(entry, CACHE_ENTRY_SIZE_LARGE, i);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }
}

/*! Test functionality of dropping entries when the cache exceeds its maximum size

    @return None, assert on failures.
*/
void 
TestCacheDrop(void)
{
    #define DROP_TEST_ARRAY_COUNT       10
    #define DROP_TEST_CACHE_MAX_COUNT   5
    #define DROP_TEST_ENTRY_SIZE        100

    PAL_Instance* pal = NULL;

    // Run the test twice. Once with the 'makeCopy' flag, once without it.
    nb_boolean makeCopy = FALSE;
    int testIndex = 0;
    for (testIndex = 0; testIndex < 2; ++testIndex)
    {
        // Array to hold the pointers for the test entries
        byte* testDataArray[DROP_TEST_ARRAY_COUNT] = {0};

        const size_t CACHE_NAME_LENGTH = 1;
        byte cacheNames[DROP_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A};

        if (!AllocateTestData(testDataArray, DROP_TEST_ENTRY_SIZE, DROP_TEST_ARRAY_COUNT))
        {
            CU_ASSERT(FALSE);
            return;
        }

        // Toggle flag
        makeCopy = makeCopy ? FALSE : TRUE;

        pal = PAL_CreateInstance();
        CU_ASSERT_PTR_NOT_NULL(pal);
        if (pal)
        {
            // Create cache with smaller size then test array
            CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, DROP_TEST_CACHE_MAX_COUNT, DROP_TEST_CACHE_MAX_COUNT, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
            CU_ASSERT_PTR_NOT_NULL(cache);
            if (cache)
            {
                int i = 0;
                NB_Error result = NE_OK;
                byte* entry = NULL;
                size_t entrySize = 0;

                // Add 10 entries. This should drop the first 5 entries
                for (i = 0; i < 10; ++i)
                {
                    result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], DROP_TEST_ENTRY_SIZE, FALSE, makeCopy, 0);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

                // Try to get the first 5 entries. This should fail
                for (i = 0; i < 5; ++i)
                {
                    // Get the entry. This should fail
                    result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }

                // Get the last 5 entries. This should succeed
                for (i = 5; i < 10; ++i)
                {
                    // Get the entry
                    result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, DROP_TEST_ENTRY_SIZE);

                    // Verify data
                    VerifyTestEntry(entry, DROP_TEST_ENTRY_SIZE, i);
                }

                CSL_CacheDealloc(cache);
            }

            PAL_Destroy(pal);
        }

        // Only free data if we made a copy. If not then the cache object owns the memory and will free it.
        if (makeCopy)
        {
            FreeTestData(testDataArray, DROP_TEST_ARRAY_COUNT);
        }
    }
}

/*! Test dropping of entries after access.

    Once cache entries get accessed it updates the timestamp. Once the cache is full it will drop the oldest
    entries. This test verifies this.

    @return None, assert on failures.
*/
void 
TestCacheAccessAndDrop(void)
{
    #define ACCESS_AND_DROP_TEST_ARRAY_COUNT       10
    #define ACCESS_AND_DROP_TEST_CACHE_MAX_COUNT   5
    #define ACCESS_AND_DROP_TEST_ENTRY_SIZE        100

    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[ACCESS_AND_DROP_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[ACCESS_AND_DROP_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A};

    if (!AllocateTestData(testDataArray, ACCESS_AND_DROP_TEST_ENTRY_SIZE, ACCESS_AND_DROP_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        // Create cache with smaller size then test array
        CSL_Cache* cache = CSL_CacheAlloc(pal, 
                                          CACHE_NAME, 
                                          ACCESS_AND_DROP_TEST_CACHE_MAX_COUNT, 
                                          ACCESS_AND_DROP_TEST_CACHE_MAX_COUNT, 
                                          OBFUSCATE, 
                                          sizeof(OBFUSCATE), 
                                          NULL,
                                          TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;
            byte* entry = NULL;
            size_t entrySize = 0;

            // Add 5 entries.
            for (i = 0; i < 5; ++i)
            {
                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], ACCESS_AND_DROP_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Access first 2 entries. This will reset the timestamp for those entries
            for (i = 0; i <= 1; ++i)
            {
                // Get the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Add 3 more entries.
            for (i = 5; i <= 7; ++i)
            {
                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], ACCESS_AND_DROP_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < ACCESS_AND_DROP_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries with index 0, 1, 5, 6, 7 in the cache
                if ((i == 0) || (i == 1) || ((i >= 5) && (i <= 7)))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, ACCESS_AND_DROP_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, ACCESS_AND_DROP_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            // Now access '0', '5' and '7'
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheFind(cache, &cacheNames[5], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheFind(cache, &cacheNames[7], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Add 2 more entries
            for (i = 8; i <= 9; ++i)
            {
                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], ACCESS_AND_DROP_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < ACCESS_AND_DROP_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries (with index) 0, 5, 7, 8, 9 in the cache
                if ((i == 0) || (i == 5) || ((i >= 7) && (i <= 9)))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, ACCESS_AND_DROP_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, ACCESS_AND_DROP_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // Don't free test data. It is owned by the cache object.
}

/*! Test protecting cache entries.

    @return None, assert on failures.
*/
void 
TestCacheProtect(void)
{
    #define PROTECT_TEST_ARRAY_COUNT       14
    #define PROTECT_TEST_CACHE_MAX_COUNT   5
    #define PROTECT_TEST_ENTRY_SIZE        1999

    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[PROTECT_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[PROTECT_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0xA9, 0xA8, 0xA7, 0xA6};

    if (!AllocateTestData(testDataArray, PROTECT_TEST_ENTRY_SIZE, PROTECT_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        // Create cache with smaller size then test array
        CSL_Cache* cache = CSL_CacheAlloc(pal, 
                                          CACHE_NAME, 
                                          PROTECT_TEST_CACHE_MAX_COUNT, 
                                          PROTECT_TEST_CACHE_MAX_COUNT, 
                                          OBFUSCATE, 
                                          sizeof(OBFUSCATE), 
                                          NULL,
                                          TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;
            byte* entry = NULL;
            size_t entrySize = 0;

            // Add 5 entries.
            for (i = 0; i < 5; ++i)
            {
                // Protect index '0', '2' and '4'
                nb_boolean protect = FALSE;
                if ((i == 0) || (i == 2) || (i == 4))
                {
                    protect = TRUE;
                }

                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_TEST_ENTRY_SIZE, protect, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Add 2 more entries.
            for (i = 5; i <= 6; ++i)
            {
                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < PROTECT_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries with index 0, 2, 4, 5, 6 in the cache
                if ((i == 0) || (i == 2) || ((i >= 4) && (i <= 6)))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, PROTECT_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, PROTECT_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            // Clear protected flag for '0'
            result = CSL_CacheClearProtectOne(cache, &cacheNames[0], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Set protected flag for '6'
            result = CSL_CacheProtect(cache, &cacheNames[6], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Add 2 more entries.
            for (i = 7; i <= 8; ++i)
            {
                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < PROTECT_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries with index 2, 4, 6, 7, 8 in the cache
                if ((i == 2) || (i == 4) || ((i >= 6) && (i <= 8)))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, PROTECT_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, PROTECT_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            // Clear all protected flags
            CSL_CacheClearProtect(cache);

            // Add 5 more entries.
            for (i = 9; i <= 13; ++i)
            {
                // Add entry, don't make copy.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < PROTECT_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries with index 9 - 13 in the cache
                if ((i >= 9) && (i <= 13))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, PROTECT_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, PROTECT_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // Don't free test data. It is owned by the cache object.
}

/*! Test that when all entries in the cache are protected that the add will fail.

    @return None, assert on failures.
*/
void 
TestCacheProtectAndFail(void)
{
    #define PROTECT_AND_FAIL_TEST_ARRAY_COUNT       15
    #define PROTECT_AND_FAIL_TEST_CACHE_MAX_COUNT   5
    #define PROTECT_AND_FAIL_TEST_ENTRY_SIZE        5

    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[PROTECT_AND_FAIL_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[PROTECT_AND_FAIL_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 
                                                          0x41, 0x42, 0x43, 0x44, 0x45};

    if (!AllocateTestData(testDataArray, PROTECT_AND_FAIL_TEST_ENTRY_SIZE, PROTECT_AND_FAIL_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        // Create cache with smaller size then test array
        CSL_Cache* cache = CSL_CacheAlloc(pal, 
                                          CACHE_NAME, 
                                          PROTECT_AND_FAIL_TEST_CACHE_MAX_COUNT, 
                                          PROTECT_AND_FAIL_TEST_CACHE_MAX_COUNT, 
                                          OBFUSCATE, 
                                          sizeof(OBFUSCATE), 
                                          NULL,
                                          TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;
            byte* entry = NULL;
            size_t entrySize = 0;

            // Add 5 entries.
            for (i = 0; i < 5; ++i)
            {
                // Add entry. Protect them all
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_AND_FAIL_TEST_ENTRY_SIZE, TRUE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Try to add 5 more entries.
            for (i = 5; i < 10; ++i)
            {
                // This should fail since all entries are protected
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_AND_FAIL_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_RES);
            }

            // Clear all protected flags
            CSL_CacheClearProtect(cache);

            // Add 5 more entries.
            for (i = 10; i < 15; ++i)
            {
                // Add entry
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PROTECT_AND_FAIL_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < PROTECT_AND_FAIL_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries with index 10 - 14 in the cache
                if ((i >= 10) && (i <= 14))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, PROTECT_AND_FAIL_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, PROTECT_AND_FAIL_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, PROTECT_AND_FAIL_TEST_ARRAY_COUNT);
}

/*! Test to lock/protect entries using the find function.

    @return None, assert on failures.
*/
void 
TestCacheFindAndProtect(void)
{
    #define FIND_AND_PROTECT_TEST_ARRAY_COUNT       15
    #define FIND_AND_PROTECT_TEST_CACHE_MAX_COUNT   5
    #define FIND_AND_PROTECT_TEST_ENTRY_SIZE        5

    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[FIND_AND_PROTECT_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[FIND_AND_PROTECT_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 
                                                          0x41, 0x42, 0x43, 0x44, 0x45};

    if (!AllocateTestData(testDataArray, FIND_AND_PROTECT_TEST_ENTRY_SIZE, FIND_AND_PROTECT_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        // Create cache with smaller size then test array
        CSL_Cache* cache = CSL_CacheAlloc(pal, 
                                          CACHE_NAME, 
                                          FIND_AND_PROTECT_TEST_CACHE_MAX_COUNT, 
                                          FIND_AND_PROTECT_TEST_CACHE_MAX_COUNT, 
                                          OBFUSCATE, 
                                          sizeof(OBFUSCATE), 
                                          NULL,
                                          TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;
            byte* entry = NULL;
            size_t entrySize = 0;

            // Add 5 entries.
            for (i = 0; i < 5; ++i)
            {
                // Add entry. Don't set the protect flag.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], FIND_AND_PROTECT_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // For the new 5 entries
            for (i = 0; i < 5; ++i)
            {
                // Find the entry and protect it.
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, TRUE, FALSE, FALSE);

                // Verify result
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, FIND_AND_PROTECT_TEST_ENTRY_SIZE);
                VerifyTestEntry(entry, FIND_AND_PROTECT_TEST_ENTRY_SIZE, i);
            }

            // Try to add 5 more entries.
            for (i = 5; i < 10; ++i)
            {
                // This should fail since all entries are protected
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], FIND_AND_PROTECT_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_RES);
            }

            // Validate again that we still have the same entries
            for (i = 0; i < 5; ++i)
            {
                // Find the entry. It is already protected.
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // Verify result
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, FIND_AND_PROTECT_TEST_ENTRY_SIZE);
                VerifyTestEntry(entry, FIND_AND_PROTECT_TEST_ENTRY_SIZE, i);
            }

            // Clear all protected flags
            CSL_CacheClearProtect(cache);

            // Add 5 more entries.
            for (i = 10; i < 15; ++i)
            {
                // Add entry. Don't protect them.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], FIND_AND_PROTECT_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through all entries
            for (i = 0; i < FIND_AND_PROTECT_TEST_ARRAY_COUNT; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should now have entries with index 10 - 14 in the cache
                if ((i >= 10) && (i <= 14))
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, FIND_AND_PROTECT_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, FIND_AND_PROTECT_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, FIND_AND_PROTECT_TEST_ARRAY_COUNT);
}

/*! Test replacing an existing item

    @return None, assert on failures.
*/
void 
TestCacheReplaceEntry(void)
{
    PAL_Instance* pal = NULL;

    #define REPLACE_ARRAY_COUNT 5
    #define REPLACE_SMALL_SIZE  500
    #define REPLACE_LARGE_SIZE  5000

    // Make two arrays. One with larger entries
    byte* smallArray[REPLACE_ARRAY_COUNT] = {0};
    byte* largeArray[REPLACE_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[REPLACE_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35};

    // Create both arrays
    CU_ASSERT(AllocateTestData(smallArray, REPLACE_SMALL_SIZE, REPLACE_ARRAY_COUNT));
    CU_ASSERT(AllocateTestData(largeArray, REPLACE_LARGE_SIZE, REPLACE_ARRAY_COUNT));

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        // Create cache 
        CSL_Cache* cache = CSL_CacheAlloc(pal, 
                                          CACHE_NAME, 
                                          REPLACE_ARRAY_COUNT, 
                                          REPLACE_ARRAY_COUNT, 
                                          OBFUSCATE, 
                                          sizeof(OBFUSCATE), 
                                          NULL,
                                          TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;
            byte* entry = NULL;
            size_t entrySize = 0;

            // Add 5 small entries.
            for (i = 0; i < 5; ++i)
            {
                // Add entry.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, smallArray[i], REPLACE_SMALL_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            for (i = 0; i < 5; ++i)
            {
                // Replace the entry with a larger entry
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, largeArray[i], REPLACE_LARGE_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            for (i = 0; i < 5; ++i)
            {
                // Replace with small entry again
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, smallArray[i], REPLACE_SMALL_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            for (i = 0; i < 5; ++i)
            {
                // Replace with large entry again
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, largeArray[i], REPLACE_LARGE_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Validate entries
            for (i = 0; i < 5; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // Verify result
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, REPLACE_LARGE_SIZE);
                VerifyTestEntry(entry, REPLACE_LARGE_SIZE, i);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(smallArray, REPLACE_ARRAY_COUNT);
    FreeTestData(largeArray, REPLACE_ARRAY_COUNT);
}

/*! Test removing some items from the cache.

    @return None, assert on failures.
*/
void 
TestCacheRemove(void)
{
    PAL_Instance* pal = NULL;

    // Array to hold the pointers for the test entries
    byte* testDataArray[CACHE_MAX_ITEMS_SMALL] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[CACHE_MAX_ITEMS_SMALL] = {0x00, 0x01, 0x02, 0x03, 0x04};

    if (!AllocateTestData(testDataArray, CACHE_ENTRY_SIZE_LARGE, CACHE_MAX_ITEMS_SMALL))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = CSL_CacheAlloc(pal, CACHE_NAME, CACHE_MAX_ITEMS_SMALL, CACHE_MAX_ITEMS_SMALL, OBFUSCATE, sizeof(OBFUSCATE), NULL, TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            NB_Error result = NE_OK;

            // Add entries, set the 'makeCopy' flag
            for (i = 0; i < CACHE_MAX_ITEMS_SMALL; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], CACHE_ENTRY_SIZE_LARGE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Now we can free the test data it is no longer needed.
            FreeTestData(testDataArray, CACHE_MAX_ITEMS_SMALL);

            // Remove one item before saving to disk
            result = CSL_CacheClearOne(cache, &cacheNames[4], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify it got removed
            result = CSL_CacheClearOne(cache, &cacheNames[4], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_NOENT);

            // Save them to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify that the files are there
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_ErrFileNotExist);


            // Delete two items
            result = CSL_CacheClearOne(cache, &cacheNames[1], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheClearOne(cache, &cacheNames[3], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Try to delete the same items again. This time they should return NE_NOENT
            result = CSL_CacheClearOne(cache, &cacheNames[1], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_NOENT);

            result = CSL_CacheClearOne(cache, &cacheNames[3], CACHE_NAME_LENGTH);
            CU_ASSERT_EQUAL(result, NE_NOENT);


            // Verify that the items got removed from the disk
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_ErrFileNotExist);


            // Get entries
            for (i = 0; i < CACHE_MAX_ITEMS_SMALL; ++i)
            {
                byte* entry = NULL;
                size_t entrySize = 0;

                // Get the entry
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // Check for removed items
                if ((i == 1) || (i == 3) || (i == 4))
                {
                    // This should return NE_NOENT.
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, CACHE_ENTRY_SIZE_LARGE);

                    // Verify data
                    VerifyTestEntry(entry, CACHE_ENTRY_SIZE_LARGE, i);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }
}

/*! Test save and reloading one accessed entry.

    @return None, assert on failures.
*/
void 
TestCacheSaveAccessed(void)
{
    #define SAVE_ACCESSED_TEST_ARRAY_COUNT       1
    #define SAVE_ACCESSED_TEST_CACHE_MAX_COUNT   10
    #define SAVE_ACCESSED_TEST_ENTRY_SIZE        1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[SAVE_ACCESSED_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[SAVE_ACCESSED_TEST_ARRAY_COUNT] = {0x31};

    if (!AllocateTestData(testDataArray, SAVE_ACCESSED_TEST_ENTRY_SIZE, SAVE_ACCESSED_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               SAVE_ACCESSED_TEST_CACHE_MAX_COUNT, 
                               SAVE_ACCESSED_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int32 savedEntries = 0;

            // Add entry
            result = CSL_CacheAdd(cache, &cacheNames[0], CACHE_NAME_LENGTH, testDataArray[0], SAVE_ACCESSED_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Find the entry, this will set the 'accessed' flag.
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify result
            CU_ASSERT_PTR_NOT_NULL(entry);
            CU_ASSERT_EQUAL(entrySize, SAVE_ACCESSED_TEST_ENTRY_SIZE);
            VerifyTestEntry(entry, SAVE_ACCESSED_TEST_ENTRY_SIZE, 0);

            // Free the copy that was returned
            nsl_free(entry);

            // Save the one accessed entry to disk
            result = CSL_CacheSaveAccessed(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 1);

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               SAVE_ACCESSED_TEST_CACHE_MAX_COUNT, 
                               SAVE_ACCESSED_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Try to load the entry from memory-only. This should fail
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, TRUE);
            CU_ASSERT_EQUAL(result, NE_NOENT);

            // Find the entry again. This time it should load it from the disk
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify result
            CU_ASSERT_PTR_NOT_NULL(entry);
            CU_ASSERT_EQUAL(entrySize, SAVE_ACCESSED_TEST_ENTRY_SIZE);
            VerifyTestEntry(entry, SAVE_ACCESSED_TEST_ENTRY_SIZE, 0);

            // Free the copy that was returned
            nsl_free(entry);

            // Find the entry again. This time it should load it from memory
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, TRUE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify result
            CU_ASSERT_PTR_NOT_NULL(entry);
            CU_ASSERT_EQUAL(entrySize, SAVE_ACCESSED_TEST_ENTRY_SIZE);
            VerifyTestEntry(entry, SAVE_ACCESSED_TEST_ENTRY_SIZE, 0);

            // Free the copy that was returned
            nsl_free(entry);

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, SAVE_ACCESSED_TEST_ARRAY_COUNT);
}

/*! Test save and reloading dirty (just added) entry.

    @return None, assert on failures.
*/
void 
TestCacheSaveDirty(void)
{
    #define SAVE_DIRTY_TEST_ARRAY_COUNT       1
    #define SAVE_DIRTY_TEST_CACHE_MAX_COUNT   10
    #define SAVE_DIRTY_TEST_ENTRY_SIZE        1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[SAVE_DIRTY_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[SAVE_DIRTY_TEST_ARRAY_COUNT] = {0x31};

    if (!AllocateTestData(testDataArray, SAVE_DIRTY_TEST_ENTRY_SIZE, SAVE_DIRTY_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               SAVE_DIRTY_TEST_CACHE_MAX_COUNT, 
                               SAVE_DIRTY_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int32 savedEntries = 0;

            // Add entry, this will set the 'dirty' flag.
            result = CSL_CacheAdd(cache, &cacheNames[0], CACHE_NAME_LENGTH, testDataArray[0], SAVE_DIRTY_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Save the one dirty entry to disk
            result = CSL_CacheSaveDirty(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 1);

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               SAVE_DIRTY_TEST_CACHE_MAX_COUNT, 
                               SAVE_DIRTY_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Try to load the entry from memory-only. This should fail
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, TRUE);
            CU_ASSERT_EQUAL(result, NE_NOENT);

            // Find the entry again. This time it should load it from the disk
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify result
            CU_ASSERT_PTR_NOT_NULL(entry);
            CU_ASSERT_EQUAL(entrySize, SAVE_DIRTY_TEST_ENTRY_SIZE);
            VerifyTestEntry(entry, SAVE_DIRTY_TEST_ENTRY_SIZE, 0);

            // Free the copy that was returned
            nsl_free(entry);

            // Find the entry again. This time it should load it from memory
            result = CSL_CacheFind(cache, &cacheNames[0], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, TRUE, TRUE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify result
            CU_ASSERT_PTR_NOT_NULL(entry);
            CU_ASSERT_EQUAL(entrySize, SAVE_DIRTY_TEST_ENTRY_SIZE);
            VerifyTestEntry(entry, SAVE_DIRTY_TEST_ENTRY_SIZE, 0);

            // Free the copy that was returned
            nsl_free(entry);

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, SAVE_DIRTY_TEST_ARRAY_COUNT);
}

/*! Test save entries to the persistent cache and replace them with new entries.

    This test also tests the scenario of the memory cache being larger than the persistent cache.

    @return None, assert on failures.
*/
void 
TestCacheSaveAndDrop(void)
{
    // Make the persistent cache smaller than the memory cache for this test.
    #define SAVE_AND_DROP_TEST_ARRAY_COUNT                  10
    #define SAVE_AND_DROP_TEST_MEMORY_CACHE_MAX_COUNT       10
    #define SAVE_AND_DROP_TEST_PERSISTENT_CACHE_MAX_COUNT   5
    #define SAVE_AND_DROP_TEST_ENTRY_SIZE                   1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;
    int i = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[SAVE_AND_DROP_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[SAVE_AND_DROP_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x41, 0x42, 0x43, 0x44, 0x45};

    if (!AllocateTestData(testDataArray, SAVE_AND_DROP_TEST_ENTRY_SIZE, SAVE_AND_DROP_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               SAVE_AND_DROP_TEST_MEMORY_CACHE_MAX_COUNT,
                               SAVE_AND_DROP_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int32 savedEntries = 0;

            // Add 5 entries
            for (i = 0; i < 5; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], SAVE_AND_DROP_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save the 5 entries to disk
            savedEntries = 0;
            result = CSL_CacheSaveDirty(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 5);

            // Add 5 more entries
            for (i = 5; i < 10; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], SAVE_AND_DROP_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save the next 5 entries to disk. This should replace the existing 5 entries in the persistent cache.
            savedEntries = 0;
            result = CSL_CacheSaveDirty(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 5);

            // Verify that all entries still exist in the memory cache. The memory cache is larger than the persistent cache for this test.
            for (i = 0; i < 10; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, TRUE);

                // Verify result
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, SAVE_AND_DROP_TEST_ENTRY_SIZE);
                VerifyTestEntry(entry, SAVE_AND_DROP_TEST_ENTRY_SIZE, i);
            }

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               SAVE_AND_DROP_TEST_MEMORY_CACHE_MAX_COUNT,
                               SAVE_AND_DROP_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Try to get the first 5 entries, they should not exist
            for (i = 0; i < 5; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_NOENT);
            }

            // Now get the next 5 entries, they should be loaded from the persistent cache
            for (i = 5; i < 10; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // Verify result
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, SAVE_AND_DROP_TEST_ENTRY_SIZE);
                VerifyTestEntry(entry, SAVE_AND_DROP_TEST_ENTRY_SIZE, i);
            }

            // These files should not exist
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_ErrFileNotExist);

            // These files should exist
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_Ok);

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, SAVE_AND_DROP_TEST_ARRAY_COUNT);
}

/*! Verify that any orphaned cache items are removed from the file system.

    @return None, assert on failures.
*/
void 
TestCacheRemoveOrphanedItems(void)
{
    // We make the persistent cache bigger than the memory cache for this test
    #define REMOVE_ORPHANED_TEST_ARRAY_COUNT                  20
    #define REMOVE_ORPHANED_TEST_MEMORY_CACHE_MAX_COUNT       20
    #define REMOVE_ORPHANED_TEST_PERSISTENT_CACHE_MAX_COUNT   20
    #define REMOVE_ORPHANED_TEST_ENTRY_SIZE                   1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    int i = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[REMOVE_ORPHANED_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[REMOVE_ORPHANED_TEST_ARRAY_COUNT] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
                                                         0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49};

    if (!AllocateTestData(testDataArray, REMOVE_ORPHANED_TEST_ENTRY_SIZE, REMOVE_ORPHANED_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               REMOVE_ORPHANED_TEST_MEMORY_CACHE_MAX_COUNT,
                               REMOVE_ORPHANED_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Add 20 entries
            for (i = 0; i < 20; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], REMOVE_ORPHANED_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save them to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Don't call CSL_CacheSaveIndex() it will leave the saved files orphaned.

            CSL_CacheDealloc(cache);
        }

        // Verify that the files are there
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000011"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000012"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000013"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000014"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000015"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000016"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000017"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000018"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000019"), PAL_Ok);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000020"), PAL_Ok);

        // Re-Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               REMOVE_ORPHANED_TEST_MEMORY_CACHE_MAX_COUNT,
                               REMOVE_ORPHANED_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Load the non-existing index file. 
            CSL_CacheLoadIndex(cache);

            // Remove orphaned files
            result = CSL_CacheValidateFilesOnDisk(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // All the files should be removed now
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000011"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000012"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000013"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000014"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000015"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000016"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000017"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000018"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000019"), PAL_ErrFileNotExist);
        CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000020"), PAL_ErrFileNotExist);

        PAL_Destroy(pal);
    }

    // Don't free test data, it is owned by the cache object.
}

/*! Test that the persistent/file cache keeps the most recent accessed items.

    This test also tests the scenario of the persistent cache being larger than the memory cache.

    @return None, assert on failures.
*/
void 
TestCachePersistentOrder(void)
{
    // We make the persistent cache bigger than the memory cache for this test
    #define PERSISTENT_ORDER_TEST_ARRAY_COUNT                  9
    #define PERSISTENT_ORDER_TEST_MEMORY_CACHE_MAX_COUNT       3
    #define PERSISTENT_ORDER_TEST_PERSISTENT_CACHE_MAX_COUNT   6
    #define PERSISTENT_ORDER_TEST_ENTRY_SIZE                   1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;
    int i = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[PERSISTENT_ORDER_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[PERSISTENT_ORDER_TEST_ARRAY_COUNT] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

    if (!AllocateTestData(testDataArray, PERSISTENT_ORDER_TEST_ENTRY_SIZE, PERSISTENT_ORDER_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               PERSISTENT_ORDER_TEST_MEMORY_CACHE_MAX_COUNT,
                               PERSISTENT_ORDER_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int32 savedEntries = 0;

            // Add 3 entries
            for (i = 0; i < 3; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PERSISTENT_ORDER_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save the 3 entries to disk
            savedEntries = 0;
            result = CSL_CacheSaveDirty(cache, 3, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 3);

            // Add 3 more entries. This should replace the entries in the memory cache
            for (i = 3; i < 6; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PERSISTENT_ORDER_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save the next 3 entries to disk. This should NOT replace the existing entries in the file cache.
            savedEntries = 0;
            result = CSL_CacheSaveDirty(cache, 3, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 3);

            // Verify that all entries can be loaded.
            for (i = 0; i < 6; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // Verify result
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, PERSISTENT_ORDER_TEST_ENTRY_SIZE);
                VerifyTestEntry(entry, PERSISTENT_ORDER_TEST_ENTRY_SIZE, i);
            }

            // Now access three of the items. This should move them to the top of the accessed list
            result = CSL_CacheFind(cache, &cacheNames[1], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheFind(cache, &cacheNames[3], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheFind(cache, &cacheNames[5], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Add 3 more entries. 
            for (i = 6; i < 9; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], PERSISTENT_ORDER_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save the next 3 entries to disk. This should replace 3 of the existing entries in the file cache but keep the last three accessed.
            savedEntries = 0;
            result = CSL_CacheSaveDirty(cache, 3, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 3);

            // For all test entries
            for (i = 0; i < 9; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // We should have the indices '1', '3', '5', '6' - '8'
                if ((i == 1) ||
                    (i == 3) ||
                    ((i >= 5) && (i <= 8)))
                {
                    // Verify result
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, PERSISTENT_ORDER_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, PERSISTENT_ORDER_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, PERSISTENT_ORDER_TEST_ARRAY_COUNT);
}


/*! Test notifications.

    @return None, assert on failures.
*/
void 
TestCacheNotifications(void)
{
    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               100,
                               100, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            byte buffer[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
            char name[] = "testEntry";

            // Set notification for remove
            void* dummyUserData = (void*)0xAABBCCDD;
            result = CSL_CacheSetNotificationFunctions(cache, &RemoveNotificationFunction, dummyUserData);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Add
            result = CSL_CacheAdd(cache, (byte*)name, sizeof(name), buffer, sizeof(buffer), FALSE, TRUE, 0);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Save to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Remove, this should trigger the notification callback
            result = CSL_CacheClearOne(cache, (byte*)name, sizeof(name));
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }
}


/*! Stress test for memory and persistent cache.

    @return None, assert on failures.
*/
void 
TestCacheStressTest(void)
{
    // We make the persistent cache bigger than the memory cache for this test
    #define STRESS_TEST_ARRAY_COUNT                  120
    #define STRESS_TEST_MEMORY_CACHE_MAX_COUNT       20
    #define STRESS_TEST_PERSISTENT_CACHE_MAX_COUNT   40
    #define STRESS_TEST_ENTRY_SIZE                   888

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[STRESS_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[STRESS_TEST_ARRAY_COUNT] = {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                                 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                                 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                                 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                                                 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                                                 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
                                                 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                                                 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
                                                 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
                                                 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
                                                 120, 121, 122, 123, 124, 125, 126, 127, 128, 129 };

    if (!AllocateTestData(testDataArray, STRESS_TEST_ENTRY_SIZE, STRESS_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               STRESS_TEST_MEMORY_CACHE_MAX_COUNT,
                               STRESS_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;
            int k = 0;
            int l = 0;

            // Add 40 items
            for (k = 0; k < 4; ++k)
            {
                for (l = 0; l < 10; ++l)
                {
                    i = k * 10 + l;

                    result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], STRESS_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

                result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Add 40 more items
            for (k = 4; k < 8; ++k)
            {
                for (l = 0; l < 10; ++l)
                {
                    i = k * 10 + l;

                    result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], STRESS_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                    CU_ASSERT_EQUAL(result, NE_OK);
                }

                result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Go through the last 40 items and access the odd numbers
            for (i = 40; i < 80; ++i)
            {
                if ((i % 2) == 1)
                {
                    // Find the entry. 
                    result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                    // Verify result
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, STRESS_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, STRESS_TEST_ENTRY_SIZE, i);
                }
            }

            // Add 20 more items. This should replace the 20 even entries in the persistent cache.
            for (i = 80; i < 100; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], STRESS_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify that we still have the 20 odd ones and the new ones
            for (i = 40; i < 100; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                // Are they the odd ones or the new ones
                if ((i >= 80) || ((i % 2) == 1))
                {
                    // Verify result
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, STRESS_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, STRESS_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            // Go through the original 20 odd ones again to keep them again.
            for (i = 40; i < 80; ++i)
            {
                if ((i % 2) == 1)
                {
                    // Find the entry. 
                    result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                    // Verify result
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, STRESS_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, STRESS_TEST_ENTRY_SIZE, i);
                }
            }

            // Add the last 20 items. That should keep the original odd ones again.
            for (i = 100; i < 120; ++i)
            {
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], STRESS_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify that we still have the 20 odd ones and the new ones
            for (i = 40; i < 120; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);

                if ((i >= 80) && (i < 100))
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
                else if ((i >= 100) || ((i % 2) == 1))
                {
                    // Verify result
                    CU_ASSERT_EQUAL(result, NE_OK);
                    CU_ASSERT_PTR_NOT_NULL(entry);
                    CU_ASSERT_EQUAL(entrySize, STRESS_TEST_ENTRY_SIZE);
                    VerifyTestEntry(entry, STRESS_TEST_ENTRY_SIZE, i);
                }
                else
                {
                    CU_ASSERT_EQUAL(result, NE_NOENT);
                }
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // Don't free data here since the data is owned (and destroyed) by the cache.
}

/*! Performance test for cache.

    @return None, assert on failures.
*/
void 
TestCachePerformance(void)
{
    // We make the persistent cache bigger than the memory cache for this test
    #define PERFORMANCE_TEST_ARRAY_COUNT                  5000
    #define PERFORMANCE_TEST_MEMORY_CACHE_MAX_COUNT       2500
    #define PERFORMANCE_TEST_PERSISTENT_CACHE_MAX_COUNT   5000
    #define PERFORMANCE_TEST_ENTRY_SIZE                   888

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;
    int i = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[PERFORMANCE_TEST_ARRAY_COUNT] = {0};

    // Fill array for cache names
    #define PERFORMANCE_TEST_CACHE_NAME_LENGTH 6
    char cacheNames[PERFORMANCE_TEST_ARRAY_COUNT][PERFORMANCE_TEST_CACHE_NAME_LENGTH];
    for (i = 0; i < PERFORMANCE_TEST_ARRAY_COUNT; ++i)
    {
        // Use 6-character string as cache name
        char name[10] = {0};
        nsl_sprintf(name, "%06d", i);
        nsl_strncpy(cacheNames[i], name, PERFORMANCE_TEST_CACHE_NAME_LENGTH);
    }

    if (!AllocateTestData(testDataArray, PERFORMANCE_TEST_ENTRY_SIZE, PERFORMANCE_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               PERFORMANCE_TEST_MEMORY_CACHE_MAX_COUNT,
                               PERFORMANCE_TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int i = 0;

            // Used for performance data
            PAL_ClockDateTime time1 = {0};
            PAL_ClockDateTime time2 = {0};
            PAL_ClockDateTime time3 = {0};
            PAL_ClockDateTime time4 = {0};
            PAL_ClockDateTime time5 = {0};
            PAL_ClockDateTime time6 = {0};
            PAL_ClockDateTime time7 = {0};
            PAL_ClockDateTime time8 = {0};
            PAL_ClockDateTime time9 = {0};
            PAL_ClockDateTime time10 = {0};
            PAL_ClockDateTime time11 = {0};
            PAL_ClockDateTime time12 = {0};
            PAL_ClockDateTime time13 = {0};
            PAL_ClockDateTime time14 = {0};

            PAL_ClockGetDateTime(&time1);

            // Add 2500 items
            for (i = 0; i < 2500; ++i)
            {
                result = CSL_CacheAdd(cache, (byte*)&cacheNames[i], PERFORMANCE_TEST_CACHE_NAME_LENGTH, testDataArray[i], PERFORMANCE_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            PAL_ClockGetDateTime(&time2);
            PAL_ClockGetDateTime(&time3);

            // Save all 2500 entries to the persistent cache
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            PAL_ClockGetDateTime(&time4);
            PAL_ClockGetDateTime(&time5);

            // Add 2500 more entries
            for (i = 2500; i < 5000; ++i)
            {
                result = CSL_CacheAdd(cache, (byte*)&cacheNames[i], PERFORMANCE_TEST_CACHE_NAME_LENGTH, testDataArray[i], PERFORMANCE_TEST_ENTRY_SIZE, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            PAL_ClockGetDateTime(&time6);
            PAL_ClockGetDateTime(&time7);

            // Save another 2500 entries to persistent cache
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            PAL_ClockGetDateTime(&time8);
            PAL_ClockGetDateTime(&time9);

            // Access the 2500 elements still in memory cache
            for (i = 2500; i < 5000; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, (byte*)&cacheNames[i], PERFORMANCE_TEST_CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Don't verify actual returned data to not throw off the performance measurement
            }

            PAL_ClockGetDateTime(&time10);
            PAL_ClockGetDateTime(&time11);

            // Access the 2500 elements from the persistent cache
            for (i = 0; i < 2500; ++i)
            {
                // Find the entry. 
                result = CSL_CacheFind(cache, (byte*)&cacheNames[i], PERFORMANCE_TEST_CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);

                // Don't verify actual returned data to not throw off the performance measurement
            }

            PAL_ClockGetDateTime(&time12);
            PAL_ClockGetDateTime(&time13);

            // Remove the 2500 entries (one by one)
            for (i = 0; i < 2500; ++i)
            {
                result = CSL_CacheClearOne(cache, (byte*)&cacheNames[i], PERFORMANCE_TEST_CACHE_NAME_LENGTH);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            PAL_ClockGetDateTime(&time14);

            //// Output performance data.
            //printf("\r\nPerformance:\r\nAdd:\t%d ms\r\n",   GetElapsedMilliseconds(&time1, &time2));
            //printf("Save to disc:\t%d ms\r\n",              GetElapsedMilliseconds(&time3, &time4));
            //printf("Add :\t%d ms\r\n",                      GetElapsedMilliseconds(&time5, &time6));
            //printf("Save to disc:\t%d ms\r\n",              GetElapsedMilliseconds(&time7, &time8));
            //printf("Access in memory:\t%d ms\r\n",          GetElapsedMilliseconds(&time9, &time10));
            //printf("Access on disc:\t%d ms\r\n",            GetElapsedMilliseconds(&time11,&time12));
            //printf("Clear:\t%d ms\r\n",                     GetElapsedMilliseconds(&time13,&time14));

            /*
                Output on Win32 machine using old linear cache implementation (release build):
            */

            // Add:    47 ms
            // Save to disc:   9360 ms
            // Add :   2578 ms
            // Save to disc:   9313 ms
            // Access in memory:       172 ms
            // Access on disc: 6266 ms
            // Clear:  609 ms

            /*
                Output on Win32 machine using new hash table implementation (release build)

                As you can see the saving to disk got slightly slower. This is due to the fact that we now have a cross-linked list
                for the priority calculation. This slows down saving to disk, BUT the regular find and add to cache functionality has
                improved by about a factor of 1000 (Add 2578 -> 16 and 
            */

            // Add:    0 ms
            // Save to disc:   12906 ms
            // Add :   16 ms
            // Save to disc:   13641 ms
            // Access in memory:       0 ms
            // Access on disc: 3468 ms
            // Clear:  547 ms


            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // Don't free data here since the data is owned (and destroyed) by the cache.
}

/*! Test resizing of cache.

    @return None, assert on failures.
*/
void 
TestCacheResizeCache(void)
{
    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    int i = 0;

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               1000,
                               1000, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);

        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Fill up cache
            for (i = 0; i < 1000; ++i)
            {
                char entryName[128] = {0};
                byte* pBuffer = nsl_malloc(1000);

                nsl_sprintf(entryName, "%d_name", i);

                result = CSL_CacheAdd(cache, (byte*)entryName, nsl_strlen(entryName), pBuffer, 1000, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save all cache entries to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-alloc smaller cache, this should remove the old exceeding entries.
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               100,
                               100, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);

        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // This should throw out exceeding items.
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-alloc large cache again.
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               1000,
                               1000, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);

        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Fill up cache again, with new entries
            for (i = 0; i < 1000; ++i)
            {
                char entryName[128] = {0};
                byte* pBuffer = nsl_malloc(1000);

                // new name, to not coflict with old entries
                nsl_sprintf(entryName, "%d_new_name", i);

                result = CSL_CacheAdd(cache, (byte*)entryName, nsl_strlen(entryName), pBuffer, 1000, FALSE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-alloc large cache again.
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               1000,
                               1000, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);

        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            for (i = 0; i < 1000; ++i)
            {
                char entryName[128] = {0};
                byte* pBuffer = NULL;
                size_t bufferLength = 0;

                // new name
                nsl_sprintf(entryName, "%d_new_name", i);

                // Access item, they should all exist
                result = CSL_CacheFind(cache, (byte*)entryName, nsl_strlen(entryName), &pBuffer, &bufferLength, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }
}

/*! Test using non-obfuscated cache functionality.

    @return None, assert on failures.
*/
void 
TestCacheNonObfuscatedFiles(void)
{
    #define NON_OBFUSCATED_TEST_ARRAY_COUNT       10
    #define NON_OBFUSCATED_TEST_CACHE_MAX_COUNT   100
    #define NON_OBFUSCATED_TEST_ENTRY_SIZE        100

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[NON_OBFUSCATED_TEST_ARRAY_COUNT] = {0};

    // Names for cache entries. They will be used as filenames to store the cache entries
    const char* name1 = "testName100.bin";
    const char* name2 = "anotherTestName100.bin";
    const char* name3 = "testName200.bin";
    const char* name4 = "_test_.bin";
    const char* name5 = "A";
    const char* name6 = "file1";
    const char* name7 = "file2";
    const char* name8 = "file3";
    const char* name9 = "file4";
    const char* name10 = "file5";

    const char* nameArray[NON_OBFUSCATED_TEST_ARRAY_COUNT] = {0};
    nameArray[0] = name1;
    nameArray[1] = name2;
    nameArray[2] = name3;
    nameArray[3] = name4;
    nameArray[4] = name5;
    nameArray[5] = name6;
    nameArray[6] = name7;
    nameArray[7] = name8;
    nameArray[8] = name9;
    nameArray[9] = name10;

    if (!AllocateTestData(testDataArray, NON_OBFUSCATED_TEST_ENTRY_SIZE, NON_OBFUSCATED_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache. Don't obfuscate entries.
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               NON_OBFUSCATED_TEST_CACHE_MAX_COUNT, 
                               NON_OBFUSCATED_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               FALSE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            char filename[128] = {0};
            int32 savedEntries = 0;
            int i = 0;

            /*
                Add first 5 entries
            */
            for (i = 0; i < 5; ++i)
            {
                result = CSL_CacheAdd(cache, (byte*)nameArray[i], nsl_strlen(nameArray[i]), testDataArray[i], NON_OBFUSCATED_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            /*
                Save the 5 entries to the persistent cache
            */
            result = CSL_CacheSaveDirty(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 5);

            /*
                Validate that the first 5 files exist (non obfuscated)
            */
            for (i = 0; i < 5; ++i)
            {
                nsl_sprintf(filename, "%s/%s", CACHE_NAME, nameArray[i]);
                CU_ASSERT_EQUAL(PAL_FileExists(pal, filename), PAL_Ok);
            }

            /*
                Validate that the last 5 files don't exist
            */
            for (i = 5; i < 10; ++i)
            {
                nsl_sprintf(filename, "%s/%s", CACHE_NAME, nameArray[i]);
                CU_ASSERT_EQUAL(PAL_FileExists(pal, filename), PAL_ErrFileNotExist);
            }

            /*
                Add next 5 entries
            */
            for (i = 5; i < 10; ++i)
            {
                result = CSL_CacheAdd(cache, (byte*)nameArray[i], nsl_strlen(nameArray[i]), testDataArray[i], NON_OBFUSCATED_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            /*
                Save the next 5 entries to the persistent cache
            */
            result = CSL_CacheSaveDirty(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, 5);

            /*
                Now all 10 entries should be on the file system
            */
            for (i = 0; i < 10; ++i)
            {
                nsl_sprintf(filename, "%s/%s", CACHE_NAME, nameArray[i]);
                CU_ASSERT_EQUAL(PAL_FileExists(pal, filename), PAL_Ok);
            }

            /*
                Verify that we can retrieve all the entries correctly
            */
            for (i = 0; i < 10; ++i)
            {
                // Get the entry
                result = CSL_CacheFind(cache, (byte*)nameArray[i], nsl_strlen(nameArray[i]), &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
                CU_ASSERT_PTR_NOT_NULL(entry);
                CU_ASSERT_EQUAL(entrySize, NON_OBFUSCATED_TEST_ENTRY_SIZE);

                // Verify data
                VerifyTestEntry(entry, NON_OBFUSCATED_TEST_ENTRY_SIZE, i);
            }

            // Remove all entries from the file system
            result = CSL_CacheClear(cache);

            // Verify they are removed from the cache
            for (i = 0; i < 10; ++i)
            {
                result = CSL_CacheFind(cache, (byte*)nameArray[i], nsl_strlen(nameArray[i]), &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_NOENT);
            }

            // Verify that they are removed from the file system as well
            for (i = 0; i < 10; ++i)
            {
                nsl_sprintf(filename, "%s/%s", CACHE_NAME, nameArray[i]);
                CU_ASSERT_EQUAL(PAL_FileExists(pal, filename), PAL_ErrFileNotExist);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, NON_OBFUSCATED_TEST_ARRAY_COUNT);
}

/*! Test using non-obfuscated cache functionality.

    Test saving the index file and reloading it.

    @return None, assert on failures.
*/
void 
TestCacheNonObfuscatedFilesReload(void)
{
    #define NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT       10
    #define NON_OBFUSCATED_RELOAD_TEST_CACHE_MAX_COUNT   100
    #define NON_OBFUSCATED_RELOAD_TEST_ENTRY_SIZE        100

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    byte* entry = NULL;
    size_t entrySize = 0;
    int i = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT] = {0};

    // Names for cache entries. They will be used as filenames to store the cache entries
    const char* name1 = "testName100.bin";
    const char* name2 = "anotherTestName100.bin";
    const char* name3 = "testName200.bin";
    const char* name4 = "_test_.bin";
    const char* name5 = "A";
    const char* name6 = "file1";
    const char* name7 = "file2";
    const char* name8 = "file3";
    const char* name9 = "file4";
    const char* name10 = "file5";

    const char* nameArray[NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT] = {0};
    nameArray[0] = name1;
    nameArray[1] = name2;
    nameArray[2] = name3;
    nameArray[3] = name4;
    nameArray[4] = name5;
    nameArray[5] = name6;
    nameArray[6] = name7;
    nameArray[7] = name8;
    nameArray[8] = name9;
    nameArray[9] = name10;

    if (!AllocateTestData(testDataArray, NON_OBFUSCATED_RELOAD_TEST_ENTRY_SIZE, NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               NON_OBFUSCATED_RELOAD_TEST_CACHE_MAX_COUNT, 
                               NON_OBFUSCATED_RELOAD_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               FALSE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            int32 savedEntries = 0;

            // Add all entries to the cache
            for (i = 0; i < NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT; ++i)
            {
                result = CSL_CacheAdd(cache, (byte*)nameArray[i], nsl_strlen(nameArray[i]), testDataArray[i], NON_OBFUSCATED_RELOAD_TEST_ENTRY_SIZE, FALSE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save to disk
            result = CSL_CacheSaveDirty(cache, -1, &savedEntries, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);
            CU_ASSERT_EQUAL(savedEntries, NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT);

            // Save index file
            result = CSL_CacheSaveIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            CSL_CacheDealloc(cache);
        }

        // Re-Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               NON_OBFUSCATED_RELOAD_TEST_CACHE_MAX_COUNT, 
                               NON_OBFUSCATED_RELOAD_TEST_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               FALSE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Load index file
            result = CSL_CacheLoadIndex(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Load all entries from the persistent cache
            for (i = 0; i < NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT; ++i)
            {
                result = CSL_CacheFind(cache, (byte*)nameArray[i], nsl_strlen(nameArray[i]), &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, NON_OBFUSCATED_RELOAD_TEST_ARRAY_COUNT);
}

/*! Test clearing all memory and persistent cache elements.

    @return None, assert on failures.
*/
void 
TestCacheClearAll(void)
{
    #define _TEST_ARRAY_COUNT                  10
    #define _TEST_MEMORY_CACHE_MAX_COUNT       20
    #define _TEST_PERSISTENT_CACHE_MAX_COUNT   20
    #define _TEST_ENTRY_SIZE                   1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    int i = 0;
    byte* entry = NULL;
    size_t entrySize = 0;

    // For statistics
    int maximumItemsMemory = 0;
    int maximumItemsPersistent = 0;
    int dirtyItems = 0;
    int cachedMemoryItems = 0;
    int cachedPersistentItems = 0;
    int protectedItems = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[_TEST_ARRAY_COUNT] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

    if (!AllocateTestData(testDataArray, _TEST_ENTRY_SIZE, _TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               _TEST_MEMORY_CACHE_MAX_COUNT,
                               _TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Add 10 entries
            for (i = 0; i < 10; ++i)
            {
                // Add entry and protect it. The clear function should still remove it.
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], _TEST_ENTRY_SIZE, TRUE, FALSE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Check stats
            CSL_CacheGetStats(cache, 
                              &maximumItemsMemory, 
                              &maximumItemsPersistent, 
                              &dirtyItems, 
                              &cachedMemoryItems, 
                              &cachedPersistentItems, 
                              &protectedItems);

            CU_ASSERT_EQUAL(maximumItemsMemory,         _TEST_MEMORY_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(maximumItemsPersistent,     _TEST_PERSISTENT_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(dirtyItems,                 10);
            CU_ASSERT_EQUAL(cachedMemoryItems,          10);
            CU_ASSERT_EQUAL(cachedPersistentItems,      0);
            CU_ASSERT_EQUAL(protectedItems,             10);

            // Save them to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify that the files are there
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_Ok);

            // Check stats
            CSL_CacheGetStats(cache, 
                              &maximumItemsMemory, 
                              &maximumItemsPersistent, 
                              &dirtyItems, 
                              &cachedMemoryItems, 
                              &cachedPersistentItems, 
                              &protectedItems);

            CU_ASSERT_EQUAL(maximumItemsMemory,         _TEST_MEMORY_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(maximumItemsPersistent,     _TEST_PERSISTENT_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(dirtyItems,                 0);
            CU_ASSERT_EQUAL(cachedMemoryItems,          10);
            CU_ASSERT_EQUAL(cachedPersistentItems,      10);
            CU_ASSERT_EQUAL(protectedItems,             10);

            // Clear memory and persistent entries (even if they are protected)
            result = CSL_CacheClear(cache);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Try to get the 10 entries. This should fail
            for (i = 0; i < 10; ++i)
            {
                // Get the entry. This should fail
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_NOENT);
            }

            // Verify that the files are gone
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_ErrFileNotExist);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_ErrFileNotExist);

            // Check stats
            CSL_CacheGetStats(cache, 
                              &maximumItemsMemory, 
                              &maximumItemsPersistent, 
                              &dirtyItems, 
                              &cachedMemoryItems, 
                              &cachedPersistentItems, 
                              &protectedItems);

            CU_ASSERT_EQUAL(maximumItemsMemory,         _TEST_MEMORY_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(maximumItemsPersistent,     _TEST_PERSISTENT_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(dirtyItems,                 0);
            CU_ASSERT_EQUAL(cachedMemoryItems,          0);
            CU_ASSERT_EQUAL(cachedPersistentItems,      0);
            CU_ASSERT_EQUAL(protectedItems,             0);

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // Don't free test data, it is owned by the cache object.
}

/*! Test clearing all memory elements but keeping the persistent entries.

    @return None, assert on failures.
*/
void 
TestCacheClearMemoryOnly(void)
{
    #define _TEST_ARRAY_COUNT                  10
    #define _TEST_MEMORY_CACHE_MAX_COUNT       20
    #define _TEST_PERSISTENT_CACHE_MAX_COUNT   20
    #define _TEST_ENTRY_SIZE                   1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    int i = 0;
    byte* entry = NULL;
    size_t entrySize = 0;

    // For statistics
    int maximumItemsMemory = 0;
    int maximumItemsPersistent = 0;
    int dirtyItems = 0;
    int cachedMemoryItems = 0;
    int cachedPersistentItems = 0;
    int protectedItems = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[_TEST_ARRAY_COUNT] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

    if (!AllocateTestData(testDataArray, _TEST_ENTRY_SIZE, _TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               _TEST_MEMORY_CACHE_MAX_COUNT,
                               _TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Add 10 entries
            for (i = 0; i < 10; ++i)
            {
                // Add entry and protect it. The clear function should still remove it (in this test)
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], _TEST_ENTRY_SIZE, TRUE, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save them to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            /*
                Clear memory entries (even if they are protected) but keep the persistent entries.
                (See TestCacheClearCheckProtected() to keep protected entries)
            */
            result = CSL_CacheClearExtended(cache, CSL_CF_MemoryOnly);
            CU_ASSERT_EQUAL(result, NE_OK);

            // Verify that the files still exist
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_Ok);

            // Check stats
            CSL_CacheGetStats(cache, 
                              &maximumItemsMemory, 
                              &maximumItemsPersistent, 
                              &dirtyItems, 
                              &cachedMemoryItems, 
                              &cachedPersistentItems, 
                              &protectedItems);

            CU_ASSERT_EQUAL(maximumItemsMemory,         _TEST_MEMORY_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(maximumItemsPersistent,     _TEST_PERSISTENT_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(dirtyItems,                 0);
            CU_ASSERT_EQUAL(cachedMemoryItems,          0);
            CU_ASSERT_EQUAL(cachedPersistentItems,      10);
            CU_ASSERT_EQUAL(protectedItems,             0);

            for (i = 0; i < 10; ++i)
            {
                // Try to get the entry from memory-only. This should fail
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, TRUE);
                CU_ASSERT_EQUAL(result, NE_NOENT);

                // Now try to get it from the persistent cache. This should succeed
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, _TEST_ARRAY_COUNT);
}

/*! Test clearing the cache, but keeping the currently protected/locked entries.

    @return None, assert on failures.
*/
void 
TestCacheClearCheckProtected(void)
{
    #define _TEST_ARRAY_COUNT                  10
    #define _TEST_MEMORY_CACHE_MAX_COUNT       20
    #define _TEST_PERSISTENT_CACHE_MAX_COUNT   20
    #define _TEST_ENTRY_SIZE                   1000

    PAL_Instance* pal = NULL;
    NB_Error result = NE_OK;
    int i = 0;
    byte* entry = NULL;
    size_t entrySize = 0;

    // For statistics
    int maximumItemsMemory = 0;
    int maximumItemsPersistent = 0;
    int dirtyItems = 0;
    int cachedMemoryItems = 0;
    int cachedPersistentItems = 0;
    int protectedItems = 0;

    // Array to hold the pointers for the test entries
    byte* testDataArray[_TEST_ARRAY_COUNT] = {0};

    const size_t CACHE_NAME_LENGTH = 1;
    byte cacheNames[_TEST_ARRAY_COUNT] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

    if (!AllocateTestData(testDataArray, _TEST_ENTRY_SIZE, _TEST_ARRAY_COUNT))
    {
        CU_ASSERT(FALSE);
        return;
    }

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    if (pal)
    {
        CSL_Cache* cache = NULL;

        CleanupTestFiles(pal);

        // Create cache 
        cache = CSL_CacheAlloc(pal, 
                               CACHE_NAME, 
                               _TEST_MEMORY_CACHE_MAX_COUNT,
                               _TEST_PERSISTENT_CACHE_MAX_COUNT, 
                               OBFUSCATE, 
                               sizeof(OBFUSCATE), 
                               NULL,
                               TRUE);
        CU_ASSERT_PTR_NOT_NULL(cache);
        if (cache)
        {
            // Add 10 entries
            for (i = 0; i < 10; ++i)
            {
                // Add entry. Protect the even ones, don't protect the odd ones.
                nb_boolean even = (nb_boolean)((i % 2) == 0);
                result = CSL_CacheAdd(cache, &cacheNames[i], CACHE_NAME_LENGTH, testDataArray[i], _TEST_ENTRY_SIZE, even, TRUE, 0);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            // Save them to disk
            result = CSL_CacheSaveDirty(cache, -1, NULL, FALSE);
            CU_ASSERT_EQUAL(result, NE_OK);

            /*
                Clear memory entries, keep the protected ones
            */
            result = CSL_CacheClearExtended(cache, CSL_CF_UnprotectedOnly);
            CU_ASSERT_EQUAL(result, NE_OK);

            // The persistent cache items should still exist
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000001"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000002"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000003"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000004"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000005"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000006"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000007"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000008"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000009"), PAL_Ok);
            CU_ASSERT_EQUAL(PAL_FileExists(pal, "TESTCACHE/00000010"), PAL_Ok);

            // Check stats
            CSL_CacheGetStats(cache, 
                              &maximumItemsMemory, 
                              &maximumItemsPersistent, 
                              &dirtyItems, 
                              &cachedMemoryItems, 
                              &cachedPersistentItems, 
                              &protectedItems);

            // We should have 5 protected memory items left
            CU_ASSERT_EQUAL(maximumItemsMemory,         _TEST_MEMORY_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(maximumItemsPersistent,     _TEST_PERSISTENT_CACHE_MAX_COUNT);
            CU_ASSERT_EQUAL(dirtyItems,                 0);
            CU_ASSERT_EQUAL(cachedMemoryItems,          5);
            CU_ASSERT_EQUAL(cachedPersistentItems,      10);
            CU_ASSERT_EQUAL(protectedItems,             5);

            // Check memory cache entries
            for (i = 0; i < 10; ++i)
            {
                nb_boolean even = FALSE;

                // Try to get the entry from the momory cache only!
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, TRUE);

                // The even ones should exist but not the odd ones
                even = (nb_boolean)((i % 2) == 0);
                CU_ASSERT_EQUAL(result, even ? NE_OK : NE_NOENT);
            }

            // Check persistent cache entries
            for (i = 0; i < 10; ++i)
            {
                // Now try to get all items from the memory OR persistent cache. We should get all of them.
                result = CSL_CacheFind(cache, &cacheNames[i], CACHE_NAME_LENGTH, &entry, &entrySize, FALSE, FALSE, FALSE);
                CU_ASSERT_EQUAL(result, NE_OK);
            }

            CSL_CacheDealloc(cache);
        }

        PAL_Destroy(pal);
    }

    // This test makes copies of the test data, we therefore have to free the data here.
    FreeTestData(testDataArray, _TEST_ARRAY_COUNT);
}

/*! Allocate array of cache test entries.

    The array is not allocated as one big memory block but as individual entries. This is 
    necessary so that the entries can be passed to the cache and the cache can take ownership
    of the entries and delete them if necessary.

    The function fills the data with different pattern for each entry.

    @return 'TRUE' on success. 'FALSE' on failure.
    @see FreeTestData
*/
boolean
AllocateTestData(byte** data,       /*!< Pointer to array to hold all pointers for the entries */
                 int entrySize,     /*!< Size of each cache entry to allocate */
                 int entryCount     /*!< Number of entries to allocate in the array */
                 )
{
    // Allocate the entries as individual entries. See explanation in the function header.

    byte pattern = START_PATTERN;
    int index = 0;
    int dataIndex = 0; 

    // For all cache entries
    for (index = 0; index < entryCount; ++index)
    {
        data[index] = nsl_malloc(entrySize);
        if (!data[index])
        {
            return FALSE;
        }

        // Fill with pattern
        for (dataIndex = 0; dataIndex < entrySize; ++dataIndex)
        {
            data[index][dataIndex] = pattern;
        }

        // Go to next pattern for next entry
        ++pattern;
    }

    return TRUE;
}

/*! Free the array of test data allocated by AllocateTestData.

    Only free the test data if it is not owned by the cache object.

    @return None
    @see AllocateTestData
*/
void
FreeTestData(byte** data,           /*!< Pointer to array of pointers to the entries to free */
             int entryCount         /*!< Number of entries in the array */
             )
{
    // Free all entries
    int index = 0;
    for (index = 0; index < entryCount; ++index)
    {
        nsl_free(data[index]);
    }
}

/*! Verify the pattern of a test entry allocated by AllocateTestData().

    @return None, Assert on failure.
    @see AllocateTestData
*/
void
VerifyTestEntry(byte* entry,        /*!< Pointer to entry to verify */
                int entrySize,      /*!< Size of entry */
                int index           /*!< Index of array specified in AllocateTestData(). This is necessary to 
                                         determine the pattern to verify against */
                )
{
    byte pattern = START_PATTERN;
    int i = 0;
    int dataIndex = 0;

    CU_ASSERT_PTR_NOT_NULL(entry);
    if (!entry)
    {
        return;
    }

    // Determine pattern of entry based on index. See AllocateTestData() for filling with pattern
    for (i = 0; i < index; ++i)
    {
        pattern++;
    }

    // Verify that the pattern matches
    for (dataIndex = 0; dataIndex < entrySize; ++dataIndex)
    {
        CU_ASSERT_EQUAL(entry[dataIndex], pattern);
    }
}

/*! Cleanup any cache test data.

    @return None
*/
void 
CleanupTestFiles(PAL_Instance* pal)
{
    PAL_FileEnum* enumerator = NULL;
    PAL_FileEnumInfo info = {0};

    boolean dirExists = PAL_FileIsDirectory(pal, CACHE_NAME);
    if (!dirExists)
    {
        return;
    }

    if (PAL_FileEnumerateCreate(pal, CACHE_NAME, FALSE, &enumerator) == PAL_Ok)
    {
        while (PAL_FileEnumerateNext(enumerator, &info) == NE_OK)
        {
            char path[1024] = {0};
            nsl_snprintf(path, 1024, "%s/%s", CACHE_NAME, info.filename);

            PAL_FileRemove(pal, path);
        }

        PAL_FileEnumerateDestroy(enumerator);
    }

    PAL_FileRemoveDirectory(pal, CACHE_NAME, FALSE);
}

/*! Calculates the time in milliseconds between the first and second time.

    @return elapsed time in milliseconds
*/
int 
GetElapsedMilliseconds(const PAL_ClockDateTime* time1, const PAL_ClockDateTime* time2)
{
    // Don't consider day swap

    int result   = ((signed)(time2->hour) - (signed)(time1->hour)) * 60 * 60 * 1000;
    result      += ((signed)(time2->minute) - (signed)(time1->minute)) * 60 * 1000;
    result      += ((signed)(time2->second) - (signed)(time1->second)) * 1000;
    result      += ((signed)(time2->milliseconds) - (signed)(time1->milliseconds));

    return result;
}

/*! Remove notificatin.

    @see TestCacheNotifications
*/
void 
RemoveNotificationFunction(CSL_Cache* cache, 
                           const byte* name,
                           size_t namelen,
                           void* userData)
{
    char testName[] = "testEntry";
    void* testUserData = (void*)0xAABBCCDD;

    // Compare values to TestCacheNotifications
    CU_ASSERT_STRING_EQUAL((char*)name, testName);
    CU_ASSERT_EQUAL(namelen, sizeof(testName));
    CU_ASSERT_PTR_EQUAL(userData, testUserData);
}


/*! @} */


