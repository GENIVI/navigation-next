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

    @file     nbsearchfilter.c
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The filter contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the NB_ContextGetDataState( filter->context ) of     
    California. Use of this filter by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "nbsearchfilter.h"
#include "nbsearchfilterprivate.h"
#include "nbcontextprotected.h"
#include "datautil.h"
#include "data_pair.h"
#include "data_string.h"
#include "vec.h"

struct NB_SearchFilter
{
    NB_Context*        context;
    CSL_Vector*        search_filter;
};


NB_DEF NB_Error
NB_SearchFilterCreate( NB_Context* context, NB_SearchFilter** filter )
{
    NB_SearchFilter* pThis = NULL;
    NB_Error err = NE_OK;

    if (context == NULL || filter == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc( sizeof( NB_SearchFilter ) );
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset( pThis, 0, sizeof( NB_SearchFilter ) );

    DATA_VEC_ALLOC( err, pThis->search_filter, data_pair );
    if (err == NE_OK)
    {
        pThis->context = context;
        *filter = pThis;
    }
    else
    {
        NB_SearchFilterDestroy( pThis );
    }

    return err;
}

NB_DEF NB_Error
NB_SearchFilterAdd( NB_SearchFilter* filter, const char* key, const char* value )
{
    NB_Error err = NE_OK;
    data_pair tmp = {0};

    if ( filter == NULL || filter->search_filter == NULL || key == NULL || value == NULL )
    {
        return NE_INVAL;
    }

    err = err ? err : data_pair_init( NB_ContextGetDataState( filter->context ), &tmp );
    err = err ? err : data_string_set( NB_ContextGetDataState( filter->context ), &tmp.key, key );
    err = err ? err : data_string_set( NB_ContextGetDataState( filter->context ), &tmp.value, value );

    if ( err == NE_OK )
    {
        if ( !CSL_VectorAppend( filter->search_filter, &tmp ) )
        {
            err = NE_NOMEM;
        }
    }

    if ( err != NE_OK )
    {
        data_pair_free( NB_ContextGetDataState( filter->context ), &tmp );
    }

    return NE_OK;
}

NB_DEF NB_Error
NB_SearchFilterDelete( NB_SearchFilter* filter, const char* key )
{
    int i = 0;
    uint32 length = 0;
    if ( filter == NULL || filter->search_filter == NULL || key == NULL )
    {
        return NE_INVAL;
    }

    length = CSL_VectorGetLength( filter->search_filter );
    for ( i = length - 1; i > -1; --i )
    {
        data_pair* tmp = ( data_pair* )CSL_VectorGetPointer( filter->search_filter, i );
        if ( tmp && nsl_strcmp(data_string_get(NB_ContextGetDataState(filter->context), &tmp->key), key) == 0)
        {
            CSL_VectorRemoveItem( filter->search_filter, tmp );
        }

    }

    return NE_OK;
}

NB_DEF NB_Error
NB_SearchFilterDestroy( NB_SearchFilter* filter )
{
    if ( filter == NULL )
    {
        return NE_INVAL;
    }

    DATA_VEC_FREE( NB_ContextGetDataState( filter->context ), filter->search_filter, data_pair );
    nsl_free( filter );

    return NE_OK;
}

NB_DEF NB_Error 
NB_SearchFilterSetFilter( NB_SearchFilter* filter, CSL_Vector** vec )
{
    NB_Error err = NE_OK;

    if ( vec == NULL )
    {
        return NE_INVAL;
    }
 
    if ( NULL != *vec )
    {
        DATA_VEC_FREE( NB_ContextGetDataState( filter->context ), *vec, data_pair);
    }

    DATA_VEC_ALLOC( err, *vec, data_pair);

    DATA_VEC_COPY( NB_ContextGetDataState( filter->context ), err, 
                   *vec,
                   filter->search_filter, data_pair );
    return err;
}

/*! @} */
