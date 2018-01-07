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
 * CSL_Dictionary.h: created 2004/12/05 by Dima Dorfman.
 *
 *
 *  This implements a dictionary which takes a string as key. It uses a hash
 *  table to store the data. It basically works like std::hash_map.
 *
 *
 * Copyright (C) 2004 Networks In Motion, Inc. All rights reserved.
 *
 * The information contained herein is confidential and proprietary to
 * Networks In Motion, Inc., and is considered a trade secret as
 * defined in section 499C of the California Penal Code. Use of this
 * information by anyone other than authorized employees of Networks
 * In Motion is granted only under a written non-disclosure agreement
 * expressly prescribing the scope and manner of such use.
 */

#ifndef CSDICT_H
#define	CSDICT_H

#include "palstdlib.h"
#include "cslexp.h"

struct CSL_Dictionary;

/*! Deletion function definition. 
    
    What is this used for?
*/
typedef void CSL_DictionaryDeleteFunction(void* userdata, const char* key, const char *value, size_t valueLength);

/*! Create a dictionary.

    @return 
*/
CSL_DEC struct CSL_Dictionary*
CSL_DictionaryAlloc(int minsize                                 /*!< Minimum number of slots in the dictionary */
                    );

/*! Create a dictionary using a delete function.

    @return 
*/
CSL_DEC struct CSL_Dictionary*
CSL_DictionaryAllocEx(int minsize,                              /*!< Minimum number of slots in the dictionary */
                      CSL_DictionaryDeleteFunction delfunc,     /*!< Function to be called to deallocate an item */
                      void* deluserdata                         /*!< Opaque user data to pass to the deallocation function */
                      );

/*! Delete an element in a dictionary.

    @return 
*/
CSL_DEC void 
CSL_DictionaryDelete(struct CSL_Dictionary *dictionary, 
                     const char *key                            /*!< Key of element to delete */
                     );

/*! Delete an element in a dictionary.

    @return None
*/
CSL_DEC void 
CSL_DictionaryDeleteEx(struct CSL_Dictionary *dictionary, 
                       const char *key,                         /*!< Key of element to delete */
                       int assert_present                       /*!< If True, cause an assertion failure if the item is NOT actually present */
                       );

/*! Destroy the dictionary.

    @return 
*/
CSL_DEC void 
CSL_DictionaryDealloc(struct CSL_Dictionary *dictionary
                      );

/*! Get an element from the dictionary.

    @return A pointer to the element in the dictionary
*/
CSL_DEC char*
CSL_DictionaryGet(struct CSL_Dictionary *dictionary, 
                  const char *key,                        /*!< Key of element to get */
                  size_t *valueLength                     /*!< The length of the element */
                  );

/*! Get the number of elements stored in the dictionary.

    @return Number of elements in the dictionary
*/
CSL_DEC int 
CSL_DictionaryLength(struct CSL_Dictionary *dictionary
                     );

/*! Get the next element.

    @return 0 if there are no additional items, 1 if there are.
*/
CSL_DEC int 
CSL_DictionaryNext(struct CSL_Dictionary *dictionary, 
                   int *iter,                               /*!< Iteration state, set to 0 to begin */
                   const char **key,                        /*!< Key of the next element */
                   char **value,                            /*!< Pointer to the next element */
                   size_t* valueLength                      /*!< Size of the next element */
                   );

/*! Set the element associated with a specific string key along with some extra space.

    This is a hack to allow the user to allocate extra space for
    the value--beyond what they want to copy in right now. With CSL_DictionaryGet
    returning a non-const, this allows the user to write to the buffer
    after it has been inserted. This doesn't allow anything that making
    a longer value to begin with can't do, but it avoids an extra copy.

    @return non-zero for success, zero for failure
*/
CSL_DEC int 
CSL_DictionarySetEx(struct CSL_Dictionary *dictionary, 
                    const char *key,                        /*!< Key of element to set */
                    const char *value,                      /*!< Pointer to the data associated with element */
                    size_t valueLength,                     /*!< Length of the element data */
                    size_t extra                            /*!< Length of the extra data to allocate */
                    );

/*! Set the element associated with a specific string key.

    @return non-zero for success, zero for failure
*/
CSL_DEC int 
CSL_DictionarySet(struct CSL_Dictionary *dictionary, 
                  const char *key,                          /*!< Key of element to add? */
                  const char *value,                        /*!< Pointer to the data associated with element */
                  size_t valueLength                               /*!< Length of the element data */
                  );

/* return 0 from the iterfun to delete the item after returning */
typedef int 
CSL_DictionaryIterationFunction(void* userdata,             /*!< User data passed to the iteration function */
                                const char* key,            /*!< Key of the element */
                                const char *value,          /*!< Value of the element */
                                size_t valueLength          /*!< Length of the element??? */
                                );


/*! Iterate through the dictionary.

    @return 
*/
CSL_DEC int 
CSL_DictionaryIteration(struct CSL_Dictionary *dictionary, 
                        CSL_DictionaryIterationFunction func,   /*!< Iteration function which gets called for all the elements */
                        void* userdata                          /*!< User data passed to the iteration callback function */
                        );

/* Not really part of the dictionary, but useful to expose anyway. */
CSL_DEC long bytehash(const char *s, size_t size);
CSL_DEC long bytehashs(const char *s);

#endif
