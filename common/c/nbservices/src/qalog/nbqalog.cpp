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

    @file     nbqalog.c
    @defgroup nbqalog QA Logging

    This API is used to log records to the QA log.
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "nbqalog.h"
#include "palstdlib.h"
#include "palclock.h"
#include "cslqalog.h"
#include "cslqarecorddefinitions.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "gpsutil.h"
#include "nbnavigationstate.h"
#include "nbnavigationstateprivate.h"
#include "nbguidancemessageprivate.h"
#include "nbnaturallaneguidancetype.h"

#include "nbmetadatatypes.h"
#include "nbspeedlimittypes.h"
}
    
#define QARECORD_LKREQUEST_BUFFER_SIZE 40
#define QARECORD_LKREPLY_BUFFER_SIZE   24
#define MAC_LENGTH                     6

// Limitations of TrafficIncident, defined in qarecords.py.
#define MAX_ROAD_NAME_LENGTH           51
#define MAX_DESCRIPTION_LENGTH         401

static const uint16 INITIAL_QARECORD_BUFFER_SIZE = 250;
static const uint32 TRAFFIC_REGION_LOCATION_SIZE =  16;
static const uint16 API_CALL_FUNCTION_LENGTH     =  50;
static const uint16 API_CALL_MESSAGE_LENGTH      =  80;
static const uint16 LKEVENT_BUFFER_SIZE          =   4;

#define KPI_FREEFORM_EVENT_LABEL_MAX_LENGTH   64
#define KPI_SESSION_BUNDLE_ID_MAX_LENGTH      64
#define KPI_SESSION_APPLICATION_ID_MAX_LENGTH 64
#define KPI_SESSION_DEVICE_ID_MAX_LENGTH      64

static NB_Error AddPlaceToRecord(CSL_QaRecord* record, NB_Place* place, const char* label);
static void LogGPSFix(NB_Context* context, uint16 recordId, NB_GpsLocation* location);
static void HexlifyRouteId(const byte* data, uint32 dataSize, int hexBufferSize, char* hexBuffer);

static uint8 ConvertContentTypeString(const char* contentType);
static uint8 ConvertProjectionTypeString(const char* projectionType);
static uint8 ConvertProjectionDatumString(const char* projectionDatum);

NB_DEF NB_Error
NB_QaLogWrite(NB_Context* context, NB_QaRecord* record)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    return CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
}

NB_DEF void 
NB_QaLogAppState(NB_Context* context, const char* state)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_ASSERT_VALID_THREAD(context);

    CSL_QaLogAppState(NB_ContextGetQaLog(context), state);
}

NB_DEF void 
NB_QaLogAudioBase(NB_Context* context, const char* name)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    char tmpstr[QCP_NAME_LEN] = {0};

    CSL_QaRecord* record = 0;
    
    PAL_Instance* pal = NB_ContextGetPal(context);
    if (!PAL_IsPalThread(pal))
    {
        return;
    }

	nsl_strlcpy(tmpstr, name, QCP_NAME_LEN);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_QCP_BASE, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteText(record, tmpstr, QCP_NAME_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }       
}

NB_DEF void
NB_QaLogAudioPronun(NB_Context* context, const char* name)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    char tmpstr[QCP_NAME_LEN] = {0};

    CSL_QaRecord* record = 0;

    PAL_Instance* pal = NB_ContextGetPal(context);
    if (!PAL_IsPalThread(pal))
    {
        return;
    }

	nsl_strlcpy(tmpstr, name, QCP_NAME_LEN);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_QCP_PRONUN, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteText(record, tmpstr, QCP_NAME_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }       
}

NB_DEF void 
NB_QaLogAudioCacheLookup(NB_Context* context, const char* id, nb_size idlen)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    char tmpstr[QCP_ID_LEN] = {0};
    CSL_QaRecord* record = 0;
    nb_size len = idlen;

    NB_ASSERT_VALID_THREAD(context);

	if (len > QCP_ID_LEN)
		len = QCP_ID_LEN;

	nsl_memcpy(tmpstr, id, len);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_QCP_CACHE_LOOKUP2, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteText(record, tmpstr, QCP_ID_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }       
}

NB_DEF void 
NB_QaLogGPSFix(NB_Context* context, NB_GpsLocation* location)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_ASSERT_VALID_THREAD(context);

    LogGPSFix(context, RECORD_ID_GPS_FIX, location);
}

NB_DEF void
NB_QaLogGPSFixFiltered(NB_Context* context, NB_GpsLocation* location)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_ASSERT_VALID_THREAD(context);

    LogGPSFix(context, RECORD_ID_GPS_FIX_FILTERED, location);
}

NB_DEF void
NB_QaLogGPSFilterState(NB_Context* context, double sum, double constant, double latitude, double longitude)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;

    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GPS_FILTER_STATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteDouble(record, sum);
        err = err ? err : CSL_QaRecordWriteDouble(record, constant);
        err = err ? err : CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogRouteOrigin(NB_Context* context, NB_GpsLocation* location)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    double uncertainty = 0.0;
    
    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GPS_FIX_ORIGIN, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, location->status);
        err = err ? err : CSL_QaRecordWriteUint16(record, (uint16) (location->valid));
        err = err ? err : CSL_QaRecordWriteUint32(record, location->gpsTime);

        err = err ? err : CSL_QaRecordWriteDouble(record, location->latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, location->longitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, location->heading);
        err = err ? err : CSL_QaRecordWriteDouble(record, location->horizontalVelocity);
        err = err ? err : CSL_QaRecordWriteDouble(record, location->altitude + 500.0);
        
        /* horizontal uncertainty magnitude */
        if (((~location->valid & (NGV_AxisUncertainty | NGV_HorizontalUncertainty | NGV_PerpendicularUncertainty)) == 0) &&
            location->horizontalUncertaintyAlongAxis > 0.0 &&
            location->horizontalUncertaintyAlongPerpendicular > 0.0)
        {
            /* from v4 gpsinfo2gpsfix function, magnitude is the
               maximum uncertainty along the axis or perpendicular
               when horizontal, axis and perpendicular uncertainties
               are all marked valid and the axis and perpendicular
               uncertainties are both greater than 0 */
            uncertainty = location->horizontalUncertaintyAlongAxis;
            if (location->horizontalUncertaintyAlongPerpendicular > uncertainty)
            {
                uncertainty = location->horizontalUncertaintyAlongPerpendicular;
            }
        }
        else
        {
            uncertainty = 0.0;
        }
        err = err ? err : CSL_QaRecordWriteDouble(record, uncertainty);

        err = err ? err : CSL_QaRecordWriteDouble(record, (location->valid & NGV_HorizontalUncertainty) ? location->horizontalUncertaintyAngleOfAxis : -1.0);
        err = err ? err : CSL_QaRecordWriteDouble(record, (location->valid & NGV_AxisUncertainty) ? location->horizontalUncertaintyAlongAxis: -1.0);
        err = err ? err : CSL_QaRecordWriteDouble(record, (location->valid & NGV_PerpendicularUncertainty) ? location->horizontalUncertaintyAlongPerpendicular : -1.0);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogNavigationState(NB_Context* context, NB_NavigationState* state, NB_NavigationPublicState* publicState)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_STATE3, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteText(record, publicState->currentStreet, NAV_STATE_STREET_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, publicState->destinationStreet, NAV_STATE_STREET_LEN);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->maneuverPos);
        err = err ? err : CSL_QaRecordWriteText(record, state->turnCode, NAV_STATE_CODE_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, state->stackCode, NAV_STATE_CODE_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, "", NAV_STATE_CODE_LEN);                        // removed unused state->nextCode

        err = err ? err : CSL_QaRecordWriteDouble(record, publicState->turnLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, publicState->turnLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, publicState->turnRemainDistance);
        err = err ? err : CSL_QaRecordWriteUint32(record, publicState->turnRemainTime);
        err = err ? err : CSL_QaRecordWriteDouble(record, 0);

        err = err ? err : CSL_QaRecordWriteDouble(record, state->remainingRouteDist);
        err = err ? err : CSL_QaRecordWriteUint32(record, state->remainingRouteTime);

        err = err ? err : CSL_QaRecordWriteDouble(record, state->lastHeading);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentSpeed);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.proj_lat);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.proj_lon);

        err = err ? err : CSL_QaRecordWriteUint8(record, 0);                                            // fix quality is not stored in state
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->routeError);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)publicState->status);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogNavigationPosition(NB_Context* context, NB_NavigationState* state, const char* action)  
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_POS2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, state->currentRoutePosition.closest_segment);
        err = err ? err : CSL_QaRecordWriteUint32(record, state->currentRoutePosition.closest_maneuver);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.segment_remain);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.segment_distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.segment_heading);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.proj_lat);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.proj_lon);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.est_speed_along_route);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->currentRoutePosition.wrong_way);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.maneuver_max_inst_distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.maneuver_distance_remaining);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.maneuver_base_speed);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->currentRoutePosition.trip_distance_after);
        err = err ? err : CSL_QaRecordWriteUint32(record, state->currentRoutePosition.trip_time_after);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->currentRoutePosition.match_type);
        err = err ? err : CSL_QaRecordWriteDouble(record, 0.0);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->offRouteCount);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->wrongWayCount);
        err = err ? err : CSL_QaRecordWriteText(record, action, NAV_POS_ACTION_LEN);
