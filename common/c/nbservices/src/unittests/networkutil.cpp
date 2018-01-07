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

@file     networkutil.h
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

/*! @{ */

#include "nbcontextaccess.h"
#include "nbnetworkconfiguration.h"
#include "protected/nbpersistentdataprotected.h"

extern "C"
{
#include "CUnit.h"
#include "networkutil.h"
#include "pal.h"
#include "palconfig.h"
#include "palfile.h"
#include "palnet.h"
#include "palclock.h"
#include "platformutil.h"
}

#include "tpsdebug.h"
#include "TpsAtlasbookProtocol.h"
#include "TpsElement.h"
#include "StringUtility.h"

#include "NBProtocolIdenParameters.h"
#include "NBProtocolIdenParametersSerializer.h"

#define CCC_NEW new (std::nothrow)

using namespace protocol;
using namespace nbmap;
using namespace nbcommon;

// Constants .....................................................................................

// // QA1 ---------------------------------------------------------------------------------------
// static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "s2f2pYdiNUJznjyuNKY881oVR/0kbE9okRESFE40";
// static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "s2f2pYdiNUJznjyuNKY881oVR/0kbE9okRESFE40";

// tj-lnx3 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "xdrHqvAq21MMjdkwHTvmXzT1PuxhXfgo5+uW+LJ9";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "xdrHqvAq21MMjdkwHTvmXzT1PuxhXfgo5+uW+LJ9";

// QA8 ---------------------------------------------------------------------------------------------
// static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "Hh1krqoAHFClU1h+RfVkOVqrjktxD/PvRic+0hS+";
// static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "Hh1krqoAHFClU1h+RfVkOVqrjktxD/PvRic+0hS+";

////CS3 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "mkX7zxVMuOKZpx+7xl0dEE1la0phoPuK6D1RkSOl";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "mkX7zxVMuOKZpx+7xl0dEE1la0phoPuK6D1RkSOl";

//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "+qWTzdsQzuvO9Ci9Ipc3/AZsmhbZCPyEfXNNqnwY";   // abnav_ccc on dev11
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "+qWTzdsQzuvO9Ci9Ipc3/AZsmhbZCPyEfXNNqnwY";   // abnav_ccc on dev11

//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "fn1uB1piJW1J9we1QVv+ycEMkf5l2MWdqTTm51Lt";   // abnav_ccc on qa11
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "fn1uB1piJW1J9we1QVv+ycEMkf5l2MWdqTTm51Lt";   // abnav_ccc on qa11

//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "bGBQCP5Vdu6FV9I6JEh9oJD3+sKwyoXloLvZ7zny";   // abnav_ccc on nbi1
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "bGBQCP5Vdu6FV9I6JEh9oJD3+sKwyoXloLvZ7zny";   // abnav_ccc on nbi1

// DEV14 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "7ekw6R/8xuJqPykKsyT15KbwxPe2ExNc5sSqHNXk";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "7ekw6R/8xuJqPykKsyT15KbwxPe2ExNc5sSqHNXk";

// QA4 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "d3f+tXtg7HrKj/kaNM9XEtRO8dEKlrErrzQaigUC";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "d3f+tXtg7HrKj/kaNM9XEtRO8dEKlrErrzQaigUC";

// DEV3 -------------------------------------------------------------------------------------------
static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "X+Jj2r6cakEell+rZqndwaBfZFudVkVDvUGACX7M";
static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "X+Jj2r6cakEell+rZqndwaBfZFudVkVDvUGACX7M";

static const char TPSLIB_FILE[]   = "tesla.tpl";
static const char GUID_FILENAME[] = "guid.bin";
static const char PERSISTENT_DATA_FILENAME[] = "ccc.bin";

#define DEFAULT_PORT_HTTP     80
#define DEFAULT_PORT_HTTPS    443
#define DEFAULT_PORT_TCP      8128
#define DEFAULT_PORT_TCP_TLS  8129

// Default count of connections for the generic HTTP download manager
#define DEFAULT_HTTP_DOWNLOAD_CONNECTION_COUNT 6

// Local Functions ...............................................................................

