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
 * CSL_Vector.c: created 2004/12/09 by Dima Dorfman.
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

/* dmcpherson
 * de-brew
 * #include "nimbrew.h"
 */
#include "paltypes.h"
#include "cslutil.h"
#include "vec.h"

struct CSL_Vector
{
	size_t itemsize;
	int size;
	int allocated;
	void *items;

#ifdef AEE_SIMULATOR
	//This variable is for tracking down our frequent failure to properly deallocate vectors
	//As we know the size of leaked items in the heap dump, these tell us the size of the items
	//on vectors that have not been freed; this can be helpful in narrowing down where the problem
	//lies.
	void *debugsize;
#endif
};

/*
 * Set size to newsize and grow or shrink the buffer appropriately.
 */
static int
_realloc(CSL_Vector *vector, int newsize)
{
	void* a = NULL;
	int i = 0;

	/* Do nothing if we have enough space but not too much extra. */
	if ((vector->allocated >= newsize && newsize >= vector->allocated / 2) ||
		(newsize == 0 && vector->allocated <= 2))
    {
		goto exit;
    }

	/*
	 * Mild overallocation based on the size of the list. Specific
	 * strategy stolen from Python.
	 */
	if (newsize == 0)
    {
		i = 0;
    }
	else
    {
		i = (newsize >> 3) + (newsize < 9 ? 3 : 6) + newsize;
    }
	if (i == 0)
    {
		nsl_free(vector->items);
		vector->items = NULL;
	}
    else
    {
		a = nsl_realloc(vector->items, i * vector->itemsize);
		if (a == NULL)
        {
			if (vector->allocated >= newsize)
            {
				goto exit;
            }
			else
            {
				return (0);
            }
		}
		vector->items = a;
	}
	vector->allocated = i;

exit:
	return (1);
}

/*
 * Set size to newsize and grow or shrink the buffer appropriately.
 */
static int
resize(CSL_Vector *vector, int newsize)
{
	int r = _realloc(vector,newsize);

	if (r)
    {
		vector->size = newsize;
    }

	return r;
}


CSL_DEF CSL_Vector *
CSL_VectorAlloc(size_t itemsize)
{
	CSL_Vector* vector = NULL;

	vector = nsl_malloc(sizeof(*vector));

	if (vector == NULL)
    {
		return (NULL);
    }

	vector->itemsize = itemsize;
	vector->allocated = vector->size = 0;
	vector->items = NULL;

#ifdef AEE_SIMULATOR
	vector->debugsize = nsl_malloc(itemsize);
#endif

	return (vector);
}

CSL_DEF CSL_Vector*
CSL_VectorAllocPlace(size_t itemsize, void* p)
{
	CSL_Vector* vector = p;

	if (vector == NULL)
    {
		return (NULL);
    }

	vector->itemsize = itemsize;
	vector->allocated = vector->size = 0;
	vector->items = NULL;
	return (vector);
}

CSL_DEF int
CSL_VectorAppend(CSL_Vector *vector, const void *item)
{
	if (!resize(vector, vector->size + 1))
    {
		return (0);
    }

	CSL_VectorSet(vector, vector->size - 1, item);
	return (1);
}

CSL_DEF void
CSL_VectorDealloc(CSL_Vector *vector)
{
#ifdef AEE_SIMULATOR
	nsl_free(vector->debugsize);
#endif

    if (vector)
    {
        if (vector->items)
        {
            nsl_free(vector->items);
        }
        nsl_free(vector);
    }
}

CSL_DEF void
CSL_VectorDeallocPlace(CSL_Vector *vector)
{
    if (vector && vector->items)
    {
        nsl_free(vector->items);
    }
}

CSL_DEF void
CSL_VectorGet(CSL_Vector *vector, int index, void *item)
{
	nsl_memcpy(item, CSL_VectorGetPointer(vector, index), vector->itemsize);
}

CSL_DEF void *
CSL_VectorGetPointer(CSL_Vector *vector, int index)
{
	nsl_assert(index < vector->size);
	return ((char *)vector->items + index * vector->itemsize);
}

