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

//
//  initccc.c
//  NavUIKitTest
//

#include "pal.h"
#include "paltaskqueue.h"
#include "paluitaskqueue.h"
#include "nbnetworkoptions.h"
#include "palfile.h"
#include "abqalog.h"
#include "nbcontextaccess.h"
#include "initccc.h"


static const byte VOICE_CACHE_OBFUSCATE_KEY[] = {
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};

/*! Obfuscate key for tile cache. */
static const byte RASTER_TILE_CACHE_OBFUSCATE_KEY[] =
{
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};

// TOKENS for System Tests (ABNAV_CCC) ==============================================================

// Current token spreadsheet at docs: //depot/scm/Tokens/Production/nbtokens.xls

// QA4 -------------------------------------------------------------------------------------------
static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "d3f+tXtg7HrKj/kaNM9XEtRO8dEKlrErrzQaigUC";
static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "d3f+tXtg7HrKj/kaNM9XEtRO8dEKlrErrzQaigUC";

// // DEV4 ------------------------------------------------------------------------------------------
// static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "+i88bW19I5thJdZYklM65s3bXk4cCckB/uaD4C2n";
// static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "+i88bW19I5thJdZYklM65s3bXk4cCckB/uaD4C2n";

//feature1------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]        = "B6hwvGHV3PY0k+gUlt+LCvPCBVYLlEbJlHaVq4p0";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]         = "B6hwvGHV3PY0k+gUlt+LCvPCBVYLlEbJlHaVq4p0";

// QA1 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "mFMANBDMXWNMGP6ISdz85fUuDHQIFWz2TJoxDMfw";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "mFMANBDMXWNMGP6ISdz85fUuDHQIFWz2TJoxDMfw";

// QA8 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "Hh1krqoAHFClU1h+RfVkOVqrjktxD/PvRic+0hS+";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "Hh1krqoAHFClU1h+RfVkOVqrjktxD/PvRic+0hS+";

// CS3 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "mkX7zxVMuOKZpx+7xl0dEE1la0phoPuK6D1RkSOl";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "mkX7zxVMuOKZpx+7xl0dEE1la0phoPuK6D1RkSOl";

// CS11 --------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "n5/F53dio8+egrnS/Cu72H0f+mYN7EY8HoDWjM9G";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "n5/F53dio8+egrnS/Cu72H0f+mYN7EY8HoDWjM9G";

// DEV1 -----------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "MOU7aZ0+fQZmE2uW8kU2IwPbpyDkzEJ7nMFyGZLY";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "MOU7aZ0+fQZmE2uW8kU2IwPbpyDkzEJ7nMFyGZLY";

// DEV11 --------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "+qWTzdsQzuvO9Ci9Ipc3/AZsmhbZCPyEfXNNqnwY";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "+qWTzdsQzuvO9Ci9Ipc3/AZsmhbZCPyEfXNNqnwY";

// DEV13 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "xyzYFFUnMb8zW00EUV5ed+dTuvFVpVuSHa+I6mDj";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "xyzYFFUnMb8zW00EUV5ed+dTuvFVpVuSHa+I6mDj";

// DEV14 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "7ekw6R/8xuJqPykKsyT15KbwxPe2ExNc5sSqHNXk";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "7ekw6R/8xuJqPykKsyT15KbwxPe2ExNc5sSqHNXk";

// CS3 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "ZTAwUoknDo5ppJsdyHWkX3WSk/GnOmYjXaBv5gr/";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "ZTAwUoknDo5ppJsdyHWkX3WSk/GnOmYjXaBv5gr/";

static const char TPSLIB_FILE[]   = "tesla.tpl";
static const char GUID_FILENAME[] = "guid.bin";
static const char PERSISTENT_DATA_FILENAME[] = "ccc.bin";

#define DEFAULT_PORT_HTTP   80
#define DEFAULT_PORT_TCP    8128

#define TEST_HTTP_DOWNLOAD_CONNECTION_COUNT 6

static char* RASTER_TILE_CACHE_NAME = "mapCache";
static char* VOICE_CACHE_NAME = "voiceCache";

#define VOICE_CACHE_MEMORY          64
#define VOICE_CACHE_PERSISTENT      64

