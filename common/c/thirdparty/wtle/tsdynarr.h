/*****************************************************************************
 *
 *  tsdynarr.h - Defines the interface for a dynamic array object.
 *
 *  Copyright (C) 2003-2010 Monotype Imaging Inc. All rights reserved.
 *
 *  Confidential Information of Monotype Imaging Inc.
 *
 ****************************************************************************/

#ifndef TSDYNARR_H
#define TSDYNARR_H

#include "tstypes.h"
#include "tsmem.h"

TS_BEGIN_HEADER

/*****************************************************************************
 *
 * Macro defines the minimum initial size of the dynamic array.
 *
 *  <GROUP tsdynarr>
 */
#define TS_DEFAULT_MINIMUM_SIZE_GROWTH 5

/*****************************************************************************
 *
 * Macro defines the minimum size of the dynamic array, below which it cannot be reduced while shrinking.
 *
 *  <GROUP tsdynarr>
 */
#define TS_DEFAULT_MINIMUM_SIZE_SHRINK 10

/*****************************************************************************
 *
 *  Defines method for initializing TsDynamicArray elements
 *
 *  <GROUP tsdynarr>
 */
typedef TsResult (*TsObjectInitFunc)(void *object);

/*****************************************************************************
 *
 *  Defines method for copying TsDynamicArray elements
 *
 *  <GROUP tsdynarr>
 */
typedef TsResult (*TsObjectCopyFunc)(void *objectDest, void *objectSrc);

/*****************************************************************************
 *
 *  Defines method for comparing elements in a TsDynamicArray
 *
 *  Description:
 *      The compare function should return an integer greater than, equal to,
 *      or less than zero, accordingly as the object pointed to by objectOne is
 *      greater than, equal to, or less than the object pointed to by objectTwo.
 *
 *      With an appropriately defined TsObjectCompareFunc that only compares
 *      the "key", the TsDynamicArray can be used as a sort of dictionary
 *      where each element contains a "key" and a "value".
 *
 *  <GROUP tsdynarr>
 */
typedef TsInt32 (*TsObjectCompareFunc)(void *objectOne, void *objectTwo);

/*****************************************************************************
 *
 *  Defines method that is called on an element in a TsDynamicArray before
 *  that element is removed.
 *
 *  <GROUP tsdynarr>
 */
typedef void (*TsObjectDoneFunc)(void *object);

struct TsDynamicArray_
{
    TsMemMgr *memMgr;
    TsLength length;
    TsLength capacity;
    TsUInt32 growthElems;
    TsUInt32 shrinkElems;
    TsBool isSorted;

    void *array;

    TsSize objSize;
    TsObjectInitFunc init;
    TsObjectCopyFunc copy;
    TsObjectCompareFunc compare;
    TsObjectDoneFunc done;
};

/*****************************************************************************
 *
 *  Dynamic array structure
 *
 *  <GROUP tsdynarr>
 */
typedef struct TsDynamicArray_ TsDynamicArray;


