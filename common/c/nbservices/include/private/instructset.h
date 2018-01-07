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
 * instructset.h: created 2005/01/12 by Mark Goddard.
 */

#ifndef INSTRUCTSET_H
#define INSTRUCTSET_H

#include "nberror.h"
#include "paltypes.h"
#include "palstdlib.h"
#include "navpublictypes.h"
#include "formatfunc.h"
#include "cslutil.h"
#include "nbcontext.h"


// Public constants ..............................................................................

#define TEXT_FONT_TOKEN        0x01        // followed by an AEEFont
#define TEXT_COLOR_TOKEN       0x02        // followed by an RGBVAL
#define TEXT_TEXT_TOKEN        0x03        // followed by a uint32 for length (l) and l characters
#define TEXT_NEWLINE_TOKEN     0x04        // standalone token

#define TPSLIB_FILE_NAME  "@tpslib@"

/*! Maximum number/length for instruction codes used in audio callback. */
#define MAX_INSTRUCTION_CODES       30
#define MAX_INSTRUCTION_CODE_LENGTH 40

#define DISTANCE_CONFIG_INVALID_VALUE            -1.0

// bitmasks for soundTypes
#define STACK_SOUND_MASK         1       // message contains stacked instruction
#define REAL_TIME_DATA_MASK      2       // message contains real-time data such as speed or distance

// Public types ..................................................................................

typedef struct _instructset instructset;

typedef enum
{
    it_invalid = 0,

    it_audio,
    it_audio_lookahead,
    it_audio_traffic,
    it_audio_speed_limit,

    it_image,

    it_txt,
    it_txt_arrival,
    it_txt_lookahead,
    it_txt_stack,
    it_txt_sec,
    it_txt_arrival_sec,
    it_txt_stack_sec,

    it_ttf

} instructtype;

typedef enum
{
    asr_audio_first = 0,
    asr_audio_none = 0,
    asr_audio_full,
    asr_audio_short,
    asr_audio_tone,
    asr_audio_max

} asr_audio_mode;

typedef enum
{
    expect_traffic_none = 0,
    expect_traffic_light,
    expect_traffic_moderate,
    expect_traffic_severe

} nav_expect_traffic;


typedef enum
{
    none = 0,               // None
    dist,                   // double - distance remaining in this maneuver
    mdist,                  // double - maneuver distance
    bdist,                  // double - base distance remaining in this maneuver (non-collapsed)
    tripdist,               // double - trip distance
    triptime,               // uint32 - trip time
    cardinal,               // string - cardinal direction for this maneuver
    start,                  // NB_Place - origin for this trip
    destination,            // NB_Place - destination for this trip
    opronun,                // binary
    dpronun,                // binary
    dpronun_base,           // binary - destination pronun (no maneuver collapsing)
    ipronun,                // binary - intersection pronun
    oname,                  // string - origin street name
    dname,                  // string - destination street name
    dname_base,             // string - destination street name (no maneuver collapsing)
    aname,                  // string - alternate name
    aname_base,             // string - alternate name (no maneuver collapsing)
    iname,                  // string - intersection street name
//  navpos,                 // NIMNavManeuverPos -
    tcode,                  // string - turn code
    scode,                  // string - stack code
    unnamed,                // uint32 - ==0 if FALSE, !=0 if TRUE
    toward,                 // uint32 - ==0 if FALSE, !=0 if TRUE
    maxdist,                // double - max distance for toward announce
    tripsettings,           // NIMRouteSettings - settings for this route
    step,                   // uint32 - current
    totalsteps,             // uint32 - totalsteps
    navigating,             // uint32 - navigating
    aheaddist,              // double - distance up the this maneuver
    tripremaindist,         // double - remaining trip distance
    tripremaintime,         // uint32 - remaining trip time
    stackdist,              // double - distance to the stacked turn
    trafficincidentdist,    // double - distance to the next traffic incident
    trafficcongestiondist,  // double - distance to the next traffic congestion
    new_incident_count,     // uint32 - number of new incidents reported in the last update
    cleared_incident_count, // uint32 - number of cleared incidents reported in the last update
    travel_time_delta,      // int32  - change in travel time in seconds
    travel_time,            // uint32 - travel time for a specific maneuver
    traffic_delay,          // uint32 - traffic delay for a specific maneuver
    traffic_enabled,        // uint32 - traffic enabled == 0 if FALSE, != 0 if TRUE
    ti_road_pronun,         // binary - primary road pronun for the traffic incident
    ti_origin_proximity,    // string - proximity connector for the origin pronun
    ti_origin_pronun,       // binary - origin road pronun for the traffic incident
    tc_road_pronun,         // binary - primary road pronun for the traffic congestion
    tc_origin_proximity,    // string - proximity connector for the origin pronun
    tc_origin_pronun,       // binary - origin road pronun for the traffic congestion
    trafficcongestionlen,   // double - length of the current traffic congestion (in meters)
    expect_traffic_type,    // uint32 - expect_traffic_none, expect_traffic_light, expect_traffic_moderate, expect_traffic_severe
    announce_delay,         // uint32 - ==0 if FALSE, !=0 if TRUE
    in_congestion,          // uint32 - ==0 if FALSE, !=0 if TRUE
    ferryonroute,           // uint32 - 0 if no ferry maneuvers on route
    speedcameraremaindist,  // double - remaining trip distance
    showspeedcamera,        // uint32 - 0 if show speed camera in trip summary screen
    speed,                  // double - current speed in m/s
    ccode,                  // string - country code in maneuver
    dside,                  // driving-side for country in maneuver
    pedestrian,             // uint32 - ==0 if FALSE, !=0 if TRUE
    significant,            // uint32 - 1 if significant, 0 if not
    laneguidance,           // uint32 - 1 if maneuver has lane guidance info, 0 if not
    naturalguidance,        // uint32 - 1 if maneuver has natural guidance point, 0 if not
    gpspoor,                // uint32 - ==0 if FALSE, !=0 if TRUE
    hwyexit,                // uint32 - 1 if maneuver has highway exit number, 0 if not
    lgpronun,               // binary - lane guidance pronun
    lgpreppronun,           // binary - lane guidance preposition pronun
    pgppronun,              // binary - prepare guidance pronun
    pgppreppronun,          // binary - prepare guidance preposition pronun
    tgppronun,              // binary - turn guidance pronun
    tgppreppronun,          // binary - turn guidance preposition pronun
    hwyexitpronun,          // binary - highway exit pronun
    dpronunavailable,       // uint32 - 1 if downloadable pronun is available, 0 if not
    prepareplayed,          // uint32 - 1 if prepare has been played
    continueplayed,         // uint32 - 1 if continue has been played
    guidancepointtype,      // uint32 - guidance point type for a specific maneuver
    buttonpressed,          // uint32 - 1 if guidance button was pressed
    removestack,            // uint32 - 1 if stack should be removed
    stackmaneuverindex,     // uint32 - stack maneuver index
    noguidance,             // uint32 - 1 if next maneuver is non-guidance, 0 if not.
    ncode,                  // string - next code
    nextmaneuverindex,      // uint32 - next maneuver index
    ttsenabled,             // unit32 - 1 if TTS enabled
    fmaneuverheading,       // string - first maneuver heading
    fmaneuvercrossstreet,   // string - first maneuver cross street name
    highway,                // uint32 - 1 if highway
    routestartheading,      // string - heading from user current position to the start of route
    directiontoroute,       // string - direction from user current position to the start of route
    ifdirectiontoroute,     // uint32 - 1 if direction to route is valid
    stackdpronunavailable,  // uint32 - 1 if downloadable pronun is available for stack, 0 if not
    fmrpronun,              // binary - first major road pronun
    longmaneuver,           // uint32 - 1 if maneuver length is longer then long maneuver limit, 0 if not
    stackadvise             // uint32 - 1 if maneuver stack advise is true, 0 if not
} ds_elem;