//        err = err ? err : CSL_QaRecordWriteDouble(record, get_nav_avg_speed(&state->averageSpeed));

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogNavigationStartup(NB_Context* context, NB_LatitudeLongitude* destinationLatLong, const char* destinationStreet, NB_RouteSettings* settings)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_STARTUP2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteDouble(record, destinationLatLong->latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, destinationLatLong->longitude);
        err = err ? err : CSL_QaRecordWriteText(record, destinationStreet, NAV_STARTUP_STREET_LEN);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)settings->route_type);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)settings->vehicle_type);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)settings->route_avoid);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogNavigationOffRoute(NB_Context* context, uint32 offRouteCount, uint32 wrongWayCount, const char* action)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_OFFROUTE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)offRouteCount);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)wrongWayCount);
        err = err ? err : CSL_QaRecordWriteText(record, action, NAV_POS_ACTION_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogSegmentMatch(NB_Context* context, struct seg_match* segment, nb_boolean bestMatch)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SEGMATCH2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)bestMatch);
        err = err ? err : CSL_QaRecordWriteUint32(record, segment->time);
        err = err ? err : CSL_QaRecordWriteDouble(record, segment->travel_speed);
        err = err ? err : CSL_QaRecordWriteUint32(record, segment->maneuver);
        err = err ? err : CSL_QaRecordWriteUint32(record, segment->segment);
        err = err ? err : CSL_QaRecordWriteDouble(record, segment->remain);
        err = err ? err : CSL_QaRecordWriteDouble(record, segment->distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, segment->proj_lat);
        err = err ? err : CSL_QaRecordWriteDouble(record, segment->proj_lon);
        err = err ? err : CSL_QaRecordWriteDouble(record, segment->heading);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)segment->dirmatch);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)segment->type);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}

NB_DEF void
NB_QaLogRouteId(NB_Context* context, const byte* data, uint32 dataSize)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ROUTE_ID, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteData(record, data, dataSize);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogPlace(NB_Context* context, NB_Place* place, const char* label)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_PLACE7, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = AddPlaceToRecord(record, place, label);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogNavigationError(NB_Context* context, NB_NavigateRouteError routeError, NB_Error nbError, NB_NetworkRequestStatus networkStatus, const char* function, const char* file, int line)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_ERROR, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, (uint32)routeError);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)nbError);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)networkStatus);

        err = err ? err : CSL_QaRecordWriteText(record, function, MAX_FILE_NAME_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, file, MAX_FILE_NAME_LEN);
        err = err ? err : CSL_QaRecordWriteInt32(record, line);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}


NB_DEF void
NB_QaLogVoiceCache(NB_Context* context, int32 maxEntries, int32 dirtyEntries, int32 cacheSize, int32 protectedEntries, int32 downloadCount, int32 deletedEntries)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_VOICECACHE2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteInt32(record, maxEntries);
        err = err ? err : CSL_QaRecordWriteInt32(record, dirtyEntries);
        err = err ? err : CSL_QaRecordWriteInt32(record, cacheSize);
        err = err ? err : CSL_QaRecordWriteInt32(record, protectedEntries);
        err = err ? err : CSL_QaRecordWriteInt32(record, downloadCount);
        err = err ? err : CSL_QaRecordWriteInt32(record, deletedEntries);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}

NB_DEF void 
NB_QaLogPrefetchArea(NB_Context* context, 
     double segmentLatitude1, double segmentLongitude1, 
	 double segmentLatitude2, double segmentLongitude2,
	 double boxLatitude1, double boxLongitude1, 
     double boxLatitude2, double boxLongitude2, 
	 double boxLatitude3, double boxLongitude3, 
     double boxLatitude4, double boxLongitude4)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_PREFETCH_AREA, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        /* Log the prefetch segment */
        err = CSL_QaRecordWriteDouble(record, segmentLatitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, segmentLongitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, segmentLatitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, segmentLongitude2);

        /* Log the prefetch box */
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude3);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude3);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude4);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude4);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    } 
}

NB_DEF void
NB_QaLogPrefetchPoint(NB_Context* context, double latitude, double longitude)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_PREFETCH_POINT, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        /* Log the prefetch segment */
        err = CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    } 
}

NB_DEF void 
NB_QaLogRequestVectorTile(NB_Context* context, uint32 tx, uint32 ty, uint32 tz)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, tx);
        err = err ? err : CSL_QaRecordWriteUint32(record, ty);
        err = err ? err : CSL_QaRecordWriteUint32(record, tz);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    } 
}

NB_DEF void
NB_QaLogReceiveVectorTile(NB_Context* context, uint32 type, uint32 tx, uint32 ty, uint32 tz)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_RECEIVED, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, type);
        err = err ? err : CSL_QaRecordWriteUint32(record, tx);
        err = err ? err : CSL_QaRecordWriteUint32(record, ty);
        err = err ? err : CSL_QaRecordWriteUint32(record, tz);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    } 
}

NB_DEF void
NB_QaLogDiscardVectorTile(NB_Context* context, uint32 tx, uint32 ty, uint32 tz)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_DISCARDED, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, tx);
        err = err ? err : CSL_QaRecordWriteUint32(record, ty);
        err = err ? err : CSL_QaRecordWriteUint32(record, tz);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    } 
}

NB_DEF void
NB_QaLogVectorTileState(NB_Context* context, uint32 totalTiles, uint32 validTiles, 
                uint32 pendingtiles, uint32 downloadingtiles, uint32 discardedtiles, 
                uint32 lastData, uint32 totalData)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_STATE, INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, totalTiles);
        err = err ? err : CSL_QaRecordWriteUint32(record, validTiles);
        err = err ? err : CSL_QaRecordWriteUint32(record, pendingtiles);
        err = err ? err : CSL_QaRecordWriteUint32(record, downloadingtiles);
        err = err ? err : CSL_QaRecordWriteUint32(record, discardedtiles);
        err = err ? err : CSL_QaRecordWriteUint32(record, lastData);
        err = err ? err : CSL_QaRecordWriteUint32(record, totalData);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    } 
}
NB_DEF void NB_QaLogRouteTrafficRegion(NB_Context* context, uint32 maneuver, double start, double length, const char* location)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_REGION, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, maneuver);
        err = err ? err : CSL_QaRecordWriteDouble(record, start);
        err = err ? err : CSL_QaRecordWriteDouble(record, length);
        err = err ? err : CSL_QaRecordWriteText(record, location, TRAFFIC_REGION_LOCATION_SIZE);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogTrafficState(NB_Context* context, uint32 currentManeuver, double maneuverDistanceOffset,
                     uint32 routeRemainTime, uint32 routeRemainDelay, double routeRemainMeter)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_STATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, currentManeuver);
        err = err ? err : CSL_QaRecordWriteDouble(record, maneuverDistanceOffset);
        err = err ? err : CSL_QaRecordWriteUint32(record, routeRemainTime);
        err = err ? err : CSL_QaRecordWriteUint32(record, routeRemainDelay);
        err = err ? err : CSL_QaRecordWriteDouble(record, routeRemainMeter);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogTrafficIncident(NB_Context* context, NB_TrafficIncident* incident, NB_Place* place, const char* label)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_INCIDENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = AddPlaceToRecord(record, place, label);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)incident->type);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)incident->severity);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)incident->entry_time);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)incident->start_time);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)incident->end_time);

        //@note: length of road was modified by someone to fix bugs, as a result, this
        //       record can't be parsed by qaextract.py any more. Since it is too risky to
        //       change it back, here we simply record the 51 bytes of road name and the
        //       first 401 bytes of description. Some information may get lost when road
        //       name or description is too long, but this is not fatal.
        err = err ? err : CSL_QaRecordWriteText(record, incident->road, MAX_ROAD_NAME_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, incident->description, MAX_DESCRIPTION_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)incident->original_item_id);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogTrafficFlow(NB_Context* context, const char* label, const char* tmcLocation, const char* color, double speed, double freeFlowSpeed)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_FLOW2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteText(record, label, PLACE_LABEL_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, tmcLocation, TMC_LOC_LEN);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)color[0]);
        err = err ? err : CSL_QaRecordWriteDouble(record, speed);
        err = err ? err : CSL_QaRecordWriteDouble(record, freeFlowSpeed);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void
