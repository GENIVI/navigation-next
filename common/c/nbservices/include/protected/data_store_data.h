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

    @file     data_store_data.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

#ifndef DATA_STORE_DATA_H
#define DATA_STORE_DATA_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"
#include "data_place.h"
#include "nbmobilecoupons.h"
#include "data_image_urls.h"

typedef struct data_store_data_
{
    /* Child Elements */
    data_place     place;                /*!< Street address of retailer; may contain lat/lon information as well. */
    CSL_Vector*    broad_categories;     /*!< Zero or more broad category associated with this store or deal. */
    CSL_Vector*    categories;           /*!< Zero or more categories associated with this store or deal. */
    CSL_Vector*    sub_categories;       /*!< Zero or more sub-categories associated with this store or deal. */
    data_image_urls                      /*!< If present, this element contains links to store images and thumbnails. */
                   image_urls;
    /* Attributes */
    data_string    retailer_name;        /*!< Name of retailer (eg: Ralphs). */
    data_string    retailer_id;          /*!< Unique ID for retailer, on CSP system. */
    uint32         coupon_count;         /*!< Number of coupons found in the store (if available). */
} data_store_data;

NB_Error    data_store_data_init(data_util_state* state, data_store_data* storeData);
void        data_store_data_free(data_util_state* state, data_store_data* storeData);

NB_Error    data_store_data_from_tps(data_util_state* state, data_store_data* storeData, tpselt tpsElement);
tpselt      data_store_data_to_tps(data_util_state* state, data_store_data* storeData);

boolean     data_store_data_equal(data_util_state* state, data_store_data* left, data_store_data* right);
NB_Error    data_store_data_copy(data_util_state* state, data_store_data* dest, data_store_data* src);

NB_Error    data_store_data_from_nbstore(data_util_state* state, data_store_data* storeData, const NB_Store* store);

uint32      data_store_data_get_tps_size(data_util_state* state, data_store_data* storeData);
void        data_store_data_to_buf(data_util_state* state, data_store_data* storeData, struct dynbuf* buffer);
NB_Error    data_store_data_from_binary(data_util_state* state, data_store_data* storeData, byte** pdata, size_t* pdatalen);


/*! @} */

#endif
