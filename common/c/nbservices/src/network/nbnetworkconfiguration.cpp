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

@file     nbnetworkconfiguration.cpp
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

extern "C"
{
#include "nbnetworkoptions.h"
#include "cslutil.h"
#include "nbnetworkconfiguration.h"
#include "palclock.h"
#include "palstdlib.h"
#include "palnet.h"
}

#include "tpsdebug.h"
#include "TpsElement.h"
#include "StringUtility.h"

#include "NBProtocolIdenParameters.h"
#include "NBProtocolIdenParametersSerializer.h"
#include "nbmacros.h"

using namespace protocol;
using namespace nbcommon;

#define VALIDATE_CREDENTIAL_LENGTH          40      /*! Length of a credential */
#define VALIDATE_MINIMUM_LANGUAGE_LENGTH    2       /*! Minimum length of a language specifier */

// Macros to it easier to write iden related codes...

// Declares variable X which is a shared_ptr of type T
#define CHECK_NEW_PTR(T, X)                        \
    shared_ptr<T> X(CCC_NEW T);                    \
    if (!X)                                        \
    {                                              \
        return NULL;                               \
    }

// Check if P is NULL, abort execution if so.
#define ASSERT_NOT_NULL(P)                          \
    if (!(P))                                       \
    {                                               \
        abort();                                    \
    }

// Convert char* to shared_ptr<string>
#define STRING_PTR(X) CHAR_PTR_TO_UTF_STRING_PTR(X)
#define STRING_PTR2(X, L) shared_ptr<string>(CCC_NEW string(X, L))

// Set attribute of X by calling proper member function of X.
#define SET_ATTR(X, Y, Z)      (X)->Set##Y(Z)
#define SET_ELT(X, Y, Z)       SET_ATTR(X, Y, Z)

// Set non-optional string as attribute.
#define SET_STRING(X, Y, Z)                         \
    ASSERT_NOT_NULL(X)                              \
    (X)->Set##Y(STRING_PTR(Z))

// Set optional string as attribute.
#define SET_STRING_OPTIONAL(X, Y, Z)                \
    if (!nsl_strempty(Z))                           \
    {                                               \
        (X)->Set##Y(STRING_PTR(Z));                 \
    }

// Set string attribute, if string is empty, set it to default.
#define SET_STRING_DEFAULT(X, Y, Z, D)              \
    (X)->Set##Y(STRING_PTR(!nsl_strempty(Z) ? Z : D))

// pack binary data into string..
#define SET_STRING_DATA(X, Y, D, L)                 \
        (X)->Set##Y(STRING_PTR2(D,L))

// Set binary data.
#define SET_BINARY_DATA(X, Y, D, L)                 \
    if (!nsl_strempty(D))                           \
    {                                               \
        shared_ptr<BinaryData> bd(new BinaryData);  \
        bd->SetData((byte*)(D), L);                 \
        (X)->Set##Y(bd);                            \
    }

static tpselt CreateIdenFromDetailedOptions(NB_NetworkOptionsDetailed* options);
static nb_boolean ValidateDetailedOptions(NB_NetworkOptionsDetailed* options);
static void CreateQualifiedHostname(const char* credential, const char* domain, char* hostname);
static nb_boolean IsIpAddress(const char* ipAddress);

NB_DEF NB_Error
NB_NetworkConfigurationCreate(NB_NetworkOptionsDetailed* options, NB_NetworkConfiguration** configuration)
{
    if (!options || !configuration)
    {
        return NE_INVAL;
    }
    *configuration = NULL;

    if (!ValidateDetailedOptions(options))
    {
        return NE_INVAL;
    }

    NB_NetworkConfiguration *netConfig = (NB_NetworkConfiguration *)nsl_malloc(sizeof(*netConfig));
    if (!netConfig)
    {
        return NE_NOMEM;
    }

    nsl_memset(netConfig, 0, sizeof(*netConfig));

    netConfig->tpslibData = (char *)nsl_malloc(options->tpslibLength);
    if (!netConfig->tpslibData)
    {
        nsl_free(netConfig);
        return NE_NOMEM;
    }

    netConfig->iden = CreateIdenFromDetailedOptions(options);
    if (options->hostname && nsl_strlen(options->hostname))
    {
        nsl_strlcpy(netConfig->hostname, options->hostname, sizeof(netConfig->hostname));
        if (!IsIpAddress(options->hostname))
        {
            nsl_strlcat(netConfig->hostname, ".", sizeof(netConfig->hostname));
            nsl_strlcat(netConfig->hostname, options->domain, sizeof(netConfig->hostname));
        }
    }
    else
    {
        CreateQualifiedHostname(options->credential, options->domain, netConfig->hostname);
    }
    netConfig->port = options->port;
    netConfig->protocol = options->protocol;
    nsl_memcpy(netConfig->tpslibData, options->tpslibData, options->tpslibLength);
    netConfig->tpslibLength = options->tpslibLength;
    COPY_TLSCONFIG(netConfig->tlsConfig, options->tlsConfig);
    netConfig->triageTimeoutSeconds = options->triageTimeoutSeconds;

    if (options->credential != NULL)
    {
        netConfig->credential = (char *)nsl_malloc(strlen(options->credential)+1);
        nsl_strcpy(netConfig->credential, options->credential);
    }

    *configuration = netConfig;

    return NE_OK;
}

