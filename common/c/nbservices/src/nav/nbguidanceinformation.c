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

/*!-----------------`---------------------------------------------------------

    @file     nbguidanceinfo.c
    @defgroup nbguidanceinfo

    Information about guidance
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

#include "palmath.h"
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbguidanceinformation.h"
#include "nbguidanceinformationprivate.h"
#include "nbguidancemessageprivate.h"
#include "nbrouteinformationprivate.h"
#include "instructset.h"
#include "paltestlog.h"
#include "tpslib.h"
#include "tpsio.h"

#include "navutil.h"

struct NB_GuidanceInformation
{
    NB_Context*   context;

    char* imageStyle;
    char* stepsStyle;
    char* pronunStyle;

    instructset* images;
    instructset* steps;
    instructset* voices;
    struct tpslib* tlBasicAudio;
    tpselt         basicAudioConfig;
    instructset** localeImages;           //array of image instructset with different countries.
    nb_size localeImageSize;
};

#define BASICAUDIOS_ELEMENT "basicaudios"
#define AUDIOITEM_ELEMENT   "audioitem"
#define NAME_ELEMENT        "name"
#define TEXT_ELEMENT        "text"
#define DURATION_ELEMENT    "duration"

static void FreeInstructionSet(char** ppstyle, instructset** ppis);
static NB_Error SetupInstructionSet(NB_Context* context, const char* pstyle, instruct_data_source ds, void* ds_user, char* datalib, nb_size datalibSize, char* configBuffer, nb_size configBufferSize, instructset** ppis);
static NB_Error    NB_GuidanceInformationEnumText(instructtype type, uint32 maneuverIndex, instructset* steps, NB_NavigateAnnouncementUnits units, NB_GuidanceInformationTextCallback callback, void* userData);
static NB_Error LoadBasicAudioSet(NB_GuidanceInformation* information, char* datalib, nb_size datalibSize, char* configBuffer, nb_size configBufferSize);
static void FreeBasicAudioSet(NB_GuidanceInformation* information);

NB_DEF NB_Error
NB_GuidanceInformationCreate(NB_Context* context, NB_GuidanceInformationConfiguration* configuration, NB_GuidanceInformation** information)
{
    NB_GuidanceInformation* pThis = NULL;
    int32 i = 0;

    if (!information || !context || !configuration )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(context);

    if ((pThis = nsl_malloc(sizeof(NB_GuidanceInformation))) == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_GuidanceInformation));

    if (configuration->localeImagesSize > 0)
    {
        pThis->localeImageSize = configuration->localeImagesSize;
        if((pThis->localeImages = nsl_malloc(sizeof(instructset*) * configuration->localeImagesSize)) == NULL)
        {
            nsl_free(pThis);
            return NE_NOMEM;
        }
    }

    pThis->context = context;

    if( configuration->datalibBuffer != NULL && configuration->datalibBufferSize > 0)
    {
        if (configuration->pronunStyle && configuration->voiceBuffer && configuration->voiceBufferSize > 0)
        {
            pThis->pronunStyle = nsl_strdup(configuration->pronunStyle);
            SetupInstructionSet(pThis->context, pThis->pronunStyle, NULL, NULL,
                                configuration->datalibBuffer, configuration->datalibBufferSize,
                                configuration->voiceBuffer, configuration->voiceBufferSize, &pThis->voices);
        }

        if (configuration->imageStyle && configuration->imagesBuffer && configuration->imagesBufferSize > 0)
        {
            pThis->imageStyle = nsl_strdup(configuration->imageStyle);
            SetupInstructionSet(pThis->context, pThis->imageStyle, NULL, NULL,
                                configuration->datalibBuffer, configuration->datalibBufferSize,
                                configuration->imagesBuffer, configuration->imagesBufferSize, &pThis->images);
        }

        if (configuration->stepsStyle && configuration->stepsBuffer && configuration->stepsBufferSize)
        {
            pThis->stepsStyle = nsl_strdup(configuration->stepsStyle);
            SetupInstructionSet(pThis->context, pThis->stepsStyle, NULL, NULL,
                                configuration->datalibBuffer, configuration->datalibBufferSize,
                                configuration->stepsBuffer, configuration->stepsBufferSize, &pThis->steps);
        }

        for (i = 0; i < configuration->localeImagesSize && configuration->imageStyle; i++)
        {
            SetupInstructionSet(pThis->context, configuration->localeImages[i].countryCode, NULL, NULL,
                                configuration->datalibBuffer, configuration->datalibBufferSize,
                                configuration->localeImages[i].imagesBuffer,
                                configuration->localeImages[i].imagesBufferSize, &pThis->localeImages[i]);
        }
    }
    else
    {
        // Voices are optional: those are not supported by NBI SDK, so appropriate data may be NULL
        if (configuration->pronunStyle && configuration->voiceBuffer && configuration->voiceBufferSize > 0)
        {
            pThis->pronunStyle = nsl_strdup(configuration->pronunStyle);
            SetupInstructionSet(pThis->context, pThis->pronunStyle, NULL, NULL,
                                configuration->voicelibBuffer, configuration->voicelibBufferSize,
                                configuration->voiceBuffer, configuration->voiceBufferSize, &pThis->voices);
        }

        if (configuration->imageStyle && configuration->imagesBuffer && configuration->imagesBufferSize > 0)
        {
            pThis->imageStyle = nsl_strdup(configuration->imageStyle);
            SetupInstructionSet(pThis->context, pThis->imageStyle, NULL, NULL,
                                configuration->imagelibBuffer, configuration->imagelibBufferSize,
                                configuration->imagesBuffer, configuration->imagesBufferSize, &pThis->images);
        }

        if (configuration->stepsStyle && configuration->stepsBuffer && configuration->stepsBufferSize)
        {
            pThis->stepsStyle = nsl_strdup(configuration->stepsStyle);
            SetupInstructionSet(pThis->context, pThis->stepsStyle, NULL, NULL,
                                configuration->stepslibBuffer, configuration->stepslibBufferSize,
                                configuration->stepsBuffer, configuration->stepsBufferSize, &pThis->steps);
        }

        for (i = 0; i < configuration->localeImagesSize && configuration->imageStyle; i++)
        {
            SetupInstructionSet(pThis->context, configuration->localeImages[i].countryCode, NULL, NULL,
                                configuration->imagelibBuffer, configuration->imagelibBufferSize,
                                configuration->localeImages[i].imagesBuffer,
                                configuration->localeImages[i].imagesBufferSize, &pThis->localeImages[i]);
        }
    }
    // BasicAudio sexp contains pronun information for TTS
    if (configuration->basicAudioBuffer && configuration->basicAudioBufferSize &&
        configuration->basicAudiolibBuffer && configuration->basicAudiolibBufferSize)
    {
        LoadBasicAudioSet(pThis, configuration->basicAudiolibBuffer, configuration->basicAudiolibBufferSize,
                          configuration->basicAudioBuffer, configuration->basicAudioBufferSize);
    }
    (*information)  = pThis;

    return NE_OK;
}

NB_DEF uint32
NB_GuidanceInformationGetCommandVersion(NB_GuidanceInformation* information)
{
    uint32 voices_version = 0;
    uint32 images_version = 0;
    uint32 steps_version = 0;
    uint32 ret = 0;
    int32 i = 0;

    if (!information)
        return 0;

    NB_ASSERT_VALID_THREAD(information->context);

    if (information->voices)
        instructset_getversion(information->voices, &voices_version);

    if (information->images)
        instructset_getversion(information->images, &images_version);

    if (information->steps)
        instructset_getversion(information->steps, &steps_version);

    ret = MAX(MAX(voices_version, images_version), steps_version);

    for (i = 0; i < information->localeImageSize; i++)
    {
        images_version = 0;
        if (instructset_getversion(information->localeImages[i], &images_version) == NE_OK)
        {
            ret = MAX(ret, images_version);
        }
    }

    return ret;
}

NB_Error
NB_GuidanceInformationGetVoices(NB_GuidanceInformation* information, instructset** voice)
{
    if (!information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    if (!information->voices)
    {
        return NE_NOENT;
    }

    *voice = information->voices;

    return NE_OK;
}

NB_Error
NB_GuidanceInformationGetSteps(NB_GuidanceInformation* information, instructset** steps)
{
    if (!information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    if (!information->steps)
    {
        return NE_NOENT;
    }

    *steps = information->steps;

    return NE_OK;
}

NB_Error
NB_GuidanceInformationGetImages(NB_GuidanceInformation* information, instructset** images)
{
    if (!information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    if (!information->images)
    {
        return NE_NOENT;
    }

    *images = information->images;

    return NE_OK;
}

NB_DEF NB_Error
NB_GuidanceInformationGetVoiceStyle(NB_GuidanceInformation* information, char** voice)
{
    if (!information || !voice)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    if (!information->pronunStyle)
    {
        return NE_NOENT;
    }

    *voice = nsl_strdup(information->pronunStyle);

    return NE_OK;
}

NB_DEF NB_Error
NB_GuidanceInformationGetStepStyle(NB_GuidanceInformation* information, char* steps)
{
    if (!information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    if (!information->stepsStyle)
    {
        return NE_NOENT;
    }

    steps = nsl_strdup(information->stepsStyle);

    return NE_OK;
}

NB_DEF NB_Error
NB_GuidanceInformationGetImageStyle(NB_GuidanceInformation* information, char* images)
{
    if (!information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    if (!information->imageStyle)
    {
        return NE_NOENT;
    }

    images = nsl_strdup(information->imageStyle);

    return NE_OK;
}

NB_DEF NB_Error
NB_GuidanceInformationSetDataSourceFunction(NB_GuidanceInformation* information, NB_GuidanceDataSourceFunction datasourceFunc, void* userData)
{
    int32 i = 0;

    if (!information)
        return NE_INVAL;

    NB_ASSERT_VALID_THREAD(information->context);

    // Voices are not yet supported by NBI SDK, so the appropriate buffer could be NULL
    if (information->voices != NULL)
    {
        instructset_setdatasource(information->voices, (instruct_data_source)datasourceFunc, userData);
    }

    if (information->images != NULL)
    {
        instructset_setdatasource(information->images, (instruct_data_source)datasourceFunc, userData);
    }

    if (information->steps != NULL)
    {
        instructset_setdatasource(information->steps, (instruct_data_source)datasourceFunc, userData);
    }

    for(i = 0; i < information->localeImageSize; i++)
    {
        instructset_setdatasource(information->localeImages[i], (instruct_data_source)datasourceFunc, userData);
    }

    return NE_OK;
}

NB_DEF NB_Error NB_GuidanceInformationDestroy(NB_GuidanceInformation* information)
{
    int32 i = 0;

    if (!information)
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    FreeInstructionSet(&information->imageStyle, &information->images);
    FreeInstructionSet(&information->stepsStyle, &information->steps);
    FreeInstructionSet(&information->pronunStyle, &information->voices);
    FreeBasicAudioSet(information);

    for (i = 0; i < information->localeImageSize; i++)
    {
        instructset_dealloc(information->localeImages[i]);
        information->localeImages[i] = NULL;
    }
    if (information->localeImages)
    {
        nsl_free(information->localeImages);
    }

    nsl_free(information);

    return NE_OK;
}

NB_DEF NB_Error
NB_GuidanceInformationGetTurnImage(NB_GuidanceInformation* information,
                                   NB_GuidanceDataSourceFunction dataSource,
                                   void* dataSourceUserData, uint32 index,
                                   NB_NavigateManeuverPos maneuverPosition,
                                   NB_NavigateAnnouncementUnits units,
                                   char* imageCodeBuffer,
                                   uint32 imageCodeBufferSize)
{
    NB_Error err = NE_OK;
    InstructionCodes codes = {{{0}}};

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    err = nb_instructset_getdata(information->images, index, &codes, NULL, NULL, NULL, it_image, NB_NAT_None, FALSE, units, maneuverPosition, NULL);

    if (err == NE_OK)
    {
        if ((codes.count != 1) || (nsl_strlen(codes.codes[0]) >= imageCodeBufferSize))
        {
            return NE_INVAL;
        }

        nsl_strcpy(imageCodeBuffer, codes.codes[0]);
    }

    return err;
}

NB_Error
NB_GuidanceInformationGetTurnCharacter(NB_GuidanceInformation* information,
                                   NB_GuidanceDataSourceFunction dataSource,
                                   void* dataSourceUserData, uint32 index,
                                   NB_NavigateManeuverPos maneuverPosition,
                                   NB_NavigateAnnouncementUnits units,
                                   char* characterBuffer,
                                   uint32 characterBufferSize)
{
    NB_Error err = NE_OK;
    InstructionCodes codes = {{{0}}};
    NB_RouteDataSourceOptions* dataSourceOption = NULL;
    data_nav_maneuver* pmaneuver = NULL;
    data_util_state* dataState = NULL;
    int32 i = 0;
    const char* instructsetName = NULL;

    NB_ASSERT_VALID_THREAD(information->context);

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    if (index == NAV_MANEUVER_NONE)
    {
        return NE_INVAL;
    }
    if (index == NAV_MANEUVER_START || index == NAV_MANEUVER_CURRENT || index == NAV_MANEUVER_INITIAL)
    {
        index = 0;
    }

    dataSourceOption = (NB_RouteDataSourceOptions*)dataSourceUserData;
    pmaneuver = NB_RouteInformationGetManeuver(dataSourceOption->route, index);
    if (!pmaneuver)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(information->context);

    NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    //Try to find TTF character in locale images instructset with current maneuver country code.
    for (i = 0; i < information->localeImageSize; i++)
    {
        instructsetName = instructset_getname(information->localeImages[i]);
        if (pmaneuver->turn_roadinfo.country_info.code &&
            nsl_strncmp(instructsetName,
                        data_string_get(dataState, &pmaneuver->turn_roadinfo.country_info.code),
                        nsl_strlen(instructsetName)) == 0)
        {
            err = nb_instructset_getdata(information->localeImages[i], index, &codes, NULL, NULL, NULL, it_ttf, NB_NAT_None, FALSE, units, maneuverPosition, NULL);
            break;
        }
    }

    //If can't find TTF character in special images instructset then try to find it in normal images instructset.
    if (err != NE_OK || (codes.count != 1) || (nsl_strlen(codes.codes[0]) >= characterBufferSize))
    {
        nsl_memset(&codes, 0, sizeof(codes));
        err = nb_instructset_getdata(information->images, index, &codes, NULL, NULL, NULL, it_ttf, NB_NAT_None, FALSE, units, maneuverPosition, NULL);
    }

    if (err == NE_OK)
    {
        if ((codes.count != 1) || (nsl_strlen(codes.codes[0]) >= characterBufferSize))
        {
            return NE_INVAL;
        }

        nsl_strlcpy(characterBuffer, codes.codes[0], characterBufferSize);
    }

    return err;
}


NB_Error
NB_GuidanceInformationGetTurnAnnouncement(NB_GuidanceInformation* information,
    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index,
    NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementSource source,
    NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message, uint32* soundTypes)
{
    NB_Error err = NE_OK;
    NB_GuidanceMessage* turnMessage = NULL;
    InstructionCodes* codes = NULL;
    instructtype instructType = it_invalid;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    err = NB_GuidanceMessageCreate(information->context, source, NB_GMT_Guidance, &turnMessage);

    if (err != NE_OK)
    {
        return err;
    }

    *message = turnMessage;

    codes = NB_GuidanceMessageGetInstructionCodes(turnMessage);

    err = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    if (err == NE_OK)
    {
        instructType = source == NB_NAS_Lookahead ? it_audio_lookahead : it_audio;

        err = nb_instructset_getdata(information->voices, index, codes, NULL, NULL, NULL,
            instructType, type, FALSE, units, maneuverPosition, soundTypes);
    }

    if (err != NE_OK)
    {
        if (turnMessage)
        {
            NB_GuidanceMessageDestroy(turnMessage);
            turnMessage = NULL;
            *message = NULL;
        }
    }

    return err;
}

NB_DEF NB_Error
NB_GuidanceInformationGetRouteCalculationAnnouncement(NB_GuidanceInformation* information,
    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
    NB_NavigateRouteCalculationType type, NB_NavigateAnnouncementType announceType, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    NB_Error err = NE_OK;
    NB_GuidanceMessage* turnMessage = NULL;
    InstructionCodes* codes = NULL;
    NB_NavigateManeuverPos maneuverPosition = NB_NMP_Invalid;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    maneuverPosition = NB_GuidanceInformationGetRouteCalculationManeuver(type);
    if (maneuverPosition == NB_NMP_Invalid)
    {
        return NE_INVAL;
    }

    err = NB_GuidanceMessageCreate(information->context, NB_NAS_Automatic, NB_GMT_Guidance, &turnMessage);

    if (err != NE_OK)
    {
        return err;
    }

    *message = turnMessage;

    codes = NB_GuidanceMessageGetInstructionCodes(turnMessage);

    NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    err = nb_instructset_getdata(information->voices, NAV_MANEUVER_NONE, codes, NULL, NULL, NULL, it_audio, announceType, FALSE, units, maneuverPosition, NULL);

    if (err != NE_OK)
    {
        if (turnMessage)
        {
            NB_GuidanceMessageDestroy(turnMessage);
            turnMessage = NULL;
            *message = NULL;
        }
    }

    return err;
}

NB_DEF NB_Error
NB_GuidanceInformationGetNavigationModeSwitchAnnouncement(NB_GuidanceInformation* information,
    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
    NB_NavigateAnnouncementType announceType, NB_NavigateAnnouncementUnits units,
    NB_GuidanceMessage** message)
{
    NB_Error err = NE_OK;
    NB_GuidanceMessage* turnMessage = NULL;
    InstructionCodes* codes = NULL;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    err = NB_GuidanceMessageCreate(information->context, NB_NAS_Automatic, NB_GMT_Guidance, &turnMessage);
    if (err != NE_OK)
    {
        return err;
    }

    *message = turnMessage;

    codes = NB_GuidanceMessageGetInstructionCodes(turnMessage);

    NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    err = nb_instructset_getdata(information->voices, NAV_MANEUVER_NONE, codes, NULL, NULL, NULL, it_audio, announceType, FALSE, units, NB_NMP_SwitchToTBT, NULL);

    if (err != NE_OK)
    {
        if (turnMessage)
        {
            NB_GuidanceMessageDestroy(turnMessage);
            turnMessage = NULL;
            *message = NULL;
        }
    }

    return err;
}

NB_DEF NB_Error
NB_GuidanceInformationEnumerateManeuverText(NB_GuidanceInformation* information, NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData)
{
    NB_Error err = NE_OK;
    instructtype type = it_invalid;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    err = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    if (err != NE_OK)
    {
        return err;
    }

    switch (textType)
    {
    case NB_NTT_Primary:
        type = it_txt;
        break;
    case NB_NTT_Secondary:
        type = it_txt_sec;
        break;
    case NB_NTT_Lookahead:
        type = it_txt_lookahead;
        break;
    case NB_NTT_Invalid:
        /* should not get here */
        nsl_assert(FALSE);
        break;
    }

    return  NB_GuidanceInformationEnumText(type, index, information->steps, units, callback, userData);
}

