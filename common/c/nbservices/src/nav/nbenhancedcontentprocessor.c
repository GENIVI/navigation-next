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

    @file     nbenhancedcontentprocessor.c
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

#include "nbenhancedcontentprocessorprivate.h"
#include "nbenhancedcontentmanagerprotected.h"
#include "nbnavigationstateprivate.h"
#include "nbrouteparametersprivate.h"
#include "nbrouteinformationprivate.h"
#include "nbcontextprotected.h"
#include "nbcontextaccess.h"
#include "nbqalog.h"
#include "palclock.h"
#include "palmath.h"
#include "transformutility.h"
#include "spatialvector.h"
#include "navtypes.h"
#include "data_polyline.h"
#include "nbrouteinformationprivate.h"


// Local types ...................................................................................

/*! The prefetch entry is used for the prefetch city tile logic. We want to first sort the vectors
    by the tile-index in order to efficiently remove duplicate tile indices. But we want to keep
    the original order by distance from the start of the route so that we can request the city tiles
    by the order of the route (tiles at the beginning of the route should get prefetched first).
 
    @see PrefetchCityModelTiles
*/
typedef struct
{
    int tileIndex;          /*!< City tile (ECM) index for the region */
    int distanceIndex;      /*!< We increment this number when we calculate the prefetch tiles starting from the beginning of the route */

} PrefetchEntry;


// Local functions ...............................................................................

static NB_Error ProcessEnhancedContentOnPositionChange(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState);
static void ProcessEnhancedContentStateOnManeuverChange(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState);

static nb_boolean PrefetchRealisticSigns(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route, uint32 startManeuver, uint32 maneuverCount);
static nb_boolean PrefetchMotorwayJunctions(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route, uint32 startManeuver, uint32 maneuverCount);
static nb_boolean PrefetchEnhancedContentData(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType, const char* dataset, const char* dataId, const char* version);

static void UpdateRealisticSignInformation(NB_EnhancedContentState* enhancedContentState,  NB_NavigationState* navigationState);
static void ClearRealisticSignInformation(NB_EnhancedContentState* enhancedContentState,  NB_NavigationState* navigationState);


// City/ECM and MJO calculations .................................................................

static NB_Error ProcessMapFrameData(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState);
static NB_Error LoadRegionsIfNecessary(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route, nb_boolean* prefetchCityTiles);
static void FreeRoute(NB_RouteId* route);
static void CopyRoute(NB_RouteId* destination, const NB_RouteId* source);

static void InitializeMapConfiguration(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState);
static void InitializeCameraProjectionParameters(CameraProjectionParameters* parameters, const NB_CameraProjectionParameters* projectionParameters, int mapFieldWidthPixel, int mapFieldHeightPixel);
static void SetupCameraProjectionContext(NB_EnhancedContentState* enhancedContentState, double latitude, double heading);
static void SetupViewingPlane(NB_EnhancedContentState* enhancedContentState, double userLatitude, double userLongitude);

static NB_Error ProcessContentRegions(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route, double distanceFromStart, const NB_LatitudeLongitude userPosition);
static void FreeContentRegions(NB_EnhancedContentState* enhancedContentState);
static void QaLogEnhancedContentRegionState(NB_Context* context, const NB_RouteContentRegion* region, NB_QaLogEnhancedContentRegionState state, const NB_LatitudeLongitude userPosition);
static ContentRegion* GetCurrentContentRegion(NB_EnhancedContentState* enhancedContentState);

static NB_Error GetMapFrameSplines(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route);
static void FreeMapFrameSplines(NB_EnhancedContentState* enhancedContentState);
static nb_boolean CanShowRegion(NB_EnhancedContentState* enhancedContentState, const NB_RouteContentRegion* region, double distanceFromStart, uint32 endManeuverIndex);
static nb_boolean IsPointVisible(NB_EnhancedContentState* enhancedContentState, double latitude, double longitude);

static nb_boolean IsTileInArea(const NB_Vector* area, int pointCount, const NB_CityBoundingBox* box);
static nb_boolean GetTileArray(const NB_Vector* area,
                               int pointCount,
                               const CityParameters* cityParameters,
                               uint32* leftColumn,
                               uint32* rightColumn,
                               uint32* topRow,
                               uint32* bottomRow,
                               nb_boolean validateBoundaries);

static nb_boolean UpdateMapFrameData(NB_EnhancedContentState* enhancedContentState);
static nb_boolean AreTileVectorsEqual(CSL_Vector* tileVector1, CSL_Vector* tileVector2);
static void FreeMapFrameTiles(NB_EnhancedContentState* enhancedContentState);
static nb_boolean GetCityModelTiles(NB_EnhancedContentState* enhancedContentState, CSL_Vector* tiles);
static nb_boolean GetCityParametersIfNecessary(NB_EnhancedContentState* enhancedContentState, const NB_RouteContentRegion* region);
static void FreeCityParameters(NB_EnhancedContentState* enhancedContentState);
static uint32 ConvertCityTileColumnRowToIndex(uint32 column, uint32 row, uint32 rowCount);

static NB_Error PrefetchCityModelTiles(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route);
static NB_Error PrefetchSegment(double startLatitude,
                            double startLongitude,
                            double endLatitude,
                            double endLongitude,
                            double segmentExtensionLength,
                            double segmentPrefetchWindowWidth,
                            const CityParameters* cityParameters,
                            CSL_Vector* cityTiles,
                            int* distanceIndex);
static int TileIndexCompareFunction(const void* left, const void* right);
static int DistanceIndexCompareFunction(const void* left, const void* right);
static int IndexCompareFunction(const void* left, const void* right);
static int RequestCityTile(void* userData, void* item);


// Public Functions ..............................................................................

NB_DEF NB_Error
NB_EnhancedContentStateGetRealisticSign(NB_EnhancedContentState* enhancedContentState, NB_RealisticSign* realisticSign)
{
    if (!enhancedContentState || !realisticSign)
    {
        return NE_INVAL;
    }

    nsl_memset(realisticSign, 0, sizeof(NB_RealisticSign));
    if (!enhancedContentState->currentSign.data)
    {
        return NE_NOENT;
    }

    *realisticSign = enhancedContentState->currentSign;
    if (realisticSign->dataSize > 0)
    {
        realisticSign->data = nsl_malloc(realisticSign->dataSize);
        if (!realisticSign->data)
        {
            return NE_NOMEM;
        }
        nsl_memcpy(realisticSign->data, enhancedContentState->currentSign.data, realisticSign->dataSize);
    }
    if (realisticSign->portraitDataSize)
    {
        realisticSign->portraitData = nsl_malloc(realisticSign->portraitDataSize);
        if (!realisticSign->portraitData)
        {
            return  NE_NOMEM;
        }
        nsl_memcpy(realisticSign->portraitData, enhancedContentState->currentSign.portraitData, realisticSign->portraitDataSize);
    }

    return NE_OK;
}


NB_DEF NB_Error
NB_EnhancedContentStateFreeRealisticSign(NB_EnhancedContentState* enhancedContentState, NB_RealisticSign* realisticSign)
{
    if (!enhancedContentState || !realisticSign)
    {
        return NE_INVAL;
    }

    if (realisticSign->data)
    {
        nsl_free(realisticSign->data);
    }

    if (realisticSign->portraitData)
    {
        nsl_free(realisticSign->portraitData);
    }

    nsl_memset(realisticSign, 0, sizeof(NB_RealisticSign));

    return NE_OK;
}


NB_DEF NB_Error
NB_EnhancedContentStateGetMapFrameTileIds(NB_EnhancedContentState* enhancedContentState, uint32* tileCount, const char*** tileIds)
{
    if (!enhancedContentState || !tileCount || !tileIds)
    {
        return NE_INVAL;
    }

    if (enhancedContentState->currentTiles == NULL || CSL_VectorGetLength(enhancedContentState->currentTiles) == 0)
    {
        return NE_NOENT;
    }
    
    *tileCount = CSL_VectorGetLength(enhancedContentState->currentTiles);
    *tileIds = (const char**)CSL_VectorGetPointer(enhancedContentState->currentTiles, 0);
    
    return NE_OK;
}


NB_DEF NB_Error
NB_EnhancedContentStateGetMapFrameSplines(NB_EnhancedContentState* enhancedContentState, uint32* splineCount, const NB_MapFrameSpline** splines)
{
    if (!enhancedContentState || !splineCount || !splines)
    {
        return NE_INVAL;
    }

    if (enhancedContentState->currentSplines == NULL || CSL_VectorGetLength(enhancedContentState->currentSplines) == 0)
    {
        return NE_NOENT;
    }

    *splineCount = CSL_VectorGetLength(enhancedContentState->currentSplines);
    *splines = (const NB_MapFrameSpline*)CSL_VectorGetPointer(enhancedContentState->currentSplines, 0);

    return NE_OK;
}

NB_DEF NB_Error
NB_EnhancedContentStateGetMapFrameManeuvers(NB_EnhancedContentState* enhancedContentState, uint32* startManeuver, uint32* endManeuver)
{
    if (!enhancedContentState || !startManeuver || !endManeuver)
    {
        return NE_INVAL;
    }

    if (enhancedContentState->currentRegionIndex == -1)
    {
        return NE_NOENT;
    }
    else
    {
        NB_RouteContentRegion* region = &(GetCurrentContentRegion(enhancedContentState)->region);
        if (! region)
        {
            return NE_NOENT;
        }
        *startManeuver = region->startManeuver;
        *endManeuver = region->endManeuver;
        return NE_OK;
    }
}


NB_DEF NB_Error
NB_EnhancedContentStateGetMapFrameProjectionParameters(NB_EnhancedContentState* enhancedContentState, 
                                                       NB_MapFrameProjectionParameters* projectionParameters)
{
    if (!enhancedContentState || !projectionParameters)
    {
        return NE_INVAL;
    }

    if (enhancedContentState->currentRegionIndex == -1)
    {
        return NE_NOENT;
    }

    // Get projection parameters for junction from content region
    if (enhancedContentState->availableContent & NB_ECSD_JunctionMapFrame)
    {
        NB_RouteContentRegion* region = &(GetCurrentContentRegion(enhancedContentState)->region);
        if (! region)
        {
            return NE_NOENT;
        }

        projectionParameters->projection        = region->projection;
        projectionParameters->datum             = region->datum;
        projectionParameters->originLatitude    = region->originLatitude;
        projectionParameters->originLongitude   = region->originLongitude;
        projectionParameters->scaleFactor       = region->scaleFactor;
        projectionParameters->falseEasting      = region->falseEasting;
        projectionParameters->falseNorthing     = region->falseNorthing;
        projectionParameters->zOffset           = region->zOffset;
    }
    
    // Get projection parameters for city model from metadata
    else if (enhancedContentState->availableContent & NB_ECSD_CityModelMapFrame)
    {
        NB_LatitudeLongitude origin = { INVALID_LATLON, INVALID_LATLON };

        if (NB_EnhancedContentManagerGetCityProjectionParameters(
                enhancedContentState->enhancedContentManager,
                enhancedContentState->currentCityParameters->datasetId,
                &projectionParameters->projection,
                &projectionParameters->datum,
                &origin,
                &projectionParameters->scaleFactor,
                &projectionParameters->falseNorthing,
                &projectionParameters->falseEasting,
                &projectionParameters->zOffset) != NE_OK)
        {
            return NE_NOENT;
        }

        projectionParameters->originLatitude    = origin.latitude;
        projectionParameters->originLongitude   = origin.longitude;
    }
    else
    {
        return NE_NOENT;
    }

    return NE_OK;
}    
 
   
// Protected Functions ..............................................................................

NB_Error 
NB_EnhancedContentProcessorStateCreate(NB_Context* context, NB_EnhancedContentState** enhancedContentState)
{
    NB_EnhancedContentState* pThis = NULL;
    NB_EnhancedContentManager* manager = NULL;

    if (!context)
    {
        return NE_INVAL;
    }

    manager = NB_ContextGetEnhancedContentManager(context);
    if (!manager)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(NB_EnhancedContentState))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(NB_EnhancedContentState));

    pThis->context = context;
    pThis->enhancedContentManager = manager;
    pThis->configured = FALSE;

    (void)NB_EnhancedContentProcessorStateReset(pThis);

    pThis->contentRegions = CSL_VectorAlloc(sizeof(ContentRegion));
    if (! pThis->contentRegions)
    {
        NB_EnhancedContentProcessorStateDestroy(pThis);
        return NE_NOMEM;
    }

    *enhancedContentState = pThis;

    return NE_OK;
}


