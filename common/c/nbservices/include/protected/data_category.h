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
 * data_category.h: created 2005/01/02 by Mark Goddard.
 */

#ifndef DATA_CATEGORY_H
#define DATA_CATEGORY_H

#include "datautil.h"
#include "data_string.h"
#include "data_parent_category.h"
#include "nbplace.h"


typedef struct data_category_ {

    /* Child Elements */
    boolean hasParentCategory;              /*!< Is the parent category present? */
    data_parent_category parent_category;   /*!< Parent category of this category */

    /* Attributes */
    data_string     code;
    data_string     name;

} data_category;

NB_Error	data_category_init(data_util_state* pds, data_category* pp);
void		data_category_free(data_util_state* pds, data_category* pp);

NB_Error	data_category_from_tps(data_util_state* pds, data_category* pp, tpselt te);
tpselt		data_category_to_tps(data_util_state* pds, data_category* pp);

boolean		data_category_equal(data_util_state* pds, data_category* pp1, data_category* pp2);
NB_Error	data_category_copy(data_util_state* pds, data_category* pp_dest, data_category* pp_src);

NB_Error	data_category_from_nimcategory(data_util_state* pds, data_category* pp, const NB_Category* pCat);

uint32   data_category_get_tps_size(data_util_state* pds, data_category* pp);
void     data_category_to_buf(data_util_state* pds, data_category* pp, struct dynbuf* pdb);
NB_Error data_category_from_binary(data_util_state* pds, data_category* pp, byte** pdata, size_t* pdatalen);

#endif