NB_QaLogTrafficId(NB_Context* context, const char* sessionId)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_SESSION, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteText(record, sessionId, SMS_LOG_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogTrafficCongestionWarning(NB_Context* context, const char* tmcLocation, double distance, NB_NavigateTrafficCongestionType type, double speed)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CONGESTION_WARNING, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteText(record, tmcLocation, TMC_LOC_LEN);
        err = err ? err : CSL_QaRecordWriteDouble(record, distance);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)type);
        err = err ? err : CSL_QaRecordWriteDouble(record, speed);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void
NB_QaLogTrafficIncidentWarning(NB_Context* context, double distance, double latitude, double longitude)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_INCIDENT_WARNING, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteDouble(record, distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void
NB_QaLogTrafficUpdate(NB_Context* context, uint32 newIncidents, uint32 clearedIncidents, uint32 travelTimeDelta)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_UPDATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, newIncidents);
        err = err ? err : CSL_QaRecordWriteUint32(record, clearedIncidents);
        err = err ? err : CSL_QaRecordWriteUint32(record, travelTimeDelta);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogAnalyticsEvent(NB_Context* context, uint32 eventId, uint32 timeStamp,
        uint32 sessionId, uint8 eventType, uint8 eventStatus)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ANALYTICS_EVENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, eventId);
        err = err ? err : CSL_QaRecordWriteUint32(record, timeStamp);
        err = err ? err : CSL_QaRecordWriteUint32(record, sessionId);
        err = err ? err : CSL_QaRecordWriteUint8(record, eventType);
        err = err ? err : CSL_QaRecordWriteUint8(record, eventStatus);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogAnalyticsConfig(NB_Context* context,
        uint32 maxHoldTime, uint32 maxDataAge, uint32 maxRequestSize,
        uint8 goldEventsPriority, uint8 poiEventsPriority,
        uint8 routeTrackingPriority, uint8 gpsProbesPriority,
        uint32 gpsProbesSampleRate)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ANALYTICS_CONFIG,
            INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, maxHoldTime);
        err = err ? err : CSL_QaRecordWriteUint32(record, maxDataAge);
        err = err ? err : CSL_QaRecordWriteUint32(record, maxRequestSize);
        err = err ? err : CSL_QaRecordWriteUint8(record, goldEventsPriority);
        err = err ? err : CSL_QaRecordWriteUint8(record, poiEventsPriority);
        err = err ? err : CSL_QaRecordWriteUint8(record, routeTrackingPriority);
        err = err ? err : CSL_QaRecordWriteUint8(record, gpsProbesPriority);
        err = err ? err : CSL_QaRecordWriteUint32(record, gpsProbesSampleRate);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogDTSRequest(NB_Context* context)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_SOURCE_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogDTSReply(NB_Context* context,
                 NB_QaLogMapType type,
                 const char* url)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_SOURCE_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)type);
        err = err ? err : CSL_QaRecordWriteText(record, url, RASTER_TILE_URL_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void
NB_QaLogTileRequest(NB_Context* context,
                    uint32 xPosition,
                    uint32 yPosition,
                    uint32 zoomLevel)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_REQUEST,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record,   xPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record,   yPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record,   zoomLevel);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void
NB_QaLogTileReceived(NB_Context* context,
                     uint32 type,
                     uint32 xPosition,
                     uint32 yPosition,
                     uint32 zoomLevel)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_RECEIVED,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, type);
        err = err ? err : CSL_QaRecordWriteUint32(record, xPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record, yPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record, zoomLevel);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogRasterTileRequest(NB_Context* context, 
                          NB_QaLogMapType mapType,
                          uint32 xPosition,
                          uint32 yPosition,
                          uint32 zoomLevel,
                          uint32 resolution,
                          uint32 tileSize,
                          const char* url)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record,    (uint8)mapType);
        err = err ? err : CSL_QaRecordWriteUint32(record,   xPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record,   yPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record,   zoomLevel);
        err = err ? err : CSL_QaRecordWriteUint32(record,   resolution);
        err = err ? err : CSL_QaRecordWriteUint32(record,   tileSize);
        err = err ? err : CSL_QaRecordWriteText(record,     url, RASTER_TILE_URL_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogRasterTileReply(NB_Context* context,
                        NB_QaLogMapType mapType,
                        uint32 xPosition,
                        uint32 yPosition,
                        uint32 zoomLevel,
                        uint32 resolution,
                        uint32 tileSize,
                        uint32 tileBufferSize,
                        NB_Error result)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_RECEIVED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record,    (uint8)mapType);
        err = err ? err : CSL_QaRecordWriteUint32(record,   xPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record,   yPosition);
        err = err ? err : CSL_QaRecordWriteUint32(record,   zoomLevel);
        err = err ? err : CSL_QaRecordWriteUint32(record,   resolution);
        err = err ? err : CSL_QaRecordWriteUint32(record,   tileSize);
        err = err ? err : CSL_QaRecordWriteUint32(record,   tileBufferSize);

        /*
            @bug: NB_Error has more then 255 values and higher error codes will be logged wrong.
                  I don't know why we log the value as an Uint8 and not Uint32. I just put the cast in for now 
                  to remove the warning. I'm assuming we have change the protocol?
         */
        err = err ? err : CSL_QaRecordWriteUint8(record,    (uint8)result);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogRasterCacheState(NB_Context* context,
                         uint32 totalCount, 
    		             uint32 memoryCount, 
                         uint32 persistentCount, 
                         uint32 pendingCount, 
                         uint32 discardedCount)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_CACHE_STATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, totalCount);
        err = err ? err : CSL_QaRecordWriteUint32(record, memoryCount);
        err = err ? err : CSL_QaRecordWriteUint32(record, persistentCount);
        err = err ? err : CSL_QaRecordWriteUint32(record, pendingCount);
        err = err ? err : CSL_QaRecordWriteUint32(record, discardedCount);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogRasterTileCenterChange(NB_Context* context,
                               double oldCenterLatitude, 
                               double oldCenterLongitude, 
                               double newCenterLatitude, 
                               double newCenterLongitude)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_CENTER_CHANGED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteDouble(record, oldCenterLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, oldCenterLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, newCenterLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, newCenterLongitude);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogRasterTileZoomChange(NB_Context* context,
                             uint32 oldZoomLevel, 
                             uint32 newZoomLevel)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_ZOOM_CHANGED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, oldZoomLevel);
        err = err ? err : CSL_QaRecordWriteUint32(record, newZoomLevel);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

// See header file for description
NB_DEF void 
NB_QaLogRasterTileEvent(NB_Context* context,
                        double centerLatitude, 
                        double centerLongitude, 
                        int zoomLevel, 
                        const char* functionName, 
                        NB_QaLogRasterTileEventType eventType)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MAPTILE_EVENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteDouble(record,   centerLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record,   centerLongitude);
        err = err ? err : CSL_QaRecordWriteUint32(record,   zoomLevel);
        err = err ? err : CSL_QaRecordWriteText(record,     functionName, RASTER_TILE_FUNCTION_NAME_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint8(record,    (uint8)eventType);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void 
NB_QaLogRouteQuery(NB_Context* context, NB_QaLogRouteQueryType type, 
                   NB_LatitudeLongitude* originLatLong, NB_LatitudeLongitude* destinationLatLong, NB_RouteSettings* settings)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ROUTE_QUERY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)type);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLatLong->latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLatLong->longitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, destinationLatLong->latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, destinationLatLong->longitude);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)settings->route_type);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)settings->vehicle_type);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)settings->route_avoid);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void 
NB_QaLogRouteReply(NB_Context* context, NB_LatitudeLongitude* originLatLong, 
                   NB_LatitudeLongitude* destinationLatLong, const byte* data, uint32 dataSize, nb_boolean full)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_ROUTE_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        char routeId[MAX_FILE_NAME_LEN] = { 0 };

        HexlifyRouteId(data, dataSize, MAX_FILE_NAME_LEN, routeId);

        err = CSL_QaRecordWriteDouble(record, originLatLong->latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLatLong->longitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, destinationLatLong->latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, destinationLatLong->longitude);
        err = err ? err : CSL_QaRecordWriteText(record, routeId, MAX_FILE_NAME_LEN);
        err = err ? err : CSL_QaRecordWriteUint8(record, full);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void 
NB_QaLogNavigationSessionStart(NB_Context* context, uint32 sessionId)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_SESSION_START, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, sessionId);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void 
NB_QaLogNavigationSessionRoute(NB_Context* context, uint32 sessionId, const byte* data, uint32 dataSize)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_SESSION_ROUTE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        char routeId[MAX_FILE_NAME_LEN] = { 0 };

        HexlifyRouteId(data, dataSize, MAX_FILE_NAME_LEN, routeId);

        err = CSL_QaRecordWriteUint32(record, sessionId);
        err = err ? err : CSL_QaRecordWriteText(record, routeId, MAX_FILE_NAME_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void 
NB_QaLogNavigationSessionRecalc(NB_Context* context, uint32 sessionId, uint8 reason)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_SESSION_RECALC, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, sessionId);
        err = err ? err : CSL_QaRecordWriteUint8(record, reason);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void 