NB_Error 
NB_EnhancedContentProcessorStateDestroy(NB_EnhancedContentState* enhancedContentState)
{
    if (!enhancedContentState)
    {
        return NE_INVAL;
    }

    if (enhancedContentState->needToFreeSign)
    {
        (void)NB_EnhancedContentStateFreeRealisticSign(enhancedContentState, &enhancedContentState->currentSign);
    }
    nsl_memset(&enhancedContentState->currentSign, 0, sizeof(NB_RealisticSign));

    NB_EnhancedContentProcessorStateReset(enhancedContentState);

    if (enhancedContentState->contentRegions)
    {
        CSL_VectorDealloc(enhancedContentState->contentRegions);
    }

    nsl_free(enhancedContentState);

    return NE_OK;
}


NB_Error
NB_EnhancedContentProcessorStateReset(NB_EnhancedContentState* enhancedContentState)
{
    if (!enhancedContentState)
    {
        return NE_INVAL;
    }

    FreeMapFrameTiles(enhancedContentState);
    FreeMapFrameSplines(enhancedContentState);
    FreeCityParameters(enhancedContentState);
    FreeContentRegions(enhancedContentState);

    enhancedContentState->stateChanged = FALSE;
    enhancedContentState->initialUpdate = TRUE;
    enhancedContentState->mapFrameTilesAvailable = FALSE;
    enhancedContentState->currentManeuver = NAV_MANEUVER_NONE;
    enhancedContentState->availableContent = NB_ECSD_None;

    enhancedContentState->lastPrefetchManeuver = NAV_MANEUVER_NONE;

    FreeRoute(&enhancedContentState->routeId);

    // Clear on demand requests without textures
    (void)NB_EnhancedContentManagerClearOnDemandRequests(enhancedContentState->enhancedContentManager, NB_ECDT_CityModel);
    (void)NB_EnhancedContentManagerClearOnDemandRequests(enhancedContentState->enhancedContentManager, NB_ECDT_MotorwayJunctionObject);
    (void)NB_EnhancedContentManagerClearOnDemandRequests(enhancedContentState->enhancedContentManager, NB_ECDT_RealisticSign);

    return NE_OK;
}


NB_Error
NB_EnhancedContentProcessorSetConfiguration(NB_EnhancedContentState* enhancedContentState, const NB_EnhancedContentMapConfiguration* configuration)
{
    if (!enhancedContentState || !configuration)
    {
        return NE_INVAL;
    }
    
    enhancedContentState->configuration = *configuration;
    enhancedContentState->configured = TRUE;

    return NB_EnhancedContentProcessorSetMapOrientation(enhancedContentState, enhancedContentState->orientation);
}


NB_Error
NB_EnhancedContentProcessorSetMapOrientation(NB_EnhancedContentState* enhancedContentState, NB_MapOrientation orientation)
{
    if (!enhancedContentState)
    {
        return NE_INVAL;
    }

    InitializeCameraProjectionParameters(&enhancedContentState->cameraParameters,
        &enhancedContentState->configuration.cameraProjection[enhancedContentState->orientation],
        enhancedContentState->configuration.mapFieldWidthPixel,
        enhancedContentState->configuration.mapFieldHeightPixel);
        
    return NE_OK;
}


NB_Error
NB_EnhancedContentProcessorUpdate(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState)
{
    NB_Error result = NE_OK;
    nb_boolean prefetchCityTiles = FALSE;

    if (!enhancedContentState || !navigationState || !enhancedContentState->configured)
    {
        return NE_INVAL;
    }

    // Reset state change
    enhancedContentState->stateChanged = FALSE;

    if (navigationState->navRoute->parameterConfiguration.wantCityModels || 
        navigationState->navRoute->parameterConfiguration.wantJunctionModels)
    {
        InitializeMapConfiguration(enhancedContentState, navigationState);

        /*
            First check if we have a new (complete) route. If so get all the regions from the route information so that
            we don't need to calculate it over and over again.
        */
        result = LoadRegionsIfNecessary(enhancedContentState, navigationState->navRoute, &prefetchCityTiles);
        if (result != NE_OK)
        {
            return result;
        }
    }

    // Do enhanced content related processing required at each position/fix
    result = ProcessEnhancedContentOnPositionChange(enhancedContentState, navigationState);
    if (result != NE_OK)
    {
        return result;
    }

    // Do enhanced content related processing required at each maneuver change
    if (navigationState->currentRoutePosition.closest_maneuver != enhancedContentState->currentManeuver)
    {
        enhancedContentState->currentManeuver = navigationState->currentRoutePosition.closest_maneuver;
        ProcessEnhancedContentStateOnManeuverChange(enhancedContentState, navigationState);
    }

    if (prefetchCityTiles)
    {
        NB_EnhancedContentDownloadType downloadType = NB_EnhancedContentManagerGetAllowableDownload(enhancedContentState->enhancedContentManager);

        // Only download if on-demand is enabled
        if (downloadType & NB_ECD0_OnDemand)
        {
            // Prefetch new ECM tiles, if necessary
            result = PrefetchCityModelTiles(enhancedContentState, navigationState->navRoute);
            if (result != NE_OK)
            {
                return result;
            }
        }
        else
        {
            TEST_LOGGING("On-Demand not enabled. Don't prefetch");
        }
    }

    // If this update is called during the process of applying a new route, do not notify client yet
    if (navigationState->applyingNewRoute)
    {
        enhancedContentState->stateChanged = FALSE;
    }

    // Otherwise, if this is the first update since reset, always notify client of initial state
    else if (enhancedContentState->initialUpdate)
    {
        enhancedContentState->initialUpdate = FALSE;
        enhancedContentState->stateChanged = TRUE;
    }

    return NE_OK;
}


// Local functions ...............................................................................

static NB_Error
ProcessEnhancedContentOnPositionChange(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState)
{
    NB_Error err = NE_OK;
    NB_RouteConfiguration* routeConfiguration = &navigationState->navRoute->parameterConfiguration;

    // Process realistic signs, if wanted
    if (routeConfiguration->wantRealisticSigns)
    {
        if (navigationState->maneuverPos == NB_NMP_Prepare || navigationState->maneuverPos == NB_NMP_Turn)
        {
            UpdateRealisticSignInformation(enhancedContentState, navigationState);
        }
        else
        {
            ClearRealisticSignInformation(enhancedContentState, navigationState);
        }
    }

    // Process for map frame data (city and junction models), if wanted and regions exist for current route
    if ((routeConfiguration->wantCityModels || routeConfiguration->wantJunctionModels)
            && CSL_VectorGetLength(enhancedContentState->contentRegions) > 0)
    {
        err = ProcessMapFrameData(enhancedContentState, navigationState);
    }

    return err;
}

static void
ProcessEnhancedContentStateOnManeuverChange(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState)
{
    nb_boolean success = TRUE;

    NB_RouteConfiguration*          routeConfiguration = 0;
    NB_RoutePrivateConfiguration*   privateRouteConfiguration = 0;

    // Get route configurations
    routeConfiguration = &navigationState->navRoute->parameterConfiguration;
    privateRouteConfiguration = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    // Process for sign and/or junction prefetching, if enabled
    if (routeConfiguration->wantRealisticSigns || routeConfiguration->wantJunctionModels)
    {
        uint32 startNextPrefetchManeuver = 0;

        if (enhancedContentState->lastPrefetchManeuver != NAV_MANEUVER_NONE)
        {
            startNextPrefetchManeuver = enhancedContentState->lastPrefetchManeuver + privateRouteConfiguration->prefs.maxEnhancedContentPrefetchManeuvers;
        }

        if (enhancedContentState->currentManeuver >= startNextPrefetchManeuver)
        {
            // Prefetch upcoming realistic signs
            if (routeConfiguration->wantRealisticSigns)
            {
                success = PrefetchRealisticSigns(
                    enhancedContentState,
                    navigationState->navRoute,
                    enhancedContentState->currentManeuver,
                    privateRouteConfiguration->prefs.maxEnhancedContentPrefetchManeuvers);
            }

            // Prefetch upcoming junctions
            if (success && routeConfiguration->wantJunctionModels)
            {
                success = PrefetchMotorwayJunctions(
                    enhancedContentState,
                    navigationState->navRoute,
                    enhancedContentState->currentManeuver,
                    privateRouteConfiguration->prefs.maxEnhancedContentPrefetchManeuvers);
            }

            // If prefetches succeed, note maneuver
            if (success)
            {
                enhancedContentState->lastPrefetchManeuver = enhancedContentState->currentManeuver;
            }
        }
    }
}

static nb_boolean
PrefetchRealisticSigns(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route, uint32 startManeuver, uint32 maneuverCount)
{
    NB_Error result = NE_OK;
    nb_boolean success = TRUE;

    NB_EnhancedContentManager* manager = enhancedContentState->enhancedContentManager;

    uint32 maneuver = 0;
    uint32 maxManeuver  =  0;

    // Prefetch realistic signs for specified maneuver count
    maxManeuver = MIN(startManeuver + maneuverCount, NB_RouteInformationGetManeuverCount(route));

    for (maneuver = startManeuver; maneuver < maxManeuver; maneuver++)
    {
        uint32 signIndex = 0;
        uint32 signCount = 0;
        
        signCount = NB_RouteInformationGetManeuverRealisticSignCount(route, maneuver);

        // NOTE: For Vx1 only none or 1 sign per maneuver will be returned, but handle more for the future
        for (signIndex = 0; signIndex < signCount; signIndex++)
        {
            NB_RouteRealisticSign sign = {0};

            // Get realistic sign info from route reply
            if (NB_RouteInformationGetManeuverRealisticSign(route, maneuver, signIndex, &sign) != NE_OK)
            {
                return FALSE;
            }

            result = NB_EnhancedContentManagerCheckFileVersion(manager, NB_ECDT_RealisticSign, sign.datasetId, sign.signId, sign.version);
            // Ignore the error NE_NOENT
            // Return NE_NOENT there is no entry of the file or the file version isn't latest.
            result = (result != NE_NOENT) ? result : NE_OK;
            if (result != NE_OK)
            {
                return FALSE;
            }

            // Check for default sign (landscape) and request if needed
            success &= PrefetchEnhancedContentData(manager, NB_ECDT_RealisticSign, sign.datasetId, sign.signId, sign.version);

            // Check for portrait version of sign if one provided, and request if needed
            if (sign.signPortraitId && *sign.signPortraitId)
            {
                result = NB_EnhancedContentManagerCheckFileVersion(manager, NB_ECDT_RealisticSign, sign.datasetId, sign.signPortraitId, sign.version);
                // Ignore the error NE_NOENT
                // Return NE_NOENT there is no entry of the file or the file version isn't latest.
                result = (result != NE_NOENT) ? result : NE_OK;
                if (result != NE_OK)
                {
                    return FALSE;
                }

                success &= PrefetchEnhancedContentData(manager, NB_ECDT_RealisticSign, sign.datasetId, sign.signPortraitId, sign.version);
            }
        }
    }

    // Return value indicates if prefetch was successful or should be tried again later
    return success;
}

