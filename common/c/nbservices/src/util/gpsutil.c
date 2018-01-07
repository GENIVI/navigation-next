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

    @file     gpsutil.c
    @defgroup gputil GPS
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "gpsutil.h"
#include "palmath.h"
#include "palclock.h"
#include "paltypes.h"
#include "csltypes.h"

/*! @{ */

static const double unctbl[] = { 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0,
    48.0, 64.0, 96.0, 128.0, 192.0, 256.0, 384.0, 512.0, 768.0, 1024.0, 1536.0, 2048.0, 3072.0, 4096.0,
    6144.0, 8192.0, 12288.0, -1.0, -2.0 };

NB_DEF uint8
uncencode(double v)
{
    uint8 n;

    for (n=0;n<(sizeof(unctbl) / sizeof(*unctbl));n++) 
        if (v <= unctbl[n])
            return n;

    return (sizeof(unctbl) / sizeof(*unctbl))-1;
}

NB_DEF boolean
gpsfix_pack(NB_GpsLocation* gpsfix, byte* buf, size_t size)
{
    uint32  gps_timestamp   = nsl_htonl(gpsfix->gpsTime);
    int32   gps_lat         = nsl_htonl((int32) (gpsfix->latitude / 0.00000536441803));
    int32   gps_lon         = nsl_htonl((int32) (gpsfix->longitude / 0.00000536441803));
    uint16  gps_heading     = nsl_htons((uint16) ((gpsfix->valid & NGV_Heading) ? (gpsfix->heading / 0.3515625) : (0xFFFF)));
    uint16  gps_hvel        = nsl_htons((uint16) ((gpsfix->valid & NGV_HorizontalVelocity) ? (gpsfix->horizontalVelocity / 0.25) : (0xFFFF)));
    uint8   gps_uncang      = (uint8) ((gpsfix->valid & NGV_HorizontalUncertainty) ? (gpsfix->horizontalUncertaintyAngleOfAxis / 5.625) : 0xFF);
    uint8   gps_unca        = (gpsfix->valid & NGV_AxisUncertainty) ? uncencode(gpsfix->horizontalUncertaintyAlongAxis) : 0xFF;
    uint8   gps_uncp        = (gpsfix->valid & NGV_PerpendicularUncertainty) ? uncencode(gpsfix->horizontalUncertaintyAlongPerpendicular) : 0xFF;

    if (size != GPS_PACKED_LEN)
        return FALSE;

    nsl_memcpy(buf, &gps_timestamp, 4);
    nsl_memcpy(buf+4, &gps_lat, 4);
    nsl_memcpy(buf+8, &gps_lon, 4);
    nsl_memcpy(buf+12, &gps_heading, 2);
    nsl_memcpy(buf+14, &gps_hvel, 2);
    buf[16] = gps_uncang;
    buf[17] = gps_unca;
    buf[18] = gps_uncp;

    return TRUE;
}

NB_DEF void
gpsfix_clear(NB_GpsLocation* gpf)
{
    gpf->gpsTime = 0;
    gpf->latitude = INVALID_LATLON;
    gpf->longitude = INVALID_LATLON;
    gpf->heading = INVALID_LATLON;
    gpf->horizontalVelocity = 0.0;
    gpf->altitude = 0.0;
    gpf->horizontalUncertaintyAngleOfAxis = 0.0;
    gpf->horizontalUncertaintyAlongAxis = 0.0;
    gpf->horizontalUncertaintyAlongPerpendicular = 0.0;
    gpf->valid = 0;
}

NB_DEF void
gpsfix_setpoint(NB_GpsLocation* gpf, double lat, double lon)
{
    gpsfix_clear(gpf);
    
    gpf->latitude  = lat;
    gpf->longitude = lon;
    gpf->horizontalUncertaintyAngleOfAxis = 0;
    gpf->horizontalUncertaintyAlongAxis = 250;
    gpf->horizontalUncertaintyAlongPerpendicular = 250;
    gpf->valid = NGV_Latitude | NGV_Longitude | NGV_AxisUncertainty | NGV_HorizontalUncertainty | NGV_PerpendicularUncertainty;
}

NB_DEF boolean 
gpsfix_validloc(NB_GpsLocation* gpf)
{
    return ((gpf->latitude != INVALID_LATLON) && (gpf->longitude != INVALID_LATLON) && 
            (gpf->valid & NGV_Latitude) && (gpf->valid & NGV_Longitude)) ? TRUE : FALSE;
}

/// @todo (BUG 56062) Need to implement translate_gps_error

