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
    
    @file     testhashtable.c
    @date     02/23/2011
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "testhashtable.h"
#include "cslhashtable.h"
#include "palstdlib.h"


// Local Constants ...............................................................................


// Local Functions ...............................................................................

// test functions
static void TestHashTableInsertFind(void);
static void TestHashTableInsertDelete(void);
static void TestHashTableCollision(void);
static void TestHashTableStressTest(void);

// helper functions
static void InsertAndVerify(CSL_HashTable* hashTable,
                            const char* key,
                            void* value);

static void DeleteAndVerify(CSL_HashTable* hashTable,
                            const char* key);

static void FindAndVerify(CSL_HashTable* hashTable,
                          const char* key,
                          void* expectedResult);

static void GetStats(CSL_HashTable* hashTable);


// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestHashTable_AddAllTests( CU_pSuite pTestSuite )
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestHashTableInsertFind",      &TestHashTableInsertFind);
    CU_add_test(pTestSuite, "TestHashTableInsertDelete",    &TestHashTableInsertDelete);
    CU_add_test(pTestSuite, "TestHashTableCollision",       &TestHashTableCollision);
    CU_add_test(pTestSuite, "TestHashTableStressTest",      &TestHashTableStressTest);
};


/*! Add common initialization code here.

    @return 0
*/
int
TestHashTable_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

    @return 0
*/
int 
TestHashTable_SuiteCleanup()
{
    return 0;
}


// Local Functions ...............................................................................

/*! Test simple insert/find.

    @return None, but assert on errors
*/
static void 
TestHashTableInsertFind(void)
{
    const char key[]    = "test";
    int value           = 100;
    int* pValue         = &value;

    CSL_HashTable* hashTable = CSL_HashTableCreate(100, NULL);
    CU_ASSERT_PTR_NOT_NULL(hashTable);

    // Insert
    InsertAndVerify(hashTable, key, pValue);

    // Find
    FindAndVerify(hashTable, key, pValue);

    CSL_HashTableDestroy(hashTable, FALSE);
}

/*! Test simple insert/delete.

    @return None, but assert on errors
*/
static void 
TestHashTableInsertDelete(void)
{
    const char key[]    = "test";
    int value           = 100;
    int* pValue         = &value;

    CSL_HashTable* hashTable = CSL_HashTableCreate(100, NULL);
    CU_ASSERT_PTR_NOT_NULL(hashTable);

    // Insert
    InsertAndVerify(hashTable, key, pValue);

    // Delete
    DeleteAndVerify(hashTable, key);

    CSL_HashTableDestroy(hashTable, FALSE);
}

/*! Test collisions for hash table entries.

    @return None, but assert on errors
*/
static void 
TestHashTableCollision(void)
{
    char key[256] = {0};
    int i = 0;

    int values[30] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};

    // Create hash table with a very small size
    CSL_HashTable* hashTable = CSL_HashTableCreate(10, NULL);
    CU_ASSERT_PTR_NOT_NULL(hashTable);

    /*
        We can actually add more entries as the given size. That will lead to collisions.
    */

    // Add 30 entries. This will lead to a lot of collisions.
    for (i = 0; i < 30; ++i)
    {
        // Just use the number as string for the key
        nsl_sprintf(key, "%d", i + 1);

        // Insert
        InsertAndVerify(hashTable, key, &values[i]);
    }

    // Now check all 30 entries
    for (i = 0; i < 30; ++i)
    {
        // Just use the number as string for the key
        nsl_sprintf(key, "%d", i + 1);

        // Find
        FindAndVerify(hashTable, key, &values[i]);
    }

    // Get stats, the collision count will be high
    GetStats(hashTable);

    // Now delete all even entries
    for (i = 0; i < 30; ++i)
    {
        if ((i % 2) == 0)
        {
            // Just use the number as string for the key
            nsl_sprintf(key, "%d", i + 1);

            // Delete
            DeleteAndVerify(hashTable, key);
        }
    }

    // Check all 30 entries. The even ones should exist, the uneven ones should be deleted.
    for (i = 0; i < 30; ++i)
    {
        // Just use the number as string for the key
        nsl_sprintf(key, "%d", i + 1);

        // Find, even entries should not exist. Odd ones should.
        FindAndVerify(hashTable, key, ((i % 2) == 0) ? NULL : &values[i]);
    }

    GetStats(hashTable);

    // Now delete the remaining odd entries
    for (i = 0; i < 30; ++i)
    {
        if ((i % 2) != 0)
        {
            // Just use the number as string for the key
            nsl_sprintf(key, "%d", i + 1);

            // Delete
            DeleteAndVerify(hashTable, key);
        }
    }

    // Try to find all entries, they should all be deleted.
    for (i = 0; i < 30; ++i)
    {
        // Just use the number as string for the key
        nsl_sprintf(key, "%d", i + 1);

        // Find should fail
        FindAndVerify(hashTable, key, NULL);
    }

    GetStats(hashTable);

    CSL_HashTableDestroy(hashTable, FALSE);
}

