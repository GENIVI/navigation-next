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
 * instructset.c: created 2005/01/12 by Mark Goddard.
 */


#include "instructset.h"
#include "palmath.h"
#include "tpselt.h"
#include "tpslib.h"
#include "tpsio.h"
#include "format.h"
#include "nbplace.h"
#include "navutil.h"
#include "nbutility.h"
#include "nbformat.h"
#include "csltypes.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbqalog.h"


// Private Constants .............................................................................
// Sorry for debugf. PAL_TestLog is only on bb10 branch
//#define ENABLE_LOGGING

#define CONFIG_NAME "config.tps"

#define VID_POINT            "point"
#define VID_MILE            "mile"
#define VID_MILES            "miles"
#define VID_FEET            "feet"
#define VID_FOOT            "foot"
#define VID_YARD            "yard"
#define VID_YARDS            "yards"
#define VID_KILOMETER        "kilometer"
#define VID_KILOMETERS        "kilometers"
#define VID_METER             "meter"
#define VID_METERS            "meters"
#define VID_SECOND            "second"
#define VID_SECONDS            "seconds"
#define VID_MINUTE            "minute"
#define VID_MINUTES            "minutes"
#define VID_HOUR            "hour"
#define VID_HOURS            "hours"
#define VID_ZERO            "zero"
#define VID_ONE                "one"
#define VID_ONE_STEP        "one-step"
#define VID_TWO                "two"
#define VID_THREE            "three"
#define VID_FOUR            "four"
#define VID_FIVE            "five"
#define VID_SIX                "six"
#define VID_SEVEN            "seven"
#define VID_EIGHT            "eight"
#define VID_NINE            "nine"
#define VID_TEN                "ten"
#define VID_ELEVEN            "eleven"
#define VID_TWELVE            "twelve"
#define VID_THIRTEEN        "thirteen"
#define VID_FORTEEN            "fourteen"
#define VID_FIFTEEN            "fifteen"
#define VID_SIXTEEN            "sixteen"
#define VID_SEVENTEEN        "seventeen"
#define VID_EIGHTEEN        "eighteen"
#define VID_NINETEEN        "nineteen"
#define VID_TWENTY            "twenty"
#define VID_THIRTY            "thirty"
#define VID_FORTY            "forty"
#define VID_FIFTY            "fifty"
#define VID_SIXTY            "sixty"
#define VID_SEVENTY            "seventy"
#define VID_EIGHTY            "eighty"
#define VID_NINETY            "ninety"
#define VID_HUNDRED            "hundred"
#define VID_FIVE_HUNDRED    "five-hundred"
#define VID_THOUSAND        "thousand"
#define VID_A_HALF          "a-half"
#define VID_HALF_A          "half-a"
#define VID_AND             "and"
#define VID_A_QUARTER       "a-quarter"


#define CONDITION_NONE  0
#define CONDITION_NOT   1
#define CONDITION_AND   2
#define CONDITION_OR    3

static const char* DESTINATION_NAME = "your-dest\0";

#define SPAN_ROAD_NAME                          "span-road-name"
#define SPAN_CLOSE                              "span-close"


// Private Types .................................................................................

struct _instructset
{
    NB_Context*             context;

    char                    name[256];
    char                    id[64];
    struct tpslib*            tl;
    tpselt                    config;

    formatfunc                ff;
    void*                    ffuser;

    instruct_data_source    dataSource;
    void*                    dsuser;

    // global var for spoken international words
    const char*                pgctchGender;

    // temporary variables to avoid allocating memory
    NB_Place                temp_place;

    uint32                  amode;
    nb_boolean              handleStack;
    uint32                  soundTypes;
    nb_boolean              handleNext;
};

typedef struct
{
    struct dynbuf dbp;

} text_instruct_state;

typedef struct NavAudioBuilderConfiguration
{
    nb_boolean isUnitruleUsed;
    char unit[MAX_INSTRUCTION_CODE_LENGTH];

    nb_boolean isUnitOverride;
    char overrideUnit[MAX_INSTRUCTION_CODE_LENGTH];

    char singularUnit[MAX_INSTRUCTION_CODE_LENGTH];
    char pluralUnit[MAX_INSTRUCTION_CODE_LENGTH];
} NavAudioBuilderConfiguration;

typedef NB_Error (append_func)(instructset* is, tpselt te, instructtype type,
                            void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);

typedef tpselt   (turn_sound_func)(instructset* is, uint32 nman, const char* turn_code, const char* pos, const char* vehicle_type);


// Private Functions .............................................................................

static const char* nav_pos_to_string(NB_NavigateManeuverPos pos);
#ifdef ENABLE_LOGGING
static const char* units_to_string(NB_NavigateAnnouncementUnits pos);
static const char* announcement_type_to_string(NB_NavigateManeuverPos pos);
static const char* instructtype_to_string(instructtype type);
#endif
static tpselt find_section(instructset* is, const char* name);
static boolean get_uint32(instructset* is, uint32 nman, ds_elem ds_elem, uint32* pval);
static boolean get_int32(instructset* is, uint32 nman, ds_elem ds_elem, int32* pval);
static boolean get_double(instructset* is, uint32 nman, ds_elem ds_elem, double* pval);
static char get_char(instructset* is, uint32 nman, ds_elem ds_elem);
static uint32 get_next_named(instructset* is, uint32 nman);
static boolean getdistance(instructset* is, uint32 nman, double* pdistance);
static boolean get_next_named_dist(instructset* is, uint32 nman, double* pdistance);
static uint32 get_next_significant(instructset* is, uint32 nman);
static uint32 get_previous_insignificant_count(instructset* is, uint32 nman);
static boolean get_next_significant_dist(instructset* is, uint32 nman, double* pdistance);

static NB_Error append_if_data(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos, ds_elem ds_elem, nb_size minlen);
static NB_Error append_if_not_data(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos, ds_elem ds_elem, nb_size minlen);

static boolean getplace(instructset* is, uint32 nman, ds_elem ds_elem, NB_Place* pplace);
static boolean get_tripsettings(instructset* is, NB_RouteSettings* ptripsettings);
static ds_elem ds_elem_from_string(const char* val);
static boolean equal(instructset* is, tpselt te, uint32 nman);
static tpselt getturnsound(instructset* is, uint32 nman, const char* turn_code, const char* pos, const char* vehicle_type);
static tpselt gettransitionsound(instructset* is, uint32 nman, const char* turn_code, const char* pos, const char* vehicle_type);
static const char* getnumsound(instructset* is, const char* num, const char* context_case);
static void getwordattribute(instructset* is, const char* word);

static append_func* get_instruct_append_handler(instructset* is, instructtype type, const char* name);
static NB_Error    append_children(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);

static const char* get_onesfn(int n);
static const char* get_tensfn(int n);

static NavAudioBuilderConfiguration* getUnitTypeOfFloat(boolean metric, boolean nonMetricYards, boolean smallUnits, double num);
static NavAudioBuilderConfiguration* createBuildConfig(const char* unit, const char* singularUnit, const char* pluralUnit);
static void AddBaseNameToInstructionArray               (instructset* is, InstructionCodes* codes, const char* name, nb_size namelen);
static void AddNameToInstructionArray               (instructset* is, InstructionCodes* codes, const char* name, nb_size namelen);
static NB_Error AddCacheIDtoInstructionArray        (instructset* is, InstructionCodes* codes, ds_elem ds_elem, uint32 nman);
static NB_Error audio_append_integer                (instructset* is, InstructionCodes* codes, int num, NavAudioBuilderConfiguration* builderConfig);
static NB_Error audio_append_integer_ex             (instructset* is, InstructionCodes* codes, double num, tpselt te_numsounds, NavAudioBuilderConfiguration* builderConfig);
static NB_Error audio_append_float                  (instructset* is, InstructionCodes* codes, double num, boolean round, boolean sayfrac, NavAudioBuilderConfiguration* builderConfig, boolean* plurar);
static NB_Error audio_append_distance               (instructset* is, InstructionCodes* codes, double dist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units);
#if 0
static NB_Error audio_append_distance_with_half_increment(instructset* is, InstructionCodes* codes, double dist, boolean showunits, double dec_limit, NB_NavigateAnnouncementUnits units);
#endif
static NB_Error audio_append_distance_with_adjust   (instructset* is, InstructionCodes* codes, double dist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units, double current_speed);
static NB_Error audio_append_time                   (instructset* is, InstructionCodes* codes, uint32 val, boolean sayunits, boolean round);

static NB_Error append_turn_sound_impl(instructset* is, tpselt te, instructtype type,
                          void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units,
                          NB_NavigateManeuverPos pos, ds_elem ds_elem, turn_sound_func* getsoundfunc);

static void text_dynbuf_append_font                 (instructset* is, struct dynbuf* dbp, NB_Font fnt);
static void text_dynbuf_append_color                (instructset* is, struct dynbuf* dbp, nb_color rgb);
static void text_dynbuf_append_text                 (instructset* is, struct dynbuf* dbp, const char* txt);
static void text_dynbuf_append_newline              (instructset* is, struct dynbuf* dbp);
static NB_Error text_dynbuf_append_data             (instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman);
static NB_Error text_dynbuf_append_name             (instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman);
static NB_Error text_dynbuf_append_address          (instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman, boolean multiline);
static NB_Error text_dynbuf_append_distance         (instructset* is, struct dynbuf* dbp, uint32 nman, NB_NavigateAnnouncementUnits units);
static NB_Error text_dynbuf_append_phonenum         (instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman);
static NB_Error text_dynbuf_append_tsettings        (instructset* is, struct dynbuf* dbp);
static NB_Error GetImageInstructionCode             (instructset* is, tpselt instruct_elt, uint32 nman, InstructionCodes* codes);

