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

@file palvector.h

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

#ifndef PALVECTOR_H
#define	PALVECTOR_H

#include "palstdlib.h"

typedef struct PAL_Vector PAL_Vector;

/*! Allocates PAL_Vector* object

@param itemsize size of items (in bytes) in the vector object
@return On Success returns a PAL_Vector object, otherwise NULL
*/
PAL_DEC PAL_Vector *PAL_VectorAlloc(size_t itemsize);

/*! Adds an item to end of the vector

@param v Pointer to vector object
@param item pointer to object
@return 1 If successful
*/
PAL_DEC int PAL_VectorAppend(PAL_Vector *v, const void *item);

/*! Destroys vector object

@param v Pointer to vector object
*/
PAL_DEC void PAL_VectorDealloc(PAL_Vector *v);

/*! Gets the object in vector at specified index

@param v Pointer to vector object
@param index index of item to get
@param item pointer to object gotten
*/
PAL_DEC void PAL_VectorGet(PAL_Vector *v, int index, void *item);

/*! Gets the object in vector at specified index

@param v Pointer to vector object
@param index index of item to get
return pointer to object gotten
*/
PAL_DEC void *PAL_VectorGetPtr(PAL_Vector *v, int index);

/*! Removes the object in vector at specified index

@param v Pointer to vector object
@param index index of item to remove
*/
PAL_DEC void PAL_VectorRemove(PAL_Vector *v, int index);

/*! Removes all item from vector object

@param v Pointer to vector object
*/
PAL_DEC void PAL_VectorRemoveAll(PAL_Vector *v);

/*! Gets the item size in vector object

@param v Pointer to vector object
*/
PAL_DEC int PAL_VectorGetItemSize(PAL_Vector* v);

/*! Gets the index of the specified object in vector

@param v Pointer to vector object
@param item pointer to specified object
return int return index of item
*/
PAL_DEC int	PAL_VectorGetIndex(PAL_Vector *v, void* item);

/*! Gets number of items in the vector object

@param v Pointer to vector object
return int length of vector
*/
PAL_DEC int PAL_VectorLen(PAL_Vector *v);

/*! Pops the last item from the vector

@param v Pointer to vector object
@param item pointer to popped item
*/
PAL_DEC void PAL_VectorPop(PAL_Vector *v, void *item);

/*! Sets the object in vector at specified index to item

@param v Pointer to vector object
@param index index of item to get
@param item pointer to item to set to 
*/
PAL_DEC void PAL_VectorSet(PAL_Vector *v, int index, const void *item);

/*! Resizes vector to specified length

@param v Pointer to vector object
@param len size to set vector to
*/
PAL_DEC int PAL_VectorSetSize(PAL_Vector *v, int len);

/*! Reallocates vector to specified length

@param v Pointer to vector object
@param len size to resize vector to
*/
PAL_DEC int PAL_VectorSetAlloc(PAL_Vector *v, int len);

/*! Copies contents of one vector to another

@param v_dest destination vector
@param v_src source vector
@param function item copy function pointer
@param puser user of copy function
return int On Success returns 1
*/
PAL_DEC int PAL_VectorCopy(PAL_Vector **v_dest, PAL_Vector *v_src, int (*copy_item)(void*, void *, const void *), void* puser);

/*! Sorts contents of vector 

@param v Pointer to vector object
@param function compare function pointer
return int On Success returns 1
*/
PAL_DEC int PAL_VectorSort(PAL_Vector *v, int (*cmp)(const void *, const void *));

/*! Sorts contents of vector 

@param v Pointer to vector object
@param function compare function pointer
return int On Success returns 1
*/
PAL_DEC void PAL_VectorSort2(PAL_Vector *v, int (*cmp)(const void *, const void *), void* tmp);

/*! Sorts contents of vector 

return size_t gets size of vector object structure
*/
PAL_DEC size_t PAL_VectorGetStructSize(void);

/*! Allocates PAL_Vector* object at specific address

@param itemsize size of items (in bytes) in the vector object
@param p adress to allocate vector object
@return On Success returns a PAL_Vector object, otherwise NULL
*/
PAL_DEC PAL_Vector *PAL_VectorAllocPlace(size_t itemsize, void* p);

/*! Destroys vector object

@param v Pointer to vector object
*/
PAL_DEC void PAL_VectorDeallocPlace(PAL_Vector *v);

#endif
