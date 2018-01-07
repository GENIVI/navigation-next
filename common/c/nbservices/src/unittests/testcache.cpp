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

    @file       testcache.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
    #include "platformutil.h"
    #include "networkutil.h"
}

#include "CUnit.h"
#include "testcache.h"
#include "Cache.h"
#include "base.h"

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................

static const uint32 TEST_MAX_CACHING_DATA_COUNT = 2;

static const uint32 TEST_MAX_DATA_SIZE = 4096;

static const char TEST_CACHING_NAME[] = "TEST_CACHE";

static const uint32 TEST_DATA_COUNT = 4;

static const char* TEST_DATA_TYPE[] = {
    "test type0",
    "test type1",
    "test type2"
};

static const char* TEST_DATA_NAME[] = {
    "test name0",
    "test name1",
    "test name2"
};

static const char* TEST_DATA[] = {
    "1234567890",
    "0123456789",
    "6789012345"
};

static const char* TEST_ADDITIONAL_KEY[] = {
    "key0",
    "key1",
    "key2"
};

static const char* TEST_ADDITIONAL_VALUE[] = {
    "value0",
    "value1",
    "value2"
};


// Local Types ..................................................................................

/*! Collection of all instances used for the unit tests */
class TestCacheInstances : public Base
{
public:
    /* See definition for description */

    TestCacheInstances();
    virtual ~TestCacheInstances();
    NB_Error Initialize();
    void Reset();

    PAL_Instance* m_pal;    /*!< Pointer to PAL instance */
    NB_Context* m_context;  /*!< Pointer to current context */
    CachePtr m_cache;       /*!< A Cache object */
};


// Test Functions ...............................................................................

static void TestCacheInitialization(void);
static void TestCacheAppendData(void);
static void TestCacheGetData(void);
static void TestCacheRemoveData(void);
static void TestCacheProtectData(void);


// Helper functions .............................................................................

static bool TestCacheAppendDataByIndex(CachePtr cache,
                                       uint32 index,
                                       bool dataProtected);
CachingIndex TestCacheGetCachingIndex(uint32 index);


// Public Functions .............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestCache_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestCacheInitialization", &TestCacheInitialization);
    CU_add_test(pTestSuite, "TestCacheAppendData", &TestCacheAppendData);
    CU_add_test(pTestSuite, "TestCacheGetData", &TestCacheGetData);
    CU_add_test(pTestSuite, "TestCacheRemoveData", &TestCacheRemoveData);
    CU_add_test(pTestSuite, "TestCacheProtectData", &TestCacheProtectData);
};

/*! Add common initialization code here

    @return 0
*/
int
TestCache_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

    @return 0
*/
int
TestCache_SuiteCleanup()
{
    return 0;
}


// Test Functions ...............................................................................

/*! Test initializing a Cache object

    @return None. CUnit Asserts get called on failures.
*/
void
TestCacheInitialization(void)
{
    NB_Error error = NE_OK;
    TestCacheInstances testInstances;

    // Initialize a Cache object in the function TestCacheInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCacheInstances::Initialize() failed");
        return;
    }
}

/*! Test appending data

    @return None. CUnit Asserts get called on failures.
*/
void
TestCacheAppendData(void)
{
    NB_Error error = NE_OK;
    TestCacheInstances testInstances;
    CachePtr cache;

    // Initialize a Cache object in the function TestCacheInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCacheInstances::Initialize() failed");
        return;
    }

    cache = testInstances.m_cache;
    if (!cache)
    {
        CU_FAIL("Pointer to Cache object is NULL");
        return;
    }

    if (!TestCacheAppendDataByIndex(cache, 0, false))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }
}

/*! Test getting data

    @return None. CUnit Asserts get called on failures.
*/
void
TestCacheGetData(void)
{
    NB_Error error = NE_OK;
    uint32 bytesGot = 0;
    char buffer[TEST_MAX_DATA_SIZE] = {0};
    TestCacheInstances testInstances;
    CachePtr cache;

    // Initialize a Cache object in the function TestCacheInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCacheInstances::Initialize() failed");
        return;
    }

    cache = testInstances.m_cache;
    if (!cache)
    {
        CU_FAIL("Pointer to Cache object is NULL");
        return;
    }

    if (!TestCacheAppendDataByIndex(cache, 0, false))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }

    CachingIndex cachingIndex = TestCacheGetCachingIndex(0);

    // Get the data.
    nsl_memset(buffer, 0, TEST_MAX_DATA_SIZE);
    //@todo: Code should be updated to match the changed interface of cache.
    /*
    bytesGot = cache->GetData(cachingIndex.m_type,
                              cachingIndex.m_name,
                              (uint8*) buffer,
                              0,
                              TEST_MAX_DATA_SIZE);
    */
    CU_ASSERT(bytesGot == (nsl_strlen(TEST_DATA[0]) + 1));
    CU_ASSERT(nsl_strcmp(buffer, TEST_DATA[0]) == 0);
}