static nb_boolean
PrefetchMotorwayJunctions(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route, uint32 startManeuver, uint32 maneuverCount)
{
    nb_boolean success = TRUE;

    uint32 maxManeuver  =  0;
    uint32 regionIndex = 0;
    uint32 regionCount = 0;

    // Prefetch junction signs for specified maneuver count
    maxManeuver = MIN(startManeuver + maneuverCount, NB_RouteInformationGetManeuverCount(route));

    // For all regions
    regionCount = CSL_VectorGetLength(enhancedContentState->contentRegions);
    for (regionIndex = 0; regionIndex < regionCount; regionIndex++)
    {
        NB_RouteContentRegion* region = &(((ContentRegion*)CSL_VectorGetPointer(enhancedContentState->contentRegions, regionIndex))->region);

        // Request junction objects that start within the current prefetch range
        if (region->type == NB_RCRT_JunctionModel
            && region->startManeuver >= startManeuver
            && region->startManeuver <= maxManeuver)
        {
            success &= PrefetchEnhancedContentData(
                enhancedContentState->enhancedContentManager,
                NB_ECDT_MotorwayJunctionObject,
                region->datasetId, region->regionId, region->version);
        }
    }

    // Check that junction specific textures are available, and if not, request them
    if (NB_EnhancedContentManagerIsMotorwayJunctionTextureAvailable(enhancedContentState->enhancedContentManager) == FALSE)
    {
        success &= (NB_EnhancedContentManagerRequestDataDownload(
                        enhancedContentState->enhancedContentManager,
                        NB_ECDT_SpecificTexture,
                        NB_EnhancedContentManagerGetMotorwayJunctionObjectsDatasetId(enhancedContentState->enhancedContentManager),
                        NULL,
                        NULL) == NE_OK);
    }


    // Return value indicates if prefetch was successful or should be tried again later
    return success;
}

/*! Prefetch MJO or SAR data.

    This function is not used for city tiles (ECM). See PrefetchCityModelTiles() for city tile prefetch.

    @return 'TRUE' on success, 'FALSE' on failure.
*/
static nb_boolean
PrefetchEnhancedContentData(NB_EnhancedContentManager* manager, NB_EnhancedContentDataType dataType, const char* dataset, const char* dataId, const char* version)
{
    // Check if data item is already present
    if (NB_EnhancedContentManagerIsDataAvailable(manager, dataType, dataset, dataId))
    {
        TEST_LOGGING(" - PrefetchEnhancedContentData(), item already prefetched: %s", dataId);

        // Don't need to request -- success
        return TRUE;
    }

    // Request data item download
    if (NB_EnhancedContentManagerRequestDataDownload(manager, dataType, dataset, dataId, version) == NE_OK)
    {
        TEST_LOGGING(" - PrefetchEnhancedContentData(): %s", dataId);

        // Queued for download -- success
        return TRUE;
    }

    // Something went wrong with either the data source check or download request.
    // Return false so this prefetch will be attempted again.
    return FALSE;
}

static void
UpdateRealisticSignInformation(NB_EnhancedContentState* enhancedContentState,  NB_NavigationState* navigationState)
{
    NB_Error result = NE_OK;

    uint32 maneuver = navigationState->currentRoutePosition.closest_maneuver;
    uint32 signCount = 0;
    
    NB_RouteRealisticSign sign = { 0 };
    uint32 signSize = 0;
    uint8* signData = 0;
    uint32 portraitSignSize = 0;
    uint8* portraitSignData = 0;

    NB_RouteCollapseManeuvers(navigationState->navRoute, maneuver, NULL, NULL, NULL,
                              NULL, NULL, &maneuver, NULL, NULL, NULL, NULL, NULL, FALSE);

    if (enhancedContentState->availableContent & NB_ECSD_RealisticSign)
    {
        if (enhancedContentState->currentSign.maneuver == maneuver)
        {
            // @todo: handle multiple signs per maneuver, when available

            // Current sign already displayed, no change
            return;
        }

        // Different maneuver sign displayed -- clear previous
        ClearRealisticSignInformation(enhancedContentState, navigationState);
    }

    // Get realistic sign count for current maneuver
    signCount = NB_RouteInformationGetManeuverRealisticSignCount(navigationState->navRoute, maneuver);

    if (signCount > 0)
    {
        // @todo: handle multiple signs per maneuver, when available

        // Get sign info from route information
        if (NB_RouteInformationGetManeuverRealisticSign(navigationState->navRoute, maneuver, 0, &sign))
        {
            return;
        }

        // Retrieve realistic sign image
        result = NB_EnhancedContentManagerGetDataItem(
            enhancedContentState->enhancedContentManager,
            NB_ECDT_RealisticSign,
            sign.datasetId,
            sign.signId,
            sign.version,
            TRUE,
            &signSize,
            &signData);

        // If there's a portrait image for the realistic sign, retrieve it
        if (result == NE_OK)
        {
            // @todo: When there is no portrait version of this sign, get another
            // copy of the standard landscape sign to be sure client gets both
            if (!sign.signPortraitId || !*sign.signPortraitId)
            {
                sign.signPortraitId = sign.signId;
            }

            result = NB_EnhancedContentManagerGetDataItem(
                enhancedContentState->enhancedContentManager,
                NB_ECDT_RealisticSign,
                sign.datasetId,
                sign.signPortraitId,
                sign.version,
                TRUE,
                &portraitSignSize,
                &portraitSignData);
        }

        // If sign image(s) were successfully retrieved, update enhanced content state
        if (result == NE_OK)
        {
            enhancedContentState->currentSign.signId = sign.signId;
            enhancedContentState->currentSign.data = signData;
            enhancedContentState->currentSign.dataSize = signSize;
            enhancedContentState->currentSign.portraitSignId = sign.signPortraitId;
            enhancedContentState->currentSign.portraitData = portraitSignData;
            enhancedContentState->currentSign.portraitDataSize = portraitSignSize;
            enhancedContentState->currentSign.maneuver = sign.maneuver;

            enhancedContentState->needToFreeSign = TRUE;

            enhancedContentState->stateChanged = TRUE;
            enhancedContentState->availableContent |= NB_ECSD_RealisticSign;

            // @todo Remove it because it's an empty func only returning NOT_SUPPORTED.
            //(void)NB_AnalyticsAddEnhancedContent(enhancedContentState->context, NB_AECD_RealisticSign, COUNTRY_CODE);
            if (NB_ContextGetQaLog(enhancedContentState->context))
            {
                NB_QaLogRealisticSignState(
                    enhancedContentState->context,
                    enhancedContentState->currentSign.signId,
                    NB_QLRSST_AutoOn,
                    navigationState->currentRoutePosition.proj_lat,
                    navigationState->currentRoutePosition.proj_lon);
            }
        }
    }
}

static void
ClearRealisticSignInformation(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState)
{
    // If realistic sign info is currently available, clear it and modify state
    if (enhancedContentState->availableContent & NB_ECSD_RealisticSign)
    {
        if (NB_ContextGetQaLog(enhancedContentState->context))
        {
            NB_QaLogRealisticSignState(
                enhancedContentState->context,
                enhancedContentState->currentSign.signId,
                NB_QLRSST_AutoOff,
                navigationState->currentRoutePosition.proj_lat,
                navigationState->currentRoutePosition.proj_lon);
        }

        if (enhancedContentState->needToFreeSign)
        {
            (void)NB_EnhancedContentStateFreeRealisticSign(enhancedContentState, &enhancedContentState->currentSign);
        }
        nsl_memset(&enhancedContentState->currentSign, 0, sizeof(NB_RealisticSign));

        enhancedContentState->stateChanged = TRUE;
        enhancedContentState->availableContent ^= NB_ECSD_RealisticSign;
    }
}


// City/ECM and MJO calculations .....................................................................................
/*
    This code is ported from JCC. In particular from:
        EnhancedContentProcessor.java
        EnhancedMapProperty.java
        MapFrameData.java
*/

/*! Process next frame for enhanced content.

    This is the entry point for calculating the map frame data. All processing is done here. This is called for every
    position change. The map frame data gets updated in the enhanced content state.

    @return None
*/
static NB_Error
ProcessMapFrameData(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState)
{
    double distanceFromStart = 0.0;
    NB_LatitudeLongitude userPosition = {0};
    userPosition.latitude  = navigationState->currentRoutePosition.proj_lat;
    userPosition.longitude = navigationState->currentRoutePosition.proj_lon;

    /*
       determine if currently positioned within a content region
       if entering a new region, determine tiles and texture needed and if they are present
           if everything present, populate necessary map frame data and notify client via callback
           else request what is needed
       else if leaving regions clear map frame data, and notify client via callback it has been cleared
    */

    // There should be a function to get the distance from the start of the route.
    distanceFromStart = navigationState->navRoute->routeLength - navigationState->remainingRouteDist;

    return ProcessContentRegions(enhancedContentState,
                          navigationState->navRoute,
                          distanceFromStart,
                          userPosition);
}

/*! Check if the route has changed and if so load all content regions.

    @return NB_Error

    @see ProcessMapFrameData
*/
NB_Error
LoadRegionsIfNecessary(NB_EnhancedContentState* enhancedContentState,
                       NB_RouteInformation* route,
                       nb_boolean* prefetchCityTiles    /*!< On return indicates if we need to prefetch city tiles. */
                       )
{
    NB_Error result = NE_OK;
    uint32 regionCount = 0;
    uint32 i = 0;
    NB_RouteId* newRouteID = NULL;

    *prefetchCityTiles = FALSE;

    // Get new route ID
    result = NB_RouteInformationGetRouteId(route, &newRouteID);
    if (result != NE_OK)
    {
        return result;
    }

    // If the route is the same then we return
    if ((newRouteID->size == enhancedContentState->routeId.size) &&
        (nsl_memcmp(newRouteID->data, enhancedContentState->routeId.data, enhancedContentState->routeId.size) == 0))
    {
        // Route is the same. Return success
        return NE_OK;
    }

    // Check if the route was completely downloaded
    if (! NB_RouteInformationIsRouteComplete(route))
    {
        // Route is not completely downloaded, yet. Don't refresh content regions but return success.
        return NE_OK;
    }

    // Free old regions.
    FreeContentRegions(enhancedContentState);

    // For all new regions
    regionCount = NB_RouteInformationGetContentRegionCount(route);
    for (i = 0; i < regionCount; ++i)
    {
        // Get content region from route-information. This calculates all the region parameters.
        ContentRegion region = {0};
        result = NB_RouteInformationGetContentRegion(route, i, &region.region);
        if (result != NE_OK)
        {
            return result;
        }

        // Add to vector in enhanced content state.
        CSL_VectorAppend(enhancedContentState->contentRegions, &region);

        switch (region.region.type)
        {
            case NB_RCRT_CityModel:
            {
                // Check city model version returned with route to city model in manifest.  If manifest versions do not match (NE_NOENT returned),
                // outdated city data on device will be removed, and 3D city model will not display for this city until manifest is re-synchronized.
                if (NB_EnhancedContentManagerCheckManifestVersion(enhancedContentState->enhancedContentManager, NB_ECDT_CityModel, region.region.datasetId, region.region.version) != NE_OK)
                {
                    enhancedContentState->availableContent |= NB_ECSD_SynchronizationNeeded;
                    enhancedContentState->stateChanged = TRUE;

                    TEST_LOGGING("City %s manifest version %s does not match manifest version returned with route", region.region.datasetId, region.region.version);
                }

                break;
            }
            case NB_RCRT_JunctionModel:
            {
                result = NB_EnhancedContentManagerCheckFileVersion(enhancedContentState->enhancedContentManager, NB_ECDT_MotorwayJunctionObject, region.region.datasetId, region.region.regionId, region.region.version);
                // Ignore the error NE_NOENT
                // Return NE_NOENT there is no entry of the file or the file version isn't latest.
                result = (result != NE_NOENT) ? result : NE_OK;
                if (result != NE_OK)
                {
                    return result;
                }

                break;
            }
            case NB_RCRT_Invalid:
                break;
        }
    }

    // Overwrite old route-id with new route-id
    CopyRoute(&enhancedContentState->routeId, newRouteID);

    if (route->parameterConfiguration.wantCityModels)
    {
        *prefetchCityTiles = TRUE;
    }

    return NE_OK;
}

/*! Free a route if it is set.

    @todo: This function should be somewhere in the route information.

    @return None
*/
void
FreeRoute(NB_RouteId* route)
{
    if (route->data)
    {
        nsl_free(route->data);
        route->data = NULL;
        route->size = 0;
    }
}

/*! Copy a route-ID. Frees old route if necessary.

    @todo: This function should be somewhere in the route information.

    @return None
*/
void
CopyRoute(NB_RouteId* destination, const NB_RouteId* source)
{
    FreeRoute(destination);

    if (source && (source->size > 0))
    {
        destination->data = nsl_malloc(source->size);
        if (destination->data)
        {
            nsl_memcpy(destination->data, source->data, source->size);
            destination->size = source->size;
        }
    }
}

