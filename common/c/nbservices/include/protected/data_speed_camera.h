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
 * data_speed_camera.h: created 2007/11/13 by Andreas Lundeen
 */

#ifndef DATA_SPEED_CAMERA_H
#define DATA_SPEED_CAMERA_H

#include "datautil.h"
#include "data_string.h"
#include "nbplace.h"

#define DATA_CAMERA_STATUS_ACTIVE		"A"
#define DATA_CAMERA_STATUS_EMPTY		"L"
#define DATA_CAMERA_STATUS_DEMOLISHED	"Z"

typedef struct data_camera_segment_
{
	boolean valid;
	boolean	on_route;
	uint32	maneuver;
	uint32	segment;
	double	tmp;
} data_camera_segment_t;

typedef struct data_speed_camera_ {

	/* Child Elements */
	data_camera_segment_t	camera_segment;

	/* Attributes */ 
	double					limit;
	double					heading;
	data_string				status;
	boolean					bi_directional;
	data_string				description;

} data_speed_camera;

NB_Error	data_speed_camera_init(data_util_state* pds, data_speed_camera* psc);
void		data_speed_camera_free(data_util_state* pds, data_speed_camera* psc);

NB_Error	data_speed_camera_from_tps(data_util_state* pds, data_speed_camera* psc, tpselt te);
tpselt		data_speed_camera_to_tps(data_util_state* pds, data_speed_camera* psc);
boolean		data_speed_camera_equal(data_util_state* pds, data_speed_camera* psc1, data_speed_camera* psc2);
NB_Error	data_speed_camera_copy(data_util_state* pds, data_speed_camera* psc_dest, data_speed_camera* psc_src);
double		data_speed_camera_get_speed_limit(data_speed_camera *psc);

uint32   data_speed_camera_get_tps_size(data_util_state* pds, data_speed_camera* psc);
void     data_speed_camera_to_buf(data_util_state* pds, data_speed_camera* psc, struct dynbuf* pdb);
NB_Error data_speed_camera_from_binary(data_util_state* pds, data_speed_camera* psc, byte** pdata, size_t* pdatalen);


#endif