NB_DEF NB_Error
NB_GuidanceInformationEnumerateStackedTurnText(NB_GuidanceInformation* information, NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData)
{
    NB_Error err = NE_OK;
    instructtype type = it_invalid;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    err = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    if (err != NE_OK)
    {
        return err;
    }

    switch (textType)
    {
    case NB_NTT_Primary:
        type = it_txt_stack;
        break;
    case NB_NTT_Secondary:
        type = it_txt_stack_sec;
        break;
    case NB_NTT_Lookahead:
        type = it_txt_lookahead;
        break;
    case NB_NTT_Invalid:
        /* should not get here */
        nsl_assert(FALSE);
        break;
    }

    return NB_GuidanceInformationEnumText(type, index, information->steps, units, callback, userData);
}

NB_DEF NB_Error
NB_GuidanceInformationEnumerateArrivalText(NB_GuidanceInformation* information, NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData)
{
    NB_Error err = NE_OK;
    instructtype type = it_invalid;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    err = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    if (err != NE_OK)
    {
        return err;
    }

    switch (textType)
    {
    case NB_NTT_Primary:
        type = it_txt_arrival;
        break;
    case NB_NTT_Secondary:
        type = it_txt_arrival_sec;
        break;
    case NB_NTT_Lookahead:
        type = it_txt_lookahead;
        break;
    case NB_NTT_Invalid:
        /* should not get here */
        nsl_assert(FALSE);
        break;
    }

    return NB_GuidanceInformationEnumText(type, index, information->steps, units, callback, userData);
}