NB_QaLogNavigationSessionEnd(NB_Context* context, uint32 sessionId)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_SESSION_END, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, sessionId);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogPOIAlongRouteSearch(NB_Context* context, double startLatitude, double startLongitude, 
                            double distance, double width, const char* name, const char* scheme, 
                            const char** categories, uint32 numberOfCategories, 
                            uint32 sliceSize)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    uint32 i;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SEARCH_ALONG_ROUTE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {

        err = CSL_QaRecordWriteDouble(record, startLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, startLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, width);

        err = err ? err : CSL_QaRecordWriteText(record, name, SEARCH_NAME_LEN);
        err = err ? err : CSL_QaRecordWriteText(record, scheme, SEARCH_SCHEME_LEN);

        err = err ? err : CSL_QaRecordWriteUint32(record, sliceSize);

        for (i=0; i<SEARCH_MAX_CATEGORIES; i++)
        {
            if (i < numberOfCategories)
            {
                err = err ? err : CSL_QaRecordWriteText(record, categories[i], SEARCH_CATEGORY_CODE_LENGTH);
            }
            else
            {
                err = err ? err : CSL_QaRecordWriteText(record, "", SEARCH_CATEGORY_CODE_LENGTH);
            }
        }

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}

NB_DEF void 
NB_QaLogNavigationStartupRegion(NB_Context* context, double centerLatitude, double centerLongitude, double radius, double distance)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_STARTUP_REGION, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {

        err = CSL_QaRecordWriteDouble(record, centerLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, centerLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, radius);
        err = err ? err : CSL_QaRecordWriteDouble(record, distance);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    
}

NB_DEF void 
NB_QaLogPronunFileDownload(NB_Context* context, const char* codeData, nb_size codeSize)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    char pronunCode[PRONUN_CODE_LENGTH];

    nb_size copyLength = codeSize;
    if (copyLength > PRONUN_CODE_LENGTH)
    {
        copyLength = PRONUN_CODE_LENGTH;  // copy at most PRONUN_CODE_LENGTH bytes
    }

    nsl_memset(pronunCode, 0, sizeof(pronunCode));
    nsl_memcpy(pronunCode, codeData, copyLength);

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_PRONUN_FILE_DOWNLOAD, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    { 

        err = CSL_QaRecordWriteText(record, pronunCode, PRONUN_CODE_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }    

}

NB_DEF void
NB_QaLogGuidanceMessageTrigger(NB_Context* context, NB_NavigateAnnouncementSource source,
                               NB_GuidanceMessageType type, double triggerDistance, 
                               double currentDistance, double currentSpeed, 
                               double totalManeuverDistance)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_TRIGGERED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {

        err = CSL_QaRecordWriteUint8(record, (uint8)source);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)type);
        err = err ? err : CSL_QaRecordWriteDouble(record, triggerDistance);
        err = err ? err : CSL_QaRecordWriteDouble(record, currentDistance);
        err = err ? err : CSL_QaRecordWriteDouble(record, currentSpeed);
        err = err ? err : CSL_QaRecordWriteDouble(record, totalManeuverDistance);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }        
}

static NB_Error
NB_QaLogWriteGuidanceMessage(NB_Context* context, CSL_QaRecord* record, NB_GuidanceMessage* message)
{
    NB_Error error = NE_OK;
//    int codeIndex;
//    int numberOfCodes = NB_GuidanceMessageGetCodeCount(message);
//    const char* code;
//
//    error = CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetSource(message));
//    error = error ? error : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetType(message));
//
//    for (codeIndex = 0; codeIndex < QALOG_MAX_PRONUN_CODES; codeIndex++)
//    {
//        if (codeIndex < numberOfCodes)
//        {
//            code = NB_GuidanceMessageGetCode(message, codeIndex);
//        }
//        else
//        {
//            code = "";
//        }
//
//        if (code != NULL)
//        {
//            error = error ? error : CSL_QaRecordWriteText(record, code, PRONUN_CODE_LENGTH);
//        }
//    }

    return error;
}

NB_DEF void 
NB_QaLogGuidanceMessageRetrieved(NB_Context* context, NB_GuidanceMessage* message)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_RETRIEVED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = NB_QaLogWriteGuidanceMessage(context, record, message);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }        
}

NB_DEF void NB_QaLogGuidanceMessagePlayed(NB_Context* context, NB_GuidanceMessage* message)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_PLAYED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = NB_QaLogWriteGuidanceMessage(context, record, message);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogCellIdRequest(NB_Context* context, NB_QaLogLocationNetworkType type, int16 signalStrengthDbm,
                      uint32 mobileCountryCodeOrSystemId, uint32 mobileNetworkCodeOrNetworkId,
                      uint32 locationAreaCode, uint32 cellId)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CELLID_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)type);
        err = err ? err : CSL_QaRecordWriteInt32(record, (int32)signalStrengthDbm);
        err = err ? err : CSL_QaRecordWriteUint32(record, mobileCountryCodeOrSystemId);
        err = err ? err : CSL_QaRecordWriteUint32(record, mobileNetworkCodeOrNetworkId);
        err = err ? err : CSL_QaRecordWriteUint32(record, locationAreaCode);
        err = err ? err : CSL_QaRecordWriteUint32(record, cellId);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogCellIdWifiRequest(NB_Context* context, int16 signalStrengthDbm, const char* macAddress)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CELLID_WIFI_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record, NB_QLLNT_WiFi);
        err = err ? err : CSL_QaRecordWriteInt32(record, (int32)signalStrengthDbm);
        err = err ? err : CSL_QaRecordWriteText(record, macAddress, (uint32)nsl_strlen(macAddress));

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogCellIdReply(NB_Context* context, double latitude, double longitude, uint32 accuracyMeters, uint32 statusCode)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CELLID_REPLY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);
        err = err ? err : CSL_QaRecordWriteUint32(record, accuracyMeters);
        err = err ? err : CSL_QaRecordWriteUint32(record, statusCode);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogKey(NB_Context* context, uint16 keyCode)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_KEY, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint16(record, keyCode);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogCamera(NB_Context* context, NB_Camera* camera)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = NULL;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CAMERA, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = AddPlaceToRecord(record, &camera->place, "safety-camera");
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->speedLimit);
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->heading);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)camera->isBidirectional);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)camera->status);
        err = err ? err : CSL_QaRecordWriteText(record, camera->description, sizeof(camera->description));
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->routeDistance);
        err = err ? err : CSL_QaRecordWriteUint32(record, camera->nman_closest);
        err = err ? err : CSL_QaRecordWriteUint32(record, camera->nseg_closest);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogCameraWarning(NB_Context* context, double distanceToCamera, NB_Camera* camera)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = NULL;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CAMERA_WARNING, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        /* camera */
        err = AddPlaceToRecord(record, &camera->place, "safety-camera");
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->distance);
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->speedLimit);
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->heading);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)camera->isBidirectional);
        err = err ? err : CSL_QaRecordWriteUint32(record, (uint32)camera->status);
        err = err ? err : CSL_QaRecordWriteText(record, camera->description, sizeof(camera->description));
        err = err ? err : CSL_QaRecordWriteDouble(record, camera->routeDistance);
        err = err ? err : CSL_QaRecordWriteUint32(record, camera->nman_closest);
        err = err ? err : CSL_QaRecordWriteUint32(record, camera->nseg_closest);

        /* distance to camera */
        err = err ? err : CSL_QaRecordWriteDouble(record, distanceToCamera);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogTrafficEvent(NB_Context* context,  NB_QaLogTrafficEventListOperation operation, NB_TrafficEvent* trafficEvent)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = NULL;

    int32 severity = -1;
    uint32 updateTime = 0;
    const char* road = "";

    double congestionLength = -1.0;
    double congestionSpeed = INVALID_SPEED;
    
    int32 incidentCategory = -1;
    double incidentLatitude = INVALID_LATLON;
    double incidentLongitude = INVALID_LATLON;
    const char* incidentDescription = "";

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TRAFFIC_EVENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        switch (trafficEvent->type)
        {
        case NB_TET_Congestion:
            severity = (int32)trafficEvent->detail.congestion.severity;
            updateTime = trafficEvent->detail.congestion.updateGpsTimeSeconds;
            road = trafficEvent->detail.congestion.road;
            congestionLength = trafficEvent->detail.congestion.length;
            congestionSpeed = trafficEvent->detail.congestion.speed;
            break;
        case NB_TET_Incident:
            severity = (int32)trafficEvent->detail.incident.severity;
            updateTime = trafficEvent->detail.incident.entry_time;
            road = trafficEvent->detail.incident.road;
            incidentCategory = trafficEvent->detail.incident.type;
            incidentLatitude = trafficEvent->place.location.latitude;
            incidentLongitude = trafficEvent->place.location.longitude;
            incidentDescription = trafficEvent->detail.incident.description;
            break;
        case NB_TET_None:
            break;
        }

        /* common/similar fields */
        err = CSL_QaRecordWriteUint32(record, operation);
        err = err ? err : CSL_QaRecordWriteUint32(record, trafficEvent->type);
        err = err ? err : CSL_QaRecordWriteInt32(record, severity);
        err = err ? err : CSL_QaRecordWriteText(record, road, sizeof(trafficEvent->detail.incident.road));
        err = err ? err : CSL_QaRecordWriteUint32(record, trafficEvent->maneuverIndex);
        err = err ? err : CSL_QaRecordWriteDouble(record, trafficEvent->distanceToEvent);
        err = err ? err : CSL_QaRecordWriteDouble(record, trafficEvent->routeRemainAfterEvent);
        err = err ? err : CSL_QaRecordWriteUint32(record, updateTime);

        /* congestion */
        err = err ? err : CSL_QaRecordWriteDouble(record, congestionLength);
        err = err ? err : CSL_QaRecordWriteDouble(record, congestionSpeed);

        /* incident */
        err = err ? err : CSL_QaRecordWriteInt32(record, incidentCategory);
        err = err ? err : CSL_QaRecordWriteDouble(record, incidentLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, incidentLongitude);
        err = err ? err : CSL_QaRecordWriteText(record, incidentDescription, sizeof(trafficEvent->detail.incident.description));

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogApp2AppCommand(NB_Context* context, const char* command)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_APP2APP_COMMAND, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, command, APP2APP_COMMAND_LENGTH);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogClientSessionId(NB_Context* context, uint32 clientSessionId)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_CLIENT_SESSION_ID, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint32(record, clientSessionId);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogNbiWifiProbes(NB_Context* context, double latitude, double longitude,
                      unsigned int locationTimestamp,
                      unsigned char numNetworks, 
                      const unsigned char* macAddress,
                      const unsigned char* ssid,
                      int rssi, unsigned int timedelta)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    
    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NBI_WIFI_PROBES_EVENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        char* macBuf = (char*)nsl_malloc(sizeof(char) * 32);
        if (macBuf == NULL)
        {
            CSL_QaRecordDestroy(record);
            return;
        }
        nsl_sprintf(macBuf, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", macAddress[0], macAddress[1],
            macAddress[2], macAddress[3], macAddress[4], macAddress[5]);

        err = CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);
        err = err ? err : CSL_QaRecordWriteUint32(record, locationTimestamp);
        err = err ? err : CSL_QaRecordWriteUint8(record, numNetworks);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)macBuf, 32);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)ssid, 30);
        err = err ? err : CSL_QaRecordWriteInt32(record, rssi);
        err = err ? err : CSL_QaRecordWriteUint32(record, timedelta);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
        nsl_free(macBuf);
    }  
}

