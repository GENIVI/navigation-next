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
 * data_pronun_query.c: created 2009/08/07 by Dat Nguyen.
 */

#include "data_pronun_query.h"
#include "data_blob.h"
#include "tpsdebug.h"
#include "cslcache.h"
#include "palmath.h"

NB_Error	
data_pronun_query_init(data_util_state* pds, data_pronun_query* pnq)
{
	NB_Error err = NE_OK;
	
    pnq->voiceName = CSL_VectorAlloc(sizeof(data_blob));

	err = err ? err : data_string_init(pds, &pnq->pronun_style);

	if (err)
		data_pronun_query_free(pds, pnq);

	return err;
}

void		
data_pronun_query_free(data_util_state* pds, data_pronun_query* pnq)
{	
    int n, l;
    if (pnq->voiceName) 
    {
        l = CSL_VectorGetLength(pnq->voiceName);

        for (n=0;n<l;n++)
            data_blob_free(pds, (data_blob*) CSL_VectorGetPointer(pnq->voiceName, n));

        CSL_VectorDealloc(pnq->voiceName);
    }

	data_string_free(pds, &pnq->pronun_style);	
}

tpselt
data_pronun_query_to_tps(data_util_state* pds, data_pronun_query* pnq)
{
	tpselt te = NULL;
	tpselt ce = NULL;
    int length = 0;
    int i=0;

	te = te_new("pronun-query");
    if (te == NULL)
    {
        goto errexit;
    }
    		    	
	if (!te_setattrc(te, "pronun-style", data_string_get(pds, &pnq->pronun_style)))
    {
		goto errexit;
    }

    if (pnq->voiceName == NULL)
    {
        goto errexit;
    }

    //sort the vector
    CSL_VectorSort(pnq->voiceName, &data_pronun_compare);

    length = CSL_VectorGetLength(pnq->voiceName);

    for (i=0; i<length; i++)
    {
        data_blob* voiceName = CSL_VectorGetPointer(pnq->voiceName, i);

        ce = te_new("pronun-item");

        if (ce == NULL)
        {
            goto errexit;
        }

        if (!te_setattr(ce, "name", (const char*) voiceName->data, voiceName->size))
        {
            goto errexit;
        }

        if (!te_attach(te, ce))
        {
            goto errexit;
        }
    }

	return te;

errexit:

	te_dealloc(te);
	te_dealloc(ce);

	return NULL;
}

NB_Error
data_pronun_query_add_voice(data_util_state* pds, data_pronun_query* pnq, const byte* name, size_t nameLen)
{
    NB_Error err = NE_OK;
    data_blob voiceName = {0};
    int i = 0;
    
    int length = CSL_VectorGetLength(pnq->voiceName);

    //check if pnq have contain the name, if not, append to voiceName
    for(i = 0; i< length; i++)
    {
        data_blob* tmpVoiceName = CSL_VectorGetPointer(pnq->voiceName, i);
        size_t min = MIN(tmpVoiceName->size, nameLen);
        if(nsl_memcmp(tmpVoiceName->data, name, min) == 0 && nameLen == tmpVoiceName->size)
        {
            return err;
        }
    }
    err = err ? err : data_blob_init(pds, &voiceName);
    err = err ? err : data_blob_set(pds, &voiceName, name, nameLen);    

    err = err ? err : CSL_VectorAppend(pnq->voiceName, &voiceName) ? NE_OK : NE_NOMEM;

    if (err)
        data_blob_free(pds, &voiceName);

    return err;
}

/*! Compares two pronun items

 @see data_pronun_query_add_voice
 @see data_blob

 @return refer to CSL_VectorCompareFunction
*/
int
data_pronun_compare(const void* left,		/* Left item to compare */
					const void* right)		/* Right item to compare */
{
    const data_blob* pronounLeft = (data_blob*) left;
    const data_blob* pronounRight = (data_blob*) right;

    int min = MIN(pronounLeft->size, pronounRight->size);

    int result = nsl_memcmp(pronounLeft->data, pronounRight->data, min);

    if (result == 0 && pronounLeft->size != pronounRight->size)
    {
        if (pronounLeft->size < pronounRight->size)
        {
            //e.g. jar-1122, jar-11223
            return -1;
        }
        else if (pronounLeft->size > pronounRight->size)
        {
            //e.g. jar-11222, jar-1
            return 1;
        }
    }

    return result;
}

