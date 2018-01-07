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

/*
 * data_formatted_text.c: created 2006/10/11 by Chetan Nagaraj.
 */

#include "data_formatted_text.h"
#include "vec.h"

NB_DEF NB_Error
data_formatted_text_init(data_util_state* pds, data_formatted_text* pft)
{

    NB_Error err = NE_OK;

    pft->vec_format_elements = CSL_VectorAlloc(sizeof(data_format_element));

    if (pft->vec_format_elements == NULL)
        err = NE_NOMEM;

    return err;

}

NB_DEF void
data_formatted_text_free(data_util_state* pds, data_formatted_text* pft)
{
    int i, len;

    if (pft->vec_format_elements) {
        len = CSL_VectorGetLength(pft->vec_format_elements);

        for (i=0; i<len; i++)
            data_format_element_free(pds, (data_format_element*) CSL_VectorGetPointer(pft->vec_format_elements, i));

        CSL_VectorDealloc(pft->vec_format_elements);
    }

    pft->vec_format_elements = NULL;

}

NB_DEF NB_Error
data_formatted_text_from_tps(data_util_state* pds, data_formatted_text* pft, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    data_format_element fmt_elt;
    int iter;

    err = data_format_element_init(pds, &fmt_elt);

    if (err != NE_OK)
        return err;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_formatted_text_free(pds, pft);

    err = err ? err : data_formatted_text_init(pds, pft);

    if (err != NE_OK)
        goto errexit;

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {
        err = data_format_element_from_tps(pds, &fmt_elt, ce);

        if (err == NE_OK) {

            if (!CSL_VectorAppend(pft->vec_format_elements, &fmt_elt)) 
                err = NE_NOMEM;

            if (err == NE_OK)
                err = data_format_element_init(pds, &fmt_elt);
            else
                data_format_element_free(pds, &fmt_elt);

            if (err != NE_OK)
                goto errexit;                
        }
    }

errexit:
    if (err != NE_OK)
        data_formatted_text_free(pds, pft);

    return err;

}

NB_DEF boolean
data_formatted_text_equal(data_util_state* pds, data_formatted_text* pft1, data_formatted_text* pft2)
{
    int ret;
    int i, len;

    ret = ((len = CSL_VectorGetLength(pft1->vec_format_elements)) == (CSL_VectorGetLength(pft2->vec_format_elements)));

    for (i=0; i<len && ret; i++)
        ret = ret && data_format_element_equal(pds, (data_format_element*)CSL_VectorGetPointer(pft1->vec_format_elements, i), (data_format_element*)CSL_VectorGetPointer(pft2->vec_format_elements, i));

    return (boolean) ret;

}

NB_DEF NB_Error
data_formatted_text_copy(data_util_state* pds, data_formatted_text* pft_dest, data_formatted_text* pft_src)
{
    NB_Error err = NE_OK;
    data_format_element fmt_elt;
    int i, len;

    err = data_format_element_init(pds, &fmt_elt);

    if (err != NE_OK)
        goto exit;

    data_formatted_text_free(pds, pft_dest);

    err = data_formatted_text_init(pds, pft_dest);

    if (err != NE_OK)
        goto exit;

    len = CSL_VectorGetLength(pft_src->vec_format_elements);

    for (i=0; i<len && err == NE_OK; i++) {
        err = data_format_element_copy(pds, &fmt_elt, (data_format_element*) CSL_VectorGetPointer(pft_src->vec_format_elements, i));
        err = err ? err : CSL_VectorAppend(pft_dest->vec_format_elements, &fmt_elt) ? NE_OK : NE_NOMEM;

        if (err == NE_OK)
            data_format_element_init(pds, &fmt_elt);
        else
            data_format_element_free(pds, &fmt_elt);

        if (err != NE_OK)
            goto exit;
    }

exit:
    return err;

}

NB_DEF void
data_formatted_text_to_buf(data_util_state* pds, data_formatted_text* pft, struct dynbuf* pdb)
{
    int i = 0, len = 0;

    if (pft->vec_format_elements == NULL)
        return;

    len = CSL_VectorGetLength(pft->vec_format_elements);
    dbufcat(pdb, (const byte*)&len, sizeof(len));

    for (i=0; i<len; i++)
        data_format_element_to_buf(pds, (data_format_element*)CSL_VectorGetPointer(pft->vec_format_elements, (int)i), pdb);
}

NB_DEF NB_Error
data_formatted_text_from_binary(data_util_state* pds, data_formatted_text* pft, byte** pdata, size_t* pdatalen)
{

    NB_Error err = NE_OK;
    data_format_element dfe;
    int i = 0, len = 0;

    err = err ? err : data_uint32_from_binary(pds, (uint32*)&len, pdata, pdatalen);

    if (err != NE_OK)
        goto errexit;

    for (i=0; i<len; i++) {
        err = data_format_element_init(pds, &dfe);

        if (err != NE_OK)
            break;

        err = err ? err : data_format_element_from_binary(pds, &dfe, pdata, pdatalen);

        if (err == NE_OK) {
            if (!CSL_VectorAppend(pft->vec_format_elements, &dfe))
                err = NE_NOMEM;
        }

        if (err != NE_OK)
            break;
    }


errexit:
    return err;

}

uint32 data_formatted_text_get_tps_size(data_util_state* pds, data_formatted_text* pft)
{
    uint32 size = 0;
    int i = 0;
    int length = 0;

    length = CSL_VectorGetLength(pft->vec_format_elements);
    size += sizeof(length);
    for (i = 0; i < length; i++)
    {
        size += data_format_element_get_tps_size(pds, CSL_VectorGetPointer(pft->vec_format_elements, i));
    }

    return size;
}
