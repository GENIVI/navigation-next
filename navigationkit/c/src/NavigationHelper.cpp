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

#include "NavigationHelper.h"
extern "C"
{
#include "nbnavigation.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "palfile.h"
}
#include "palmath.h"

namespace nbnav
{
const char*  LOCALE_IMAGE_PREFIX        = "images-";

NB_GuidanceInformation* CreateGuidanceInformation(NB_Context* context, PAL_Instance* pal,
                                                  const char* workFolder, const char* baseVoicePath,
                                                  const char* voiceStyle, const char* locale,
                                                  bool isTTSEnabled)
{
    NB_GuidanceInformationConfiguration gparams;
    memset(&gparams, 0, sizeof(gparams));
    NB_GuidanceInformation* guidanceInfo = NULL;

    char* buf = NULL;
    nb_size size = 0;

    char* voiceBuf = NULL;
    nb_size voiceBufSize = 0;

    char* imagesBuf = NULL;
    nb_size imagesBufSize = 0;

    char* stepsBuf = NULL;
    nb_size stepsBufSize = 0;

    char* basicAudioBuf = NULL;
    nb_size basicAudioBufSize = 0;

    char* basicAudiolibBuf = NULL;
    nb_size basicAudiolibBufSize = 0;

    gparams.imageStyle = const_cast<char*>("routing-images");
    gparams.pronunStyle = const_cast<char*>(voiceStyle);
    gparams.stepsStyle = const_cast<char*>("directions");

    string workPath(workFolder);
    string basePath = workPath + "NK_" + locale + PATH_DELIMITER;

    string fullPath = basePath + "tpslib";
    NB_Error err = LoadFile(pal, fullPath, &buf, &size);
    if (err == NE_OK)
    {
        gparams.datalibBuffer = buf;
        gparams.datalibBufferSize = size;
    }

    if (err == NE_OK)
    {
        fullPath = basePath + "voices.tps";
        err = LoadFile(pal, fullPath, &voiceBuf, &voiceBufSize);
        if (err == NE_OK)
        {
            gparams.voiceBuffer = voiceBuf;
            gparams.voiceBufferSize = voiceBufSize;
        }
    }

    if (err == NE_OK)
    {
        fullPath = basePath + "directions.tps";
        err = LoadFile(pal, fullPath, &stepsBuf, &stepsBufSize);
        if (err == NE_OK)
        {
            gparams.stepsBuffer = stepsBuf;
            gparams.stepsBufferSize = stepsBufSize;
        }
    }

    if (err == NE_OK)
    {
        if (isTTSEnabled)
        {
            gparams.basicAudiolibBuffer = buf;
            gparams.basicAudiolibBufferSize = size;
        }
        else
        {
            fullPath = baseVoicePath;
            fullPath += "tpslib";
            err = LoadFile(pal, fullPath, &basicAudiolibBuf, &basicAudiolibBufSize);
            if (err == NE_OK && basicAudiolibBufSize > 0)
            {
                gparams.basicAudiolibBuffer = basicAudiolibBuf;
                gparams.basicAudiolibBufferSize = basicAudiolibBufSize;
            }
            else
            {
                //if not have that lib, use common tpslib
                gparams.basicAudiolibBuffer = buf;
                gparams.basicAudiolibBufferSize = size;
            }
        }
    }

    if (err == NE_OK)
    {
        if (isTTSEnabled)
        {
            fullPath = basePath + "TTS_basicaudio.tps";
        }
        else
        {
            fullPath = workPath + "NK_" + locale + "_audio" +
            PATH_DELIMITER + voiceStyle + PATH_DELIMITER + "basicaudio.tps";

        }
        err = LoadFile(pal, fullPath, &basicAudioBuf, &basicAudioBufSize);
        if (err == NE_OK)
        {
            gparams.basicAudioBuffer = basicAudioBuf;
            gparams.basicAudioBufferSize = basicAudioBufSize;
        }
    }

    string commonBasePath(workPath + "NK_common");
    commonBasePath = commonBasePath + PATH_DELIMITER;

    if (err == NE_OK)
    {
        fullPath = commonBasePath + "images.tps";
        err = LoadFile(pal, fullPath, &imagesBuf, &imagesBufSize);
        if (err == NE_OK)
        {
            gparams.imagesBuffer = imagesBuf;
            gparams.imagesBufferSize = imagesBufSize;
        }
    }

    //Load all images-[countryCode].tps files into parameter, just like "images-USA.tps".
    if (err == NE_OK)
    {
        PAL_FileEnum* fileEnum = NULL;
        PAL_FileEnumInfo info;
        memset(&info, 0, sizeof(info));
        char* dotAt = NULL;
        commonBasePath.assign(workPath + "NK_common");
        if (PAL_Ok == PAL_FileEnumerateCreate(pal, commonBasePath.c_str(), TRUE, &fileEnum))
        {
            commonBasePath = commonBasePath + PATH_DELIMITER;
            while(PAL_FileEnumerateNext(fileEnum, &info) != PAL_ErrNoData && err == NE_OK)
            {
                if (nsl_strncmp(info.filename, LOCALE_IMAGE_PREFIX, nsl_strlen(LOCALE_IMAGE_PREFIX)) == 0)
                {
                    ++gparams.localeImagesSize;
                    gparams.localeImages = (NB_GuidanceInformationConfigureImage*)nsl_realloc(
                        gparams.localeImages, sizeof(NB_GuidanceInformationConfigureImage) * gparams.localeImagesSize);
                    if (!gparams.localeImages)
                    {
                        err = NE_NOMEM;
                        break;
                    }
                    nsl_strlcpy(gparams.localeImages[gparams.localeImagesSize - 1].countryCode,
                                info.filename+nsl_strlen(LOCALE_IMAGE_PREFIX),
                                sizeof(gparams.localeImages[gparams.localeImagesSize - 1].countryCode));
                    dotAt = nsl_strchr(gparams.localeImages[gparams.localeImagesSize - 1].countryCode, '.');
                    if (dotAt)
                    {
                        *dotAt = '\0';
                    }
                    fullPath = commonBasePath + info.filename;
                    err = LoadFile(pal, fullPath,
                                   &gparams.localeImages[gparams.localeImagesSize - 1].imagesBuffer,
                                   &gparams.localeImages[gparams.localeImagesSize - 1].imagesBufferSize);
                }
            }
            PAL_FileEnumerateDestroy(fileEnum);
        }
    }

    if (err == NE_OK)
    {
        (void)NB_GuidanceInformationCreate(context, &gparams, &guidanceInfo);
    }

    if (buf)
    {
        nsl_free(buf);
    }
    if (voiceBuf)
    {
        nsl_free(voiceBuf);
    }
    if (imagesBuf)
    {
        nsl_free(imagesBuf);
    }
    if (stepsBuf)
    {
        nsl_free(stepsBuf);
    }
    if (basicAudioBuf)
    {
        nsl_free(basicAudioBuf);
    }
    if (basicAudiolibBuf)
    {
        nsl_free(basicAudiolibBuf);
    }
    if (gparams.localeImages)
    {
        do
        {
            --gparams.localeImagesSize;
            nsl_free(gparams.localeImages[gparams.localeImagesSize].imagesBuffer);
        }while (gparams.localeImagesSize > 0);
        nsl_free(gparams.localeImages);
    }

    return guidanceInfo;
}

void SetupRouteConfiguration(NB_RouteConfiguration* config, const Preferences* preference, bool enhancedStartup)
{
    nsl_memset(config, 0, sizeof(NB_RouteConfiguration));

    config->wantOriginLocation                = TRUE;
    config->wantDestinationLocation           = TRUE;
    config->wantRouteMap                      = FALSE;
    config->wantCrossStreets                  = FALSE;
    config->wantPolyline                      = FALSE;
    config->wantManeuvers                     = FALSE;
    config->wantFerryManeuvers                = TRUE;
    config->wantTowardsRoadInformation        = FALSE;
    config->wantIntersectionRoadInformation   = FALSE;
    config->wantCountryInformation            = TRUE;
    config->wantEnterRoundaboutManeuvers      = TRUE;
    config->wantEnterCountryManeuvers         = TRUE;
    config->maxPronunFiles                    = 6;
    config->wantRouteExtents                  = TRUE;
    config->wantMaxTurnDistanceForStreetCount = TRUE;
    config->wantUnpavedRoadInformation        = TRUE;

    // enhanced content disabled by default
    config->wantCityModels     = FALSE;
    config->wantJunctionModels = FALSE;
    config->wantRealisticSigns = preference->GetRealisticSignEnabled();

    // want speed camera request disabled by default
    config->wantSpeedCameras = FALSE;

    config->wantGuidanceConfig = TRUE;
    config->wantExitNumbers = TRUE;
    config->wantNaturalGuidance = preference->GetNaturalGuidanceEnabled();
    config->wantExtendedNaturalGuidance = preference->GetNaturalGuidanceEnabled();
    config->wantLaneGuidance = preference->GetLaneGuidanceEnabled();
    config->wantAlternateRoutes = preference->GetMultipleRoutesEnabled();
    config->wantLabelPoints = preference->GetMultipleRoutesEnabled();
    config->wantPlayTimes = TRUE;
    config->wantCompleteRoute = TRUE;
    config->wantBridgeManeuvers = preference->IsBridgeManeuversEnabled() ? TRUE : FALSE;
    config->wantTunnelManeuvers = preference->IsTunnelManeuversEnabled() ? TRUE : FALSE;
    if (preference->GetRecalcCriteria() == Preferences::Tight)
    {
        config->routeDetectionLevel = NBRDL_Tight;
    }

    if (!preference->GetDownloadableAudioEnabled())
    {
        config->maxPronunFiles = 0;
        nsl_strlcpy(config->supportedPhoneticsFormats, "IPA", sizeof(config->supportedPhoneticsFormats));
    }

    // want speed limit request when any preference speed limit is enabled
    config->wantSpeedLimit = preference->IsSpeedLimitAlertEnabled() ||
    preference->IsSpeedLimitSignEnabled() ||
    preference->IsSchoolZoneAlertEnabled();

    config->wantSpecialRegions = TRUE;

    config->wantFirstManeuverCrossStreets = TRUE;
    config->orManeuverTurnAngleTolerance = preference->GetOriginManeuverTurnAngleTolerance();
    config->wantSecondaryNameForExits = preference->IsSecondaryNameForExits() ? TRUE : FALSE;
    config->wantCheckUnsupportedCountries = preference->IsCheckUnsupportedCountries() ? TRUE : FALSE;

    // send the want-first-major-road when enhanced startup
    config->wantFirstMajorRoad = preference->GetEnableEnhancedNavigationStartup() && enhancedStartup;
    config->wantPedestrianManeuvers = preference->GetWantPedestrianManeuvers();
    config->viaPointThreshold = preference->GetViaPointThreshold();
    config->announcementHeadsUpInterval = preference->GetAnnouncementHeadsUpInterval();

}

void SetupStaticRouteConfiguration(NB_RouteConfiguration* config, const Preferences* preference)
{
    nsl_memset(config, 0, sizeof(NB_RouteConfiguration));

    config->wantOriginLocation                = TRUE;
    config->wantDestinationLocation           = TRUE;
    config->wantRouteMap                      = FALSE;
    config->wantCrossStreets                  = FALSE;
    config->wantPolyline                      = FALSE;
    config->wantManeuvers                     = TRUE;
    config->wantFerryManeuvers                = TRUE;
    config->wantTowardsRoadInformation        = FALSE;
    config->wantIntersectionRoadInformation   = FALSE;
    config->wantCountryInformation            = TRUE;
    config->wantEnterRoundaboutManeuvers      = TRUE;
    config->wantEnterCountryManeuvers         = TRUE;
    config->maxPronunFiles                    = 0;
    config->wantRouteExtents                  = TRUE;
    config->wantMaxTurnDistanceForStreetCount = FALSE;
    config->wantUnpavedRoadInformation        = TRUE;
    config->wantCityModels                    = FALSE;
    config->wantJunctionModels                = FALSE;
    config->wantRealisticSigns                = FALSE;
    config->wantSpeedLimit                    = FALSE;
    config->wantSpeedCameras                  = FALSE;
    config->wantGuidanceConfig                = FALSE;
    config->wantExitNumbers                   = TRUE;
    config->wantNaturalGuidance               = FALSE;
    config->wantExtendedNaturalGuidance       = FALSE;
    config->wantLaneGuidance                  = FALSE;
    config->wantLabelPoints                   = TRUE;
    config->wantPlayTimes                     = FALSE;
    config->wantCompleteRoute                 = TRUE;
    config->routeDetectionLevel               = NBRDL_Tight;
    config->wantSpecialRegions                = FALSE;
    config->disableNavigation                 = TRUE;

    config->wantAlternateRoutes = preference->GetMultipleRoutesEnabled();
    config->wantBridgeManeuvers = preference->IsBridgeManeuversEnabled() ? TRUE : FALSE;
    config->wantTunnelManeuvers = preference->IsTunnelManeuversEnabled() ? TRUE : FALSE;
    if (!preference->GetDownloadableAudioEnabled())
    {
        config->maxPronunFiles = 0;
        nsl_strlcpy(config->supportedPhoneticsFormats, "IPA", sizeof(config->supportedPhoneticsFormats));
    }
}

void SetupRouteSummaryConfiguration(NB_RouteConfiguration* config)
{
    nsl_memset(config, 0, sizeof(NB_RouteConfiguration));
    config->wantOriginLocation      = TRUE;
    config->wantDestinationLocation = TRUE;
    config->wantManeuvers           = TRUE;
    config->disableNavigation       = TRUE;
    config->wantCompleteRoute       = TRUE;
    config->wantRouteSummary        = TRUE;
    config->wantRouteSummaryDelayTime = TRUE;
    config->routeDetectionLevel     = NBRDL_Tight;
}

void SetupRouteOptions(NB_RouteOptions* options, const RouteOptions* routeOptions,
                       const Preferences* preference, const char* voiceStyle)
{
    nsl_memset(options, 0, sizeof(NB_RouteOptions));

    switch (preference->NavTrafficFor())
    {
        case Preferences::TrafficForNavigation:
            options->traffic = NB_TrafficType_Avoid;
            break;
        case Preferences::TrafficForAlert:
            options->traffic = NB_TrafficType_Warn;
            break;
        default:
            options->traffic = NB_TrafficType_None;
            break;
    }
    if (preference->GetDownloadableAudioEnabled())
    {
        options->pronunStyle = const_cast<char*>(voiceStyle);
    }

    switch (routeOptions->GetRouteType())
    {
        case Fastest:
            options->type = NB_RouteType_Fastest;
            break;
        case Shortest:
            options->type = NB_RouteType_Shortest;
            break;
        case Easiest:
            options->type = NB_RouteType_Easiest;
            break;
        default:
            options->type = NB_RouteType_Fastest;
            break;
    }

    switch (routeOptions->GetTransportationMode())
    {
        case Car:
            options->transport = NB_TransportationMode_Car;
            break;
        case Truck:
            options->transport = NB_TransportationMode_Truck;
            break;
        case Bicycle:
            options->transport = NB_TransportationMode_Bicycle;
            break;
        case Pedestrian:
            options->transport = NB_TransportationMode_Pedestrian;
            break;
        default:
            options->transport = NB_TransportationMode_Car;
            break;
    }

    uint32 avoid = 0;
    if (routeOptions->GetAvoidFeatures() & AVD_HOV)
    {
        avoid |= NB_RouteAvoid_HOV;
    }
    if (routeOptions->GetAvoidFeatures() & AVD_Toll)
    {
        avoid |= NB_RouteAvoid_Toll;
    }
    if (routeOptions->GetAvoidFeatures() & AVD_Highway)
    {
        avoid |= NB_RouteAvoid_Highway;
    }
    if (routeOptions->GetAvoidFeatures() & AVD_Ferry)
    {
        avoid |= NB_RouteAvoid_Ferry;
    }
    if (routeOptions->GetAvoidFeatures() & AVD_Unpaved)
    {
        avoid |= NB_RouteAvoid_Unpaved;
    }
    options->avoid = (NB_RouteAvoid)avoid;
}

void SetupLaneGuidanceConfiguration(NB_Context* context, NB_RouteParameters* pRouteParameters, const char* workpath)
{
    char* datalib = NULL;
    nb_size datalibSize = 0;
    char* configBuffer = NULL;
    nb_size configBufferSize = 0;
    string path(workpath);
    string basePath(path + "NK_common");
    basePath = basePath + PATH_DELIMITER;
    string fullPath = basePath + "tpslib";
    PAL_Instance* pal = NB_ContextGetPal(context);
    NB_Error error = LoadFile(pal, fullPath, &datalib, &datalibSize);

    if (error != NE_OK)
    {
        return;
    }

    fullPath = basePath + "ttfmapping.tps";
    error = LoadFile(pal, fullPath, &configBuffer, &configBufferSize);
    if (error == NE_OK)
    {
        NB_RouteParametersSetFontMapFromBuffer(context, pRouteParameters, (char *)datalib,
                                               datalibSize, (char *)configBuffer, configBufferSize);
    }
    if (configBuffer)
    {
        nsl_free(configBuffer);
    }
    fullPath = basePath + "hovmapping.tps";
    error = LoadFile(pal, fullPath, &configBuffer, &configBufferSize);
    if (error == NE_OK)
    {
        NB_RouteParametersSetHOVFontMapFromBuffer(context, pRouteParameters, (char *)datalib, datalibSize,
                                                  (char *)configBuffer, configBufferSize);
    }
    if (configBuffer)
    {
        nsl_free(configBuffer);
    }
    fullPath = basePath + "distance.tps";
    error = LoadFile(pal,fullPath, &configBuffer, &configBufferSize);
    if (error == NE_OK)
    {
        NB_RouteParametersSetConfigurationFromBuffer(context, pRouteParameters, (char*)datalib, datalibSize, (char*)configBuffer, configBufferSize);
    }
    if (configBuffer)
    {
        nsl_free(configBuffer);
    }
    
    if (datalib)
    {
        nsl_free(datalib);
    }
}

void AddToneIntoVoiceCache(PAL_Instance* m_pal, NB_Context* context, const char* workpath)
{
    string path(workpath);
    string commonBasePath(path + "NK_common");
    commonBasePath += PATH_DELIMITER;
    commonBasePath += "tones";

    // get voice cache from context.
    CSL_Cache* voiceCache = NB_ContextGetVoiceCache(context);
    if (!voiceCache)
    {
        return;
    }

    PAL_FileEnum* fileEnum = NULL;
    if(PAL_FileEnumerateCreate(m_pal, commonBasePath.c_str(), TRUE, &fileEnum) != PAL_Ok)
    {
        return;
    }

    PAL_FileEnumInfo fileInfo;
    memset(&fileInfo, 0, sizeof(fileInfo));
    string fullPath;

    // enumerate tone files and add them into voice cache.
    char* data = NULL;
    nb_size size = 0;
    while(PAL_FileEnumerateNext(fileEnum, &fileInfo) == PAL_Ok)
    {
        data = NULL;
        size = 0;
        fullPath = commonBasePath + PATH_DELIMITER + fileInfo.filename;
        if (LoadFile(m_pal, fullPath, &data, &size) == NE_OK && data && size>0)
        {
            // don't need to free buffer when cache add successfully for it is taken ownership by cache.
            // minus 4 length to ignore the ".aac" suffix
            if (CSL_CacheAdd(voiceCache, (byte*)fileInfo.filename, nsl_strlen(fileInfo.filename) - 4,
                             (byte*)data, size, TRUE, FALSE, 0) != NE_OK)
            {
                nsl_free(data);
            }
        }
    }
    PAL_FileEnumerateDestroy(fileEnum);
}

void SetupNavigationConfiguration(NB_NavigationConfiguration* configuration, const Preferences* preference)
{
    nsl_memset(configuration, 0 , sizeof(NB_NavigationConfiguration));

    configuration->gpsFixMinimumSpeed           = preference->GetSpeedThresholdForGpsHeading();
    configuration->startupRegionSizeFactor      = 1.5;
    configuration->filteredGpsMinimumSpeed      = 5.5;
    configuration->filteredGpsConstant          = 0.97;
    configuration->trafficNotifyMaximumRetries  = 3;
    configuration->trafficNotifyDelay           = 15 * 1000;
    configuration->completeRouteMaximumAttempts = 2;
    configuration->completeRouteRequestDelay    = 5 * 1000;
    configuration->pronunRequestDelay           = 15 * 1000;
    configuration->cameraSearchDelay            = 15 * 1000;
    configuration->cameraSearchDistance         = 99999999;
    configuration->startupRegionMinimumSize     = 150;
    configuration->excessiveRecalcMaximumCount  = 3;
    configuration->excessiveRecalcTimePeriod    = 0; //turn off the confirm recal.
    configuration->filteredGpsEnabled           = FALSE;
    configuration->cameraSearchEnabled          = FALSE;
    configuration->offRouteIgnoreCount          = preference->GetOffRouteIgnoreCount();
    configuration->nameChangedManeuverEnabled   = preference->IsNeedNCManeuver() ? TRUE : FALSE;
    configuration->m1ManeuverEnabled            = preference->IsNeedM1Maneuver() ? TRUE : FALSE;
    configuration->continueOnManeuverEnabled    = preference->IsNeedCOManeuver() ? TRUE : FALSE;
}

void SetupForPedestrianAndBike(NB_RouteOptions* options, NB_RouteConfiguration* config,
                               const RouteOptions* routeOption)
{
    if (routeOption->GetTransportationMode() == Pedestrian)
    {
        if (config)
        {
            config->wantExitNumbers = FALSE;
            config->wantGuidanceConfig = FALSE;
            config->wantRealisticSigns = FALSE;
            config->wantSpecialRegions = FALSE;
            config->wantSpeedLimit = FALSE;
            config->wantNaturalGuidance = FALSE;
            config->wantLaneGuidance = FALSE;
            config->wantExtendedNaturalGuidance = FALSE;
            config->wantMaxTurnDistanceForStreetCount = FALSE;
        }

        if (options)
        {
            options->traffic = NB_TrafficType_None;
        }
    }

    if (routeOption->GetTransportationMode() == Bicycle)
    {
        if (config)
        {
            config->wantExitNumbers = FALSE;
            config->wantRealisticSigns = FALSE;
            config->wantSpecialRegions = FALSE;
            config->wantSpeedLimit = FALSE;
            config->wantLaneGuidance = FALSE;
        }
    }
}

static const uint32 MAX_PATH_LENGTH = 256;
static const uint32 SCREEN_WIDTH    = 780;
static const uint32 SCREEN_HEIGHT   = 585;

static const char NBGM_VERSION[] = "1";

static const char ECM_FILE_FORMAT[] = "BIN";
static const char MJO_FILE_FORMAT[] = "BIN";
static const char SAR_FILE_FORMAT[] = "PNG";
static const char SPD_FILE_FORMAT[] = "PNG";
static const char TEXTURE_FILE_FORMAT[] = "PNG";
static const char COMPRESSED_TEXTURE_FILE_FORMAT[] = "zip";
static const char SPR_FILE_FORMAT[] = "PNG";

static const char CITYMODEL_PATH[] = "Citymodels";
static const char MJO_PATH[] = "MJO";
static const char SAR_PATH[] = "SAR";
static const char SPD_PATH[] = "SPD";
static const char TEXTURE_PATH[] = "Textures";
static const char SPR_PATH[] = "SPR";
static const char RESOURCE_DIRECTORY[] = "Nav";

static const char DEFAULT_COUNTRY[] = "USA";

void SetupEnhancedContentManagerConfiguration(NB_Context* context, NB_EnhancedContentManagerConfiguration* config,
                                              const char* workpath)
{
    PAL_Instance* pal = NB_ContextGetPal(context);
    const char* basePath = NULL;
    const char* m_locale = NB_ContextGetLocale(context);
    char fullPath[MAX_PATH_LENGTH] = { 0 };

    config->screenWidth = SCREEN_WIDTH;
    config->screenHeight = SCREEN_HEIGHT;
    config->screenResolution = SCREEN_WIDTH * SCREEN_HEIGHT;
    config->maxRealisticSignsCacheSize     = 100;
    config->maxMotorwayJunctionsCacheSize  = 100;
    config->maxSpeedLimitImagesCacheSize   = 100;
    config->maxSpecialRegionsCacheSize     = 100;

    config->enhancedCityModelEnabled = TRUE;
    config->motorwayJunctionObjectsEnabled = TRUE;
    config->realisticSignsEnabled = TRUE;
    config->speedLimitsSignEnabled = TRUE;

    nsl_strlcpy(config->country, DEFAULT_COUNTRY, sizeof(config->country));
    nsl_strlcpy(config->language, m_locale, sizeof(config->language));
    nsl_strlcpy(config->nbgmVersion, NBGM_VERSION, sizeof(config->nbgmVersion));

    basePath = workpath;
    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    nsl_strlcpy(config->persistentMetadataPath, fullPath, sizeof(config->persistentMetadataPath));

    // Add this path to nbcontext so it can be cleared when doing master clear...
    NB_ContextAddCachePath(context, fullPath);

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    PAL_FileAppendPath(pal, fullPath, sizeof(fullPath), CITYMODEL_PATH);
    nsl_strlcpy(config->enhancedCityModelPath, fullPath, sizeof(config->enhancedCityModelPath));

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    PAL_FileAppendPath(pal, fullPath, sizeof(fullPath), MJO_PATH);
    nsl_strlcpy(config->motorwayJunctionObjectPath, fullPath, sizeof(config->motorwayJunctionObjectPath));

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    PAL_FileAppendPath(pal, fullPath, sizeof(fullPath), TEXTURE_PATH);
    nsl_strlcpy(config->texturesPath, fullPath, sizeof(config->texturesPath));

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    PAL_FileAppendPath(pal, fullPath, sizeof(fullPath), SAR_PATH);
    nsl_strlcpy(config->realisticSignsPath, fullPath, sizeof(config->realisticSignsPath));

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    PAL_FileAppendPath(pal, fullPath, sizeof(fullPath), SPD_PATH);
    nsl_strlcpy(config->speedLimitsSignsPath, fullPath, sizeof(config->speedLimitsSignsPath));

    nsl_memset(fullPath, 0, sizeof(fullPath));
    nsl_strlcpy(fullPath, basePath, sizeof(fullPath));
    nsl_strlcat(fullPath, RESOURCE_DIRECTORY, sizeof(fullPath));
    PAL_FileAppendPath(pal, fullPath, sizeof(fullPath), SPR_PATH);
    nsl_strlcpy(config->specialRegionsPath, fullPath, sizeof(config->specialRegionsPath));

    nsl_strlcpy(config->enhancedCityModelFileFormat, ECM_FILE_FORMAT, sizeof(config->enhancedCityModelFileFormat));
    nsl_strlcpy(config->motorwayJunctionObjectFileFormat, MJO_FILE_FORMAT, sizeof(config->motorwayJunctionObjectFileFormat));
    nsl_strlcpy(config->realisticSignsFileFormat, SAR_FILE_FORMAT, sizeof(config->realisticSignsFileFormat));
    nsl_strlcpy(config->speedLimitsSignFileFormat, SPD_FILE_FORMAT, sizeof(config->speedLimitsSignFileFormat));
    nsl_strlcpy(config->textureFileFormat, TEXTURE_FILE_FORMAT, sizeof(config->textureFileFormat));
    nsl_strlcpy(config->compressedTextureFileFormat, COMPRESSED_TEXTURE_FILE_FORMAT, sizeof(config->compressedTextureFileFormat));
    nsl_strlcpy(config->specialRegionsFileFormat, SPR_FILE_FORMAT, sizeof(config->specialRegionsFileFormat));
}

NB_EnhancedContentManager* CreateEnhancedContentManager(NB_Context* context, NB_RouteConfiguration* routeConfiguration,
                                                        const char* workpath)
{
    if (routeConfiguration->wantCityModels || routeConfiguration->wantJunctionModels || routeConfiguration->wantRealisticSigns)
    {
        NB_EnhancedContentManagerConfiguration config;
        memset(&config, 0, sizeof(config));
        SetupEnhancedContentManagerConfiguration(context, &config, workpath);

        config.enhancedCityModelEnabled = routeConfiguration->wantCityModels;
        config.motorwayJunctionObjectsEnabled = routeConfiguration->wantJunctionModels;
        config.realisticSignsEnabled = routeConfiguration->wantRealisticSigns;
        config.specialRegionsEnabled = routeConfiguration->wantSpecialRegions;

        NB_EnhancedContentSynchronizationCallback synchronizationCallback = {0};
        NB_EnhancedContentDownloadCallback downloadCallback = {0};
        NB_EnhancedContentManager* manager = NULL;

        if (NB_EnhancedContentManagerCreate(context, &config, &synchronizationCallback, &downloadCallback, &manager) == NE_OK)
        {
            NB_ContextSetEnhancedContentManagerNoOwnershipTransfer(context, manager);
        }
        return manager;
    }
    return NULL;
}

void SetupEnhancedContentMapConfiguration(NB_EnhancedContentMapConfiguration* enhancedConfiguration)
{
    /*  From SDS NavData-MapPerspective-vx-1.3

     Map perspective Z2, used for rendering the ECM, shall be defined as follows:

     Portrait Orientation:
     Camera Height (AB): 150 meters
     Camera to Avatar (BF): 12 meters
     Avatar to Horizon (FG): 750 meters
     Bottom to Avatar (DH):  70 pixels
     Horizontal Field-of-View: 55 degrees
     Avatar Scale: 4

     Landscape Orientation:
     Camera Height (AB): 150 meters
     Camera to Avatar (BF): 12 meters
     Avatar to Horizon (FG): 380 meters
     Bottom to Avatar (DH):  70 pixels
     Horizontal Field-of-View: 90 degrees
     Avatar Scale: 4
     */

    // Initialize configuration parameters to default values
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].cameraHeightMeters                = 150;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].cameraToAvatarHorizontalMeters    = 12;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].avatarToHorizonHorizontalMeters   = 750;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].avatarToMapBottomPixel            = 70;
    enhancedConfiguration->cameraProjection[NB_MO_Portrait].horizontalFieldOfViewDegrees      = 55;

    enhancedConfiguration->cameraProjection[NB_MO_Landscape].cameraHeightMeters               = 150;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].cameraToAvatarHorizontalMeters   = 12;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].avatarToHorizonHorizontalMeters  = 380;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].avatarToMapBottomPixel           = 70;
    enhancedConfiguration->cameraProjection[NB_MO_Landscape].horizontalFieldOfViewDegrees     = 90;

    enhancedConfiguration->waitBetweenCityModelDisplaySeconds = 15;
    enhancedConfiguration->maximumJunctionViewDistanceMeters  = 200;

    enhancedConfiguration->mapFieldWidthPixel     = 480;
    enhancedConfiguration->mapFieldHeightPixel    = 758;
}

