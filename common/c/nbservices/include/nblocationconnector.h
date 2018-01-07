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

    @file     nblocationconnector.h
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

/*! @{ */

#pragma once

#include "abpalgps.h"
#include "nblocationtypes.h"
#include "nbcontext.h"

// API .............................................................................................
/*!
@defgroup NBLOCATIONFUNCTIONS Functions
@{
*/

// Location Services functions ...................................................................................

/*! This function creates the location povider instance.
    This function should be called before an other NB_LS_Location... functions to create the location provider.
    The caller is responsible for saving the pointer returned in the locationContext parameter.
    The context is then used in all subsequent calls to the NB Location API.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationContextCreate(
    NB_LocationContext ** locationContext  /*!< Pointer to the NB location context */
    );

/*! This function closes the location provider and releases all associated resources.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationContextDestroy(
    NB_LocationContext * locationContext   /*!< Pointer to the NB location context */
    );

/*! This function passes pointers to PAL and NB context to the location context.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationSetPalAndContext(
    NB_LocationContext* locationContext,  /*!< Pointer to the NB location context */
    PAL_Instance* pal,                     /*!< Pointer to the PAL instance */
    NB_Context* nbContext                  /*!< Pointer to the NB_Context instance required for internal needs. */
);

/*! This function is used to get a single location with predefined settings.
    Three different standard cases are supported: Fast, Normal and Accurate.
    The behavior of Location Services when this request is issued depends on the state
    of the SGPS hardware:

    \li If the SGPS hardware is warm (meaning it has acquired satellites and is ready
    to provide a fix immediately), then accurate SGPS fixes will be returned for all
    cases.

    \li If the SGPS hardware is not warm (meaning it needs to start looking for
    satellites, which can take a significant amount of time) then

    \li  - Fast will request a Cell ID/Wi-Fi based location from the server

    \li  - Normal will request both SGPS and Cell ID/Wi-Fi based locations, using a timeout of 10 seconds

    \li  - Accurate will request an SGPS fix with a timeout of 60 seconds. If SGPS cannot be acquired, 5 seconds before the timeout expires, the device will request a Cell ID/Wi-Fi based location and will fall back to it when the timer expires

    Note that you must call NB_LocationInitialize first.

    @returns NB_Error
    @see NB_LocationInitialize
    @see NB_LS_LocationCallback
*/
NB_DEC NB_Error NB_LocationGetOneFix(
    NB_LocationContext* locationContext,  /*!< Pointer to the NB location context */
    NB_Context* nbContext,                 /*!< Pointer to the client-specific NB_Context */
    NB_LS_LocationCallback callback,         /*!< Pointer to the callback function that will receive fixes */
    NB_LS_FixType fixType,                   /*!< Type of fix desired */
    void* appData                          /*!< Application context that will be passed with each fix */
    );

/*! This function cancels an outstanding location request that was previously initiated by NB_LocationGetOneFix.
    If several requests were added with the same callback, then all those requests are cancelled.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationCancelGetOneFix(
    NB_LocationContext*  locationContext,   /*!< Pointer to the NB location context */
    NB_LS_LocationCallback  callback,       /*!< Pointer to the callback function that was passed to
                                                 NB_LocationGetOneFix. */
    void* appData                           /*!< Pointer to the app-specific data.
                                                 If omitted (NULL), then all requests for this callback function are cancelled. */
    );

/*! This function is used to get the current state of the location provider.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationGetState(
    NB_LocationContext * locationContext,  /*!< Pointer to the NB location context */
    NB_LS_LocationState * locationState       /*!< Out parameter that will contain the location provider state */
    );

/*! This function is used to initiate a tracking session without having to define all
    the parameters. Instead, Location Services will use a default configuration and will
    take care of all the details internally.

    Note that you must call NB_LocationInitialize first.

    @returns NB_Error
    @see NB_LocationInitialize
    @see NB_LS_LocationCallback
    @see NB_LocationStopReceivingFixes
*/
NB_DEC NB_Error NB_LocationStartReceivingFixes(
    NB_LocationContext * locationContext,          /*!< Pointer to the NB location context */
    NB_Context* nbContext,                          /*!< Pointer to the client-specific NB_Context */
    NB_LS_LocationCallback callback,                  /*!< Pointer to the callback function that will receive fixes */
    void* appData                                   /*!< Application context that will be passed with each fix */
    );

/*! This function is used to stop the tracking session that was started by
    calling NB_LocationStartReceivingFixes.

    @returns NB_Error
    @see NB_LocationStartReceivingFixes
*/
NB_DEC NB_Error NB_LocationStopReceivingFixes(
    NB_LocationContext * locationContext,     /*!< Pointer to the NB location context */
    NB_LS_LocationCallback  callback,         /*!< Pointer to the callback function that was passed to
                                                   NB_LocationStartReceivingFixes.  */
    void* appData                             /*!< Pointer to the app-specific data.
                                                   If omitted (NULL), then all requests for this callback function will be cancelled. */
    );

/*! This function is used to initialize the location provider. It should be called before an other
    NB_LS_Location... functions to initialize location provider. Functions is available for AtlasBook only.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationInitialize(
    NB_LocationContext* locationContext,               /*!< Pointer to the NB location context */
    const NB_LS_LocationConfig* config,                   /*!< Pointer to the location configuration */
    const ABPAL_GpsConfig* gpsConfig,                   /*!< Pointer to the GPS configuration */
    uint32 gpsConfigCount,                              /*!< Array size of gpsconfig */
    NB_LocationInitializeCallback initializeCallback,  /*!< Callback function that is called when the provider is initialized */
    NB_LocationDeviceStateCallback devStateCallback,   /*!< Optional callback function that is called when the location HW state changes. Pass NULL if you do not wish to receive device state notifications */
    uint16 gpsDesiredAccuracy,                         /*!< Desired position. */
    void *callbackData                                  /*!< Optional pointer for user supplied data that will be passed to the callback */
    );

/*! This function is used to reset location tiles cache.

    @returns NB_Error
*/
NB_DEC NB_Error NB_LocationResetCache(
    NB_Context* nbContext);     /*!< Pointer to NB context */

/*! @} */

/*! @} */
