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

    @file     data_auth_reply.c
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

#include "data_auth_reply.h"
#include "tpselt.h"
#include "data_feature.h"
#include "nbextappcontent.h"

NB_Error
data_auth_reply_init(data_util_state* pds, data_auth_reply* par)
{
    NB_Error err = NE_OK;

    par->status_code = 0;
    
    par->vec_feature = CSL_VectorAlloc(sizeof(data_feature));
    if (par->vec_feature == NULL)
    {
        return NE_NOMEM;
    }

	par->vec_message = CSL_VectorAlloc(sizeof(data_message));
	if (par->vec_message == NULL)
	{
		return NE_NOMEM;
	}    

	err = err ? err : data_client_stored_message_init(pds, &par->client_stored_message);
    err = err ? err : data_subscribed_message_init(pds, &par->subscribed_message);
    err = err ? err : data_pin_message_init(pds, &par->pin_message);

    par->client_stored_message_valid = FALSE;
    par->subscribed_message_valid = FALSE;
    par->pin_message_valid = FALSE;

    return err;
}


void
data_auth_reply_free(data_util_state* pds, data_auth_reply* par)
{
    int  l, n = 0;
    data_feature* pdata_feature = NULL;
	data_message* pdata_message = NULL;	

    if (par->vec_feature)
    {
        l = CSL_VectorGetLength(par->vec_feature);

        for (n = 0; n < l; n++)
        {
            pdata_feature = (data_feature*)CSL_VectorGetPointer(par->vec_feature, n);            
            data_feature_free(pds, pdata_feature);
        }
        CSL_VectorDealloc(par->vec_feature);
        par->vec_feature = NULL;
    }

	if (par->vec_message)
	{
		l = CSL_VectorGetLength(par->vec_message);

		for (n=0; n< l; n++)
		{
			pdata_message = (data_message*)CSL_VectorGetPointer(par->vec_message, n);
			data_message_free(pds, pdata_message);
		}
		CSL_VectorDealloc(par->vec_message);
		par->vec_message = NULL;
	}

	data_client_stored_message_free(pds, &par->client_stored_message);
    data_subscribed_message_free(pds, &par->subscribed_message);
    data_pin_message_free(pds, &par->pin_message);
}

NB_Error
data_auth_reply_from_tps(data_util_state* pds, data_auth_reply* par, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;
    int iter = 0;
    data_feature pdata_feature;
	data_message data_msg;

	err = data_feature_init(pds, &pdata_feature);

	err = data_message_init(pds, &data_msg);

    data_auth_reply_free(pds, par);

    err = data_auth_reply_init(pds, par);
    if (err != NE_OK)
    {
        return err;
    }

    if (te == NULL) 
    {
        err = NE_INVAL;
        goto errexit;
    }

	while ((ce = te_nextchild (te, &iter)) != NULL)
	{
		if (nsl_strcmp(te_getname(ce), "message") == 0)
		{
			err = err ? err : data_message_from_tps(pds, &data_msg, ce);

			if (err == NE_OK)
			{
				if (!CSL_VectorAppend(par->vec_message, &data_msg))
				{
					err = NE_NOMEM;
				}

				if (err == NE_OK)
				{
					err = err ? err : data_message_init(pds, &data_msg);
				}
				else
				{
					data_message_free(pds, &data_msg);
					goto errexit;
				}
			}
		}    
        else if (nsl_strcmp(te_getname(ce), "feature") == 0) 
        {
            err = err ? err : data_feature_from_tps(pds, &pdata_feature, ce);
            if (err == NE_OK) 
            {
                if (!CSL_VectorAppend(par->vec_feature, &pdata_feature))
                {
                    err = NE_NOMEM;
                }               

                if (err == NE_OK)
                {
                    nsl_memset(&pdata_feature, 0, sizeof(pdata_feature)); 
                }
                else
                {
                    data_feature_free(pds, &pdata_feature); 
					goto errexit;
                }
            }
        }
		else if (nsl_strcmp(te_getname(ce), "client-stored-message") == 0)	
		{
			err = err ? err : data_client_stored_message_from_tps(pds, &par->client_stored_message, ce);	
			par->client_stored_message_valid = TRUE;
		}
        else if (nsl_strcmp(te_getname(ce), "subscribed-message") == 0)
        {
            err = err ? err : data_subscribed_message_from_tps(pds, &par->subscribed_message, ce);
            par->subscribed_message_valid = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "pin-message") == 0)
        {
            err = err ? err : data_pin_message_from_tps(pds, &par->pin_message, ce);
            par->pin_message_valid = TRUE;
        }
    }

	par->status_code = te_getattru(te, "status-code");