NB_DEF void
NB_QaLogMetadataSourceRequest(NB_Context* context, const char* language, const int wantedContentCount,
                              const nb_boolean wantExtendedMaps, const uint32 screenWidth, const uint32 screenHeight,
                              const uint32 screenResolution)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_METADATA_SOURCE_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)language, 8);
        err = err ? err : CSL_QaRecordWriteInt32(record, wantedContentCount);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(wantExtendedMaps ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteInt32(record, screenWidth);
        err = err ? err : CSL_QaRecordWriteInt32(record, screenHeight);
        err = err ? err : CSL_QaRecordWriteInt32(record, screenResolution);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogWantedContent(NB_Context* context, const unsigned char requestType, const char* contentType, const char* country)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_WANTED_CONTENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        uint8  contentTypeByte = ConvertContentTypeString(contentType);
        err = CSL_QaRecordWriteUint8(record, requestType);
        err = err ? err : CSL_QaRecordWriteUint8(record, contentTypeByte);
        err = err ? err : CSL_QaRecordWriteText(record, country, 3);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogMetadataSourceResponse(NB_Context* context, const char* citySummary, const char* baseUrl,
                               const char* projectionType, const int templateCount)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_METADATA_SOURCE_RESPONSE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        uint8 projectionTypeByte = ConvertProjectionTypeString(projectionType);
        err = CSL_QaRecordWriteText(record, (const char*)citySummary, 40);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)baseUrl, 200);
        err = err ? err : CSL_QaRecordWriteUint8(record, projectionTypeByte);
        err = err ? err : CSL_QaRecordWriteInt32(record, templateCount);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogMetadataSourceTemplate(NB_Context* context, const char* templateType, const char* templateString, const uint32 cacheId,
            const char* metadataId, const char* manifestVersion, const uint32 manifestSize, const char* displayName,
            const double boxLatitude1, const double boxLongitude1, const double boxLatitude2, const double boxLongitude2,
            const char* projectionDatum, const double projectionFalseEasting, const double projectionFalseNorthing,
            const double originLatitude, const double originLongitude, const double scaleFactor, const double zOffset)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_METADATA_SOURCE_TEMPLATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        uint8 templateTypeByte = ConvertContentTypeString(templateType);
        uint8 projectionDatumByte = ConvertProjectionDatumString(projectionDatum);
        err = CSL_QaRecordWriteUint8(record, templateTypeByte);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)templateString, 80);
        err = err ? err : CSL_QaRecordWriteInt32(record, cacheId);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)metadataId, 20);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)manifestVersion, 10);
        err = err ? err : CSL_QaRecordWriteInt32(record, manifestSize);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)displayName, 30);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude2);
        err = err ? err : CSL_QaRecordWriteUint8(record, projectionDatumByte);
        err = err ? err : CSL_QaRecordWriteDouble(record, projectionFalseEasting);
        err = err ? err : CSL_QaRecordWriteDouble(record, projectionFalseNorthing);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, scaleFactor);
        err = err ? err : CSL_QaRecordWriteDouble(record, zOffset);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogManifestRequest(NB_Context* context, const int wantedContentCount)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MANIFEST_REQUEST, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteInt32(record, wantedContentCount);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogManifestResponse(NB_Context* context, const char* datasetId, const int tileContentCount,
            const char* firstTileId, const double firstTileBoxLatitude1, const double firstTileBoxLongitude1,
            const double firstTileBoxLatitude2, const double firstTileBoxLongitude2,
            const char* lastTileId, const double lastTileBoxLatitude1, const double lastTileBoxLongitude1,
            const double lastTileBoxLatitude2, const double lastTileBoxLongitude2)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MANIFEST_RESPONSE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)datasetId, 20);
        err = err ? err : CSL_QaRecordWriteInt32(record, tileContentCount);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)firstTileId, 20);
        err = err ? err : CSL_QaRecordWriteDouble(record, firstTileBoxLatitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, firstTileBoxLongitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, firstTileBoxLatitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, firstTileBoxLongitude2);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)lastTileId, 20);
        err = err ? err : CSL_QaRecordWriteDouble(record, lastTileBoxLatitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, lastTileBoxLongitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, lastTileBoxLatitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, lastTileBoxLongitude2);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogManifestResponseContent(NB_Context* context, const char* contentFieldId, const char* contentFileVersion,
            const double boxLatitude1, const double boxLongitude1, const double boxLatitude2, const double boxLongitude2)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_MANIFEST_RESPONSE_CONTENT, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)contentFieldId, 20);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)contentFileVersion, 10);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude1);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLatitude2);
        err = err ? err : CSL_QaRecordWriteDouble(record, boxLongitude2);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogRealisticSignInfo(NB_Context* context, const char* sarId, const char* sarType, const char* datasetId,
            const char* version,const int maneuver)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SAR_INFO, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)sarId, 20);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)sarType, 10);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)datasetId, 20);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)version, 10);
        err = err ? err : CSL_QaRecordWriteInt32(record, maneuver);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogRealisticSignState(NB_Context* context, const char* sarId, const unsigned char sarState, const double latitude, const double longitude)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SAR_STATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)sarId, 20);
        err = err ? err : CSL_QaRecordWriteUint8(record, sarState);
        err = err ? err : CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

