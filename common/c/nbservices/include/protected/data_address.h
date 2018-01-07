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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_address.h: created 2004/12/09 by Mark Goddard.
 */

#ifndef DATA_ADDRESS_H
#define DATA_ADDRESS_H

#include "datautil.h"
#include "data_string.h"
#include "data_extended_address.h"
#include "data_compact_address.h"
#include "nblocation.h"

#include "nbexp.h"

#define DATA_ADDRESS_ATTRS			\
	X(type, type, FALSE)			\
	X(sa, sa, FALSE)			\
	X(xstr, xstr, FALSE)			\
	X(str, str, FALSE)			\
	X(city, city, FALSE)			\
	X(county, county, FALSE)		\
	X(state, state, FALSE)			\
	X(postal, postal, FALSE)		\
	X(country, country, FALSE)		\
	X(airport, airport, FALSE)		\
	X(freeform, freeform, TRUE)		\
	X(country_name, country-name, TRUE)

typedef struct data_address_ {
    /* Child Elements */
    data_extended_address   extended_address;       /*!< If present, an extended formatted address (max of 5 lines)
                                                         for POI detail screen shall be provided. */
    data_compact_address    compact_address;        /*!< If present, a compact formatted address (max of 2 lines)
                                                         for list view shall be provided. */

// name is used to identify this variable in struct data_address, while
// wirename is going to be filled into tps protocol. optional flag indicates
// whether this attribute is optional in tsp protocol.
#define X(name, wirename, optional) data_string name;

	DATA_ADDRESS_ATTRS

#undef X
} data_address;

NB_Error	data_address_init(data_util_state* pds, data_address* pa);
void		data_address_free(data_util_state* pds, data_address* pa);

tpselt		data_address_to_tps(data_util_state* pds, data_address* pa);
NB_Error	data_address_from_tps(data_util_state* pds, data_address* pa, tpselt te);

boolean		data_address_equal(data_util_state* pds, data_address* pa1, data_address* pa2);
NB_Error	data_address_copy(data_util_state* pds, data_address* pa_dest, data_address* pa_src);

void		data_address_to_buf(data_util_state* pds, data_address* pa, struct dynbuf *dbp);
NB_Error	data_address_from_binary(data_util_state* pds, data_address* pa,
				byte** ppdata, size_t* pdatalen);

NB_Error	data_address_from_nimlocation(data_util_state* pds, data_address* pa, const NB_Location* pLocation);
NB_Error    data_address_to_nimlocation(data_util_state* state, data_address* address, NB_Location* location);

uint32   data_address_get_tps_size(data_util_state* pds, data_address* pa);

#endif

