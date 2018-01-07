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
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
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

#include <jni.h>
#include "com_navbuilder_nb_internal_NBInternal.h"
#include "jninbcontext.h"
#include "jninbnetworkconfiguration.h"
#include "jnihelper.h"
#include "logutil.h"
#include <stdlib.h>

extern "C"{
#include "paltypes.h"
#include "nbcontext.h"
#include "nbnetwork.h"
#include "palconfig.h"
#include "palfile.h"
#include "palnet.h"
#include "nbcontextaccess.h"
}

#define RASTER_TILE_CACHE_NAME      "mapCache"
#define VOICE_CACHE_NAME            "voiceCache"

#define VOICE_CACHE_MEMORY          64
#define VOICE_CACHE_PERSISTENT      64

#define RASTERTILE_CACHE_MEMORY     1000
#define RASTERTILE_CACHE_PERSISTENT 2000

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

static const char PERSISTENT_DATA_FILENAME[] = "ccc.bin";

void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    //int level = LogLevelMedium;
	LOGI("NetworkNotifyEventCallback: event=%u\n", event);

    if (event == NB_NetworkNotifyEvent_ClientGuidAssigned)
    {
        NB_NetworkNotifyEventClientGuid* guid = (NB_NetworkNotifyEventClientGuid*)data;
/*        PAL_Instance* pal = (PAL_Instance*)userData;
        PAL_File* file = 0;
        PAL_Error err = PAL_Ok;
        
        err = PAL_FileOpen(pal, GUID_FILENAME, PFM_Create, &file);
        if (!err)
        {
            uint32 written = 0;
            err = PAL_FileWrite(file, guid->guidData, guid->guidDataLength, &written);
            PAL_FileClose(file);
        }*/
    }
}

/*PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength)
{
    if (dataBuffer == NULL || dataBufferLength == NULL)
    {
        return PAL_ErrBadParam;
    }

    return PAL_FileLoadFile(pal, filename, (unsigned char**)dataBuffer, (uint32*)dataBufferLength);
}*/

uint64 charToUint64(const char* str)
{
	const char *p;
	char c;
	int digval;
	uint64 number;

	p = str;            /* p is our scanning pointer */
    number = 0;            /* start with zero */

    c = *p++;            /* read char */

	if (c == '-')
        c = *p++;		/* skip sign */
    else if (c == '+')
        c = *p++;        /* skip sign */

    for (;;) {    /* exit in middle of loop */
        /* convert c to value */
		if (c == '\0')
			break;
        digval = c - '0';

        number = number * 10 + digval;

        c = *p++;        /* read next digit */
    }

	return number;
}

NB_NetworkConfiguration* GetNetworkConfiguration(PAL_Instance* pal, JNINBNetworkConfiguration* jniNetworkConfig,
												 const char* tpsBuffer, int tpsBufferLength)
{
    NB_NetworkConfiguration* configuration = NULL;
/*
    AB_NetworkOptionsDetailed options = { 0 };

	options.adsAppId = "N/A";
    options.classId = 0;
	options.credential = jniNetworkConfig->getAuthenticationToken();
	options.device = jniNetworkConfig->getDevice();
    options.domain = jniNetworkConfig->getHostName();
    options.firmwareVersion = "N/A";
	options.hostname = "";
	options.language = jniNetworkConfig->getLocale();
	options.mdn = charToUint64(jniNetworkConfig->getMdn());
    options.min = 0;
    options.platformId = 0;
	options.port = jniNetworkConfig->getHostPort();
    options.priceType = 0;
	options.protocol = NB_NetworkProtocol_TCP;
    options.tpslibData = tpsBuffer;
    options.tpslibLength = tpsBufferLength;
	options.clientGuidData = jniNetworkConfig->getClientGuid();
    options.clientGuidLength = jniNetworkConfig->getClientGuidSize();
    options.isEmulator = FALSE;

	NB_Error err = AB_NetworkConfigurationCreate(&options, &configuration);
*/
    return configuration;
}

nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData)
{
    PAL_Instance* pal = (PAL_Instance*)userData;
    PAL_Error err = PAL_Ok;
    PAL_File* persistFile = 0;
    nb_boolean successfulSave = FALSE;

    // write out serialized persistent data to file
    /*err = PAL_FileOpen(pal, PERSISTENT_DATA_FILENAME, PFM_Create, &persistFile);
    if (!err)
    {
        NB_Error result = NE_OK;
        uint8* data = 0;
        nb_size dataSize = 0;
        uint32 dataWritten = 0;

        result = NB_PersistentDataSerialize(persistentData, (const uint8**)&data, &dataSize);
        //CU_ASSERT_EQUAL(result, NE_OK);
        //CU_ASSERT_PTR_NOT_NULL(data);

        if (!result)
        {
            err = PAL_FileWrite(persistFile, data, dataSize, &dataWritten);
            //CU_ASSERT_EQUAL(err, PAL_Ok);
            //CU_ASSERT_EQUAL(dataSize, (nb_size)dataWritten);

            err = PAL_FileClose(persistFile);
            //CU_ASSERT_EQUAL(err, PAL_Ok);

            nsl_free(data);

            if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
            {
                //LOGOUTPUT(LogLevelHigh, (" modified persistent data saved to %s ... ", PERSISTENT_DATA_FILENAME));
                successfulSave = TRUE;
            }
        }
    }*/

    //CU_ASSERT(successfulSave);
    return successfulSave;
}

NB_PersistentData* GetPersistentData(PAL_Instance* pal)
{
    NB_PersistentData* persistentData = 0;
    NB_PersistentDataChangedCallback callback = {0};

    uint8* serializedData = 0;
    size_t serializedDataSize = 0;
    
/*    if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
    {
        GetFileData(pal, PERSISTENT_DATA_FILENAME, (char**)&serializedData, &serializedDataSize);
    }*/

    callback.callback = SavePersistentDataToFileCallback;
    callback.callbackData = pal;  
    NB_PersistentDataCreate(serializedData, serializedDataSize, &callback, &persistentData);

    if (serializedData)
    {
        nsl_free(serializedData);
    }

    return persistentData;
}

NB_Error SetTargetMappings(NB_Context* context)
{
    NB_Error err = NE_OK;

         NB_TargetMapping targets[] = {
            { "geocode",            "geocode,usa" },
            { "map",                "map,global" },
            { "nav",                "nav,global" },
            { "proxpoi",            "proxpoi,global" },
            { "reverse-geocode",    "reverse-geocode,global" },
            { "traffic-notify",     "traffic-notify,global" },
            { "vector-tile",        "vector-tile,global" }
        };

        err = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));

    return err;
}

nb_boolean CreateContext(PAL_Instance* pal,
							NB_NetworkConfiguration*  networkConfig,
							NB_Context** context)
{
    NB_PersistentData* persistentData = 0;
    NB_CacheConfiguration voiceCacheConfig = {0};
    NB_CacheConfiguration rasterTileCacheConfig = {0};

    NB_NetworkNotifyEventCallback callback = { NetworkNotifyEventCallback, 0 };
    PAL_Error palErr = PAL_Ok;

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

    persistentData = GetPersistentData(pal);

    if (persistentData)
    {
        callback.callbackData = pal;
        (void)NB_ContextCreate(pal, networkConfig, &callback, &voiceCacheConfig, &rasterTileCacheConfig, persistentData, context);

        if (*context)
        {
            //NB_Error err = NE_OK;
            //AB_QaLogHeader header = { 0 };
            //
            //header.productName = "SysTests";
            //header.productVersion = GetApplicationVersion();
            //header.platformId = 1;                          /// @todo What should be used for platform id?
            //header.mobileDirectoryNumber = GetMobileDirectoryNumber(pal);
            //header.mobileInformationNumber = GetMobileInformationNumber(pal);

            //err = AB_QaLogCreate(*context, &header, "SystemTests.qa", Test_OptionsGet()->verboseQaLog);
            ////CU_ASSERT_EQUAL(err, NE_OK);

            NB_Error err = SetTargetMappings(*context);
            //CU_ASSERT_EQUAL(err, NE_OK);
        }
    }

    return (*context) ? TRUE : FALSE;
}

