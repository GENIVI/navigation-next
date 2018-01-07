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

/*-
 * nav_avg_speed.h: created 2004/05/07 by Mark Goddard.
 *
 * Copyright (c) 2004 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 *
 */

#ifndef NAV_AVG_SPEED_H
#define	NAV_AVG_SPEED_H

#include "paltypes.h"
#include "cslutil.h"

struct nav_avg_speed {

	double	n;
	double  a;
	double  v;

	int		num_pts_for_valid;
	int		num_pts;

	double	default_speed;
	double	min_valid_speed;
	double	manuever_speed;

	int		min_update_below;
	int		min_update_above;

	uint32	last_time;
};

boolean		create_nav_avg_speed(struct nav_avg_speed* pnas, double a, int num_pts_for_valid, double default_speed, 
								 double min_valid_speed, int min_update_below, int min_update_above);
void		free_nav_avg_speed(struct nav_avg_speed* pnas);

void		reset_nav_avg_speed(struct nav_avg_speed* pnas, double maneuever_speed);
double		get_nav_avg_speed(struct nav_avg_speed* pnas);
void		add_nav_avg_speed(struct nav_avg_speed* pnas, double speed, uint32 time, boolean valid);

#endif

