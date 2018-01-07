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
 * navutil.h: created 2005/01/06 by Mark Goddard.
 */

#ifndef NAVUTIL_H
#define NAVUTIL_H

#include "navtypes.h"
#include "navprefs.h"
#include "nbgpstypes.h"
#include "data_nav_reply.h"
#include "instructset.h"

/* Initialization */
void init_navinstruct(navinstruct* instruct,
                      double bdm,            /*!< base multiplier */
                      double bda,            /*!< base addition */
                      double adm,            /*!< announce multiplier*/
                      double cdm,            /*!< continue multiplier*/
                      double pdm,            /*!< prepare multiplier*/
                      double pmm,            /*!< prepare minimum multiplier*/
                      double idm,            /*!< instruct multiplier*/
                      double xdm,            /*!< show statck multiplier*/
                      double vdm,            /*!< show vector multiplier*/
                      double ldm,            /*!< lane guidance multiplier*/
                      double tdi,            /*!< Show tape distance in inches*/
                      double tdm,            /*!< Show tape distance in meters*/
                      double stdi,           /*!< Tape distance in inches*/
                      double stdm            /*!< Tape distance in meters*/
                      );

/* Heading Operations */
boolean compare_heading(double heading1, double heading2, double tolerance, double* pdiff);
boolean heading_between(double heading1, double heading2, double heading_test, double buffer);

/* GPS Uncertainty Calculations */
double unc_to_pt(NB_GpsLocation* pfix, double pt_lat, double pt_lon);

/* Route Position Operations */
void clear_route_position(struct route_position* pos);

/* Maneuver Type Determination */
enum manuever_type get_maneuver_type(const char* szturn);
boolean is_informational(const char* szturn);
boolean is_speedcamera(const char* szturn);
boolean is_ferry_entry(const char* szturn);
boolean is_ferry_exit(const char* szturn);
boolean is_stay(const char* szturn);
boolean is_stay_highway(const char* szturn);
boolean is_merge(const char* szturn);
boolean is_keep(const char* szturn);
boolean is_tunnel_entry(const char* szturn);
boolean is_roundaboutexit(const char* szturn);
boolean is_unsupported_man(const char* szturn);

boolean get_maneuver_display(data_util_state* pdus, data_nav_reply* preply, uint32 nman, uint32* pdispnum, uint32* pcount);

NB_Error nav_get_file(PAL_Instance* pal, void* pUser, const char* isname, const char* isid, const char* name, size_t name_len, char** pbuf, nb_size* psize);
NB_Error nav_format(void* pUser, uint32 type, void* data, uint32 flags, char* buf, size_t size, int nIndex);

void convert_reserved_phrases(instructset* steps, data_util_state* pdus, data_nav_reply* pnr);

/* Get average heading of heading1 and heading2. */
double get_average_heading(double heading1, double heading2);

/* Set name changed(NC) maneuver enabled. */
void set_namechanged_maneuver_enabled(nb_boolean enabled);

/* Get M1 maneuver enabled */
nb_boolean get_m1_maneuver_enabled();

/* Set M1 maneuver enabled */
void set_m1_maneuver_enabled(nb_boolean enabled);

const char get_traffic_color(double delayPercentage, boolean isTrafficValid);
#endif
