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

    @file     NBLocationConnector.cpp
    @date     05/05/2011
    @defgroup NBLOCATIONSERVICES Location Services

    This is the C-Style DLL interface to the NBLocationConnector functions
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/


extern "C" {
  #include "palfile.h"
  #include "nblocationconnector.h"
  #include <nbqalog.h>
  #include <nbcontextprotected.h>
  #include <nbcontextaccess.h>
  #include <nbwifiprobes.h>
}

#include "gpslocationrequest.h"
#include "locationcontext.h"
#include "lsabpalgps.h"
#include "cellIdlocationrequest.h"
#include "wifilocationrequest.h"
#include "emulocationrequest.h"
#include "combinedrequestmanager.h"
#include "trackermanager.h"
#include "statemanager.h"
#include "wifiprobes.h"
#include "radioinformation.h"
#include "locationcachedb.h"

/*!< Maximum age of wifi scanned networks to accept as current for location request (WF1). */
const int NBLK_MAX_WIFI_AGE = 10;
/*!< Maximum time to wait for a network-based location before a timeout is returned (TWF1). */
const int NBLK_NETWORK_LOCATION_TIMEOUT = 30;
/*!< Amount of time to wait for a Wi-Fi network scan to complete (TWF2). */
const int NBLK_WIFI_NETWORK_SCAN_TIMEOUT = 6;
/*!< Maximum age of a fix to be considered valid for a fast location request (SGPS1). */
const int NBLK_MAX_FAST_SGPS_AGE = 5;
/*!< Maximum age of a fix to be considered valid on a Normal or Accurate location request (SGPS2). */
const int NBLK_MAX_NORMAL_SGPS_AGE = 2;
/*!< Timeout when waiting for an SGPS fix on a Fast request (TSGPS1). */
const int NBLK_FAST_SGPS_FIX_TIMEOUT = 4;
/*!< Timeout when waiting for an SGPS fix on a Normal request (TSGPS2). */
const int NBLK_NORMAL_SGPS_FIX_TIMEOUT = 8;
/*!< Timeout when waiting for an SGSP fix on an Accurate request (TSGPS3). */
const int NBLK_ACCURATE_SGPS_FIX_TIMEOUT = 120;
/*!< Timer to request a new Cell ID position (TCELL). */
const int NBLK_CELLID_REQUEST_TIMER = 10;
/*!< Timer to request a new WiFi network scan (TWIFI). */
const int NBLK_WIFI_SCAN_TIMER = 15;

//-------------------------------------------------------------------------------
//                   Function prototypes
//-------------------------------------------------------------------------------
NB_Error DestroyCommonMembers(NB_LocationContext* locationContext);


//------------------------------------------------------------------------------- LocationContextCreate
NB_Error NB_LocationContextCreate(NB_LocationContext** locationContext)
{
    if (locationContext == NULL)
    {
        return NE_INVAL;
    }

    *locationContext = (NB_LocationContext*)nsl_malloc(sizeof(NB_LocationContext));

    if (*locationContext == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(*locationContext, 0, sizeof (NB_LocationContext));

    return NE_OK;
}

//------------------------------------------------------------------------------- LocationContextCreate
NB_Error NB_LocationContextDestroy(NB_LocationContext* locationContext)
{
    if (!locationContext)
    {
        return NE_INVAL;
    }

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, "");
    }

    DestroyCommonMembers(locationContext);

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    nsl_free(locationContext);

    return NE_OK;
}

//------------------------------------------------------------------------------- LocationContextInit
// Initialize common location context data
NB_Error NB_LocationSetPalAndContext(NB_LocationContext* locationContext, PAL_Instance* pal, NB_Context* nbContext)
{
    if (!locationContext || !pal || !nbContext)
    {
        return NE_INVAL;
    }

    NB_Error err = NE_OK;
    PAL_Error result = PAL_Ok;
    locationContext->m_PAL_Instance = pal;
    locationContext->internalContext = nbContext;

    if (locationContext->m_PAL_Instance != NULL)
    {
        result = PAL_RadioContextCreate(locationContext->m_PAL_Instance,
                                    PRST_All, &locationContext->m_PAL_RadioContext);
    }

    locationContext->m_cacheDb = new LocationCacheDb(locationContext->m_PAL_Instance);
    locationContext->m_abpalGps = new AbpalGps(locationContext);
    locationContext->m_gpsLocationRequest = new GpsLocationRequest(locationContext, locationContext->m_abpalGps);

    locationContext->m_combinedRequestManager = new CombinedRequestManager(locationContext);
    locationContext->m_trackerManager = new TrackerManager(locationContext);
    locationContext->m_stateManager = new StateManager(locationContext);
    locationContext->m_radioInformation = new RadioInformation(locationContext->m_PAL_RadioContext, locationContext->m_PAL_Instance);

    if (locationContext->m_PAL_Instance == NULL ||
        // it's possible situation for BREW devices that don't support ITelephone, but have GPS
        (result != PAL_ErrUnsupported && locationContext->m_PAL_RadioContext == NULL) ||
        locationContext->m_abpalGps == NULL ||
        locationContext->m_gpsLocationRequest == NULL ||
        locationContext->m_combinedRequestManager == NULL ||
        locationContext->m_trackerManager == NULL ||
        locationContext->m_stateManager == NULL ||
        locationContext->m_radioInformation == NULL ||
        locationContext->m_cacheDb == NULL)
    {
        err = NE_NOTINIT;
    }

    return err;
}