/*! Get file data

! Important: Caller has to call free() once the data is no longer needed !

@return binary data
*/
PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength)
{
    if (dataBuffer == NULL || dataBufferLength == NULL)
    {
        return PAL_ErrBadParam;
    }

    return PAL_FileLoadFile(pal, filename, (unsigned char**)dataBuffer, (uint32*)dataBufferLength);
}

const char* GetToken()
{
    return DEFAULT_NETWORK_TOKEN_TCP;
}

const char* GetDomain()
{
    char* domain = 0;

    domain = "navbuilder.nimlbs.net";

    return domain;
}

NB_NetworkProtocol GetNetProtocol()
{
    return NB_NetworkProtocol_TCP;
}

uint16 GetPort()
{
    return DEFAULT_PORT_TCP;
}

const char* GetTpslibFile()
{
    return TPSLIB_FILE;
}

const char* GetHostname()
{
    return 0;
}

NB_Error SetTargetMappings(NB_Context* context)
{
    NB_Error err = NE_OK;

    NB_TargetMapping targets[] = {
        { "geocode",            "geocode,global" },
        { "map",                "map,global" },
        { "nav",                "nav,usa" },
        { "proxpoi",            "proxpoi,global" },
        { "reverse-geocode",    "reverse-geocode,global" },
        { "traffic-notify",     "traffic-notify,global" },
        { "vector-tile",        "vector-tile,global" }
    };
    err = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));

    return err;
}

NB_NetworkConfiguration* GetNetworkConfiguration(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, const char* tpsBuffer, size_t tpsBufferLength, const char* guidBuffer, size_t guidBufferLength)
{
    AB_NetworkOptionsDetailed options = { 0 };
    NB_NetworkConfiguration* configuration = NULL;
    char deviceName[50] = { 0 };

    GetDeviceName(pal, deviceName, sizeof(deviceName) / sizeof(deviceName[0]));

    options.adsAppId = "N/A";
    options.classId = 0;
    options.credential = credential;
    options.device = "cricketnav_android";//deviceName;
    options.domain = GetDomain();
    options.firmwareVersion = "N/A";
    options.hostname = hostname;
    options.language = "en-US";
    options.mdn = GetMobileDirectoryNumber(pal);
    options.min = GetMobileInformationNumber(pal);
    options.platformId = 0;
    options.port = port;
    options.priceType = 0;
    options.protocol = protocol;
    options.tpslibData = tpsBuffer;
    options.tpslibLength = tpsBufferLength;
    options.clientGuidData = guidBuffer;
    options.clientGuidLength = guidBufferLength;
    options.isEmulator = RunningInEmulator();

    (void)AB_NetworkConfigurationCreate(&options, &configuration);

    return configuration;
}

/*! Get persistent data object.

Create a persistent data object and initialize it with the serialized content of ccc.bin
from previous test runs.  Set data change callback to SavePersistentDataToFileCallback.

@return pointer to created and initialized persistent data change objecte
*/
NB_PersistentData* GetPersistentData(PAL_Instance* pal)
{
    NB_PersistentData* persistentData = 0;

    uint8* serializedData = 0;
    size_t serializedDataSize = 0;

    if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
    {
        CU_ASSERT_EQUAL(GetFileData(pal, PERSISTENT_DATA_FILENAME, (char**)&serializedData, &serializedDataSize), PAL_Ok);
    }

    CU_ASSERT_EQUAL(NB_PersistentDataCreate(serializedData, serializedDataSize, 0, &persistentData), NE_OK);

    if (serializedData)
    {
        nsl_free(serializedData);
    }

    return persistentData;
}

void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    if (event == NB_NetworkNotifyEvent_ClientGuidAssigned)
    {
        NB_NetworkNotifyEventClientGuid* guid = (NB_NetworkNotifyEventClientGuid *)data;
        PAL_Instance* pal = (PAL_Instance*)userData;
        PAL_File* file = 0;
        PAL_Error err = PAL_Ok;

        err = PAL_FileOpen(pal, GUID_FILENAME, PFM_Create, &file);
        if (!err)
        {
            uint32 written = 0;
            err = PAL_FileWrite(file, guid->guidData, guid->guidDataLength, &written);
            PAL_FileClose(file);
        }
    }
}

