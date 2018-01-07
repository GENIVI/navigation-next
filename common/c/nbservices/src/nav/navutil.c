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
 * navutil.c: created 2005/01/06 by Mark Goddard.
 */

#include "navutil.h"
#include "palmath.h"
#include "nbspatial.h"
#include "palfile.h"
#include "csltypes.h"
#include "transformutility.h"

// (-XXX.YYYYYY; -MMM.ZZZZZZ)
#define NB_LOCATION_COORDINATES_TEXTBUF_LEN    (26)
// Streetnum Street (taking into account supplementary symbols)
#define NB_LOCATION_STREET_ADDRESS_TEXTBUF_LEN  (NB_LOCATION_MAX_STREETNUM_LEN + \
                                                 NB_LOCATION_MAX_STREET_LEN + \
                                                 NB_LOCATION_MAX_CITY_LEN + 1)

// City, State Postal (taking into account supplementary symbols)
#define NB_LOCATION_CITY_ADDRESS_TEXTBUF_LEN    (NB_LOCATION_MAX_CITY_LEN + \
                                                 NB_LOCATION_MAX_STATE_LEN + \
                                                 NB_LOCATION_MAX_POSTAL_LEN + 3)

// global variable of name changed maneuver enabled
static nb_boolean g_ncEnabled = FALSE;

static nb_boolean isM1ManeuverEnabled = FALSE;

/* Initialization */

void    init_navinstruct(navinstruct* instruct,
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
                         )
{
    instruct->base_dist_mul = ((bdm != DISTANCE_CONFIG_INVALID_VALUE) ? bdm : instruct->base_dist_mul);
    instruct->base_dist_add = ((bda != DISTANCE_CONFIG_INVALID_VALUE) ? bda : instruct->base_dist_add);
    instruct->mult[base_dist] = 1.0;
    instruct->mult[announce_dist] = ((adm != DISTANCE_CONFIG_INVALID_VALUE) ? adm : instruct->mult[announce_dist]);
    instruct->mult[continue_dist] = ((cdm != DISTANCE_CONFIG_INVALID_VALUE) ? cdm : instruct->mult[continue_dist]);
    instruct->mult[prepare_dist] = ((pdm != DISTANCE_CONFIG_INVALID_VALUE) ? pdm : instruct->mult[prepare_dist]);
    instruct->mult[prepare_min_dist] = ((pmm != DISTANCE_CONFIG_INVALID_VALUE) ? pmm : instruct->mult[prepare_min_dist]);
    instruct->mult[instruct_dist] = ((idm != DISTANCE_CONFIG_INVALID_VALUE) ? idm : instruct->mult[instruct_dist]);
    instruct->mult[show_stack_dist] = ((xdm != DISTANCE_CONFIG_INVALID_VALUE) ? xdm : instruct->mult[show_stack_dist]);
    instruct->mult[show_vector_dist] = ((vdm != DISTANCE_CONFIG_INVALID_VALUE) ? vdm : instruct->mult[show_vector_dist]);
    /* @TODO: Accord BR 140731: Visual Lane Guidance shall display at the same time as Voice Lane Guidance is
     announced as part of "Prepare to" instruction specific to maneuver point and
     SHALL end when the Turn maneuver has been completed.

     We change this
        instruct->mult[lane_guidance_dist] = (( ldm != DISTANCE_CONFIG_INVALID_VALUE) ? ldm : instruct->mult[lane_guidance_dist]);
     to
    */
    instruct->mult[lane_guidance_dist] = (( ldm != DISTANCE_CONFIG_INVALID_VALUE) ? ldm : pdm);
    instruct->stdi = ((stdi != DISTANCE_CONFIG_INVALID_VALUE) ? stdi : instruct->stdi);
    instruct->stdm = ((stdm != DISTANCE_CONFIG_INVALID_VALUE) ? stdm : instruct->stdm);
    instruct->tdm = ((tdm != DISTANCE_CONFIG_INVALID_VALUE) ? tdm : instruct->tdm);
    instruct->tdi = ((tdi != DISTANCE_CONFIG_INVALID_VALUE) ? tdi : instruct->tdi);
}

/* Heading Operations */

