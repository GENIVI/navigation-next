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

    @file   data_compact_address.h
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

#ifndef DATA_COMPACT_ADDRESS_H
#define DATA_COMPACT_ADDRESS_H

/*! @{ */

#include "datautil.h"
#include "dynbuf.h"
#include "data_line_separator.h"
#include "vec.h"
#include "nblocation.h"

typedef struct data_compact_address_
{
    /* Child Elements */
    boolean hasLineSeparator;               /*!< Is the line separator present? */
    data_line_separator line_separator;     /*!< If present, must insert between to line 1 and
                                                 line2 when address is shown in one line. If not
                                                 present for one line address, client shall put a
                                                 space between line1 and line2.*/
    CSL_Vector* vector_address_line;        /*!< Formatted address lines */

    /* Attributes */

} data_compact_address;

NB_Error    data_compact_address_init(data_util_state* state, data_compact_address* compactAddress);
void        data_compact_address_free(data_util_state* state, data_compact_address* compactAddress);

NB_Error    data_compact_address_from_tps(data_util_state* state, data_compact_address* compactAddress, tpselt tpsElement);

boolean     data_compact_address_equal(data_util_state* state, data_compact_address* compactAddress1, data_compact_address* compactAddress2);
NB_Error    data_compact_address_copy(data_util_state* state, data_compact_address* destinationCompactAddress, data_compact_address* sourceCompactAddress);

uint32      data_compact_address_get_tps_size(data_util_state* state, data_compact_address* compactAddress);
void        data_compact_address_to_buf(data_util_state* state, data_compact_address* compactAddress, struct dynbuf* buf);
NB_Error    data_compact_address_from_binary(data_util_state* state, data_compact_address* compactAddress, byte** data, size_t* dataSize);

NB_Error    data_compact_address_from_nimlocation(data_util_state* state, data_compact_address* compactAddress, const NB_Location* location);
NB_Error    data_compact_address_to_nimlocation(data_util_state* state, data_compact_address* compactAddress, NB_Location* location);

/*! @} */

#endif