#define RASTERTILE_CACHE_MEMORY     1000
#define RASTERTILE_CACHE_PERSISTENT 2000

void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData);
NB_NetworkConfiguration* GetNetworkConfiguration(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, const char* tpsBuffer, size_t tpsBufferLength, const char* guidBuffer, size_t guidBufferLength);
//nb_boolean CreateContext(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, NB_Context** context);
//static nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData);
//static nb_boolean TestPersistentDataChangedCallback(NB_PersistentData* persistentData, void* userData);
NB_Error SetTargetMappings(NB_Context* context);


nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize);
const char* TestNetwork_GetDomain();
uint64 GetMobileDirectoryNumber(PAL_Instance* pal);
uint64 GetMobileInformationNumber(PAL_Instance* pal);
nb_boolean RunningInEmulator();
PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength);
NB_PersistentData* GetPersistentData(PAL_Instance* pal);
nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData);
nb_version GetApplicationVersion();
PAL_Instance* PAL_CreateInstance();
void PAL_DestroyInstance(PAL_Instance* pal);

void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
//    int level = LogLevelMedium;
    
    if (event == NB_NetworkNotifyEvent_ClientGuidAssigned)
    {
        NB_NetworkNotifyEventClientGuid* guid = static_cast<NB_NetworkNotifyEventClientGuid*>(data);
        PAL_Instance* pal = static_cast<PAL_Instance*>(userData);
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
    
//    if (g_notifyEventInfo)
//    {
//        switch (event)
//        {
//            case NB_NetworkNotifyEvent_ConnectionOpened:
//                g_notifyEventInfo->openCount++;
//                break;
//                
//            case NB_NetworkNotifyEvent_ConnectionClosed:
//                g_notifyEventInfo->closeCount++;
//                if (g_notifyEventInfo->closeEvent)
//                {
//                    SetCallbackCompletedEvent(g_notifyEventInfo->closeEvent);
//                }
//                break;
//                
//            case NB_NetworkNotifyEvent_DataPending:
//                if (*((nb_boolean*)data))
//                {
//                    g_notifyEventInfo->dataPendingTrueCount++;
//                }
//                else
//                {
//                    g_notifyEventInfo->dataPendingFalseCount++;
//                }
//                break;
//                
//            default:
//                // No action
//                break;
//        }
//        
//        if (Test_OptionsGet()->logLevel >= level)
//        {
//            char text[100] = { 0 };
//            
//            switch (event)
//            {
//                case NB_NetworkNotifyEvent_ConnectionClosed:
//                    nsl_strcpy(text, "Connection Closed");
//                    break;
//                    
//                case NB_NetworkNotifyEvent_ConnectionOpened:
//                    nsl_strcpy(text, "Connection Opened");
//                    break;
//                    
//                case NB_NetworkNotifyEvent_DataPending:
//                    nsl_sprintf(text, "DataPending - %s", *((nb_boolean*)data) ? "True" : "False");
//                    break;
//                    
//                case NB_NetworkNotifyEvent_ClientGuidAssigned:
//                    nsl_strcpy(text, "GUID assigned");
//                    break;
//                    
//                case NB_NetworkNotifyEvent_IdenError:
//                {
//                    NB_NetworkNotifyEventError* error = data;
//                    nsl_sprintf(text, "iden error - %d (%s)", error->code, error->description ? error->description : "?");
//                }
//                    break;
//                    
//                default:
//                    nsl_sprintf(text, "Unknown (%d)", event);
//                    break;
//            }
//            
//            LOGOUTPUT(level, ("Network Notify Event: %s\n", text));
//        }
//    }
}

NB_NetworkConfiguration* GetNetworkConfiguration(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, const char* tpsBuffer, size_t tpsBufferLength, const char* guidBuffer, size_t guidBufferLength)
{
    NB_NetworkOptionsDetailed options = { 0 };
    NB_NetworkConfiguration* configuration = NULL;
    char deviceName[50] = { 0 };
    
    GetDeviceName(pal, deviceName, sizeof(deviceName) / sizeof(deviceName[0]));
    
    options.adsAppId = "N/A";
    options.classId = 0;
    options.credential = credential;
    options.device = deviceName;
    options.domain = TestNetwork_GetDomain();
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

    NB_NetworkConfigurationCreate(&options, &configuration);

    return configuration;
}

