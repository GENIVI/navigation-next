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
 * data_pronun_query.h: created 2009/08/06 by Dat Nguyen.
 */

#ifndef DATA_PRONUN_QUERY_H
#define DATA_PRONUN_QUERY_H

#include "datautil.h"
#include "data_cache_contents.h"
#include "data_map_cache_contents.h"
#include "data_route_style.h"
#include "data_string.h"
#include "data_recalc.h"
#include "data_detour_avoid.h"
#include "data_nav_reply.h"
#include "nbroutesettings.h"
#include "nbexp.h"
#include "cslcache.h"

typedef struct data_pronun_query_ {
	/* Attributes */
	data_string				pronun_style;
    struct CSL_Vector*		voiceName;
} data_pronun_query;

NB_Error	data_pronun_query_init(data_util_state* pds, data_pronun_query* pnq);
void		data_pronun_query_free(data_util_state* pds, data_pronun_query* pnq);
NB_Error    data_pronun_query_add_voice(data_util_state* pds, data_pronun_query* pnq, const byte* name, size_t nameLen);
tpselt		data_pronun_query_to_tps(data_util_state* pds, data_pronun_query* pnq);

int         data_pronun_compare(const void* left, const void* right);

#endif