/*****************************************************************************
 *
 *  Creates a new TsDynamicArray object.
 *
 *  Parameters:
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      objectSize  - [in] size of each element of array
 *      init        - [in] pointer to function to initialize element or NULL
 *      copy        - [in] pointer to function to copy element or NULL
 *      compare     - [in] pointer to function to compare elements or NULL
 *      done        - [in] pointer to function to finish element or NULL
 *
 *  Return value:
 *      Pointer to new TsDynamicArray object or NULL upon error.
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsDynamicArray *)
TsDynamicArray_new(TsMemMgr *memMgr, TsSize objectSize,
                   TsObjectInitFunc init,
                   TsObjectCopyFunc copy,
                   TsObjectCompareFunc compare,
                   TsObjectDoneFunc done);


/*****************************************************************************
 *
 *  Initializes a memory-managed TsDynamicArray object.
 *
 *  Parameters:
 *      da          - [in] this
 *      memMgr      - [in] pointer to a memory manager object or NULL
 *      objectSize  - [in] size of each element of array
 *      init        - [in] pointer to function to initialize element or NULL
 *      copy        - [in] pointer to function to copy element or NULL
 *      compare     - [in] pointer to function to compare elements or NULL
 *      done        - [in] pointer to function to finish element or NULL
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsResult)
TsDynamicArray_initex(TsDynamicArray *da, TsMemMgr *memMgr, TsSize objectSize,
                      TsObjectInitFunc init,
                      TsObjectCopyFunc copy,
                      TsObjectCompareFunc compare,
                      TsObjectDoneFunc done);


/*****************************************************************************
 *
 *  Initializes a non-memory-managed TsDynamicArray object.
 *
 *  Parameters:
 *      da          - [in] this
 *      objectSize  - [in] size of each element of array
 *      init        - [in] pointer to function to initialize element or NULL
 *      copy        - [in] pointer to function to copy element or NULL
 *      compare     - [in] pointer to function to compare elements or NULL
 *      done        - [in] pointer to function to finish element or NULL
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsResult)
TsDynamicArray_init(TsDynamicArray *da, TsSize objectSize,
                    TsObjectInitFunc init,
                    TsObjectCopyFunc copy,
                    TsObjectCompareFunc compare,
                    TsObjectDoneFunc done);



/*****************************************************************************
 *
 *  Sets the minimum number of elements by which a TsDynamicArray object grows.
 *
 *  Parameters:
 *      da          - [in] this
 *      count       - [in] minimum number of elements by which to grow
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsResult)
TsDynamicArray_setGrowthElemCount(TsDynamicArray *da, TsUInt32 count);


/*****************************************************************************
 *
 *  Gets the minimum number of elements by which a TsDynamicArray object grows.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      TsUInt32 - minimum number of elements by which to grow
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsUInt32)
TsDynamicArray_getGrowthElemCount(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Sets the number of elements of extra capacity above which a TsDynamicArray
 *  object shrinks when TsDynamicArray_shrink() is called.
 *
 *  Parameters:
 *      da          - [in] this
 *      count       - [in] maximum number of extra elements of capacity
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsResult)
TsDynamicArray_setShrinkElemCount(TsDynamicArray *da, TsUInt32 count);


/*****************************************************************************
 *
 *  Gets the number of elements of extra capacity above which a TsDynamicArray
 *  object shrinks when TsDynamicArray_shrink() is called.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      TsUInt32 - maximum number of elements of extra capacity
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsUInt32)
TsDynamicArray_getShrinkElemCount(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Copies one TsDynamicArray to another.
 *
 *  Parameters:
 *      da          - [out] destination array
 *      daSrc       - [in]  source array
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsResult)
TsDynamicArray_copy(TsDynamicArray *da, TsDynamicArray *daSrc);


/*****************************************************************************
 *
 *  Clears the contents of a TsDynamicArray, calling the done method
 *  (if not NULL) on each element.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void)
TsDynamicArray_clear(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Grows a TsDynamicArray to twice the sum of the existing length and the number of elements to be added.
 *
 *  Parameters:
 *      da          - [in] this
 *      elements    - [in] number of elements to be added.
 *
 *  Return value:
 *      pointer to first of the new elements
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void *)
TsDynamicArray_grow(TsDynamicArray *da, TsLength elements);


/*****************************************************************************
 *
 *  Insert an object into a TsDynamicArray.  If there is already an identical
 *  element in the array, it will NOT be inserted again.  Element comparison
 *  is done using the provided TsObjectCompareFunc, if provided at
 *  TsDynamicArray creation or initialization, otherwise using the binary
 *  comparison macro "TS_MEMCMP()".
 *
 *  Parameters:
 *      da          - [in] this
 *      object      - [in] object to insert
 *
 *  Return value:
 *      index at which object was inserted (or found if it was already there),
 *      -1 if an error occurred
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsIndex)
TsDynamicArray_insert(TsDynamicArray *da, void *object);


/*****************************************************************************
 *
 *  Insert an object into a TsDynamicArray.  If there is already an identical
 *  element in the array, it WILL be inserted again.  Element comparison
 *  is done using the provided TsObjectCompareFunc, if provided at
 *  TsDynamicArray creation or initialization, otherwise using the binary
 *  comparison macro "TS_MEMCMP()".
 *
 *  Parameters:
 *      da          - [in] this
 *      object      - [in] object to insert
 *
 *  Return value:
 *      index at which object was inserted, -1 if an error occurred
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsIndex)
TsDynamicArray_insertDuplicates(TsDynamicArray *da, void *object);


/*****************************************************************************
 *
 *  Insert an object into a TsDynamicArray before the element currently at
 *  the specified index.
 *
 *  Parameters:
 *      da          - [in] this
 *      object      - [in] object to insert
 *      index       - [in] index of element before which to insert new element
 *
 *  Return value:
 *      index at which object was inserted, -1 if an error occurred
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsIndex)
TsDynamicArray_insertBefore(TsDynamicArray *da, void *object, TsIndex index);


/*****************************************************************************
 *
 *  Append an object to the end of a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *      object      - [in] object to append
 *
 *  Return value:
 *      index at which object was appended, -1 if an error occurred
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsIndex)
TsDynamicArray_append(TsDynamicArray *da, void *object);


/*****************************************************************************
 *
 *  Replace an object in a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *      object      - [in] new object to replace old object
 *      index       - [in] index at which to make replacement
 *
 *  Return value:
 *      result - TS_OK if all went well, error code otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsResult)
TsDynamicArray_replace(TsDynamicArray *da, void *object, TsIndex index);


/*****************************************************************************
 *
 *  Get a pointer to an object at a specific index in a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *      index       - [in] index of object
 *
 *  Return value:
 *      pointer to object at index, NULL if not found
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void *)
TsDynamicArray_element(TsDynamicArray *da, TsIndex i);


/*****************************************************************************
 *
 *  Get a pointer to the last object in a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      pointer to last object in array, NULL if array empty
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void *)
TsDynamicArray_last(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Get a length of a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      length of array (number of elements)
 *
 *  Notes:
 *      This macro is equivalent to this functional form:
 *      <CODE>
 *          TS_EXPORT(TsLength)
 *          TsDynamicArray_length(TsDynamicArray *da);
 *      </CODE>
 *
 *
 *  <GROUP tsdynarr>
 */