boolean
compare_heading(double heading1, double heading2, double tolerance, double* pdiff)
{
    double head_diff;

    if ((heading1 == INVALID_LATLON) || (heading2 == INVALID_LATLON)) {

        if (pdiff != NULL)
            *pdiff = 360.0;

        return FALSE;
    }

    if (heading1 > heading2)
        head_diff = heading1 - heading2;
    else
        head_diff = heading2 - heading1;

    if (head_diff > 180.0)
        head_diff = 360.0 - head_diff;

    if (pdiff != NULL)
        *pdiff = head_diff;

    return (boolean) (head_diff <= tolerance);
}

boolean
heading_between(double heading1, double heading2, double heading_test, double buffer)
{
    double min_heading = heading1 < heading2 ? heading1 : heading2;
    double max_heading = heading1 < heading2 ? heading2 : heading1;
    double d = max_heading - min_heading;

    if (compare_heading(min_heading, heading_test, buffer, NULL) ||
            compare_heading(max_heading, heading_test, buffer, NULL))
        return TRUE;

    if (d < 180.0)
        return (boolean) ((heading_test >= min_heading) && (heading_test <= max_heading));
    else
        return (boolean) ((heading_test < min_heading) || (heading_test > max_heading));
}

/* GPS Uncertainty Calculations */

double
unc_to_pt(NB_GpsLocation* pfix, double pt_lat, double pt_lon)
{
    double heading;
    double t,st,ct;

    NB_SpatialGetLineOfSightDistance(pfix->latitude, pfix->longitude, pt_lat, pt_lon, &heading);


    t = heading - pfix->horizontalUncertaintyAngleOfAxis;

    ct = nsl_cos(TO_RAD(t));
    st = nsl_sin(TO_RAD(t));

    return nsl_sqrt((pfix->horizontalUncertaintyAlongAxis * pfix->horizontalUncertaintyAlongAxis * ct * ct) +
                 (pfix->horizontalUncertaintyAlongPerpendicular * pfix->horizontalUncertaintyAlongPerpendicular * st * st));
}

/* Route Position Operations */

void
clear_route_position(struct route_position* pos)
{
    pos->closest_maneuver                    = NAV_MANEUVER_NONE;
    pos->closest_segment                     = SEGMENT_NONE;
    pos->est_speed_along_route               = -1;
    pos->proj_lat                            = INVALID_LATLON;
    pos->proj_lon                            = INVALID_LATLON;
    pos->segment_distance                    = -1;
    pos->segment_heading                     = INVALID_LATLON;
    pos->segment_remain                      = -1;
    pos->wrong_way                           = FALSE;
    pos->valid_heading                       = FALSE;
    pos->maneuver_max_inst_distance          = -1;
    pos->maneuver_distance_remaining         = -1;
    pos->maneuver_base_speed                 = -1;

    pos->trip_distance_after                 = -1;
    pos->trip_time_after                     = 0;

	pos->snap_map                            = FALSE;
	pos->near_window_end                     = FALSE;

	pos->match_type                          = rt_match_none;
}


/* Maneuver Type Determination */

enum manuever_type
get_maneuver_type(const char* szturn)
{
    enum manuever_type type = manuever_type_normal;

    if (nsl_strncmp(szturn, "EX.", 3) == 0 ||
        nsl_strncmp(szturn, "ER.", 3) == 0 ||
        nsl_strncmp(szturn, "KH.", 3) == 0 ||
        nsl_strncmp(szturn, "MR.", 3) == 0 ||
        nsl_strncmp(szturn, "SH.", 3) == 0)
    {
        type = maneuver_type_hwy;
    }
    else if ((nsl_strcmp(szturn, "DT.U") != 0) && (nsl_strncmp(szturn, "DT.", 3) == 0))
    {
        type = maneuver_type_dest;
    }
    else if (nsl_strncmp(szturn, "SC.", 3) == 0)
    {
        type = maneuver_type_camera;
    }

    return type;
}

boolean
is_informational(const char* szturn)
{
    if (g_ncEnabled)
    {
        return FALSE;
    }
    return (boolean)(nsl_strcmp("NC.", szturn) == 0);
}

boolean
is_ferry_entry(const char* szturn)
{
    return (boolean)(nsl_strcmp("FE.", szturn) == 0);
}

boolean
is_ferry_exit(const char* szturn)
{
    return (boolean)(nsl_strcmp("FX.", szturn) == 0);
}

boolean
is_speedcamera(const char* szturn)
{
    return (boolean)(nsl_strcmp("SC.", szturn) == 0);
}

boolean
is_stay(const char* szturn)
{
    return (boolean)(nsl_strncmp("ST.", szturn, 3) == 0);
}