nb_boolean CreateContext(PAL_Instance* pal,
                         const char* credential,
                         const char* hostname,
                         NB_NetworkProtocol protocol,
                         uint16 port,
                         uint32 httpDownloadConnectionCount,
                         NB_Context** context)
{
    NB_NetworkConfiguration* config = 0;
    NB_PersistentData* persistentData = 0;

    NB_NetworkNotifyEventCallback callback = { NetworkNotifyEventCallback, 0 };

    PAL_Error palErr = PAL_Ok;
    char* tpsData = 0;
    size_t tpsDataSize = 0;
    char* guidData = 0;
    size_t guidDataSize = 0;

    if (!pal || !credential || nsl_strlen(credential) == 0)
    {
        return FALSE;
    }

    palErr = GetFileData(pal, GetTpslibFile(), &tpsData, &tpsDataSize);
    CU_ASSERT_EQUAL_FATAL(palErr, PAL_Ok);

    if (PAL_FileExists(pal, GUID_FILENAME) == PAL_Ok)
    {
        palErr = GetFileData(pal, GUID_FILENAME, &guidData, &guidDataSize);
        CU_ASSERT_EQUAL_FATAL(palErr, PAL_Ok);
    }

    config = GetNetworkConfiguration(pal, credential, hostname, protocol, port, tpsData, tpsDataSize, guidData, guidDataSize);
    CU_ASSERT_PTR_NOT_NULL(config);

    persistentData = GetPersistentData(pal);
    CU_ASSERT_PTR_NOT_NULL(persistentData);

    if (config && persistentData)
    {
        callback.callbackData = pal;
        (void)NB_ContextCreate(pal, config, &callback, 0, 0, persistentData, context);
        CU_ASSERT_PTR_NOT_NULL(*context);

        if (*context)
        {
            NB_Error error = NE_OK;

            error = SetTargetMappings(*context);
            CU_ASSERT_EQUAL(error, NE_OK);

            error = NB_ContextInitializeGenericHttpDownloadManager(*context, httpDownloadConnectionCount);
            CU_ASSERT_EQUAL(error, NE_OK);
        }

        AB_NetworkConfigurationDestroy(config);
    }

    nsl_free(guidData);
    nsl_free(tpsData);

    return (*context) ? TRUE : FALSE;
}

/* See header file for description */
uint8
    CreatePalAndContext(PAL_Instance** pal,
    NB_Context** context)
{
    return CreatePalAndContextWithHttpConnectionCount(DEFAULT_HTTP_DOWNLOAD_CONNECTION_COUNT,
        pal,
        context);
}

/* See header file for description */
uint8
    CreatePalAndContextWithHttpConnectionCount(uint32 httpConnectionCount,
    PAL_Instance** pal,
    NB_Context** context)
{
    *pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(*pal);

    if (*pal)
    {
        nb_boolean result = FALSE;

        PAL_NetInitialize(*pal);

        result = CreateContext(*pal, GetToken(), GetHostname(), GetNetProtocol(), GetPort(), httpConnectionCount, context);
        CU_ASSERT_EQUAL(result, TRUE);
        CU_ASSERT_PTR_NOT_NULL(*context);
    }

    if ((*pal) && !(*context))
    {
        PAL_NetShutdown(*pal);
        PAL_Destroy(*pal);
        *pal = 0;
    }

    return (*pal) ? 1 : 0;
}

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
// copied over from abservices to prevent forward reference for the nbservices unit test

#define VALIDATE_CREDENTIAL_LENGTH          40      /*! Length of a credential */
#define VALIDATE_MINIMUM_LANGUAGE_LENGTH    2       /*! Minimum length of a language specifier */

static void
    CreateQualifiedHostname(const char* credential, const char* domain, char* hostname)
{
    byte sha[20];

    get_sha_hash(credential, nsl_strlen(credential), sha);
    hexlify(20, (const char*)sha, hostname);
    nsl_strcat(hostname, ".");
    nsl_strcat(hostname, domain);
}