/*! Initialize map configuration.

    @return None

    @see ProcessMapFrameData
*/
static void 
InitializeMapConfiguration(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState) 
{
    // Get current position
    struct route_position* position = &navigationState->currentRoutePosition;
    double userLatitude  = position->proj_lat;
    double userLongitude = position->proj_lon;

    /* Not sure if this is correct. Which heading should we use? */
    double heading = 0.0;
    if (position->valid_heading)
    {
        heading = position->segment_heading;
    }
    else
    {
        heading = navigationState->lastHeading;
    }

    // Calculate camera projection context based on the camera projection parameters.
    SetupCameraProjectionContext(enhancedContentState, userLatitude, heading);

    // The viewing plane is the projection of the frustum (visible area) to the x/y-plane
    SetupViewingPlane(enhancedContentState, userLatitude, userLongitude);
}

/*! Initialize the camera projection parameters.

    The input parameters are fixed and are specified in the new format defined in Nav SDS
    //depot/Projects/372 - VZ Navigator Vx1/Published/Documentation/SDS/VZNvx_NavigationSDS.doc #5
*/
void
InitializeCameraProjectionParameters(CameraProjectionParameters* parameters,    /*!< Parameters to initialize */
                                     const NB_CameraProjectionParameters* projectionParameters,
                                     int mapFieldWidthPixel,                    /*!< I'm assuming this is the navigation view width/height */
                                     int mapFieldHeightPixel
                                     ) 
{
    double tanIAH = 0.0;
    double verticalFieldOfViewRadians = 0.0;
    double angleDAH = 0.0;
    double angleBAF = 0.0;
    double angleBAD = 0.0;

    if ((mapFieldWidthPixel <= 0) || (mapFieldHeightPixel <= 0)) 
    {
        return;
    }
    
    nsl_memset(parameters, 0, sizeof(CameraProjectionParameters));

    parameters->cameraHeightMeters                  = projectionParameters->cameraHeightMeters;
    parameters->cameraToFrustumEndHorizontalMeters  = projectionParameters->cameraToAvatarHorizontalMeters + projectionParameters->avatarToHorizonHorizontalMeters;        
    parameters->cameraToAvatarHorizontalMeters      = projectionParameters->cameraToAvatarHorizontalMeters;
    parameters->avatarToHorizonHorizontalMeters     = projectionParameters->avatarToHorizonHorizontalMeters;
    
    parameters->aspectRatio = ((double) mapFieldHeightPixel / (double) mapFieldWidthPixel);        
    parameters->tangentHalfHorizontalFieldOfView    = nsl_tan(TO_RAD(projectionParameters->horizontalFieldOfViewDegrees) / 2.0);

    parameters->cameraToHorizonMeters = 
        (int) nsl_sqrt(projectionParameters->cameraHeightMeters * projectionParameters->cameraHeightMeters + parameters->cameraToFrustumEndHorizontalMeters * parameters->cameraToFrustumEndHorizontalMeters);
    
    /*
        Compare this to the "3D Map Perspective" on page 50 diagram in "VZNvx_NavigationSDS.doc".
        A = Camera
        B = Horizontal Camera position
        D = Map bottom
        F = Avatar
        I = Center of map projection (not in diagram)
        H = Avatar position on map projection
        G = Horizon

        tanIAH = HI/AI
        tanIAD = DI/AI

        Equating AI, HI/tanIAH = DI/tanIAD

        tanIAH = (HI/DI) * tanIAD
        tanIAH = ((DI-DH)/DI) * tanIAD
        tanIAH = ((1-(DH/DI)) * tanIAD where IAD is half of verticalFieldOfView
     */
    tanIAH = (1.0 - ((double)projectionParameters->avatarToMapBottomPixel / ((double) mapFieldHeightPixel / 2.0))) * (parameters->aspectRatio * parameters->tangentHalfHorizontalFieldOfView);
    verticalFieldOfViewRadians = 2 * (nsl_atan(parameters->aspectRatio * parameters->tangentHalfHorizontalFieldOfView));

    angleDAH = (verticalFieldOfViewRadians / 2.0 - nsl_atan(tanIAH));
    angleBAF = nsl_atan((double) parameters->cameraToAvatarHorizontalMeters / (double) parameters->cameraHeightMeters);
    angleBAD = angleBAF - angleDAH;

    parameters->cameraToFrustumStartHorizontalMeters = (int) (nsl_tan(angleBAD) * (double)parameters->cameraHeightMeters);
    parameters->cameraToFrustumStartMeters           = (int) ((double)parameters->cameraHeightMeters / nsl_cos(angleBAD));
}

/*! Initialize camera projection context based on camera projection parameters.

    The camera projection parameters have to be initialized before calling this function.

    I think this is called for every GPS fix.
*/
void 
SetupCameraProjectionContext(NB_EnhancedContentState* enhancedContentState,
                             double latitude,                               /*!< Latitude of camera */
                             double heading) 
{
    // The parameters are the input, the context is the output
    const CameraProjectionParameters* parameters    = &enhancedContentState->cameraParameters;
    CameraProjectionContext* context                = &enhancedContentState->cameraContext;

    context->horizontalAvatarOffset = parameters->cameraToAvatarHorizontalMeters;

    context->cosLatitude            = nsl_cos(TO_RAD(latitude));
    context->cosLatRadius           = context->cosLatitude * RADIUS_EARTH_METERS;

    context->rotateYSine            = nsl_sin(TO_RAD(heading));
    context->rotateYCosine          = nsl_cos(TO_RAD(heading));

    context->dxAvatar               = context->rotateYSine * context->horizontalAvatarOffset;
    context->dzAvatar               = context->rotateYCosine * context->horizontalAvatarOffset;

    context->dxMaxViewDist          = context->rotateYSine * parameters->cameraToFrustumEndHorizontalMeters;
    context->dzMaxViewDist          = context->rotateYCosine * parameters->cameraToFrustumEndHorizontalMeters;

    context->dxMinViewDist          = context->rotateYSine * parameters->cameraToFrustumStartHorizontalMeters;
    context->dzMinViewDist          = context->rotateYCosine * parameters->cameraToFrustumStartHorizontalMeters;

    context->widthMaxViewPlane      = parameters->cameraToHorizonMeters * parameters->tangentHalfHorizontalFieldOfView;
    context->widthMinViewPlane      = parameters->cameraToFrustumStartMeters * parameters->tangentHalfHorizontalFieldOfView;
}

/*! Calculate viewing plane based on user/camera position.

    The viewing plane(4 points) is the projection of the viewing frustum to the x/y plane. We use it to 
    calculate the needed ECM/MJO tiles required for drawing.

    @return None

    @see InitializeMapConfiguration
*/
static void
SetupViewingPlane(NB_EnhancedContentState* enhancedContentState, 
                  double userLatitude,                                /*!< Position of user */
                  double userLongitude                                /*!< Position of user */
                  )
{
    const CameraProjectionContext* context = &enhancedContentState->cameraContext;
    NB_Vector* plane = enhancedContentState->mapFrameData.viewingPlane;

    NB_Vector userPosition      = {0};
    NB_Vector cameraOffset      = {0};

    NB_Vector cameraPosition    = {0};
    NB_Vector startOffset       = {0};
    NB_Vector endOffset         = {0};

    NB_Vector startPosition     = {0};
    NB_Vector endPosition       = {0};

    NB_Vector  unitZ            = {0};
    NB_Vector  normalStart      = {0};
    NB_Vector  normalEnd        = {0};
    NB_Vector  tempVector       = {0};

    /*
        This code is ported from the JCC. We calculate the viewing frustum (visible 3-D space) based on the current 
        user and camera position. We then project the frustum to the x/y-plane. That is our view-strip.
    */

    // Convert to mercator. Don't use vector_from_lat_lon()!
    NB_SpatialConvertLatLongToMercator(userLatitude, userLongitude, &userPosition.x, &userPosition.y);

    vector_set(&cameraOffset, 
               context->dxAvatar / context->cosLatRadius,
               context->dzAvatar / context->cosLatRadius,
               0.0);

    vector_subtract(&cameraPosition, &userPosition, &cameraOffset);

    vector_set(&startOffset, 
               context->dxMinViewDist / context->cosLatRadius,
               context->dzMinViewDist / context->cosLatRadius,
               0.0);

    vector_set(&endOffset, 
               context->dxMaxViewDist / context->cosLatRadius,
               context->dzMaxViewDist / context->cosLatRadius,
               0.0);

    vector_add(&startPosition, &cameraPosition, &startOffset);
    vector_add(&endPosition, &cameraPosition, &endOffset);

    vector_set(&unitZ, 0.0, 0.0, 1.0);

    vector_subtract(&tempVector, &startPosition, &endPosition);

    vector_cross_product(&normalEnd, &tempVector, &unitZ);

    // In JCC this just calls MathVector.Normalize. I'm assuming it's normalize-to-length
    vector_normalize_to_length(&normalEnd, &normalEnd, context->widthMaxViewPlane / context->cosLatRadius);

    vector_cross_product(&normalStart, &tempVector, &unitZ);

    // In JCC this just calls MathVector.Normalize. I'm assuming it's normalize-to-length
    vector_normalize_to_length(&normalStart, &normalStart, context->widthMinViewPlane / context->cosLatRadius);

    /*
        Project frustum to x/y-plane. The polygon (4-points) is our viewing-plane.
    */
    vector_add(     &plane[0], &startPosition, &normalStart);
    vector_add(     &plane[1], &endPosition,   &normalEnd);
    vector_subtract(&plane[2], &endPosition,   &normalEnd);
    vector_subtract(&plane[3], &startPosition, &normalStart);
}

