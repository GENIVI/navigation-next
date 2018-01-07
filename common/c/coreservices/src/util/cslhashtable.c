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

    @file     cslhashtable.c
    @date     02/23/2011
*/
/*
    See header file for description.

    (C) Copyright 2011 by TeleCommunication Systems

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*@{*/

#include "cslhashtable.h"
#include "palmath.h"
#include "vec.h"


// Local constants ...................................................................................................

/* 
    Since we use open addressing for colision resolution, we need to make sure that our load factor is below 0.7, 
    otherwise the hash table becomes very inefficient. E.g. a multiplier of '2' creates a maximum load factor of 0.5. Use
    a non-round number, i think that leads to better results. Dont' make the multiplier too big to not waste too much memory. 
    The best max-index would be a prime numbers, so if we find an easy way to calculate the nearest prime number then
    this might benefit keeping collisions down.
*/
const double SIZE_MULTIPLIER = 1.93;

// Add additional buffer to allow for collisions at the end of the table. The larger of the two values are taken.
// (one uses the percentage of the table to calculate the buffer, the other one is an absolute value)
const double SIZE_END_BUFFER_MULTIPLIER = 0.1;  // 10% extra buffer
const uint32 SIZE_END_BUFFER = 100;             // At least 100 entries


// Local structures ..................................................................................................

/*! Entries in the hash table. */
typedef struct 
{
    byte*   key;            /*!< Key of the entry */
    uint32  keyLength;      /*!< Key length */
    void*   value;          /*!< Value to be stored in the hash table */

} TableEntry;

/*! Hash table structure */
struct CSL_HashTable
{
    /* We currently don't keep that actual entry count here. If the need arises then we can add it and keep track
       of the number of entries in the table */

    CSL_HashTableHashFunction   hashFunction;   /*!< Hash function. This is either a user defined hash function or the 
                                                     default hash function. */
    TableEntry*                 table;          /*!< Array of hash table entrys. */
    uint32                      totalSize;      /*!< Total size of the table including buffer. This is not the size passed in  
                                                     CSL_HashTableCreate() but is larger */
    uint32                      maxIndex;       /*!< Maximum allowed index in the table. This is used to calculate the hash
                                                     table index. This value is smaller then totalSize. (But also larger then
                                                     the passed in 'size' in CSL_HashTableCreate(). */
};

// Local functions ...................................................................................................

static uint32 DefaultHashFunction(const byte* key, uint32 keyLength);
static uint32 GetTableIndex(CSL_HashTable* hashTable, const byte* key, uint32 keyLength);

static nb_boolean InsertEntry(CSL_HashTable* hashTable,
                              const byte* key,
                              uint32 keyLength,
                              void* value,
                              nb_boolean copyKey);


// Public Functions ..................................................................................................

/* See header file for description */
CSL_DEF CSL_HashTable*
CSL_HashTableCreate(uint32 size,
                    CSL_HashTableHashFunction hashFunction)
{
    CSL_HashTable* hashTable = nsl_malloc(sizeof(CSL_HashTable));
    if (!hashTable)
    {
        return NULL;
    }
    nsl_memset(hashTable, 0, sizeof(CSL_HashTable));

    // If a hash function is passed in, use it. Otherwise use our default hash function.
    if (hashFunction)
    {
        hashTable->hashFunction = hashFunction;
    }
    else
    {
        hashTable->hashFunction = &DefaultHashFunction;
    }

    // Calulate size of table. 
    hashTable->totalSize = (uint32)((double)size * SIZE_MULTIPLIER
                         + MAX((double)size * SIZE_END_BUFFER_MULTIPLIER, SIZE_END_BUFFER));

    // The maximum index is smaller then the total table to allow for collisions at the end
    hashTable->maxIndex = (uint32)((double)size * SIZE_MULTIPLIER);

    hashTable->table = nsl_malloc(sizeof(TableEntry) * hashTable->totalSize);
    if (! hashTable->table)
    {
        CSL_HashTableDestroy(hashTable, FALSE);
        return NULL;
    }
    nsl_memset(hashTable->table, 0, sizeof(TableEntry) * hashTable->totalSize);

    return hashTable;
}

/* See header file for description */
CSL_DEF void 
CSL_HashTableDestroy(CSL_HashTable* hashTable, nb_boolean freeValueData)
{
    uint32 i = 0;

    if (! hashTable)
    {
        return;
    }

    // Go through the table and free any allocated data.
    for (i = 0; i < hashTable->totalSize; ++i)
    {
        TableEntry* entry = &hashTable->table[i];

        if (entry->key)
        {
            nsl_free(entry->key);
        }
        if (freeValueData && entry->value)
        {
            nsl_free(entry->value);
        }
    }

    if (hashTable->table)
    {
        nsl_free(hashTable->table);
    }
    nsl_free(hashTable);
}