nb_boolean CCCComponent::CCC_CreateContext(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, NB_Context** context)
{
    NB_NetworkConfiguration* config = 0;
    NB_PersistentData* persistentData = 0;
    NB_CacheConfiguration voiceCacheConfig = {0};
    NB_CacheConfiguration rasterTileCacheConfig = {0};
    
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
    
    // Voice cache settings
    voiceCacheConfig.cacheName                          = VOICE_CACHE_NAME;
    voiceCacheConfig.maximumItemsInMemoryCache          = VOICE_CACHE_MEMORY;
    voiceCacheConfig.maximumItemsInPersistentCache      = VOICE_CACHE_PERSISTENT;
    voiceCacheConfig.obfuscateKey                       = (byte*)VOICE_CACHE_OBFUSCATE_KEY;
    voiceCacheConfig.obfuscateKeySize                   = sizeof(VOICE_CACHE_OBFUSCATE_KEY);
    voiceCacheConfig.configuration                      = NULL;
    
    // Voice cache settings
    rasterTileCacheConfig.cacheName                     = RASTER_TILE_CACHE_NAME;
    rasterTileCacheConfig.maximumItemsInMemoryCache     = RASTERTILE_CACHE_MEMORY;
    rasterTileCacheConfig.maximumItemsInPersistentCache = RASTERTILE_CACHE_PERSISTENT;
    rasterTileCacheConfig.obfuscateKey                  = (byte*)RASTER_TILE_CACHE_OBFUSCATE_KEY;
    rasterTileCacheConfig.obfuscateKeySize              = sizeof(RASTER_TILE_CACHE_OBFUSCATE_KEY);
    rasterTileCacheConfig.configuration                 = NULL;

    palErr = GetFileData(pal, TPSLIB_FILE, &tpsData, &tpsDataSize);
//    CU_ASSERT_EQUAL_FATAL(palErr, PAL_Ok);
    
    if (PAL_FileExists(pal, GUID_FILENAME) == PAL_Ok)
    {
        palErr = GetFileData(pal, GUID_FILENAME, &guidData, &guidDataSize);
//        CU_ASSERT_EQUAL_FATAL(palErr, PAL_Ok);
    }
    
    config = GetNetworkConfiguration(pal, credential, hostname, protocol, port, tpsData, tpsDataSize, guidData, guidDataSize);
//    CU_ASSERT_PTR_NOT_NULL(config);
    
    persistentData = GetPersistentData(pal);
//    CU_ASSERT_PTR_NOT_NULL(persistentData);
    
    if (config && persistentData)
    {
        callback.callbackData = pal;
        (void)NB_ContextCreate(pal, config, &callback, &voiceCacheConfig, &rasterTileCacheConfig, persistentData, context);
//        CU_ASSERT_PTR_NOT_NULL(*context);
        
        if (*context)
        {
            NB_Error error = NE_OK;
            AB_QaLogHeader header = { 0 };
            
            header.productName = "SysTests";
            header.productVersion = GetApplicationVersion();
            header.platformId = 1;                          /// @todo What should be used for platform id?
            header.mobileDirectoryNumber = GetMobileDirectoryNumber(pal);
            header.mobileInformationNumber = GetMobileInformationNumber(pal);
            
            error = AB_QaLogCreate(*context, &header, "SystemTests.qa", FALSE);
//            CU_ASSERT_EQUAL(error, NE_OK);
            
            error = SetTargetMappings(*context);
//            CU_ASSERT_EQUAL(error, NE_OK);
            
            error = NB_ContextInitializeGenericHttpDownloadManager(*context, TEST_HTTP_DOWNLOAD_CONNECTION_COUNT);
//            CU_ASSERT_EQUAL(error, NE_OK);
        }
        
        NB_NetworkConfigurationDestroy(config);
    }
    
    nsl_free(tpsData);
    if (guidData)
    {
        nsl_free(guidData);
    }

    PAL_UiTaskQueueAdd(pal, UI_InitComplete, this);

    return (*context) ? TRUE : FALSE;
}