/*! Process content regions.

    This function does all the work. It calculates if there is a enhanced content region to display and
    calculates all the map frame data for it.

    @return None

    @see ProcessMapFrameData
*/
static NB_Error
ProcessContentRegions(NB_EnhancedContentState* enhancedContentState,
                      NB_RouteInformation* route,
                      double distanceFromStart,                         /*!< Current user position from start of route, in meters */
                      const NB_LatitudeLongitude userPosition           /*!< Current (projected) user position */
                      )
{
    NB_Error err = NE_OK;
    nb_boolean mapFrameDataUpdated = FALSE;

    // This returns NULL if we are currently not in a content region
    ContentRegion* currentRegion = GetCurrentContentRegion(enhancedContentState);

    // Used for CanShowRegion()
    uint32 endManeuverIndex = NB_RouteInformationGetManeuverCount(route) - 1;
    
    // If currently in region, and region is still visible, update map frame data
    if (currentRegion && CanShowRegion(enhancedContentState, &currentRegion->region, distanceFromStart, endManeuverIndex))
    {
        uint32 currentTimeStampSec = PAL_ClockGetUnixTime();

//        TEST_LOGGING("ProcessContentRegions(), In region: YES");

        // Only update map frame data if enhanced content has not been shown yet at all
        // or has not been shown for a specified time period (seconds)
        if (enhancedContentState->lastRegionShowTimestampSec == 0
            || (currentTimeStampSec - enhancedContentState->lastRegionShowTimestampSec) > enhancedContentState->configuration.waitBetweenCityModelDisplaySeconds)
        {
            enhancedContentState->lastRegionShowTimestampSec = 0;
            
            // Update map frame data for current content region
            mapFrameDataUpdated = UpdateMapFrameData(enhancedContentState);
            
            if (mapFrameDataUpdated == FALSE)
            {
                enhancedContentState->lastRegionShowTimestampSec = currentRegion->lastShowTimestampSec;
            }
        }
    }

    // Currently not in region or current region is no longer visible, so look for next region
    else
    {
        uint32 regionCount = 0;
        uint32 regionIndex = 0;

        // If currently in a region, reset it
        if (currentRegion)
        {
            enhancedContentState->currentRegionIndex = -1;
        }

//        TEST_LOGGING("ProcessContentRegions(), In region: NO");

        enhancedContentState->lastRegionShowTimestampSec = 0;

        // Iterate through content regions to determine if in one
        regionCount = CSL_VectorGetLength(enhancedContentState->contentRegions);
        for (regionIndex = 0; regionIndex < regionCount; regionIndex++)
        {
            NB_RouteContentRegion* region = &(((ContentRegion*)CSL_VectorGetPointer(enhancedContentState->contentRegions, regionIndex))->region);

            // Check if we're in a region
            if (CanShowRegion(enhancedContentState, region, distanceFromStart, endManeuverIndex))
            {
                enhancedContentState->currentRegionIndex = regionIndex;

                // Update map frame data for new content region
                mapFrameDataUpdated = UpdateMapFrameData(enhancedContentState);

                // Get collection of splines for the new region
                err = GetMapFrameSplines(enhancedContentState, route);
                if (err != NE_OK)
                {
                    return err;
                }

                // Current region has been found, stop looking
                break;
            }
        }
    }

//    TEST_LOGGING("ProcessContentRegions(), In region: %d, mapFrameDataUpdated: %d",  
//                 (enhancedContentState->currentRegionIndex == -1) ? 0 : 1,
//                 mapFrameDataUpdated);

    /*
        The map-frame data flag is set if we have all the tiles for the current map fame. We only switch to 3-D city/MJO view
        when we have all tiles for that particular frame. That means we trigger the callback if we toggle from having tiles and
        having no-tiles. We only do this if we are already inside a region.
     */
    if (mapFrameDataUpdated != enhancedContentState->mapFrameTilesAvailable)
    {
        TEST_LOGGING("ProcessContentRegions(), frame toggle. Trigger callback.");
        
        enhancedContentState->stateChanged = TRUE;
        enhancedContentState->mapFrameTilesAvailable = mapFrameDataUpdated;
    }
    
    if (enhancedContentState->availableContent & (NB_ECSD_CityModelMapFrame | NB_ECSD_JunctionMapFrame))
    {
        // If we don't have tiles for the current frame then we clear the available-content flags.
        if (! mapFrameDataUpdated)
        {
//          TEST_LOGGING("ProcessContentRegions(), No frame data. Reset available content flags.");
            enhancedContentState->availableContent &= ~(NB_ECSD_CityModelMapFrame | NB_ECSD_JunctionMapFrame);
            enhancedContentState->stateChanged = TRUE;
        }
    }

    // Is QA logging enabled
    if (NB_ContextGetQaLog(enhancedContentState->context))
    {
        // Did we enter/exit a region
        ContentRegion* newRegion = GetCurrentContentRegion(enhancedContentState);
        if (currentRegion != newRegion)
        {
            // Did we exit a region
            if (currentRegion)
            {
                // Log region exit
                QaLogEnhancedContentRegionState(enhancedContentState->context, &currentRegion->region, NB_QLECRS_Exit, userPosition);
            }

            // Did we enter a region
            if (newRegion)
            {
                // Log region enter
                QaLogEnhancedContentRegionState(enhancedContentState->context, &newRegion->region, NB_QLECRS_Enter, userPosition);
            }
        }
    }

    return err;
}

static void
QaLogEnhancedContentRegionState(NB_Context* context, const NB_RouteContentRegion* region, NB_QaLogEnhancedContentRegionState state, const NB_LatitudeLongitude userPosition)
{
    NB_QaLogEnhancedContentType regionType = NB_QLECT_Undefined;
    
    switch (region->type)
    {
        case NB_RCRT_CityModel:
            regionType = NB_QLECT_CityModels;
            break;
        case NB_RCRT_JunctionModel:
            regionType = NB_QLECT_Junctions;
            break;
        default:
            break;
    }

    NB_QaLogEnhancedContentState(context, region->regionId, regionType, state, userPosition.latitude, userPosition.longitude);
}

/*! Free content regions vector in content state.

    This should be called when the route is cleared.
*/
static void 
FreeContentRegions(NB_EnhancedContentState* enhancedContentState)
{
    // Don't free the vector here. It gets destroyed in NB_EnhancedContentProcessorStateDestroy().

    if (enhancedContentState->contentRegions)
    {
        CSL_VectorRemoveAll(enhancedContentState->contentRegions);
    }
    enhancedContentState->currentRegionIndex = -1;
    enhancedContentState->lastRegionShowTimestampSec = 0;
}

/*! Return the current content region.

    @return Current content region or NULL if we are currently not in a region
*/
static ContentRegion*
GetCurrentContentRegion(NB_EnhancedContentState* enhancedContentState)
{
    if (enhancedContentState->contentRegions)
    {
        if (enhancedContentState->currentRegionIndex != -1)
        {
            return (ContentRegion*)CSL_VectorGetPointer(enhancedContentState->contentRegions, enhancedContentState->currentRegionIndex);
        }
    }

    return NULL;
}

static NB_Error
GetMapFrameSplines(NB_EnhancedContentState* enhancedContentState, NB_RouteInformation* route)
{
    ContentRegion* currentRegion = NULL;

    FreeMapFrameSplines(enhancedContentState);

    currentRegion = GetCurrentContentRegion(enhancedContentState);
    if (!currentRegion)
    {
        return NE_INVAL;
    }

    if (currentRegion->region.splinePathCount > 0)
    {
        uint32 splineIndex = 0;

        enhancedContentState->currentSplines = CSL_VectorAlloc(sizeof(NB_MapFrameSpline));
        if (!enhancedContentState->currentSplines)
        {
            return NE_NOMEM;
        }

        for (splineIndex = 0; splineIndex < currentRegion->region.splinePathCount; splineIndex++)
        {
            NB_MapFrameSpline spline = { 0 };

            if (NB_RouteInformationGetContentRegionSplinePath(
                    route, enhancedContentState->currentRegionIndex, splineIndex, 
                    &spline.pathId, &spline.splineSize, &spline.spline) == NE_OK)
            {
                if (!CSL_VectorAppend(enhancedContentState->currentSplines, &spline))
                {
                    CSL_VectorDealloc(enhancedContentState->currentSplines);
                    enhancedContentState->currentSplines = NULL;
                    return NE_NOMEM;
                }
            }
        }
    }

    return NE_OK;
}

static void
FreeMapFrameSplines(NB_EnhancedContentState* enhancedContentState)
{
    if (enhancedContentState->currentSplines != NULL)
    {
        CSL_VectorDealloc(enhancedContentState->currentSplines);
        enhancedContentState->currentSplines = NULL;
    }
}

/*! Check if we are within this region.

    @return TRUE if we are within this region; FALSE otherwise
*/
static nb_boolean
CanShowRegion(NB_EnhancedContentState* enhancedContentState, 
              const NB_RouteContentRegion* region,              /*!< Region to check */
              double distanceFromStart,                         /*!< Current user position from start of route, in meters */
              uint32 endManeuverIndex                           /*!< End maneuver of route */
              )
{
    double regionEnd = region->distanceFromRouteStart + region->length;
    
    // Check if the user is inside the given region
    switch (region->type)
    {
        case NB_RCRT_CityModel:
        {
//            if (regionEnd > distanceFromStart)
//            {
//                TEST_LOGGING("\r\nECM - CanShowRegion: Current Distance: %1.1f, Current - avatar: %1.1f, Region, Start: %1.1f, End: %1.1f", 
//                              distanceFromStart, 
//                              distanceFromStart - enhancedContentState->configuration.maximumCameraToAvatarDistanceMeters,
//                              region->distanceFromRouteStart,
//                              regionEnd);
//            }
            
            if ((region->distanceFromRouteStart == 0
                 || (distanceFromStart - enhancedContentState->configuration.maximumCameraToAvatarDistanceMeters) >= region->distanceFromRouteStart)
                && distanceFromStart <= regionEnd)
            {
                // If the region end maneuver is the last maneuver (destination) then we stay in the region
                if (region->endManeuver == endManeuverIndex)
                {
                    return TRUE;
                }
                // Check if the end point of the region comes into view. If it is not in view then
                // we can show the region.
                if (! IsPointVisible(enhancedContentState, region->endPoint.latitude, region->endPoint.longitude))
                {
                    return TRUE;
                }

                // this case is added to fix bug 138926. QA insist it should be same with Android, through it is incorrect.
                if ((distanceFromStart + enhancedContentState->configuration.maximumJunctionViewDistanceMeters) <= regionEnd)
                {
                    return TRUE;
                }
            }
            break;
        }

        case NB_RCRT_JunctionModel:
        {
//            if (regionEnd > distanceFromStart)
//            {
//                TEST_LOGGING("MJO - CanShowRegion: Current Distance: %1.1f, Current - avatar: %1.1f, Current + constant: %1.1f, Region, start: %1.1f, end: %1.1f", 
//                             distanceFromStart,
//                             distanceFromStart - enhancedContentState->configuration.maximumCameraToAvatarDistanceMeters,
//                             distanceFromStart + enhancedContentState->configuration.maximumJunctionViewDistanceMeters,
//                             region->distanceFromRouteStart,
//                             regionEnd);
//            }
            
            if ((region->distanceFromRouteStart == 0
                 || (distanceFromStart - enhancedContentState->configuration.maximumCameraToAvatarDistanceMeters) >= region->distanceFromRouteStart)
                && (distanceFromStart + enhancedContentState->configuration.maximumJunctionViewDistanceMeters) <= regionEnd)
            {
                return TRUE;
            }
            break;
        }

        case NB_RCRT_Invalid:
            break;
    }

    return FALSE;
}

/*! Check if the given point is in the visible viewing plane.

    The viewing plane has to be calculated first.

    @return 'TRUE' if point is visible, 'FALSE' otherwise

    @see SetupViewingPlane
*/
nb_boolean
IsPointVisible(NB_EnhancedContentState* enhancedContentState, double latitude, double longitude)
{
    NB_Vector point = {0};

    // Convert to mercator. Don't use vector_from_lat_lon()!
    NB_SpatialConvertLatLongToMercator(latitude, longitude, &point.x, &point.y);

    // Check if the point is in the viewing plane. The function assumes a convex polygon and clockwise order of points.
    return vector_is_point_in_polygon(&point, enhancedContentState->mapFrameData.viewingPlane, VIEWING_PLANE_POINT_COUNT);
}

/*! Check if the given tile is in the given area/polygon.

    @return 'TRUE' if tile is inside the given area, 'FALSE' otherwise

    @see SetupViewingPlane
*/
nb_boolean
IsTileInArea(const NB_Vector* area,             /*!< Polygon of area to compare the tile to */
             int pointCount,                    /*!< Number of points in area (polygon). */
             const NB_CityBoundingBox* box      /*!< Tile to check */
             )
{
    // Tile corners of tile bounding box in mercator coordinates
    NB_Vector tileCorners[4];
    nsl_memset(tileCorners, 0, sizeof(tileCorners));

    /*
        @todo: Optimization needed.
        We currently convert a lot of coordinates (both tile corners for every tile) from lat/lon to
        mercator. It should be possible to just find the corners of the viewing plane and then just
        pass in the tiles in mercator coordinates. That would be way faster. For not we just use 
        the same algorithm as the JCC.
    */

    // Convert top-left and bottom-right corners. Don't use vector_from_lat_lon()!
    NB_SpatialConvertLatLongToMercator(box->point1.latitude, box->point1.longitude, &tileCorners[0].x, &tileCorners[0].y);
    NB_SpatialConvertLatLongToMercator(box->point2.latitude, box->point2.longitude, &tileCorners[2].x, &tileCorners[2].y);

    // Set top-right and bottom-left corners
    tileCorners[1].x = tileCorners[2].x;
    tileCorners[1].y = tileCorners[0].y;
    tileCorners[3].x = tileCorners[0].x;
    tileCorners[3].y = tileCorners[2].y;

    return vector_polygons_intersect(tileCorners, 4, TRUE, area, pointCount, FALSE);
}