/*! Stress test.

    @return None, but assert on errors
*/
static void 
TestHashTableStressTest(void)
{
    /*
        We create a hash table with 1000 entries and add all 1000 entries. The collision rate should still be low.
    */

    #define TABLE_COUNT 1000

    // Array of pointers to all our values
    char* values[TABLE_COUNT] = {0};
    char key[100] = {0};
    uint32 i = 0;

    CSL_HashTable* hashTable = CSL_HashTableCreate(TABLE_COUNT, NULL);
    CU_ASSERT_PTR_NOT_NULL(hashTable);

    // Add all entries to the table
    for (i = 0; i < TABLE_COUNT; ++i)
    {
        // Create a value based on the index.
        char* pValue = nsl_malloc(100);
        nsl_memset(pValue, 0, 100);
        nsl_sprintf(pValue, "%04d %04d %04d %04d", i, i, i, i);

        // Use the index as key, this is not a very good key so we will have collisions.
        nsl_sprintf(key, "%d", i);

        // Insert
        InsertAndVerify(hashTable, key, pValue);

        // Save pointer in local array
        values[i] = pValue;
    }

    GetStats(hashTable);

    // Delete some random values
    nsl_sprintf(key, "%d",  50);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",   5);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",  78);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 987);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 234);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 999);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 123);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 256);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 333);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",  77);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",  93);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",   1);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 400);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 409);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 388);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 688);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 701);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 888);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d", 876);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",  23);  DeleteAndVerify(hashTable, key);
    nsl_sprintf(key, "%d",  65);  DeleteAndVerify(hashTable, key);

    GetStats(hashTable);

    // Verify that they are deleted
    nsl_sprintf(key, "%d",  50);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",   5);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",  78);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 987);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 234);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 999);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 123);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 256);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 333);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",  77);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",  93);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",   1);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 400);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 409);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 388);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 688);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 701);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 888);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d", 876);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",  23);  FindAndVerify(hashTable, key, NULL);
    nsl_sprintf(key, "%d",  65);  FindAndVerify(hashTable, key, NULL);

    GetStats(hashTable);

    // Re-insert the items
    nsl_sprintf(key, "%d",  50);  InsertAndVerify(hashTable, key, values[ 50]);
    nsl_sprintf(key, "%d",   5);  InsertAndVerify(hashTable, key, values[  5]);
    nsl_sprintf(key, "%d",  78);  InsertAndVerify(hashTable, key, values[ 78]);
    nsl_sprintf(key, "%d", 987);  InsertAndVerify(hashTable, key, values[987]);
    nsl_sprintf(key, "%d", 234);  InsertAndVerify(hashTable, key, values[234]);
    nsl_sprintf(key, "%d", 999);  InsertAndVerify(hashTable, key, values[999]);
    nsl_sprintf(key, "%d", 123);  InsertAndVerify(hashTable, key, values[123]);
    nsl_sprintf(key, "%d", 256);  InsertAndVerify(hashTable, key, values[256]);
    nsl_sprintf(key, "%d", 333);  InsertAndVerify(hashTable, key, values[333]);
    nsl_sprintf(key, "%d",  77);  InsertAndVerify(hashTable, key, values[ 77]);
    nsl_sprintf(key, "%d",  93);  InsertAndVerify(hashTable, key, values[ 93]);
    nsl_sprintf(key, "%d",   1);  InsertAndVerify(hashTable, key, values[  1]);
    nsl_sprintf(key, "%d", 400);  InsertAndVerify(hashTable, key, values[400]);
    nsl_sprintf(key, "%d", 409);  InsertAndVerify(hashTable, key, values[409]);
    nsl_sprintf(key, "%d", 388);  InsertAndVerify(hashTable, key, values[388]);
    nsl_sprintf(key, "%d", 688);  InsertAndVerify(hashTable, key, values[688]);
    nsl_sprintf(key, "%d", 701);  InsertAndVerify(hashTable, key, values[701]);
    nsl_sprintf(key, "%d", 888);  InsertAndVerify(hashTable, key, values[888]);
    nsl_sprintf(key, "%d", 876);  InsertAndVerify(hashTable, key, values[876]);
    nsl_sprintf(key, "%d",  23);  InsertAndVerify(hashTable, key, values[ 23]);
    nsl_sprintf(key, "%d",  65);  InsertAndVerify(hashTable, key, values[ 65]);

    GetStats(hashTable);

    // Verify that all entries are there
    for (i = 0; i < TABLE_COUNT; ++i)
    {
        nsl_sprintf(key, "%d", i);  
        FindAndVerify(hashTable, key, values[i]);
    }

    CSL_HashTableDestroy(hashTable, TRUE);
}


// Helper Functions ..................................................................................................

void 
InsertAndVerify(CSL_HashTable* hashTable,
                const char* key,
                void* value)
{
    nb_boolean result = CSL_HashTableInsert(hashTable, (const byte*)key, nsl_strlen(key), value);
    CU_ASSERT_EQUAL(result, TRUE);
}

void 
DeleteAndVerify(CSL_HashTable* hashTable,
                const char* key)
{
    nb_boolean result = CSL_HashTableDelete(hashTable, (const byte*)key, nsl_strlen(key));
    CU_ASSERT_EQUAL(result, TRUE);
}

void 
FindAndVerify(CSL_HashTable* hashTable,
              const char* key,
              void* expectedResult)
{
    void* pReturn = CSL_HashTableFind(hashTable, (const byte*)key, nsl_strlen(key));
    CU_ASSERT_PTR_EQUAL(pReturn, expectedResult);
}

void 
GetStats(CSL_HashTable* hashTable)
{
    uint32 totalTableSize   = 0;
    uint32 entryCount       = 0;
    uint32 collisionCount   = 0;
    double averageLookup    = 1.0;

    NB_Error result = CSL_HashTableGetStats(hashTable, &totalTableSize, &entryCount, &collisionCount, &averageLookup);

    //// test
    //printf("\r\nHash Table\r\n\tTotal size:\t%d\r\n", totalTableSize);
    //printf("\tEntry Count:\t%d\r\n", entryCount);
    //printf("\tCollision Count:\t%d\r\n", collisionCount);
    //printf("\tAverage Lookup:\t%1.2f\r\n", averageLookup);

    CU_ASSERT_EQUAL(result, NE_OK);
}


/*! @} */


