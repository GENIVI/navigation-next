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

    @file     palradio.h
    @date     4/27/2010
    @defgroup PAL_RADIO PAL Radio Functions

    Platform-independent radio API.

    The following functions provide platform-independent radio support.
*/
/*
    (C) Copyright 2010 by TeleCommunication Systems, Inc.
                                                                  
    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PALRADIO_H
#define	PALRADIO_H

#include "pal.h"

/*! External reference for a PAL radio context.

    This typedef provides an opaque reference to a created radio context object.
*/
typedef struct PAL_RadioContext PAL_RadioContext;

/*! Type of cellular network currently in use.

    Those are simplified, e.g. UMTS network is recognized as GSM.
*/
typedef enum 
{
    PRSM_NoService,  /* No service available */
    PRSM_CDMA,       /* Also applicable for 1XEVDO, etc. */
    PRSM_GSM         /* Also applicable for UMTS, etc.   */
} PAL_RadioSystemMode;

/*! The required PAL Radio service type. Provided at the creation of RadioContext.

    If client wants cellular network information, type should be Telephony.
    If client wants Wifi network information, type should be Wifi.
    Type 'Both' is also available.
*/
typedef enum 
{
    PRST_Telephony,  /* Use TAPI */
    PRST_Wifi,       /* Use Wifi API */
    PRST_All         /* Use all APIs */
} PAL_RadioServiceType;

/*! Parameters that could be retrieved for GSM network. */
typedef struct
{
    uint32 mobileCountryCode;
    uint32 mobileNetworkCode;
    uint32 locationAreaCode;
    uint32 cellId;
    int16 signalStrength;
} PAL_RadioGSMInfo;

/*! Parameters that could be retrieved for CDMA network. */
typedef struct
{
    uint32 systemId;
    uint32 networkId;
    uint32 cellId;
    int16 signalStrength;
} PAL_RadioCDMAInfo;

/*! Parameters that could be retrieved for WiFi network. */
typedef struct
{
    const unsigned char* macAddress;  /* MAC address of AP in binary form - 6 bytes */
    int16 signalStrength;             /* Received signal strength in dBm */
} PAL_RadioWifiInfo;


/*! Complete set of parameters that could be retrieved for WiFi network. */
typedef struct
{
    const unsigned char* macAddress;  /* MAC address of AP in binary form - 6 bytes */
    const unsigned char* ssid;        /* Service set identifier, null-terminated string */
    uint32 frequency;                 /* Frequency in kHz */
    int16 signalStrength;             /* Received signal strength in dBm */
    nb_boolean isPrivacy;             /* WEP/WPA/WPA2 encryption requirement */
} PAL_RadioCompleteWifiInfo;

/*! Callback with cellular network system mode value.

    This function is provided by client, and then called back from RadioContext, as the result of request.
*/
typedef void (*PAL_RadioSystemModeCallback)(
    PAL_Error status,                   /*!< Status of the request */
    PAL_RadioSystemMode systemMode,     /*!< Retrieved cellular system mode */
    void* userData                      /*!< User data passed in PAL_RadioGetNetworkType */ 
    );

/*! Callback with parameters for GSM network.

    This function is provided by client, and then called back from RadioContext, as the result of request.
*/
typedef void (*PAL_RadioGSMNetworkInfoCallback)(
    PAL_Error status,            /*!< Status of the request */
    PAL_RadioGSMInfo* gsmInfo,   /*!< Retrieved GSM information. Pointer cannot be used outside this callback. User should copy data. */
    void* userData               /*!< User data passed in PAL_GetGSMNetworkInfo */ 
    );

/*! Callback with parameters for CDMA network.

    This function is provided by client, and then called back from RadioContext, as the result of request.
*/
typedef void (*PAL_RadioCDMANetworkInfoCallback)(
    PAL_Error status,              /*!< Status of the request */
    PAL_RadioCDMAInfo* cdmaInfo,   /*!< Retrieved CDMA information. Pointer cannot be used outside this callback. User should copy data. */
    void* userData                 /*!< User data passed in PAL_GetCDMANetworkInfo */ 
    );

