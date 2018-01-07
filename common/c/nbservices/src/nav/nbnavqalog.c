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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbnavqalog.h"
#include "nbqalog.h"
#include "palclock.h"
#include "cslqalog.h"
#include "cslqarecorddefinitions.h"

#include "nbcontextprotected.h"
#include "nbnavigationstate.h"
#include "nbnavigationstateprivate.h"

#include "nbguidancemessageprivate.h"

static const uint16 INITIAL_QARECORD_BUFFER_SIZE = 250;

static NB_Error
NB_QaLogWriteGuidanceMessage(NB_Context* context, CSL_QaRecord* record, NB_GuidanceMessage* message)
{
    NB_Error error = NE_OK;
    int codeIndex;
    int numberOfCodes = NB_GuidanceMessageGetCodeCount(message);
    const char* code;

    error = error ? error : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetSource(message));
    error = error ? error : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetType(message));

    for (codeIndex = 0; codeIndex < QALOG_MAX_PRONUN_CODES; codeIndex++)
    {
        if (codeIndex < numberOfCodes)
        {
            code = NB_GuidanceMessageGetCode(message, codeIndex);
        }
        else
        {
            code = "";
        }

        if (code != NULL)
        {
            error = error ? error : CSL_QaRecordWriteText(record, code, PRONUN_CODE_LENGTH);
        }
    }

    return error;
}

NB_DEF void
NB_QaLogGuidanceMessageRetrieved(NB_Context* context, NB_GuidanceMessage* message)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_RETRIEVED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {

        err = err ? err : NB_QaLogWriteGuidanceMessage(context, record, message);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void NB_QaLogGuidanceMessagePlayed(NB_Context* context, NB_GuidanceMessage* message)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_PLAYED, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {

        err = err ? err : NB_QaLogWriteGuidanceMessage(context, record, message);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogNavigationState(NB_Context* context, NB_NavigationState* state, NB_NavigationPublicState* publicState)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_STATE4, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteText(record, publicState->currentStreet, NAV_STATE_STREET_LEN);
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

        err = err ? err : CSL_QaRecordWriteText(record, state->speedSignId, NB_NAVIGATION_STATE_SPEED_LIMIT_SIGN_ID_LEN);

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void
NB_QaLogNavigationPosition(NB_Context* context, NB_NavigationState* state, const char* action)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_POS2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint32(record, state->currentRoutePosition.closest_segment);
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
        err = err ? err : CSL_QaRecordWriteDouble(record, get_nav_avg_speed(&state->averageSpeed));

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void
NB_QaLogNavigationStartup(NB_Context* context, NB_LatitudeLongitude* destinationLatLong, const char* destinationStreet, NB_RouteSettings* settings)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_NAV_STARTUP2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteDouble(record, destinationLatLong->latitude);
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
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_OFFROUTE, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (!err)
    {
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)offRouteCount);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)wrongWayCount);
        err = err ? err : CSL_QaRecordWriteText(record, action, NAV_POS_ACTION_LEN);

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
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    // Ignore call if QA logging is disabled
    if (NB_ContextGetQaLog(context) == NULL)
    {
        return;
    }

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

NB_DEF void
NB_QaLogGuidanceMessageRetrieved2(NB_Context* context, NB_GuidanceMessage* message, double maneuverRemaining, double currentSpeed, double playtime, double distance)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_RETRIEVED2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        int codeIndex = 0;
        int numberOfCodes = NB_GuidanceMessageGetCodeCount(message);
        const char* code = NULL;

        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetSource(message));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetType(message));
        err = err ? err : CSL_QaRecordWriteDouble(record, maneuverRemaining);
        err = err ? err : CSL_QaRecordWriteDouble(record, currentSpeed);
        err = err ? err : CSL_QaRecordWriteDouble(record, playtime);
        // todo add element in qarecords.py
        // err = err ? err : CSL_QaRecordWriteDouble(record, distance);

        for (codeIndex = 0; codeIndex < QALOG_MAX_PRONUN_CODES; codeIndex++)
        {
            if (codeIndex < numberOfCodes)
            {
                code = NB_GuidanceMessageGetCode(message, codeIndex);
            }
            else
            {
                code = "";
            }
            if (code != NULL)
            {
                err = err ? err : CSL_QaRecordWriteText(record, code, PRONUN_CODE_LENGTH);
            }
        }

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        CSL_QaRecordDestroy(record);
    }
}