/* See header for description */
NB_DEF void 
NB_QaLogEnhancedContentState(NB_Context* context, 
                             const char* enhancedContentId,
                             NB_QaLogEnhancedContentType contentType,
                             NB_QaLogEnhancedContentRegionState state,
                             double latitude,
                             double longitude)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_EC_STATE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, enhancedContentId, 20);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)contentType);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state);
        err = err ? err : CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, longitude);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogEnhancedContentInfo(NB_Context* context, const char* ecId, const unsigned char ecType, const char* version, const char* datasetId,
            const int startManeuver, const double maneuverOffset, const int endManeuver, const double endManeuverOffset,
            const unsigned char projectionType, const unsigned char projectionDatum, const double projectionFalseEasting, const double projectionFalseNorthing,
            const double originLatitude, const double originLongitude, const double scaleFactor, const double zOffset)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_EC_INFO, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)ecId, 20);
        err = err ? err : CSL_QaRecordWriteUint8(record, ecType);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)version, 10);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)datasetId, 10);
        err = err ? err : CSL_QaRecordWriteInt32(record, startManeuver);
        err = err ? err : CSL_QaRecordWriteDouble(record, maneuverOffset);
        err = err ? err : CSL_QaRecordWriteInt32(record, endManeuver);
        err = err ? err : CSL_QaRecordWriteDouble(record, endManeuverOffset);
        err = err ? err : CSL_QaRecordWriteUint8(record, projectionType);
        err = err ? err : CSL_QaRecordWriteUint8(record, projectionDatum);
        err = err ? err : CSL_QaRecordWriteDouble(record, projectionFalseEasting);
        err = err ? err : CSL_QaRecordWriteDouble(record, projectionFalseNorthing);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, originLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, scaleFactor);
        err = err ? err : CSL_QaRecordWriteDouble(record, zOffset);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogEnhancedContentDownload(NB_Context* context, const char* ecId,
            const unsigned char ecType,
            const unsigned char queueType,
            const char* version,
            const int size,
            const char* url)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_EC_DOWNLOAD2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)ecId, 20);
        err = err ? err : CSL_QaRecordWriteUint8(record, ecType);
        err = err ? err : CSL_QaRecordWriteUint8(record, queueType);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)version, 10);
        err = err ? err : CSL_QaRecordWriteInt32(record, size);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)url, 200);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogEnhancedContentDownloadResult(NB_Context* context, const char* ecId,
            const unsigned char ecType,
            const unsigned char queueType,
            const char* version,
            const int size,
            const unsigned char status)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_EC_DOWNLOAD_RESULT2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, (const char*)ecId, 20);
        err = err ? err : CSL_QaRecordWriteUint8(record, ecType);
        err = err ? err : CSL_QaRecordWriteUint8(record, queueType);
        err = err ? err : CSL_QaRecordWriteText(record, (const char*)version, 10);
        err = err ? err : CSL_QaRecordWriteInt32(record, size);
        err = err ? err : CSL_QaRecordWriteUint8(record, status);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogApiCall(NB_Context* context, NB_QaLogApiCallRecordType type, const char* function, const char* message)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = NULL;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NBI_API_CALL,
        1 + API_CALL_FUNCTION_LENGTH + API_CALL_MESSAGE_LENGTH, &record);

    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)type);
        err = err ? err : CSL_QaRecordWriteText(record, function, API_CALL_FUNCTION_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, message, API_CALL_MESSAGE_LENGTH);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogLocationEvent (NB_Context* context, NB_QaLogLocationEventType eventID)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = NULL;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NBI_LOCATION_EVENT, LKEVENT_BUFFER_SIZE, &record);

    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteUint8(record, (unsigned char)eventID);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF NB_Error
NB_QaLogLocationRequestByCellID (NB_Context* context, int sid, int nid, int bid, int rssi)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;
    unsigned char emptyMAC[MAC_LENGTH] = {0};
    CSL_QaRecord* record = 0;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NBI_NETWORK_LOCATION_REQUEST,
        QARECORD_LKREQUEST_BUFFER_SIZE, &record);

    if (!err)
    {
        // Write CellID-specific data.
        err = CSL_QaRecordWriteInt32(record, sid);
        err = err ? err : CSL_QaRecordWriteInt32(record, nid);
        err = err ? err : CSL_QaRecordWriteInt32(record, bid);
        err = err ? err : CSL_QaRecordWriteInt32(record, rssi);

        // Write zero values for Wifi-specific data.
        // Number of networks
        err = err ? err : CSL_QaRecordWriteUint8(record, 0);
        // MAC Adress
        err = err ? err : CSL_QaRecordWriteData(record, emptyMAC, 6);
        // RSSI
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);
        // timestamp
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF NB_Error
NB_QaLogLocationRequestByWiFi (NB_Context* context, unsigned char numNetworks,
                               const unsigned char* macAddress, int rssi, int timestamp)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;

    CSL_QaRecord* record = NULL;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NBI_NETWORK_LOCATION_REQUEST,
        QARECORD_LKREQUEST_BUFFER_SIZE, &record);

    if (!err)
    {
        // Write zero values for CellID-specific data.
        // SID
        err = CSL_QaRecordWriteInt32(record, 0);
        // NID
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);
        // Base station ID
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);
        // RSSI
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);

        // Write Wifi-specific data.
        // Number of networks
        err = err ? err : CSL_QaRecordWriteUint8(record, numNetworks);
        // MAC address
        err = err ? err : CSL_QaRecordWriteData(record, macAddress, 6);
        // RSSI
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);
        // timestamp
        err = err ? err : CSL_QaRecordWriteInt32(record, 0);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF NB_Error
NB_QaLogLocationReply(NB_Context* context, double latitude, double longitude,
                      unsigned short errorRadius, NB_QaLogLocationSourceType source)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;

    CSL_QaRecord* record = 0;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NBI_NETWORK_LOCATION_REPLY,
        QARECORD_LKREPLY_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteDouble(record, longitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, latitude);
        err = err ? err : CSL_QaRecordWriteUint16(record, errorRadius);
        err = err ? err : CSL_QaRecordWriteUint8(record, (unsigned char)source);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF NB_Error
NB_QaLogKpiSessionOpen(NB_Context* context,
                        const char* bundleId,
                        const char* applicationId,
                        const char* deviceId)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }

    NB_Error err = NE_OK;

    CSL_QaRecord* record = 0;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_KPI_SESSION_OPEN,
        INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteText(record, bundleId, KPI_SESSION_BUNDLE_ID_MAX_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, applicationId, KPI_SESSION_APPLICATION_ID_MAX_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, deviceId, KPI_SESSION_DEVICE_ID_MAX_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF NB_Error
NB_QaLogKpiSessionClose(NB_Context* context,
                        const char* bundleId,
                        const char* applicationId,
                        const char* deviceId)
{
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }

    NB_Error err = NE_OK;

    CSL_QaRecord* record = 0;

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_KPI_SESSION_CLOSE,
        INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteText(record, bundleId, KPI_SESSION_BUNDLE_ID_MAX_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, applicationId, KPI_SESSION_APPLICATION_ID_MAX_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, deviceId, KPI_SESSION_DEVICE_ID_MAX_LENGTH);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF NB_Error
NB_QaLogKpiFreeformEvent(NB_Context* context,
                        const char* eventLabel,
                        const char* data)
{
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }

    NB_Error err = NE_OK;

    CSL_QaRecord* record = 0;

    if (CSL_QaLogKpiFreeformEventFilterIsDisabled(NB_ContextGetQaLog(context), eventLabel))
    {
        return NE_OK; // log is disabled, just exit
    }

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_KPI_FREEFORM_EVENT,
        INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        uint32 dataLen = (uint32)nsl_strlen(data);
        err = CSL_QaRecordWriteText(record, eventLabel, KPI_FREEFORM_EVENT_LABEL_MAX_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, dataLen);
        err = err ? err : CSL_QaRecordWriteData(record, (const byte*)data, dataLen);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF uint32
NB_QaLogKpiIntervalBegin(NB_Context* context,
        const char* eventLabel)

{
    char buf[32] = {0};
    uint32 intervalID = 0;

    if (!context || !eventLabel)
    {
        return 0;
    }

    intervalID = CSL_QaLogKpiIidAllocate(NB_ContextGetQaLog(context));

    nsl_snprintf(buf, sizeof(buf), "intervalID=%d", intervalID);
    NB_QaLogKpiFreeformEvent(context, eventLabel, buf);

    return intervalID;
}


NB_DEF void
NB_QaLogKpiIntervalEnd(NB_Context* context,
        uint32 intervalID,
        const char* eventLabel)
{
    char buf[32] = {0};
    if (!context || !eventLabel)
    {
        return;
    }

    nsl_snprintf(buf, sizeof(buf), "intervalID=%d", intervalID);
    NB_QaLogKpiFreeformEvent(context, eventLabel, buf);
    CSL_QaLogKpiIidRelease(NB_ContextGetQaLog(context), intervalID);
}


NB_DEF NB_Error
NB_QaLogFilterSet(NB_Context* context,
        uint16 id,
        nb_boolean enable)
{
    if (!context)
    {
        return NE_INVAL;
    }

    return CSL_QaLogFilterSet(NB_ContextGetQaLog(context), id, enable);
}


NB_DEF NB_Error
NB_QaLogFilterSetAll(NB_Context* context,
        nb_boolean enable)
{
    if (!context)
    {
        return NE_INVAL;
    }

    return CSL_QaLogFilterSetAll(NB_ContextGetQaLog(context), enable);
}


NB_DEF NB_Error
NB_QaLogKpiFreeformEventFilterSet(NB_Context* context,
        const char* eventLabel,
        nb_boolean enable)
{
    if (!context || !eventLabel)
    {
        return NE_INVAL;
    }

    return CSL_QaLogKpiFreeformEventFilterSet(NB_ContextGetQaLog(context), eventLabel, enable);
}


NB_DEF void
NB_QaLogKpiFreeformEventFilterSetAll(NB_Context* context,
        nb_boolean enable)
{
    if (!context)
    {
        return;
    }

    CSL_QaLogKpiFreeformEventFilterSetAll(NB_ContextGetQaLog(context), enable);
}


NB_DEF NB_Error
NB_QaLogTileDownloadSuccess(NB_Context* context,
                            uint32 x,
                            uint32 y,
                            uint32 zoomLevel,
                            const char* layerId,
                            uint32 tileSize,
                            double downloadTime)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    NB_ASSERT_VALID_THREAD(context);
    if (layerId == NULL)
    {
        return NE_INVAL;
    }

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_DOWNLOAD_SUCCESS,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, x);
        err = err ? err : CSL_QaRecordWriteUint32(record, y);
        err = err ? err : CSL_QaRecordWriteUint32(record, zoomLevel);
        err = err ? err : CSL_QaRecordWriteText(record,  layerId, NB_MAP_LAYERID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, tileSize);
        err = err ? err : CSL_QaRecordWriteDouble(record, downloadTime);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
    return err;
}

