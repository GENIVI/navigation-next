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

    @file     data_facebook_event.c
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.                
                                                                  
    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_spelling_suggestion.h"

NB_Error
data_spelling_suggestion_init(data_util_state* pds, data_spelling_suggestion* pss)
{
    NB_Error err = NE_OK;

    err = err ? err : data_search_filter_init(pds, &pss->search_filter);

    return err;
}

void	
data_spelling_suggestion_free(data_util_state* pds, data_spelling_suggestion* pss)
{
    data_search_filter_free(pds, &pss->search_filter);
}

NB_Error
data_spelling_suggestion_from_tps(data_util_state* pds, data_spelling_suggestion* pss, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pss, data_spelling_suggestion);

    if (err != NE_OK)
    {
        return err;
    }
    
    ce = te_getchild(te, "search-filter");
    if (ce == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }
    DATA_FROM_TPS(pds, err, ce, &pss->search_filter, data_search_filter);


errexit:
    if (err != NE_OK)
    {
        DATA_FREE(pds, pss, data_spelling_suggestion);
    }

    return err;
}

boolean
data_spelling_suggestion_equal(data_util_state* pds, data_spelling_suggestion* lhs, data_spelling_suggestion* rhs)
{
    int ret = TRUE;

    ret = ret && data_search_filter_equal(pds, &lhs->search_filter, &rhs->search_filter);

    return (boolean) ret;
}

NB_Error
data_spelling_suggestion_copy(data_util_state* pds, data_spelling_suggestion* dst, data_spelling_suggestion* src)
{
    NB_Error err = NE_OK;

    data_spelling_suggestion_free(pds, dst);
    data_spelling_suggestion_init(pds, dst);

    err = err ? err : data_search_filter_copy(pds, &dst->search_filter, &src->search_filter);

    return err;
}

/*! @} */
