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

    @file     data_fileset.c
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

#include "data_fileset.h"
#include "data_file.h"


NB_Error
data_fileset_init(data_util_state* pds, data_fileset* fs)
{
    NB_Error err = NE_OK;

    fs->vec_files = CSL_VectorAlloc(sizeof(data_file));

    if (fs->vec_files == 0)
    {
        err = NE_NOMEM;
    }

    err = err ? err : data_blob_init(pds, &fs->name);
    fs->time_stamp = 0;

    if (err)
    {
        data_fileset_free(pds, fs);
    }

    return err;
}

void		
data_fileset_free(data_util_state* pds, data_fileset* fs)
{
    int l = 0;
    int n = 0;
    
    if (fs->vec_files)
    {
        l = CSL_VectorGetLength(fs->vec_files);

        for (n = 0; n < l; n++)
        {
            data_file_free(pds, (data_file*) CSL_VectorGetPointer(fs->vec_files, n));
        }
        CSL_VectorDealloc(fs->vec_files);
        fs->vec_files = NULL;
    }

    data_blob_free(pds, &fs->name);
}

NB_Error	
data_fileset_from_tps(data_util_state* pds, data_fileset* fs, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;

    int iter = 0;
    data_file file;

    char* data = 0;
    size_t size = 0;
    
    if (te == 0)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_fileset_free(pds, fs);

    data_file_init(pds, &file);

    err = data_fileset_init(pds, fs);

    if (err)
    {
        return err;
    }

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != 0)
    {
        if (nsl_strcmp(te_getname(ce), "file") == 0)
        {
            err = data_file_from_tps(pds, &file, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(fs->vec_files, &file))
                {
                    err = NE_NOMEM;
                }
                if (err == NE_OK)
                {
                    data_file_init(pds, &file);
                }
                else
                {
                    data_file_free(pds, &file);
                }
            }

            if (err != NE_OK)
            {
                goto errexit;
            }
        }
    }

    if (te_getattr(te, "name", &data, &size))
    {
        err = err ? err : data_blob_set(pds, &fs->name, (byte*) data, size);
    }
    else {
        err = NE_INVAL;
        goto errexit;
    }

    if (!te_getattru64(te, "ts", &fs->time_stamp))
    {
        err = NE_INVAL;
    }
    
errexit:
    if (err != NE_OK)
    {
        data_fileset_free(pds, fs);
    }
    return err;
}

boolean		
data_fileset_equal(data_util_state* pds, data_fileset* fs1, data_fileset* fs2)
{
	int ret = 0;
	int i = 0;
	
	int len = CSL_VectorGetLength(fs1->vec_files);

	ret = ((len == CSL_VectorGetLength(fs2->vec_files))
	         && data_blob_equal(pds, &fs1->name, &fs2->name)
	         && (fs1->time_stamp == fs2->time_stamp));
	
	for (i = 0; i < len && ret; i++)
	{
		ret = ret && data_file_equal(pds, (data_file*)CSL_VectorGetPointer(fs1->vec_files, i), (data_file*)CSL_VectorGetPointer(fs2->vec_files, i));
	}

	return (boolean) ret;
}

NB_Error	
data_fileset_copy(data_util_state* pds, data_fileset* fs_dest, data_fileset* fs_src)
{
    NB_Error err = NE_OK;
    int i = 0;
    int len = 0;
    
    data_file file;
    data_file_init(pds, &file);

    data_fileset_free(pds, fs_dest);
    err = err ? err : data_fileset_init(pds, fs_dest);
    
    len = CSL_VectorGetLength(fs_src->vec_files);

    for (i = 0; i < len && err == NE_OK; i++)
    {
        err = err ? err : data_file_copy(pds, &file, CSL_VectorGetPointer(fs_src->vec_files, i));
        err = err ? err : CSL_VectorAppend(fs_dest->vec_files, &file) ? NE_OK : NE_NOMEM;

        if (err)
        {
            data_file_free(pds, &file);
        }
        else
        {
            data_file_init(pds, &file);
        }
    }

    err = err ? err : data_blob_copy(pds, &fs_dest->name, &fs_src->name);
    fs_dest->time_stamp = fs_src->time_stamp;

    return err;
}


/*! @} */