NB_DEF NB_Error
NB_NetworkConfigurationDestroy(NB_NetworkConfiguration* configuration)
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

    if (configuration->credential)
    {
        nsl_free(configuration->credential);
    }

    nsl_free(configuration);

    return NE_OK;
}

static void
CreateQualifiedHostname(const char* credential, const char* domain, char* hostname)
{
    byte sha[20];

    get_sha_hash(credential, nsl_strlen(credential), sha);
    hexlify(20, (const char*)sha, hostname);
    nsl_strcat(hostname, ".");
    nsl_strcat(hostname, domain);
}

//@note: this is platform independent code, if some fields are not optional on some special
//       platform, then "options" of this platform should be well configured. Here we simply
//       create IdenParameters based on given options, without checking any
//       platform-specific requirements.

static tpselt
CreateIdenFromDetailedOptions(NB_NetworkOptionsDetailed* options)
{
    CHECK_NEW_PTR(IdenParameters, pIdenParameters);
    pIdenParameters->SetWantIdenReceipt(true);

    CHECK_NEW_PTR(FormattedIdenParameters, formattedIden);
    CHECK_NEW_PTR(UserInfo, userInfo);
    userInfo->SetMdn(StringUtility::NumberToStringPtr(options->mdn));

    if (!nsl_strempty(options->clientGuidData))
    {
        SET_BINARY_DATA(userInfo, ClientGuid, options->clientGuidData, options->clientGuidLength);
    }
    else
    {
        pIdenParameters->SetWantClientGuidAssigned(true);
    }
    formattedIden->SetUserInfo(userInfo);

    CHECK_NEW_PTR(DeviceInfo, deviceInfo);
    SET_STRING_OPTIONAL(deviceInfo, DeviceUniqueId, options->deviceUniqueIdentifier);
    SET_STRING_OPTIONAL(deviceInfo, DeviceSerialNumber, options->deviceSerialNumber);
    formattedIden->SetDeviceInfo(deviceInfo);

    CHECK_NEW_PTR(PlatformInfo, platformInfo);
    SET_STRING(platformInfo, Version, options->firmwareVersion);
    SET_STRING(platformInfo, Manufacturer, options->manufacturer);
    SET_STRING_DEFAULT(platformInfo, Model, options->model, "");
    formattedIden->SetPlatformInfo(platformInfo);

    CHECK_NEW_PTR(ProductInfo, productInfo);
    SET_STRING(productInfo, Credential, options->credential);
    formattedIden->SetProductInfo(productInfo);

    CHECK_NEW_PTR(LocaleInfo, localeInfo);
    SET_STRING_OPTIONAL(localeInfo, CountryCode, options->countryCode);
    SET_STRING(localeInfo, Language, options->language);
    formattedIden->SetLocaleInfo(localeInfo);

    CHECK_NEW_PTR(CarrierInfo, carrierInfo);
    SET_STRING_OPTIONAL(carrierInfo, Carrier, options->carrier);
    SET_ATTR(carrierInfo, Mcc, options->mcc);
    SET_ATTR(carrierInfo, Mnc, options->mnc);
    formattedIden->SetCarrierInfo(carrierInfo);

    CHECK_NEW_PTR(SessionInfo, sessionInfo);
    SET_ATTR(sessionInfo, ClientSession, PAL_ClockGetGPSTime());
    SET_ATTR(sessionInfo, ActiveSession, shared_ptr<BinaryData>(new BinaryData));
    SET_ELT(formattedIden, SessionInfo, sessionInfo);

    if (!nsl_strempty(options->pushMessageGuid))
    {
        CHECK_NEW_PTR(PushMessageInfo, pushMessageInfo);
        SET_STRING_DATA(pushMessageInfo, PushMessageGuid,
                        options->pushMessageGuid,
                        options->pushMessageGuidLength);
        formattedIden->SetPushMessageInfo(pushMessageInfo);
    }

    pIdenParameters->SetFormattedIdenParameters(formattedIden);
    TpsElementPtr tpsElement = IdenParametersSerializer::serialize(pIdenParameters);
    return tpsElement ? tpsElement->GetTPSElement() : NULL;
}

/*! Validate detailed options
@param options The detailed options to validate
@return true if options appear valid; false, otherwise
*/
nb_boolean ValidateDetailedOptions(NB_NetworkOptionsDetailed* options)
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
        options->protocol != NB_NetworkProtocol_TCP &&
        options->protocol != NB_NetworkProtocol_TCPTLS)
    {
        return FALSE;
    }

    if ((options->clientGuidData && options->clientGuidLength == 0) ||
        (!options->clientGuidData && options->clientGuidLength != 0) ||
        (options->pushMessageGuid && options->pushMessageGuidLength == 0) ||
        (!options->pushMessageGuid && options->pushMessageGuidLength != 0))
    {
        return FALSE;
    }

    return TRUE;
}

nb_boolean
IsIpAddress(const char* ipString)
{
    int octet = 0;
    int numOctet = 0;
    while(*ipString)
    {
        if(((int)*ipString - 48 <= 9) && ((int)*ipString - 48 >= 0))
        {
            octet *= 10;
            octet += (int)*ipString - 48;
        }
        else if (*ipString == '.' && octet >= 0 && octet <= 255)
        {
            numOctet++;
            octet = 0;
        }
        else
        {
            break;
        }
        ipString++;
        if(!(*ipString) && numOctet == 3 && octet >= 0 && octet <= 255)
        {
            return TRUE;
        }
        if(numOctet > 3)
        {
            break;
        }
    }
    return FALSE;
}

/*! @} */