uint8 myCreatePalAndContext(PAL_Instance** pal, NB_Context** context,
							JNINBNetworkConfiguration* jniNetworkConfig,
							char* tpsData, int tpsDataSize)
{
    NB_Error result;


	PAL_Config palConfig = { 1 };
    *pal = PAL_Create(&palConfig);

#if 0
    if (*pal)
    {
        nb_boolean rc = FALSE;

        PAL_NetInitialize(*pal);
		NB_NetworkConfiguration* netConfig = GetNetworkConfiguration(*pal, jniNetworkConfig, tpsData, tpsDataSize);

		rc = CreateContext(*pal, netConfig, context);

		AB_NetworkConfigurationDestroy(netConfig);

        if (*context) 
        {
            NB_GpsHistory* history = NULL;
            NB_VectorTileManager* manager = NULL;
            NB_VectorTileConfiguration config = {0};

            result = NB_GpsHistoryCreate(*context, 100 /* Max fixes */, 
                5.5 /* m/s to consider heading valid */, &history);

            result = NB_ContextSetGpsHistory(*context, history);

            config.mapPasses = 8;
            config.mapZoomLevel = 16;

            result = NB_VectorTileManagerCreate(*context, &config, NULL, &manager);

            result = NB_ContextSetVectorTileManager(*context, manager);
        }
    }

    if ((*pal) && !(*context))
    {
        PAL_NetShutdown(*pal);
        PAL_Destroy(*pal);
        *pal = 0;
    }
#endif
    return (*pal) ? 1 : 0;
}

JNIEXPORT jint JNICALL Java_com_navbuilder_nb_internal_NBInternal_nativeCreateContext
(JNIEnv *env, jobject thiz, jobject jnetworkconfig, jbyteArray jtpsdata)
{
	LOGI("---> nativeCreateContext\n");

	PAL_Instance* pal = 0;
	NB_Context* context = 0;

	JNINBNetworkConfiguration* jniNetworkConfig = new JNINBNetworkConfiguration(env, jnetworkconfig);
	int tpsDataSize = 0;
	char* tpsData = jByteArrayToChars(env, jtpsdata, &tpsDataSize);

	if (jniNetworkConfig && tpsData)
	{

		uint8 rc = myCreatePalAndContext(&pal, &context, jniNetworkConfig, tpsData, tpsDataSize);

		if (rc) {
			JNINBContext* jni_context = new JNINBContext();

			if (jni_context == NULL)
			{
				printf("ERROR");
				//return NE_NOMEM;
			}

			jint err = env->GetJavaVM(&jni_context->mJavaVM);
			jni_context->nb_context = context;
			jni_context->pal_instance = pal;

			delete jniNetworkConfig;
			free(tpsData);

			LOGI("<--- nativeCreateContext SUCCESS: %d\n", jni_context);
			return (jint) jni_context;
		}
	}

	if (jniNetworkConfig)
		delete jniNetworkConfig;

	if (tpsData)
		free(tpsData);

	LOGE("<--- nativeCreateContext FAILED\n");
	return (jint) 0;
}

JNIEXPORT void JNICALL Java_com_navbuilder_nb_internal_NBInternal_nativeDestroyContext
(JNIEnv *env, jobject thiz, jint jcontext)
{
	LOGI("---> nativeDestroyContext\n");
	JNINBContext* jnicontext = (JNINBContext*) jcontext;
	LOGI("---> nativeDestroyContext jnicontext=%d\n", jnicontext);

	if (jnicontext)
	{
		NB_ContextDestroy(jnicontext->nb_context);
		PAL_Destroy(jnicontext->pal_instance);

		delete jnicontext;
	}

	LOGI("<--- nativeDestroyContext\n");
}