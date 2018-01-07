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

    @file   data_extended_address.h
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef DATA_EXTENDED_ADDRESS_H
#define DATA_EXTENDED_ADDRESS_H

/*! @{ */

#include "datautil.h"
#include "dynbuf.h"
#include "vec.h"
#include "nblocation.h"

typedef struct data_extended_address_
{
    /* Child Elements */
    CSL_Vector* vector_address_line;        /*!< Formatted address lines */

    /* Attributes */


} data_extended_address;

NB_Error    data_extended_address_init(data_util_state* state, data_extended_address* extendedAddress);
void        data_extended_address_free(data_util_state* state, data_extended_address* extendedAddress);

NB_Error    data_extended_address_from_tps(data_util_state* state, data_extended_address* extendedAddress, tpselt tpsElement);

boolean     data_extended_address_equal(data_util_state* state, data_extended_address* extendedAddress1, data_extended_address* extendedAddress2);
NB_Error    data_extended_address_copy(data_util_state* state, data_extended_address* destinationExtendedAddress, data_extended_address* sourceExtendedAddress);

uint32      data_extended_address_get_tps_size(data_util_state* state, data_extended_address* extendedAddress);
void        data_extended_address_to_buf(data_util_state* state, data_extended_address* extendedAddress, struct dynbuf* buf);
NB_Error    data_extended_address_from_binary(data_util_state* state, data_extended_address* extendedAddress, byte** data, size_t* dataSize);

NB_Error    data_extended_address_from_nimlocation(data_util_state* state, data_extended_address* extendedAddress, const NB_Location* location);
NB_Error    data_extended_address_to_nimlocation(data_util_state* state, data_extended_address* extendedAddress, NB_Location* location);

/*! @} */

#endif