#define TsDynamicArray_length(da) (da)->length


/*****************************************************************************
 *
 *  Get a pointer to an object in a TsDynamicArray that matches an object.
 *  Element comparison is done using the provided TsObjectCompareFunc, if
 *  provided at TsDynamicArray creation or initialization, otherwise using
 *  the binary comparison macro "TS_MEMCMP()".  With this method and an
 *  appropriately defined TsObjectCompareFunc that only compares the "key",
 *  the TsDynamicArray can be used as a sort of dictionary where each element
 *  contains a "key" and a "value".
 *
 *  Parameters:
 *      da          - [in] this
 *      compare     - [in] object to compare elements against
 *
 *  Return value:
 *      pointer to matching element of array, NULL if not found
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void *)
TsDynamicArray_find(TsDynamicArray *da, void *compare);


/*****************************************************************************
 *
 *  Get an index to an object in a TsDynamicArray that matches an object.
 *  Element comparison is done using the provided TsObjectCompareFunc, if
 *  provided at TsDynamicArray creation or initialization, otherwise using
 *  the binary comparison macro "TS_MEMCMP()".
 *
 *  Parameters:
 *      da          - [in] this
 *      compare     - [in] object to compare elements against
 *
 *  Return value:
 *      index to matching element of array, -1 if not found
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsIndex)
TsDynamicArray_findIndex(TsDynamicArray *da, void *compare);


/*****************************************************************************
 *
 *  Remove an object from a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *      index       - [in] index of object to remove
 *
 *  Return value:
 *      TRUE if successful, FALSE otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsBool)
TsDynamicArray_remove(TsDynamicArray *da, TsIndex i);


/*****************************************************************************
 *
 *  Remove the last object from a TsDynamicArray
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      TRUE if successful, FALSE otherwise
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsBool)
TsDynamicArray_removeLast(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Reduces the size of the TsDynamicArray to twice of what is needed.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      number of elements by which the array was reduced
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsSize)
TsDynamicArray_shrink(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Reduces the capacity of the TsDynamicArray to the minimum necessary to
 *  contain its current contents.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      number of elements by which the array was reduced
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(TsSize)
TsDynamicArray_compact(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Calls the done method (if not NULL) for all elements in the
 *  TsDynamicArray, then frees the memory for those elements.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void)
TsDynamicArray_done(TsDynamicArray *da);


/*****************************************************************************
 *
 *  Delete a TsDynamicArray. Calls the done method (if not NULL) on each
 *  element, frees the elements, then frees the TsDynamicArray itself.
 *
 *  Parameters:
 *      da          - [in] this
 *
 *  Return value:
 *      void
 *
 *  <GROUP tsdynarr>
 */
TS_EXPORT(void)
TsDynamicArray_delete(TsDynamicArray *da);


TS_END_HEADER

#endif /* TSDYNARR_H */