//------------------------------------------------------------------------------- LocationContextDestroy
// Destroy common location context data
NB_Error DestroyCommonMembers(NB_LocationContext* locationContext)
{
    if (!locationContext)
    {
        return NE_INVAL;
    }

    delete locationContext->m_radioInformation;
    delete locationContext->m_trackerManager;
    delete locationContext->m_stateManager;
    delete locationContext->m_combinedRequestManager;

    delete locationContext->m_gpsLocationRequest;
    delete locationContext->m_emuLocationRequest;
    delete locationContext->m_abpalGps;
    delete locationContext->m_cacheDb;

    locationContext->m_radioInformation = NULL;
    locationContext->m_trackerManager = NULL;
    locationContext->m_stateManager = NULL;
    locationContext->m_combinedRequestManager = NULL;

    locationContext->m_gpsLocationRequest = NULL;
    locationContext->m_emuLocationRequest = NULL;
    locationContext->m_abpalGps = NULL;
    locationContext->m_cacheDb = NULL;

    NB_ContextSetWifiProbesNoOwnershipTransfer(locationContext->internalContext, NULL);
    NB_WifiProbesDestroy(locationContext->m_wifiProbes);

    PAL_RadioContextDestroy(locationContext->m_PAL_RadioContext);

    return NE_OK;
}

//------------------------------------------------------------------------------- NB_LocationInitialize
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationInitialize(NB_LocationContext              *locationContext,
                                      const NB_LS_LocationConfig      *config,
                                      const ABPAL_GpsConfig           *gpsConfig,
                                      uint32                          gpsConfigCount,
                                      NB_LocationInitializeCallback   initializeCallback,
                                      NB_LocationDeviceStateCallback  devStateCallback,
                                      uint16                          gpsDesiredAccuracy,
                                      void                            *callbackData)
{
    if (!locationContext || !locationContext->m_stateManager || !locationContext->m_abpalGps || !config || !gpsConfig)
    {
        return NE_INVAL;
    }

    if (locationContext->m_stateManager->IsAlreadyInitialized())
    {
        // The whole thing is already initialized. But we may adjust additional parameters.
        // For now, it is warm-up fix. Later, might be something else in addition.
        if (config->warmUpFix)
        {
            locationContext->locationConfig.warmUpFix = TRUE;
            locationContext->m_abpalGps->StartWarmupFix();
        }

        if (initializeCallback)
        {
            initializeCallback(callbackData, locationContext);
        }

        return NE_OK;
    }

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, "");
    }

    locationContext->locationConfig.warmUpFix = config->warmUpFix;
    locationContext->locationConfig.emulationMode = config->emulationMode;
    nsl_strlcpy(locationContext->locationConfig.emulationFilename, config->emulationFilename, sizeof(locationContext->locationConfig.emulationFilename));
    locationContext->locationConfig.emulationPlayStart = config->emulationPlayStart;
    locationContext->locationConfig.collectWiFiProbes = config->collectWiFiProbes;

    // setup timeouts and fix max ages
    locationContext->maxWiFiAge = NBLK_MAX_WIFI_AGE;

    locationContext->networkLocationTimeout = NBLK_NETWORK_LOCATION_TIMEOUT;

    locationContext->wiFiNetworkScanTimeout = NBLK_WIFI_NETWORK_SCAN_TIMEOUT;

    locationContext->maxFastSGPSAge = NBLK_MAX_FAST_SGPS_AGE;

    locationContext->maxNormalSGPSAge = NBLK_MAX_NORMAL_SGPS_AGE;

    locationContext->fastSGPSFixTimeout = NBLK_FAST_SGPS_FIX_TIMEOUT;

    locationContext->normalSGPSFixTimeout = NBLK_NORMAL_SGPS_FIX_TIMEOUT;

    locationContext->accurateSGPSFixTimeout = NBLK_ACCURATE_SGPS_FIX_TIMEOUT;

    locationContext->cellIDRequestTimer = NBLK_CELLID_REQUEST_TIMER;

    locationContext->wiFiScanTimer = NBLK_WIFI_SCAN_TIMER;

    locationContext->gpsDesiredAccuracy = gpsDesiredAccuracy;

    locationContext->initializeCallback = initializeCallback;
    locationContext->callbackData = callbackData;

    locationContext->m_stateManager->SetUserCallback(devStateCallback, callbackData);

    NB_Error err = NE_OK;

    // Even if we asked for emulation mode, we have to re-create m_emuLocationRequest,
    // to re-read the emulation file.
    if (locationContext->m_emuLocationRequest)
    {
        delete locationContext->m_emuLocationRequest;
    }

    locationContext->m_emuLocationRequest = NULL;

    if (locationContext->locationConfig.emulationMode)
    {
        // Setup the emulator
        locationContext->m_emuLocationRequest = new EmuLocationRequest(locationContext,
            locationContext->locationConfig.emulationFilename,
            locationContext->locationConfig.emulationPlayStart);

        NB_LS_LocationState state = NBPGS_Undefined;
        if (locationContext->m_emuLocationRequest != NULL)
        {
            locationContext->m_emuLocationRequest->GetState(&state);
        }

        if (state != NBPGS_Initialized)
        {
            err = NEGPS_GENERAL;
        }
    }
    else
    {
        // Initialize GPS
        err = locationContext->m_abpalGps->Initialize(config, gpsConfig, gpsConfigCount);
    }

    if (err == NE_OK)
    {
        NB_Error probesErr = WifiProbesCreate(locationContext);

        if (probesErr == NE_OK)
        {
            err = WifiProbesStartAnalyticsTimer(locationContext);
        }
        else if (probesErr != NE_NOSUPPORT)
        {
            err = probesErr;
        }
   }

    // Will report the state NBPGS_Initialized from now on.
    if (err == NE_OK)
    {
        locationContext->m_stateManager->Initialize();
    }

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    return err;
}