NB_DEF NB_Error 
translate_gps_error(uint32 status, boolean ignore_minor)
{
    NB_Error err = NE_OK;
/*   
    switch (status) {
        case AEEGPS_ERR_NO_ERR:
            err = NE_OK;
            break;
        case AEEGPS_ERR_GENERAL_FAILURE:
            err = NE_GPS;
            break;
        case AEEGPS_ERR_TIMEOUT:
            err = NE_GPS_TIMEOUT;
            break;
        case AEEGPS_ERR_ACCURACY_UNAVAIL:
        case AEEGPS_ERR_INFO_UNAVAIL:
            err = NE_GPS_NO_INFO;
            break;
        case AEEGPS_ERR_PRIVACY_REFUSED:
            err = NE_GPS_PRIVACY;
            break;
        case AEEGPS_ERR_RECEIVER_BUSY:
            err = NE_GPS_BUSY;
            break;
        case AEEGPS_ERR_DNS_LOOKUP: 
            err = NE_DNS_LOOKUP_ERROR;
            break;
        default:
            err = NE_GPS;
    }
    
    if (ignore_minor && status != AEEGPS_ERR_PRIVACY_REFUSED && status != AEEGPS_ERR_GENERAL_FAILURE)
        err = NE_OK;
    
    if (err == NE_GPS && is_e911_only()) {
        err = NE_GPS_LOCATION_OFF;
    }
*/    
    return err;
}

NB_DEF boolean 
is_gps_error(NB_Error err)
{
    return (boolean) (err == NE_GPS || err == NE_GPS_PRIVACY || 
                      err == NE_GPS_TIMEOUT || err == NE_GPS_NO_INFO ||
                      err == NE_GPS_BUSY || err == NE_GPS_LOCATION_OFF);// || err == NE_DNS_LOOKUP_ERROR);
}

/* GPS Quality Check */
NB_DEF boolean 
gps_good(double nav_sifter_error, NB_GpsLocation* pfix)
{
    // If the error in the fix is beyond our tolerance, ignore it and continue
    // We currently don't consider the uncertainty angle
    if ((pfix->horizontalUncertaintyAlongPerpendicular <  0) || (pfix->horizontalUncertaintyAlongPerpendicular > nav_sifter_error))
    {
        return FALSE;
    }

    return TRUE;
}

NB_DEF boolean 
heading_good(double min_valid_speed, NB_GpsLocation* pfix)
{
    return (boolean)((pfix->valid & NGV_Heading) && (pfix->valid & NGV_HorizontalVelocity) && 
        (pfix->horizontalVelocity >= min_valid_speed) );
}

NB_DEF int        
get_gps_qual(uint32 gps_warn, NB_GpsLocation* pfix, uint32 gps_time)
{
    uint32 cur_time = PAL_ClockGetGPSTime();
    uint32 gps_age = 0;
    uint32 gps_age_time = MAX(gps_time, (uint32)pfix->gpsTime);

    if (gpsfix_validloc(pfix))
    {
        /* Handle the case where the number of leap seconds is incorrect and the GPS time is in the future. */
        if (gps_age_time > cur_time)
            return 1;

        gps_age = (uint32) (cur_time - gps_age_time);

        if (gps_age <= gps_warn)
            return 1;
    }

    return 0;
}

NB_DEF boolean
gpsfix_identical(NB_GpsLocation* gpsfix1, NB_GpsLocation* gpsfix2)
{
    if (gpsfix1->gpsTime != gpsfix2->gpsTime)
    {
        return FALSE;
    }

    if (gpsfix1->valid != gpsfix2->valid)
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_Latitude) && (gpsfix1->latitude  != gpsfix2->latitude))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_Longitude) && (gpsfix1->longitude  != gpsfix2->longitude))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_Altitude) && (gpsfix1->altitude  != gpsfix2->altitude))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_Heading) && (gpsfix1->heading  != gpsfix2->heading))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_HorizontalVelocity) && (gpsfix1->horizontalVelocity  != gpsfix2->horizontalVelocity))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_VerticalVelocity) && (gpsfix1->verticalVelocity  != gpsfix2->verticalVelocity))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_HorizontalUncertainty) && (gpsfix1->horizontalUncertaintyAngleOfAxis  != gpsfix2->horizontalUncertaintyAngleOfAxis))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_AxisUncertainty) && (gpsfix1->horizontalUncertaintyAlongAxis  != gpsfix2->horizontalUncertaintyAlongAxis))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_PerpendicularUncertainty) && (gpsfix1->horizontalUncertaintyAlongPerpendicular  != gpsfix2->horizontalUncertaintyAlongPerpendicular))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_VerticalUncertainty) && (gpsfix1->verticalUncertainty  != gpsfix2->verticalUncertainty))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_UTCOffset) && (gpsfix1->utcOffset  != gpsfix2->utcOffset))
    {
        return FALSE;
    }

    if ((gpsfix1->valid & NGV_SatelliteCount) && (gpsfix1->numberOfSatellites  != gpsfix2->numberOfSatellites))
    {
        return FALSE;
    }

    return TRUE;
}

/*! @} */