/*! Test removing data

    @return None. CUnit Asserts get called on failures.
*/
void
TestCacheRemoveData(void)
{
    NB_Error error = NE_OK;
    TestCacheInstances testInstances;
    CachePtr cache;

    // Initialize a Cache object in the function TestCacheInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCacheInstances::Initialize() failed");
        return;
    }

    cache = testInstances.m_cache;
    if (!cache)
    {
        CU_FAIL("Pointer to Cache object is NULL");
        return;
    }

    if (!TestCacheAppendDataByIndex(cache, 0, true))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }

    CachingIndex cachingIndex = TestCacheGetCachingIndex(0);

    // Remove the data. The data can be removed even if the protected flag is true.
    cache->RemoveData(cachingIndex.m_type, cachingIndex.m_name);
    CU_ASSERT(!(cache->IsItemExisting(cachingIndex.m_type, cachingIndex.m_name)));
}

/*! Test protecting data

    @return None. CUnit Asserts get called on failures.
*/
void
TestCacheProtectData(void)
{
    NB_Error error = NE_OK;
    TestCacheInstances testInstances;
    CachePtr cache;

    // Initialize a Cache object in the function TestCacheInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestCacheInstances::Initialize() failed");
        return;
    }

    cache = testInstances.m_cache;
    if (!cache)
    {
        CU_FAIL("Pointer to Cache object is NULL");
        return;
    }

    CachingIndex cachingIndex0 = TestCacheGetCachingIndex(0);
    CachingIndex cachingIndex1 = TestCacheGetCachingIndex(1);
    CachingIndex cachingIndex2 = TestCacheGetCachingIndex(2);

    if (!TestCacheAppendDataByIndex(cache, 0, false))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }

    if (!TestCacheAppendDataByIndex(cache, 1, false))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }

    if (!TestCacheAppendDataByIndex(cache, 2, false))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }

    /* There are only the data of index 1 and 2 existing. The data of index 0 is removed.
       Because the maximum count of the caching data is set to 2.
    */
    CU_ASSERT(!(cache->IsItemExisting(cachingIndex0.m_type, cachingIndex0.m_name)));
    CU_ASSERT(cache->IsItemExisting(cachingIndex1.m_type, cachingIndex1.m_name));
    CU_ASSERT(cache->IsItemExisting(cachingIndex2.m_type, cachingIndex2.m_name));
    
    //@todo: Code should be updated to match the changed interface of cache.
    //cache->SetItemProtected(cachingIndex1.m_type, cachingIndex1.m_name, true);

    if (!TestCacheAppendDataByIndex(cache, 0, true))
    {
        CU_FAIL("TestCacheAppendDataByIndex() failed");
        return;
    }

    /* There are only the data of index 0 and 1 existing. The data of index 2 is removed.
       Because the data of index 1 is protected.
    */
    CU_ASSERT(cache->IsItemExisting(cachingIndex0.m_type, cachingIndex0.m_name));
    CU_ASSERT(cache->IsItemExisting(cachingIndex1.m_type, cachingIndex1.m_name));
    CU_ASSERT(!(cache->IsItemExisting(cachingIndex2.m_type, cachingIndex2.m_name)));

    /* The data of index 2 cannot be added to the cache. Because the cache is full and
       all data are protected. User should remove all data in this case.
    */
    if (TestCacheAppendDataByIndex(cache, 2, true))
    {
        CU_FAIL("Appending data succeeds when the cache is full");
        return;
    }

    CU_ASSERT(cache->IsItemExisting(cachingIndex0.m_type, cachingIndex0.m_name));
    CU_ASSERT(cache->IsItemExisting(cachingIndex1.m_type, cachingIndex1.m_name));
    CU_ASSERT(!(cache->IsItemExisting(cachingIndex2.m_type, cachingIndex2.m_name)));
}


// Helper functions .............................................................................

