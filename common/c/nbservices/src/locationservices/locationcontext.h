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

    @file     NBLocationContext.h
    @date     2/27/2010
    @defgroup NBLOCATIONSERVICES Location Services

    This is the C-Style DLL interface to the NB_LocationContext
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

/*! @{ */

#pragma once

extern "C" {
  #include "nblocationtypes.h"
  #include "nbwifiprobes.h"
  #include "nbcontext.h"
  #include "paldebuglog.h"
  #include "pal.h"
  #include "palradio.h"
}
#include "palmath.h"

class AbpalGps;
class GpsLocationRequest;
class CellIDLocationRequest;
class WifiLocationRequest;
class EmuLocationRequest;
class CombinedRequestManager;
class TrackerManager;
class StateManager;
class RadioInformation;
class LocationCacheDb;

//typedef struct NBContext NBContext;
typedef struct NB_LocationContext NB_LocationContext;

/*! @struct NB_LocationContext

    This is the opaque NB_LocationContext object
*/
struct NB_LocationContext {
    NB_Context*                        internalContext;         /*!< Link to internal NB_Context instance, that could be required for QA logging, etc.*/
    NB_LS_LocationConfig               locationConfig;          /*!< Location config */
    NB_LocationInitializeCallback      initializeCallback;      /*!< Callback to send result of location requests */
    const void*                        callbackData;            /*!< User data for callbacks */

    AbpalGps*                        m_abpalGps;               /*!< Pointer to ABPAL GPS object wrapper */
    GpsLocationRequest*              m_gpsLocationRequest;     /*!< Pointer to object which requests location via GPS */
    EmuLocationRequest*              m_emuLocationRequest;     /*!< Pointer to object which requests location from some emulation file */
    CombinedRequestManager*          m_combinedRequestManager; /*!< Pointer to object which requests location using all requests */
    TrackerManager*                  m_trackerManager;         /*!< Pointer to tracker object */
    StateManager*                    m_stateManager;           /*!< Object that holds the global LK state */
    RadioInformation*                m_radioInformation;       /*!< Object that retrieves cell ID and wifi information */
    LocationCacheDb*                 m_cacheDb;                /*!< Object that manages location cache DB */

    PAL_Instance*                    m_PAL_Instance;           /*!< Pointer to PAL interface */
    PAL_RadioContext*                m_PAL_RadioContext;       /*!< Pointer to PAL radio interface */
    NB_WifiProbes*                   m_wifiProbes;             /*!< Pointer to NB_WifiProbes object */

    int maxWiFiAge;                 /*!< Maximum age(in seconds) of wi-fi scanned networks to accept as current for location request (WF1). Set to 0 to get default value. */
    int networkLocationTimeout;     /*!< Maximum time(in seconds) to wait for a network-based location before a timeout is returned (TWF1). Set to 0 to get default value. */
    int wiFiNetworkScanTimeout;     /*!< Amount of time(in seconds) to wait for a Wi-Fi network scan to complete (TWF2). Set to 0 to get default value. */
    int maxFastSGPSAge;             /*!< Maximum age(in seconds) of a fix to be considered valid for a fast location request (SGPS1). Set to 0 to get default value. */
    int maxNormalSGPSAge;           /*!< Maximum age(in seconds) of a fix to be considered valid on a Normal or Accurate location request (SGPS2). Set to 0 to get default value. */
    int fastSGPSFixTimeout;         /*!< Timeout(in seconds) when waiting for an SGPS fix on a Fast request (TSGPS1). Set to 0 to get default value. */
    int normalSGPSFixTimeout;       /*!< Timeout(in seconds) when waiting for an SGPS fix on a Normal request (TSGPS2). Set to 0 to get default value. */
    int accurateSGPSFixTimeout;     /*!< Timeout(in seconds) when waiting for an SGSP fix on an Accurate request (TSGPS3). Set to 0 to get default value. */
    int cellIDRequestTimer;         /*!< Timer(in seconds) to request a new Cell ID position (TCELL). Set to 0 to get default value. */
    int wiFiScanTimer;              /*!< Timer(in seconds) to request a new WiFi network scan (TWIFI). Set to 0 to get default value. */
    uint16 gpsDesiredAccuracy;      /*!< Desired position. */
};

/*! @struct NB_LocationTrackingInfo

    It needs to support multiple tracking requests. It is not used after API simplification, but it might be used in the future.
*/
typedef struct NB_LocationTrackingInfo NB_LocationTrackingInfo;

/*! @} */