NB_Error LoadFile(PAL_Instance* m_pal, string fullName, char** pbuf, nb_size* psize)
{
    return (PAL_FileLoadFile(m_pal, fullName.c_str(), (unsigned char**)pbuf, (uint32*)psize) == PAL_Ok) ? NE_OK : NE_INVAL;
}

Place NbPlaceToPlace(const NB_Place& nbPlace)
{
    MapLocation location;
    location.airport.assign(nbPlace.location.airport);
    location.areaname.assign(nbPlace.location.areaname);
    location.number.assign(nbPlace.location.streetnum);
    location.street.assign(nbPlace.location.street1);
    location.street2.assign(nbPlace.location.street2);
    location.city.assign(nbPlace.location.city);
    location.county.assign(nbPlace.location.county);
    location.country.assign(nbPlace.location.country);
    location.state.assign(nbPlace.location.state);
    location.postal.assign(nbPlace.location.postal);
    location.freeform.assign(nbPlace.location.freeform);

    location.center.latitude = nbPlace.location.latitude;
    location.center.longitude = nbPlace.location.longitude;

    Place place(string(nbPlace.name), location);

    for (int i = 0; i < nbPlace.numcategory; ++i)
    {
        Category category;
        category.code.assign(nbPlace.category[i].code);
        category.name.assign(nbPlace.category[i].name);
        place.AddCategory(category);
    }

    for (int i = 0; i < nbPlace.numphone; ++i)
    {
        Phone phone;
        phone.country.assign(nbPlace.phone[i].country);
        phone.area.assign(nbPlace.phone[i].area);
        phone.number.assign(nbPlace.phone[i].number);

        switch (nbPlace.phone[i].type)
        {
            case NB_Phone_Primary:
                phone.type = Primary;
                break;
            case NB_Phone_Secondary:
                phone.type = Secondary;
                break;
            case NB_Phone_National:
                phone.type = National;
                break;
            case NB_Phone_Mobile:
                phone.type = Mobile;
                break;
            case NB_Phone_Fax:
                phone.type = Fax;
                break;
            default:
                phone.type = Primary;
                break;
        }
        place.AddPhoneNumber(phone);
    }

    return place;
}