NB_DEF void NB_QaLogGuidanceMessagePlayed2(NB_Context* context, NB_GuidanceMessage* message, double maneuverRemaining, double playtime, double distance)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_MESSAGE_PLAYED2, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        int codeIndex = 0;
        int numberOfCodes = NB_GuidanceMessageGetCodeCount(message);
        const char* code = NULL;

        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetSource(message));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)NB_GuidanceMessageGetType(message));
        err = err ? err : CSL_QaRecordWriteDouble(record, maneuverRemaining);
        err = err ? err : CSL_QaRecordWriteDouble(record, playtime);
        err = err ? err : CSL_QaRecordWriteDouble(record, distance);

        for (codeIndex = 0; codeIndex < QALOG_MAX_PRONUN_CODES; codeIndex++)
        {
            if (codeIndex < numberOfCodes)
            {
                code = NB_GuidanceMessageGetCode(message, codeIndex);
            }
            else
            {
                code = "";
            }
            if (code != NULL)
            {
                err = err ? err : CSL_QaRecordWriteText(record, code, PRONUN_CODE_LENGTH);
            }
        }

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void NB_QaLogGuidanceAudioPlayTime(NB_Context* context, double playtime, NB_GuidanceMessage* message)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_AUDIO_PLAYTIME, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        int codeIndex = 0;
        int numberOfCodes = NB_GuidanceMessageGetCodeCount(message);
        const char* code = NULL;

        err = err ? err : CSL_QaRecordWriteDouble(record, playtime);

        for (codeIndex = 0; codeIndex < QALOG_MAX_PRONUN_CODES; codeIndex++)
        {
            if (codeIndex < numberOfCodes)
            {
                code = NB_GuidanceMessageGetCode(message, codeIndex);
            }
            else
            {
                code = "";
            }

            if (code != NULL)
            {
                err = err ? err : CSL_QaRecordWriteText(record, code, PRONUN_CODE_LENGTH);
            }
        }

        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);
        CSL_QaRecordDestroy(record);
    }
}

NB_DEF void NB_QaLogGuidanceTimimgData(NB_Context* context, double turnGap, double turn,
                                                            double prepareGap, double prepare,
                                                            double continueGap, double turnLength,
                                                            double prepareLength, nb_boolean isPrepareGuideUsed,
                                                            nb_boolean isTurnGuideUsed)
{
    NB_Error err = NE_OK;
    CSL_QaRecord* record = 0;

    NB_ASSERT_VALID_THREAD(context);

    err = CSL_QaRecordCreate(PAL_ClockGetGPSTime(), RECORD_ID_GUIDANCE_TIMING_DATA, INITIAL_QARECORD_BUFFER_SIZE, &record);
    if (err == NE_OK)
    {
        err = err ? err : CSL_QaRecordWriteDouble(record, turnGap);
        err = err ? err : CSL_QaRecordWriteDouble(record, turn);
        err = err ? err : CSL_QaRecordWriteDouble(record, prepareGap);
        err = err ? err : CSL_QaRecordWriteDouble(record, prepare);
        err = err ? err : CSL_QaRecordWriteDouble(record, continueGap);
        err = err ? err : CSL_QaRecordWriteDouble(record, turnLength);
        err = err ? err : CSL_QaRecordWriteDouble(record, prepareLength);
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(isPrepareGuideUsed ? 1 : 0));
        err = err ? err : CSL_QaRecordWriteUint8(record, (uint8)(isTurnGuideUsed ? 1 : 0));
        err = err ? err : CSL_QaLogWrite(NB_ContextGetQaLog(context), record);

        CSL_QaRecordDestroy(record);
    }
}
