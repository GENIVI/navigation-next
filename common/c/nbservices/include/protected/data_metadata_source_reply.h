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

    @file     data_metadata_source_reply.h
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

#ifndef DATA_METADATA_SOURCE_REPLY_H
#define DATA_METADATA_SOURCE_REPLY_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"

typedef struct data_metadata_source_reply_
{
    /* Child Elements */
    CSL_Vector*     vec_content_source;     /*!< Information about content source
                                                 Maybe referred to as maptile-source for backwards compatibility. */
    CSL_Vector*     vec_maptile_source;     /*!< Information about maptile source.
                                                 This is included for backward compatibility only and will only be
                                                 returned if no wanted-content elements were sent in the query,
                                                 otherwise this data will be sent in a content-source element.*/
    data_string     city_summary_display;   /*!< Display text of available city model content
                                                 Only returned if city models were requested. */
    CSL_Vector*     vec_extapp;             /*!< Ext app vector. */

    /* Attributes */
    uint64          time_stamp;             /*!< Time stamp associated with returned metadata*/

} data_metadata_source_reply;

NB_Error    data_metadata_source_reply_init(data_util_state* state, data_metadata_source_reply* metadataSourceReply);
void        data_metadata_source_reply_free(data_util_state* state, data_metadata_source_reply* metadataSourceReply);

NB_Error    data_metadata_source_reply_from_tps(data_util_state* state, data_metadata_source_reply* metadataSourceReply, tpselt tpsElement);

/*! @} */

#endif