//------------------------------------------------------------------------------- NB_LocationCancelGetOneFix
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationCancelGetOneFix( NB_LocationContext* locationContext, NB_LS_LocationCallback callback, void* appData )
{
    if (!locationContext || !locationContext->m_combinedRequestManager || !callback)
    {
        return NE_INVAL;
    }
    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, "");
    }

    locationContext->m_combinedRequestManager->RequestCancel(callback, appData);

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    return NE_OK;
}

//------------------------------------------------------------------------------- NB_LocationGetState
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationGetState( NB_LocationContext* locationContext, NB_LS_LocationState* locationState )
{
    if (!locationContext || !locationState || !locationContext->m_stateManager)
    {
        return NE_INVAL;
    }
    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, "");
    }

    *locationState = locationContext->m_stateManager->GetState();

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    return NE_OK;
}

//------------------------------------------------------------------------------- NB_LocationGetOneFix
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationGetOneFix(NB_LocationContext* locationContext,
                                        NB_Context* nbContext,
                                        NB_LS_LocationCallback callback,
                                        NB_LS_FixType fixType,
                                        void* appData)
{
    if (!locationContext || !locationContext->m_combinedRequestManager || !nbContext ||
            !callback || (fixType != NBFixType_Fast && fixType != NBFixType_Normal && fixType != NBFixType_Accurate))
    {
        return NE_INVAL;
    }

    char message[30] = {0};
    nsl_sprintf(message, "fixType %d", fixType);
    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, message);
    }

    NB_Error err = locationContext->m_combinedRequestManager->RequestStart(callback, appData, fixType, nbContext);

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    return err;
}

//------------------------------------------------------------------------------- NB_LocationStartReceivingFixes
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationStartReceivingFixes(NB_LocationContext* locationContext,
                                                  NB_Context* nbContext,
                                                  NB_LS_LocationCallback callback, void* appData)
{
    if (!locationContext || !locationContext->internalContext || !locationContext->m_trackerManager || !nbContext ||
            !callback)
    {
        return NE_INVAL;
    }
    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, "");
    }

    NB_Error err = locationContext->m_trackerManager->BeginTracking(callback, appData, nbContext);

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    return err;
}

//------------------------------------------------------------------------------- NB_LocationStopReceivingFixes
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationStopReceivingFixes(NB_LocationContext* locationContext, NB_LS_LocationCallback callback, void* appData)
{
    if (!locationContext || !locationContext->m_trackerManager || !callback)
    {
        return NE_INVAL;
    }
    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_StartFunction, __FUNCTION__, "");
    }

    locationContext->m_trackerManager->EndTracking(callback, appData);

    if (locationContext->internalContext)
    {
        NB_QaLogApiCall(locationContext->internalContext, NB_QLAC_EndFunction, __FUNCTION__, "");
    }

    return NE_OK;
}

//------------------------------------------------------------------------------- NB_LocationResetCache
// See nblocationconnector.h for description
NB_DEF NB_Error NB_LocationResetCache(NB_Context* nbContext)
{
    if (nbContext == NULL)
    {
        return NE_INVAL;
    }

    LocationCacheDb* cacheDb = new LocationCacheDb(NB_ContextGetPal(nbContext));
    if (cacheDb == NULL)
    {
        return NE_NOMEM;
    }

    cacheDb->ClearDatabase();
    delete cacheDb;

    return NE_OK;
}
