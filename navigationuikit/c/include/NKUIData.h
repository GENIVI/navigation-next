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
@file         NKUIData.h
@defgroup     nkui
*/
/*
(C) Copyright 2014 by TeleCommunications Systems, Inc.

The information contained herein is confidential, proprietary to
TeleCommunication Systems, Inc., and considered a trade secret as defined
in section 499C of the penal code of the State of California. Use of this
information by anyone other than authorized employees of TeleCommunication
Systems is granted only under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#ifndef __NKUI_NKUIData_h
#define __NKUI_NKUIData_h

#include "NavApiNavigation.h"

namespace nkui
{

/*! this struct include data show on LNURouteInfoView, this view include by rts header. */
typedef struct
{
    /*! traffic, it Should be 'R'= red 'G' = Green 'Y' yellow */
    char  traffic;
    /*! this time is arrival time, it's a gps time. */
    uint32 arrivalTime;
    /*! Trip distance text. */
    std::string tripDistance;
    /*! Trip time */
    std::string tripTime;
    /*! for rts is via, for detour is detour1/2/3  */
    std::string majorRoadPrefix;
    /*! major road name */
    std::string majorRoad;
    /*! when a route include a highway, HOV ..., it will be not empty. */
    std::string routeTipText;
    /*! trans portation mode */
    nbnav::TransportationMode mode;
}RouteInfoData;

/*!
 *  LNUManeuverInfoView type
 */
typedef enum
{
    MIT_INVALID = 0,
    MIT_Normal,
    MIT_Arrival,
    MIT_StartOffroute,
    MIT_Pedestrian
} LNUManeuverInfoViewType;

/*! Lane information describe by ture-type-font */
typedef struct
{
    uint16 character;
    uint32 color; // Format: RGBA.
}LaneFont;

#define LANE_ARROW_MAX 4
/*! A Lane information */
typedef struct
{
    LaneFont         laneType;                                       /*!< Lane type */
    LaneFont         laneDividerType;                                /*!< Divider Type */
    LaneFont         nonHighLightArrowDirections[LANE_ARROW_MAX];    /*!< Arrow in no highlight */
    LaneFont         highLightArrowDirections[LANE_ARROW_MAX];       /*!< Arrow in highlight */
} LaneItem;

/*!
 *  the data display on navigation header
 */
typedef struct
{
    /*! turn distance. */
    std::string distance;
    /*! the text show header's center. */
    std::string primaryText;
    /*! the text show below primary text field. */
    std::string secondText;
    /*! exit number show at right top corner. */
    std::string exitText;
    /*! turn arrow. */
    LaneFont turnArrow;
    /*! the display style. */
    LNUManeuverInfoViewType viewStyle;
} UINavigationHeaderData;

/*!
 *  the data display on naviagtion footer
 */
typedef struct
{
    /*! trip distance. */
    std::string distance;
    /*! traffic meter. have 3 type 'R' 'G' 'Y' */
    char traffic;
    /*! arrival time */
    std::string arrivalTime;
    /*! arrival title */
    std::string arrivalTitle;
} UINavigationFooterData;

struct MenuOption
{
    uint32          optionID;
    std::string     displayName;
};

typedef struct
{
    MenuOption option;
    uint32     position;
} ExtraMenuItem;

typedef struct
{
    double          latitude;
    double          longitude;
    std::string     name;
} PinBubbleParameter;

enum PreservedAudio
{
    PreservedAudio_PressStartToNavigate     = 0

    , PreservedAudio_Num
    , PreservedAudio_None = PreservedAudio_Num
};

enum AudioDataType
{
    AudioDataType_AAC       = 0
    , AudioDataType_TTS

    , AudioDataType_Num
};

typedef struct
{
    double left;
    double top;
    double width;
    double height;
}WidgetPosition;


}

#endif


/*! @} */