/* See header file for description */
CSL_DEF nb_boolean
CSL_HashTableInsert(CSL_HashTable* hashTable,
                    const byte* key,
                    uint32 keyLength,
                    void* value)
{
    // Insert entry and make a copy for the new key
    return InsertEntry(hashTable, key, keyLength, value, TRUE);
}

/* See header file for description */
CSL_DEF nb_boolean
CSL_HashTableDelete(CSL_HashTable* hashTable,
                    const byte* key,
                    uint32 keyLength)
{
    uint32 index = GetTableIndex(hashTable, key, keyLength);

    // Double check that the entry exists
    if (hashTable->table[index].key == NULL)
    {
        return FALSE;
    }

    /*
        We use open addressing for this hash table. That means that deleting an entry is potentially the
        costly operation. If we have an entry after the deleted entry then we have to re-insert all elements
        until we find the next gap. This is still relatively fast, if it is not too many entries since insertion
        is really fast.
    */

    // If there is no following entry then we are in luck and we don't have to do any re-insertion.
    if (hashTable->table[index + 1].key == NULL)
    {
        // Delete the entry and return. The entry has to match our given key!
        TableEntry* entry = &hashTable->table[index];
        nsl_assert(nsl_memcmp(key, entry->key, keyLength) == 0);

        nsl_free(entry->key);
        entry->key = NULL;
        entry->keyLength = 0;
        entry->value = NULL;

        return TRUE;
    }

    {
        uint32 i = 0;
        uint32 insertCount = 0;
        nb_boolean isDeleted = FALSE;
        nb_boolean insertResult = TRUE;

        /* We create a temporary vector to hold all entries which we have to re-insert. */
        CSL_Vector* temporaryVector = CSL_VectorAlloc(sizeof(TableEntry));
        if (!temporaryVector)
        {
            return FALSE;
        }

        // Loop until we reach the first gap, starting from the entry index
        for (i = index; i < hashTable->totalSize; ++i)
        {
            TableEntry* entry = &hashTable->table[i];

            // If we reach a gap then we stop
            if (entry->key == NULL)
            {
                break;
            }

            // Check if it is the entry which we need to delete. Only compare if we haven't found the entry, yet.
            if ((! isDeleted) && (nsl_memcmp(key, entry->key, MAX(keyLength, entry->keyLength)) == 0))
            {
                // Delete entry (it gets cleared below)
                nsl_free(entry->key);
                isDeleted = TRUE;
            }
            else
            {
                // Add entry to temporary vector for re-insertion
                if (!CSL_VectorAppend(temporaryVector, entry))
                {
                    CSL_VectorDealloc(temporaryVector);
                    return FALSE;
                }
            }

            // Clear entry
            entry->key = NULL;
            entry->keyLength = 0;
            entry->value = NULL;
        }

        // Re-insert all entries
        insertCount = CSL_VectorGetLength(temporaryVector);
        for (i = 0; i < insertCount; ++i)
        {
            // Re-insert entry, but don't allocate memory for key, we already have it allocated.
            TableEntry* insertEntry = CSL_VectorGetPointer(temporaryVector, i);
            if (! InsertEntry(hashTable, insertEntry->key, insertEntry->keyLength, insertEntry->value, FALSE))
            {
                insertResult = FALSE;
            }
        }

        CSL_VectorDealloc(temporaryVector);

        return insertResult;
    }
}

/* See header file for description */
CSL_DEF void
CSL_HashTableDeleteAll(CSL_HashTable* hashTable)
{
    uint32 i = 0;

    // Go through the entire table and clear all elements.
    for (i = 0; i < hashTable->totalSize; ++i)
    {
        if (hashTable->table[i].key)
        {
            // Delete the key, but don't delete the value. It is owned by the user.
            nsl_free(hashTable->table[i].key);
            hashTable->table[i].key = NULL;
            hashTable->table[i].keyLength = 0;
            hashTable->table[i].value = NULL;
        }
    }
}

/* See header file for description */
CSL_DEF void* 
CSL_HashTableFind(CSL_HashTable* hashTable,
                  const byte* key,
                  uint32 keyLength)
{
    uint32 i = 0;
    uint32 index = GetTableIndex(hashTable, key, keyLength);

    // Look for the entry, starting at the calculated index
    for (i = index; i < hashTable->totalSize; ++i)
    {
        TableEntry* entry = &hashTable->table[i];

        // If the slot is empty then the entry does not exist
        if (entry->key == NULL)
        {
            return NULL;
        }

        // If the key matches then we found the entry
        if (nsl_memcmp(key, entry->key, MAX(keyLength, entry->keyLength)) == 0)
        {
            return entry->value;
        }
    }

    // We should never reach this. If we do, then there is a problem with the hash function or the table size is too small.
    return NULL;
}

