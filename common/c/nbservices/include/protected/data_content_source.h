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

    @file     data_content_source.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_CONTENT_SOURCE_H
#define DATA_CONTENT_SOURCE_H

#include "datautil.h"
#include "data_url.h"

typedef struct data_content_source_
{
    /* Child Elements */
    boolean             internal_source;            /*!< If present the map source is owned by TCS */
    data_url            url;                        /*!< Base URL for all url-args-template elements */
    CSL_Vector*         vec_url_args_template;      /*!< This element specifies the URL template for a
                                                         particular type of extended data. */
    /* Attributes */
    data_string         gen;                        /*!< Generation of the map tiles
                                                         This attribute is deprecated. It should not be returned
                                                         if want-extended-maps is present in the query. */
    data_string         projection;                 /*!< Map projection type
                                                         Supported types : 'mercator', 'transverse-mercator',
                                                         or 'universal-transverse-mercator'. */
    data_string         country;                    /*!< Country code */

} data_content_source;

NB_Error    data_content_source_init(data_util_state* state, data_content_source* contentSource);
void        data_content_source_free(data_util_state* state, data_content_source* contentSource);

NB_Error    data_content_source_from_tps(data_util_state* state, data_content_source* contentSource, tpselt tpsElement);

boolean     data_content_source_equal(data_util_state* state, data_content_source* contentSource1, data_content_source* contentSource2);
NB_Error    data_content_source_copy(data_util_state* state, data_content_source* destinationContentSource, data_content_source* sourceContentSource);

#endif // DATA_CONTENT_SOURCE_H

/*! @} */