NB_DEF NB_Error
NB_QaLogTileDownloadFailure(NB_Context* context,
                            uint32 x,
                            uint32 y,
                            uint32 zoomLevel,
                            const char* layerId,
                            uint32 errorCode)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    NB_ASSERT_VALID_THREAD(context);
    if (layerId == NULL)
    {
        return NE_INVAL;
    }

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_TILE_DOWNLOAD_FAILURE,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, x);
        err = err ? err : CSL_QaRecordWriteUint32(record, y);
        err = err ? err : CSL_QaRecordWriteUint32(record, zoomLevel);
        err = err ? err : CSL_QaRecordWriteText(record,  layerId, NB_MAP_LAYERID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, errorCode);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
    return err;
}

NB_DEF NB_Error
NB_QaLogUnifiedMapConfigInitializationFailure(NB_Context* context,
                                              unsigned char failureType,
                                              uint32 retriedTime,
                                              uint32 exceptionNumber
                                              )
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(),
                             RECORD_ID_UNIFIED_MAP_CONFIG_INITIALIZATION_FAILURE,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteUint8(record, failureType);
        err = err ? err : CSL_QaRecordWriteUint32(record, retriedTime);
        err = err ? err : CSL_QaRecordWriteUint32(record, exceptionNumber);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF NB_Error
NB_QaLogPrefetchWindow(NB_Context* context, double startLatitude,
                       double startLongitude, double endLatitude,
                       double endLongitude, double startDistance, double endDistance)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(),
                             RECORD_ID_PREFETCH_WINDOW,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = CSL_QaRecordWriteDouble(record, startLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, startLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, endLatitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, endLongitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, startDistance);
        err = err ? err : CSL_QaRecordWriteDouble(record, endDistance);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

    return err;
}

NB_DEF void
NB_QaLogPerformance(const NB_Context* context, const char* msg, uint32 timeMs)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error err = NE_OK;
    CSL_QaRecord* record = NULL;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_PERFORMANCE,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteText(record, msg, PERFORMANCE_MSG_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint32(record, timeMs);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog((NB_Context*)context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogLayerAvailabilityMatrixCheckResult(NB_Context* context,
                                           int32 x,
                                           int32 y,
                                           const char* layerId,
                                           nb_boolean hasData)
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }
    
    NB_Error      err    = NE_OK;
    CSL_QaRecord* record = NULL;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_LAYER_AVAILABLITY_MATRIX_CHECK,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteInt32(record, x);
        err = err ? err : CSL_QaRecordWriteInt32(record, y);
        err = err ? err : CSL_QaRecordWriteText(record, layerId, NB_MAP_LAYERID_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(hasData ? 1 : 0));
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog((NB_Context*)context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogDAMCheckResult(NB_Context* context,/*!< NB_Context instance pointer */
                       int32 x,            /*!< x coordinate of tile */
                       int32 y,            /*!< y coordinate of tile */
                       uint8 bit,          /*!< bitset that DAM checked */
                       nb_boolean hasData  /*!< If this layer has data or not */
                       )
{
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_Error      err    = NE_OK;
    CSL_QaRecord* record = NULL;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_DAM_CHECK,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = CSL_QaRecordWriteInt32(record, x);
        err = err ? err : CSL_QaRecordWriteInt32(record, y);
        err = err ? err : CSL_QaRecordWriteUint8(record, bit);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(hasData ? 1 : 0));
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog((NB_Context*)context), record);

        CSL_QaRecordDestroy(record);
    }
}

// Local Functions ..................................................................................................

static NB_Error
AddPlaceToRecord(CSL_QaRecord* record, NB_Place* place, const char* label)
{
    NB_Error err = NE_OK;
    int i = 0;
    
    err = CSL_QaRecordWriteText(record, label, PLACE_LABEL_LEN);
    err = err ? err : CSL_QaRecordWriteText(record, place->name, sizeof(place->name));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.areaname, sizeof(place->location.areaname));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.streetnum, sizeof(place->location.streetnum));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.street1, sizeof(place->location.street1));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.street2, sizeof(place->location.street2));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.city, sizeof(place->location.city));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.county, sizeof(place->location.county));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.state, sizeof(place->location.state));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.postal, sizeof(place->location.postal));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.country, sizeof(place->location.country));
    err = err ? err : CSL_QaRecordWriteText(record, place->location.airport, sizeof(place->location.airport));
    err = err ? err : CSL_QaRecordWriteDouble(record, place->location.latitude);
    err = err ? err : CSL_QaRecordWriteDouble(record, place->location.longitude);
    err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)place->location.type);
    
    for (i = 0; !err && i < NB_PLACE_NUM_PHONE; i++)
    {
        err = CSL_QaRecordWriteUint8(record, (uint8)place->phone[i].type);
        err = err ? err : CSL_QaRecordWriteText(record, place->phone[i].country, sizeof(place->phone[0].country));
        err = err ? err : CSL_QaRecordWriteText(record, place->phone[i].area, sizeof(place->phone[0].area));
        err = err ? err : CSL_QaRecordWriteText(record, place->phone[i].number, sizeof(place->phone[0].number));
    }
    err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)place->numphone);
    
    for (i = 0; !err && i < NB_PLACE_NUM_CAT; i++)
    {
        err = CSL_QaRecordWriteText(record, place->category[i].code, sizeof(place->category[0].code));
        err = err ? err : CSL_QaRecordWriteText(record, place->category[i].name, sizeof(place->category[0].name));
    }
    err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)place->numcategory);
    
    
    return err;
}

static void
LogGPSFix(NB_Context* context, uint16 recordId, NB_GpsLocation* location)
{
    NB_Error err = NE_OK;
    
    CSL_QaRecord* record = 0;
    
    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), recordId, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = CSL_QaRecordWriteUint32(record, location->gpsTime);
        err = err ? err : CSL_QaRecordWriteUint32(record, location->status);
        err = err ? err : CSL_QaRecordWriteInt32(record, (int32) (location->latitude / 0.00000536441803));
        err = err ? err : CSL_QaRecordWriteInt32(record, (int32) (location->longitude / 0.00000536441803));
        err = err ? err : CSL_QaRecordWriteUint16(record, (uint16) (location->altitude + 500.0));
        err = err ? err : CSL_QaRecordWriteUint16(record, (uint16) ((location->valid & NGV_Heading) ? (location->heading / 0.3515625) : (0xFFFF)));
        err = err ? err : CSL_QaRecordWriteUint16(record, (uint16) (location->horizontalVelocity * 4.0));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8) (location->verticalVelocity * 2.0));
        err = err ? err : CSL_QaRecordWriteUint8(record, 0);  /* AEEGPSAccuracy in BREW -- not available here */
        err = err ? err : CSL_QaRecordWriteUint16(record, (uint16) (location->valid)); /* use least significant 16 bits to maintain compatibility */
        err = err ? err : CSL_QaRecordWriteUint8(record, (location->valid & NGV_AxisUncertainty) ? uncencode(location->horizontalUncertaintyAlongAxis) : 0xFF);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8) ((location->valid & NGV_HorizontalUncertainty) ? (location->horizontalUncertaintyAngleOfAxis / 5.625) : 0xFF));
        err = err ? err : CSL_QaRecordWriteUint8(record, (location->valid & NGV_PerpendicularUncertainty) ? uncencode(location->horizontalUncertaintyAlongPerpendicular) : 0xFF);
        err = err ? err : CSL_QaRecordWriteUint16(record, (uint16) location->verticalUncertainty);
        
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        
        CSL_QaRecordDestroy(record);
    }
}

