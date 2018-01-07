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
* PAL_Vector.c: created 2004/12/09 by Dima Dorfman.
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
#if defined(_WIN32) && defined(_DEBUG) //this is for memory leak analysis tool
#define USE_NATIVE_MEM_API
#endif

#include "paltypes.h"
#include "palvector.h"
#include "heapsort.h"

struct PAL_Vector 
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
_realloc(struct PAL_Vector *vector, int newsize)
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
resize(struct PAL_Vector *vector, int newsize)
{
    int r = _realloc(vector,newsize);

    if (r)
    {
        vector->size = newsize;
    }

    return r;
}


 struct PAL_Vector *
PAL_VectorAlloc(size_t itemsize)
{
    struct PAL_Vector* vector = NULL;

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

 struct PAL_Vector*
PAL_VectorAllocPlace(size_t itemsize, void* p)
{
    struct PAL_Vector* vector = p;

    if (vector == NULL)
    {
        return (NULL);
    }

    vector->itemsize = itemsize;
    vector->allocated = vector->size = 0;
    vector->items = NULL;
    return (vector);
}

 int
PAL_VectorAppend(struct PAL_Vector *vector, const void *item)
{
    if (!resize(vector, vector->size + 1))
    {
        return (0);
    }

    PAL_VectorSet(vector, vector->size - 1, item);
    return (1);
}

 void
PAL_VectorDealloc(struct PAL_Vector *vector)
{
#ifdef AEE_SIMULATOR
    nsl_free(vector->debugsize);
#endif

    nsl_free(vector->items);
    nsl_free(vector);
}

 void 
PAL_VectorDeallocPlace(struct PAL_Vector *vector)
{
    nsl_free(vector->items);
}

 void
PAL_VectorGet(struct PAL_Vector *vector, int index, void *item)
{
    nsl_memcpy(item, PAL_VectorGetPtr(vector, index), vector->itemsize);
}

 void *
PAL_VectorGetPtr(struct PAL_Vector *vector, int index)
{
    nsl_assert(index < vector->size);
    return ((char *)vector->items + index * vector->itemsize);
}

 void 
PAL_VectorRemove(struct PAL_Vector *vector, int index)
{
    int n = 0;

    // @todo: I think we could just do a block copy of all the elements after index???

    for (n = index; n < (vector->size - 1); n++)
    {
        nsl_memcpy(PAL_VectorGetPtr(vector, n), PAL_VectorGetPtr(vector,n+1), vector->itemsize);
    }

    (void)resize(vector, vector->size - 1);
}

 int 
PAL_VectorRemoveItem(struct PAL_Vector *vector, void *item)
{
    int index = PAL_VectorGetIndex(vector, item);
    PAL_VectorRemove(vector, index);
    return 1;
}

/*! Remove all elements from the vector .

This does not destroy the vector itself.

@return None

@see PAL_VectorRemove
*/
 void
PAL_VectorRemoveAll(struct PAL_Vector* vector)
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

 int
PAL_VectorGetIndex(struct PAL_Vector *vector, void* item)
{
    int index = 0;
    for (index = 0; index < vector->size; index++)
    {
        if (nsl_memcmp(PAL_VectorGetPtr(vector, index), item, vector->itemsize) == 0)
        {
            return index;
        }
    }
    return -1;
}

 int
PAL_VectorGetItemSize(struct PAL_Vector *vector)
{
    return vector->itemsize;
}

 int
PAL_VectorLen(struct PAL_Vector *vector)
{
    if (vector == NULL)
    {
        return 0;
    }

    return (vector->size);
}

 void
PAL_VectorPop(struct PAL_Vector *vector, void *item)
{
    nsl_assert(vector->size > 0);
    PAL_VectorGet(vector, vector->size - 1, item);
    (void)resize(vector, vector->size - 1);
}

 void
PAL_VectorSet(struct PAL_Vector *vector, int index, const void *item)
{
    nsl_memcpy(PAL_VectorGetPtr(vector, index), item, vector->itemsize);
}

 int 
PAL_VectorSetSize(struct PAL_Vector *vector, int len)
{
    return resize(vector, len);
}

 int 
PAL_VectorSetAlloc(struct PAL_Vector *vector, int len)
{
    return _realloc(vector,len);
}

static int
simple_copy_item(void* pUser, void* pItemDest, const void* pItemSrc)
{
    nsl_memcpy(pItemDest, pItemSrc, *(int*)pUser);
    return 1;
}

 int
PAL_VectorCopy(struct PAL_Vector **v_dest, struct PAL_Vector *v_src, int (*copy_item)(void*, void *, const void *), void* puser)
{
    int n = 0;

    if (copy_item == NULL) 
    {
        copy_item = simple_copy_item; 
        puser = &v_src->itemsize;
    }

    *v_dest = PAL_VectorAlloc(v_src->itemsize);

    if (*v_dest == NULL)
    {
        return 0;
    }

    PAL_VectorSetAlloc(*v_dest, v_src->size);

    for (n = 0; n < v_src->size; n++) 
    {
        if (!resize(*v_dest, (*v_dest)->size+1))
        {
            return 0;
        }

        if (!copy_item(puser, PAL_VectorGetPtr(*v_dest, n), PAL_VectorGetPtr(v_src,n))) 
        {
            resize(*v_dest, (*v_dest)->size-1);
            return 0;
        }
    }

    return 1;
}

 int
PAL_VectorSort(struct PAL_Vector *vector, int (*cmp)(const void *, const void *))
{
    void* k = NULL;

    if (vector->size > 1) 
    {
        k = (void*) nsl_malloc(vector->itemsize);
        if (k == NULL)
        {
            return (0);
        }
        pal_heapsort(vector->items, vector->size, vector->itemsize, k, cmp);
        nsl_free(k);
    }
    return (1);
}


 int
PAL_VectorCoreSort(struct PAL_Vector *vector, int (*cmp)(const void *, const void *))
{
    void* k = NULL;

    if (vector->size > 1) 
    {
        k = (void*) nsl_malloc(vector->itemsize);
        if (k == NULL)
        {
            return (0);
        }
        pal_heapsort(vector->items, vector->size, vector->itemsize, k, cmp);
        nsl_free(k);
    }
    return (1);
}


 void 
PAL_VectorSort2(struct PAL_Vector *vector, int (*cmp)(const void *, const void *), void* tmp)
{
    if (vector->size > 1)
    {
        pal_heapsort(vector->items, vector->size, vector->itemsize, tmp, cmp);
    }
}

 size_t 
PAL_VectorGetStructureSize()
{
    return sizeof(struct PAL_Vector);
}

 int
PAL_VectorForEach(struct PAL_Vector* vector, int (*foreach)(void*, void*), void* userData)
{
    int i = 0;

    for (i = 0; i < vector->size; i++)
    {
        if (!(foreach)(userData, PAL_VectorGetPtr(vector, i)))
        {
            return 0;
        }
    }

    return 1;
}