CSL_DEF void
CSL_VectorRemove(CSL_Vector *vector, int index)
{
    //check if index is valid
    if((0 <= index) && (index < vector->size))
    {
        // Only copy data if it is not the last element
        if (index < (vector->size - 1))
        {
            // Do a memory block copy of all the data past the removed item
            nsl_memmove(CSL_VectorGetPointer(vector, index),
                        CSL_VectorGetPointer(vector, index + 1),
                        vector->itemsize * (vector->size - index - 1));
        }

	    (void)resize(vector, vector->size - 1);
    }
}

CSL_DEF int
CSL_VectorRemoveItem(CSL_Vector *vector, void *item)
{
    int index = CSL_VectorGetIndex(vector, item);
    CSL_VectorRemove(vector, index);
	return 1;
}

/* See header file for description */
CSL_DEF void
CSL_VectorRemoveAll(CSL_Vector* vector)
{
    // Free all elements and set everything to zero
    if (vector->items)
    {
        nsl_free(vector->items);
        vector->items = NULL;
    }
	vector->size = 0;
	vector->allocated = 0;

    // Don't reset the itemsize. We still need that.
}

/* See header for description */
CSL_DEF void
CSL_VectorRemoveDuplicates(CSL_Vector* input,
                           CSL_VectorCompareFunction compareFunction)
{
    int i = 0;
    int j = 0;

    // If the vector has less then 2 elements then we're done
    int length = CSL_VectorGetLength(input);
    if (length < 2)
    {
        return;
    }

    // Walk vector backwards starting from the second to last element
    for (i = length - 1; i > 0; --i)
    {
        for (j = i-1; j >= 0; j--)
        {
            // If the entries match then we remove the second item. This leaves our remaining vector intact.
            if (compareFunction((byte*)CSL_VectorGetPointer(input, i), (byte*)CSL_VectorGetPointer(input, j)) == 0)
            {
                CSL_VectorRemove(input, i);
                break;
            }
        }
    }
}

CSL_DEF int
CSL_VectorGetIndex(CSL_Vector *vector, void* item)
{
    int index = 0;
    for (index = 0; index < vector->size; index++)
    {
        if (nsl_memcmp(CSL_VectorGetPointer(vector, index), item, vector->itemsize) == 0)
        {
            return index;
        }
    }
    return -1;
}

CSL_DEF int
CSL_VectorGetItemSize(CSL_Vector *vector)
{
	return (int)vector->itemsize;
}

CSL_DEF int
CSL_VectorGetLength(CSL_Vector *vector)
{
	if (vector == NULL)
    {
		return 0;
    }

	return (vector->size);
}

CSL_DEF void
CSL_VectorPop(CSL_Vector *vector, void *item)
{
	nsl_assert(vector->size > 0);
	CSL_VectorGet(vector, vector->size - 1, item);
	(void)resize(vector, vector->size - 1);
}

CSL_DEF void
CSL_VectorSet(CSL_Vector *vector, int index, const void *item)
{
	nsl_memcpy(CSL_VectorGetPointer(vector, index), item, vector->itemsize);
}

/* See header file for description */
CSL_DEF int
CSL_VectorSetSize(CSL_Vector *vector, int len)
{
	return resize(vector, len);
}

/* See header file for description */
CSL_DEF int
CSL_VectorSetAlloc(CSL_Vector *vector, int len)
{
	return _realloc(vector,len);
}

static int
simple_copy_item(void* pUser, void* pItemDest, const void* pItemSrc)
{
	nsl_memcpy(pItemDest, pItemSrc, *(int*)pUser);
	return 1;
}

CSL_DEF int
CSL_VectorCopy(CSL_Vector **v_dest, CSL_Vector *v_src, int (*copy_item)(void*, void *, const void *), void* puser)
{
	int n = 0;

	if (copy_item == NULL)
    {
		copy_item = simple_copy_item;
		puser = &v_src->itemsize;
	}

	*v_dest = CSL_VectorAlloc(v_src->itemsize);

	if (*v_dest == NULL)
    {
		return 0;
    }

	CSL_VectorSetAlloc(*v_dest, v_src->size);

	for (n = 0; n < v_src->size; n++)
    {
		if (!resize(*v_dest, (*v_dest)->size+1))
        {
			return 0;
        }

		if (!copy_item(puser, CSL_VectorGetPointer(*v_dest, n), CSL_VectorGetPointer(v_src,n)))
        {
			resize(*v_dest, (*v_dest)->size-1);
			return 0;
		}
	}

	return 1;
}

