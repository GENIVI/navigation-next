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

    @file     data_link.c
    @created  07/15/2009
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_link.h"

NB_DEF NB_Error
data_link_init(data_util_state* pds, data_link* pdl)
{
    NB_Error err = NE_OK;

    err = data_string_init(pds, &pdl->text);
    err = err ? err : data_string_init(pds, &pdl->href);

    return err;
}

NB_DEF void
data_link_free(data_util_state* pds, data_link* pdl)
{
    data_string_free (pds, &pdl->text);
    data_string_free (pds, &pdl->href);
}

NB_DEF NB_Error
data_link_from_tps(data_util_state* pds, data_link* pdl, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        return err;
    }

    data_link_free(pds, pdl);

    err = data_link_init(pds, pdl);

    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(pds, &pdl->href, te, "href");
    err = err ? err : data_string_from_tps_attr(pds, &pdl->text, te, "text");

    if (err != NE_OK)
    {
        data_link_free(pds, pdl);
    }

    return err;
}

NB_DEF boolean
data_link_equal(data_util_state* pds, data_link* pdl1, data_link* pdl2)
{
    return (boolean) (data_string_equal(pds, &pdl1->href, &pdl2->href) &&
                        data_string_equal(pds, &pdl1->text, &pdl2->text));
}

NB_DEF NB_Error
data_link_copy(data_util_state* pds, data_link* pdl_dest, data_link* pdl_src)
{
    NB_Error err = NE_OK;

    data_link_free(pds, pdl_dest);

    err = data_link_init(pds, pdl_dest);
    err = err ? err : data_string_copy(pds, &pdl_dest->href, &pdl_src->href);
    err = err ? err : data_string_copy(pds, &pdl_dest->text, &pdl_src->text);

    return err;
}

uint32 data_link_get_tps_size(data_util_state* pds, data_link* pdl)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pdl->text);
    size += data_string_get_tps_size(pds, &pdl->href);

    return size;
}

void data_link_to_buf(data_util_state* pds, data_link* pdl, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pdl->text, pdb);
    data_string_to_buf(pds, &pdl->href, pdb);
}

NB_Error data_link_from_binary(data_util_state* pds, data_link* pdl, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &pdl->text, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pdl->href, pdata, pdatalen);

    return err;
}

/*! @} */