static void
FreeInstructionSet(char** ppstyle, instructset** ppis)
{
    if (*ppis != NULL)
    {
        instructset_dealloc(*ppis);
    }

    *ppis = NULL;

    if (*ppstyle != NULL)
    {
        nsl_free(*ppstyle);
    }

    *ppstyle = NULL;
}

static NB_Error
SetupInstructionSet(NB_Context* context, const char* pstyle, instruct_data_source ds, void* ds_user, char* datalib, nb_size datalibSize, char* configBuffer, nb_size configBufferSize, instructset** ppis)
{
    NB_Error err = NE_OK;

    if (pstyle != NULL)
    {

        *ppis = instructset_alloc(context, pstyle, nav_format, NULL);

        if (*ppis == NULL)
        {
            err = NE_NOMEM;
            goto errexit;
        }

        instructset_setdatasource(*ppis, ds, ds_user);

        err = instructset_load(*ppis, datalib, datalibSize, configBuffer, configBufferSize);

        if (err != NE_OK)
            return err;
    }

errexit:

    if (err != NE_OK)
        FreeInstructionSet(NULL, ppis);

    return err;
}

static NB_Error
NB_GuidanceInformationEnumText(instructtype type, uint32 maneuverIndex, instructset* steps, NB_NavigateAnnouncementUnits units, NB_GuidanceInformationTextCallback callback, void* userData)
{
    NB_Error            err         = NE_OK;
    byte*                tdata       = NULL;
    size_t                tsize       = 0;
    byte*                d           = NULL;
    NB_Font            font        = NB_Font_Normal;
    nb_color            color       = 0;
    boolean                newline     = FALSE;
    uint32                txtlen      = 0;
    char                addtxt[512] = {0};

    // This is not used for text requests
    InstructionCodes    dummyCodes  = {{{0}}};

    if (!steps)
    {
        return NE_INVAL;
    }

    // Get the text data
    err = nb_instructset_getdata(steps,
        maneuverIndex,
        &dummyCodes,
        &tdata,
        &tsize,
        NULL,
        type,
        NB_NAT_None,
        FALSE,
        units,
        NB_NMP_Turn, NULL);

    if (err == NE_OK && tdata != NULL)
    {
        d = tdata;

        font = NB_Font_Normal;
        color = MAKE_NB_COLOR(0,0,0);
        newline = TRUE;

        while (d < tdata+tsize)
        {
            switch (*d)
            {
            case TEXT_FONT_TOKEN:
                nsl_memcpy(&font, d+1, sizeof(font));
                d += (sizeof(font) + 1);
                break;

            case TEXT_COLOR_TOKEN:
                nsl_memcpy(&color, d+1, sizeof(color));
                d += (sizeof(color) + 1);
                break;

            case TEXT_TEXT_TOKEN:
                nsl_memcpy(&txtlen, d+1, sizeof(txtlen));
                nsl_strlcpy(addtxt, (char*)(d+sizeof(txtlen)+1), sizeof(addtxt));

                // Call user supplied callback
                callback(font, color, addtxt, newline, userData);

                d += (sizeof(txtlen)+txtlen+1);
                newline = FALSE;
                break;

            case TEXT_NEWLINE_TOKEN:

                if (newline)
                {
                    nsl_strlcpy(addtxt, " ", sizeof(addtxt));

                    // Call user supplied callback
                    callback(font, color, addtxt, TRUE, userData);
                }

                newline = TRUE;
                d++;
                break;

            default:
                nsl_assert((0)); // Unexpected Text Token
            }
        }

        nsl_free(tdata);
    }

    return err;
}