/*! Return the tile matrix based on the given area/polygon.

    This function returns the area where the area lies. It might contain tiles which are not actually visible if 
    the area is at an angle to the tiles. So the returned tiles need to be double checked against visibility.

    @return 'TRUE' on success, 'FALSE' otherwise
*/
nb_boolean 
GetTileArray(const NB_Vector* area,                 /*!< Area/Polygon to check for tiles */
             int pointCount,                        /*!< Number of points in area/polygon */
             const CityParameters* cityParameters,  /*!< Citi parameters for region to check */
             uint32* leftColumn,                    /*!< On return the columns/rows of the tiles where the viewing plane lies */
             uint32* rightColumn, 
             uint32* topRow, 
             uint32* bottomRow,
             nb_boolean validateBoundaries          /*!< If set to TRUE then we make sure that the given area is inside the city boundaries.
                                                         If the area is outside the city boundaries then the function returns FALSE. */
             )
{
    const uint32 INVALID_INDEX = 0xFFFFFFFF;
    nb_boolean result = FALSE;

    double minLatitude  = 0.0;
    double maxLatitude  = 0.0;
    double minLongitude = 0.0;
    double maxLongitude = 0.0;
    int i = 0;

    *leftColumn     = INVALID_INDEX;
    *rightColumn    = INVALID_INDEX;
    *topRow         = INVALID_INDEX;
    *bottomRow      = INVALID_INDEX;

    /*
        We have to first convert the are to lat/lon values. We then find the minimum and maximum lat/lon values.
    */

    // Convert first point. Don't use vector_to_lat_lon()!
    NB_SpatialConvertMercatorToLatLong(area[0].x, area[0].y, &minLatitude, &minLongitude);
    maxLatitude = minLatitude;
    maxLongitude = minLongitude;

    // Convert remaining points
    for (i = 1; i < pointCount; ++i)
    {
        double latitude  = 0.0;
        double longitude = 0.0;

        // Convert to lat/lon. Don't use vector_to_lat_lon()!
        NB_SpatialConvertMercatorToLatLong(area[i].x, area[i].y, &latitude, &longitude);
        
        minLatitude  = MIN(latitude, minLatitude);
        minLongitude = MIN(longitude, minLongitude);

        maxLatitude  = MAX(latitude, maxLatitude);
        maxLongitude = MAX(longitude, maxLongitude);
    }

    // If the flag is set, then we check that the given area is inside the city boundaries
    if (validateBoundaries)
    {
//        TEST_LOGGING("Min-Max:      %1.5f, %1.5f, %1.5f, %1.5f", 
//                     minLatitude, 
//                     maxLatitude, 
//                     minLongitude, 
//                     maxLongitude);
//
//        TEST_LOGGING("Bounding-Box: %1.5f, %1.5f, %1.5f, %1.5f", 
//                     cityParameters->cityBoundingBox.point1.latitude, 
//                     cityParameters->cityBoundingBox.point2.latitude, 
//                     cityParameters->cityBoundingBox.point1.longitude, 
//                     cityParameters->cityBoundingBox.point2.longitude); 
        
        if ((minLatitude  < cityParameters->cityBoundingBox.point1.latitude) ||
            (maxLatitude  > cityParameters->cityBoundingBox.point2.latitude) ||
            (minLongitude < cityParameters->cityBoundingBox.point1.longitude) ||
            (maxLongitude > cityParameters->cityBoundingBox.point2.longitude))
        {
            TEST_LOGGING("! GetTileArray(): Area outside city boundaries, return FALSE.");
            return FALSE;
        }
    }
    
    /*
        Now find the column/rows of the tile array based on the lat/lon values
    */

    // Check columns. Use '<=' because need to check the extra column for end comparison.
    for (i = 0; i <= (signed)cityParameters->tileColumnCount; ++i)
    {
        if (*leftColumn == INVALID_INDEX)
        {
            if (cityParameters->columnLongitudes[i] > minLongitude)
            {
                *leftColumn = MAX(i - 1, 0);
            }
        }
        if (*rightColumn == INVALID_INDEX)
        {
            if (cityParameters->columnLongitudes[i] > maxLongitude)
            {
                *rightColumn = MAX(i - 1, 0);
                break;
            }
        }
    }
    // If the longitude is outside the city boundary then we make sure we include the last column
    if (*rightColumn == INVALID_INDEX)
    {
        *rightColumn = cityParameters->tileColumnCount - 1;
    }

    // Check rows. Use '<=' because need to check the extra row for end comparison.
    for (i = 0; i <= (signed)cityParameters->tileRowCount; ++i)
    {
        if (*topRow == INVALID_INDEX)
        {
            if (cityParameters->rowLatitudes[i] > minLatitude)
            {
                *topRow = MAX(i - 1, 0);
            }
        }

        if (*bottomRow == INVALID_INDEX)
        {
            if (cityParameters->rowLatitudes[i] > maxLatitude)
            {
                *bottomRow = MAX(i - 1, 0);
                break;
            }
        }
    }
    // If the latitude is outside the city boundary then we make sure we include the last row
    if (*bottomRow == INVALID_INDEX)
    {
        *bottomRow = cityParameters->tileRowCount - 1;
    }
    
    // All values have to be valid
    result = (nb_boolean)((*leftColumn != INVALID_INDEX) && (*rightColumn != INVALID_INDEX) && (*topRow != INVALID_INDEX) && (*bottomRow != INVALID_INDEX));

    if (! result)
    {
        TEST_LOGGING(" !!! GetTileArray(): INVALID INDICES");
    }
    
    return result;
}

/*! Generate map frame data for the current content region.

    @return TRUE if map frame data is ready; FALSE otherwise
*/
static nb_boolean
UpdateMapFrameData(NB_EnhancedContentState* enhancedContentState)
{
    nb_boolean tilesAvailable = FALSE;
    NB_EnhancedContentStateData availableMapFrameType = NB_ECSD_None;

    CSL_Vector* tileIds = NULL;
    char fullTilePath[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1] = { 0 };

    // Verify currently in a content region
    ContentRegion* currentRegion = GetCurrentContentRegion(enhancedContentState);
    if (!currentRegion)
    {
        return FALSE;
    }

    switch (currentRegion->region.type)
    {
        case NB_RCRT_CityModel:
        {
            CSL_Vector* boundingBoxes = CSL_VectorAlloc(sizeof(NB_CityBoundingBox));
            if(!boundingBoxes)
            {
                return FALSE;
            }

            tilesAvailable = GetCityModelTiles(enhancedContentState, boundingBoxes);

            if (tilesAvailable)
            {
                // All city model tiles are available, add each tile to tile list
                int tileCount = CSL_VectorGetLength(boundingBoxes);
                int index = 0;

                tileIds = CSL_VectorAlloc(sizeof(char*));
                if (!tileIds)
                {
                    CSL_VectorDealloc(boundingBoxes);
                    return FALSE;
                }

                for (index = 0; index < tileCount; index++)
                {
                    NB_CityBoundingBox* tile = (NB_CityBoundingBox*)CSL_VectorGetPointer(boundingBoxes, index);

                    if (NB_EnhancedContentManagerGetDataItemPath(
                        enhancedContentState->enhancedContentManager,
                        NB_ECDT_CityModel,
                        currentRegion->region.datasetId,
                        tile->name, FALSE, sizeof(fullTilePath), fullTilePath) == NE_OK)
                    {
                        char* copiedFullTilePath = nsl_strdup(fullTilePath);
                        if (!CSL_VectorAppend(tileIds, &copiedFullTilePath))
                        {
                            nsl_free(copiedFullTilePath);
                            CSL_VectorDealloc(boundingBoxes);
                            CSL_VectorDealloc(tileIds);
                            return FALSE;
                        }
                    }
                }

                // Notify that complete city model map frame is available
                availableMapFrameType = NB_ECSD_CityModelMapFrame;
                currentRegion->lastShowTimestampSec = PAL_ClockGetUnixTime();

                // @todo Remove it because it's an empty func only returning NOT_SUPPORTED.
                //(void)NB_AnalyticsAddEnhancedContent(enhancedContentState->context, NB_AECD_CityModels, COUNTRY_CODE);
            }

            CSL_VectorDealloc(boundingBoxes);
            break;
        }

        case NB_RCRT_JunctionModel:
        {
            // Verify that motorway junction object data item is available
            tilesAvailable = NB_EnhancedContentManagerIsDataAvailableWithDependencies(
                enhancedContentState->enhancedContentManager,
                NB_ECDT_MotorwayJunctionObject,
                NB_EnhancedContentManagerGetMotorwayJunctionObjectsDatasetId(enhancedContentState->enhancedContentManager),
                currentRegion->region.regionId);

            // If junction tile is available, add to tile list
            if (tilesAvailable)
            {
                tileIds = CSL_VectorAlloc(sizeof(char*));

                if (NB_EnhancedContentManagerGetDataItemPath(
                    enhancedContentState->enhancedContentManager,
                    NB_ECDT_MotorwayJunctionObject,
                    currentRegion->region.datasetId,
                    currentRegion->region.regionId,
                    FALSE, sizeof(fullTilePath), fullTilePath) == NE_OK)
                {
                    // Append full path for single junction tile
                    char* copiedFullTilePath = nsl_strdup(fullTilePath);
                    if (!CSL_VectorAppend(tileIds, &copiedFullTilePath))
                    {
                        nsl_free(copiedFullTilePath);
                        CSL_VectorDealloc(tileIds);
                        return FALSE;
                    }

                    // Notify that complete junction map frame is available
                    availableMapFrameType = NB_ECSD_JunctionMapFrame;

                    // @todo Remove it because it's an empty func only returning NOT_SUPPORTED.
                    //(void)NB_AnalyticsAddEnhancedContent(enhancedContentState->context, NB_AECD_Junction, COUNTRY_CODE);
                }
                else
                {
                    // Could not get full data item path
                    tilesAvailable = FALSE;
                }
            }
            break;
        }

        case NB_RCRT_Invalid:
            break;
    }

    if (tilesAvailable)
    {
        // Compare new tiles to existing tiles to determine if callback should be made
        nb_boolean tilesChanged = !AreTileVectorsEqual(tileIds, enhancedContentState->currentTiles);

        // Free previous map frame tiles
        FreeMapFrameTiles(enhancedContentState);

        // Set verified tile and content type, and notify client via callback if tiles have changed
        enhancedContentState->currentTiles = tileIds;
        enhancedContentState->availableContent |= availableMapFrameType;

        if (tilesChanged)
        {
            enhancedContentState->stateChanged = TRUE;
        }
    }

    return tilesAvailable;
}

/*! Compare two tile vectors to determine if they are equal

*/
static nb_boolean
AreTileVectorsEqual(CSL_Vector* tileVector1, CSL_Vector* tileVector2)
{
    int tileVector1Count = 0;
    int tileVector2Count = 0;
    int index = 0;

    if (tileVector1 == NULL)
    {
        return (nb_boolean)(tileVector2 == NULL);
    }
    
    if (tileVector2 == NULL)
    {
        return (nb_boolean)(tileVector1 == NULL);
    }

    tileVector1Count = CSL_VectorGetLength(tileVector1);
    tileVector2Count = CSL_VectorGetLength(tileVector2);
 
    if (tileVector1Count != tileVector2Count)       
    {
        // Different tile count so not equal
        return FALSE;
    }
    
    for (index = 0; index < tileVector1Count; index++)
    {
        char** tileVector1Id = (char**)CSL_VectorGetPointer(tileVector1, index);
        char** tileVector2Id = (char**)CSL_VectorGetPointer(tileVector2, index);
        
        if (nsl_strcmp(*tileVector1Id, *tileVector2Id) != 0)
        {
            // Tile ID different so not equal
            return FALSE;
        }
    }
    
    // Tile vectors are equal
    return TRUE;
}

static void
FreeMapFrameTiles(NB_EnhancedContentState* enhancedContentState)
{
    if (enhancedContentState->currentTiles != NULL)
    {
        int count = CSL_VectorGetLength(enhancedContentState->currentTiles);
        int index = 0;

        for (index = 0; index < count; index++)
        {
            char** tileId = (char**)CSL_VectorGetPointer(enhancedContentState->currentTiles, index);
            nsl_free(*tileId);
        }
        CSL_VectorDealloc(enhancedContentState->currentTiles);
        enhancedContentState->currentTiles = NULL;
    }
}