/*
static tpselt
AttachNewElement(tpselt te, const char* name)
{
tpselt ce = te_new(name);
if (ce)
{
if (!te_attach(te, ce))
{
te_dealloc(ce);
ce = 0;
}
}

return ce;
}*/

static tpselt
    CreateIdenFromDetailedOptions(AB_NetworkOptionsDetailed* options)
{
    shared_ptr<BinaryData> bd(new BinaryData);
    shared_ptr<IdenParameters>  idenInformation(CCC_NEW IdenParameters());
    if (!idenInformation)
    {
        return NULL;
    }
    idenInformation->SetEmulator(true);
    idenInformation->SetWantIdenReceipt(true);
    idenInformation->SetWantClientGuidAssigned(true);

    shared_ptr<FormattedIdenParameters> formattedIden(CCC_NEW FormattedIdenParameters());

    shared_ptr<UserInfo> userInfo(CCC_NEW UserInfo());
    if (!userInfo) {
        return NULL;
    }

    userInfo->SetMdn(StringUtility::NumberToStringPtr(options->mdn));
    bd->SetData((byte*)options->clientGuidData, nsl_strlen(options->clientGuidData));
    userInfo->SetClientGuid(bd);
    formattedIden->SetUserInfo(userInfo);

    shared_ptr<DeviceInfo> deviceInfo(CCC_NEW DeviceInfo());
    if (!deviceInfo) {
        return NULL;
    }

    // udid can be optional, check if it exists first
    //if (options->deviceUniqueIdentifier != NULL && nsl_strlen(options->deviceUniqueIdentifier) > 0)
    //{
    //    deviceInfo->SetDeviceUniqueId(CHAR_PTR_TO_UTF_STRING_PTR(options->deviceUniqueIdentifier));
    //}

    deviceInfo->SetDeviceSerialNumber(CHAR_PTR_TO_UTF_STRING_PTR(""));
    formattedIden->SetDeviceInfo(deviceInfo);

    shared_ptr<PlatformInfo> platformInfo(CCC_NEW PlatformInfo());
    if (!platformInfo) {
        return NULL;
    }
    platformInfo->SetVersion(CHAR_PTR_TO_UTF_STRING_PTR(options->firmwareVersion));
    platformInfo->SetManufacturer(CHAR_PTR_TO_UTF_STRING_PTR(options->device));
    platformInfo->SetModel(CHAR_PTR_TO_UTF_STRING_PTR(""));
    formattedIden->SetPlatformInfo(platformInfo);

    shared_ptr<ProductInfo> productInfo(CCC_NEW ProductInfo());
    if (!productInfo) {
        return NULL;
    }
    productInfo->SetCredential(CHAR_PTR_TO_UTF_STRING_PTR(options->credential));
    formattedIden->SetProductInfo(productInfo);

    shared_ptr<LocaleInfo> localeInfo(CCC_NEW LocaleInfo());
    if (!localeInfo) {
        return NULL;
    }
    //localeInfo->SetCountryCode(CHAR_PTR_TO_UTF_STRING_PTR(options->countryCode));
    localeInfo->SetLanguage(CHAR_PTR_TO_UTF_STRING_PTR(options->language));
    formattedIden->SetLocaleInfo(localeInfo);

    shared_ptr<CarrierInfo> carrierInfo(CCC_NEW CarrierInfo());
    if (!carrierInfo) {
        return NULL;
    }
    if (options->carrier)
    {
        carrierInfo->SetCarrier(CHAR_PTR_TO_UTF_STRING_PTR(options->carrier));
    }

    // carrierInfo->SetMcc(options->mcc);
    // carrierInfo->SetMnc(options->mnc);
    formattedIden->SetCarrierInfo(carrierInfo);

    shared_ptr<SessionInfo> sessionInfo(CCC_NEW SessionInfo());
    if (!sessionInfo) {
        return NULL;
    }
    sessionInfo->SetClientSession(PAL_ClockGetGPSTime());
    sessionInfo->SetActiveSession(shared_ptr<BinaryData>(new BinaryData));
    formattedIden->SetSessionInfo(sessionInfo);

    shared_ptr<PushMessageInfo> pushMessageInfo(CCC_NEW PushMessageInfo());
    if (!pushMessageInfo) {
        return NULL;
    }
    //    pushMessageInfo->SetPushMessageGuid(CHAR_PTR_TO_UTF_STRING_PTR(options->pushMessageGuid));
    formattedIden->SetPushMessageInfo(pushMessageInfo);

    idenInformation->SetFormattedIdenParameters(formattedIden);

    TpsElementPtr tpsElement = IdenParametersSerializer::serialize(idenInformation);
    if (!tpsElement)
    {
        return NULL;
    }

    dumpelt(tpsElement->GetTPSElement(), 10);

    //tpselt ce = te_getchild(tpsElement->GetTPSElement(), "formatted-iden-parameters");

    //// Get the client guid from iden. // formatted-iden-parameters
    //if ( ce)
    //{
    //    return NULL;
    //}

    return tpsElement->GetTPSElement();
}

