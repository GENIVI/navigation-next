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

/*--------------------------------------------------------------------------

(C) Copyright 2010 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "nbgmarray.h"
#include "palstdlib.h"
#include "nbrecommon.h"

struct NBRE_Array
{
    NBRE_ArrayElement* elements;
    uint32 size;
    uint32 bufferSize;
};

static PAL_Error
ArrayGrow(NBRE_Array* array)
{
    uint32 oldSize = array->bufferSize;
    NBRE_ArrayElement* oldElements = array->elements;

    uint32 newSize = oldSize + (oldSize>>1)  + 1;
    NBRE_ArrayElement* newElements = (NBRE_ArrayElement*) nsl_malloc(sizeof(NBRE_ArrayElement) * newSize);
    if(newElements == NULL)
    {
        return PAL_ErrNoMem;
    }
    nsl_memset(newElements, 0, sizeof(NBRE_ArrayElement) * newSize);

    nsl_memcpy(newElements, oldElements, sizeof(NBRE_ArrayElement)*oldSize);
    nsl_free(oldElements);

    array->elements = newElements;
    array->bufferSize = newSize;

    return PAL_Ok;
}

NBRE_Array*
NBRE_ArrayCreate(uint32 capability)
{
    NBRE_Array* result = (NBRE_Array*)nsl_malloc(sizeof(NBRE_Array));
    if(result == NULL)
    {
        return NULL;
    }
    nsl_memset(result, 0, sizeof(NBRE_Array));

    if(capability<1)
    {
        capability =1;
    }
    result->elements = (NBRE_ArrayElement*) nsl_malloc(sizeof(NBRE_ArrayElement) * capability);
    if(result->elements == NULL)
    {
        nsl_free(result);
        return NULL;
    }
    nsl_memset(result->elements, 0, sizeof(NBRE_ArrayElement) * capability);

    result->bufferSize = capability;
    return result;
}

void
NBRE_ArrayDestroy(NBRE_Array* array)
{
    if(array != NULL)
    {
        if(array->elements != NULL)
        {
            nsl_free(array->elements);
        }
        nsl_free(array);
    }
}

NBRE_ArrayElement
NBRE_ArrayGetAt(NBRE_Array* array, uint32 index)
{
    NBRE_ArrayElement result = NULL;
    if(array != NULL)
    {        
        nbre_assert(index<array->size);
        result = array->elements[index];
    }
    return result;
}

void
NBRE_ArraySetAt(NBRE_Array* array, uint32 index, NBRE_ArrayElement element)
{
    if(array != NULL)
    {
        nbre_assert(index<array->size);
        array->elements[index] = element;
    }
}

PAL_Error
NBRE_ArrayAppend(NBRE_Array* array, NBRE_ArrayElement element)
{
    PAL_Error err = PAL_ErrBadParam;
    if(array != NULL)
    {
        err = PAL_Ok;
        if( array->size >= array->bufferSize )
        {
            err = ArrayGrow(array);
        }
        if(err == PAL_Ok)
        {
            array->elements[array->size++] = element;
        }
    }
    return err;
}

uint32
NBRE_ArrayGetSize(NBRE_Array* array)
{
    if(array!=NULL)
    {
        return array->size;
    }
    return 0;
}

PAL_Error
NBRE_ArrayRemove(NBRE_Array* array, uint32 index, uint32 count)
{
    PAL_Error err = PAL_Ok;
    uint32 i=0;
    if(array == NULL)
    {
        return PAL_ErrBadParam;
    }
    if(index + count>array->size)
    {
        return PAL_ErrOverflow;
    }

    for(i=index+count; i<array->size;++i)
    {
        array->elements[i-count] = array->elements[i];
    }
    array->size -= count;

    nsl_memset(array->elements+array->size, 0, sizeof(NBRE_ArrayElement)*(array->bufferSize-array->size));
    return err;
}

PAL_Error
NBRE_ArrayClear(NBRE_Array* array)
{
    PAL_Error err = PAL_Ok;
    if(array == NULL)
    {
        return PAL_ErrBadParam;
    }
    array->size =0;

    nsl_memset(array->elements, 0, sizeof(NBRE_ArrayElement)*array->bufferSize);
    return err;
}

void NBRE_ArraySort(NBRE_Array* array, NBRE_CompareFunc compareFunc)
{
    if (array != NULL && compareFunc != NULL)
    {
        nsl_qsort(array->elements, array->size, sizeof(void*), compareFunc);
    }
}
