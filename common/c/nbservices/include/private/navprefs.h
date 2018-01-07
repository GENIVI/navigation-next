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
 * navprefs.h: created 2005/01/06 by Mark Goddard.
 */

#ifndef NAVPREFS_H
#define NAVPREFS_H

#include "nberror.h"
#include "paltypes.h"

#define NB_TURN_CODE_LEN	12	


// Public types ..................................................................................

// @todo: Move to the correct place once prefetch is implemented
typedef struct
{
	double	startHeight;
	double	heightInc;
	double	height;
	double	fieldOfView;
	double	scale;
	double	distanceFromTurn;

} NB_VectorMapPreferences;


// it is the same as NB_ManueverType
enum manuever_type 
{
	manuever_type_normal = 0,
	maneuver_type_hwy,
	maneuver_type_dest,
	maneuver_type_origin,
	maneuver_type_merge,	
	maneuver_type_ferry,
	maneuver_type_camera,
	maneuver_type_max  // placeholder to size arrays
};

// it is the same as NB_InstructDistanceType
enum instruct_dist_type 
{
    base_dist = 0,
    announce_dist,
    continue_dist,
    prepare_dist,
    prepare_min_dist,
    instruct_dist,
    show_stack_dist,
    show_vector_dist,
    lane_guidance_dist,
    dist_max
};

// it is the same as NB_NavigationInstruction
typedef struct 
{
	double		base_dist_mul;
	double		base_dist_add;
	double		mult[dist_max];

	double		stdi;	// Show tape distance in inches
	double		stdm;	// Show tape distance in meters
	double		tdm;	// Tape distance in meters
	double		tdi;	// Tape distance in inches

} navinstruct;

typedef struct 
{
	char		command[NB_TURN_CODE_LEN];
	uint32		time;

} navturntime;

typedef struct
{
    char                command[NB_TURN_CODE_LEN];
    enum manuever_type     type;
}navturnindex;

typedef struct LaneGuidanceFontMap
{
    uint32  key;
    char    unselectedChars[NB_TURN_CODE_LEN];
    char    selectedChars[NB_TURN_CODE_LEN];
}LaneGuidanceFontMap;

typedef struct HovFontMap
{
    char    arrowChars[NB_TURN_CODE_LEN];
    char    hovChars[NB_TURN_CODE_LEN];
}HovFontMap;
#endif
