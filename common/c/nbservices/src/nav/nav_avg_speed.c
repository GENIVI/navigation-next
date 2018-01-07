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
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
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
 * nav_avg_speed.c: created 2004/05/07 by Mark Goddard
 */

#include "nav_avg_speed.h"

boolean	
create_nav_avg_speed(struct nav_avg_speed* pnas, double a, int num_pts_for_valid, double default_speed, 
					 double min_valid_speed, int min_update_below, int min_update_above)
{
	pnas->v = 0;
	pnas->n = 0;
	pnas->a = a;
	pnas->num_pts = 0;

	pnas->num_pts_for_valid = num_pts_for_valid;
	pnas->default_speed = default_speed;
	pnas->min_valid_speed = min_valid_speed;
	pnas->manuever_speed = default_speed;

	pnas->min_update_below = min_update_below;
	pnas->min_update_above = min_update_above;
	pnas->last_time = 0;

	return TRUE;
}

void									
free_nav_avg_speed(struct nav_avg_speed* pnas)
{
	pnas->num_pts_for_valid = 0;
	pnas->default_speed = 0.0; 
	pnas->v = 0;
	pnas->n = 0;
	pnas->a = 0;
	pnas->num_pts = 0;
}

void									
reset_nav_avg_speed(struct nav_avg_speed* pnas, double maneuever_speed)
{
	pnas->v = 0;
	pnas->n = 0;
	pnas->num_pts = 0;
	pnas->manuever_speed = maneuever_speed;

	if (pnas->manuever_speed <= 0.0)
		pnas->manuever_speed = pnas->default_speed;

	pnas->last_time = 0;
}

double								
get_nav_avg_speed(struct nav_avg_speed* pnas)
{
	if (pnas->num_pts >= pnas->num_pts_for_valid)
		return pnas->v / pnas->n;
	else
		return pnas->manuever_speed;
}

void									
add_nav_avg_speed(struct nav_avg_speed* pnas, double speed, uint32 time, boolean valid)
{
	double v = 0.0;

	if (!valid)
		return;

	if (speed < pnas->min_valid_speed) {

		if ((pnas->last_time + pnas->min_update_below) > time)
			return;

		v = pnas->min_valid_speed;
	}
	else {

		if ((pnas->last_time + pnas->min_update_above) > time)
			return;

		v = speed;
	}

	pnas->n = 1.0 + (pnas->a * pnas->n);
	pnas->v = v + (pnas->a * pnas->v);
	pnas->num_pts++;
	pnas->last_time = time;
}

