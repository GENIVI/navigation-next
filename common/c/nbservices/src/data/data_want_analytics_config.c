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

    @file     data_want_analytics_config.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#include "data_want_analytics_config.h"

NB_Error
data_want_analytics_config_init(data_util_state* pds,
        data_want_analytics_config* pwac)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pwac->event_categories, data_event_categories);

    return err;
}

void
data_want_analytics_config_free(data_util_state* pds,
        data_want_analytics_config* pwac)
{
    DATA_FREE(pds, &pwac->event_categories, data_event_categories);
}

tpselt
data_want_analytics_config_to_tps(data_util_state* pds,
        data_want_analytics_config* pwac)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("want-analytics-config");

    if (te == NULL)
    {
        goto errexit;
    }

    ce = data_event_categories_to_tps(pds, &pwac->event_categories);
    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_want_analytics_config_equal(data_util_state* pds,
        data_want_analytics_config* pwac1, data_want_analytics_config* pwac2)
{
    return (boolean) data_event_categories_equal(pds, &pwac1->event_categories,
                                                      &pwac2->event_categories);
}

NB_Error
data_want_analytics_config_copy(data_util_state* pds,
        data_want_analytics_config* pwac_dest,
        data_want_analytics_config* pwac_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pwac_dest, data_want_analytics_config);

    DATA_COPY(pds, err, &pwac_dest->event_categories,
              &pwac_src->event_categories, data_event_categories);

    return err;
}

uint32 data_want_analytics_config_get_tps_size(data_util_state* pds,
        data_want_analytics_config* pwac)
{
    return data_event_categories_get_tps_size(pds, &pwac->event_categories);
}

/*! @} */

