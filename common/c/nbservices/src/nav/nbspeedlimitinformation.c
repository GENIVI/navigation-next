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

 @file     nbspeedlimitinformation.c
 */
/*
 (C) Copyright 2011 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of Networks In Motion is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
#include "nbcontextprotected.h"
#include "nbspeedlimitprocessorprivate.h"
#include "nbspeedlimittypes.h"
#include "nbspeedlimitinformationprivate.h"
#include "data_speed_limits_data.h"
#include "data_speed_region.h"
#include "instructset.h"
#include "nbguidancemessage.h"
#include "nbguidancemessageprivate.h"
#include "nbguidanceinformationprivate.h"

// Local Functions ...............................................................................

/*! Get Speed Regions from data_speed_limits_data

 @param state An data_util_state object
 @param speedLimitData An data_speed_limits_data object
 @param reply An NB_MetadataSourceReply object set from data_content_source
 @param vectorRegions vector of NB_SpeedRegion added by reply
 @returns NB_Error
 */

static NB_Error
SpeedLimitDataGetSpeedRegions(data_util_state* state, data_speed_limits_data* speedLimitData, CSL_Vector** vectorRegions);

/*! SpeedRegions Initialize

 @param state                       An data_util_state object
 @param speedRegions                Initialize Object
 @returns NB_Error
 */
static NB_Error
NB_SpeedRegion_init(data_util_state* state, NB_SpeedRegion* speedRegions);

#if 0
/*! SpeedRegions copy

 @param state                       An data_util_state object
 @param destinationSpeedRegions     destination
 @param sourceSpeedRegions          source
 @returns NB_Error
 */
static NB_Error
NB_SpeedRegion_copy(data_util_state* state, NB_SpeedRegion* destinationSpeedRegions, NB_SpeedRegion* sourceSpeedRegions);
#endif

/*! SpeedRegions free

 @param state                       An data_util_state object
 @param speedRegions                free Object
 */
static void
NB_SpeedRegion_free(data_util_state* state, NB_SpeedRegion* speedRegions);

/*! string copy

 @param state                       An data_util_state object
 @param destination                 destination
 @param sourctString                source
 @returns NB_Error
 */
static NB_Error
DataStringCopy(data_util_state* state, char** destination, data_string* sourctString);



// Public functions ..............................................................................

