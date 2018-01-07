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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*!--------------------------------------------------------------------------

    @file     cslhashtable.h
    @date     02/23/2011

    Hash table implementation. This hash table uses a default hash function
    which works well for string keys. If other keys than strings are used
    it might be necessary to use a different hash function.

    This hash table doesn't currently allow resizing of the table. That means 
    that the size of the table must be known in advance. We could add resizing
    of the talbe but resizing a hash table is very inefficent performance wise.
    (all entries have to be re-hashed) so I don't think we have a need for it.

    This implementation uses open addressing for collision resolution. See 
    Wikipedia or other resources for a description of this approach.
    This works very well if we keep the load factor low (e.g. below 0.5). We 
    accomplish this by making the actual table larger (e.g. twice as big) then
    the maximum size. This works if we have 10,000 entries or less which is totally
    sufficient for our usage. Should we need a hash table for more entries then
    we could consider changing the collision resolution algorithm.

    This hash table just takes pointers to a data block. It does not copy any data.
    Should we want to store the actual data blocks in the hash table then we might
    consider other colision resolutions since open addressing would waste too much
    memory.
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

/*@{*/

#ifndef _CSL_HASH_TABLE_H_
#define	_CSL_HASH_TABLE_H_

#include "paltypes.h"
#include "pal.h"
#include "palstdlib.h"
#include "cslexp.h"
#include "nberror.h"

typedef struct CSL_HashTable CSL_HashTable;

/*! Hash function to generate a hash value based on a given key.

    The hash value can by (should be) a large 4-byte (uint32) value. The hash table will create an appropriate index
    into the hash table using the hash value.

    @return Hash value
*/
typedef uint32 (*CSL_HashTableHashFunction)(const byte* key,    /*!< Unique key to use to generate the hash */
                                            uint32 keyLength    /*!< Length in bytes of key */
                                            );

/*! Callback function for CSL_HashTableForEach().

    Gets called for each element in the hash table.

    @return None
*/
typedef void (*CSL_HashTableForEachCallback)(void* value,       /*!< The value in the hash table */
                                             void* userData     /*!< User data passed in CSL_HashTableForEach() */
                                             );


/*! Create a hash table.

    The hash table will hold more entries then the given 'size' parameter, but it is advised to not add more
    entries because it will increase the chance for collisions and at some point insertion will fail.

    The default hash table will work well for any byte key, including string keys. Try to avoid very small (one-byte) keys
    since they don't perform well.

    @return Hash table instance on success, NULL on failure.
*/
CSL_DEC CSL_HashTable*
CSL_HashTableCreate(uint32 size,                                /*!< Maximum elements to store in the hash table */
                    CSL_HashTableHashFunction hashFunction      /*!< Optional hash function. Set to NULL to use default hash function. */
                    );

/*! Destroy a hash table.

    @return None
*/
CSL_DEC void 
CSL_HashTableDestroy(CSL_HashTable* hashTable,                  /*!< Hash table instance returned by CSL_HashTableCreate */
                     nb_boolean freeValueData                   /*!< Set to TRUE to destroy the value pointers associated with
                                                                     the hash entries */
                     );

/*! Insert a new entry to the hash table.

    @return 'TRUE' on success, 'FALSE' if the entry does not exist or an error occured.
*/
CSL_DEC nb_boolean
CSL_HashTableInsert(CSL_HashTable* hashTable,                   /*!< Hash table instance returned by CSL_HashTableCreate */
                    const byte* key,                            /*!< key of entry to insert. */
                    uint32 keyLength,                           /*!< Length in bytes of key */ 
                    void* value                                 /*!< Value to add to the hash table */
                    );

/*! Deletes an entry from the hash table.

    This does not delete the associated value pointers. It is the responsibility of the caller to free that data.

    @return 'TRUE' on success, 'FALSE' otherwise.
*/
CSL_DEC nb_boolean 
CSL_HashTableDelete(CSL_HashTable* hashTable,                   /*!< Hash table instance returned by CSL_HashTableCreate */
                    const byte* key,                            /*!< Key of entry to delete */
                    uint32 keyLength                            /*!< Length in bytes of key */
                    );

/*! Deletes all entries from the hash table.

    This does not delete the associated value pointers. It is the responsibility of the caller to free that data.

    @return None
*/
CSL_DEC void
CSL_HashTableDeleteAll(CSL_HashTable* hashTable                 /*!< Hash table instance returned by CSL_HashTableCreate */
                       );

/*! Find an entry in the hash table.

    @return The entry if it exists, NULL if the entry was not found.
*/
CSL_DEC void* 
CSL_HashTableFind(CSL_HashTable* hashTable,                     /*!< Hash table instance returned by CSL_HashTableCreate */
                  const byte* key,                              /*!< Key of entry to find. */
                  uint32 keyLength                              /*!< Length in bytes of key */
                  );

/*! Iterate through all elements in the hash table.

    This function calls a user supplied callback function for each element in the hash table.

    @return None
*/
CSL_DEC void
CSL_HashTableForEach(CSL_HashTable* hashTable,                  /*!< Hash table instance returned by CSL_HashTableCreate */
                     CSL_HashTableForEachCallback callback,     /*!< Callback function to call for each element */
                     void* userData                             /*!< User data passed to the callback */
                     );

/*! Get statistics for the hash table.

    ! Important:
    This should only be used for testing since it is very inefficient!

    @return NB_Error
*/
CSL_DEC NB_Error 
CSL_HashTableGetStats(CSL_HashTable* hashTable,                 /*!< Hash table instance returned by CSL_HashTableCreate */
                      uint32* totalTableSize,                   /*!< On return the total size of the internal hash table. This is 
                                                                     larger then the requested maximum size for the hash table. */
                      uint32* entryCount,                       /*!< Number of entries currently in the hash table */
                      uint32* collisionCount,                   /*!< Number of collisions (same index for different keys). The
                                                                     higher this number the less efficient the hash table performs. */
                      double* averageLookupCount                /*!< Average count for find-lookup. If all the entries are in the correct slot
                                                                     then this value would be 1.0 (no collisions). The higher this number
                                                                     the less efficient the table is. */
                      );


#endif  // _CSL_HASH_TABLE_H_

/*! @} */


