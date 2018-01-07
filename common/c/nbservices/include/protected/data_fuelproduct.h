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
 * data_fuelproduct.h: created 2007/09/10 by Angie Sheha
 */

#ifndef DATA_FUELPRODUCT_H
#define DATA_FUELPRODUCT_H

#include "datautil.h"
#include "data_string.h"
#include "data_fueltype.h"
#include "data_price.h"
#include "nbexp.h"

typedef struct data_fuelproduct_ {

	/* Child Elements */
	data_price		price;
	data_fueltype	fuel_type;

	/* Attributes */
	data_string		units;

	boolean			average;
	boolean			low;

} data_fuelproduct;

NB_Error	data_fuelproduct_init(data_util_state* pds, data_fuelproduct* pp);
void		data_fuelproduct_free(data_util_state* pds, data_fuelproduct* pp);

NB_Error	data_fuelproduct_from_tps(data_util_state* pds, data_fuelproduct* pp, tpselt te);

boolean		data_fuelproduct_equal(data_util_state* pds, data_fuelproduct* pp1, data_fuelproduct* pp2);
NB_Error	data_fuelproduct_copy(data_util_state* pds, data_fuelproduct* pp_dest, data_fuelproduct* pp_src);

uint32   data_fuelproduct_get_tps_size(data_util_state* pds, data_fuelproduct* pp);
void     data_fuelproduct_to_buf(data_util_state* pds, data_fuelproduct* pp, struct dynbuf* pdb);
NB_Error data_fuelproduct_from_binary(data_util_state* pds, data_fuelproduct* pp, byte** pdata, size_t* pdatalen);

#endif

