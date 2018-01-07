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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_cache_contents.c: created 2005/01/06 by Mark Goddard.
 */

#include "data_cache_contents.h"

static void
data_cache_contents_clear_variables(data_util_state* pds, data_cache_contents* pcc)
{
	
}

static data_blob *
find_cache_entry( data_cache_contents *cache, byte *name, size_t namelen);

NB_Error	
data_cache_contents_init(data_util_state* pds, data_cache_contents* pcc)
{
	NB_Error err = NE_OK;

	pcc->vec_cache_item = CSL_VectorAlloc(sizeof(data_cache_item));
	
	if (pcc->vec_cache_item == NULL)
		err = NE_NOMEM;

	err = err ? err : data_pair_init(pds, &pcc->pronun_style_pair);

	data_cache_contents_clear_variables(pds, pcc);

	if (err)
		data_cache_contents_free(pds, pcc);

	return err;
}

void		
data_cache_contents_free(data_util_state* pds, data_cache_contents* pcc)
{
	int n,l;

	if (pcc->vec_cache_item) {
		l = CSL_VectorGetLength(pcc->vec_cache_item);

		for (n=0;n<l;n++)
			data_cache_item_free(pds, (data_cache_item*) CSL_VectorGetPointer(pcc->vec_cache_item, n));

		CSL_VectorDealloc(pcc->vec_cache_item);
	}

	pcc->vec_cache_item = NULL;

	data_pair_free(pds, &pcc->pronun_style_pair);

	data_cache_contents_clear_variables(pds, pcc);
}

NB_Error
data_cache_contents_copy(data_util_state* pds, data_cache_contents* dst, data_cache_contents* src)
{
    NB_Error err = NE_OK;
    int n, l;
    data_cache_item	ce;

    err = data_cache_item_init(pds, &ce);

    l = CSL_VectorGetLength(src->vec_cache_item);
    for (n = 0; n < l && err == NE_OK; n++) 
    {
    	data_blob *de = CSL_VectorGetPointer(src->vec_cache_item, n);
    	data_blob *fe = find_cache_entry(dst,de->data, de->size);
    	if (fe == NULL)
    	{
			err = err ? err : data_cache_item_copy(pds, &ce, (data_cache_item*)de);
			err = err ? err : CSL_VectorAppend(dst->vec_cache_item, &ce) ? NE_OK : NE_NOMEM;
			if (err)
				data_cache_item_free(pds, &ce);
    	}

        data_cache_item_init(pds, &ce);
    }

    err = err ? err : data_pair_copy(pds, &dst->pronun_style_pair, &src->pronun_style_pair);

    return err;
}

tpselt		
data_cache_contents_to_tps(data_util_state* pds, data_cache_contents* pcc)
{
	tpselt te;
	tpselt ce = NULL;
	int n,l;
	data_cache_item* pci = NULL;

	te = te_new("cache-contents");

	if (te == NULL)
		goto errexit;

	if (pcc->vec_cache_item) {
		l = CSL_VectorGetLength(pcc->vec_cache_item);

		for (n=0;n<l;n++) {

			pci = (data_cache_item*) CSL_VectorGetPointer(pcc->vec_cache_item, n);

			if ((ce = data_cache_item_to_tps(pds, pci)) != NULL && te_attach(te, ce))
				ce = NULL;
			else
				goto errexit;
		}
	}

	if (nsl_strlen(data_string_get(pds, &pcc->pronun_style_pair.key)) > 0 &&
		nsl_strlen(data_string_get(pds, &pcc->pronun_style_pair.value)) > 0) {

		if ((ce = data_pair_to_tps(pds, &pcc->pronun_style_pair)) != NULL && te_attach(te, ce))
			ce = NULL;
		else
			goto errexit;
	}

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);
	return NULL;
}

NB_Error	
data_cache_contents_clear(data_util_state* pds, data_cache_contents* pcc)
{
  NB_Error err = NE_OK;

  DATA_VEC_FREE(pds, pcc->vec_cache_item, data_cache_item);
  DATA_VEC_ALLOC(err, pcc->vec_cache_item, data_cache_item);

	return err;
}

NB_Error	
data_cache_contents_add(data_util_state* pds, data_cache_contents* pcc, data_cache_item* pce)
{
	NB_Error err = NE_OK;
	data_cache_item	ce;

	data_cache_item_init(pds, &ce);

	err = err ? err : data_cache_item_copy(pds, &ce, pce);
	err = err ? err : CSL_VectorAppend(pcc->vec_cache_item, &ce) ? NE_OK : NE_NOMEM;

	if (err)
		data_cache_item_free(pds, &ce);

	return err;
}

int			
data_cache_contents_num(data_util_state* pds, data_cache_contents* pcc)
{
	return CSL_VectorGetLength(pcc->vec_cache_item);
}

data_blob *find_cache_entry( data_cache_contents *cache, byte *name, size_t namelen)
{
    struct CSL_Vector *vcache = cache->vec_cache_item;
    // check if pThis->dataQuery.cache_contents contain the cache item
    int i, length = CSL_VectorGetLength(vcache);
    for (i = 0; i < length; i++)
    {
        data_blob* tmpCacheItem = CSL_VectorGetPointer(vcache, i);
        if (tmpCacheItem->size == namelen && nsl_memcmp(tmpCacheItem->data, name, namelen) == 0)
        {
            return tmpCacheItem;
        }
    }
    return 0;
}