NB_Error SetTargetMappings(NB_Context* context)
{
    NB_Error err = NE_OK;
    
//    if ((Test_OptionsGet()->carrier & TestCarrierVerizon) != 0)
    {
        NB_TargetMapping targets[] = {
            { "geocode",            "geocode,usa" },
            { "map",                "map,usa" },
            { "nav",                "nav,usa" },
            { "proxpoi",            "proxpoi,usa" },
            { "reverse-geocode",    "reverse-geocode,usa" }
        };
        err = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));
    }
//    else
//    {
//        NB_TargetMapping targets[] = {
//            { "geocode",            "geocode,global" },
//            { "map",                "map,global" },
//            { "nav",                "nav,global" },
//            { "proxpoi",            "proxpoi,global" },
//            { "reverse-geocode",    "reverse-geocode,global" },
//            { "traffic-notify",     "traffic-notify,global" },
//            { "vector-tile",        "vector-tile,global" }
//        };
//        err = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));
//    }
    
    return err;
}

nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize)
{
    nsl_strncpy(buffer, "iphone", bufferSize);
    return TRUE;
}

const char* TestNetwork_GetDomain()
{
    return "navbuilder.nimlbs.net";
}

#define DEFAULT_MOBILE_IDENTIFIER_NUMBER    999999999999999ULL

uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    return 3752450334;
}

uint64 GetMobileInformationNumber(PAL_Instance* pal)
{
    return 0;
}

nb_boolean RunningInEmulator()
{
    return FALSE;
}

PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength)
{
    if (dataBuffer == NULL || dataBufferLength == NULL)
    {
        return PAL_ErrBadParam;
    }
    
    return PAL_FileLoadFile(pal, filename, (unsigned char**)dataBuffer, (uint32*)dataBufferLength);
}

NB_PersistentData* GetPersistentData(PAL_Instance* pal)
{
    NB_PersistentData* persistentData = 0;
    NB_PersistentDataChangedCallback callback = {0};
    
    uint8* serializedData = 0;
    size_t serializedDataSize = 0;
    
    if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
    {
        GetFileData(pal, PERSISTENT_DATA_FILENAME, (char**)&serializedData, &serializedDataSize);
//        CU_ASSERT_EQUAL(GetFileData(pal, PERSISTENT_DATA_FILENAME, (char**)&serializedData, &serializedDataSize), PAL_Ok);
    }
    
    callback.callback = SavePersistentDataToFileCallback;
    callback.callbackData = pal;
    NB_PersistentDataCreate(serializedData, serializedDataSize, &callback, &persistentData);
//    CU_ASSERT_EQUAL(NB_PersistentDataCreate(serializedData, serializedDataSize, &callback, &persistentData), NE_OK);
    
    if (serializedData)
    {
        nsl_free(serializedData);
    }
    
    return persistentData;
}

nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData)
{
    PAL_Instance* pal = (PAL_Instance*)userData;
    PAL_Error err = PAL_Ok;
    PAL_File* persistFile = 0;
    nb_boolean successfulSave = FALSE;
    
    // write out serialized persistent data to file
    err = PAL_FileOpen(pal, PERSISTENT_DATA_FILENAME, PFM_Create, &persistFile);
    if (!err)
    {
        NB_Error result = NE_OK;
        uint8* data = 0;
        nb_size dataSize = 0;
        uint32 dataWritten = 0;
        
        result = NB_PersistentDataSerialize(persistentData, (const uint8**)&data, &dataSize);
//        CU_ASSERT_EQUAL(result, NE_OK);
//        CU_ASSERT_PTR_NOT_NULL(data);
        
        if (!result)
        {
            err = PAL_FileWrite(persistFile, data, dataSize, &dataWritten);
//            CU_ASSERT_EQUAL(err, PAL_Ok);
//            CU_ASSERT_EQUAL(dataSize, (nb_size)dataWritten);
            
            err = PAL_FileClose(persistFile);
//            CU_ASSERT_EQUAL(err, PAL_Ok);
            
            nsl_free(data);
            
            if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
            {
//                LOGOUTPUT(LogLevelHigh, (" modified persistent data saved to %s ... ", PERSISTENT_DATA_FILENAME));
                successfulSave = TRUE;
            }
        }
    }
    
//    CU_ASSERT(successfulSave);
    return successfulSave;
}