CSL_DEF int
CSL_VectorSort(CSL_Vector *vector, CSL_VectorCompareFunction compareFunction)
{
	void* k = NULL;

	if (vector->size > 1)
    {
		k = (void*) nsl_malloc(vector->itemsize);
		if (k == NULL)
        {
			return (0);
        }
		nim_heapsort(vector->items, vector->size, vector->itemsize, k, compareFunction);
		nsl_free(k);
	}
	return (1);
}


CSL_DEF int
CSL_VectorCoreSort(CSL_Vector *vector, CSL_VectorCompareFunction compareFunction)
{
	void* k = NULL;

	if (vector->size > 1)
    {
		k = (void*) nsl_malloc(vector->itemsize);
		if (k == NULL)
        {
			return (0);
        }
		nim_heapsort(vector->items, vector->size, vector->itemsize, k, compareFunction);
		nsl_free(k);
	}
	return (1);
}


CSL_DEF void
CSL_VectorSort2(CSL_Vector *vector, CSL_VectorCompareFunction compareFunction, void* tmp)
{
	if (vector->size > 1)
    {
		nim_heapsort(vector->items, vector->size, vector->itemsize, tmp, compareFunction);
    }
}

CSL_DEF size_t
CSL_VectorGetStructureSize()
{
	return sizeof(CSL_Vector);
}

CSL_DEF int
CSL_VectorForEach(CSL_Vector* vector, CSL_VectorForEachCallback callback, void* userData)
{
    int i = 0;

    byte* item = vector->items;

	for (i = 0; i < vector->size; i++)
    {
        if (!(callback)(userData, (void*)item))
        {
			return 0;
		}
        item += vector->itemsize;
	}

    return 1;
}

/* See header for description */
CSL_DEF nb_boolean
CSL_VectorDifference(CSL_Vector* input1,
                     CSL_Vector* input2,
                     CSL_Vector* output,
                     CSL_VectorCompareFunction compareFunction)
{
    byte* item1 = NULL;
    byte* item2 = NULL;
    byte* last1 = NULL;
    byte* last2 = NULL;
    size_t itemSize = 0;

    // Validate parameters
    if ((!input1) || (!input2) || (!output) || (!compareFunction))
    {
        return FALSE;
    }

    // All vector have to have same item size
    if ((input1->itemsize != input2->itemsize) || (input1->itemsize != output->itemsize))
    {
        return FALSE;
    }
    itemSize = input1->itemsize;

    CSL_VectorRemoveAll(output);

    // If the first vector is empty then the output vector will be empty and we don't do anything
    if (input1->size == 0)
    {
        return TRUE;
    }

    // If the second vector is empty then we just copy the first vector to the output vector
    if (input2->size == 0)
    {
        if (!resize(output, input1->size))
        {
            return FALSE;
        }
        nsl_memcpy(output->items, input1->items, input1->size * itemSize);
        return TRUE;
    }

    // Get first and last element from both input vectors
    item1 = input1->items;
    item2 = input2->items;
    last1 = CSL_VectorGetPointer(input1, input1->size - 1);
    last2 = CSL_VectorGetPointer(input2, input2->size - 1);

    // Walk both input vectors simultaneous
    while ((item1 <= last1) && (item2 <= last2))
    {
        // Compare the two values
        int compareResult = compareFunction(item1, item2);
        if (compareResult < 0)
        {
            // Item in vector one but not vector two, add to output vector
            if (CSL_VectorAppend(output, item1) == 0)
            {
                return FALSE;
            }
            item1 += itemSize;
        }
        else if (compareResult > 0)
        {
            // Item in vector two but not vector one, ignore
            item2 += itemSize;
        }
        else
        {
            // Both values are equal, advance both vectors
            item1 += itemSize;
            item2 += itemSize;
        }
    }

    // If there are any items left in vector one, copy them to the output vector
    if (item1 <= last1)
    {
        // Do a bulk copy instead of copying them one by one

        int remainingCount = (int)(last1 - item1) / itemSize + 1;
        int oldSize = output->size;

        if (!resize(output, oldSize + remainingCount))
        {
            return FALSE;
        }
        nsl_memcpy(CSL_VectorGetPointer(output, oldSize), item1, remainingCount * itemSize);
    }

    return TRUE;
}