boolean
is_stay_highway(const char* szturn)
{
    return (boolean)(nsl_strncmp("SH.", szturn, 3) == 0);
}

boolean
is_merge(const char* szturn)
{
    return (boolean)(nsl_strncmp("MR.", szturn, 3) == 0);
}

boolean
is_keep(const char* szturn)
{
    return (boolean)(nsl_strncmp("KP.", szturn, 3) == 0);
}

boolean
is_tunnel_entry(const char* szturn)
{
    return (boolean)(nsl_strncmp("TE.", szturn, 3) == 0);
}

boolean
is_roundaboutexit(const char* szturn)
{
    return (boolean)(nsl_strncmp("RX.", szturn, 3) == 0);
}

boolean
is_unsupported_man(const char* szturn)
{
    return (boolean)(nsl_strncmp("SS.", szturn, 3) == 0) || (boolean)(nsl_strncmp("TX.", szturn, 3) == 0);
}

boolean
get_maneuver_display(data_util_state* pdus, data_nav_reply* preply, uint32 nman, uint32* pdispnum, uint32* pcount)
{
    uint32 numman = data_nav_reply_num_nav_maneuver(pdus, preply);
    data_nav_maneuver* pmaneuver;
    uint32 n;

    pmaneuver = data_nav_reply_get_nav_maneuver(pdus, preply, nman);

    if ((pmaneuver == NULL) || (is_informational(data_string_get(pdus, &pmaneuver->command)))) {

        if (pdispnum)
            *pdispnum = NAV_MANEUVER_NONE;
        if (pcount)
            *pcount = 0;

        return FALSE;
    }

    if (pdispnum)
        *pdispnum = 0;
    if (pcount)
        *pcount = 0;

    for (n=0; n<numman; n++) {

        pmaneuver = data_nav_reply_get_nav_maneuver(pdus, preply, n);

        if (!is_informational(data_string_get(pdus, &pmaneuver->command))) {

            if (n < nman && pdispnum != NULL)
                (*pdispnum)++;

            if (pcount)
                (*pcount)++;
        }
    }

    if (pdispnum)
        (*pdispnum)++;

    return TRUE;
}

NB_Error
nav_get_file(PAL_Instance* pal, void* pUser, const char* isname, const char* isid, const char* name, size_t name_len, char** pbuf, nb_size* psize)
{
    NB_Error        err = NE_OK;
    unsigned char*  file_data = NULL;
    size_t          file_length = 0;
    char            filename[255] = {0};
    char            nametmp[64];
    PAL_Error       pal_erro = PAL_Ok;

    if (nsl_strncmp(TPSLIB_FILE_NAME, name, name_len) == 0) {

        nsl_strlcpy(filename, "data.tpl", sizeof(filename));
    }
    else {

        nsl_strncpy(nametmp, name, MIN(name_len+1, sizeof(nametmp)-1));

        nsl_strlcpy(filename, isname, sizeof(filename));

        // :KLUDGE: Potentially unsafe.
        // The entire function is potentially unsafe because of the string size limitations.
        // It should be rewritten completely. For now I change it to make it compile on WinCE

#if defined(WINCE) || defined(WIN32) || defined(__APPLE__)
        nsl_strcat(filename, "/");
        nsl_strcat(filename, nametmp);
#else
        nsl_strlcat(filename, "/", sizeof(filename));
        nsl_strlcat(filename, nametmp, sizeof(filename));
#endif
    }

    pal_erro = PAL_FileLoadFile(pal, filename, &file_data, (uint32*)&file_length);
    if (!pal_erro)
    {
        err = NE_RES;
    }

    *pbuf  = (char*)    file_data;
    *psize = (nb_size)  file_length;

    return err;
}