nb_version GetApplicationVersion()
{
    /// @todo Add version resource to application
    nb_version version = { 9, 0, 0, 9 };
    return version;
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = {0};
    palConfig.multiThreaded = TRUE;// dummy config
    return PAL_Create(&palConfig);
}

void PAL_DestroyInstance(PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }
    
    PAL_Destroy(pal);
}

uint8 CreatePalAndContext(PAL_Instance** pal, NB_Context** context)
{
//    NB_Error result;
    *pal = PAL_CreateInstance();
//    CU_ASSERT_PTR_NOT_NULL(*pal);
    
    if (*pal)
    {
//        nb_boolean rc = FALSE;
        
        PAL_NetInitialize(*pal);
        
//        rc = CreateContext(*pal, DEFAULT_NETWORK_TOKEN_TCP, NULL, NB_NetworkProtocol_TCP, DEFAULT_PORT_TCP, context);
//        CU_ASSERT_EQUAL(rc, TRUE);
//        CU_ASSERT_PTR_NOT_NULL(*context);
        
//        if (*context)
//        {
//            NB_GpsHistory* history = NULL;
//            NB_VectorTileManager* manager = NULL;
//            NB_VectorTileConfiguration config = {0};
//            
//            result = NB_GpsHistoryCreate(*context, 100 /* Max fixes */,
//                                         5.5 /* m/s to consider heading valid */, &history);
//            CU_ASSERT_PTR_NOT_NULL(history);
//            CU_ASSERT_EQUAL(result, NE_OK);
//
//            result = NB_ContextSetGpsHistory(*context, history);
//            CU_ASSERT_EQUAL(result, NE_OK);
//
//            config.mapPasses = 8;
//            config.mapZoomLevel = 16;
//            
//            
//            result = NB_VectorTileManagerCreate(*context, &config, NULL, &manager);
//            CU_ASSERT_PTR_NOT_NULL(manager);
//            CU_ASSERT_EQUAL(result, NE_OK);
//
//            result = NB_ContextSetVectorTileManager(*context, manager);
//            CU_ASSERT_EQUAL(result, NE_OK);
//        }
    }
    
    if ((*pal) && !(*context))
    {
        PAL_NetShutdown(*pal);
        PAL_DestroyInstance(*pal);
        *pal = 0;
    }

    return (*pal) ? 1 : 0;
}

CCCComponent::CCCComponent(void)
{
    m_pPal = NULL;
    m_pContext = NULL;
}

CCCComponent::~CCCComponent(void)
{
    
}

void CCCComponent::Initialize(void)
{
    m_pPal = PAL_CreateInstance();
    
    if (m_pPal)
    {
//        nb_boolean rc = FALSE;
        PAL_NetInitialize(m_pPal);

        TaskId taskId = 0;
        PAL_EventTaskQueueAdd(m_pPal, CCC_Initialize, this, &taskId);
//        rc = CCC_CreateContext(m_pPal, DEFAULT_NETWORK_TOKEN_TCP, NULL, NB_NetworkProtocol_TCP, DEFAULT_PORT_TCP, &m_pContext);
    }
}

void CCCComponent::AddListener(void* pListener)
{
    m_listenerVector.push_back(pListener);
}

PAL_Instance* CCCComponent::GetPalInstance(void) const
{
    return m_pPal;
}

NB_Context* CCCComponent::GetContext(void) const
{
    return m_pContext;
}

void CCCComponent::UI_InitComplete(PAL_Instance* pal, void* userData)
{
    CCCComponent* pThis = static_cast<CCCComponent*>(userData);
    pThis->UI_InitComplete();
}

void CCCComponent::UI_InitComplete(void)
{
    int count = m_listenerVector.size();
    for (int i=0; i<count; ++i)
    {
        id<CCCEventListener> pListener = (__bridge id<CCCEventListener>)m_listenerVector[i];
        [pListener initCCCComplete];
    }
}

void CCCComponent::CCC_Initialize(PAL_Instance* pal, void* userData)
{
    CCCComponent* pThis = static_cast<CCCComponent*>(userData);
    pThis->CCC_CreateContext(pal, DEFAULT_NETWORK_TOKEN_TCP, "", NB_NetworkProtocol_TCP, DEFAULT_PORT_TCP, &pThis->m_pContext);
}
