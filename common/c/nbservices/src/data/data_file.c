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
 * data_file.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_file.h"

NB_DEF NB_Error
data_file_init(data_util_state* pds, data_file* f)
{
    NB_Error err = NE_OK;

    err = err ? err : data_blob_init(pds, &f->name);
    err = err ? err : data_blob_init(pds, &f->data);
    err = err ? err : data_play_time_init(pds, &f->play_time);

    return err;
}

NB_DEF void
data_file_free(data_util_state* pds, data_file* f)
{
    data_blob_free(pds, &f->name);
    data_blob_free(pds, &f->data);
    data_play_time_free(pds, &f->play_time);
}

NB_DEF NB_Error
data_file_from_tps(data_util_state* pds, data_file* f, tpselt te)
{
    NB_Error err = NE_OK;
    char*    data;
    size_t    size;
    tpselt ce = NULL;

    if (te == NULL)
    {
        return NE_INVAL;
    }

    data_file_free(pds, f);

    err = data_file_init(pds, f);

    if (err != NE_OK)
    {
        return err;
    }

    if (te_getattr(te, "data", &data, &size))
    {
        err = err ? err : data_blob_set(pds, &f->data, (byte*) data, size);
    }

    if (te_getattr(te, "name", &data, &size))
    {
        err = err ? err : data_blob_set(pds, &f->name, (byte*) data, size);
    }

    if ((ce = te_getchild(te, "play-time")) != NULL)
        err = err ? err : data_play_time_from_tps(pds, &f->play_time, ce);
    else
        err = err ? err : data_play_time_init(pds, &f->play_time);


        if (err != NE_OK)
    {
        data_file_free(pds, f);
    }

  return err;
}

NB_DEF boolean
data_file_equal(data_util_state* pds, data_file* f1, data_file* f2)
{
    return (boolean) (data_blob_equal(pds, &f1->name, &f2->name) &&
                      data_blob_equal(pds, &f1->data, &f2->data) && 
                      data_play_time_equal(pds, &f1->play_time, &f2->play_time)
                      );
}

NB_DEF NB_Error
data_file_copy(data_util_state* pds, data_file* f_dest, data_file* f_src)
{
    NB_Error err = NE_OK;

    data_file_free(pds, f_dest);

    err = err ? err : data_file_init(pds, f_dest);

    err = err ? err : data_blob_copy(pds, &f_dest->data, &f_src->data);
    err = err ? err : data_blob_copy(pds, &f_dest->name, &f_src->name);
    err = err ? err : data_play_time_copy(pds, &f_dest->play_time, &f_src->play_time);

    return err;
}