/* See header file for description */
CSL_DEF void
CSL_HashTableForEach(CSL_HashTable* hashTable, CSL_HashTableForEachCallback callback, void* userData)
{
    uint32 i = 0;

    // Go through all elements and call the callback function for each valid element
    for (i = 0; i < hashTable->totalSize; ++i)
    {
        if (hashTable->table[i].key)
        {
            callback(hashTable->table[i].value, userData);
        }
    }
}

/* See header file for description */
CSL_DEF NB_Error 
CSL_HashTableGetStats(CSL_HashTable* hashTable,
                      uint32* totalTableSize,
                      uint32* entryCount,
                      uint32* collisionCount,
                      double* averageLookupCount)
{
    uint32 i = 0;

    if (!hashTable || !totalTableSize || !entryCount || !collisionCount || !averageLookupCount)
    {
        return NE_INVAL;
    }
    *totalTableSize     = hashTable->totalSize;
    *entryCount         = 0;
    *collisionCount     = 0;
    *averageLookupCount = 0.0;

    // Check collisions
    for (i = 0; i < hashTable->totalSize; ++i)
    {
        uint32 expectedIndex = 0;
        TableEntry* entry = &hashTable->table[i];

        if (entry->key)
        {
            ++(*entryCount);

            // Calculate expected index. If it doesn't match the actual index then we have a collision
            expectedIndex = GetTableIndex(hashTable, entry->key, entry->keyLength);
            if (expectedIndex != i)
            {
                ++(*collisionCount);
            }
        }
    }

    /*  
        Check average lookup time. This is very inefficient, we check every key and check the lookup.
    */
    for (i = 0; i < hashTable->totalSize; ++i)
    {
        TableEntry* entry = &hashTable->table[i];
        if (entry->key)
        {
            uint32 j = 0;
            double lookupCount = 0;

            // Compare to CSL_HashTableFind() logic

            uint32 index = GetTableIndex(hashTable, entry->key, entry->keyLength);

            // Look for the entry, starting at the calculated index
            for (j = index; j < hashTable->totalSize; ++j)
            {
                TableEntry* innerEntry = &hashTable->table[j];

                lookupCount += 1.0;

                // If the key matches then we found the entry
                if (nsl_memcmp(innerEntry->key, entry->key, MAX(innerEntry->keyLength, entry->keyLength)) == 0)
                {
                    // Exit inner loop
                    break;
                }
            }

            *averageLookupCount += lookupCount;
        }
    }

    if (*entryCount != 0)
    {
        *averageLookupCount = (*averageLookupCount) / (double)(*entryCount);
    }

    return NE_OK;
}


// Local functions ...................................................................................................

/*! Default hash function used.

    Written by Bob Jenkins. This performs very well for string keys.

    @see http://en.wikipedia.org/wiki/Jenkins_hash_function

    @return hash value
*/
uint32 
DefaultHashFunction(const byte* key, uint32 keyLength)
{
    uint32 hash = 0;
    uint32 i = 0;

    for (; i < keyLength; ++i)
    {
        hash += key[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

	return hash;
}

/*! Get the table index based on the key.
    
    @return table index
*/
uint32 
GetTableIndex(CSL_HashTable* hashTable, const byte* key, uint32 keyLength)
{
    // Call the hash function to generate a large 4-byte value based on the key. Then call the modulo operator
    // to create the actual index in the table.
    return (hashTable->hashFunction(key, keyLength) % hashTable->maxIndex);
}

/*! Insert an entry into the hash table.

    @return 'TRUE' on success, 'FALSE' otherwise.
*/
nb_boolean 
InsertEntry(CSL_HashTable* hashTable,   /*!< See CSL_HashTableInsert for description of most parameters */
            const byte* key,
            uint32 keyLength,
            void* value,
            nb_boolean copyKey          /*!< Make a copy of the key, if set. Otherwise just copy the pointer. */
            )
{
    uint32 i = 0;
    uint32 index = GetTableIndex(hashTable, key, keyLength);

    /*  
        We use "open addressing" for our hash table. That means we insert the entry into the first open slot
        starting from the index created by the hash.
    */
    for (i = index; i < hashTable->totalSize; ++i)
    {
        // If the slot is open, add the entry
        TableEntry* entry = &hashTable->table[i];
        if (entry->key == NULL)
        {
            if (copyKey)
            {
                entry->key = nsl_malloc(keyLength);
                if (!entry->key)
                {
                    return FALSE;
                }
                nsl_memcpy(entry->key, key, keyLength);
            }
            else
            {
                // Key is already allocated. Just copy the pointer
                entry->key = (byte*)key;
            }

            entry->keyLength = keyLength;
            entry->value = value;
            return TRUE;
        }
    }

    // We should never reach this. If we do, then there is a problem with the hash function or the table size is too small.
    return FALSE;
}


/*! @} */


