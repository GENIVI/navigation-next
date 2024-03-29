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

    @file     data_search_reply.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef DATA_SEARCH_REPLY_H
#define DATA_SEARCH_REPLY_H

/*! @{ */

#include "datautil.h"
#include "data_iter_result.h"
#include "data_fuel_pricesummary.h"

typedef struct data_search_reply_
{
    /* Child Elements */
    data_iter_result        iter_result;                /*!< Iteration state for this operation */
    CSL_Vector*             vector_proxmatch;           /*!< Results which match the criteria specified */
    CSL_Vector*             vector_suggest_match;       /*!< Results which represent suggestions. These may only be
                                                             present in the reply if the query used a result-style with
                                                             key = 'suggest'. */
    CSL_Vector*             vector_suggest_list;        /*!< List of suggest match. */
    CSL_Vector*             vector_file;                /*!< A set of files (including images) associated with the POI
                                                             matches. A file image might be common to multiple proxmatch
                                                             elements.*/
    data_fuel_pricesummary  fuel_pricesummary;          /*!< Present only if ALL search results are gas stations. This
                                                             element will never be present alongside suggestions. */

    /* Attributes */

} data_search_reply;

NB_Error    data_search_reply_init(data_util_state* state, data_search_reply* searchReply);
void        data_search_reply_free(data_util_state* state, data_search_reply* searchReply);

NB_Error    data_search_reply_from_tps(data_util_state* state, data_search_reply* searchReply, tpselt tpsElement);

boolean     data_search_reply_equal(data_util_state* state, data_search_reply* searchReply1, data_search_reply* searchReply2);
NB_Error    data_search_reply_copy(data_util_state* state, data_search_reply* destinationSearchReply, data_search_reply* sourceSearchReply);

/*! @} */

#endif