void PlaceToNbPlace(const Place& place, NB_Place* nbPlace)
{
    nsl_memset(nbPlace, 0, sizeof(NB_Place));

    place.GetName().copy(nbPlace->name, NB_PLACE_NAME_LEN);

    nbPlace->numcategory = MIN(place.GetCategoriesCount(), NB_PLACE_NUM_CAT);
    for (int i = 0; i < nbPlace->numcategory; ++i)
    {
        Category category;
        if (place.GetCategory(i, category) == NE_OK)
        {
            category.code.copy(nbPlace->category[i].code, NB_CATEGORY_CODE_LEN);
            category.name.copy(nbPlace->category[i].name, NB_CATEGORY_NAME_LEN);
        }
    }

    nbPlace->numphone = MIN(place.GetPhoneNumberCount(), NB_PLACE_NUM_PHONE);
    for (int i = 0; i < nbPlace->numphone; ++i)
    {
        Phone phone;
        if (place.GetPhoneNumber(i, phone) == NE_OK)
        {
            phone.country.copy(nbPlace->phone[i].country, NB_PHONE_COUNTRY_LEN);
            phone.area.copy(nbPlace->phone[i].area, NB_PHONE_AREA_LEN);
            phone.number.copy(nbPlace->phone[i].number, NB_PHONE_NUMBER_LEN);

            switch (phone.type)
            {
                case Primary:
                    nbPlace->phone[i].type = NB_Phone_Primary;
                    break;
                case Secondary:
                    nbPlace->phone[i].type = NB_Phone_Secondary;
                    break;
                case National:
                    nbPlace->phone[i].type = NB_Phone_National;
                    break;
                case Mobile:
                    nbPlace->phone[i].type = NB_Phone_Mobile;
                    break;
                case Fax:
                    nbPlace->phone[i].type = NB_Phone_Fax;
                    break;
                default:
                    nbPlace->phone[i].type = NB_Phone_Primary;
                    break;
            }
        }
    }

    MapLocation location = place.GetLocation();
    location.airport.copy(nbPlace->location.airport, NB_LOCATION_MAX_AIRPORT_LEN);
    location.areaname.copy(nbPlace->location.areaname, NB_LOCATION_MAX_AREANAME_LEN);
    location.number.copy(nbPlace->location.streetnum, NB_LOCATION_MAX_STREETNUM_LEN);
    location.street.copy(nbPlace->location.street1, NB_LOCATION_MAX_STREET_LEN);
    location.street2.copy(nbPlace->location.street2, NB_LOCATION_MAX_STREET_LEN);
    location.city.copy(nbPlace->location.city, NB_LOCATION_MAX_CITY_LEN);
    location.county.copy(nbPlace->location.county, NB_LOCATION_MAX_COUNTY_LEN);
    location.country.copy(nbPlace->location.country, NB_LOCATION_MAX_COUNTRY_LEN);
    location.state.copy(nbPlace->location.state, NB_LOCATION_MAX_STATE_LEN);
    location.postal.copy(nbPlace->location.postal, NB_LOCATION_MAX_POSTAL_LEN);
    location.freeform.copy(nbPlace->location.freeform, NB_LOCATION_MAX_FREE_FORM_LEN);
    nbPlace->location.latitude = location.center.latitude;
    nbPlace->location.longitude = location.center.longitude;
    nbPlace->location.type = NB_Location_None;
}

