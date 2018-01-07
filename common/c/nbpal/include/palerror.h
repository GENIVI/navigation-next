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

@file palerror.h
@defgroup PALERROR_H PALERROR

@brief PAL Error enumeration

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALERROR_H
#define PALERROR_H

/*! PAL Error codes */
typedef enum {

    PAL_Ok = 0,                                 /*!< PAL OK  */

    // GENERAL ERROR                            
    PAL_Failed          = 0xFFFFFFFF,           /*!< PAL General Failure */
    PAL_ErrBadParam = 0X80000001,               /*!< PAL Bad Parameter */
    PAL_ErrNoMem,                               /*!< PAL Not enough memory */
    PAL_ErrNoData,                              /*!< PAL Data is NULL */
    PAL_ErrNoInit,                              /*!< PAL Interface has not been initialized */
    PAL_ErrUnsupported,                         /*!< PAL Behavior not supported */        
    PAL_ErrRange,                               /*!< PAL parameter is out of range */
    PAL_ErrNotFound,                            /*!< PAL Result can't be found */
    PAL_ErrWrongFormat,                         /*!< PAL Wrong format */
    PAL_ErrOverflow,                            /*!< PAL Over flow */
    PAL_ErrAlreadyExist,                        /*!< PAL Already exist */
    PAL_ErrEOF,                                 /*!< PAL EOF Error */
    PAL_ErrNotReady,                            /*!< PAL Not ready */

    // GPS error
    PAL_ErrGpsGeneralFailure = 0x80010000,      /*!< PAL GPS General Error */
    PAL_ErrGpsTimeout,                          /*!< PAL GPS Timeout */
    PAL_ErrGpsInformationUnavailable,           /*!< PAL GPS Information not available */
    PAL_ErrGpsAccuracyUnavailable,              /*!< PAL GPS Accuracy information unavailable */
    PAL_ErrGpsPrivacyRefused,                   /*!< PAL GPS Privacy policy refused */
    PAL_ErrGpsServerError,                      /*!< PAL GPS Server error */
    PAL_ErrGpsRejected,                         /*!< PAL GPS Rejected */
    PAL_ErrGpsNoResources,                      /*!< PAL GPS Not enough resources */
    PAL_ErrGpsBusy,                             /*!< PAL GPS module busy */
    PAL_ErrGpsInvalidState,                     /*!< PAL GPS Invalid state */
    PAL_ErrGpsInvalidCriteria,                  /*!< PAL GPS Invalid criteria */
    PAL_ErrGpsNoCallback,                       /*!< PAL GPS No GPS callback */
    PAL_ErrGpsHasCallback,                      /*!< PAL GPS Has GPS callback */
    PAL_ErrGpsFailedServiceAuthorization,       /*!< PAL GPS Failed Service Authorization */
    PAL_ErrGpsPendingServiceAuthorization,      /*!< PAL GPS Pending Service Authorization */
    PAL_ErrGpsNotProvisioned,                   /*!< PAL GPS Not Provisioned */
    PAL_ErrGpsLocationDisabled,                 /*!< PAL GPS Location Disabled */
    PAL_ErrGpsTimeoutStartNavigation,			/*!< PAL GPS Timeout when start navigation*/

    // NETWORK error
    PAL_ErrNetGeneralFailure = 0x80020000,      /*!< PAL Network General Error */
    PAL_ErrNetUninitialized,                    /*!< PAL Network Not initialized */
    PAL_ErrNetTimeout,                          /*!< PAL Network Timeout */
    PAL_ErrNetDnsMalformedHostName,             /*!< PAL Network DNS Malformed host name */
    PAL_ErrNetDnsUnknownHostName,               /*!< PAL Network DNS Unknown host name */
    PAL_ErrNetNoService,                        /*!< PAL Network No service */
    PAL_ErrNetInProgress,                       /*!< PAL Network Operation is in progress */

    // HTTP error
    PAL_ErrHttpNoContent,                       /*!< HTTP 204 - The server successfully processed the request,
                                                     but is not returning any content. */
    PAL_ErrHttpNotModified,                     /*!< HTTP 304 - Indicates the resource has not been modified
                                                     since last requested. */
    PAL_ErrHttpBadRequest,                      /*!< HTTP 400 - The request cannot be fulfilled due to bad syntax. */
    PAL_ErrHttpResourceNotFound,                /*!< HTTP 404 - The requested resource could not be found but may
                                                     be available again in the future. */
    PAL_ErrHttpProxyAuthRequired,                /*!< HTTP 407 - The client must first authenticate itself with the
                                                     proxy. See the function PAL_NetSetProxyCredentials. */
    PAL_ErrHttpInternalServerError,             /*!< HTTP 500 - A generic server error message, given when no more
                                                     specific message is suitable. */
    PAL_ErrHttpBadGateway,                      /*!< HTTP 502 - The server was acting as a gateway or proxy and
                                                     received an invalid response from the upstream server. */

    // AUDIO error
    PAL_ErrAudioBusy = 0x80030001,              /*!< PAL Audio Busy Error */
                PAL_ErrAudioBadData,                        /*!< PAL Audio data is bad */
    PAL_ErrAudioPrepError,                      /*!< PAL Audio data preperation error */
    PAL_ErrAudioDeviceOpen,                     /*!< PAL Audio cannot open device */
    PAL_ErrAudioGeneral,                        /*!< PAL Audio general error */

    // FILE error
    PAL_ErrFileFailed = 0x8004001,              /*!< PAL File Failed error */
    PAL_ErrFileNotExist,                        /*!< PAL File Not exist error */
    PAL_ErrFileExist,                           /*!< PAL File Exist error */
    PAL_ErrFileNoPermission,                    /*!< PAL File No permission error */
    PAL_ErrFileNotOpen,                         /*!< PAL File Not open error */

    // LOG error
    PAL_ErrLogFailed = 0x8005001,               /*!< PAL Log Failed error */
    PAL_ErrLogSeverity,                         /*!< PAL Log severity set too high to log this event */

    // Radio error
    PAL_ErrRadioLibraryNotLoad = 0x8006001,     /*!< PAL Radio library cannot be loaded */
    PAL_ErrRadioNoInit,                         /*!< PAL Radio initialization error */
    PAL_ErrRadioBssidScanFailed,                /*!< PAL Radio BSSID scan failed */
    PAL_ErrRadioWlanNotFound,                   /*!< PAL Radio WLAN Access Points not found */

    // Task queue error
    PAL_ErrQueueNotEmpty = 0x8007001,           /*!< PAL Queue is not empty */
    PAL_ErrQueueCannotStartNewThread,           /*!< PAL Queue cannot start new thread */
    PAL_ErrQueueNotFound,                       /*!< PAL Queue not found */


} PAL_Error;

#endif

/*! @} */