/*! Determine what tiles are needed for current city model content region, and check if they are ready.

    If all needed tiles are present, the provided tiles CSL_Vector will contain the NB_CityBoundingBox for each.
    
    @return TRUE if any tiles needed for current location are ready; FALSE if no tiles are ready.
*/
static nb_boolean
GetCityModelTiles(NB_EnhancedContentState* enhancedContentState, 
                  CSL_Vector* tiles                                 /*!< On return will contain the tiles */
                  )
{
    CityParameters* cityParameters = NULL;

    uint32 leftColumn = 0;
    uint32 rightColumn = 0;
    uint32 bottomRow = 0;
    uint32 topRow = 0;

    uint32 column = 0;
    uint32 row = 0;

    ContentRegion* currentRegion = GetCurrentContentRegion(enhancedContentState);
    if (! currentRegion)
    {
        return FALSE;
    }

    // Verify that we have the city parameters for the current region
    if (GetCityParametersIfNecessary(enhancedContentState, &currentRegion->region) == FALSE)
    {
        // Error in retrieving city parameters
        return FALSE;
    }
    cityParameters = enhancedContentState->currentCityParameters;

    // Get the viewable area. This can still contain invisible tiles. Validate the city boundaries.
    if (!GetTileArray(enhancedContentState->mapFrameData.viewingPlane, 
                      VIEWING_PLANE_POINT_COUNT, 
                      cityParameters, 
                      &leftColumn, 
                      &rightColumn, 
                      &topRow, 
                      &bottomRow,
                      TRUE))
    {
        return FALSE;
    }

    TEST_LOGGING("");
    
    // Go through all the tiles and check if the tile is actually visible. Some of the corner tiles can be invisible, in which
    // case we don't want to load them.
    for (column = leftColumn; column <= rightColumn; column++)
    {
        for (row = topRow; row <= bottomRow; row++)
        {
            uint32 tileIndex = ConvertCityTileColumnRowToIndex(column, row, cityParameters->tileRowCount);

            NB_CityBoundingBox* tile = &cityParameters->tileBoxes[tileIndex];

            if (IsTileInArea(enhancedContentState->mapFrameData.viewingPlane, VIEWING_PLANE_POINT_COUNT, tile))
            {
                // Check if we have all the data (including textures) for this tile.
                /*
                    @todo: This checks for every tile for every gps fix if the city-textures are available. We should only
                           check the city textures once and only check the tile-data here.
                */
                
                if (NB_EnhancedContentManagerIsDataAvailableWithDependencies(
                        enhancedContentState->enhancedContentManager,
                        NB_ECDT_CityModel,
                        cityParameters->datasetId,
                        tile->name))
                {
//                    TEST_LOGGING("GetCityModelTiles() Tile %s available", tile->name);
                    
                    CSL_VectorAppend(tiles, tile);
                }
                else
                {
                    TEST_LOGGING("GetCityModelTiles() Tile %s NOT available", tile->name);
                    
                    CSL_VectorRemoveAll(tiles);
                    return FALSE;
                }
            }
        }
    }

    return (nb_boolean)(CSL_VectorGetLength(tiles) > 0);
}

/*! Free current city parameters and associated allocated data.

*/
static void
FreeCityParameters(NB_EnhancedContentState* enhancedContentState)
{
    if (enhancedContentState->currentCityParameters != NULL)
    {
        nsl_free(enhancedContentState->currentCityParameters->datasetId);
        nsl_free(enhancedContentState->currentCityParameters->version);
        nsl_free(enhancedContentState->currentCityParameters->tileBoxes);
        nsl_free(enhancedContentState->currentCityParameters->columnLongitudes);
        nsl_free(enhancedContentState->currentCityParameters->rowLatitudes);
        nsl_free(enhancedContentState->currentCityParameters);
        enhancedContentState->currentCityParameters = NULL;
    }
}

/*! Get the city parameters for the given content region, if not retrieved, yet.

    This function frees any old city parameters.
    The city parameters will be stored in the enhanced content state on success.

    @return TRUE if city parameters successfully retrieved and assembled; FALSE otherwise
*/
static nb_boolean
GetCityParametersIfNecessary(NB_EnhancedContentState* enhancedContentState, 
                             const NB_RouteContentRegion* region                /*!< Content region for which to retrieve the city parameters for */
                             )
{
    CityParameters newCityParameters = { 0 };
    CityParameters* oldCityParameters = enhancedContentState->currentCityParameters;
    NB_Error result = NE_OK;
    uint32 column = 0;
    uint32 row = 0;
    uint32 index = 0;
    char* underscore = NULL;
    char* endOfNumber = NULL;
    
    // Verify city parameters exist and is correct for current region
    if (oldCityParameters &&
        (nsl_stricmp(oldCityParameters->datasetId, region->datasetId) == 0) &&
        (nsl_stricmp(oldCityParameters->version, region->version) == 0))
    {
        // Existing city parameter match the region. Return success.
        return TRUE;
    }

    // Free previous city parameters (if set)
    FreeCityParameters(enhancedContentState);

    // Get bounding box for city
    result = NB_EnhancedContentManagerGetCityBoundingBox(
        enhancedContentState->enhancedContentManager,
        region->datasetId,
        &newCityParameters.cityBoundingBox);

    if (result != NE_OK)
    {
        // No city bounding box available
        return FALSE;
    }

    // Get tile bounding boxes for city
    result = NB_EnhancedContentManagerGetCityTileBoundingBoxes(
                enhancedContentState->enhancedContentManager,
                region->datasetId,
                &newCityParameters.tileBoxesCount,
                &newCityParameters.tileBoxes);

    if (result != NE_OK || newCityParameters.tileBoxesCount == 0 || newCityParameters.tileBoxes == NULL)
    {
        // No city tile boxes returned
        return FALSE;
    }

    // @todo FROM JCC: Need to find another way to determine row and column count,
    // because the following method is not universal!
    // Look at last tile in returned tile bounding box array, parse the X & Y from the
    // box name, and assume those values plus 1 are the column and row counts.
    underscore = nsl_strchr(newCityParameters.tileBoxes[newCityParameters.tileBoxesCount - 1].name, '_');

    if (underscore != NULL)
    {
        newCityParameters.tileColumnCount = (uint32)nsl_strtoul(newCityParameters.tileBoxes[newCityParameters.tileBoxesCount - 1].name, &endOfNumber, 10);
        newCityParameters.tileRowCount = (uint32)nsl_strtoul(underscore + 1, &endOfNumber, 10);
    }

    if (newCityParameters.tileColumnCount == 0 || newCityParameters.tileRowCount == 0)
    {
        // Unable to determine row & column count from last box tile name
        nsl_free(newCityParameters.tileBoxes);
        return FALSE;
    }

    // Column and row from tile name are zero-based, so increment them for the count
    newCityParameters.tileColumnCount++;
    newCityParameters.tileRowCount++;

    // Build lat/lon arrays for tile matrix to use for determining correct tile for
    // a location in city, adding an extra column and row for end comparison
    newCityParameters.columnLongitudes = nsl_malloc((newCityParameters.tileColumnCount + 1) * sizeof(double));
    newCityParameters.rowLatitudes = nsl_malloc((newCityParameters.tileRowCount + 1) * sizeof(double));

    if (newCityParameters.columnLongitudes == NULL || newCityParameters.rowLatitudes == NULL)
    {
        // Unable to allocate column and/or row arrays
        nsl_free(newCityParameters.tileBoxes);
        nsl_free(newCityParameters.columnLongitudes);
        nsl_free(newCityParameters.rowLatitudes);
        return FALSE;
    }

    // Build column longitude array
    for (column = 0; column < newCityParameters.tileColumnCount; column++)
    {
        index = ConvertCityTileColumnRowToIndex(column, 0, newCityParameters.tileRowCount);
        newCityParameters.columnLongitudes[column] = newCityParameters.tileBoxes[index].point1.longitude;
    }
    index = ConvertCityTileColumnRowToIndex(newCityParameters.tileColumnCount - 1, 0, newCityParameters.tileRowCount);
    newCityParameters.columnLongitudes[newCityParameters.tileColumnCount] = newCityParameters.tileBoxes[index].point2.longitude;
    
    // Build row latitude array
    for (row = 0; row < newCityParameters.tileRowCount; row++)
    {
        index = ConvertCityTileColumnRowToIndex(0, row, newCityParameters.tileRowCount);
        newCityParameters.rowLatitudes[row] = newCityParameters.tileBoxes[index].point1.latitude;
    }
    index = ConvertCityTileColumnRowToIndex(0, newCityParameters.tileRowCount - 1, newCityParameters.tileRowCount);
    newCityParameters.rowLatitudes[newCityParameters.tileRowCount] = newCityParameters.tileBoxes[index].point2.latitude;

    // Success -- finish city parameters and allocate a copy for enhanced content state
    newCityParameters.datasetId = nsl_strdup(region->datasetId);
    newCityParameters.version = nsl_strdup(region->version);

    enhancedContentState->currentCityParameters = nsl_malloc(sizeof(CityParameters));

    if (!enhancedContentState->currentCityParameters)
    {
        return FALSE;
    }

    *enhancedContentState->currentCityParameters = newCityParameters;

    return TRUE;
}

/*! Convert tile array column and row to index.

    @return index into one-dimensional tile array for given column and row
*/
static uint32
ConvertCityTileColumnRowToIndex(uint32 column, uint32 row, uint32 rowCount)
{
    return column * rowCount + row;
}