errexit:
	data_message_free(pds, &data_msg);

	data_feature_free(pds, &pdata_feature);

    if (err != NE_OK)
    {
        data_auth_reply_free(pds, par);
    }

    return err;
}

NB_Error
data_auth_reply_copy(data_util_state* pds, data_auth_reply* par_dest, data_auth_reply* par_src)
{
    NB_Error err = NE_OK;
	int i, len;
	data_message	message;
	data_feature	feature;

    data_auth_reply_free(pds, par_dest);

    err = err ? err : data_auth_reply_init(pds, par_dest);

    par_dest->status_code = par_src->status_code;

    par_dest->client_stored_message_valid = par_src->client_stored_message_valid;
    if (par_dest->client_stored_message_valid)
    {
	    err = err ? err : data_client_stored_message_copy(pds, &par_dest->client_stored_message, &par_src->client_stored_message);
    }

    par_dest->subscribed_message_valid = par_src->subscribed_message_valid;
    if (par_dest->subscribed_message_valid)
    {
        err = err ? err : data_subscribed_message_copy(pds, &par_dest->subscribed_message, &par_src->subscribed_message);
    }
    par_dest->pin_message_valid = par_src->pin_message_valid;
    if(par_dest->pin_message_valid)
    {
        err = err ? err : data_pin_message_copy(pds, &par_dest->pin_message, &par_src->pin_message);
    }

    if (par_src->vec_message)
    {
	    len = CSL_VectorGetLength(par_src->vec_message);

	    for (i = 0; i < len && err == NE_OK; i++) {

		    err = err ? err : data_message_copy(pds, &message, CSL_VectorGetPointer(par_src->vec_message, i));
		    err = err ? err : CSL_VectorAppend(par_dest->vec_message, &message) ? NE_OK : NE_NOMEM;

		    if (err)
			    data_message_free(pds, &message);
		    else
			    err = err ? err : data_message_init(pds, &message);
	    }
    }

    if (par_src->vec_feature)
    {
	    len = CSL_VectorGetLength(par_src->vec_feature);

	    for (i = 0; i < len && err == NE_OK; i++) {

		    err = err ? err : data_feature_copy(pds, &feature, CSL_VectorGetPointer(par_src->vec_feature, i));
		    err = err ? err : CSL_VectorAppend(par_dest->vec_feature, &feature) ? NE_OK : NE_NOMEM;

		    if (err)
			    data_feature_free(pds, &feature);
		    else
			    err = err ? err : data_feature_init(pds, &feature);
	    }
    }

    return err;
}

int
data_auth_reply_num_feature(data_util_state* pds, data_auth_reply* par)
{
    return CSL_VectorGetLength(par->vec_feature);
}

int 
data_auth_reply_num_message(data_util_state* pds, data_auth_reply* par)
{
	return CSL_VectorGetLength(par->vec_message);
}

int32
data_auth_reply_extapp_features(data_util_state* pds, data_auth_reply* par)
{
    int32 ext_app_mask = NB_TPA_None;
    int i = 0;

    if (!pds || !par)
    {
        return ext_app_mask;
    }

    if (par->vec_feature)
    {
        data_feature* feature = NULL;
        int len = 0;

        len = CSL_VectorGetLength(par->vec_feature);

        for (i = 0; i < len; i++)
        {
            feature = (data_feature*)CSL_VectorGetPointer(par->vec_feature, i);
            if (feature)
            {
                if (feature->exp_app_feature)
                {
                   if (data_string_compare_cstr(&feature->name, "ASAPP"))
                    {
                        ext_app_mask |= NB_TPA_Aisle411;
                    }
                    else if (data_string_compare_cstr(&feature->name, "HSAPP"))
                    {
                        ext_app_mask |= NB_TPA_HopStop;
                    }
                    else if (data_string_compare_cstr(&feature->name, "FDAPP"))
                    {
                        ext_app_mask |= NB_TPA_Fandango;
                    }
                    else if (data_string_compare_cstr(&feature->name, "OTAPP"))
                    {
                        ext_app_mask |= NB_TPA_OpenTable;
                    }
                }
            }
        }
    }

    return ext_app_mask;
}

/*! @} */