NB_Error
nav_format(void* pUser, uint32 type, void* data, uint32 flags, char* buf, size_t size, int nIndex)
{
    NB_Error ret_status = NE_INVAL;

    if (type == FORMAT_LOCATION)
    {
        NB_Location* location = (NB_Location*)data;

        if (!nsl_strempty(location->areaname))
        {
            // Format as "areaname"
            char temp_str[NB_LOCATION_MAX_AREANAME_LEN+1];
            nsl_snprintf(temp_str, NB_LOCATION_MAX_AREANAME_LEN+1, "%s", location->areaname);
            nsl_strlcpy(buf, temp_str, MIN(nsl_strlen(temp_str)+1, size));
        }
        else if (!nsl_strempty(location->airport))
        {
            // Format as "airport"
            char temp_str[NB_LOCATION_MAX_AIRPORT_LEN+1];
            nsl_snprintf(temp_str, NB_LOCATION_MAX_AIRPORT_LEN+1, "%s", location->airport);
            nsl_strlcpy(buf, temp_str, MIN(NB_LOCATION_MAX_AIRPORT_LEN+1, size));
        }
        else if (!nsl_strempty(location->street1) && !nsl_strempty(location->streetnum))
        {
            // Format as "streetnum street"
            char temp_str[NB_LOCATION_STREET_ADDRESS_TEXTBUF_LEN+1];
            nsl_snprintf(temp_str, NB_LOCATION_STREET_ADDRESS_TEXTBUF_LEN+1,
                "%s %s", location->streetnum, location->street1);
            nsl_strlcpy(buf, temp_str, MIN(nsl_strlen(temp_str)+1, size));
        }
        else if (!nsl_strempty(location->city) && !nsl_strempty(location->state) && !nsl_strempty(location->postal))
        {
            // Format as "city, state postal"
            char temp_str[NB_LOCATION_CITY_ADDRESS_TEXTBUF_LEN+1];
            nsl_snprintf(temp_str, NB_LOCATION_CITY_ADDRESS_TEXTBUF_LEN+1,
                                           "%s, %s %s", location->city,
                                                        location->state,
                                                        location->postal);
            nsl_strlcpy(buf, temp_str, MIN(nsl_strlen(temp_str)+1, size));
        }
        else if (flags & FORMAT_FLAGS_ALLOW_LATLON)
        {
            // Raw destination (lat; lon)
            char temp_str[NB_LOCATION_COORDINATES_TEXTBUF_LEN+1];
            nsl_snprintf(temp_str, NB_LOCATION_COORDINATES_TEXTBUF_LEN+1,
                "(%.6f; %.6f)", location->latitude, location->longitude);
            nsl_strlcpy(buf, temp_str, MIN(NB_LOCATION_COORDINATES_TEXTBUF_LEN+1, size));
        }
        ret_status = NE_OK;

    } // (type == FORMAT_LOCATION)

    return ret_status;

}

void
convert_reserved_phrases(instructset* steps, data_util_state* pdus, data_nav_reply* pnr)
{
    int n, l;
    data_nav_maneuver *pnav;
    l = data_nav_reply_num_nav_maneuver(pdus, pnr);

    for (n = 0; n < l; n++)
    {
        pnav = data_nav_reply_get_nav_maneuver(pdus, pnr, n);
        /* Loop over all maneuvers */
        if (pnav->current_roadinfo.unnamed && pnav->current_roadinfo.trans != ROADINFO_INVALID_TRANS)
        {
            data_string_set(pdus, &pnav->current_roadinfo.primary, instructset_getreservedphrase(steps, pnav->current_roadinfo.trans));
        }
        if (pnav->turn_roadinfo.unnamed && pnav->current_roadinfo.trans != ROADINFO_INVALID_TRANS)
        {
            data_string_set(pdus, &pnav->turn_roadinfo.primary, instructset_getreservedphrase(steps, pnav->turn_roadinfo.trans));
        }
    }
}

double
get_average_heading(double heading1, double heading2)
{
    double gap = 0.0;
    if (heading1 > heading2)
    {
        gap = heading1 - heading2;
    }
    else
    {
        gap = heading2 - heading1;
    }

    if (gap <= 180)
    {
        return (heading1 + heading2) / 2;
    }
    else
    {
        double averageTmp = (heading1 + heading2 + 360) / 2;
        if (averageTmp >= 360)
        {
            averageTmp -= 360;
        }
        return averageTmp;
    }
}

void
set_namechanged_maneuver_enabled(nb_boolean enabled)
{
    g_ncEnabled = enabled;
}

nb_boolean
get_m1_maneuver_enabled()
{
    return isM1ManeuverEnabled;
}

void
set_m1_maneuver_enabled(nb_boolean enabled)
{
    isM1ManeuverEnabled = enabled;
}

const char
get_traffic_color(double delayPercentage, boolean isTrafficValid)
{
    if (!isTrafficValid)
    {
        return 'U';
    }

    if (delayPercentage <= 25.00)
    {
        return 'G';
    }
    else if (delayPercentage <= 75.00)
    {
        return 'Y';
    }
    else
    {
        return 'R';
    }
}