static void
HexlifyRouteId(const byte* data, uint32 dataSize, int hexBufferSize, char* hexBuffer)
{
    // The most bytes that can be hexlify'd is half the buffer size
    int length = hexBufferSize / 2;
    if ((int)dataSize < length)
    {
        length = dataSize;
    }
    
    hexlify(length, (const char*)data, hexBuffer);
}

static uint8
ConvertContentTypeString(const char* contentType)
{
    uint8 contentTypeByte = NB_QLECT_Undefined;

    if (nsl_strcmp(contentType, DATASET_TYPE_CITY_MODEL) == 0)
    {
        contentTypeByte = NB_QLECT_CityModels;
    }
    else if (nsl_strcmp(contentType, DATASET_TYPE_MOTORWAY_JUNCTION_OBJECT) == 0)
    {
        contentTypeByte = NB_QLECT_Junctions;
    }
    else if (nsl_strcmp(contentType, DATASET_TYPE_REALISTIC_SIGN) == 0)
    {
        contentTypeByte = NB_QLECT_RealisticSigns;
    }
    else if (nsl_strcmp(contentType, DATASET_TYPE_SPECIFIC_TEXTURE) == 0)
    {
        contentTypeByte = NB_QLECT_Textures;
    }
    else if (nsl_strcmp(contentType, DATASET_TYPE_COMMON_TEXTURE) == 0)
    {
        contentTypeByte = NB_QLECT_CommonTextures;
    }

    return contentTypeByte;
}

static uint8
ConvertProjectionTypeString(const char* projectionType)
{
    uint8 projectionTypeByte = NB_QLPT_Undefined;

    if (nsl_strcmp(projectionType, PROJECTION_TYPE_MERCATOR) == 0)
    {
        projectionTypeByte = NB_QLPT_Mercator;
    }
    else if (nsl_strcmp(projectionType, PROJECTION_TYPE_TRANSVERSE_MERCATOR) == 0)
    {
        projectionTypeByte = NB_QLPT_TransverseMercator;
    }
    else if (nsl_strcmp(projectionType, PROJECTION_TYPE_UNIVERSAL_TRANSVERSE_MERCATOR) == 0)
    {
        projectionTypeByte = NB_QLPT_UniversalTransverseMercator;
    }

    return projectionTypeByte;
}

static uint8
ConvertProjectionDatumString(const char* projectionDatum)
{
    uint8 projectionDatumByte = NB_QLPD_Undefined;

    if (nsl_strcmp(projectionDatum, PROJECTION_DATUM_WGS84) == 0)
    {
        projectionDatumByte = NB_QLPD_WGS84;
    }
    else if (nsl_strcmp(projectionDatum, PROJECTION_DATUM_D_NORTH_AMERICAN_1983) == 0)
    {
        projectionDatumByte = NB_QLPD_NAD83;
    }

    return projectionDatumByte;
}

NB_DEF void
NB_QaLogSpeedLimitSetting(NB_Context* context,
                          NB_QaLogSpeedLimitInformation* info
                          )
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    NB_ASSERT_VALID_THREAD(context);
    if (info == NULL)
    {
        return;
    }

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SPEED_LIMIT_INFO,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_speedLimitEnable ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_displayEnable ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_speedAlertenable ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_speedWarningSoundType));
        err = err ? err : CSL_QaRecordWriteDouble(record, info->m_speedWarningRange);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_zoneAlertEnable ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_zoneWarningSoundType));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(info->m_debugEnable ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteUint32(record, info->m_debugOffset);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }

}

NB_DEF void
NB_QaLogSpeedLimitState(NB_Context* context,
                        NB_QaLogSpeedLimitStateData* state
                        )
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    NB_ASSERT_VALID_THREAD(context);
    if (state == NULL) {
        return;
    }

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_SPEED_LIMIT_STATE,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)state->m_warnType);
        err = err ? err : CSL_QaRecordWriteText(record,  state->m_signId, NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->m_latitude);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->m_longitude);
        err = err ? err : CSL_QaRecordWriteUint32(record, state->m_maneuverId);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->m_offset);
        err = err ? err : CSL_QaRecordWriteDouble(record, state->m_speedLimit);
        err = err ? err : CSL_QaRecordWriteUint8(record, state->m_zoneType);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogRoadInfo(NB_Context* context,
                 int32 maneuverIndex,
                 uint8 roadType,
                 NB_QaLogRoadInfoData* roadInfo
                 )
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    data_util_state * dataState = NULL;
    NB_ASSERT_VALID_THREAD(context);

    dataState = NB_ContextGetDataState(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_DATA_ROAD_INFO,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteInt32(record, maneuverIndex);
        err = err ? err : CSL_QaRecordWriteUint8(record, roadType);

        err = err ? err : CSL_QaRecordWriteText(record, roadInfo->primary, NB_DATA_ROAD_INFO_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, roadInfo->secondary, NB_DATA_ROAD_INFO_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, roadInfo->exit_number, NB_DATA_ROAD_INFO_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, roadInfo->exit_number_pronun, NB_DATA_ROAD_INFO_TEXT_LENGTH);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogLaneInfo(NB_Context* context,
                 int32 maneuverIndex,
                 NB_QaLogLaneInfoData * laneInfo
                 )
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    data_util_state * dataState = NULL;
    NB_ASSERT_VALID_THREAD(context);

    dataState = NB_ContextGetDataState(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_LANE_INFO2,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteInt32(record, maneuverIndex);
        err = err ? err : CSL_QaRecordWriteText(record, laneInfo->turnCommane, NB_COMMAND_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteInt32(record, laneInfo->lane_position);
        err = err ? err : CSL_QaRecordWriteInt32(record, laneInfo->number_of_lanes);
        err = err ? err : CSL_QaRecordWriteText(record, laneInfo->lg_pronun, NB_LANE_INFO_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, laneInfo->lgprep_pronun, NB_LANE_INFO_TEXT_LENGTH);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogLaneItemInfo(NB_Context* context,
                     int32 maneuverIndex,
                     NB_QaLogLaneItmeInfoData * laneGuidance
                     )
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    data_util_state * dataState = NULL;
    NB_ASSERT_VALID_THREAD(context);

    dataState = NB_ContextGetDataState(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_LANE_ITEM_INFO,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteInt32(record, maneuverIndex);
        err = err ? err : CSL_QaRecordWriteUint32(record, laneGuidance->lane_item);
        err = err ? err : CSL_QaRecordWriteUint32(record, laneGuidance->divider_item);
        err = err ? err : CSL_QaRecordWriteUint32(record, laneGuidance->no_highlight_arrow_item);
        err = err ? err : CSL_QaRecordWriteUint32(record, laneGuidance->highlight_arrow_item);
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}
NB_DEF void
NB_QaLogGuidancePoint(NB_Context* context,
                     int32 maneuverIndex,
                     NB_QaLogGuidanceInfoData * guidance
                     )
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;
    data_util_state * dataState = NULL;
    NB_ASSERT_VALID_THREAD(context);

    dataState = NB_ContextGetDataState(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_POINT2,
                             INITIAL_QARECORD_BUFFER_SIZE, &record);

    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteInt32(record, maneuverIndex);
        err = err ? err : CSL_QaRecordWriteText(record, guidance->turnCommand, NB_COMMAND_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteUint8(record, guidance->guidancePropuse);
        err = err ? err : CSL_QaRecordWriteText(record, guidance->gp_pronun, NB_GUIDANCE_POINT_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, guidance->gpprep_pronun, NB_GUIDANCE_POINT_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteText(record, guidance->description, NB_GUIDANCE_POINT_TEXT_LENGTH);
        err = err ? err : CSL_QaRecordWriteDouble(record, guidance->maneuver_point_offset);
        err = err ? err : CSL_QaRecordWriteDouble(record, guidance->point.lat);
        err = err ? err : CSL_QaRecordWriteDouble(record, guidance->point.lon);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF NB_Error
NB_QaLogWriteBuffer(NB_Context* context,
                    const uint8* data,
                    uint32 dataSize)
{
    NB_ASSERT_VALID_THREAD(context);
    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return NE_NOTINIT;
    }

    return CSL_QaLogWriteData(NB_ContextGetQaLog(context), data, dataSize);
}

/*! @} */