/*! Callback with parameters for WiFi network.

    This function is provided by client, and then called back from RadioContext, as the result of request.
*/
typedef void (*PAL_RadioWifiNetworkInfoCallback)(
    PAL_Error status,              /*!< Status of the request */
    PAL_RadioWifiInfo* wifiInfo,   /*!< Retrieved Wifi information. Pointer cannot be used outside this callback. User should copy data. */
    void* userData                 /*!< User data passed in PAL_GetWifiNetworkInfo */ 
    );

/*! Callback with parameters for all WiFi networks.

    This function is provided by client, and then called back from RadioContext, as the result of request.
*/
typedef void (*PAL_RadioAllWifiNetworksInfoCallback)(
    PAL_Error status,                      /*!< Status of the request */
    PAL_RadioCompleteWifiInfo** wifisInfo, /*!< Array of retrieved Wifis information. Pointer cannot be used outside this callback. User should copy data. */
    unsigned long wifisInfoCount,          /*!< Length of array of retrieved Wifis information. */
    void* userData                         /*!< User data passed in PAL_GetWifiNetworkInfo */ 
    );

/*! This function creates RadioContext object. 

    This function should be called before any other PAL_Radio function.

    @return PAL error code
    @see PAL_RadioServiceType
*/
PAL_DEC PAL_Error PAL_RadioContextCreate(
    PAL_Instance* pal,                  /*!< Previously created PAL object */
    PAL_RadioServiceType type,          /*!< Required PAL Radio service type */
    PAL_RadioContext** radioContext     /*!< RadioContext object, created as the result of this function */
    );

/*! This function destroys RadioContext object. 

    @return PAL error code
    @see PAL_RadioContextCreate
*/
PAL_DEC PAL_Error PAL_RadioContextDestroy(
    PAL_RadioContext* radioContext);

/*! This function provides current cellular network system mode, via user callback.

    @return PAL error code
    @see PAL_RadioSystemModeInfoCallback
*/
PAL_DEC PAL_Error PAL_RadioGetSystemMode(
    PAL_RadioContext* radioContext,
    PAL_RadioSystemModeCallback callback,
    void* userData);

/*! This function provides GSM cellular network information, via user callback.

    @return PAL error code
    @see PAL_RadioGSMNetworkInfoCallback
*/
PAL_DEC PAL_Error PAL_RadioGetGSMNetworkInfo(
    PAL_RadioContext* radioContext,
    PAL_RadioGSMNetworkInfoCallback callback,
    void* userData);

/*! This function provides CDMA cellular network information, via user callback.

    @return PAL error code
    @see PAL_RadioCDMANetworkInfoCallback
*/
PAL_DEC PAL_Error PAL_RadioGetCDMANetworkInfo(
    PAL_RadioContext* radioContext,
    PAL_RadioCDMANetworkInfoCallback callback,
    void* userData);

/*! This function provides WiFi network information, via user callback.

    @return PAL error code
    @see PAL_RadioWifiNetworkInfoCallback
*/
PAL_DEC PAL_Error PAL_RadioGetWifiNetworkInfo(
    PAL_RadioContext* radioContext,
    PAL_RadioWifiNetworkInfoCallback callback,
    void* userData);

/*! This function provides all WiFi networks information, via user callback.

    @return PAL error code
    @see PAL_RadioAllWifiNetworksInfoCallback
*/
PAL_DEC PAL_Error PAL_RadioGetAllWifiNetworksInfo(
    PAL_RadioContext* radioContext,
    PAL_RadioAllWifiNetworksInfoCallback callback,
    void* userData);

/*! This function allows to cancel last PAL_RadioWifiNetworkInfoCallback.

    @return PAL error code
*/
PAL_DEC PAL_Error PAL_RadioCancelWifiNetworkInfo(
    PAL_RadioContext* radioContext);

/*! This function allows to cancel PAL_RadioAllWifiNetworksInfoCallback.

    @return PAL error code
*/
PAL_DEC PAL_Error PAL_RadioCancelAllWifiNetworksInfo(
    PAL_RadioContext* radioContext,
    PAL_RadioAllWifiNetworksInfoCallback callback);

#endif

/*! @} */