/*! Prefetch all city tiles.

    This should only be called if the content regions change!

    @return None
*/
static NB_Error
PrefetchCityModelTiles(NB_EnhancedContentState* enhancedContentState,
                       NB_RouteInformation* route)
{
    /*
        This function has to be called only once we have new content regions (after getting a new route).
    */

    // @todo: Get from preferences
    double prefetchTolerance = 1.2;
    NB_Error err = NE_OK;
    CameraProjectionContext*    context     = &enhancedContentState->cameraContext;
    CameraProjectionParameters* parameters  = &enhancedContentState->cameraParameters;

    double segmentExtensionLength = (parameters->avatarToHorizonHorizontalMeters * prefetchTolerance) 
                                  / context->cosLatRadius;

    double segmentPrefetchWindowWidth = ((parameters->tangentHalfHorizontalFieldOfView * parameters->cameraToHorizonMeters) * prefetchTolerance) 
                                      / context->cosLatRadius;

    int regionIndex = 0;
    int regionCount = CSL_VectorGetLength(enhancedContentState->contentRegions);

    // Create vector for city tiles. The vector contains PrefetchEntry.
    CSL_Vector* cityTiles = CSL_VectorAlloc(sizeof(PrefetchEntry));
    if (!cityTiles)
    {
        return NE_NOMEM;
    }

    // Go through all regions. 
    for (regionIndex = 0; regionIndex < regionCount; regionIndex++)
    {
        NB_RouteContentRegion* region = &(((ContentRegion*)CSL_VectorGetPointer(enhancedContentState->contentRegions, regionIndex))->region);
        uint32 maneuverIndex = 0;
        CityParameters* cityParameters = NULL;
        
        // Used for prefetch tile sorting
        int distanceIndex = 0;

        // Only check ECM regions, we prefetch MJOs in PrefetchMotorwayJunctions()
        if (region->type != NB_RCRT_CityModel)
        {
            continue;
        }

        // If the city is unselected then we don't prefetch anything for that city
        if (!NB_EnhancedContentManagerIsCitySelected(enhancedContentState->enhancedContentManager, region->datasetId))
        {
            TEST_LOGGING("City unselected, don't prefetch: %s", region->datasetId);
            continue;
        }

        // If the city is already preloaded then we don't need to prefetch anything for that city
        if (NB_EnhancedContentManagerIsCityAvailable(enhancedContentState->enhancedContentManager, region->datasetId))
        {
            TEST_LOGGING("City preloaded, don't prefetch: %s", region->datasetId);
            continue;
        }

        // If textures for this city are not yet downloaded, request them.

        if (!NB_EnhancedContentManagerIsCityTextureAvailable(enhancedContentState->enhancedContentManager, region->datasetId))
        {
            TEST_LOGGING("Request city textures for prefetch: %s", region->datasetId);


            // Request textures for this city.
            /*
                @todo: This function needs to check if the download request is already in progress, if so don't re-request it again!!!
             */
            err = NB_EnhancedContentManagerRequestDataDownload(enhancedContentState->enhancedContentManager, 
                                                         NB_ECDT_SpecificTexture,
                                                         region->datasetId,
                                                         NULL,
                                                         NULL);
            if (err != NE_OK)
            {
                goto cleanup;
            }
        }

        /*
            This function loads the city parameter for the given region into the currentCityParameters in the enhancedContentState.
            If we try to prefetch multiple ECM cities then this would overwrite the currentCityParameters and once we enter the
            first region it would then retrieve the city parameters for the first city again. This would only happen if we go through
            multiple 3D cities on one route, which is not very likely. 
        */
        if (!GetCityParametersIfNecessary(enhancedContentState, region))
        {
            continue;
        }
        cityParameters = enhancedContentState->currentCityParameters;

        // For all maneuvers in that region
        for (maneuverIndex = region->startManeuver; maneuverIndex <= region->endManeuver; ++maneuverIndex)
        {
            int segmentIndex = 0;
            int segmentCount = 0;
            double segmentDistance = 0;

            const data_nav_maneuver* maneuver = NB_RouteInformationGetManeuver(route, maneuverIndex);
            if (!maneuver)
            {
                continue;
            }

            // For all segments in this maneuver
            segmentCount = maneuver->polyline.numsegments;
            for (segmentIndex = 0; segmentIndex < segmentCount; segmentIndex++) 
            {
                const pl_segment* segment = &maneuver->polyline.segments[segmentIndex];

                // Did we reach the end of the region
                if ((maneuverIndex == region->endManeuver) && (segmentDistance > region->endManeuverOffset))
                {
                    // No need to prefetch more tiles for this region
                    break;
                }

                // Add segment length. The length is given in "fixed point" values. Divide by POLYLINE_LEN_SCALE_FACTOR to get the correct length
                segmentDistance += segment->len / POLYLINE_LEN_SCALE_FACTOR;

                if ((maneuverIndex == region->startManeuver) && (segmentDistance < region->startManeuverOffset)) 
                {
                    // We haven't reach the start position of the region, yet. Skip this segment.
                    continue;
                }

                // Is it the last segment
                if (segmentIndex == (segmentCount - 1))
                {
                    // Prefetch all ECM tiles for this segment
                    /*
                        @todo: Java uses the maneuver data-point as the next-segment. That doesn't seem to be right.
                               I would assume the data-point of the maneuver is at the start of the maneuver. 
                               Should we use the next maneuver???
                    */
                    // The segment values are given in "fixed point" values, we have to devide it by POLYLINE_LL_SCALE_FACTOR to 
                    // get the real latitude/longitude values.
                    err = PrefetchSegment(segment->lat / POLYLINE_LL_SCALE_FACTOR,
                                    segment->lon / POLYLINE_LL_SCALE_FACTOR,
                                    maneuver->point.lat,
                                    maneuver->point.lon,
                                    segmentExtensionLength,
                                    segmentPrefetchWindowWidth,
                                    cityParameters,
                                    cityTiles,
                                    &distanceIndex);
                    if (err != NE_OK)
                    {
                        goto cleanup;
                    }
                }
                else
                {
                    const pl_segment* nextSegment = &maneuver->polyline.segments[segmentIndex + 1];

                    // Prefetch all ECM tiles for this segment
                    // The segment values are given in "fixed point" values, we have to devide it by POLYLINE_LL_SCALE_FACTOR to 
                    // get the real latitude/longitude values.
                    err = PrefetchSegment(segment->lat / POLYLINE_LL_SCALE_FACTOR,
                                    segment->lon / POLYLINE_LL_SCALE_FACTOR,
                                    nextSegment->lat / POLYLINE_LL_SCALE_FACTOR,
                                    nextSegment->lon / POLYLINE_LL_SCALE_FACTOR,
                                    segmentExtensionLength,   
                                    segmentPrefetchWindowWidth,
                                    cityParameters,
                                    cityTiles,
                                    &distanceIndex);
                    if (err != NE_OK)
                    {
                        goto cleanup;
                    }
                } 
            }
        }

        CSL_VectorRemoveDuplicates(cityTiles, &TileIndexCompareFunction);

        // Now we want to sort the vector again by the distance so that we download closest tiles first
        CSL_VectorSort(cityTiles, &DistanceIndexCompareFunction);
        
        /*
            Request all tiles for download.

            @todo: We should have a function to request the entire array at once!
        */
        CSL_VectorForEach(cityTiles, &RequestCityTile, enhancedContentState);

        // Clear all city tiles for this region
        CSL_VectorRemoveAll(cityTiles);
    }

cleanup:
    CSL_VectorDealloc(cityTiles);
    return err;
}

/*! Prefetch segment of route.

    @return None

    @see PrefetchCityModelTiles
*/
static NB_Error
PrefetchSegment(double startLatitude,                   /*!< Start of segment */
                double startLongitude,                  /*!< Start of segment */
                double endLatitude,                     /*!< End of segment (start of next segment) */
                double endLongitude,                    /*!< End of segment (start of next segment) */
                double segmentExtensionLength,          /*!< Extension in meters of segment (view to horizon) */
                double segmentPrefetchWindowWidth,      /*!< Width of segment area to prefetch */
                const CityParameters* cityParameters,   /*!< City parameters for this region */
                CSL_Vector* cityTiles,                  /*!< Input/Output: City tiles for this segment get added to this vector. 
                                                             The vector contains the 'PrefetchEntry' */
                int* distanceIndex                      /*!< In/Out: Simple counter to indicate distance increments from start of route. */
                )
{
    double segmentStartX = 0.0;
    double segmentStartY = 0.0;
    double segmentEndX   = 0.0;
    double segmentEndY   = 0.0;

    double normalX = 0.0;
    double normalY = 0.0;

    NB_Vector segmentVector         = {0};
    NB_Vector segmentStart          = {0};
    NB_Vector segmentEnd            = {0};
    NB_Vector segmentVirtualEnd     = {0};
    NB_Vector segmentNormalisedExt  = {0};

    NB_Vector unit_z                = {0.0, 0.0, 1.0};
    NB_Vector normal                = {0};

    #define SEGMENT_POINT_COUNT     4
    NB_Vector segmentArea[SEGMENT_POINT_COUNT];

    nsl_memset(segmentArea, 0, sizeof(segmentArea));

    // Convert to mercator. Don't use vector_from_lat_lon()!
    NB_SpatialConvertLatLongToMercator(startLatitude, startLongitude, &segmentStartX, &segmentStartY);
    NB_SpatialConvertLatLongToMercator(endLatitude, endLongitude, &segmentEndX, &segmentEndY);

    normalX = segmentStartY - segmentEndY;
    normalY = segmentEndX - segmentStartX;
    if (normalX == 0 && normalY == 0)
    {
        return NE_INVAL;
    }

    vector_set(&segmentVector, segmentEndX - segmentStartX, segmentEndY - segmentStartY, 0);
    vector_set(&segmentStart, segmentStartX, segmentStartY, 0);
    vector_set(&segmentEnd, segmentEndX, segmentEndY, 0);

    vector_normalize_to_length(&segmentNormalisedExt, &segmentVector, segmentExtensionLength);
    vector_add(&segmentVirtualEnd, &segmentEnd, &segmentNormalisedExt);

    vector_cross_product(&normal, &segmentVector, &unit_z);
    vector_normalize_to_length(&normal, &normal, segmentPrefetchWindowWidth);

    /*
        Calculate corners of segment area. This is the area which we want to prefetch city tiles for.
    */
    vector_add(     &segmentArea[0], &segmentStart,      &normal);
    vector_add(     &segmentArea[1], &segmentVirtualEnd, &normal);
    vector_subtract(&segmentArea[2], &segmentVirtualEnd, &normal);
    vector_subtract(&segmentArea[3], &segmentStart,      &normal);

    /*
        Compare the segment area with the city tiles. And return all tiles which intersect.
    */
    {
        uint32 leftColumn = 0;
        uint32 rightColumn = 0;
        uint32 bottomRow = 0;
        uint32 topRow = 0;

        uint32 column = 0;
        uint32 row = 0;

        // Get tile array for the segment area. This can still contain invisible tiles. Don't validate boundaries for segment check
        // (We want to check as much of an area as possible)
        if (!GetTileArray(segmentArea, SEGMENT_POINT_COUNT, cityParameters, &leftColumn, &rightColumn, &topRow, &bottomRow, FALSE))
        {
            return NE_INVAL;
        }

        // Just increment the distance entry for each segment. The smaller the value the closer the tile is to the 
        // start of the route.
        ++(*distanceIndex);

        // Go through all the tiles and check if the tile is actually visible. Some of the corner tiles can be invisible, in which
        // case we don't want to prefetch them.
        for (column = leftColumn; column <= rightColumn; column++)
        {
            for (row = topRow; row <= bottomRow; row++)
            {
                uint32 tileIndex = ConvertCityTileColumnRowToIndex(column, row, cityParameters->tileRowCount);

                NB_CityBoundingBox* tile = &cityParameters->tileBoxes[tileIndex];

                if (IsTileInArea(segmentArea, SEGMENT_POINT_COUNT, tile))
                {
                    PrefetchEntry entry = {0};
                    entry.tileIndex = tileIndex;
                    entry.distanceIndex = *distanceIndex;
                    // Add the entry to the returned vector. The calling function will remove any duplicates.
                    if (!CSL_VectorAppend(cityTiles, &entry))
                    {
                        return NE_NOMEM;
                    }
                }
            }
        }
    }

    return NE_OK;
}

/*! Compares two PrefetchEntry's, using the tile-index to compare.
 
    @see PrefetchEntry
*/
int
TileIndexCompareFunction(const void* left, const void* right)
{
    const PrefetchEntry* _left  = (PrefetchEntry*)left;
    const PrefetchEntry* _right = (PrefetchEntry*)right;

    return IndexCompareFunction(&_left->tileIndex, &_right->tileIndex);
}

/*! Compares two PrefetchEntry's, using the distance-index to compare.
 
    @see PrefetchEntry
 */
int
DistanceIndexCompareFunction(const void* left, const void* right)
{
    const PrefetchEntry* _left  = (PrefetchEntry*)left;
    const PrefetchEntry* _right = (PrefetchEntry*)right;

    return IndexCompareFunction(&_left->distanceIndex, &_right->distanceIndex);
}

/*! Compare function for int compare.
 
    Subfunction of DistanceIndexCompareFunction() and TileIndexCompareFunction().

    The entries are 'int' values.

    @return see CSL_VectorCompareFunction

    @see CSL_VectorCompareFunction
    @see PrefetchCityModelTiles
*/
static int 
IndexCompareFunction(const void* left, const void* right)
{
    if (*((int*)left) < *((int*)right))
    {
        return -1;
    }
    if (*((int*)left) > *((int*)right))
    {
        return 1;
    }
    return 0;
}

/*! Request a city tile for download.

    This is a callback function from CSL_VectorForEach().

    @return see CSL_VectorForEach

    @see CSL_VectorForEach
    @see PrefetchCityModelTiles
*/
int 
RequestCityTile(void* userData, void* item)
{
    NB_Error result = NE_OK;
    NB_EnhancedContentState* state = (NB_EnhancedContentState*)userData;
    PrefetchEntry* entry = (PrefetchEntry*)item;

    const NB_CityBoundingBox* tile = &state->currentCityParameters->tileBoxes[entry->tileIndex];

    /*
        @todo: Do we have to check if the item is already available? I think NB_EnhancedContentManagerRequestDataDownload()
               should make that check, but I think it currently doesn't.
    */

    if (! NB_EnhancedContentManagerIsDataAvailable(state->enhancedContentManager,
                                                   NB_ECDT_CityModel,
                                                   state->currentCityParameters->datasetId,
                                                   tile->name))
    {
        TEST_LOGGING("Request city tile for prefetch: %s", tile->name);

        // Request the item for download. Use the tile name, is this correct?
        result = NB_EnhancedContentManagerRequestDataDownload(state->enhancedContentManager,
                                                              NB_ECDT_CityModel,
                                                              state->currentCityParameters->datasetId,
                                                              tile->name,
                                                              NULL);
    }
    else
    {
        TEST_LOGGING("City tile already available, no prefetch: %s", tile->name);
    }

    if (result != NE_OK)
    {
        // Not much we can do here. Don't return error, try to download other tiles.
    }

    // Return non-zero to continue
    return 1;
}