/*! Array of instruction codes.

    This structure holds an array of instruction codes which are passed back to
    the API user in the audio callback. We use a static array for simplicity. Could
    be changed to dynamic array if needed.

    @see instructset_getdata
*/
typedef struct
{
    char    codes[MAX_INSTRUCTION_CODES][MAX_INSTRUCTION_CODE_LENGTH];  /*! Array of instruction codes. */
    int     count;                                                      /*!< Number of entries in the array. */

    /*! Array of pointers to the above array

        In order to hide the internal allocated array we need to make a pointer array
        which points to all the chars in the array above. This way if we change the
        size later then the API user doesn't need to know. Call instructset_get_instruction_array()
        to retrieve an initialized array.

        @see instructset_get_instruction_array
    */
    char*   pointerArray[MAX_INSTRUCTION_CODES];

} InstructionCodes;


// Public functions ..............................................................................

typedef boolean  (*instruct_data_source) (ds_elem elem, uint32 nman, void* pUser, const char** pdata, nb_size* psize, boolean* pfreedata);

instructset*    instructset_alloc(NB_Context* context, const char* name, formatfunc formatfunc, void* formatuser);
void            instructset_dealloc(instructset* is);
NB_Error        instructset_load(instructset* is, char* datalib, nb_size datalibSize, char* configBuffer, nb_size configBufferSize);
void            instructset_setdatasource(instructset* is, instruct_data_source ds, void* user);
const char*     instructset_getname(instructset* is);

/* See source file for description. */
NB_Error instructset_getdata(instructset* is,
                             uint32 nman,
                             InstructionCodes* codes,
                             byte** pdata,
                             size_t* psize,
                             char** pdatatype,
                             instructtype type,
                             NB_NavigateAnnouncementType amode,
                             boolean announce,
                             NB_NavigateAnnouncementUnits units);

NB_Error
nb_instructset_getdata(instructset* is,
                       uint32 nman,
                       InstructionCodes* codes,    /*!< Returned array of instruction codes. */
                       byte** pdata,               /*!< Only used for text. Call nsl_free() to free. */
                       size_t* psize,              /*!< Only used for text */
                       char** pdatatype,           /*!< Only used for text */
                       instructtype type,
                       NB_NavigateAnnouncementType amode,
                       boolean announce,
                       NB_NavigateAnnouncementUnits units,
                       NB_NavigateManeuverPos pos,
                       uint32* soundTypes);

NB_Error        instructset_getversion(instructset* is, uint32* pversion);
const char*     instructset_getreservedphrase(instructset* is, uint32 trans);

/* See source file for description. */
char** instructset_get_instruction_array(InstructionCodes* codes);


#endif
