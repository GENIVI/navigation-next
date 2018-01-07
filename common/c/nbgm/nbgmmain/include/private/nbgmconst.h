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

    @file nbgmconst.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_CONST_H_
#define _NBGM_CONST_H_

#include "nbrecolor.h"
#include "palmath.h"
#include "nbremath.h"
#include "nbgmplatformconfig.h"
#include "nbgmcommon.h"

static const uint32 PACK_TEXTURE_IMAGE_WIDTH = 256;


#define FONT_NAME_SANS "00"
#define FONT_NAME_SANS_BOLD "01"
#define FONT_NAME_SERIF "10"
#define FONT_NAME_SERIF_BOLD "11"
#define FONT_NAME_SANS_ITALIC "001"
#define FONT_NAME_SANS_BOLD_ITALIC "011"
#define FONT_NAME_SERIF_ITALIC "101"
#define FONT_NAME_SERIF_BOLD_ITALIC "111"

static const float NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_WIDTH_SCALE = 2.0f;
static const float NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_FIRST_SEGMENT_LENGTH = METER_TO_MERCATOR(90.0f);
static const float NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_SECOND_SEGMENT_LENGTH = METER_TO_MERCATOR(46.0f);
static const float NAV_VECTOR_MANEUVER_TURN_ARROW_TAIL_SECOND_SEGMENT_MIN_LENGTH = METER_TO_MERCATOR(1.0f);
static const float NAV_VECTOR_MANEUVER_OUTLINE_PERCENTAGE = 0.25f;

static const float NAV_VECTOR_ROUTE_WIDTH = METER_TO_MERCATOR(16.0f);

static const NBRE_Color NAV_VECTOR_AVATAR_HALO_OUTLINE_COLOR(0.176f,0.667f,0.843f,0.95f);
static const NBRE_Color NAV_VECTOR_AVATAR_HALO_INNER_COLOR(0.176f,0.667f,0.843f,0.20f);

static const float LABEL_LAYOUT_REFRESH_DELAY_SECONDS = 0.7f;
static const float RENDER_DELAY_SECONDS_AFTER_DATA_SYNC = 1.0f;
// When road polyline direction changes over 45 degree, break at that point to avoid Z shape label
static const float LABEL_MAX_DIRECTION_CHANGE_COS = 0.71f;

static const float ROAD_BASIC_WIDTH = METER_TO_MERCATOR(15.f);

static const float BASIC_HEIGHT = METER_TO_MERCATOR(1500.f);

static const float VECTOR_MAP_MODEL_SCALE_FACTOR = 10000.0f;
static const float ECM_MAP_MODEL_SCALE_FACTOR = 0.01f;

static const float VECTOR_AVATA_SIZE_FACTOR = 20.0f;
static const float ECM_AVATA_SIZE_FACTOR = 4.0f;
static const float REFERENCE_AVATAR_BASIC_HEIGHT = 600.f;
//Follow me mode avatar size, in DP
static const float AVATAR_MAP_MODE_SIZE = 31.41f;

static const float POI_DEFAULT_DP_SIZE = 48.0f * 160.0f / 326.0f;
static const float MAP_PIN_DEFAULT_DP_SIZE = 64.0f * 160.0f / 326.0f;

//used for building purple route hight light
static const float ECM_ROUTE_POLYLINE_SGEMENT_LENGTH = 10.0f;
static const float ECM_ROUTE_POLYLINE_MIN_LENGTH = 0.0001f;

static const float NAV_ECM_ROUTE_POLYLINE_HEIGHT_OFFSET = 1.0f;
static const float NAV_ECM_ROUTE_POLYLINE_HALF_WIDTH = 1.2f;
static const float NAV_ECM_MANEUVER_TURN_ARROW_HEAD_LENGTH = 8.0f;
static const float NAV_ECM_MANEUVER_TURN_ARROW_HEAD_HALF_WIDTH = 3.0f;
static const float NAV_ECM_MANEUVER_TURN_ARROW_TAIL_BACKWARD_SEGMENT_LENGTH = 90.0f;
static const float NAV_ECM_MANEUVER_TURN_ARROW_TAIL_FORWARD_SEGMENT_LENGTH = 46.0f;
static const float NAV_ECM_MANEUVER_TURN_ARROW_TAIL_FORWARD_SEGMENT_MIN_LENGTH = 1.0f;

static const float NAV_ECM_ROUTE_MATCH_RANGE = 20.f;
static const uint8 NAV_ECM_SNAP_ROUTE_MAX_MATCH_NUMBER = 20;
static const float NAV_ECM_SNAP_ROUTE_HEADING_OFFSET = 45.f;

static const float NAV_ECM_ROUTE_WIDTH = 2.4f;
static const NBRE_Color NAV_ECM_ROUTE_COLOR(136/255.0f, 6/255.0f, 190/255.0f, 1.0f);
static const NBRE_Color NAV_ECM_ARROW_COLOR(0, 254/255.0f, 103/255.0f, 1.0f);

static const uint32 NBRE_INVALID_HEADING = 0xffffffff;

static const float MAP_DEFAULT_HEIGHT = 700.0f;

static const double VECTOR_EPSILON = 0.000000001;

// point shield distance
static const float POINT_SHIELD_SAME_NAME_DISTANCE_DP = 500.0f;
static const float POINT_SHIELD_SAME_TYPE_DISTANCE_DP = 240.0f;

static const float SATELLITE_TRANSPARENT_ALPHA = 0.55f;
static const float B3D_TRANSPARENT_ALPHA = 0.6f;

// flag min & max size in mm
#define STANDARD_FLAG_MIN_SIZE 4.5f
#define STANDARD_FLAG_MAX_SIZE 18.0f

// flag init size
#define STANDARD_FLAG_SIZE METER_TO_MERCATOR(45.0f)

#define VECTOR_POLYLINE_SHAPE_TEXTURE_NAME "VectorPolylineShapeTexture"
#define VECTOR_POLYLINE_COLOR_TEXTURE_NAME "VectorPolylineColorTexture"

#define VECTOR_OUTLINE_POLYLINE_TEXTURE_NAME "VectorOutlinePolylineTexture"

static const int8 MAP_PATH_ARROW_ZOOMLEVEL_RANGE[2] = {17, 22};

#define MIN_LAYOUT_ZOOM_LEVEL 11
#define MAX_LAYOUT_ZOOM_LEVEL 22
#define SUB_LEVELS_PER_ZOOM_LEVEL 2
#define PATH_ARROW_LEVEL_MIN (MAP_PATH_ARROW_ZOOMLEVEL_RANGE[0] - MIN_LAYOUT_ZOOM_LEVEL) * SUB_LEVELS_PER_ZOOM_LEVEL
#define PATH_ARROW_LEVEL_MAX ((MAP_PATH_ARROW_ZOOMLEVEL_RANGE[1] + 1 - MIN_LAYOUT_ZOOM_LEVEL) * SUB_LEVELS_PER_ZOOM_LEVEL - 1)

static int8 NAV_VIEW_ZOOM_LEVEL = 17;

// Object picking masks
#define PICKING_ELEMENT_MASK_STANDARD    0x1     // Standard object, such as polyline & polygons

// static poi label padding in mm
#define STATIC_POI_LABEL_PADDING_MM 0.66f

#endif
