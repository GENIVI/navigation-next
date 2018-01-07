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

/*********************************************************************/
/*                                                                   */
/* (C) Copyright 2000 - 2005 by Networks In Motion, Inc.             */
/*                                                                   */
/* The information contained herein is confidential and propvmetary  */
/* to Networks In Motion, Inc., and considered a trade secret as     */
/* defined in section 499C of the penal code of the State of         */
/* California.                                                       */
/*                                                                   */
/* This intellectual property is for internal use only by            */
/* Networks In Motion, Inc.  This source code contains propvmetary   */
/* information of Networks In Motion, Inc. and shall not be used or  */
/* copied by or disclosed to anyone other than a Networks In Motion, */
/* Inc. certified employee that has obtained written authorization   */
/* to view or modify said source code.                               */
/*                                                                   */
/*********************************************************************/

/*
 * data_vector_map.h: created 2005/06/03 by Mark Goddard.
 */

#ifndef data_vector_map_H
#define data_vector_map_H

#include "datautil.h"
#include "vec.h"
#include "data_road.h"
#include "data_polyline.h"
#include "nbexp.h"

typedef struct data_vector_map_ {

	/* Child Elements */
	struct CSL_Vector*			vec_roads;

	/* Attributes */

} data_vector_map;

NB_Error	data_vector_map_init(data_util_state* pds, data_vector_map* pvm);
void		data_vector_map_free(data_util_state* pds, data_vector_map* pvm);

NB_Error	data_vector_map_from_tps(data_util_state* pds, data_vector_map* pvm, tpselt te);
NB_Error	data_vector_map_copy(data_util_state* pds, data_vector_map* pvm_dest, data_vector_map* pvm_src);

#endif