NB_NavigateManeuverPos
NB_GuidanceInformationGetRouteCalculationManeuver(NB_NavigateRouteCalculationType type)
{
    uint32 maneuver = NB_NMP_Invalid;

    switch (type)
    {
    case NB_RCT_Initial:
        maneuver = NB_NMP_Calc;
        break;
    case NB_RCT_Traffic:
        maneuver = NB_NMP_RecalculateTraffic;
        break;
    case NB_RCT_Recalc:
        maneuver = NB_NMP_Recalculate;
        break;
    case NB_RCT_Reload:
        maneuver = NB_NMP_SoftRecalculate;
        break;
    case NB_RCT_ConfirmRecalc:
        maneuver = NB_NMP_RecalculateConfirm;
        break;
    case NB_RCT_None:
        /* should not get here */
        nsl_assert(FALSE);
        break;
    case NB_RCT_RouteUpdated:
        maneuver = NB_NMP_RouteUpdated;
        break;
    }

    return maneuver;
}

static NB_Error
LoadBasicAudioSet(NB_GuidanceInformation* information, char* datalib, nb_size datalibSize, char* configBuffer, nb_size configBufferSize)
{
    struct tps_unpackstate *tpsups = NULL;
    struct tpslib*         tl = NULL;
    tpselt                 configTps = NULL;
    NB_Error               err = NE_INVAL;
    const char*            name = NULL;

    tl = tpslib_preload(datalib, datalibSize);

    if (tl != NULL)
    {
        tpsups = tps_unpack_start(tl, configBufferSize);

        if (tpsups != NULL)
        {
            (void) tps_unpack_feed(tpsups, configBuffer, configBufferSize); // tps_unpack_result will report failure
            if (tps_unpack_result(tpsups, &configTps) == TIO_READY)
            {
                name = te_getname(configTps);
                if (nsl_strcmp(BASICAUDIOS_ELEMENT, name) == 0)
                {
                    information->tlBasicAudio = tl;
                    information->basicAudioConfig = configTps;
                    err = NE_OK;
                }
            }
        }
    }

    if (err != NE_OK)
    {
        if (tl != NULL)
        {
            tpslib_dealloc(tl);
        }

        if (configTps != NULL)
        {
            te_dealloc(configTps);
        }
        information->tlBasicAudio = NULL;
        information->basicAudioConfig = NULL;
    }

    return err;
}

