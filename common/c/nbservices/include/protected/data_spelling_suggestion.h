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

    @file     data_spelling_suggestion.h

    Interface for Spelling Suggestion TPS element.

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

#ifndef DATA_SPELLING_SUGGESTION_H
#define DATA_SPELLING_SUGGESTION_H

#include "datautil.h"
#include "data_search_filter.h"
#include "nbexp.h"

typedef struct data_spelling_suggestion_
{
    /* Child Elements */
    data_search_filter          search_filter;

    /* Attributes */

} data_spelling_suggestion;

NB_Error    data_spelling_suggestion_init(data_util_state* pds, data_spelling_suggestion* pss);
void        data_spelling_suggestion_free(data_util_state* pds, data_spelling_suggestion* pss);

NB_Error    data_spelling_suggestion_from_tps(data_util_state* pds, data_spelling_suggestion* pss, tpselt te);

boolean     data_spelling_suggestion_equal(data_util_state* pds, data_spelling_suggestion* pss1, data_spelling_suggestion* pss2);
NB_Error    data_spelling_suggestion_copy(data_util_state* pds, data_spelling_suggestion* pss_dest, data_spelling_suggestion* pss_src);

#endif

/*! @} */