void ConfigNBNavigation(NB_Navigation* nbNavigation, const Preferences& preference, NB_NavigateAnnouncementUnits units)
{
    // Set announce type with preference measurement setting.
    NB_NavigationSetAnnounceType(nbNavigation, NB_NAT_Street, units);

    // Set enhanced content configuration parameters for navigating with enhanced content
    if (preference.GetRealisticSignEnabled())
    {
        NB_EnhancedContentMapConfiguration enhancedConfig;
        nsl_memset(&enhancedConfig, 0, sizeof(enhancedConfig));
        SetupEnhancedContentMapConfiguration(&enhancedConfig);

        (void)NB_NavigationSetEnhancedContentMapConfiguration(nbNavigation, &enhancedConfig);
    }

    (void)NB_NavigationSetTrafficPollingInterval (nbNavigation, TRAFFIC_POLLING_INTERVAL);

    NB_SpeedLimitConfiguration speedLimitConfig;
    nsl_memset(&speedLimitConfig, 0, sizeof(speedLimitConfig));
    speedLimitConfig.displaySpeedLimit    = preference.IsSpeedLimitSignEnabled() ? TRUE : FALSE;
    speedLimitConfig.schoolAlert          = preference.IsSchoolZoneAlertEnabled() ? TRUE : FALSE;
    speedLimitConfig.schoolWarningType    = SP_Tone;
    speedLimitConfig.speedAlert           = preference.IsSpeedLimitAlertEnabled() ? TRUE : FALSE;
    speedLimitConfig.speedWarningType     = SP_Tone;
    speedLimitConfig.speedWarningLevel    = preference.GetSpeedingWarningLevel();
    speedLimitConfig.speedingDisplayAlert = preference.GetSpeedingDisplayAlertEnabled() ? TRUE : FALSE;

    NB_NavigationSetSpeedLimitConfiguration(nbNavigation, &speedLimitConfig);

    NB_TrafficSetting trafficSetting;
    trafficSetting.trafficAnnouncementEnabled = preference.IsTrafficAnnouncements() ? TRUE : FALSE;
    trafficSetting.trafficIncidentAnnouncementEnabled = preference.IsTrafficIncidentAnnouncements() ? TRUE : FALSE;
    NB_NavigationSetTrafficSetting(nbNavigation, &trafficSetting);
}
}