static void
FreeBasicAudioSet(NB_GuidanceInformation* information)
{
    if (information->tlBasicAudio != NULL)
    {
        tpslib_dealloc(information->tlBasicAudio);
    }

    if (information->basicAudioConfig != NULL)
    {
        te_dealloc(information->basicAudioConfig);
    }
}

NB_DEF NB_Error NB_GuidanceInformationGetPronunInformation(NB_GuidanceInformation* guidanceInformation, const char* key,
                                                           char* textBuffer, uint32* textBufferSize,
                                                           double* duration)
{
    tpselt ce = NULL;
    int iter = 0;
    const char* name = NULL;
    const char* stringTmp = NULL;

    if (!guidanceInformation || !key ||
        (!duration && !textBuffer && !textBufferSize
        && (textBufferSize && (*textBufferSize) == 0)))
    {
        return NE_INVAL;
    }

    // basicAudio sexp is not loaded
    if (!guidanceInformation->basicAudioConfig || !guidanceInformation->tlBasicAudio)
    {
        return NE_NOENT;
    }

    while ((ce = te_nextchild(guidanceInformation->basicAudioConfig, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), AUDIOITEM_ELEMENT) == 0)
        {
            name = te_getattrc(ce, NAME_ELEMENT);

            if (name && nsl_strcmp(name, key) == 0)
            {
                size_t textLen = 0;
                stringTmp = te_getattrc(ce, TEXT_ELEMENT);
                textLen = nsl_strlen(stringTmp);

                if (textBuffer && textBufferSize && (*textBufferSize) >= textLen && textLen > 0)
                {
                    nsl_strlcpy(textBuffer, stringTmp, (*textBufferSize));
                    (*textBufferSize) = (uint32)(textLen + 1);
                }
                else if (textBufferSize)
                {
                    (*textBufferSize) = 0;
                }

                if (duration)
                {
                    char* v = NULL;
                    size_t vlen = 0;
                    te_getattr(ce, DURATION_ELEMENT, &v, &vlen);
                    if (v && vlen)
                    {
                        *duration = nsl_atof(v);
                    }
                }

                return NE_OK;
            }
        }
    }

    return NE_NOENT;
}

NB_DEF NB_Error
NB_GuidanceInformationGetNavigationModeSwitchToStaticAnnouncement(NB_GuidanceInformation* information,
    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData,
    NB_NavigateAnnouncementType announceType, NB_NavigateAnnouncementUnits units,
    NB_GuidanceMessage** message)
{
    NB_Error err = NE_OK;
    NB_GuidanceMessage* turnMessage = NULL;
    InstructionCodes* codes = NULL;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    NB_ASSERT_VALID_THREAD(information->context);

    err = NB_GuidanceMessageCreate(information->context, NB_NAS_Automatic, NB_GMT_Guidance, &turnMessage);
    if (err != NE_OK)
    {
        return err;
    }

    *message = turnMessage;

    codes = NB_GuidanceMessageGetInstructionCodes(turnMessage);

    NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    err = nb_instructset_getdata(information->voices, NAV_MANEUVER_NONE, codes, NULL, NULL, NULL, it_audio, announceType, FALSE, units, NB_NMP_SwitchToStaticMode, NULL);

    if (err != NE_OK)
    {
        if (turnMessage)
        {
            NB_GuidanceMessageDestroy(turnMessage);
            turnMessage = NULL;
            *message = NULL;
        }
    }

    return err;
}


/*! @} */
