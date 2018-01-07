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

/*!--------------------------------------------------------------------------

 @file     nbnaturallaneguidancetype.h
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/


#ifndef NB_NATURAL_LANE_GUIDANCE_TYPE_H
#define NB_NATURAL_LANE_GUIDANCE_TYPE_H

#include "paltypes.h"
#include "data_point.h"

// Divider Types
enum LaneDividerType
{
    LDT_LONG_DASHED   = 1,
    LDT_DOUBLE_SOLID  = 2,
    LDT_SINGLE_SOLID  = 3,
    LDT_SOLID_DASHED  = 4,
    LDT_DASHED_SOLID  = 5,
    LDT_SHORT_DASHED  = 6,
    LDT_SHADED_AREA   = 7,
    LDT_DASHED_BLOCKS = 8,
    LDT_PHYS_DIVIDER  = 9,
    LDT_DOUBLE_DASHED = 10,
    LDT_NONE          = 11,
    LDT_XING_ALERT    = 12,
    LDT_CTR_TURN_LANE = 13
};
// Bitmask for Arrow type
const static unsigned long AT_STRAIGHT     = 1;
const static unsigned long AT_SLIGHT_RIGHT = 2;
const static unsigned long AT_RIGHT        = 4;
const static unsigned long AT_HARD_RIGHT   = 8;
const static unsigned long AT_UTURN_LEFT   = 16;
const static unsigned long AT_HARD_LEFT    = 32;
const static unsigned long AT_LEFT         = 64;
const static unsigned long AT_SLIGHT_LEFT  = 128;
const static unsigned long AT_MERGE_RIGHT  = 256;
const static unsigned long AT_MERGE_LEFT   = 512;
const static unsigned long AT_MERGE_LANES  = 1024;
const static unsigned long AT_UTURN_RIGHT  = 2048;
const static unsigned long AT_SECOND_RIGHT = 4096;
const static unsigned long AT_SECOND_LEFT  = 8192;

// Bitmask for Lane Types
const static unsigned long LT_REGULAR     = 1;
const static unsigned long LT_HOV         = 2;
const static unsigned long LT_REVERSIBLE  = 4;
const static unsigned long LT_EXPRESS     = 8;
const static unsigned long LT_ACCEL       = 16;
const static unsigned long LT_DECEL       = 32;
const static unsigned long LT_AUX         = 64;
const static unsigned long LT_SLOW        = 128;
const static unsigned long LT_PASSING     = 256;
const static unsigned long LT_SHOULDER    = 512;
const static unsigned long LT_REGULATED   = 1024;
const static unsigned long LT_TURN        = 2048;
const static unsigned long LT_CENTER_TURN = 4096;
const static unsigned long LT_TRUCK_PARK  = 8192;
const static unsigned long LT_TOLL_STRUCT = 16384;


#define NB_DATA_ROAD_INFO_TEXT_LENGTH       20
#define NB_LANE_INFO_TEXT_LENGTH            20
#define NB_GUIDANCE_POINT_TEXT_LENGTH       20
#define NB_COMMAND_TEXT_LENGTH              8

enum road_info_type
{
    NB_ROAD_INFO_TYPE_CURRENT = 0,
    NB_ROAD_INFO_TYPE_TURN = 1
};
enum guidance_propuse
{
    NB_GUIDANCE_PROPUSE_NONE = 0,
    NB_GUIDANCE_PROPUSE_TURN = 1,
    NB_GUIDANCE_PROPUSE_PREPARE = 2,
    NB_GUIDANCE_PROPUSE_BOTH    =3
};
struct NB_QaLogRoadInfoData
{
    char primary[NB_DATA_ROAD_INFO_TEXT_LENGTH];
    char secondary[NB_DATA_ROAD_INFO_TEXT_LENGTH];
    char exit_number[NB_DATA_ROAD_INFO_TEXT_LENGTH];
    char exit_number_pronun[NB_DATA_ROAD_INFO_TEXT_LENGTH];
};
struct NB_QaLogLaneInfoData
{
    char turnCommane[NB_COMMAND_TEXT_LENGTH];
    int32 lane_position;
    int32 number_of_lanes;
    char lg_pronun[NB_LANE_INFO_TEXT_LENGTH];
    char lgprep_pronun[NB_LANE_INFO_TEXT_LENGTH];
};
struct NB_QaLogLaneItmeInfoData
{
    uint32 lane_item;
    uint32 divider_item;
    uint32 no_highlight_arrow_item;
    uint32 highlight_arrow_item;
};
struct NB_QaLogGuidanceInfoData
{
    char turnCommand[NB_COMMAND_TEXT_LENGTH];
    uint8 guidancePropuse;
    char gp_pronun[NB_GUIDANCE_POINT_TEXT_LENGTH];
    char gpprep_pronun[NB_GUIDANCE_POINT_TEXT_LENGTH];
    char description[NB_GUIDANCE_POINT_TEXT_LENGTH];
    double maneuver_point_offset;
    data_point point;
};

#endif
