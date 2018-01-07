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
 * data_string.c: created 2004/12/09 by Mark Goddard.
 */

#include "data_string.h"

NB_DEF NB_Error
data_string_init(data_util_state* pds, data_string* ps)
{
    *ps = NULL;

    return NE_OK;
}

NB_DEF void
data_string_free(data_util_state* pds, data_string* ps)
{
	if(ps != NULL && *ps != NULL)
	{
		nsl_free(*ps);
    	data_string_init(pds, ps);
    	ps = NULL;
	}
}

NB_DEF NB_Error
data_string_set(data_util_state* pds, data_string* ps, const char* val)
{
    NB_Error err;
    size_t len = val != NULL ? nsl_strlen(val) : 0;

    data_string_free(pds, ps);

    err = data_string_alloc(pds, ps, len+1);

    if (err == NE_OK)
        nsl_strcpy(*ps, val != NULL ? val : "");

    return err;
}

#if 0
/* BREW SPECIFIC */
NB_DEF NB_Error
data_string_setw(data_util_state* pds, data_string* ps, const AECHAR* val)
{
    NB_Error err;
    size_t len = val != NULL ? WSTRLEN(val) : 0;

    data_string_free(pds, ps);

    err = data_string_alloc(pds, ps, len+1);

    if (err == NE_OK) {
        if (val != NULL)
            WSTRTOSTR(val, *ps, len+1);
        else
            (*ps)[0] = 0;
    }

    return err;
}
#endif

NB_DEF NB_Error
data_string_set_i64(data_util_state* pds, data_string* ps, int64 val)
{
    char buf[64];

    nsl_int64tostr(val, buf, sizeof(buf));

    return data_string_set(pds, ps, buf);
}

NB_DEF NB_Error
data_string_set_u64(data_util_state* pds, data_string* ps, uint64 val)
{
    char buf[64];

    nsl_uint64tostr(val, buf, sizeof(buf));

    return data_string_set(pds, ps, buf);
}

NB_DEF NB_Error
data_string_from_tps_attr(data_util_state* pds, data_string* ps, tpselt te, const char* name)
{
    char* data;
    size_t size;

    if (te == NULL || name == NULL)
        return NE_INVAL;

    if (!te_getattr(te, name, &data, &size))
        return NE_INVAL;

    return data_string_set(pds, ps, data);
}

NB_DEF NB_Error
data_string_alloc(data_util_state* pds, data_string* ps, size_t size)
{
    *ps = nsl_malloc(size);

    if ((*ps) == NULL)
        return NE_NOMEM;

    (*ps)[0] = 0;

    return NE_OK;
}

NB_DEF const char*
data_string_get(data_util_state* pds, const data_string* ps)
{
    if ((*ps) == NULL)
        return "";
    else
        return (const char*) *ps;
}

NB_DEF void
data_string_get_copy(data_util_state* pds, const data_string* ps, char* buf, size_t bufsize)
{
    nsl_strlcpy(buf, data_string_get(pds, ps), bufsize);
}

#if 0
/* BREW SPECIFIC */
NB_DEF void
data_string_getw(data_util_state* pds, const data_string* ps, AECHAR* buf, size_t bufsize)
{
    const char* val = (const char*) *ps;

    if (val == NULL)
        val = "";

    STRTOWSTR(val, buf, bufsize);
}
#endif

NB_DEF boolean
data_string_equal(data_util_state* pds, data_string* ps1, data_string* ps2)
{
    return (boolean) (str_cmpx(*ps1,*ps2) == 0);
}

NB_DEF NB_Error
data_string_copy(data_util_state* pds, data_string* ps_dest, data_string* ps_src)
{
    data_string_free(pds, ps_dest);
    data_string_init(pds, ps_dest);

    if (*ps_src == NULL)
        return NE_OK;

    return data_string_set(pds, ps_dest, data_string_get(pds, ps_src));
}

NB_DEF boolean
data_string_compare_cstr(data_string* ps, const char* cs)
{
    return (boolean) (str_cmpx(*ps,cs) == 0);
}

NB_DEF void
data_string_to_buf(data_util_state* pds, data_string* ps, struct dynbuf *dbp)
{
    size_t len = 0;
    if (*ps != NULL)
    {
        len = nsl_strlen(*ps)+1;
    }

    dbufcat(dbp, (const byte*) &len, sizeof(len));
    if (len > 0)
        dbufcat(dbp, (const byte*) *ps, len);
}

NB_DEF NB_Error
data_string_from_binary(data_util_state* pds, data_string* ps,
                byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    int len;

    if (*pdatalen < sizeof(len))
        return NE_BADDATA;

    nsl_memcpy(&len, *ppdata, sizeof(len));

    *ppdata += sizeof(len);
    *pdatalen -= sizeof(len);

    if (len > 0)
    {
        err = data_string_set(pds, ps, (const char*) *ppdata);
        if (err == NE_OK) {

            *ppdata += len;
            *pdatalen -= len;
        }
    }
    else
    {
        data_string_free(pds, ps);
    }

    return err;
}

NB_DEF uint32
data_string_get_tps_size(data_util_state* pds, data_string* ps)
{
    if (*ps)
    {
        return (uint32)nsl_strlen(*ps);
    }
    else
    {
        return 0;
    }
}