/*! Validate detailed options
@param options The detailed options to validate
@return true if options appear valid; false, otherwise
*/
static nb_boolean
    ValidateDetailedOptions(AB_NetworkOptionsDetailed* options)
{
    if (!options->credential || nsl_strlen(options->credential) != VALIDATE_CREDENTIAL_LENGTH)
    {
        return FALSE;
    }

    if (!options->domain || nsl_strlen(options->domain) == 0)
    {
        return FALSE;
    }

    if (!options->tpslibData || options->tpslibLength == 0)
    {
        return FALSE;
    }

    if (!options->language || nsl_strlen(options->language) < VALIDATE_MINIMUM_LANGUAGE_LENGTH)
    {
        return FALSE;
    }

    if (options->protocol != NB_NetworkProtocol_HTTP &&
        options->protocol != NB_NetworkProtocol_HTTPS &&
        options->protocol != NB_NetworkProtocol_TCP)
    {
        return FALSE;
    }

    if ((options->clientGuidData && options->clientGuidLength == 0) ||
        (!options->clientGuidData && options->clientGuidLength != 0))
    {
        return FALSE;
    }

    return TRUE;
}

NB_Error
    AB_NetworkConfigurationCreate(AB_NetworkOptionsDetailed* options, NB_NetworkConfiguration** configuration)
{
    NB_NetworkConfiguration* netConfig = NULL;

    if (!options || !configuration)
    {
        return NE_INVAL;
    }
    *configuration = NULL;

    if (!ValidateDetailedOptions(options))
    {
        return NE_INVAL;
    }

    netConfig = (NB_NetworkConfiguration*)nsl_malloc(sizeof(*netConfig));
    if (!netConfig)
    {
        return NE_NOMEM;
    }

    nsl_memset(netConfig, 0, sizeof(*netConfig));

    netConfig->tpslibData = (char *) nsl_malloc(options->tpslibLength);
    if (!netConfig->tpslibData)
    {
        nsl_free(netConfig);
        return NE_NOMEM;
    }

    netConfig->iden = CreateIdenFromDetailedOptions(options);
    if (options->hostname && nsl_strlen(options->hostname))
    {
        nsl_strlcpy(netConfig->hostname, options->hostname, sizeof(netConfig->hostname));
        nsl_strlcat(netConfig->hostname, ".", sizeof(netConfig->hostname));
        nsl_strlcat(netConfig->hostname, options->domain, sizeof(netConfig->hostname));
    }
    else
    {
        CreateQualifiedHostname(options->credential, options->domain, netConfig->hostname);
    }
    netConfig->port = options->port;
    netConfig->protocol = options->protocol;
    nsl_memcpy(netConfig->tpslibData, options->tpslibData, options->tpslibLength);
    netConfig->tpslibLength = options->tpslibLength;
    *configuration = netConfig;

    return NE_OK;
}

NB_Error
    AB_NetworkConfigurationDestroy(NB_NetworkConfiguration* configuration)
{
    if (!configuration)
    {
        return NE_INVAL;
    }

    if (configuration->iden)
    {
        te_dealloc(configuration->iden);
    }

    if (configuration->tpslibData)
    {
        nsl_free(configuration->tpslibData);
    }
    nsl_free(configuration);

    return NE_OK;
}

/*! @} */