/* See header file for description */
NB_Error
NB_SpeedLimitInformationCreateFromDataNavReply(NB_Context* context,                      /*!< Pointer to current context */
                                      data_nav_reply * reply,                             /*!< tps element */
                                      NB_SpeedLimitInformation** information    /*!< return information*/
                                      )
{
    NB_Error error = NE_OK;

    NB_SpeedLimitInformation* pThis = 0;
    data_speed_limits_data * speedLimitData ;
    data_util_state * state = NULL;

    CSL_Vector * vectorRegions = NULL;

    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    if (reply -> speed_regions == NULL)
    {
        *information = NULL;
        return NE_NOENT;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->m_context = context;
    state = NB_ContextGetDataState(pThis->m_context);
    speedLimitData = reply->speed_regions;

    error = error ? error : SpeedLimitDataGetSpeedRegions(state, speedLimitData, &vectorRegions);

    if (error == NE_OK)
    {
        pThis->m_speedRegions = vectorRegions;
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return error;
}

NB_Error
NB_SpeedLimitInformationDestroy(NB_SpeedLimitInformation* information    /*!< Destory information*/
                                )
{
    NB_Error error = NE_OK;
    CSL_Vector * regions = NULL;
    data_util_state  * state = NULL;
    if (information == NULL)
    {
        return NE_INVAL;
    }
    regions = information->m_speedRegions;
    state = NB_ContextGetDataState(information->m_context);
    DATA_VEC_FREE(state, regions, NB_SpeedRegion);
    nsl_free(information);

    return error;
}

NB_Error
NB_SpeedLimitInformationGetSpeedRegions(NB_SpeedLimitInformation* information,  /*!< Speed limit information */
                                        CSL_Vector** speedRegions          /*!< return vector */
                                        )
{
    NB_Error error = NE_OK;
    if (information == NULL)
    {
        return NE_INVAL;
    }
    *speedRegions = information->m_speedRegions;
    return error;

}


NB_Error
NB_SpeedLimitInformationFreeSpeedRegions(NB_SpeedLimitInformation* information,  /*!< Speed limit information */
                                         CSL_Vector* speedRegions            /*!< free vector */
                                         )
{
    return NE_OK;
}

NB_Error
NB_SpeedLimitInformationGetAnnouncement(NB_SpeedLimitState* speedLimitState,
                                        NB_GuidanceInformation* information,
                                        NB_GuidanceDataSourceFunction dataSource,
                                        void* dataSourceUserData,
                                        uint32 index,
                                        NB_NavigateAnnouncementSource source,
                                        NB_NavigateAnnouncementType type,
                                        NB_NavigateAnnouncementUnits units,
                                        NB_GuidanceMessage** message)
{
    NB_Error error = NE_OK;
    NB_GuidanceMessage* speedLimitMessage = NULL;
    InstructionCodes* codes = NULL;
    NB_GuidanceMessageType guidanceMessagetype = NB_GMT_None;
    instructset* voices = NULL;
    NB_NavigateManeuverPos pos = NB_NMP_Invalid;

    if ( information == NULL || dataSource == NULL )
    {
        return NE_INVAL;
    }

    // Get guidance message type by speedLimitState
    if (speedLimitState->specialZoneAlertPending)
    {
        guidanceMessagetype = NB_GMT_SchoolZoneWarning;
        pos = NB_NMP_SchoolZone;
    }
    else if (speedLimitState->speedLimitAlertPending)
    {
        guidanceMessagetype = NB_GMT_SpeedWarning;
        pos = NB_NMP_SpeedLimitZone;
    }

    // Create speed limit message
    error = NB_GuidanceMessageCreate(speedLimitState->context, source,  guidanceMessagetype, &speedLimitMessage);

    if (error != NE_OK)
    {
        return error;
    }

    *message = speedLimitMessage;
    codes = NB_GuidanceMessageGetInstructionCodes(speedLimitMessage);

    error = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    NB_GuidanceInformationGetVoices(information, &voices);

    if (error == NE_OK)
    {
        error = nb_instructset_getdata(voices, index, codes, NULL, NULL, NULL,
                                     it_audio_speed_limit, type, (source != NB_NAS_Automatic) ? TRUE : FALSE, units, pos, NULL);
    }

    if (error != NE_OK)
    {
        if (speedLimitMessage)
        {
            NB_GuidanceMessageDestroy(speedLimitMessage);
            *message = NULL;
        }
    }

    return error;
}

// Local functions
NB_Error
SpeedLimitDataGetSpeedRegions(data_util_state* state, data_speed_limits_data* speedLimitData, CSL_Vector** vectorRegions)
{

    NB_Error error = NE_OK;
    NB_SpeedRegion region;
    int length = 0, n = 0;
    data_speed_region * dataRegion = NULL;
    CSL_Vector * vecTmp = NULL;
    if (state == NULL || speedLimitData == NULL) {
        return  NE_INVAL;
    }

    vecTmp = CSL_VectorAlloc(sizeof(NB_SpeedRegion));
    if (vecTmp == NULL)
    {
        return NE_NOMEM;
    }

    // File regions from speedLimitData->speed_regions
    length = CSL_VectorGetLength(speedLimitData->speed_regions);
    for (n = 0; n < length; n++)
    {
        dataRegion = (data_speed_region *) CSL_VectorGetPointer(speedLimitData->speed_regions, n);

        error = NB_SpeedRegion_init(state, &region);
        if (error != NE_OK)
        {
            break;
        }
        error = error ? error : DataStringCopy(state, &region.m_datasetid, &dataRegion->dataset_id);
        error = error ? error : DataStringCopy(state, &region.m_signId, &dataRegion->sign_id);
        error = error ? error : DataStringCopy(state, &region.m_signidHighlighted, &dataRegion->sign_id_highlighted);
        error = error ? error : DataStringCopy(state, &region.m_version, &dataRegion->version);
        error = error ? error : DataStringCopy(state, &region.m_versionHighlighted, &dataRegion->version_highlighted);
        if (error != NE_OK)
        {
            break;
        }
        region.m_endManeuver = dataRegion->end_maneuver_index;
        region.m_endManeuverOffset = dataRegion->end_maneuver_offset;
        region.m_startManeuver = dataRegion->start_maneuver_index;
        region.m_startManeuverOffset = dataRegion->start_maneuver_offset;
        region.m_speedLimit = dataRegion->speed_limit;

        if (dataRegion->special_speed_zone != NULL)
        {
            if (nsl_strcmp(dataRegion->special_speed_zone->speed_zone_type, "SZ")==0)
            {
                region.m_speedZoneType = NB_SpeedZoneType_SchoolZone;
            }
            else if (nsl_strcmp(dataRegion->special_speed_zone->speed_zone_type, "EZ")==0)
            {
                region.m_speedZoneType = NB_SpeedZoneType_SafetyEnhancedZone;
            }
            else if (nsl_strcmp(dataRegion->special_speed_zone->speed_zone_type, "CZ")==0)
            {
                region.m_speedZoneType = NB_SpeedZoneType_ConstructionZone;
            }
            else
            {
                region.m_speedZoneType = NB_SpeedZoneType_None;
            }
            region.m_warnAhead = dataRegion->special_speed_zone->warn_ahead;
        }
        else
        {
            region.m_speedZoneType = NB_SpeedZoneType_None;
            region.m_warnAhead = 0;
        }

        CSL_VectorAppend(vecTmp, &region);

    }while (FALSE);

    if (error != NE_OK)
    {
        DATA_VEC_FREE(state, vecTmp, NB_SpeedRegion);
        vecTmp = NULL;
    }

    *vectorRegions = vecTmp;
    return error;
}

NB_Error
DataStringCopy(data_util_state * state, char ** destination, data_string * string)
{
    int length = 0;
    char * buffer = NULL;
    if (destination == NULL || string == NULL)
    {
        return NE_INVAL;
    }

    length = data_string_get_tps_size(state, string) + 1 ;

    buffer = (char *)nsl_malloc(length);

    if (buffer == NULL) {
        return NE_NOMEM;
    }

    data_string_get_copy(state, string, buffer, length);
    *destination = buffer;
    return NE_OK;
}

NB_Error
NB_SpeedRegion_init(data_util_state* state, NB_SpeedRegion* speedRegions)
{
    NB_Error error = NE_OK;
    if (speedRegions == NULL || state == NULL)
    {
        return NE_INVAL;
    }
    nsl_memset(speedRegions, 0, sizeof(NB_SpeedRegion));
    return error;
}

#if 0
NB_Error
NB_SpeedRegion_copy(data_util_state* state, NB_SpeedRegion* destinationSpeedRegions, NB_SpeedRegion* sourceSpeedRegions)
{
    NB_Error error = NE_OK;
    if (state == NULL || destinationSpeedRegions == NULL || sourceSpeedRegions == NULL)
    {
        return NE_INVAL;
    }

    do
    {
        DATA_REINIT(state, error, destinationSpeedRegions, NB_SpeedRegion);
        if (error != NE_OK)
        {
            break;
        }
        error = error ? error : DataStringCopy(state, &destinationSpeedRegions->m_datasetid, &sourceSpeedRegions->m_datasetid);
        error = error ? error : DataStringCopy(state, &destinationSpeedRegions->m_signId, &sourceSpeedRegions->m_signId);
        error = error ? error : DataStringCopy(state, &destinationSpeedRegions->m_signidHighlighted, &sourceSpeedRegions->m_signidHighlighted);
        error = error ? error : DataStringCopy(state, &destinationSpeedRegions->m_version, &sourceSpeedRegions->m_version);
        error = error ? error : DataStringCopy(state, &destinationSpeedRegions->m_versionHighlighted, &sourceSpeedRegions->m_versionHighlighted);
        if (error != NE_OK)
        {
            break;
        }
        destinationSpeedRegions->m_endManeuver = sourceSpeedRegions->m_endManeuver;
        destinationSpeedRegions->m_endManeuverOffset = sourceSpeedRegions->m_endManeuverOffset;
        destinationSpeedRegions->m_startManeuver = sourceSpeedRegions->m_startManeuver;
        destinationSpeedRegions->m_startManeuverOffset = sourceSpeedRegions->m_startManeuverOffset;
        destinationSpeedRegions->m_speedLimit = sourceSpeedRegions->m_speedLimit;
        destinationSpeedRegions->m_warnAhead = sourceSpeedRegions->m_warnAhead;
        destinationSpeedRegions->m_speedZoneType = sourceSpeedRegions->m_speedZoneType;
    }while (FALSE);

    if(error != NE_OK)
    {
    	if (destinationSpeedRegions->m_datasetid) {
    		data_string_free(state, (data_string*)destinationSpeedRegions->m_datasetid);
    	}
    	if (destinationSpeedRegions->m_signId) {
    		data_string_free(state, (data_string*)destinationSpeedRegions->m_signId);
    	}
    	if (destinationSpeedRegions->m_signidHighlighted) {
    		data_string_free(state, (data_string*)destinationSpeedRegions->m_signidHighlighted);
    	}
    	if (destinationSpeedRegions->m_version) {
    		data_string_free(state, (data_string*)destinationSpeedRegions->m_version);
    	}
    	if (destinationSpeedRegions->m_versionHighlighted) {
    		data_string_free(state, (data_string*)destinationSpeedRegions->m_versionHighlighted);
    	}
    }

    return  error;
}
#endif

void
NB_SpeedRegion_free(data_util_state* state, NB_SpeedRegion* speedRegions)
{
    if (state == NULL || speedRegions == NULL) {
        return;
    }
    if (speedRegions->m_datasetid != NULL)
    {
        nsl_free(speedRegions->m_datasetid);
    }
    if (speedRegions->m_signId != NULL)
    {
        nsl_free(speedRegions->m_signId);
    }
    if (speedRegions->m_signidHighlighted !=NULL)
    {
        nsl_free(speedRegions->m_signidHighlighted);
    }
    if (speedRegions->m_version)
    {
        nsl_free(speedRegions->m_version);
    }
    if (speedRegions->m_versionHighlighted)
    {
        nsl_free(speedRegions->m_versionHighlighted);
    }

}
