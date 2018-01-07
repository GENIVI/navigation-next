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
 * CSL_Vector.h: created 2004/12/05 by Dima Dorfman.
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

#ifndef VEC_H
#define	VEC_H

#include "paltypes.h"
#include "palstdlib.h"
#include "cslexp.h"

typedef struct CSL_Vector CSL_Vector;

/*! Compare function to compare two vector entries.

    @return "negative value" if left side is smaller, "positive value" if right side is smaller, "zero" if both sides are equal.
*/
typedef int (*CSL_VectorCompareFunction)(const void* left, const void* right);

/*! Callback function for CSL_VectorForEach.

    @return I think 'zero' to abort for-each loop. non-zero to continue loop.
*/
typedef int (*CSL_VectorForEachCallback)(void* userData, void* itemPointer);


CSL_DEC CSL_Vector *CSL_VectorAlloc(size_t itemsize);
CSL_DEC int CSL_VectorAppend(CSL_Vector *vector, const void *item);
CSL_DEC void CSL_VectorDealloc(CSL_Vector *vector);
CSL_DEC void CSL_VectorGet(CSL_Vector *vector, int index, void *item);
CSL_DEC void *CSL_VectorGetPointer(CSL_Vector *vector, int index);
CSL_DEC void CSL_VectorRemove(CSL_Vector *vector, int index);
CSL_DEC int CSL_VectorRemoveItem(CSL_Vector *vector, void *item);

/*! Remove all elements from the vector .

    This does not destroy the vector itself.

    @return None

    @see CSL_VectorRemove
*/
CSL_DEC void CSL_VectorRemoveAll(CSL_Vector *vector);

/*! Remove any duplicate entries from the vector.

    @return None
*/
CSL_DEC void
CSL_VectorRemoveDuplicates(CSL_Vector* input,                           /*!< Input vector */
                           CSL_VectorCompareFunction compareFunction    /*!< Compare functions to compare elements. */
                           );

CSL_DEC int CSL_VectorGetItemSize(CSL_Vector* vector);
CSL_DEC int	 CSL_VectorGetIndex(CSL_Vector *vector, void* item);
CSL_DEC int CSL_VectorGetLength(CSL_Vector *vector);
CSL_DEC void CSL_VectorPop(CSL_Vector *vector, void *item);
CSL_DEC void CSL_VectorSet(CSL_Vector *vector, int index, const void *item);

/*! Allocates enough space and resizes the vector. Does change size.

    @return I think non-zero for success.
*/
CSL_DEC int CSL_VectorSetSize(CSL_Vector *vector, int len);

/*! Reserve space for items to avoid later reallocations. Does not change size.

    @return I think non-zero for success.
*/
CSL_DEC int CSL_VectorSetAlloc(CSL_Vector *vector, int len);

CSL_DEC int CSL_VectorCopy(CSL_Vector **v_dest, CSL_Vector *v_src, int (*copy_item)(void*, void *, const void *), void* puser);
CSL_DEC int CSL_VectorSort(CSL_Vector *vector, CSL_VectorCompareFunction compareFunction);
CSL_DEC int CSL_VectorCoreSort(CSL_Vector *vector, CSL_VectorCompareFunction compareFunction);
CSL_DEC void CSL_VectorSort2(CSL_Vector *vector, CSL_VectorCompareFunction compareFunction, void* tmp);
CSL_DEC int CSL_VectorForEach(CSL_Vector* vector, CSL_VectorForEachCallback callback, void* userData);

/*! Compares two vectors and adds all elements which are different to the output vector.

    This function is modeled after to the C++ Standard Template Library function "std::set_difference".

    The difference is formed by the elements that are present in the first vector, but not in the second one.
    Both input vectors have to be sorted. Both vectors have to contain the same kind of entries.

    @return TRUE for success, FALSE for failure.
*/
CSL_DEC nb_boolean
CSL_VectorDifference(CSL_Vector* input1,                /*!< First input vector */
                     CSL_Vector* input2,                /*!< Second input vector */
                     CSL_Vector* output,                /*!< The output vector containing the difference between input vector one and two */
                     CSL_VectorCompareFunction compareFunction /*!< Compare functions to compare elements. The input vectors have to sorted using
                                                                    the same compare function. */
                     );

CSL_DEC size_t CSL_VectorGetStructureSize(void);
CSL_DEC CSL_Vector *CSL_VectorAllocPlace(size_t itemsize, void* p);
CSL_DEC void CSL_VectorDeallocPlace(CSL_Vector *vector);

#endif
