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
 * data_phone.h: created 2005/01/02 by Mark Goddard.
 */

#ifndef DATA_PHONE_H
#define DATA_PHONE_H

#include "datautil.h"
#include "data_formatted_phone.h"
#include "data_string.h"

#include "nbplace.h"

#include "nbexp.h"

typedef struct data_phone_ {

    /* Child Elements */
    data_formatted_phone    formatted_phone;    /*!< A string of formatted phone number */

    /* Attributes */
    data_string     kind;
    data_string     country;
    data_string     area;
    data_string     number;

} data_phone;

NB_DEC  NB_Error	data_phone_init(data_util_state* pds, data_phone* pp);
NB_DEC  void		data_phone_free(data_util_state* pds, data_phone* pp);

NB_DEC  NB_Error	data_phone_from_tps(data_util_state* pds, data_phone* pp, tpselt te);
NB_DEC  tpselt		data_phone_to_tps(data_util_state* pds, data_phone* pp);

NB_DEC  boolean		data_phone_equal(data_util_state* pds, data_phone* pp1, data_phone* pp2);
NB_DEC  NB_Error	data_phone_copy(data_util_state* pds, data_phone* pp_dest, data_phone* pp_src);

NB_DEC  NB_Error	data_phone_from_nimphone(data_util_state* pds, data_phone* pp, const NB_Phone* pPhone);

uint32   data_phone_get_tps_size(data_util_state* pds, data_phone* pp);
void     data_phone_to_buf(data_util_state* pds, data_phone* pp, struct dynbuf* pdb);
NB_Error data_phone_from_binary(data_util_state* pds, data_phone* pp, byte** pdata, size_t* pdatalen);

#endif