/*! Append the data to the cache by index

    @return true if appending data successful, false otherwise.
*/
bool
TestCacheAppendDataByIndex(CachePtr cache,      /*!< A Cache object */
                           uint32 index,        /*!< Index of the data to append */
                           bool dataProtected   /*!< Is the data protected? */
                           )
{
    NB_Error error = NE_OK;

    if ((!cache) || (index >= TEST_DATA_COUNT))
    {
        CU_FAIL("Parameter cache or index is invalid");
        return false;
    }

    CachingIndex cachingIndex = TestCacheGetCachingIndex(index);
    shared_ptr<string> type = cachingIndex.m_type;
    shared_ptr<string> name = cachingIndex.m_name;

    shared_ptr<map<string, shared_ptr<string> > > additionalData(new map<string, shared_ptr<string> >());
    if (!additionalData)
    {
        CU_FAIL("Out of memory when allocated additional data");
        return false;
    }

    shared_ptr<string> value(new string(TEST_ADDITIONAL_VALUE[index]));
    if (!value)
    {
        CU_FAIL("Out of memory when allocated value1");
        return false;
    }

    additionalData->insert(pair<string, shared_ptr<string> >(TEST_ADDITIONAL_KEY[0], value));

    // Remove the old data and append the new data.
    //@todo: Code should be updated to match the changed interface of cache.
    /*
    cache->RemoveData(type, name);
    error = cache->AppendData(type,
                              name,
                              (const uint8*) TEST_DATA[index],
                              nsl_strlen(TEST_DATA[index]) + 1,
                              additionalData,
                              true);
    */
    if (error != NE_OK)
    {
        return false;
    }

    CU_ASSERT(cache->IsItemExisting(type, name));

    // The data added to cache is unprotected by default.
    //@todo: Code should be updated to match the changed interface of cache.
    /*
    if (dataProtected)
    {
        cache->SetItemProtected(type, name, dataProtected);
    }
    */
    CU_ASSERT_EQUAL(cache->IsItemProtected(type, name), dataProtected);

    return true;
}

/*! Get a caching index

    @return A caching index
*/
CachingIndex
TestCacheGetCachingIndex(uint32 index)
{
    shared_ptr<string> type(new string(TEST_DATA_TYPE[index]));
    if (!type)
    {
        CU_FAIL("Out of memory when allocated type");
        return CachingIndex();
    }

    shared_ptr<string> name(new string(TEST_DATA_NAME[index]));
    if (!name)
    {
        CU_FAIL("Out of memory when allocated name");
        return CachingIndex();
    }

    return CachingIndex(type, name);
}


// TestCache functions ..........................................................................

/*! TestCacheInstances constructor */
TestCacheInstances::TestCacheInstances() : m_pal(NULL),
                                           m_context(NULL),
                                           m_cache()
{
    // Nothing to do here.
}

/*! TestCacheInstances destructor */
TestCacheInstances::~TestCacheInstances()
{
    Reset();
}

/*! Initialize a TestCacheInstances object

    @return NE_OK if success,
            NE_EXIST if this object has already been initialized,
            NE_NOTINIT if this object is not properly initialized.
*/
NB_Error
TestCacheInstances::Initialize()
{
    NB_Error error = NE_OK;

    // Check if this TestCacheInstances object has already been initialized.
    if (m_pal && m_context && m_cache)
    {
        return NE_EXIST;
    }

    // Check if this TestOffboardTileInstances object is not properly initialized.
    if (m_pal || m_context || m_cache)
    {
        return NE_NOTINIT;
    }

    // Create a PAL and context.
    if (!CreatePalAndContext(&m_pal, &m_context))
    {
        Reset();
        return NE_NOTINIT;
    }
    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    // Create and initialize a Cache object.
    shared_ptr<string> cachingPath(new string(GetBasePath()));
    if (!cachingPath)
    {
        Reset();
        return NE_NOMEM;
    }

    shared_ptr<string> cachingName(new string(TEST_CACHING_NAME));
    if (!cachingName)
    {
        Reset();
        return NE_NOMEM;
    }

    m_cache.reset(new Cache());
    if (!m_cache)
    {
        Reset();
        return NE_NOMEM;
    }

    error = m_cache->Initialize(m_context,
                                TEST_MAX_CACHING_DATA_COUNT,
                                cachingName,
                                cachingPath);
    if (error != NE_OK)
    {
        Reset();
        return error;
    }

    return error;
}

/*! Reset a TestCacheInstances object to be uninitialized

    Destroy the PAL, context, and Cache object in the TestCacheInstances object.

    @return None
*/
void
TestCacheInstances::Reset()
{
    m_cache.reset();

    if (m_context)
    {
        NB_Error error = NE_OK;

        error = NB_ContextDestroy(m_context);
        CU_ASSERT_EQUAL(error, NE_OK);
        m_context = NULL;
    }

    if (m_pal)
    {
        PAL_Destroy(m_pal);
        m_pal = NULL;
    }
}

/*! @} */
