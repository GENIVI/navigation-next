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

    @file     data_recommended_option.c
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

#include "data_recommended_option.h"

NB_Error data_recommended_option_init(data_util_state* pds, data_recommended_option* pro)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pro->formatted_text, data_formatted_text);

    return err;
}

void data_recommended_option_free(data_util_state* pds, data_recommended_option* pro)
{
    DATA_FREE(pds, &pro->formatted_text, data_formatted_text);
}

NB_Error data_recommended_option_copy(data_util_state* pds, data_recommended_option* pro_dest, data_recommended_option* pro_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pro_dest, data_recommended_option);

    DATA_COPY(pds, err, &pro_dest->formatted_text, &pro_src->formatted_text, data_formatted_text);

    return err;
}

NB_Error data_recommended_option_from_tps(data_util_state* pds, data_recommended_option* pro, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt	ce;
    int iter;

    DATA_REINIT(pds, err, pro, data_recommended_option);

    if (err != NE_OK)
        goto errexit;

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if(nsl_strcmp(te_getname(ce), "formatted-text") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pro->formatted_text, data_formatted_text);
        }

        if (err != NE_OK) {

            goto errexit;
        }
    }
errexit:
    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_recommended_option_from_tps failed with error: %d", err));
        DATA_FREE(pds, pro, data_recommended_option);
    }

    return err;
}

/*! @} */