// General append handlers
static NB_Error append_if                           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_start_name                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_start_phonenum            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_destination_name          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_destination_phonenum      (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_oname                     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_dname                     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_dname_base                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_aname                     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_aname_base                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_next                         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_equal                     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_equal                 (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_stack                     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_stack                 (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_turn_sound                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_stack_sound                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_transition_sound             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_stack_transition_sound       (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_toward_name               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_toward_name           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_unnamed                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_unnamed               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_next_named                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_next_named            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_next_named                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_next_named_dist_lt_max    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_lookahead                 (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_lookahead             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_navigating                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_navigating            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_ferry_on_route            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_new_incidents             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_cleared_incidents         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_travel_time_delta         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_travel_time_delta_pos     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_travel_time_delta_neg     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_traffic                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_traffic_delay             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_traffic_delay_threshold   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_plural                    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_plural                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_ti_road_pronun            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_ti_origin_pronun          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_tc_road_pronun            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_tc_origin_pronun          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_in_congestion             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_in_congestion         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_show_speed_camera         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_next_significant             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_pedestrian                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_pedestrian            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_iname                     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_lane_guidance             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_lane_guidance         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_natural_guidance          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_natural_guidance      (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_gps_poor                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_gps_poor              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_hwy_exit                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_hwy_exit              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_dpronun_available         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_dpronun_available     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_prepare_played            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_prepare_played        (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_guidance_point_type       (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_condition                 (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_stacked_sound             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_stacked_sound         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_continue_played           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_continue_played       (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_no_guidance               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_fmaneuver_heading         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_fmaneuver_heading     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_fmaneuver_crossstreet     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_fmaneuver_crossstreet (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_opronun_available         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_not_opronun_available     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_direction_to_route        (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_no_direction_to_route     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_last_maneuver_beforeDT     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error append_if_fmrpronun_available     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);

// audio append handlers
static NB_Error audio_append_sound                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_opronun                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_dpronun                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_dpronun_base           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_ipronun                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_dist                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_tdist                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_next_named_dist        (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_step_number            (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_total_steps            (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_ahead_dist             (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_trdist                 (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_stackdist              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_trafficcongestiondist  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_trafficincidentdist    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_stackdist              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_new_incident_count     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_cleared_incident_count (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_travel_time_delta      (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_traffic_delay          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_ti_road_pronun         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_ti_origin_proximity    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_ti_origin_pronun       (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_tc_road_pronun         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_tc_origin_proximity    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_tc_origin_pronun       (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_expect_traffic         (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_trafficongestionlen    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_cameradist             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_next_significant_dist  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_lgpronun               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_lgpreppronun           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_pgppronun              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_pgppreppronun          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_tgppronun              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_tgppreppronun          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_hwyexitpronun          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_rdistpronun            (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_fmaneuver_heading      (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_fmaneuver_crossstreet  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_fmaneuver_opronun      (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_routestart_heading     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_direction_to_route     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error audio_append_fmrpronun              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);


// Text append handlers
static NB_Error text_append_dist                    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_font                    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_color                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_text                    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_cardinal                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_start_name              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_start_address           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_start_phonenum          (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_destination_name        (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_destination_address     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_destination_phonenum    (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_oname                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_dname                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_dname_base              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_aname                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_aname_base              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_tdist                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_ttime                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_tsettings               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_newline                 (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_step_number             (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_total_steps             (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_ahead_dist              (instructset* id, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_trdist                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_ferriesremaining        (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_iname                   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_trtime                  (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_stackdist               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_travel_time             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_traffic_delay           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_cameradist              (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_next_significant_dist   (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_lgpronun                (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_lgprepronun             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_pgppronun               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_tgppronun               (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_hwyexitpronun           (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);
static NB_Error text_append_rdistpronun             (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos);


static boolean is_txt_instruction(instructtype type);
static boolean is_audio_instruction(instructtype type);
static boolean is_lookahead_instruction(instructtype type);

void CopyInstructionName(char *dst, const char *src, nb_size len);

// boolean expressions realted functions
nb_boolean evaluate_expression(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos, uint8 condition_type);
nb_boolean evaluate_and(CSL_Vector* vec);
nb_boolean evaluate_or(CSL_Vector* vec);
nb_boolean evaluate_not(CSL_Vector* vec);
nb_boolean get_value(instructset* is, uint32 nman, const char* name, nb_boolean is_lookahead, NB_NavigateManeuverPos pos);

// Public Functions ..............................................................................

instructset*
instructset_alloc(NB_Context* context, const char* name, formatfunc ff, void* formatuser)
{
    instructset* is = nsl_malloc(sizeof(instructset));

    if (is == NULL)
    {
        return NULL;
    }

    nsl_memset(is, 0, sizeof(*is));

    is->context = context;

    nsl_strlcpy(is->name, name, sizeof(is->name));

    is->ff = ff;
    is->ffuser = formatuser;

    is->tl = NULL;
    is->config = NULL;

    return is;
}

void
instructset_dealloc(instructset* is)
{
    if (is != NULL)
    {
        if (is->tl != NULL)
        {
            tpslib_dealloc(is->tl);
        }

        if (is->config != NULL)
        {
            te_dealloc(is->config);
        }

        nsl_free(is);
    }
}

NB_Error
instructset_load(instructset* is, char* datalib, nb_size datalibSize, char* configBuffer, nb_size configBufferSize)
{
    struct tps_unpackstate *tpsups = NULL;

    is->tl = tpslib_preload(datalib, datalibSize);

    if (is->tl == NULL)
    {
        return NE_INVAL;
    }

    tpsups = tps_unpack_start(is->tl, configBufferSize);

    if (tpsups == NULL)
    {
        return NE_INVAL;
    }

    (void) tps_unpack_feed(tpsups, configBuffer, configBufferSize); // tps_unpack_result will report failure

    if (tps_unpack_result(tpsups, &is->config) != TIO_READY)
    {
        return NE_BADDATA;
    }

    return NE_OK;
}

void
instructset_setdatasource(instructset* is, instruct_data_source dataSource, void* user)
{
    is->dataSource = dataSource;
    is->dsuser = user;
}

const char*
instructset_getname(instructset* is)
{
    if (is)
    {
        return is->name;
    }
    return NULL;
}

/*! Retrieve instruction codes.

This function returns instruction codes for audio and image requests. It returns
a byte buffer for text requests. If the byte buffer was returned the data has to
be freed using nsl_free().

@return NB_Error
*/
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
                    uint32* soundTypes)
{
    NB_Error                        err             = NE_OK;
    tpselt                            section_elt     = NULL;
    tpselt                            instruct_elt    = NULL;
    tpselt                            te              = NULL;
    int                                iter            = 0;
    int                             matchqual       = 0;
    text_instruct_state             tis             = {{0}};
    const char*                        turn_code       = NULL;
    nb_size                            turn_size       = 0;
    boolean                         turn_free       = FALSE;
    const char*                        stack_code      = NULL;
    nb_size                            stack_size      = 0;
    boolean                         stack_free      = FALSE;
    const char*                        next_code       = NULL;
    nb_size                            next_size       = 0;
    boolean                         next_free       = FALSE;
    const char*                        data            = NULL;
    boolean                            freedata        = FALSE;
    int                                turnmatch       = 0;
    int                             stackmatch      = 0;
    const char*                     pos_pat         = NULL;
    const char*                     pos_val         = NULL;
    const char*                     data_mime_type  = NULL;

#ifdef ENABLE_LOGGING
    debugf("nb_instructset_getdata nman=%d instructtype=%s NB_NavigateAnnouncementType=%s", nman, instructtype_to_string(type), announcement_type_to_string(amode));
    debugf("nb_instructset_getdata NB_NavigateAnnouncementUnits=%s NB_NavigateManeuverPos=%s", units_to_string(units), nav_pos_to_string(pos) );
#endif

    if (is == NULL)
    {
        return NE_INVAL;
    }

    // This is not used (anymore) for audio and image requests. Only text requests currently use it.
    if (pdata && psize)
    {
        *pdata = NULL;
        *psize = 0;
    }

    if (is->dataSource == NULL)
    {
        return NE_INVAL;
    }

    if (data != NULL && freedata)
    {
        nsl_free((void*)data);
    }

    data = NULL;

    // Get turn instruction code
    if (!is->dataSource(tcode, nman, is->dsuser, &turn_code, &turn_size, &turn_free))
    {
        turn_size = 0;
        turn_free = FALSE;
    }
#ifdef ENABLE_LOGGING
    else
    {
        debugf("nb_instructset_getdata turn_code=%s", turn_code);
    }
#endif
    // Get stack instruction code
    if (!is->dataSource(scode, nman, is->dsuser, &stack_code, &stack_size, &stack_free))
    {
        stack_size = 0;
        stack_free = FALSE;
    }
#ifdef ENABLE_LOGGING
    else
    {
        debugf("nb_instructset_getdata stack_code=%s", stack_code);
    }
#endif

    if (nman == NAV_MANEUVER_CURRENT || nman == NAV_MANEUVER_NONE || nman == NAV_MANEUVER_CAMERA ||
        nman == NAV_MANEUVER_CAMERA_SPEEDING)
    {
        next_code = NULL;
        next_size = 0;
        next_free = FALSE;
    }
    else if (!is->dataSource(ncode, nman, is->dsuser, &next_code, &next_size, &next_free))
    {
        next_size = 0;
        next_free = FALSE;
    }
#ifdef ENABLE_LOGGING
    else
    {
        debugf("nb_instructset_getdata next_code=%s", next_code);
    }
#endif

    is->amode = (uint32) amode;
    is->handleStack = FALSE;
    is->soundTypes = 0;
    is->handleNext = FALSE;

    switch (type)
    {
    case it_audio:
    case it_audio_lookahead:
        switch (amode)
        {
        case NB_NAT_Tone:
                section_elt = find_section(is, "tone");
            break;
        case NB_NAT_Voice:
            if (announce)
            {
                section_elt = find_section(is, "short-annc");
                if (section_elt == NULL)
                {
                    section_elt = find_section(is, "short");
                }
            }
            else
            {
                section_elt = find_section(is, "short");
            }
            break;
        case NB_NAT_Street:
            if (announce)
            {
                section_elt = find_section(is, "long-annc");
                if (section_elt == NULL)
                {
                    section_elt = find_section(is, "long");
                }

                //if street names are not available, fall back to short prompts
                if (section_elt == NULL)
                {
                    section_elt = find_section(is, "short-annc");
                    if (section_elt == NULL)
                    {
                        section_elt = find_section(is, "short");
                    }
                }
            }
            else
            {
                section_elt = find_section(is, "long");
            }

            break;
        default:
            break;
        }
        break;
    case it_audio_traffic:
        switch (amode)
        {
        case NB_NAT_Tone:
            section_elt = find_section(is, "tone");
            break;
        default:
            section_elt = find_section(is, "traffic");
            break;
        }
        break;
    case it_audio_speed_limit:
        switch (amode)
        {
        case NB_NAT_Tone:
            section_elt = find_section(is, "tone");
            break;
        case NB_NAT_Voice:
            section_elt = find_section(is, "speed-limit");
            break;
        default:
            break;
        }
        break;
    case it_image:
        section_elt = find_section(is, "image");
        break;
    case it_ttf:
        section_elt = find_section(is, "ttf");
        break;

    case it_txt:
    case it_txt_lookahead:
        section_elt = find_section(is, "static-text");
        break;
    case it_txt_arrival:
        section_elt = find_section(is, "arrival-text");
        break;
    case it_txt_stack:
        section_elt = find_section(is, "stack-text");
        break;
    case it_txt_sec:
        section_elt = find_section(is, "sec-text");
        break;
    case it_txt_arrival_sec:
        section_elt = find_section(is, "sec-arrival-text");
        break;
    case it_txt_stack_sec:
        section_elt = find_section(is, "sec-stack-text");
        break;
    default:
        break;
    }

    iter = 0;
    matchqual = 0;
    instruct_elt = NULL;

#ifdef ENABLE_LOGGING
    if (section_elt)
    {
        debugf("section_elt name=%s", te_getattrc(section_elt,"name")); // the section in voices.sexp file, for example   (section (name tone)
    }
    else
    {
        debugf("section_elt is not found");
    }
    debugf("find proper instruction in section for turn - %s  stack_code - %s  pos - %s", turn_code, stack_code, nav_pos_to_string(pos));
#endif
    while (section_elt != NULL && (te = te_nextchild(section_elt, &iter)) != NULL)
    {
        if (str_cmpx("instruction", te_getname(te)) == 0)
        {
            pos_pat = te_getattrc(te, "pos");
            pos_val = nav_pos_to_string(pos);
#ifdef ENABLE_LOGGING
            // comment this out if do not want verbose logs
            debugf("current 'instruction' has turn - %s  stack_code - %s  pos - %s", te_getattrc(te, "turn"), te_getattrc(te, "stacknext"), pos_pat);
#endif
            // if a "pos" is not specified, match all
            if (pos_pat != NULL)
            {
                if (pos_val != NULL)
                {
                    if (!str_match_simple_pattern(pos_pat, pos_val))
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }

            turnmatch    =    str_match_simple_pattern(te_getattrc(te, "turn"), turn_code);
            stackmatch    =    str_match_simple_pattern(te_getattrc(te, "stacknext"), stack_code);

            if (pos == NB_NMP_SchoolZone
                || pos == NB_NMP_SpeedLimitZone
                || pos == NB_NMP_ConfirmDetour
                || (turnmatch && stackmatch && (turnmatch + stackmatch) > matchqual))
            {
#ifdef ENABLE_LOGGING
                debugf("'instruction' is FOUND: turn - %s  stack_code - %s  pos - %s", te_getattrc(te, "turn"), te_getattrc(te, "stacknext"), pos_pat);
#endif
                instruct_elt = te;
                matchqual = turnmatch + stackmatch;
            }
        }
    }


#ifdef ENABLE_LOGGING
    if (!instruct_elt)
    {
        debugf("instruct_elt is not found");
    }
#endif
    if (is->config)
    {
        data_mime_type = te_getattrc(is->config, "media-type");
    }

    if (instruct_elt != NULL)
    {
        switch (type)
        {
        case it_audio:
        case it_audio_lookahead:
        case it_audio_traffic:
        case it_audio_speed_limit:

            // This function fills the instruction code array with all the audio codes to play.
            err = err ? err : append_children(is, instruct_elt, type, codes, nman, units, pos);
            //@todo (BUG 56050) bypass DT.U for voice
            if (turn_code)
            {
                if ( nsl_strcmp(turn_code, "DT.U") == 0 && err == NE_NOENT)
                {
                    err = NE_OK;
                }
            }
            break;

        case it_image:
        case it_ttf:

            // Get the image instruction code
            err = err ? err : GetImageInstructionCode(is, instruct_elt, nman, codes);
            break;
        case it_txt:
        case it_txt_arrival:
        case it_txt_lookahead:
        case it_txt_stack:
        case it_txt_sec:
        case it_txt_arrival_sec:
        case it_txt_stack_sec:
            {
                if (str_cmpx(data_mime_type, "text/x-nim-formatted") != 0)
                {
                    err = NE_BADDATA;
                }
                else
                {
                    err = dbufnew(&tis.dbp, 200);

                    if (err == NE_OK)
                    {
                        err = append_children(is, instruct_elt, type, &tis, nman, units, pos);

                        if (err == NE_OK && dbufget(&tis.dbp) != NULL && dbuflen(&tis.dbp) > 0)
                        {
                            *pdata = nsl_malloc(dbuflen(&tis.dbp));

                            if ((*pdata) == NULL)
                            {
                                err = NE_NOMEM;
                                *pdata = NULL;
                                *psize = 0;
                            }
                            else
                            {
                                nsl_memcpy(*pdata, dbufget(&tis.dbp), dbuflen(&tis.dbp));
                                *psize = dbuflen(&tis.dbp);
                            }
                        }

                        dbufdel(&tis.dbp);
                    }
                }
                break;
            }
        default:
            break;
        }
    }
    else
    {
#ifdef DEBUG_FAILED_INSTRUCTIONS
        DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_INFO, ("failed to find instruction in %s, pos: %s, tc: %s, sc: %s\n", is->name, nav_pos_to_string(pos), turn_code ? turn_code : "<null>", stack_code ? stack_code : "<null>"));
#endif
        err = NE_NOENT;
    }

    if (turn_free && turn_code != NULL)
    {
        nsl_free((void*)turn_code);
    }

    if (stack_free && stack_code != NULL)
    {
        nsl_free((void*)stack_code);
    }

    if (next_free && next_code != NULL)
    {
        nsl_free((void*)next_code);
    }

    if (pdatatype != NULL)
    {
        if (data_mime_type != NULL && nsl_strlen(data_mime_type) > 0)
        {
            *pdatatype = nsl_malloc(nsl_strlen(data_mime_type)+1);

            if (*pdatatype != NULL)
            {
                nsl_memcpy(*pdatatype, data_mime_type, nsl_strlen(data_mime_type)+1);
            }
            else
            {
                err = NE_NOMEM;

                if ((*pdata) != NULL)
                {
                    nsl_free(*pdata);
                    *pdata = NULL;
                    *psize = 0;
                }
            }
        }
        else
        {
            *pdatatype = NULL;
        }
    }
    if (soundTypes)
    {
        *soundTypes = is->soundTypes;
        if (is->handleStack)
        {
            *soundTypes |= STACK_SOUND_MASK;
        }
    }

    return err;
}

NB_Error
instructset_getversion(instructset* is, uint32* pversion)
{
    if (is->config == NULL)
        return NE_NOENT;

    *pversion = te_getattru(is->config, "command-set-version");
    return NE_OK;
}

/* Added to handle reserved phrases */
const char*
instructset_getreservedphrase(instructset* is, uint32 trans)
{
    const char*    reserved = NULL;
    boolean    exitloop = FALSE;
    int iter = 0;
    tpselt ce;
    tpselt te_reservedphrases = te_getchild(is->config, "reserved-phrases");

    if (te_reservedphrases == NULL)
        return NULL;

    while (!exitloop && ((ce = te_nextchild(te_reservedphrases, &iter)) != NULL))
    {
        if (str_cmpx(te_getname(ce), "reserved") == 0 &&
            (trans == te_getattru(ce, "unnamed-road-type")))
        {
            reserved = te_getattrc(ce, "unnamed-road-phrase");
            exitloop = TRUE;
        }
    }

    return reserved;
}

/*! Retrieve an initialized pointer to the given array.

    Call this function to retrieve an array of char pointers which can be passed to NBI_NavigateAudioCallback

    @return Pointer to char pointer array

    @see instructset_getdata
    @see NBI_NavigateAudioCallback
*/
char**
instructset_get_instruction_array(InstructionCodes* codes   /*!< Instruction codes from which to get the char pointer array. */
                                 )
{
    // Copy all the array pointers in the "char pointer array"
    int i = 0;
    for (i = 0; i < MAX_INSTRUCTION_CODES; i++)
    {
        // Copy pointer
        codes->pointerArray[i] = codes->codes[i];
    }

    // Return the pointer to the array
    return (char**)&(codes->pointerArray);
}


// Private Functions .............................................................................

const char*
nav_pos_to_string(NB_NavigateManeuverPos pos)
{
    const char* r = NULL;

    switch (pos)
    {
        case NB_NMP_Current:
            r = "current";
            break;
        case NB_NMP_Continue:
            r = "continue";
            break;
        case NB_NMP_ShowTurn:
            r = "show-turn";
            break;
        case NB_NMP_Prepare:
            r = "prepare";
            break;
        case NB_NMP_Turn:
            r = "turn";
            break;
        case NB_NMP_Past:
            r = "past-turn";
            break;
        case NB_NMP_Recalculate:
            r = "recalc";
            break;
        case NB_NMP_SoftRecalculate:
            r = "soft-recalc";
            break;
        case NB_NMP_RecalculateConfirm:
            r = "recalc-confirm";
            break;
        case NB_NMP_RecalculateTraffic:
            r = "recalc-traffic";
            break;
        case NB_NMP_TrafficCongestion:
            r = "traffic-congestion";
            break;
        case NB_NMP_TrafficIncident:
            r = "traffic-incident";
            break;
        case NB_NMP_TrafficAlert:
            r = "traffic-alert";
            break;
        case NB_NMP_TrafficDelay:
            r = "traffic-delay";
            break;
        case NB_NMP_Calc:
            r = "calc";
            break;
        case NB_NMP_RouteUpdated:
            r = "recalc-update";
            break;
        case NB_NMP_ConfirmDetour:
            r = "confirm-detour";
            break;
        case NB_NMP_SpeedLimitZone:
            r = "Speed-Limit-Zone";
            break;
        case NB_NMP_SchoolZone:
            r = "School-Zone";
            break;
        case NB_NMP_SwitchToTBT:
            r = "switch-to-tbt";
            break;
        case NB_NMP_SwitchToStaticMode:
            // needs to include in to instruction r = "switch-to-static-mode";
        	r = "soft-recalc";
        	break;
        case NB_NMP_Invalid:
        default:
            // Could get there at initialRoute state, return NULL.
            break;
    }

    return r;
}

#ifdef ENABLE_LOGGING
const char* units_to_string(NB_NavigateAnnouncementUnits units)
{
    const char* r = NULL;

    switch (units)
    {
        case NB_NAU_Kilometers:
            r = "NB_NAU_Kilometers";
            break;
        case NB_NAU_Miles:
            r = "NB_NAU_Miles";
            break;
        case NB_NAU_MilesYards:
            r = "NB_NAU_MilesYards";
            break;
        default:
            return "";
    }
    return r;
}

const char* announcement_type_to_string(NB_NavigateAnnouncementType type)
{
    const char* r = NULL;

    switch (type)
    {
        case NB_NAT_Tone:
            r = "NB_NAT_Tone";
            break;
        case NB_NAT_Voice:
            r = "NB_NAT_Voice";
            break;
        case NB_NAT_Street:
            r = "NB_NAT_Street";
            break;
        default:
            return "";
    }

    return r;
}

const char* instructtype_to_string(instructtype type)
{
    const char* r = NULL;

    switch (type)
    {
        case it_invalid:
            r = "it_invalid";
            break;
        case it_audio:
            r = "it_audio";
            break;
        case it_audio_lookahead:
            r = "it_audio_lookahead";
            break;
        case it_audio_traffic:
            r = "it_audio_traffic";
            break;
        case it_audio_speed_limit:
            r = "it_audio_speed_limit";
            break;
        case it_image:
            r = "it_image";
            break;
        case it_txt:
            r = "it_txt";
            break;
        case it_txt_arrival:
            r = "it_txt_arrival";
            break;
        case it_txt_lookahead:
            r = "it_txt_lookahead";
            break;
        case it_txt_stack:
            r = "it_txt_stack";
            break;
        case it_txt_sec:
            r = "it_txt_sec";
            break;
        case it_txt_arrival_sec:
            r = "it_txt_arrival_sec";
            break;
        case it_txt_stack_sec:
            r = "it_txt_stack_sec";
            break;
        case it_ttf:
            r = "it_ttf";
            break;
        default:
            return "";
    }

    return r;
}
#endif


tpselt
find_section(instructset* is, const char* name)
{
    tpselt    ce      = NULL;
    int        iter    = 0;

    if (is->config)
    {
        while ((ce = te_nextchild(is->config, &iter)) != NULL)
        {
            if (str_cmpx(te_getname(ce), "section") == 0 &&
                str_cmpx(te_getattrc(ce, "name"), name) == 0)
            {
                return ce;
            }
        }
     }

    return NULL;
}

boolean
get_uint32(instructset* is, uint32 nman, ds_elem ds_elem, uint32* pval)
{
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    boolean     retval      = FALSE;

    if (is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata) && datalen == sizeof(uint32))
    {
        nsl_memcpy(pval, data, sizeof(*pval));
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
    }

    return retval;
}

boolean
get_int32(instructset* is, uint32 nman, ds_elem ds_elem, int32* pval)
{
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    boolean     retval      = FALSE;

    if (is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata) && datalen == sizeof(int32))
    {
        nsl_memcpy(pval, data, sizeof(*pval));
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
    }

    return retval;
}

boolean
get_double(instructset* is, uint32 nman, ds_elem ds_elem, double* pval)
{
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    boolean     retval      = FALSE;

    if (is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata) && datalen == sizeof(double))
    {
        nsl_memcpy(pval, data, sizeof(*pval));
        retval = TRUE;
    }
    else
    {
        retval = FALSE;
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
    }

    return retval;
}

char
get_char(instructset* is, uint32 nman, ds_elem ds_elem)
{
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    char        retval      = '\0';

    if (is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata) && datalen >= sizeof(retval))
    {
        retval = *data;
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
    }

    return retval;
}

uint32
get_next_named(instructset* is, uint32 nman)
{
    uint32 is_unnamed = 0;

    if ((nman == NAV_MANEUVER_START) || (nman == NAV_MANEUVER_NONE) ||
        (nman == NAV_MANEUVER_CURRENT) || (nman == NAV_MANEUVER_INITIAL))
    {
        nman = 0;
    }

    while (TRUE)
    {
        if (!get_uint32(is, nman, unnamed, &is_unnamed))
        {
            return NAV_MANEUVER_NONE;
        }
        if (is_unnamed)
        {
            nman++;
        }
        else
        {
            return nman;
        }
    }
}

boolean
getdistance(instructset* is, uint32 nman, double* pdistance)
{
    if (nman == NAV_MANEUVER_CAMERA)
        // Get camera distance
        return get_double(is, nman, speedcameraremaindist, pdistance);
    else
        return get_double(is, nman, nman == NAV_MANEUVER_NONE ? tripdist : dist, pdistance);
}

boolean
get_next_named_dist(instructset* is, uint32 nman, double* pdistance)
{
    double d            = 0;
    double d2           = 0;
    uint32 nextnamed    = 0;
    uint32 n            = 0;

    if (!get_double(is, nman, dist, &d))
    {
        return FALSE;
    }

    nextnamed = get_next_named(is, nman);

    for (n = nman+1; n <= nextnamed && get_double(is, n, dist, &d2); n++)
    {
        d += d2;
    }

    *pdistance = d;

    return TRUE;
}

uint32
get_next_significant(instructset* is, uint32 nman)
{
    uint32 significant_needed = 2;  // look for the second significant maneuver, starting with the current one.
    uint32 is_significant = 0;

    while (TRUE)
    {
        if (!get_uint32(is, nman, significant, &is_significant))
        {
            return NAV_MANEUVER_NONE;
        }
        if (is_significant)
        {
            significant_needed--;

            if (significant_needed == 0)
            {
                return nman;
            }
        }

        nman++;
    }

    return NAV_MANEUVER_NONE;
}

uint32
get_previous_insignificant_count(instructset* is, uint32 nman)
{
    uint32 insignificant_count = 0;
    uint32 is_significant = 0;

    while (TRUE)
    {
        if (!get_uint32(is, nman, significant, &is_significant))
        {
            return NAV_MANEUVER_NONE;
        }

        if (!is_significant)
        {
            insignificant_count++;
        }

        if (0 == nman)
        {
            return insignificant_count;
        }

        nman--;
    }

    return NAV_MANEUVER_NONE;
}

boolean
get_next_significant_dist(instructset* is, uint32 nman, double* pdistance)
{
    double d            = 0;
    double d2           = 0;
    uint32 nextsignificant    = 0;
    uint32 n            = 0;

    if (!get_double(is, nman, dist, &d))
    {
        return FALSE;
    }

    nextsignificant = get_next_significant(is, nman);

    if (nextsignificant == NAV_MANEUVER_NONE)
    {
        return FALSE;
    }

    for (n = nman+1; n <= nextsignificant && get_double(is, n, dist, &d2); n++)
    {
        d += d2;
    }

    *pdistance = d;

    return TRUE;
}

NB_Error
append_if_data(instructset* is, tpselt te, instructtype type,
    void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos, ds_elem ds_elem, nb_size minlen)
{
    NB_Error    err     = NE_OK;
    nb_size        datalen = 0;

    if (is->dataSource(ds_elem, nman, is->dsuser, NULL, &datalen, NULL) && datalen >= minlen)
    {
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return err;
}

NB_Error
append_if_not_data(instructset* is, tpselt te, instructtype type,
    void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos, ds_elem ds_elem, nb_size minlen)
{
    NB_Error    err     = NE_OK;
    nb_size        datalen = 0;

    if (!is->dataSource(ds_elem, nman, is->dsuser, NULL, &datalen, NULL) || datalen < minlen)
    {
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return err;
}

boolean
getplace(instructset* is, uint32 nman, ds_elem ds_elem, NB_Place* pplace)
{
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    boolean     retval      = FALSE;

    if (is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata) && datalen == sizeof(NB_Place))
    {
        nsl_memcpy(pplace, data, sizeof(*pplace));
        retval = TRUE;
    }
    else
    {
        nsl_memset(pplace, 0, sizeof(*pplace));
        retval = FALSE;
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
    }

    return retval;
}

boolean
get_tripsettings(instructset* is, NB_RouteSettings* ptripsettings)
{
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    boolean     retval      = FALSE;

    if (is->dataSource(tripsettings, NAV_MANEUVER_START, is->dsuser, &data, &datalen, &freedata) && datalen == sizeof(NB_RouteSettings))
    {
        nsl_memcpy(ptripsettings, data, sizeof(*ptripsettings));
        retval = TRUE;
    }
    else
    {
        nsl_memset(ptripsettings, 0, sizeof(*ptripsettings));
        retval = FALSE;
    }

    if (freedata && data != NULL)
    {
        nsl_free((char*)data);
    }

    return retval;
}

ds_elem
ds_elem_from_string(const char* val)
{
    if (val == NULL)
        return none;
    else if (str_cmpx(val, "dist") == 0)
        return dist;
    else if (str_cmpx(val, "mdist") == 0)
        return mdist;
    else if (str_cmpx(val, "tripdist") == 0)
        return tripdist;
    else if (str_cmpx(val, "triptime") == 0)
        return triptime;
    else if (str_cmpx(val, "cardinal") == 0)
        return cardinal;
    else if (str_cmpx(val, "start") == 0)
        return start;
    else if (str_cmpx(val, "destination") == 0)
        return destination;
    else if (str_cmpx(val, "opronun") == 0)
        return opronun;
    else if (str_cmpx(val, "dpronun") == 0)
        return dpronun;
    else if (str_cmpx(val, "oname") == 0)
        return oname;
    else if (str_cmpx(val, "dname") == 0)
        return dname;
    else if (str_cmpx(val, "dname-base") == 0)
        return dname_base;
    else if (str_cmpx(val, "aname") == 0)
        return aname;
    else if (str_cmpx(val, "aname-base") == 0)
        return aname_base;
    //else if (str_cmpx(val, "navpos") == 0)
    //    return navpos;
    else if (str_cmpx(val, "tcode") == 0)
        return tcode;
    else if (str_cmpx(val, "scode") == 0)
        return scode;
    else if (str_cmpx(val, "unnamed") == 0)
        return unnamed;
    else if (str_cmpx(val, "tsettings") == 0)
        return tripsettings;
    else if (str_cmpx(val, "step") == 0)
        return step;
    else if (str_cmpx(val, "totalsteps") == 0)
        return totalsteps;
    else if (str_cmpx(val, "navigating") == 0)
        return navigating;
    else if (str_cmpx(val, "aheaddist") == 0)
        return aheaddist;
    else if (str_cmpx(val, "tripremaindist") == 0)
        return tripremaindist;
    else if (str_cmpx(val, "tripremaintime") == 0)
        return tripremaintime;
    else if (str_cmpx(val, "stack-dist") == 0)
        return stackdist;
    else if (str_cmpx(val, "trafficincidentdist") == 0)
        return trafficincidentdist;
    else if (str_cmpx(val, "trafficcongestiondist") == 0)
        return trafficcongestiondist;
    else if (str_cmpx(val, "new-incident-count") == 0)
        return new_incident_count;
    else if (str_cmpx(val, "cleared-incident-count") == 0)
        return cleared_incident_count;
    else if (str_cmpx(val, "travel-time-delta") == 0)
        return travel_time_delta;
    else if (str_cmpx(val, "travel-time") == 0)
        return travel_time;
    else if (str_cmpx(val, "traffic-delay") == 0)
        return traffic_delay;
    else if (str_cmpx(val, "traffic-enabled") == 0)
        return traffic_enabled;
    else if (str_cmpx(val, "ti-road-pronun") == 0)
        return ti_road_pronun;
    else if (str_cmpx(val, "ti-origin-proximity") == 0)
        return ti_origin_proximity;
    else if (str_cmpx(val, "ti-origin-pronun") == 0)
        return ti_origin_pronun;
    else if (str_cmpx(val, "tc-road-pronun") == 0)
        return tc_road_pronun;
    else if (str_cmpx(val, "tc-origin-proximity") == 0)
        return tc_origin_proximity;
    else if (str_cmpx(val, "tc-origin-pronun") == 0)
        return tc_origin_pronun;
    else if (str_cmpx(val, "speedcameraremaindist") == 0)
        return speedcameraremaindist;
    else if (str_cmpx(val, "pedestrian") == 0)
        return pedestrian;
    else
        return none;
}

boolean
equal(instructset* is, tpselt te, uint32 nman)
{
    boolean retval = FALSE;

    ds_elem ds_s1 = ds_elem_from_string(te_getattrc(te, "s1"));
    ds_elem ds_s2 = ds_elem_from_string(te_getattrc(te, "s2"));

    const char*    data1 = NULL;
    nb_size    data1len = 0;
    boolean    freedata1 = FALSE;

    const char*    data2 = NULL;
    nb_size    data2len = 0;
    boolean    freedata2 = FALSE;

    is->dataSource(ds_s1, nman, is->dsuser, &data1, &data1len, &freedata1);
    is->dataSource(ds_s2, nman, is->dsuser, &data2, &data2len, &freedata2);

    if (data1 == NULL && data2 == NULL)
        retval = TRUE;
    else if (data1 == NULL && data2 != NULL)
        retval = FALSE;
    else if (data1 != NULL && data2 == NULL)
        retval = FALSE;
    else if (data1len != data2len)
        retval = FALSE;
    else
        retval = (boolean) (nsl_memcmp(data1, data2, data1len) == 0);

    if (freedata1 && data1 != NULL)
        nsl_free((char*)data1);
    if (freedata2 && data2 != NULL)
        nsl_free((char*)data2);

    return retval;
}

tpselt
getturnsound(instructset* is, uint32 nman, const char* turn_code, const char* pos, const char* vehicle_type)
{
    tpselt te_turnsounds = te_getchild(is->config, "turn-sounds");
    tpselt te;
    int iter = 0;

    if (te_turnsounds == NULL)
        return NULL;
#ifdef ENABLE_LOGGING
            debugf("find 'define-turn-sound' for turn_code - %s pos - %s", turn_code,pos);
#endif
    while ((te = te_nextchild(te_turnsounds, &iter)) != NULL) {
#ifdef ENABLE_LOGGING
            // comment this out if do not want verbose logs
            debugf("current 'define-turn-sound' has turn_code - %s pos - %s", te_getattrc(te, "turn"), te_getattrc(te, "pos"));
#endif
        if (str_cmpx(te_getname(te), "define-turn-sound") == 0 &&
            str_match_simple_pattern(te_getattrc(te, "pos"), pos) &&
            str_match_simple_pattern(te_getattrc(te, "turn"), turn_code) &&
            str_match_simple_pattern(te_getattrc(te, "vehicle-type"), vehicle_type))
        {
#ifdef ENABLE_LOGGING
            debugf("FOUND 'define-turn-sound' has turn_code - %s pos - %s", te_getattrc(te, "turn"), te_getattrc(te, "pos"));
#endif
            return te;
        }
    }

    return NULL;
}

tpselt
gettransitionsound(instructset* is, uint32 nman, const char* turn_code, const char* pos, const char* vehicle_type)
{
    tpselt te_turnsounds = te_getchild(is->config, "transition-sounds");
    tpselt te = NULL;
    int iter = 0;

    if (te_turnsounds == NULL)
        return NULL;

    while ((te = te_nextchild(te_turnsounds, &iter)) != NULL) {

        if (str_cmpx(te_getname(te), "define-transition-sound") == 0 &&
            str_match_simple_pattern(te_getattrc(te, "pos"), pos) &&
            str_match_simple_pattern(te_getattrc(te, "turn"), turn_code))
            return te;
    }

    return NULL;
}

const char*
getnumsound(instructset* is, const char* num, const char* context_case)
{
    // This checks the num-sounds rule of voices-xx-xx.sexp file
    tpselt te_numsounds = te_getchild(is->config, "num-sounds");
    tpselt te = NULL;
    int iter = 0;
    boolean ignoreGender = FALSE, ignoreCase = FALSE;

    if (te_numsounds == NULL)
        return NULL;

    // loop through children define-num-sound
    while ((te = te_nextchild(te_numsounds, &iter)) != NULL) {

        ignoreGender = FALSE;
        ignoreCase = FALSE;

        // get gender value neuter/masculine/feminine
        if (te_getattrc(te, "gender") == NULL)
            ignoreGender = TRUE;

        // get case value normative/dative
        if (te_getattrc(te, "case") == NULL)
            ignoreCase = TRUE;

        // returns sound id value if found;
        // if value is not there, empty string "" is returned for te_getattrc
        if (str_cmpx(te_getname(te), "define-num-sound") == 0 &&
            str_match_simple_pattern(te_getattrc(te, "num"), num) &&
            (str_match_simple_pattern(te_getattrc(te, "gender"), is->pgctchGender) || ignoreGender) &&
            (str_match_simple_pattern(te_getattrc(te, "case"), context_case) || ignoreCase))
        {
            tpselt te_child = te_getchild(te, "sound");
            if (te_child)
            {
                return te_getattrc(te_child, "id");
            }
        }
    }

    // if all fails
    return NULL;
}

void
getwordattribute(instructset* is, const char* word)
{
    // This checks the num-sounds rule of voices-xx-xx.sexp file
    tpselt te_wordattributes = te_getchild(is->config, "word-attributes");
    tpselt te = NULL;
    tpselt child = NULL;
    int iter = 0;

    if (te_wordattributes == NULL)
        return ;

    // loop through children define-word-attribute
    while ((te = te_nextchild(te_wordattributes, &iter)) != NULL) {

        // if define-word-attribute and child word exist,
        if (str_cmpx(te_getname(te), "define-word-attribute") == 0 &&
            str_match_simple_pattern(te_getattrc(te, "word"), word))
        {
            // type is child of gender
            child = te_getchild(te, "gender");
            if (child == NULL)
            {
                return;
            }

            // if type exist, return type value masculine or feminine
            if (str_cmpx(te_getname(child), "gender") == 0)
                is->pgctchGender = te_getattrc(child, "type");

            // get out once found.
            return;
        }

    }
    return ;
}



const char*
get_onesfn(int n)
{
    switch(n) {
        case 0:        return VID_ZERO;
        case 1:        return VID_ONE;
        case 2:        return VID_TWO;
        case 3:        return VID_THREE;
        case 4:        return VID_FOUR;
        case 5:        return VID_FIVE;
        case 6:        return VID_SIX;
        case 7:        return VID_SEVEN;
        case 8:        return VID_EIGHT;
        case 9:        return VID_NINE;
        case 10:    return VID_TEN;
        case 11:    return VID_ELEVEN;
        case 12:    return VID_TWELVE;
        case 13:    return VID_THIRTEEN;
        case 14:    return VID_FORTEEN;
        case 15:    return VID_FIFTEEN;
        case 16:    return VID_SIXTEEN;
        case 17:    return VID_SEVENTEEN;
        case 18:    return VID_EIGHTEEN;
        case 19:    return VID_NINETEEN;
    };
    return "";
}
const char*
get_tensfn(int n)
{
    switch (n) {
        case 0:        return VID_TWENTY;
        case 1:        return VID_THIRTY;
        case 2:        return VID_FORTY;
        case 3:        return VID_FIFTY;
        case 4:        return VID_SIXTY;
        case 5:        return VID_SEVENTY;
        case 6:        return VID_EIGHTY;
        case 7:        return VID_NINETY;
    };

    return "";
}


static void
AddBaseNameToInstructionArray(instructset* is, InstructionCodes* codes, const char* name, nb_size namelen)
{
    char logname[64] = {0};

    if (namelen < 0)
    {
        namelen = (nb_size)nsl_strlen(name);
    }

    CopyInstructionName(logname, name, MIN(namelen+1, sizeof(logname)));

    NB_QaLogAudioBase(is->context, logname);

    AddNameToInstructionArray(is, codes, name, namelen);
}

static void
AddNameToInstructionArray(instructset* is, InstructionCodes* codes, const char* name, nb_size namelen)
{
    if (namelen < 0)
    {
        namelen = (nb_size)nsl_strlen(name);
    }
#ifdef ENABLE_LOGGING
    debugf("added code #%d, code - %s", codes->count, name);
#endif
    // Just add the audio instruction code to the array.
    if (codes->count < (sizeof(codes->codes)/sizeof(codes->codes[0])))
    {
        CopyInstructionName(codes->codes[(codes->count)++], name, MIN(namelen+1, sizeof(codes->codes[0])));
    }
}

NB_Error
AddCacheIDtoInstructionArray(instructset* is, InstructionCodes* codes, ds_elem ds_elem, uint32 nman)
{
    NB_Error    err         = NE_OK;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;
    CSL_Cache*  voiceCache  = NULL;

    // This calls navcore_data_source() or directions_data_source()
    // This returns the cache id
    if (is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata))
    {
        // Add the cache id as instruction code
        AddNameToInstructionArray(is, codes, data, datalen);

        // Verify item with cache id is in voice cache
        voiceCache = NB_ContextGetVoiceCache(is->context);
        if (voiceCache && !CSL_CacheIsItemInCache(voiceCache, (byte*)data, datalen))
        {
            // Voice cache currently does not contain item
            err = NE_NOENT;

            //Ignore it for NIM# pronouns
            if (nsl_strncmp(data, "NIM", 3) == 0)
            {
                err = NE_OK;
            }

            //Ignore it when the TTS enabled.
            if (is->dataSource(ttsenabled, nman, is->dsuser, &data, &datalen, &freedata))
            {
                if ((uint32)(*data) == TRUE)
                {
                    err = NE_OK;
                }
            }
        }
    }
    else
    {
        // Cache id was not provided
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_integer_ex(instructset* is, InstructionCodes* codes, double num, tpselt te_numsounds, NavAudioBuilderConfiguration* builderConfig)
{
    // this function supports spoken numbers for other languages than English.
    tpselt te = NULL;
    int iter = 0;
    int position_integer = 0;
    int val = 0;
    int num_as_int = 0;
    const char* roman = 0;
    const char* numString;
    char intToStr[12] = { 0 };
    char pattern[50] = { 0 };
    char temp[50] = { 0 };
    nb_size lengthTemp = 0;

    //We should never try to append a negative number audio
    if (num < 0.0)
    {
        return NE_OK;
    }

    num_as_int = (int) num;        // takes up integer portion of num

    if (te_numsounds == NULL)
        return NE_OK;

    // define-num-rule from voice-xx-xx.sexp file
    // loop through it.
    // Algorithm:
    // Find position of the number and extract out the digit from that number base position.
    // Compare it with value in voice sexp file.
    // If it matches, go to the next child of the rule; recurse again.
    // When determining num-sound, find out the digit of its base position or
    // simply sound off the number (esp. true for non-English language; special word
    // for number greater than 10, for eg.)
    while ((te = te_nextchild(te_numsounds, &iter)) != NULL) {

        // proceed to get num value if child matches one of these 3.
        if (str_cmpx(te_getname(te), "if-GT") == 0 ||
            str_cmpx(te_getname(te), "if-EQ") == 0 ||
            str_cmpx(te_getname(te), "if-LT") == 0)
        {

            // get num value; M,C,I,X,X0,MCXI,XI or i.
            roman = te_getattrc(te, "num");

            // get the digit of the integer.
            if(str_cmpx(roman, "M") == 0)
                position_integer = num_as_int / 1000;
            else if(str_cmpx(roman, "C") == 0)
                position_integer = (num_as_int % 1000) / 100;
            else if( str_cmpx(roman, "XI") == 0)
                position_integer = (num_as_int % 1000) % 100;
            else if( str_cmpx(roman, "X0") == 0)
                position_integer = ((num_as_int % 1000) % 100) - (((num_as_int % 1000) % 100) % 10);
            else if(str_cmpx(roman, "X") == 0)
                position_integer = ((num_as_int % 1000) % 100) / 10;
            else if(str_cmpx(roman, "I")  == 0 )
                position_integer = ((num_as_int % 1000) % 100) % 10;
            else if(str_cmpx(roman, "i") == 0)
                position_integer = (int)((num - (double)num_as_int) * 10);
            else if(str_cmpx(roman, "MCXI") == 0)
                position_integer = (int)num;


            // convert digit from int to string if matches if-EQ
            if(str_cmpx(te_getname(te), "if-EQ") == 0)
            {
                nsl_sprintf(intToStr, "%d", position_integer);
            }
            val = nsl_atoi(te_getattrc(te, "val"));

            // The attribute value string may be surrounded by single-quotes (').
            // This is even recommended for sequences, because it allows easier parsing by Python-based tools.
            // Remove them for our code
            nsl_strlcpy(temp, te_getattrc(te, "val"), sizeof(temp));
            lengthTemp = (nb_size)nsl_strlen(temp);
            if (temp[0] == '\'' && temp[lengthTemp - 1] == '\'')
            {
                nsl_strncpy(pattern, temp + 1, lengthTemp - 2);
            }
            else
            {
                nsl_strlcpy(pattern, temp, sizeof(pattern));
            }

            // deals with possible case where values could be one of these - 2|3|4|5|6
            if ((str_cmpx(te_getname(te), "if-GT") == 0 && position_integer > val) ||
                (str_cmpx(te_getname(te), "if-EQ") == 0 && str_match_simple_pattern(pattern, intToStr)) ||
                (str_cmpx(te_getname(te), "if-LT") == 0 && position_integer < val)
                )
            {
                // recurse function to find next level num-sound
                audio_append_integer_ex(is, codes, num, te, builderConfig);
                continue;
            }
        }

        // deals with num-sound child
        if (str_cmpx(te_getname(te), "num-sound") == 0)
        {
            // for num-sound children; ger value of num of num-sound
            roman = te_getattrc(te, "num");

            // get the digit of the integer.
            if(str_cmpx(roman, "M") == 0)
                position_integer = num_as_int / 1000;
            else if(str_cmpx(roman, "C") == 0)
                position_integer = (num_as_int % 1000) / 100;
            else if(str_cmpx(roman, "C00") == 0)
                position_integer = num_as_int % 1000;
            else if( str_cmpx(roman, "XI") == 0)
                position_integer = (num_as_int % 1000) % 100;
            else if( str_cmpx(roman, "X0") == 0)
                position_integer = ((num_as_int % 1000) % 100) - (((num_as_int % 1000) % 100) % 10);
            else if(str_cmpx(roman, "X") == 0)
                position_integer = ((num_as_int % 1000) % 100) / 10;
            else if(str_cmpx(roman, "I")  == 0 )
                position_integer = ((num_as_int % 1000) % 100) % 10;
            else if(str_cmpx(roman, "i") == 0)
                position_integer = (int)((num - (double) num_as_int) * 10);
            else if(str_cmpx(roman, "MCXI") == 0)
                position_integer = (int)num;
            else
            {
                position_integer = (int)nsl_strtoul(roman, NULL, 0);
            }

            // handle half differently
            if (position_integer == 0 && str_cmpx(roman, "0.5") == 0)
            {
                nsl_sprintf(intToStr, "%s", roman);
            }
            else
            {
                // convert digit from int to string if matches if-EQ
                nsl_sprintf(intToStr, "%d", position_integer);
            }

            // get sound id value
            numString = getnumsound(is, (const char*) intToStr, te_getattrc(te, "case"));
            if (numString == NULL)
                continue;

            AddBaseNameToInstructionArray(is, codes, numString, -1);
            continue;
        }

        if (str_cmpx(te_getname(te), "sound") == 0)
        {
            numString = te_getattrc(te, "id");
            if (numString != NULL)
            {
                AddBaseNameToInstructionArray(is, codes, numString, -1);
            }
            continue;
        }
        if (str_cmpx(te_getname(te), "force-singular") == 0)
        {
            if(builderConfig != NULL)
            {
                builderConfig->isUnitOverride = TRUE;
                nsl_strlcpy(builderConfig->overrideUnit, builderConfig->singularUnit, MAX_INSTRUCTION_CODE_LENGTH);
                audio_append_integer_ex(is, codes, num, te, builderConfig);
                builderConfig->isUnitOverride = FALSE;
            }
            continue;
        }
        if (str_cmpx(te_getname(te), "force-plural") == 0)
        {
            if(builderConfig != NULL)
            {
                builderConfig->isUnitOverride = TRUE;
                nsl_strlcpy(builderConfig->overrideUnit, builderConfig->pluralUnit, MAX_INSTRUCTION_CODE_LENGTH);
                audio_append_integer_ex(is, codes, num, te, builderConfig);
                builderConfig->isUnitOverride = FALSE;
            }
            continue;
        }
        if (str_cmpx(te_getname(te), "distance-unit") == 0)
        {
            if(builderConfig != NULL)
            {
                AddBaseNameToInstructionArray(is, codes, builderConfig->isUnitOverride?builderConfig->overrideUnit:builderConfig->unit, -1);
                builderConfig->isUnitruleUsed = TRUE;
            }
            continue;
        }
    }
    return NE_OK;
}

NB_Error
audio_append_integer(instructset* is, InstructionCodes* codes, int num, NavAudioBuilderConfiguration* builderConfig)
{
    NB_Error err    = NE_OK;

    tpselt te_numsounds = te_getchild(is->config, "define-num-rule");

    //We should never try to append a negative number audio
    if (num < 0)
    {
        return NE_OK;
    }

    if (te_numsounds != NULL)
    {
        // non-English spoken numbering rule
        err = audio_append_integer_ex(is, codes, (double)num, te_numsounds, builderConfig);
    }
    else
    {
        int thousands   = num / 1000;
        int hundreds    = (num - (thousands * 1000)) / 100;
        int remain      = num - thousands * 1000 - hundreds * 100;
        int tens        = 0;
        int ones        = 0;

        if (thousands > 0)
        {
            err = audio_append_integer(is, codes, thousands, builderConfig);

            if (err != NE_OK)
            {
                goto errexit;
            }
            AddBaseNameToInstructionArray(is, codes, VID_THOUSAND, -1);
        }

        if (hundreds > 0)
        {
            err = err ? err : audio_append_integer(is, codes, hundreds, builderConfig);
            if (err != NE_OK)
            {
                goto errexit;
            }

            AddBaseNameToInstructionArray(is, codes, VID_HUNDRED, -1);
        }

        if (remain > 0)
        {
            tens = remain / 10;
            ones = remain - (tens * 10);

            if (tens < 2)
            {
                ones += tens * 10;
                tens = 0;
            }

            nsl_assert((tens >= 0 && tens < 10)); // tens out of range

            if (tens > 0)
            {
                AddBaseNameToInstructionArray(is, codes, get_tensfn(tens-2), -1);

                if (err != NE_OK)
                {
                    goto errexit;
                }
            }

            nsl_assert((ones >= 0 && ones < 20)); // ones out of range

            if (ones > 0)
            {
                AddBaseNameToInstructionArray(is, codes, get_onesfn(ones), -1);

                if (err != NE_OK)
                goto errexit;
            }
        }
    }

errexit:
    return err;
}

NB_Error
audio_append_float(instructset* is, InstructionCodes* codes, double num, boolean round, boolean sayfrac, NavAudioBuilderConfiguration* builderConfig, boolean* plurar)
{
    NB_Error err = NE_OK;
    int n = 0;
    int f = 0;
    tpselt te_numsounds = NULL;

    n = (int)num;
    f = (int)(100.0 * (num - (double)n));

    te_numsounds = te_getchild(is->config, "define-num-rule");

    if (te_numsounds != NULL)
    {
        // non-English spoken numbering rule
        err = audio_append_integer_ex(is, codes, num, te_numsounds, builderConfig);
    }
    else
    {
        err = err ? err : audio_append_integer(is, codes, n, builderConfig);

        if (err != NE_OK)
        {
            goto errexit;
        }

        if (sayfrac && f > 0.0)
        {
            AddBaseNameToInstructionArray(is, codes, VID_POINT, -1);
            err = audio_append_integer(is, codes, f, builderConfig);
            if (plurar)
            {
                *plurar = TRUE;
            }
            return err;
        }
        else if(round && f > 0.0)
        {
            if (n > 0)
            {
                if (f == 50)
                {
                    AddBaseNameToInstructionArray(is, codes, VID_AND, -1);
                    AddBaseNameToInstructionArray(is, codes, VID_A_HALF, -1);
                }
            }
            else
            {
                if (f == 25)
                {
                    AddBaseNameToInstructionArray(is, codes, VID_A_QUARTER, -1);
                }
                else
                {
                    AddBaseNameToInstructionArray(is, codes, VID_HALF_A, -1);
                }
            }
            if (plurar)
            {
                *plurar = (n != 1) ? TRUE : FALSE;
            }
        }
    }

errexit:
    return err;
}

NB_Error
audio_append_distance(instructset* is, InstructionCodes* codes, double dist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units)
{
    NB_Error err = NE_OK;
    nb_boolean metric = (units == NB_NAU_Kilometers) ? TRUE : FALSE;
    nb_boolean nonMetricYards = (units == NB_NAU_MilesYards) ? TRUE : FALSE;
    double tempDist = dist;
    boolean smallUnits = FALSE;
    boolean sayFrac = FALSE;
    NavAudioBuilderConfiguration* builderConfig = NULL;

    if (metric)
    {
        dist = tempDist / 1000.0;    // convert Meters to Kilometers
    }
    else
    {
        dist = tempDist / 1609.0;    // convert Meters to Miles
    }

    if ((dist <= 0.17 && showunits && !metric)/* mi */ || (dist <= 0.375 && showunits && metric)/* km->m */)
    {
        smallUnits = TRUE;

        if (metric)
        {
            if (round)
            {
                if(dist <= 0.15)
                {
                    dist = 100;
                }
                else if(dist <= 0.25)
                {
                    dist = 200;
                }
                else if(dist <= 0.375)
                {
                    dist = 300;
                }
            }
            else
            {
                dist = tempDist;    // meters
            }
        }
        else if(nonMetricYards)
        {
            if (round)
            {
                if(dist <= 0.04)
                {
                    dist = 50;
                }
                else if(dist <= 0.07)
                {
                    dist = 100;
                }
                else if(dist <= 0.17)
                {
                    dist = 200;
                }
            }
            else
            {
                dist = tempDist * 1.0936;    // yards
            }
        }
        else
        {
            if (round)
            {
                if(dist <= 0.03)
                {
                    dist = 100;
                }
                else if(dist <= 0.05)
                {
                    dist = 200;
                }
                else if(dist <= 0.07)
                {
                    dist = 300;
                }
                else if(dist <= 0.17)
                {
                    dist = 500;
                }
            }
            else
            {
                dist = tempDist * 3.2808;    // feet
            }
        }

        if(round)
        {
            // Ignore the decimal part, since there are impossible case for real case
            dist = (int)(dist + 0.5);
        }
    }
    else
    {
        sayFrac = ((dist < dec_limit) && (!round)) ? TRUE : FALSE;
        if(round)
        {
            int n = 0;
            double f = 0;

            n = (int)dist;
            f = dist - n;

            if (n < 10)
            {
                if(metric)
                {
                    if ((f < 0.375) && (n == 0))
                    {
                        f = 0.5;
                    }
                    else if (f < 0.375)
                    {
                        f = 0;
                    }
                    else if ((f >= 0.375) && (f < 0.75))
                    {
                        f = 0.5;
                    }
                    else
                    {
                        n += 1;
                        f = 0;
                    }
                }
                else
                {
                    if ((f < 0.17) && (n == 0))
                    {
                        f = 0.25;
                    }
                    else if (f < 0.17)
                    {
                        f = 0;
                    }
                    else if ((f >= 0.17) && (f < 0.375))
                    {
                        f = 0.25;
                    }
                    else if ((f >= 0.375) && (f < 0.75))
                    {
                        f = 0.5;
                    }
                    else
                    {
                        n += 1;
                        f = 0;
                    }
                }
            }
            else
            {
                if (f > 0.5)
                {
                    n += 1;
                }
                f = 0;
            }

            dist = n + f;
        }
    }

    builderConfig = getUnitTypeOfFloat(metric, nonMetricYards, smallUnits, dist);
    // Need to determine the gender attribute first for instructset_getnumsound() to get sound id
    getwordattribute(is,  builderConfig->unit);
    err = audio_append_float(is, codes, dist, round, sayFrac, builderConfig, NULL);
    if (err != NE_OK)
    {
        goto errexit;
    }

    if(!builderConfig->isUnitruleUsed)
    {
        AddBaseNameToInstructionArray(is, codes, builderConfig->unit, -1);
    }
errexit:
    if (builderConfig)
    {
        nsl_free(builderConfig);
    }
    return (err);
}


static NavAudioBuilderConfiguration* getUnitTypeOfFloat(boolean metric, boolean nonMetricYards, boolean smallUnits, double num)
{
    nb_boolean plural = FALSE;
    int n = 0;
    int f = 0;
    int r = 0;

    n = (int)num;
    f = (int)(100.0 * (num - n));
    r = n * 100 + f;

    if(r == 100 || r == 50 || r == 25 || r == 125)
    {
        plural = FALSE;
    }
    else
    {
        plural = TRUE;
    }

    if (metric)
    {
        if (smallUnits)
        {
            return createBuildConfig(plural ? VID_METERS : VID_METER, VID_METER, VID_METERS);
        }
        else
        {
            return createBuildConfig((plural ? VID_KILOMETERS : VID_KILOMETER), VID_KILOMETER, VID_KILOMETERS);
        }
    }
    else
    {
         if (smallUnits)
         {
             if(nonMetricYards)
             {
                 return createBuildConfig(plural ? VID_YARDS : VID_YARD, VID_YARD, VID_YARDS);
             }
             else
             {
                 return createBuildConfig(plural ? VID_FEET : VID_FOOT, VID_FOOT, VID_FEET);
             }

         }
         else
         {
             return createBuildConfig(plural ? VID_MILES : VID_MILE, VID_MILE, VID_MILES);
         }
    }
}

static NavAudioBuilderConfiguration* createBuildConfig(const char* unit, const char* singularUnit, const char* pluralUnit)
{
    NavAudioBuilderConfiguration* buildConfig = nsl_malloc(sizeof(NavAudioBuilderConfiguration));
    nsl_memset(buildConfig, 0, sizeof(NavAudioBuilderConfiguration));
    nsl_strlcpy(buildConfig->unit, unit, MAX_INSTRUCTION_CODE_LENGTH);
    nsl_strlcpy(buildConfig->singularUnit, singularUnit, MAX_INSTRUCTION_CODE_LENGTH);
    nsl_strlcpy(buildConfig->pluralUnit, pluralUnit, MAX_INSTRUCTION_CODE_LENGTH);

    return buildConfig;
}

#if 0
// deprecated
NB_Error
audio_append_distance_with_half_increment(instructset* is, InstructionCodes* codes, double dist, boolean showunits, double dec_limit, NB_NavigateAnnouncementUnits units)
{
    NB_Error err = NE_OK;
    nb_boolean metric = (units == NB_NAU_Kilometers) ? TRUE : FALSE;
    boolean plural = FALSE;
    double tempDist = dist;
    int n = 0;
    double f = 0;

    if (metric)
    {
        dist = tempDist / 1000.0;    // convert Meters to Kilometers
    }
    else
    {
        dist = tempDist / 1609.0;    // convert Meters to Miles
    }
    n = (int)dist;
    f = dist - n;

    if (n < 10)
    {
        if (f < 0.25)
        {
            if (n > 0)
            {
                f = 0;
            }
            else
            {
                err = audio_append_float(is, codes, dist, TRUE, TRUE, NULL, NULL);
                if (err != NE_OK)
                {
                    goto errexit;
                }
            }
        }
        else if ((f >= 0.25) && (f < 0.75))
        {
            f = 1;
        }
        else
        {
            n += 1;
            f = 0;
        }
    }
    else
    {
        if(f>0.5)
        {
            n += 1;
        }
        f = 0;
    }

    if (n > 0)
    {
        err = audio_append_integer(is, codes, n, NULL);
        if (err != NE_OK)
        {
            goto errexit;
        }
        if (n > 1)
        {
            plural = TRUE;
        }
        if (f > 0)
        {
            plural = TRUE;
            AddBaseNameToInstructionArray(is, codes, VID_AND, -1);
        }
    }

    if(f > 0 && n > 0)
    {
        AddBaseNameToInstructionArray(is, codes, VID_A_HALF, -1);
    }
    else if(f >= 0.25)
    {
        AddBaseNameToInstructionArray(is, codes, VID_HALF_A, -1);
    }

    if (showunits)
    {
        if(metric)
        {
            AddBaseNameToInstructionArray(is, codes, plural ? VID_KILOMETERS : VID_KILOMETER, -1);
        }
        else
        {
            AddBaseNameToInstructionArray(is, codes, plural ? VID_MILES : VID_MILE, -1);
        }
    }

    is->soundTypes |= REAL_TIME_DATA_MASK;

errexit:
    return (err);
}
#endif

NB_Error
audio_append_distance_with_adjust(instructset* is, InstructionCodes* codes, double dist, boolean showunits, double dec_limit, boolean round, NB_NavigateAnnouncementUnits units, double current_speed)
{
    NB_GetAnnounceSnippetLength snippetLengthCallback = NULL;
    void* callbackData = NULL;
    double playTime = 0.0;

    if (NE_OK == NB_ContextGetSnippetLengthCallback(is->context, &snippetLengthCallback, &callbackData))
    {
        int i = 0;
        for (; i < codes->count; i++)
        {
            playTime += snippetLengthCallback(codes->codes[i], callbackData);
        }
    }

    dist += (playTime * current_speed);

    return audio_append_distance(is, codes, dist, showunits, dec_limit, round, units);
}

NB_Error
audio_append_time(instructset* is, InstructionCodes* codes, uint32 val, boolean sayunits, boolean round)
{
    NB_Error err = NE_OK;
    uint32 seconds_in = 0;
    uint32 hours = 0;
    uint32 minutes = 0;
    uint32 seconds = 0;
    const char* audioName = NULL;
    NavAudioBuilderConfiguration* builderConfigHours = NULL;
    NavAudioBuilderConfiguration* builderConfigMinutes = NULL;

    // val is in seconds
    seconds_in = val;

    // Need to determine the gender attribute first for instructset_getnumsound() to get sound id
    getwordattribute(is, VID_SECONDS);

    seconds_to_h_m_s(seconds_in, &hours, &minutes, &seconds);

    if (hours > 0)
    {
        if (hours > 1)
        {
            audioName = VID_HOURS;
        }
        else
        {
            audioName = VID_HOUR;
        }
        getwordattribute(is, audioName);
        builderConfigHours = createBuildConfig(audioName, VID_HOUR, VID_HOURS);
        err = err ? err : audio_append_integer(is, codes, (int) hours, builderConfigHours);
        if (!builderConfigHours->isUnitruleUsed)
        {
            AddBaseNameToInstructionArray(is, codes, audioName, -1);
        }
    }

    if (minutes > 0)
    {
        if (minutes > 1)
        {
            audioName = VID_MINUTES;
        }
        else
        {
            audioName = VID_MINUTE;
        }
        getwordattribute(is, audioName);
        builderConfigMinutes = createBuildConfig(audioName, VID_MINUTE, VID_MINUTES);
        err = err ? err : audio_append_integer(is, codes, (int) minutes, builderConfigMinutes);
        if (!builderConfigMinutes->isUnitruleUsed)
        {
            AddBaseNameToInstructionArray(is, codes, audioName, -1);
        }
    }

    if (builderConfigHours)
    {
        nsl_free(builderConfigHours);
    }
    if (builderConfigMinutes)
    {
        nsl_free(builderConfigMinutes);
    }

    return err;
}

NB_Error
append_turn_sound_impl(instructset* is, tpselt te, instructtype type,
                          void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units,
                          NB_NavigateManeuverPos pos, ds_elem ds_elem, turn_sound_func* getsoundfunc)
{
    NB_Error err = NE_OK;
    tpselt te_ts = NULL;
    const char*    turncode = NULL;
    nb_size    turncodelen = 0;
    boolean    freeturncode = FALSE;
    const char* posOverride;
    const char*    vehicle_type = NULL;

    if (is->dataSource(ds_elem, nman, is->dsuser, &turncode, &turncodelen, &freeturncode))
    {
        posOverride = te_getattrc(te, "pos");
        vehicle_type = te_getattrc(te, "vehicle-type");

        if (posOverride == NULL)
            posOverride = nav_pos_to_string(pos);

        te_ts = getsoundfunc(is, nman, turncode, posOverride, vehicle_type);
    }

    if (te_ts != NULL)
    {
        err = err ? err : append_children(is, te_ts, type, instruct_state, nman, units, pos);
    }

    if (freeturncode && turncode != NULL)
    {
        nsl_free((char*)turncode);
    }

    return err;
}

void
text_dynbuf_append_font(instructset* is, struct dynbuf* dbp, NB_Font fnt)
{
#ifndef NDEBUG
#if 0
// too many BREW dependencies
    const char* fName = "unknown";
    switch (fnt) {

        case NB_Font_NORMAL:
            fName = "normal";
            break;
        case NB_Font_BOLD:
            fName = "bold";
            break;
        case NB_Font_LARGE:
            fName = "large";
            break;
        case NB_Font_LARGE_BOLD:
            fName = "large-bold";
            break;
        case NB_Font_CUSTOM1:
            fName = "custom-1";
            break;
        case NB_Font_CUSTOM2:
            fName = "custom-2";
            break;
        case NB_Font_CUSTOM3:
            fName = "custom-3";
            break;
        case NB_Font_CUSTOM4:
            fName = "custom-4";
            break;
    }
    DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_INFO, ("Font: %s", fName));
#endif
#endif
    dbufadd(dbp, TEXT_FONT_TOKEN);
    dbufcat(dbp, (const byte*)&fnt, sizeof(fnt));
}

void
text_dynbuf_append_color(instructset* is, struct dynbuf* dbp, nb_color rgb)
{
    DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_INFO, ("Color: (%d,%d,%d)", ((rgb >> 8) & 0xFF), ((rgb >> 16) & 0xFF),((rgb >> 24) & 0xFF)));

    dbufadd(dbp, TEXT_COLOR_TOKEN);
    dbufcat(dbp, (const byte*)&rgb, sizeof(rgb));
}

void
text_dynbuf_append_text(instructset* is, struct dynbuf* dbp, const char* txt)
{
    nb_size len = (nb_size)(nsl_strlen(txt)+1);

    DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_INFO, ("Text: \"%s\"", txt));

    dbufadd(dbp, TEXT_TEXT_TOKEN);
    dbufcat(dbp, (const byte*)&len, sizeof(len));
    dbufcat(dbp, (const byte*)txt, len);
}

void
text_dynbuf_append_newline(instructset* is, struct dynbuf* dbp)
{
    DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_INFO, ("New Line"));

    dbufadd(dbp, TEXT_NEWLINE_TOKEN);
}

NB_Error
text_dynbuf_append_data(instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman)
{
    NB_Error err = NE_OK;
    const char*    data = NULL;
    nb_size    datalen = 0;
    boolean    freedata = FALSE;
    nb_size    len = 0;

    if (!is->dataSource(ds_elem, nman, is->dsuser, &data, &datalen, &freedata))
    {
        data = NULL;
        datalen = 0;
    }
    else
    {
        len = (nb_size)(nsl_strlen(data)+1);

        dbufadd(dbp, TEXT_TEXT_TOKEN);
        dbufcat(dbp, (const byte*)&len, sizeof(len));
        dbufcat(dbp, (const byte*) data, (size_t) datalen);
        err = err ? err : dbuferr(dbp);

        if (freedata)
        {
            nsl_free((void*)data);
        }
    }

    return err;
}

NB_Error
text_dynbuf_append_name(instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman)
{
    NB_Error err = NE_OK;

    if (getplace(is, nman, ds_elem, &is->temp_place) && nsl_strlen(is->temp_place.name) > 0) {

        text_dynbuf_append_text(is, dbp, is->temp_place.name);
        err = dbuferr(dbp);
    }

    return err;
}

NB_Error
text_dynbuf_append_address(instructset* is, struct dynbuf* dbp, ds_elem ds_elem,
                           uint32 nman, boolean multiline)
{
    NB_Error err = NE_OK;
    int n;
    char    txt[512] = {0};

    if (getplace(is, nman, ds_elem, &is->temp_place))
    {
        if (multiline)
        {
            for (n=0; n<FORMAT_LOC_LINES;n++)
            {
                //FormatLocation(is->nbState, &is->temp_place.location, nsl_strlen(is->temp_place.name) == 0 ? TRUE : FALSE, txt, sizeof(txt), n);
                //WSTRTOSTR(wtxt, txt, sizeof(txt));

                if (is->ff != NULL)
                {
                    err = err ? err : is->ff(is->ffuser, FORMAT_LOCATION, &is->temp_place.location, nsl_strlen(is->temp_place.name) == 0 ? FORMAT_FLAGS_ALLOW_LATLON : FORMAT_FLAGS_NONE, txt, sizeof(txt), n);
                }

                text_dynbuf_append_text(is, dbp, txt);
                text_dynbuf_append_newline(is, dbp);
            }
        }
        else
        {
            //FormatLocation(is->nbState, &is->temp_place.location, nsl_strlen(is->temp_place.name) == 0 ? TRUE : FALSE, txt, sizeof(txt), FORMAT_LOC_ALL);
            //STRTOWSTR(wtxt, txt, sizeof(txt));

            if (is->ff)
            {
                err = err ? err : is->ff(is->ffuser, FORMAT_LOCATION,  &is->temp_place.location, nsl_strlen(is->temp_place.name) == 0 ? FORMAT_FLAGS_ALLOW_LATLON : FORMAT_FLAGS_NONE, txt, sizeof(txt), FORMAT_INDEX_ALL);
            }

            text_dynbuf_append_text(is, dbp, txt);
        }
    }

    return err;
}

NB_Error
text_dynbuf_append_distance(instructset* is, struct dynbuf* dbp, uint32 nman, NB_NavigateAnnouncementUnits units)
{
    NB_Error err = NE_OK;
    char    txt[512] = {0};
    double distance;

    if (getdistance(is, nman, &distance)) {

        format_distance_f(distance, txt, sizeof(txt), TRUE, 20.0, TRUE, units, TRUE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, dbp, txt);

        err = dbuferr(dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_dynbuf_append_phonenum(instructset* is, struct dynbuf* dbp, ds_elem ds_elem, uint32 nman)
{
    NB_Error err = NE_OK;
    char    txt[FORMATTED_PHONE_LEN] = {0};

    if (is->ff == NULL)
        return NE_NOTINIT;

    if (getplace(is, nman, ds_elem, &is->temp_place) && PlaceHasPhoneNumber(&is->temp_place)) {

//        FormatPhone(is->nbState, &is->temp_place.phone[0], txt, sizeof(txt));
//        WSTRTOSTR(wtxt, txt, sizeof(txt));

        err = is->ff(is->ffuser, FORMAT_PHONE, &is->temp_place.phone[0], FORMAT_FLAGS_NONE, txt, sizeof(txt), FORMAT_INDEX_ALL);

        text_dynbuf_append_text(is, dbp, txt);
    }

    return err;
}

NB_Error
text_dynbuf_append_tsettings(instructset* is, struct dynbuf* dbp)
{
    NB_Error err = NE_OK;
    NB_RouteSettings tripsettings;
    char    txt[200] = {0};

    if (is->ff == NULL)
        return NE_NOTINIT;

    if (get_tripsettings(is, &tripsettings)) {

//        err = FormatRouteSettings(is->nbState, &tripsettings, txt, sizeof(txt));
//        WSTRTOSTR(wtxt, txt, sizeof(txt));

        err = is->ff(is->ffuser, FORMAT_ROUTESETTINGS, &tripsettings, FORMAT_FLAGS_NONE, txt, sizeof(txt), FORMAT_INDEX_ALL);

        text_dynbuf_append_text(is, dbp, txt);
    }

    return err;
}

/*! Get the image instruction code.

    Gets called from instructset_getdata()

    @return NB_Error

    @see instructset_getdata
*/
NB_Error
GetImageInstructionCode(instructset* is,
                        tpselt instruct_elt,        /*!< Input tps element. */
                        uint32 nman,                /*!< Manuever */
                        InstructionCodes* codes)    /*!< Returned instruction code. Only one entry is added. */
{
    tpselt  image_elt   = NULL;
    char*   id          = NULL;
    size_t    idlen       = 0;
    int     iter        = 0;

    boolean leftDrivingSide = (get_char(is, nman, dside) == 'L');

    while ((image_elt = te_nextchild(instruct_elt, &iter)) != NULL)
    {
        if (str_cmpx(te_getname(image_elt), "if-left-side-traffic") == 0)
        {
            if (leftDrivingSide)
            {
                image_elt = te_getchild(image_elt, "image");
            }
            else
            {
                continue;
            }
        }
        else if (str_cmpx(te_getname(image_elt), "if-right-side-traffic") == 0)
        {
            if (!leftDrivingSide)
            {
                image_elt = te_getchild(image_elt, "image");
            }
            else
            {
                continue;
            }
        }

        if (str_cmpx(te_getname(image_elt), "image") == 0)
        {
            if (te_getattr(image_elt, "id", &id, &idlen))
            {
                // Add the image instruction code to the array and return
                nsl_strlcpy(codes->codes[(codes->count)++], id, MAX_INSTRUCTION_CODE_LENGTH);
                return NE_OK;
            }
        }
    }

    return NE_NOENT;
}

NB_Error
append_children(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error        err             = NE_OK;
    tpselt            tex             = NULL;
    int                iter            = 0;
    append_func*    append_func     = NULL;

    if (is->dataSource == NULL)
    {
        return NE_INVAL;
    }
#ifdef ENABLE_LOGGING
        debugf("append_children is '%s'", te_getname(te));
#endif
    while ((tex = te_nextchild(te, &iter)) != NULL && err == NE_OK)
    {
        append_func = get_instruct_append_handler(is, type, te_getname(tex));

        if (append_func)
        {
#ifdef ENABLE_LOGGING
        debugf("append_children is '%s'", te_getname(tex));
#endif
            err = append_func(is, tex, type, instruct_state, nman, units, pos);
        }
    }
    return err;
}

append_func*
get_instruct_append_handler(instructset* is, instructtype type, const char* name)
{
    append_func* retval = NULL;

    // @todo (BUG 56053): All the string compare functions are really inefficient. It should use a hash table
    // or other more efficient ways.

    if (str_cmpx(name, "sound") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_sound;
    }
    else if (str_cmpx(name, "image") == 0) {

        // currently not handled in this framework
    }
    else if (str_cmpx(name, "opronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_opronun;
    }
    else if (str_cmpx(name, "dpronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_dpronun;
    }
    else if (str_cmpx(name, "dpronun-base") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_dpronun_base;
    }
    else if (str_cmpx(name, "ipronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_ipronun;
    }
    else if (str_cmpx(name, "if") == 0) {
        retval = append_if;
    }
    else if (str_cmpx(name, "dist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_dist;
        else if (is_txt_instruction(type))
            retval = text_append_dist;
    }
    else if (str_cmpx(name, "font") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_font;
    }
    else if (str_cmpx(name, "color") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_color;
    }
    else if (str_cmpx(name, "text") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_text;
    }
    else if (str_cmpx(name, "cardinal") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_cardinal;
    }
    else if (str_cmpx(name, "if-start-name") == 0) {

        retval = append_if_start_name;
    }
    else if (str_cmpx(name, "start-name") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_start_name;
    }
    else if (str_cmpx(name, "start-address") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_start_address;
    }
    else if (str_cmpx(name, "if-start-phonenum") == 0) {

        retval = append_if_start_phonenum;
    }
    else if (str_cmpx(name, "start-phonenum") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_start_phonenum;
    }
    else if (str_cmpx(name, "if-destination-name") == 0) {

        retval = append_if_destination_name;
    }
    else if (str_cmpx(name, "destination-name") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_destination_name;
    }
    else if (str_cmpx(name, "destination-address") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_destination_address;
    }
    else if (str_cmpx(name, "if-destination-phonenum") == 0) {

        retval = append_if_destination_phonenum;
    }
    else if (str_cmpx(name, "destination-phonenum") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_destination_phonenum;
    }
    else if (str_cmpx(name, "oname") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_oname;
    }
    else if (str_cmpx(name, "dname") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_dname;
    }
    else if (str_cmpx(name, "dname-base") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_dname_base;
    }
    else if (str_cmpx(name, "aname") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_aname;
    }
    else if (str_cmpx(name, "aname-base") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_aname_base;
    }
    else if (str_cmpx(name, "if-oname") == 0) {

        retval = append_if_oname;
    }
    else if (str_cmpx(name, "if-dname") == 0) {

        retval = append_if_dname;
    }
    else if (str_cmpx(name, "if-dname-base") == 0) {

        retval = append_if_dname_base;
    }
    else if (str_cmpx(name, "if-aname") == 0) {

        retval = append_if_aname;
    }
    else if (str_cmpx(name, "if-aname-base") == 0) {

        retval = append_if_aname_base;
    }
    else if (str_cmpx(name, "next") == 0) {

        retval = append_next;
    }
    else if (str_cmpx(name, "tripdist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_tdist;
        else if (is_txt_instruction(type))
            retval = text_append_tdist;
    }
    else if (str_cmpx(name, "triptime") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_ttime;
    }
    else if (str_cmpx(name, "newline") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_newline;
    }
    else if (str_cmpx(name, "if-equal") == 0) {

        retval = append_if_equal;
    }
    else if (str_cmpx(name, "if-not-equal") == 0) {

        retval = append_if_not_equal;
    }
    else if (str_cmpx(name, "if-stack") == 0) {

        retval = append_if_stack;
    }
    else if (str_cmpx(name, "if-not-stack") == 0) {

        retval = append_if_not_stack;
    }
    else if (str_cmpx(name, "turn-sound") == 0) {

        retval = append_turn_sound;
    }
    else if (str_cmpx(name, "stack-sound") == 0) {

        retval = append_stack_sound;
    }
    else if (str_cmpx(name, "transition-sound") == 0) {

        retval = append_transition_sound;
    }
    else if (str_cmpx(name, "stack-transition-sound") == 0) {

        retval = append_stack_transition_sound;
    }
    else if (str_cmpx(name, "if-toward-name") == 0)
    {
        retval = append_if_toward_name;
    }
    else if (str_cmpx(name, "if-not-toward-name") == 0)
    {
        retval = append_if_not_toward_name;
    }
    else if (str_cmpx(name, "if-unnamed") == 0) {

        retval = append_if_unnamed;
    }
    else if (str_cmpx(name, "if-not-unnamed") == 0) {

        retval = append_if_not_unnamed;
    }
    else if (str_cmpx(name, "if-next-named") == 0) {

        retval = append_if_next_named;
    }
    else if (str_cmpx(name, "if-not-next-named") == 0) {

        retval = append_if_not_next_named;
    }
    else if (str_cmpx(name, "next-named") == 0) {

        retval = append_next_named;
    }
    else if (str_cmpx(name, "if-next-named-dist-lt-max") == 0) {

        retval = append_if_next_named_dist_lt_max;
    }
    else if (str_cmpx(name, "next-named-dist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_next_named_dist;
    }
    else if (str_cmpx(name, "tripsettings") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_tsettings;
    }
    else if (str_cmpx(name, "if-lookahead") == 0) {

        retval = append_if_lookahead;
    }
    else if (str_cmpx(name, "if-not-lookahead") == 0) {

        retval = append_if_not_lookahead;
    }
    else if (str_cmpx(name, "if-navigating") == 0) {

        retval = append_if_navigating;
    }
    else if (str_cmpx(name, "if-not-navigating") == 0) {

        retval = append_if_not_navigating;
    }
    else if (str_cmpx(name, "step-number") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_step_number;
        else if (is_txt_instruction(type))
            retval = text_append_step_number;
    }
    else if (str_cmpx(name, "total-steps") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_total_steps;
        else if (is_txt_instruction(type))
            retval = text_append_total_steps;
    }
    else if (str_cmpx(name, "ahead-dist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_ahead_dist;
        else if (is_txt_instruction(type))
            retval = text_append_ahead_dist;
    }
    else if (str_cmpx(name, "tripremaindist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_trdist;
        else if (is_txt_instruction(type))
            retval = text_append_trdist;
    }
    else if (str_cmpx(name, "tripremaintime") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_trtime;
    }
    else if (str_cmpx(name, "if-ferry-on-route") == 0) {
        retval = append_if_ferry_on_route;
    }
    else if (str_cmpx(name, "ferriesremaining") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_ferriesremaining;
    }
    else if (str_cmpx(name, "stack-dist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_stackdist;
        else if (is_txt_instruction(type))
            retval = text_append_stackdist;
    }
    else if (str_cmpx(name, "trafficincidentdist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_trafficincidentdist;
    }
    else if (str_cmpx(name, "trafficcongestiondist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_trafficcongestiondist;
    }
    else if (str_cmpx(name, "if-new-incidents") == 0) {

        retval = append_if_new_incidents;
    }
    else if (str_cmpx(name, "if-cleared-incidents") == 0) {

        retval = append_if_cleared_incidents;
    }
    else if (str_cmpx(name, "if-travel-time-delta") == 0) {

        retval = append_if_travel_time_delta;
    }
    else if (str_cmpx(name, "if-travel-time-delta-pos") == 0) {

        retval = append_if_travel_time_delta_pos;
    }
    else if (str_cmpx(name, "if-travel-time-delta-neg") == 0) {

        retval = append_if_travel_time_delta_neg;
    }
    else if (str_cmpx(name, "if-traffic") == 0) {

        retval = append_if_traffic;
    }
    else if (str_cmpx(name, "if-traffic-delay") == 0) {

        retval = append_if_traffic_delay;
    }
    else if (str_cmpx(name, "if-traffic-delay-threshold") == 0) {

        retval = append_if_traffic_delay_threshold;
    }
    else if (str_cmpx(name, "if-plural") == 0) {

        retval = append_if_plural;
    }
    else if (str_cmpx(name, "if-not-plural") == 0) {

        retval = append_if_not_plural;
    }
    else if (str_cmpx(name, "if-last-maneuver-beforeDT") == 0) {
        retval = append_if_last_maneuver_beforeDT;
    }
    else if (str_cmpx(name, "if-fmrpronun-available") == 0) {
        retval = append_if_fmrpronun_available;
    }
    else if (str_cmpx(name, "new-incident-count") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_new_incident_count;
    }
    else if (str_cmpx(name, "cleared-incident-count") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_cleared_incident_count;
    }
    else if (str_cmpx(name, "travel-time-delta") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_travel_time_delta;
    }
    else if (str_cmpx(name, "travel-time") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_travel_time;
    }
    else if (str_cmpx(name, "traffic-delay") == 0) {

        if (is_txt_instruction(type))
            retval = text_append_traffic_delay;
        else if (is_audio_instruction(type))
            retval = audio_append_traffic_delay;
    }
    else if (str_cmpx(name, "if-ti-road-pronun") == 0) {

        retval = append_if_ti_road_pronun;
    }
    else if (str_cmpx(name, "ti-road-pronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_ti_road_pronun;
    }
    else if (str_cmpx(name, "if-ti-origin-pronun") == 0) {

        retval = append_if_ti_origin_pronun;
    }
    else if (str_cmpx(name, "ti-origin-proximity") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_ti_origin_proximity;
    }
    else if (str_cmpx(name, "ti-origin-pronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_ti_origin_pronun;
    }
    else if (str_cmpx(name, "if-tc-road-pronun") == 0) {

        retval = append_if_tc_road_pronun;
    }
    else if (str_cmpx(name, "tc-road-pronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_tc_road_pronun;
    }
    else if (str_cmpx(name, "if-tc-origin-pronun") == 0) {

        retval = append_if_tc_origin_pronun;
    }
    else if (str_cmpx(name, "tc-origin-proximity") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_tc_origin_proximity;
    }
    else if (str_cmpx(name, "tc-origin-pronun") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_tc_origin_pronun;
    }
    else if (str_cmpx(name, "expect-traffic") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_expect_traffic;
    }
    else if (str_cmpx(name, "if-in-congestion") == 0) {

        retval = append_if_in_congestion;
    }
    else if (str_cmpx(name, "if-not-in-congestion") == 0) {

        retval = append_if_not_in_congestion;
    }
    else if (str_cmpx(name, "trafficcongestionlen") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_trafficongestionlen;
    }
    else if (str_cmpx(name, "if-show-speed-camera") == 0) {
        retval = append_if_show_speed_camera;
    }
    else if (str_cmpx(name, "speedcameraremaindist") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_cameradist;
        else if (is_txt_instruction(type))
            retval = text_append_cameradist;
    }
    else if (str_cmpx(name, "if-pedestrian") == 0) {

        retval = append_if_pedestrian;
    }
    else if (str_cmpx(name, "if-not-pedestrian") == 0) {

        retval = append_if_not_pedestrian;
    }
    else if (str_cmpx(name, "if-iname") == 0) {

        retval = append_if_iname;
    }
    else if (str_cmpx(name, "iname") == 0) {

        retval = text_append_iname;
    }
    else if (str_cmpx(name, "next-significant") == 0) {

        retval = append_next_significant;
    }
    else if (str_cmpx(name, "next-significant-dist") == 0) {

        if (is_audio_instruction(type))
            retval = audio_append_next_significant_dist;
        else if (is_txt_instruction(type))
            retval = text_append_next_significant_dist;
    }
    else if (str_cmpx(name, "if-lane-guidance") == 0) {

        retval = append_if_lane_guidance;
    }
    else if (str_cmpx(name, "if-not-lane-guidance") == 0) {

        retval = append_if_not_lane_guidance;
    }
    else if (str_cmpx(name, "if-natural-guidance") == 0) {

        retval = append_if_natural_guidance;
    }
    else if (str_cmpx(name, "if-not-natural-guidance") == 0) {

        retval = append_if_not_natural_guidance;
    }
    else if (str_cmpx(name, "lgpronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_lgpronun;
        else if (is_txt_instruction(type))
            retval = text_append_lgpronun;
    }
    else if (str_cmpx(name, "lgpreppronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_lgpreppronun;
        else if (is_txt_instruction(type))
            retval = text_append_lgprepronun;
    }
    else if (str_cmpx(name, "pgppronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_pgppronun;
        else if (is_txt_instruction(type))
            retval = text_append_pgppronun;
    }
    else if (str_cmpx(name, "pgppreppronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_pgppreppronun;
    }
    else if (str_cmpx(name, "tgppronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_tgppronun;
        else if (is_txt_instruction(type))
            retval = text_append_tgppronun;
    }
    else if (str_cmpx(name, "tgppreppronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_tgppreppronun;
    }
    else if (str_cmpx(name, "if-gps-poor") == 0) {

        retval = append_if_gps_poor;
    }
    else if (str_cmpx(name, "if-not-gps-poor") == 0) {

        retval = append_if_not_gps_poor;
    }
    else if (str_cmpx(name, "if-hwy-exit") == 0) {

        retval = append_if_hwy_exit;
    }
    else if (str_cmpx(name, "if-not-hwy-exit") == 0) {
        retval = append_if_not_hwy_exit;
    }
    else if (str_cmpx(name, "hwyexitpronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_hwyexitpronun;
        else if (is_txt_instruction(type))
            retval = text_append_hwyexitpronun;
    }
    else if (str_cmpx(name, "rdistpronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_rdistpronun;
        else if (is_txt_instruction(type))
            retval = text_append_rdistpronun;
    }
    else if (str_cmpx(name, "if-dpronun-available") == 0) {
        retval = append_if_dpronun_available;
    }
    else if (str_cmpx(name, "if-not-dpronun-available") == 0) {
        retval = append_if_not_dpronun_available;
    }
    else if (str_cmpx(name, "if-prepare-played") == 0) {
        retval = append_if_prepare_played;
    }
    else if (str_cmpx(name, "if-not-prepare-played") == 0) {
        retval = append_if_not_prepare_played;
    }
    else if (str_cmpx(name, "if-guidance-point-type") == 0) {
        retval = append_if_guidance_point_type;
    }
    else if (str_cmpx(name, "if-condition") == 0) {
        retval = append_if_condition;
    }
    else if (str_cmpx(name, "if-stacked-sound") == 0) {
        retval = append_if_stacked_sound;
    }
    else if (str_cmpx(name, "if-not-stacked-sound") == 0) {
        retval = append_if_not_stacked_sound;
    }
    else if (str_cmpx(name, "if-continue-played") == 0) {
        retval = append_if_continue_played;
    }
    else if (str_cmpx(name, "if-not-continue-played") == 0) {
        retval = append_if_not_continue_played;
    }
    else if (str_cmpx(name, "if-no-guidance") == 0) {
        retval = append_if_no_guidance;
    }
    else if (str_cmpx(name, "first-maneuver-heading") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_fmaneuver_heading;
    }
    else if (str_cmpx(name, "first-maneuver-cross-street") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_fmaneuver_crossstreet;
    }
    else if (str_cmpx(name, "first-maneuver-opronun") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_fmaneuver_opronun;
    }
    else if (str_cmpx(name, "if-first-maneuver-heading") == 0) {
        retval = append_if_fmaneuver_heading;
    }
    else if (str_cmpx(name, "if-not-first-maneuver-heading") == 0) {
        retval = append_if_not_fmaneuver_heading;
    }
    else if (str_cmpx(name, "if-first-maneuver-cross-street") == 0) {
        retval = append_if_fmaneuver_crossstreet;
    }
    else if (str_cmpx(name, "if-not-first-maneuver-cross-street") == 0) {
        retval = append_if_not_fmaneuver_crossstreet;
    }
    else if (str_cmpx(name, "if-opronun-available") == 0) {
        retval = append_if_opronun_available;
    }
    else if (str_cmpx(name, "if-not-opronun-available") == 0) {
        retval = append_if_not_opronun_available;
    }
    else if (str_cmpx(name, "heading-to-route-start") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_routestart_heading;
    }
    else if (str_cmpx(name, "direction-to-route") == 0) {
        if (is_audio_instruction(type))
            retval = audio_append_direction_to_route;
    }
    else if (str_cmpx(name, "if-direction-to-route") == 0) {
        retval = append_if_direction_to_route;
    }
    else if (str_cmpx(name, "if-no-direction-to-route") == 0) {
        retval = append_if_no_direction_to_route;
    }
    else if (str_cmpx(name, "fmrpronun") == 0)
    {
        retval = audio_append_fmrpronun;
    }
/*

 The following tags are not handed while appending children:
 [multiline, turn-sounds, turn-sounds, transition-sounds]
 [define-turn-sound, define-transition-sound]

 - turn-sound: []
 - turn-sound: [pos]
 - transition-sound: []
 - transition-sound: [pos]
 - stack-transition-sound: []
 - stack-transition-sound: [pos]
*/
    return retval;
}

NB_Error
append_if(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
          NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error        err         = NE_OK;
    const char*        turn_code   = NULL;
    nb_size            turn_size   = 0;
    boolean         turn_free   = FALSE;
    const char*        stack_code  = NULL;
    nb_size            stack_size  = 0;
    boolean         stack_free  = FALSE;
    const char*        next_code   = NULL;
    nb_size            next_size   = 0;
    boolean         next_free   = FALSE;

    if (!is->dataSource(tcode, nman, is->dsuser, &turn_code, &turn_size, &turn_free))
    {
        turn_size = 0;
        turn_free = FALSE;
    }

    if (!is->dataSource(scode, nman, is->dsuser, &stack_code, &stack_size, &stack_free))
    {
        stack_size = 0;
        stack_free = FALSE;
    }

    if (!is->dataSource(ncode, nman, is->dsuser, &next_code, &next_size, &next_free))
    {
        next_size = 0;
        next_free = FALSE;
    }
#ifdef ENABLE_LOGGING
    debugf("Find 'if' for: turn - %s  stack_code - %s  next_code - %s", turn_code, stack_code, next_code);
#endif
    if (str_match_simple_pattern(te_getattrc(te, "turn"), turn_code) &&
        str_match_simple_pattern(te_getattrc(te, "stacknext"), stack_code) &&
        str_match_simple_pattern(te_getattrc(te, "next"), next_code)) {

#ifdef ENABLE_LOGGING
      debugf("'if' is FOUND: turn - %s  stack_code - %s  next_code - %s", te_getattrc(te, "turn"), te_getattrc(te, "stacknext"), te_getattrc(te, "next"));
#endif
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    if (turn_free && turn_code != NULL)
        nsl_free((void*)turn_code);

    if (stack_free && stack_code != NULL)
        nsl_free((void*)stack_code);

    if (next_free && next_code != NULL)
        nsl_free((void*)next_code);

    return err;
}

NB_Error
append_if_start_name(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;

    if (getplace(is, nman, start, &is->temp_place) && nsl_strlen(is->temp_place.name) > 0)
    {
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return err;
}

NB_Error
append_if_start_phonenum(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;

    if (getplace(is, nman, start, &is->temp_place) && PlaceHasPhoneNumber(&is->temp_place))
    {
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return err;
}

NB_Error
append_if_destination_name(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;

    if (getplace(is, nman, destination, &is->temp_place) && nsl_strlen(is->temp_place.name) > 0
        && nsl_strcmp(is->temp_place.name, is->temp_place.location.areaname) != 0)
    {
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return err;
}

NB_Error
append_if_destination_phonenum(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                               NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;

    if (getplace(is, nman, destination, &is->temp_place) && PlaceHasPhoneNumber(&is->temp_place))
    {
        err = append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return err;
}

NB_Error
append_if_oname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, oname, 2);
}

NB_Error
append_if_dname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, dname, 2);
}

NB_Error
append_if_dname_base(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, dname_base, 2);
}

NB_Error
append_if_aname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, aname, 2);
}

NB_Error
append_if_aname_base(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, aname_base, 2);
}

NB_Error
append_next(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 nextManeuverIndex = 0;
    if (get_uint32(is, nman, nextmaneuverindex, &nextManeuverIndex))
    {
        is->handleNext = TRUE;
        return append_children(is, te, type, instruct_state, nextManeuverIndex, units, pos);
    }
    return NE_OK;
}

NB_Error
append_next_significant(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    nman = get_next_significant(is, nman);

    if (nman == NAV_MANEUVER_NONE)
    {
        return NE_OK;
    }

    return append_children(is, te, type, instruct_state, nman, units, pos);
}

NB_Error
append_if_equal(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    if (equal(is, te, nman))
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_equal(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    if (!equal(is, te, nman))
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_stack(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 value = 0;
    if (get_uint32(is, nman, removestack, &value) && !value)
    {
        return append_if_data(is, te, type, instruct_state, nman, units, pos, scode, 2);
    }
    return NE_OK;
}

NB_Error
append_if_not_stack(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_not_data(is, te, type, instruct_state, nman, units, pos, scode, 2);
}

NB_Error
append_turn_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_turn_sound_impl(is, te, type, instruct_state, nman, units,
                                  pos, tcode, getturnsound);
}

NB_Error
append_stack_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error    err            = NE_OK;
    tpselt      section_elt    = NULL;
    tpselt      instruct_elt   = NULL;
    tpselt      ter            = NULL;
    int         iter           = 0;
    const char* turn_code      = NULL;
    nb_size     turn_size      = 0;
    boolean     turn_free      = FALSE;
    const char* pos_pat        = NULL;
    const char* pos_val        = NULL;

    if (is->handleStack == TRUE)
    {
        return NE_OK;
    }
    if (is->dataSource == NULL || (type != it_audio && type != it_audio_lookahead))
    {
        return NE_INVAL;
    }
    is->handleStack = TRUE;

    // add messages codes for next maneuver

    // Get stack instruction code
    if (!is->dataSource(nman == NAV_MANEUVER_INITIAL ? tcode : scode,
                        nman == NAV_MANEUVER_INITIAL ? 0 : nman, is->dsuser, &turn_code, &turn_size, &turn_free))
    {
        turn_size = 0;
        turn_free = FALSE;
    }
    else
    {
        switch ((NB_NavigateAnnouncementType)is->amode)
        {
            case NB_NAT_Tone:
                section_elt = find_section(is, "tone");
                break;
            case NB_NAT_Voice:
                section_elt = find_section(is, "short");
                break;
            case NB_NAT_Street:
                section_elt = find_section(is, "long");
                break;
            default:
                break;
        }

        iter = 0;
        instruct_elt = NULL;

        while (section_elt != NULL && (ter = te_nextchild(section_elt, &iter)) != NULL)
        {
            if (str_cmpx("instruction", te_getname(ter)) == 0)
            {
                pos_pat = te_getattrc(ter, "pos");
                pos_val = nav_pos_to_string(NB_NMP_Prepare);
#ifdef ENABLE_LOGGING
            // comment this out if do not want verbose logs
        debugf("current 'instruction' has turn - %s  pos - %s", turn_code, pos_pat);
#endif
                // if a "pos" is not specified, match all
                if (pos_pat != NULL)
                {
                    if (pos_val != NULL)
                    {
                        if (!str_match_simple_pattern(pos_pat, pos_val))
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }

                if (str_match_simple_pattern(te_getattrc(ter, "turn"), turn_code) != 0)
                {
#ifdef ENABLE_LOGGING
                debugf("'instruction' is FOUND: turn - %s  pos - %s", turn_code, pos_pat);
#endif
                    instruct_elt = ter;
                    break;
                }
            }
        }

        if (instruct_elt != NULL)
        {
            uint32 stackManeuverIndex = 0;

            if (nman != NAV_MANEUVER_INITIAL && !get_uint32(is, nman, stackmaneuverindex, &stackManeuverIndex))
            {
                return NE_NOENT;
            }

            // This function fills the instruction code array with all the audio codes to play.
            err = err ? err : append_children(is, instruct_elt, type, (void*)instruct_state, stackManeuverIndex, units, NB_NMP_Prepare);
            //@todo (BUG 56050) bypass DT.U for voice
            if ( nsl_strcmp(turn_code, "DT.U") == 0 && err == NE_NOENT)
            {
                err = NE_OK;
            }
        }
        else
        {
    #ifdef DEBUG_FAILED_INSTRUCTIONS
            DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_INFO, ("failed to find instruction in %s, pos: %s, tc: %s\n", is->name, nav_pos_to_string(pos), turn_code ? turn_code : "<null>"));
    #endif
            err = NE_NOENT;
        }
        if (turn_free)
        {
           nsl_free((void*)turn_code);
        }
    }

    return err;
}

NB_Error
append_transition_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_turn_sound_impl(is, te, type, instruct_state, nman, units, pos,
        tcode, gettransitionsound);
}

NB_Error
append_stack_transition_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                              NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_turn_sound_impl(is, te, type, instruct_state, nman, units, pos,
        scode, gettransitionsound);
}

NB_Error
append_if_toward_name(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_toward;

    if (get_uint32(is, nman, toward, &is_toward) && is_toward)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_not_toward_name(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                          NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_toward;

    if (!get_uint32(is, nman, toward, &is_toward) || !is_toward)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_unnamed(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_unnamed;

    if (get_uint32(is, nman, unnamed, &is_unnamed) && is_unnamed)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_not_unnamed(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_unnamed;

    if (!get_uint32(is, nman, unnamed, &is_unnamed) || !is_unnamed)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_next_named(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 nman_nextnamed = get_next_named(is, nman);

    if (nman_nextnamed != NAV_MANEUVER_NONE)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_not_next_named(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 nman_nextnamed = get_next_named(is, nman);

    if (nman_nextnamed == NAV_MANEUVER_NONE)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_next_named(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 nman_nextnamed = get_next_named(is, nman);

    if (nman_nextnamed != NAV_MANEUVER_NONE)
    {
        return append_children(is, te, type, instruct_state, nman_nextnamed, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_next_named_dist_lt_max(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    double d;
    double md;

    if (get_double(is, nman, maxdist, &md) &&
        get_next_named_dist(is, nman, &d) &&
        (d <= md))
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_lookahead(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    if (is_lookahead_instruction(type))
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_lookahead(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    if (!is_lookahead_instruction(type))
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_navigating(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 nav;

    if (get_uint32(is, nman, navigating, &nav) && nav)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_navigating(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 nav;

    if (!get_uint32(is, nman, navigating, &nav) || !nav)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_ferry_on_route(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 ferry_on_route = 0;

    if (get_uint32(is, NAV_MANEUVER_NONE, ferryonroute, &ferry_on_route) && ferry_on_route)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_new_incidents(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 ni;

    if (get_uint32(is, nman, new_incident_count, &ni) && ni > 0)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_cleared_incidents(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 ci;

    if (get_uint32(is, nman, cleared_incident_count, &ci) && ci > 0)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

#define TRAVEL_TIME_DELTA_THRESHOLD 60

NB_Error
append_if_travel_time_delta(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    int32 ttd;

    if (get_int32(is, nman, travel_time_delta, &ttd) &&
        (ttd > TRAVEL_TIME_DELTA_THRESHOLD || ttd < -TRAVEL_TIME_DELTA_THRESHOLD))
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_travel_time_delta_pos(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    int32 ttd;

    if (get_int32(is, nman, travel_time_delta, &ttd) && ttd > 0)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_travel_time_delta_neg(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    int32 ttd;

    if (get_int32(is, nman, travel_time_delta, &ttd) && ttd < 0)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_traffic(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    int32 traffic;

    if (get_int32(is, nman, traffic_enabled, &traffic) && traffic != 0)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_traffic_delay(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 traffic;

    if (get_uint32(is, nman, traffic_delay, &traffic) && traffic > 0)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_traffic_delay_threshold(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 announce_delay_val;

    if (get_uint32(is, nman, announce_delay, &announce_delay_val) &&
        announce_delay_val)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_plural(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    ds_elem elem;
    uint32 val;

    elem = ds_elem_from_string(te_getattrc(te, "field"));

    if (get_uint32(is, nman, elem, &val) && val > 1)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_plural(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    ds_elem elem;
    uint32 val;

    elem = ds_elem_from_string(te_getattrc(te, "field"));

    if (get_uint32(is, nman, elem, &val) && val == 1)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_ti_road_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, ti_road_pronun, 2);
}

NB_Error
append_if_ti_origin_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, ti_origin_pronun, 2);
}

NB_Error
append_if_tc_road_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, tc_road_pronun, 2);
}

NB_Error
append_if_tc_origin_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_data(is, te, type, instruct_state, nman, units, pos, ti_origin_pronun, 2);
}

NB_Error
append_if_in_congestion(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 val;

    if (get_uint32(is, nman, in_congestion, &val) && val)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_in_congestion(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 val;

    if (get_uint32(is, nman, in_congestion, &val) && !val)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_pedestrian(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 val;
    NB_RouteSettings tripsettings;

    if (get_tripsettings(is, &tripsettings))
    {
        if ((tripsettings.vehicle_type == NB_TransportationMode_Pedestrian) &&
                get_uint32(is, nman, pedestrian, &val))
            return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_pedestrian(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 val;
    NB_RouteSettings tripsettings;

    if (get_tripsettings(is, &tripsettings))
    {
        if ((tripsettings.vehicle_type != NB_TransportationMode_Pedestrian) &&
                get_uint32(is, nman, pedestrian, &val))
            return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}


NB_Error
append_if_show_speed_camera(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 showcamera = 0;

    if (get_uint32(is, NAV_MANEUVER_NONE, showspeedcamera, &showcamera) && showcamera)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;

}

NB_Error
append_if_iname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_iname;

    if (get_uint32(is, nman, iname, &is_iname) && is_iname)
    {
    }

    return NE_OK;
}

NB_Error
append_if_lane_guidance(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 has_laneguidance = 0;
    uint32 has_naturalguidance = 0;

    if (get_uint32(is, nman, laneguidance, &has_laneguidance) &&
        get_uint32(is, nman, naturalguidance, &has_naturalguidance) &&
        has_laneguidance && !has_naturalguidance)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_lane_guidance(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 has_laneguidance = 0;

    if (get_uint32(is, nman, laneguidance, &has_laneguidance) && !has_laneguidance)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_natural_guidance(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 has_naturalguidance = 0;

    if (get_uint32(is, nman, naturalguidance, &has_naturalguidance) && has_naturalguidance)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_natural_guidance(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 has_naturalguidance = 0;

    if (get_uint32(is, nman, naturalguidance, &has_naturalguidance) && !has_naturalguidance)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}


NB_Error
append_if_gps_poor(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_gpspoor = 0;

    if (get_uint32(is, nman, gpspoor, &is_gpspoor) && is_gpspoor)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_gps_poor(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_gpspoor = 0;

    if (!is->handleStack && get_uint32(is, nman, gpspoor, &is_gpspoor) && !is_gpspoor)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_hwy_exit(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 has_hwyexit = 0;

    if (get_uint32(is, nman, hwyexit, &has_hwyexit) && has_hwyexit)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_hwy_exit(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 has_hwyexit = 0;

    if (get_uint32(is, nman, hwyexit, &has_hwyexit) && !has_hwyexit)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_dpronun_available(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 dpronun_available = 0;

    if (get_uint32(is, nman,
                   (is->handleNext || is->handleStack) ? stackdpronunavailable : dpronunavailable,
                   &dpronun_available) && dpronun_available)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_dpronun_available(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 dpronun_available = 0;

    if (get_uint32(is, nman,
                   (is->handleNext || is->handleStack) ? stackdpronunavailable : dpronunavailable,
                   &dpronun_available) && !dpronun_available)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_prepare_played(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 prepare_played = 0;

    if (get_uint32(is, nman, prepareplayed, &prepare_played) && prepare_played)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_prepare_played(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 prepare_played = 0;

    if (get_uint32(is, nman, prepareplayed, &prepare_played) && !prepare_played)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_continue_played(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 continue_played = 0;

    if (get_uint32(is, nman, continueplayed, &continue_played) && continue_played)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_continue_played(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                             NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 continue_played = 0;

    if (get_uint32(is, nman, continueplayed, &continue_played) && !continue_played)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_guidance_point_type(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 guidancePointType = 0;
    uint32 guidancePointTypeInManeuver = 0;
    const char* guidancePointString = NULL;
    nb_boolean negativeCondition = FALSE;

    guidancePointString = te_getattrc(te, "type");

    if (!guidancePointString || nsl_strlen(guidancePointString) == 0)
    {
        return NE_INVAL;
    }

    /*"type" can be, for example, a positive: 1, 5 and so on, or negative: !1, !5..
       so, we should define form of comparison be checking first char on '!'*/

    if (guidancePointString[0] == '!')
    {
        negativeCondition = TRUE;
        guidancePointType = nsl_atoi(guidancePointString + 1);
    }
    else
    {
        guidancePointType = nsl_atoi(guidancePointString);
    }

    if (get_uint32(is, nman, guidancepointtype, &guidancePointTypeInManeuver))
    {
        if ( (negativeCondition && guidancePointTypeInManeuver != guidancePointType) ||
            (!negativeCondition && guidancePointTypeInManeuver == guidancePointType))
        {
            return append_children(is, te, type, instruct_state, nman, units, pos);
        }
    }

    return NE_OK;
}

NB_Error
append_if_condition(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    nb_boolean condition = FALSE;
    NB_Error   err       = NE_OK;
    tpselt     tex       = NULL;
    int        iter      = 0;

    if (is->dataSource == NULL)
    {
        return NE_INVAL;
    }

    while ((tex = te_nextchild(te, &iter)) != NULL && err == NE_OK)
    {
#ifdef ENABLE_LOGGING
        debugf("if condition is '%s'", te_getname(tex));
#endif
        if (str_cmpx(te_getname(tex), "expression") == 0)
        {
            condition = evaluate_expression(is, tex, type, instruct_state, nman, units, pos, CONDITION_NONE);
        }
        else if (str_cmpx(te_getname(tex), "then") == 0)
        {
            if (condition)
            {
                append_children(is, tex, type, instruct_state, nman, units, pos);
            }
        }
        else if (str_cmpx(te_getname(tex), "else") == 0)
        {
            if (!condition)
            {
                append_children(is, tex, type, instruct_state, nman, units, pos);
            }
        }
    }

    return NE_OK;
}

NB_Error
append_if_stacked_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    if (is->handleStack)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_not_stacked_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    if (!is->handleStack)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }

    return NE_OK;
}

NB_Error
append_if_no_guidance(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_noguidance = 0;

    if (get_uint32(is, nman, noguidance, &is_noguidance) && is_noguidance)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_fmaneuver_heading(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_highway = 0;

    if (get_uint32(is, 0, highway, &is_highway) && !is_highway)
    {
        return append_children(is, te, type, instruct_state, 0, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_not_fmaneuver_heading(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_highway = 0;

    if (get_uint32(is, 0, highway, &is_highway) && is_highway)
    {
        return append_children(is, te, type, instruct_state, 0, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_fmaneuver_crossstreet(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_highway = 0;

    if (get_uint32(is, 0, highway, &is_highway) && !is_highway)
    {
        return append_children(is, te, type, instruct_state, 0, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_not_fmaneuver_crossstreet(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    uint32 is_highway = 0;

    if (get_uint32(is, 0, highway, &is_highway) && is_highway)
    {
        return append_children(is, te, type, instruct_state, 0, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_opronun_available(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_dpronun_available(is, te, type, instruct_state, 0, units, pos);
}

NB_Error
append_if_not_opronun_available(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    return append_if_not_dpronun_available(is, te, type, instruct_state, 0, units, pos);
}

NB_Error
append_if_direction_to_route(instructset* is, tpselt te, instructtype type, void* instruct_state,
                             uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    uint32 ifDirectionToRoute = 0;

    if (get_uint32(is, 0, ifdirectiontoroute, &ifDirectionToRoute) && ifDirectionToRoute)
    {
        return append_children(is, te, type, instruct_state, 0, units, pos);
    }
    return NE_OK;
}

NB_Error
append_if_no_direction_to_route(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    uint32 ifDirectionToRoute = 0;

    if (get_uint32(is, 0, ifdirectiontoroute, &ifDirectionToRoute) && !ifDirectionToRoute)
    {
        return append_children(is, te, type, instruct_state, 0, units, pos);
    }
    return NE_OK;
}

NB_Error append_if_last_maneuver_beforeDT(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    const char* turn_code = NULL;
    nb_size turn_size = 0;
    boolean turn_free = FALSE;
    uint32 nextManeuverIndex = 0;
    if (get_uint32(is, nman, nextmaneuverindex, &nextManeuverIndex))
    {
        if (!is->dataSource(tcode, nextManeuverIndex, is->dsuser, &turn_code, &turn_size, &turn_free))
        {
            turn_code = NULL;
        }
        
        if (str_cmpx(turn_code, "DT.") == 0)
        {
            return append_children(is, te, type, instruct_state, nman, units, pos);
        }
    }
    return NE_OK;
}

NB_Error append_if_fmrpronun_available     (instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    const char*       data        = NULL;
    nb_size           datalen     = 0;
    boolean           freedata    = FALSE;
    if (is->dataSource(fmrpronun, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        return append_children(is, te, type, instruct_state, nman, units, pos);
    }
    
    return NE_NOENT;
}

NB_Error
audio_append_sound(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    char*    id = NULL;
    size_t    idlen = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (te_getattr(te, "id", &id, &idlen))
    {
        AddBaseNameToInstructionArray(is, codes, id, (nb_size)idlen);
    }

    return err;
}

NB_Error
audio_append_opronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error error = NE_OK;
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "opronun");

    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    error = AddCacheIDtoInstructionArray(is, codes, opronun, nman);
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
    return error;
}

NB_Error
audio_append_dpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error error = NE_OK;
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "dpronun");
    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    error = AddCacheIDtoInstructionArray(is, codes, dpronun, nman);
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
    return error;
}

NB_Error
audio_append_dpronun_base(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                          NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error error = NE_OK;
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "dpronun-base");
    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    error = AddCacheIDtoInstructionArray(is, codes, dpronun_base, nman);
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
    return error;
}

NB_Error
audio_append_ipronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error error = NE_OK;
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    NB_QaLogAudioPronun(is->context, "ipronun");

    error = AddCacheIDtoInstructionArray(is, codes, ipronun, nman);
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
    return error;
}

NB_Error
audio_append_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (getdistance(is, nman, &distance))
    {
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_cameradist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance=0;
    double curspeed=0;
    //double time;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, NAV_MANEUVER_CAMERA, speedcameraremaindist, &distance))
    {
        // Get the speed from Navigate
        if (get_double(is, nman, speed, &curspeed))
        {
            /// @todo (BUG 56054)
            // Get length of instruction from audio_instruct_state->qcp
            /*  this is BREW specific
            if (qcp_combine_gettime(audio_instruct_state, &time) == NE_OK)
            {
                // Maybe add 2 seconds to allow for distance
                // Interpolate distance as: distance = (distance-speed*qcp_time)
                distance = FSUB(distance,FMUL(curspeed,FADD(time,2.0)));
            }
            */
        }
        err = err ? err : audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
audio_append_tdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (getdistance(is, NAV_MANEUVER_NONE, &distance))
    {
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_next_named_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                             NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance = 0.0;
    double curspeed = 0.0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_next_named_dist(is, nman, &distance) && get_double(is, nman, speed, &curspeed))
    {
        err = audio_append_distance_with_adjust(is, codes, distance, TRUE, 20.0, TRUE, units, curspeed);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_step_number(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    uint32 val = 0;
    uint32 previous_insignificant_count = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    previous_insignificant_count = get_previous_insignificant_count(is, nman);

    if (NAV_MANEUVER_NONE == previous_insignificant_count)
    {
        return NAV_MANEUVER_NONE;
    }

    nman = nman - previous_insignificant_count;

    if (get_uint32(is, nman, step, &val))
    {
        err = audio_append_integer(is, codes, (int) val, NULL);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

 NB_Error
audio_append_total_steps(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    uint32 val = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_uint32(is, nman, totalsteps, &val))
    {
        /* Make sure the correct file is used for Swedish en/ett */
        if (val == (uint32)1)
        {
            AddBaseNameToInstructionArray(is, codes, VID_ONE_STEP, -1);
        }
        else
        {
            err = audio_append_integer(is, codes, (int) val, NULL);
        }

        err = audio_append_integer(is, codes, (int) val, NULL);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_ahead_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, nman, aheaddist, &distance))
    {
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_trdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, NAV_MANEUVER_NONE, tripremaindist, &distance))
    {
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_stackdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, NAV_MANEUVER_NONE, stackdist, &distance))
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    else
        err = NE_NOENT;

    return err;
}

NB_Error
audio_append_trafficincidentdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, NAV_MANEUVER_NONE, trafficincidentdist, &distance))
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    else
        err = NE_NOENT;

    return err;
}

NB_Error
audio_append_trafficcongestiondist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, NAV_MANEUVER_NONE, trafficcongestiondist, &distance))
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    else
        err = NE_NOENT;

    return err;
}

NB_Error
audio_append_new_incident_count(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    uint32 val = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_uint32(is, nman, new_incident_count, &val))
    {
        err = audio_append_integer(is, codes, (int) val, NULL);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_cleared_incident_count(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    uint32 val = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_uint32(is, nman, cleared_incident_count, &val))
    {
        err = audio_append_integer(is, codes, (int) val, NULL);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_travel_time_delta(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                               NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    int32 val = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_int32(is, nman, travel_time_delta, &val))
    {
        err = audio_append_time(is, codes, (uint32) (val > 0 ? val : -val), TRUE, TRUE);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_traffic_delay(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    uint32 val = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_uint32(is, NAV_MANEUVER_NONE, traffic_delay, &val))
    {
        err = audio_append_time(is, codes, val, TRUE, TRUE);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_ti_road_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error error = NE_OK;
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "ti-road-pronun");
    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    error = AddCacheIDtoInstructionArray(is, codes, ti_road_pronun, nman);
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
    return error;
}

NB_Error
audio_append_ti_origin_proximity(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    AddBaseNameToInstructionArray(is, codes, "at", 2);
    return NE_OK;
}

NB_Error
audio_append_ti_origin_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                              NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "ti-origin-pronun");

    return AddCacheIDtoInstructionArray(is, codes, ti_origin_pronun, nman);
}

NB_Error
audio_append_tc_road_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "tc-road-pronun");

    return AddCacheIDtoInstructionArray(is, codes, tc_road_pronun, nman);

}

NB_Error
audio_append_tc_origin_proximity(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    AddBaseNameToInstructionArray(is, codes, "at", 2);
    return NE_OK;
}

NB_Error
audio_append_tc_origin_pronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                              NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "tc-origin-pronun");

    return AddCacheIDtoInstructionArray(is, codes, tc_origin_pronun, nman);

}

NB_Error
audio_append_expect_traffic(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                            NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    uint32 val = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_uint32(is, NAV_MANEUVER_NONE, expect_traffic_type, &val))
    {
        switch ((nav_expect_traffic) val)
        {
            case expect_traffic_light: // light traffic
                AddBaseNameToInstructionArray(is, codes, "light-traffic", -1);
                break;
            case expect_traffic_moderate: // moderate traffic
                AddBaseNameToInstructionArray(is, codes, "moderate-traffic", -1);
                break;
            case expect_traffic_severe: // severe traffic
                AddBaseNameToInstructionArray(is, codes, "severe-traffic", -1);
                break;
            default:
                break;
        }
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_trafficongestionlen(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance = 0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_double(is, NAV_MANEUVER_NONE, trafficcongestionlen, &distance))
    {
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_next_significant_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance = 0.0;
    double curspeed = 0.0;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    if (get_next_significant_dist(is, nman, &distance) && get_double(is, nman, speed, &curspeed))
    {
        err = audio_append_distance_with_adjust(is, codes, distance, TRUE, 20.0, TRUE, units, curspeed);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_lgpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "lg-pronun");

    if (is->dataSource(lgpronun, nman, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_lgpreppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "lg-preppronun");

    if (is->dataSource(lgpreppronun, nman, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_pgppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "pgp-pronun");

    if (is->dataSource(pgppronun, nman, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_pgppreppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "pg-ppreppronun");

    if (is->dataSource(pgppreppronun, nman, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_tgppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "tgp-pronunn");

    if (is->dataSource(tgppronun, nman, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_tgppreppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;
    const char*    data        = NULL;
    nb_size        datalen     = 0;
    boolean        freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "tgp-preppronun");

    if (is->dataSource(tgppreppronun, nman, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_hwyexitpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "hwyexit-pronun");

    return AddCacheIDtoInstructionArray(is, codes, hwyexitpronun, nman);
}

NB_Error
audio_append_rdistpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                         NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    double distance;

    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes = instruct_state;

    // Use the previou maneuver distance if it was (RE.).
    // Otherwise it will use the distance of (RX.*) to play for (RE.).
    if (nman > 1)
    {
        const char* turn_code = NULL;
        nb_size turn_size = 0;
        boolean turn_free = FALSE;
        uint32 prevMan = nman - 1;
        if (!is->dataSource(tcode, prevMan, is->dsuser, &turn_code, &turn_size, &turn_free))
        {
            turn_code = NULL;
        }

        if (str_cmpx(turn_code, "RE.") == 0)
        {
            nman = prevMan;
        }
    }

    if (getdistance(is, nman, &distance))
    {
        err = audio_append_distance(is, codes, distance, TRUE, 20.0, TRUE, units);
        is->soundTypes |= REAL_TIME_DATA_MASK;
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
audio_append_fmaneuver_heading(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                               NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes       = instruct_state;
    const char*       data        = NULL;
    nb_size           datalen     = 0;
    boolean           freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "first-maneuver-heading");

    if (is->dataSource(fmaneuverheading, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_fmaneuver_crossstreet(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes       = instruct_state;
    const char*       data        = NULL;
    nb_size           datalen     = 0;
    boolean           freedata    = FALSE;
    uint32            is_unnamed  = 0;

    NB_QaLogAudioPronun(is->context, "first-maneuver-cross-street");
    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    if (is->dataSource(fmaneuvercrossstreet, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
    }
    else
    {
        if (get_uint32(is, 0, unnamed, &is_unnamed))
        {
            if (is_unnamed)
            {
                if (is->dataSource(dpronun_base, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
                {
                    AddBaseNameToInstructionArray(is, codes, data, datalen);
                }
            }
            else
            {
                if (is->dataSource(dpronun, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
                {
                    AddBaseNameToInstructionArray(is, codes, data, datalen);
                }
            }
        }
        else
        {
            data    = DESTINATION_NAME;
            datalen = (nb_size)nsl_strlen(DESTINATION_NAME);
            AddBaseNameToInstructionArray(is, codes, data, datalen);
        }
    }
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));

    is->soundTypes |= REAL_TIME_DATA_MASK;
    return NE_OK;
}

NB_Error
audio_append_fmaneuver_opronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                               NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error error = NE_OK;
    InstructionCodes* codes = instruct_state;

    NB_QaLogAudioPronun(is->context, "first-maneuver-opronun");

    AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
    error = AddCacheIDtoInstructionArray(is, codes, opronun, 0);
    AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
    return error;
}

NB_Error
audio_append_routestart_heading(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes       = instruct_state;
    const char*       data        = NULL;
    nb_size           datalen     = 0;
    boolean           freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "heading-to-route-start");

    if (is->dataSource(routestartheading, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_direction_to_route(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes       = instruct_state;
    const char*       data        = NULL;
    nb_size           datalen     = 0;
    boolean           freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "direction-to-route");

    if (is->dataSource(directiontoroute, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        return NE_OK;
    }

    return NE_NOENT;
}

NB_Error
audio_append_fmrpronun(instructset* is, tpselt te, instructtype type, void* instruct_state,
                       uint32 nman, NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // The instruction state points to a InstructionCodes structure
    InstructionCodes* codes       = instruct_state;
    const char*       data        = NULL;
    nb_size           datalen     = 0;
    boolean           freedata    = FALSE;

    NB_QaLogAudioPronun(is->context, "first-major-road");

    if (is->dataSource(fmrpronun, 0, is->dsuser, &data, &datalen, &freedata) && data && datalen)
    {
        AddBaseNameToInstructionArray(is, codes, SPAN_ROAD_NAME, strlen(SPAN_ROAD_NAME));
        AddBaseNameToInstructionArray(is, codes, data, datalen);
        AddBaseNameToInstructionArray(is, codes, SPAN_CLOSE, strlen(SPAN_CLOSE));
        return NE_OK;
    }

    return NE_NOENT;
}

// Text append handlers
NB_Error
text_append_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_distance(is, &text_instruct_state->dbp, nman, units);
}

NB_Error
text_append_font(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    if (str_cmpx(te_getattrc(te, "name"), "normal") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Normal);
    else if (str_cmpx(te_getattrc(te, "name"), "bold") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Bold);
    else if (str_cmpx(te_getattrc(te, "name"), "large") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Large);
    else if (str_cmpx(te_getattrc(te, "name"), "large-bold") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Large_Bold);
    else if (str_cmpx(te_getattrc(te, "name"), "custom-1") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Custom1);
    else if (str_cmpx(te_getattrc(te, "name"), "custom-2") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Custom2);
    else if (str_cmpx(te_getattrc(te, "name"), "custom-3") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Custom3);
    else if (str_cmpx(te_getattrc(te, "name"), "custom-4") == 0)
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Custom4);
    else
        text_dynbuf_append_font(is, &text_instruct_state->dbp, NB_Font_Normal);

    return dbuferr(&text_instruct_state->dbp);
}

NB_Error
text_append_color(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    text_dynbuf_append_color(is, &text_instruct_state->dbp,
        MAKE_NB_COLOR(te_getattru(te, "red"), te_getattru(te, "green"), te_getattru(te, "blue")));

    return dbuferr(&text_instruct_state->dbp);
}

NB_Error
text_append_text(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;
    const char* val = NULL;

    val = te_getattrc(te, "data");

    if (val != NULL && nsl_strlen(val) > 0)
        text_dynbuf_append_text(is, &text_instruct_state->dbp, val);

    return dbuferr(&text_instruct_state->dbp);
}

NB_Error
text_append_cardinal(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, cardinal, nman);
}

NB_Error
text_append_start_name(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_name(is, &text_instruct_state->dbp, start, nman);
}

NB_Error
text_append_start_address(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                          NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_address(is, &text_instruct_state->dbp, start, nman,
                (boolean) (te_getchild(te, "multiline") != NULL));
}

NB_Error
text_append_start_phonenum(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                           NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_phonenum(is, &text_instruct_state->dbp, start, nman);

}

NB_Error
text_append_destination_name(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                             NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_name(is, &text_instruct_state->dbp, destination, nman);

}

NB_Error
text_append_destination_address(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_address(is, &text_instruct_state->dbp, destination, nman,
                (boolean) (te_getchild(te, "multiline") != NULL));
}

NB_Error
text_append_destination_phonenum(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                 NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_phonenum(is, &text_instruct_state->dbp, destination, nman);

}

NB_Error
text_append_oname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, oname, nman);
}

NB_Error
text_append_dname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, dname, nman);
}

NB_Error
text_append_dname_base(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, dname_base, nman);
}

NB_Error
text_append_aname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, aname, nman);
}

NB_Error
text_append_aname_base(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, aname_base, nman);
}

NB_Error
text_append_tdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_distance(is, &text_instruct_state->dbp, NAV_MANEUVER_NONE, units);
}

NB_Error
text_append_ttime(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    uint32 time;

    if (get_uint32(is, NAV_MANEUVER_NONE, triptime, &time)) {

        format_time_n(time, txt, sizeof(txt), TRUE, FALSE, FALSE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_tsettings(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_tsettings(is, &text_instruct_state->dbp);

}

NB_Error
text_append_newline(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                    NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    text_dynbuf_append_newline(is, &text_instruct_state->dbp);
    return dbuferr(&text_instruct_state->dbp);
}

NB_Error
text_append_step_number(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
    char    txt[64];
    uint32 val;
    uint32 previous_insignificant_count = 0;

    previous_insignificant_count = get_previous_insignificant_count(is, nman);

    if (NAV_MANEUVER_NONE == previous_insignificant_count)
    {
        return NAV_MANEUVER_NONE;
    }

    nman = nman - previous_insignificant_count;

    if (get_uint32(is, nman, step, &val))
    {
        nsl_sprintf(txt, "%u", val);
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
text_append_total_steps(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
    char    txt[64];
    uint32 val;

    if (get_uint32(is, nman, totalsteps, &val)) {

        nsl_sprintf(txt, "%u", val);
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_ahead_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    double distance;

    if (get_double(is, nman, aheaddist, &distance)) {

        format_distance_f(distance, txt, sizeof(txt), TRUE, 20.0, TRUE, units, TRUE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);

        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_trdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    double distance;

    if (get_double(is, NAV_MANEUVER_NONE, tripremaindist, &distance)) {

        format_distance_f(distance, txt, sizeof(txt), TRUE, 20.0, TRUE, units, TRUE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);

        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_cameradist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                       NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
    char    txt[512];
    double distance;

    if (get_double(is, NAV_MANEUVER_CAMERA, speedcameraremaindist, &distance)) {

        format_distance_ex_f(distance, txt, sizeof(txt), TRUE, 20.0, TRUE, units, TRUE, 500.0);
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);

        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;
    return err;
}

NB_Error
text_append_ferriesremaining(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                             NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
    uint32 ferry_on_route = 0;
    char    txt[16];

    if (get_uint32(is, NAV_MANEUVER_NONE, ferryonroute, &ferry_on_route) && ferry_on_route)
    {
        nsl_sprintf(txt, "%u", ferry_on_route);
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    return err;
}

NB_Error
text_append_iname(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    text_instruct_state* text_instruct_state = instruct_state;

    return text_dynbuf_append_data(is, &text_instruct_state->dbp, iname, nman);
}


NB_Error
text_append_trtime(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    uint32 time;

    if (get_uint32(is, NAV_MANEUVER_NONE, tripremaintime, &time)) {

        format_time_n(time, txt, sizeof(txt), TRUE, FALSE, FALSE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_stackdist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    double distance;

    if (get_double(is, NAV_MANEUVER_NONE, stackdist, &distance)) {

        format_distance_f(distance, txt, sizeof(txt), TRUE, 20.0, TRUE, units, TRUE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);

        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_travel_time(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    uint32 travel_time_val;

    if (get_uint32(is, nman, travel_time, &travel_time_val)) {

        format_time_n(travel_time_val, txt, sizeof(txt), TRUE, FALSE, FALSE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_traffic_delay(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                          NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
//    AECHAR    wtxt[512];
    char    txt[512];
    uint32 traffic_delay_val;

    if (get_uint32(is, nman, traffic_delay, &traffic_delay_val)) {

        format_time_n(traffic_delay_val, txt, sizeof(txt), TRUE, FALSE, FALSE);
//        WSTRTOSTR(wtxt, txt, sizeof(txt));
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);
        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_next_significant_dist(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                                  NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    NB_Error err = NE_OK;
    text_instruct_state* text_instruct_state = instruct_state;
    char    txt[512];
    double distance;

    if (get_next_significant_dist(is, nman, &distance)) {

        format_distance_f(distance, txt, sizeof(txt), TRUE, 20.0, TRUE, units, TRUE);
        text_dynbuf_append_text(is, &text_instruct_state->dbp, txt);

        err = dbuferr(&text_instruct_state->dbp);
    }
    else
        err = NE_NOENT;

    return err;
}

NB_Error
text_append_lgpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // @TODO:
    return NE_NOSUPPORT;
}

NB_Error
text_append_lgprepronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                     NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // @TODO:
    return NE_NOSUPPORT;
}

NB_Error
text_append_pgppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // @TODO:
    return NE_NOSUPPORT;
}

NB_Error
text_append_tgppronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                      NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // @TODO:
    return NE_NOSUPPORT;
}

NB_Error
text_append_hwyexitpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                          NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // @TODO:
    return NE_NOSUPPORT;
}

NB_Error
text_append_rdistpronun(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                        NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos)
{
    // @TODO:
    return NE_NOSUPPORT;
}


boolean
is_txt_instruction(instructtype type)
{
    return (boolean)(type == it_txt || type == it_txt_arrival || type == it_txt_lookahead || type == it_txt_stack ||
                     type == it_txt_sec || type == it_txt_arrival_sec || type == it_txt_stack_sec);
}

boolean
is_audio_instruction(instructtype type)
{
    return (boolean)(type == it_audio || type == it_audio_lookahead || type == it_audio_traffic || type == it_audio_speed_limit);
}
/*
boolean
is_image_instruction(instructtype type)
{
    return (boolean)(type == it_image || type == it_image_stack || type == it_image_vecmap || type == it_image_sec || type == it_image_sec_stack);
}
*/
boolean
is_lookahead_instruction(instructtype type)
{
    return (boolean)(type == it_txt_lookahead || type == it_audio_lookahead);
}

void
CopyInstructionName(char *dst, const char *src, nb_size len)
{
    // Create a null terminated string from at most 'len' source characters
    nsl_memset(dst, 0, len);
    nsl_memcpy(dst, src, len - 1);
}

nb_boolean evaluate_expression(instructset* is, tpselt te, instructtype type, void* instruct_state, uint32 nman,
                   NB_NavigateAnnouncementUnits units, NB_NavigateManeuverPos pos, uint8 condition_type)
{
    CSL_Vector* result_vec = NULL;
    nb_boolean  result     = FALSE;
    NB_Error    err        = NE_OK;
    tpselt      tex        = NULL;
    int         iter       = 0;

    if (is->dataSource == NULL)
    {
        return FALSE;
    }

    result_vec = CSL_VectorAlloc(sizeof(nb_boolean));

    if (!result_vec)
    {
        return FALSE;
    }

    while ((tex = te_nextchild(te, &iter)) != NULL && err == NE_OK)
    {
#ifdef ENABLE_LOGGING
        debugf("expression is '%s'", te_getname(tex));
#endif
        if (str_cmpx(te_getname(tex), "value") == 0)
        {
            result = get_value(is, nman, te_getattrc(tex, "key"), is_lookahead_instruction(type), pos);
            CSL_VectorAppend(result_vec, &result);
        }
        else if (str_cmpx(te_getname(tex), "not") == 0)
        {
            result = evaluate_expression(is, tex, type, instruct_state, nman, units, pos, CONDITION_NOT);
            CSL_VectorAppend(result_vec, &result);
        }
        else if (str_cmpx(te_getname(tex), "and") == 0)
        {
            result = evaluate_expression(is, tex, type, instruct_state, nman, units, pos, CONDITION_AND);
            CSL_VectorAppend(result_vec, &result);
        }
        else if (str_cmpx(te_getname(tex), "or") == 0)
        {
            result = evaluate_expression(is, tex, type, instruct_state, nman, units, pos, CONDITION_OR);
            CSL_VectorAppend(result_vec, &result);
        }
    }

    result = FALSE;

    switch(condition_type)
    {
        case CONDITION_NONE:
            if (CSL_VectorGetLength(result_vec) > 0)
            {
                result = *((nb_boolean*)CSL_VectorGetPointer(result_vec, 0));
            }
            break;
        case CONDITION_NOT:
            result = evaluate_not(result_vec);
            break;

        case CONDITION_AND:
            result = evaluate_and(result_vec);
            break;

        case CONDITION_OR:
            result = evaluate_or(result_vec);
            break;
    }

    CSL_VectorDealloc(result_vec);
    result_vec = NULL;

    return result;
}

nb_boolean evaluate_and(CSL_Vector* vec)
{
    nb_boolean result = FALSE;

    if (vec && CSL_VectorGetLength(vec) > 0)
    {
        int i = 0;
        int length = CSL_VectorGetLength(vec);

        for (i = 0; i < length; i++)
        {
            result = *((nb_boolean*)CSL_VectorGetPointer(vec, i));

            if (!result)
            {
                break;
            }
        }
    }

    return result;
}

nb_boolean evaluate_or(CSL_Vector* vec)
{
    nb_boolean result = FALSE;

    if (vec && CSL_VectorGetLength(vec) > 0)
    {
        int i = 0;
        int length = CSL_VectorGetLength(vec);

        for (i = 0; i < length; i++)
        {
            result = *((nb_boolean*)CSL_VectorGetPointer(vec, i));

            if (result)
            {
                break;
            }
        }
    }

    return result;
}

nb_boolean evaluate_not(CSL_Vector* vec)
{
    nb_boolean result = FALSE;

    if (vec && CSL_VectorGetLength(vec) > 0)
    {
        result = !(*((nb_boolean*)CSL_VectorGetPointer(vec, 0)));
    }

    return result;
}

nb_boolean get_value(instructset* is, uint32 nman, const char* name, nb_boolean is_lookahead, NB_NavigateManeuverPos pos)
{
    uint32 value = 0;
    nb_boolean retval = FALSE;

    if (!is || !name)
    {
        return FALSE;
    }
#ifdef ENABLE_LOGGING
        debugf("get_value for '%s'", name);
#endif
    if (str_cmpx(name, "lane-guidance") == 0)
    {
        retval = (get_uint32(is, nman, laneguidance, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "natural-guidance") == 0)
    {
        retval = (get_uint32(is, nman, naturalguidance, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "dpronun-available") == 0)
    {
        retval = (get_uint32(is, nman,
                             (is->handleNext || is->handleStack) ? stackdpronunavailable : dpronunavailable,
                             &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "prepare-played") == 0)
    {
        retval = (get_uint32(is, nman, prepareplayed, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "continue-played") == 0)
    {
        retval = (get_uint32(is, nman, continueplayed, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "gps-poor") == 0)
    {
        retval = (get_uint32(is, nman, gpspoor, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "lookahead") == 0)
    {
        retval = is_lookahead;
    }
    else if (str_cmpx(name, "hwy-exit") == 0)
    {
        retval = (get_uint32(is, nman, hwyexit, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "unnamed") == 0)
    {
        retval = (get_uint32(is, nman, unnamed, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "stack-advise") == 0)
    {
        uint32 value = 0;
        nb_size size = 0;

        if (get_uint32(is, nman, removestack, &value) && !value &&
            get_uint32(is, nman, stackadvise, &value) && value &&
            is->dataSource(scode, nman, is->dsuser, NULL, &size, NULL) && size >= 2)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "next-named") == 0)
    {
        uint32 nman_nextnamed = get_next_named(is, nman);
        if (nman_nextnamed != NAV_MANEUVER_NONE)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "toward-name") == 0)
    {
        retval = (get_uint32(is, nman, toward, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "traffic-delay-threshold") == 0)
    {
        retval = (get_uint32(is, nman, announce_delay, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "ti-road-pronun") == 0)
    {
        retval = (get_uint32(is, nman, ti_road_pronun, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "ti-origin-pronun") == 0)
    {
        retval = (get_uint32(is, nman, ti_origin_pronun, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "tc-road-pronun") == 0)
    {
        retval = (get_uint32(is, nman, tc_road_pronun, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "tc-origin-pronun") == 0)
    {
        retval = (get_uint32(is, nman, tc_origin_pronun, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "in-congestion") == 0)
    {
        retval = (get_uint32(is, nman, in_congestion, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "button-pressed") == 0)
    {
        retval = (get_uint32(is, nman, buttonpressed, &value) && value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "gp-traffic-light") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_TrafficLight)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "gp-stop-sign") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_StopSign)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "gp-end-of-the-road") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_EndOfTheRoad)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "gp-bridge") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_Bridge)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "gp-street-count") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_StreetCount)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "gp-train-tracks") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_TrainTracks)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "gp-natural-gp") == 0)
    {
        if (get_uint32(is, nman, guidancepointtype, &value) && value == NBGPT_NavteqNaturalGuidance)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "stacked-sound") == 0)
    {
        if (is->handleStack)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "position-continue") == 0)
    {
        if (pos == NB_NMP_Continue)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "position-prepare") == 0)
    {
        if (pos == NB_NMP_Prepare)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "position-turn") == 0)
    {
        if (pos == NB_NMP_Turn)
        {
            retval = TRUE;
        }
    }
    else if (str_cmpx(name, "first-maneuver-heading") == 0)
    {
        retval = (get_uint32(is, 0, highway, &value) && !value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "first-maneuver-cross-street") == 0)
    {
        const char* data     = NULL;
        nb_size     datalen  = 0;
        boolean     freedata = FALSE;
        retval = (get_uint32(is, 0, highway, &value) && !value &&
                  is->dataSource(fmaneuvercrossstreet, 0, is->dsuser, &data, &datalen, &freedata) &&
                  data && datalen) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "vehicle-type-pedestrian") == 0)
    {
        NB_RouteSettings tripsettings;
        if (get_tripsettings(is, &tripsettings))
        {
            retval = (tripsettings.vehicle_type == NB_TransportationMode_Pedestrian) ? TRUE : FALSE;
        }
    }
    else if (str_cmpx(name, "direction-to-route ") == 0)
    {
        retval = (get_uint32(is, 0, ifdirectiontoroute, &value) && !value) ? TRUE : FALSE;
    }
    else if (str_cmpx(name, "long-maneuver") == 0)
    {
        retval = (get_uint32(is, nman, longmaneuver, &value) && value) ? TRUE : FALSE;
    }

    return retval;
}
