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
 @file     NativeNBContext.cpp
 */
/*
 (C) Copyright 2013 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "nativeltkcontext.h"
#include "paldebuglog.h"
#include "palstdlib.h"
#include "paltestlog.h"
#include <jni.h>
#include "MapServicesConfiguration.h"
#include <nbnetwork.h>
#include <nbnetworkoptions.h>
#include <palfile.h>
#include "MetadataConfiguration.h"
#include "JNIUtility.h"
#include <android/log.h>
#include <nbcontextaccess.h>
#include "contextbasedsingleton.h"
#include "abqalog.h"
#include "QaLog.h"
#include "QaLogListenerInterface.h"

static const char PERSISTENT_DATA_FILENAME[] = "ccc.bin";
static std::string persistentDataFullPath;

#define  LOG_TAG    "NativeNBContext"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOG_LINE   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s, %d", __func__, __LINE__)

struct fields_context
{
    jclass clazz;
    jclass nativeNBContextClazz;
    jclass nativeSslFactoryClazz;
    jfieldID adsAppId;
    jfieldID credential;
    jfieldID device;
    jfieldID deviceSerialNumber;
    jfieldID model;
    jfieldID domain;
    jfieldID firmwareVersion;
    jfieldID sslTrust;
    jfieldID hostname;
    jfieldID language;
    jfieldID mdn;
    jfieldID min;
    jfieldID platformId;
    jfieldID port;
    jfieldID priceType;
    jfieldID protocol;
    jfieldID tps;
    jfieldID guid;
    jfieldID pin;
    jfieldID mcc;
    jfieldID mnc;
    jfieldID countryCode;
    jfieldID isEmulator;
    jfieldID carrier;
    jfieldID pushMessageGuid;
    jfieldID pushGuidEncode;
    jfieldID deviceUniqueIdentifier;
    jfieldID manufacturer;
    jmethodID onUploadComplete;
    jmethodID onRequestCancelled;
    jmethodID onRequestError;
    jmethodID onRequestProgress;
    jmethodID onRequestStart;
};

static fields_context fields;

//these 2 only for ssl trust feature.
//TODO:make the risk lowest, so add 2 globe variable. this need refactor after app release.
static string g_domian;
static jint g_sslTrust;

struct NetworkConfigTask
{
    NetworkConfigTask(NB_Context* context, NB_NetworkConfiguration* configuration)
        : nbContext(context), networkConfiguration(configuration)
    {}

    NB_Context* nbContext;
    NB_NetworkConfiguration* networkConfiguration;
};

static NB_NetworkConfiguration*
GetNetworkConfiguration(JNIEnv* env, jobject networkOptions, bool isQaLogServer)
{
    NB_NetworkOptionsDetailed options = { 0 };
    NB_NetworkConfiguration* configuration = NULL;

    jstring str = (jstring)env->GetObjectField(networkOptions, fields.adsAppId);
    string adsAppId = NewStdString(env, str);
    options.adsAppId = adsAppId.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.adsAppId:%s", options.adsAppId);

    str = (jstring)env->GetObjectField(networkOptions, fields.credential);
    string credential = NewStdString(env, str);
    options.credential = isQaLogServer ? ("EPVSQrhmn2zvyMvfAH1YChO3mi2RcQB2drzC/KOG") : (credential.c_str());
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.credential:%s", options.credential);

    str = (jstring)env->GetObjectField(networkOptions, fields.device);
    string device = NewStdString(env, str);
    options.device = device.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.device:%s", options.device);

    str = (jstring)env->GetObjectField(networkOptions, fields.deviceSerialNumber);
    string deviceSerialNumber = NewStdString(env, str);
    options.deviceSerialNumber = deviceSerialNumber.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.deviceSerialNumber:%s", options.deviceSerialNumber);

    str = (jstring)env->GetObjectField(networkOptions, fields.model);
    string model = NewStdString(env, str);
    options.model = model.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.model:%s", options.model);

    str = (jstring)env->GetObjectField(networkOptions, fields.firmwareVersion);
    string firmwareVersion = NewStdString(env, str);
    options.firmwareVersion = firmwareVersion.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.firmwareVersion:%s", options.firmwareVersion);

    str = (jstring)env->GetObjectField(networkOptions, fields.domain);
    string domain = NewStdString(env, str);
    if(!isQaLogServer)
    {
        g_domian = domain;
    }
    options.domain = isQaLogServer ? ("nimupdate.com") : (domain.c_str());
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.domain:%s", options.domain);

    str = (jstring)env->GetObjectField(networkOptions, fields.hostname);
    string hostname = NewStdString(env, str);
    options.hostname = isQaLogServer ? ("qalog") : (hostname.c_str());
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.hostname:%s", options.hostname);

    str = (jstring)env->GetObjectField(networkOptions, fields.language);
    string language = NewStdString(env, str);
    options.language = language.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.language:%s", options.language);

    str = (jstring)env->GetObjectField(networkOptions, fields.mdn);
    string mdn = NewStdString(env, str);
    nsl_sscanf(mdn.c_str(), "%llu", &options.mdn);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.mdn:%llu", options.mdn);

    str = (jstring)env->GetObjectField(networkOptions, fields.manufacturer);
    string manufacturer = NewStdString(env, str);
    options.manufacturer = manufacturer.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.manufacturer:%s", options.manufacturer);

    str = (jstring)env->GetObjectField(networkOptions, fields.min);
    string min = NewStdString(env, str);
    nsl_sscanf(min.c_str(), "%llu", &options.min);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.min:%llu", options.min);

    str = (jstring)env->GetObjectField(networkOptions, fields.pin);
    string pin = NewStdString(env, str);
    options.pin = pin.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.pin:%s", options.pin);

    str = (jstring)env->GetObjectField(networkOptions, fields.countryCode);
    string countryCode = NewStdString(env, str);
    options.countryCode = countryCode.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.countryCode:%s", options.countryCode);

    str = (jstring)env->GetObjectField(networkOptions, fields.carrier);
    string carrier = NewStdString(env, str);
    options.carrier = carrier.c_str();
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.carrier:%s", options.carrier);

    str = (jstring)env->GetObjectField(networkOptions, fields.pushMessageGuid);
    string pushMessageGuid = NewStdString(env, str);
    if(pushMessageGuid.empty())
    {
        options.pushMessageGuid = NULL;
        options.pushMessageGuidLength = 0;
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.pushMessageGuid:pointer[NULL] - len[0]");
    }
    else
    {
        options.pushMessageGuid = pushMessageGuid.c_str();
        options.pushMessageGuidLength = pushMessageGuid.length();
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.pushMessageGuid:pointer[%p] - len[%d]", options.pushMessageGuid, options.pushMessageGuidLength);
    }

    str = (jstring)env->GetObjectField(networkOptions, fields.deviceUniqueIdentifier);
    string deviceUniqueIdentifier;
    if (str)
    {
        deviceUniqueIdentifier = NewStdString(env, str);
        options.deviceUniqueIdentifier = deviceUniqueIdentifier.c_str();
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.deviceUniqueIdentifier:%s", options.deviceUniqueIdentifier);
    }

    options.platformId = env->GetIntField(networkOptions, fields.platformId);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.platformId:%d", options.platformId);

    g_sslTrust = (env->GetBooleanField(networkOptions, fields.sslTrust)) ? TRUE : FALSE;
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.sslTrust:%d", g_sslTrust);

    options.port = env->GetIntField(networkOptions, fields.port);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.port:%d", options.port);

    options.priceType = env->GetIntField(networkOptions, fields.priceType);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.priceType:%d", options.priceType);

    options.protocol = (NB_NetworkProtocol)env->GetIntField(networkOptions, fields.protocol);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.protocol:%d", options.protocol);

    options.mcc = env->GetIntField(networkOptions, fields.mcc);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.mcc:%d", options.mcc);

    options.mnc = env->GetIntField(networkOptions, fields.mnc);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.mnc:%d", options.mnc);

    options.isEmulator = (env->GetIntField(networkOptions, fields.isEmulator) == 0) ? FALSE:TRUE;
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.isEmulator:%d", options.isEmulator);

    options.pushGuidEncode = env->GetIntField(networkOptions, fields.pushGuidEncode);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.pushGuidEncode:%d", options.pushGuidEncode);

    jbyteArray data = (jbyteArray)env->GetObjectField(networkOptions, fields.tps);
    jbyte* tpsData = env->GetByteArrayElements(data, NULL);
    jsize tpsDataSize = env->GetArrayLength(data);
    options.tpslibData = (char*)tpsData;
    options.tpslibLength = tpsDataSize;

    data = (jbyteArray)env->GetObjectField(networkOptions, fields.guid);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "networkOptions.guid:%p", data);
    if(data)
    {
        jbyte* guidData = env->GetByteArrayElements(data, NULL);
        jsize guidDataSize = env->GetArrayLength(data);
        options.clientGuidData = (char*)guidData;
        options.clientGuidLength = guidDataSize;
    }
    else
    {
        options.clientGuidData = NULL;
        options.clientGuidLength = 0;
    }

    NB_NetworkConfigurationCreate(&options, &configuration);
    return configuration;
}

static JavaVM* cachedVM = NULL;

JNIEnv* getJNIEnv()
{
    JNIEnv* env = NULL;
    if (cachedVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        // Should never happen
        assert(false);
    }
    return env;
}


class QaLogListener: public QaLogListenerInterface
{
public:
    QaLogListener(jobject androidObj)
    {
        mAndroidObject = getJNIEnv()->NewGlobalRef(androidObj);
    }

    ~QaLogListener()
    {
        getJNIEnv()->DeleteGlobalRef(mAndroidObject);
    }

    virtual void UploadStarted()
    {
        JNIEnv* env = getJNIEnv();
        env->CallVoidMethod(mAndroidObject, fields.onRequestStart);
    }

    virtual void UploadCanceled()
    {
        JNIEnv* env = getJNIEnv();
        env->CallVoidMethod(mAndroidObject, fields.onRequestCancelled);
    }

    virtual void UploadCompleted(const std::string& logid)
    {
        JNIEnv* env = getJNIEnv();
        jstring logidString = env->NewStringUTF(logid.c_str());

        env->CallVoidMethod(mAndroidObject, fields.onUploadComplete, logidString);

        env->DeleteLocalRef(logidString);
    }

    virtual void UploadProgress(int percentage)
    {
        JNIEnv* env = getJNIEnv();
        env->CallVoidMethod(mAndroidObject, fields.onRequestProgress, static_cast<jint>(percentage));
    }

    virtual void LogFileEmpty()
    {
        JNIEnv* env = getJNIEnv();
        env->CallVoidMethod(mAndroidObject, fields.onRequestError, static_cast<jint>(NE_INVAL));
    }

    virtual void UploadFailed (NB_Error error)
    {
        JNIEnv* env = getJNIEnv();
        env->CallVoidMethod(mAndroidObject, fields.onRequestError, static_cast<jint>(error));
    }

private:
    jobject mAndroidObject;
};


static void JNI_CreateQalog(JNIEnv* env, jobject thiz, jlong ltkContext, jobject qalogNetworkOptions)
{
    //waiting for java side ok.

    NativeLTKContext* nativeLTKContext = static_cast<NativeLTKContext*>((void*)ltkContext);
    if(!nativeLTKContext || !(nativeLTKContext->nbContext))
    {
        return;
    }

    NB_NetworkConfiguration* qaLogNetworkConfiguration = GetNetworkConfiguration(env, qalogNetworkOptions, true);
    if(!qaLogNetworkConfiguration)
    {
        LOGE("-----CreateNativeContext: GetNetworkConfiguration for qalog failed");
        return;
    }
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: After GetNetworkConfiguration for qalog");

    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(nativeLTKContext->nbContext);
    if(qaLogPtr)
    {
        if(!qaLogPtr->Initialize(qaLogNetworkConfiguration, nativeLTKContext->pal))
        {
            return;
        }
        LOGE("Initialization QA log instance success.");
    }

    nativeLTKContext->listener = new QaLogListener(thiz);
    qaLogPtr->RegisterListener(nativeLTKContext->listener);
}


static void JNI_SetQaLogConfiguration(JNIEnv* env, jobject thiz,
        jlong ltkContext,
        jint major, jint minor, jint feature, jint build,
        jlong mdn, jstring productName, jint platformId, jstring filename)
{
    LOGE("QaLog::JNI_SetQaLogConfiguration");
    NativeLTKContext* nativeLTKContext = static_cast<NativeLTKContext*>((void*)ltkContext);
    if(!nativeLTKContext || !(nativeLTKContext->nbContext))
    {
        return;
    }

    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(nativeLTKContext->nbContext);
    if(!qaLogPtr)
    {
        return;
    }

    const char *tmpProductName = env->GetStringUTFChars(productName, NULL);
    const char *tmpFilename = env->GetStringUTFChars(filename, NULL);

    AB_QaLogHeader qalogHeader = {0};
    qalogHeader.productName = tmpProductName;
    qalogHeader.productVersion.major = (int)major;
    qalogHeader.productVersion.minor = (int)minor;
    qalogHeader.productVersion.feature = (int)feature;
    qalogHeader.productVersion.build = (int)build;
    qalogHeader.platformId = 0;
    qalogHeader.mobileDirectoryNumber = (uint64)mdn;
    qalogHeader.mobileInformationNumber = 0;

    qaLogPtr->SetQaLogConfiguration(&qalogHeader, tmpFilename);

    env->ReleaseStringUTFChars(productName, tmpProductName);
    env->ReleaseStringUTFChars(filename, tmpFilename);
}

static void JNI_StartLogging(JNIEnv* env, jobject thiz, jlong ltkContext, jboolean verbose)
{
    NativeLTKContext* nativeLTKContext = static_cast<NativeLTKContext*>((void*)ltkContext);
    if(!nativeLTKContext || !(nativeLTKContext->nbContext))
    {
        return;
    }

    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(nativeLTKContext->nbContext);
    if(!qaLogPtr)
    {
        return;
    }
    qaLogPtr->StartLogging(verbose == TRUE);
}

static void JNI_UploadLogFile(JNIEnv* env, jobject thiz, jlong ltkContext)
{
    NativeLTKContext* nativeLTKContext = static_cast<NativeLTKContext*>((void*)ltkContext);
    if(!nativeLTKContext || !(nativeLTKContext->nbContext))
    {
        return;
    }

    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(nativeLTKContext->nbContext);
    if(!qaLogPtr)
    {
        return;
    }
    qaLogPtr->UploadLogFile();
}

static void JNI_CancelUpload(JNIEnv* env, jobject thiz, jlong ltkContext)
{
    NativeLTKContext* nativeLTKContext = static_cast<NativeLTKContext*>((void*)ltkContext);
    if(!nativeLTKContext || !(nativeLTKContext->nbContext))
    {
        return;
    }

    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(nativeLTKContext->nbContext);
    if(!qaLogPtr)
    {
        return;
    }
    qaLogPtr->CancelUpload();
}


// TODO:   move to app side
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

// TODO:   move to app side
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

static void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
}

static void CCC_ResetNetwork(PAL_Instance* pal, void* userData)
{
    NetworkConfigTask* task = static_cast<NetworkConfigTask*>(userData);
    if (!task)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----CCC_ResetNetwork: nbContext is null.");
        return;
    }

    NB_Error err = NB_ContextNetworkReset(task->nbContext, task->networkConfiguration, NULL);
    if (err != NE_OK)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----CCC_ResetNetwork: reset failed.");
    }

    NB_NetworkConfigurationDestroy(task->networkConfiguration);
    delete task;
}

//The code will be delete after the sample APP UI done.
static void CCC_InternalTestQaLog(PAL_Instance* pal, void* userData)
{
    if(!userData)
    {
        return;
    }
    NB_Context* nbContext = (NB_Context*)userData;
    NB_Error err = NE_OK;
    AB_QaLogHeader header = { 0 };

    nb_version version = {0};
    version.build = 4;
    version.feature = 4;
    version.major = 4;
    version.minor = 4;
    header.productName = "jbai_SysTests";
    header.productVersion = version;
    header.platformId = 1;                          /// @todo What should be used for platform id?
    header.mobileDirectoryNumber = 4444;
    header.mobileInformationNumber = 4444;

    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(nbContext);
    if(qaLogPtr)
    {
        qaLogPtr->SetQaLogConfiguration(&header, "/data/data/com.locationtoolkit.sample.map3d/cache/qalog_111");
        qaLogPtr->StartLogging(true);
        qaLogPtr->UploadLogFile();
        qaLogPtr->CancelUpload();
    }
}

/*! Save persistent data to file.

All contexts created via CreateContext will include a persistent data object that will
be created with the content from ccc.bin, and will invoke this callback and save to ccc.bin
when there is a data change.

@return Non-zero if data successfully serialized and written to disk
*/
nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData)
{
    PAL_Error err = PAL_Ok;
    PAL_File* persistFile = 0;
    nb_boolean successfulSave = FALSE;

    PAL_Instance* pal = static_cast<PAL_Instance*>(userData);
    if(!pal || !persistentData || persistentDataFullPath.empty())
    {
        return successfulSave;
    }

    // write out serialized persistent data to file
    err = PAL_FileOpen(pal, persistentDataFullPath.c_str(), PFM_Create, &persistFile);
    if (!err)
    {
        NB_Error result = NE_OK;
        uint8* data = 0;
        nb_size dataSize = 0;
        uint32 dataWritten = 0;

        result = NB_PersistentDataSerialize(persistentData, (const uint8**)&data, &dataSize);
        if(result != NE_OK || !data)
        {
            return successfulSave;
        }

        err = PAL_FileWrite(persistFile, data, dataSize, &dataWritten);
        if( err != PAL_Ok || dataSize != (nb_size)dataWritten )
        {
            return successfulSave;
        }

        PAL_FileClose(persistFile);
        nsl_free(data);

        if (PAL_FileExists(pal, persistentDataFullPath.c_str()) == PAL_Ok)
        {
            PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, " modified persistent data saved to %s ... ", persistentDataFullPath.c_str());
            successfulSave = TRUE;
        }
    }

    return successfulSave;
}

PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength)
{
    if (dataBuffer == NULL || dataBufferLength == NULL)
    {
        return PAL_ErrBadParam;
    }

    return PAL_FileLoadFile(pal, filename, (unsigned char**)dataBuffer, (uint32*)dataBufferLength);
}

/*! Get persistent data object.

Create a persistent data object and initialize it with the serialized content of ccc.bin
from previous test runs.  Set data change callback to SavePersistentDataToFileCallback.

@return pointer to created and initialized persistent data change objecte
*/
NB_PersistentData* GetPersistentData(PAL_Instance* pal)
{
    NB_PersistentData* persistentData = 0;
    NB_PersistentDataChangedCallback callback = {0};

    if(persistentDataFullPath.empty())
    {
        return persistentData;
    }

    uint8* serializedData = 0;
    size_t serializedDataSize = 0;

    if (PAL_FileExists(pal, persistentDataFullPath.c_str()) == PAL_Ok)
    {
        GetFileData(pal, persistentDataFullPath.c_str(), (char**)&serializedData, &serializedDataSize);
    }

    callback.callback = SavePersistentDataToFileCallback;
    callback.callbackData = pal;
    NB_Error ret = NB_PersistentDataCreate(serializedData, serializedDataSize, &callback, &persistentData);

    if (ret == NE_OK && serializedData)
    {
        nsl_free(serializedData);
    }

    return persistentData;
}


static jint JNI_CreateNativeContext(JNIEnv* env, jobject thiz, jobject networkOptions, jfloat dpi, jstring workFolder)
{
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext");

//    NBContextConfiguration ltkContext = GetNBContextConfiguration();
    PAL_Config palConfig = { 0 };
    /*
     :KLUDGE:
     Set the multi-threaded flag to TRUE! This should be taken from a platform-specific configuration!
     */
    palConfig.multiThreaded = TRUE;
    palConfig.dpi = dpi;

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: pre-Pal create");
    PAL_Instance* palInstance = PAL_Create(&palConfig);
    if (palInstance == NULL)
    {
        return 0;
        LOGE("-----CreateNativeContext: Create PAL failed");
    }
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: post-Pal create");

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: Before PAL_NetInitialize");
    PAL_Error palResult = PAL_NetInitialize(palInstance);
    if (palResult != PAL_Ok)
    {
        LOGE("-----CreateNativeContext: PAL init failed");
        return 0;
    }
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: After PAL_NetInitialize");

    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: Before GetNetworkConfiguration");
    NB_NetworkConfiguration* networkConfiguration = GetNetworkConfiguration(env, networkOptions, false);
    if(!networkConfiguration)
    {
        LOGE("-----CreateNativeContext: GetNetworkConfiguration failed");
        return 0;
    }
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: After GetNetworkConfiguration");

    NB_NetworkConfiguration* qaLogNetworkConfiguration = GetNetworkConfiguration(env, networkOptions, true);
    if(!qaLogNetworkConfiguration)
    {
        LOGE("-----CreateNativeContext: GetNetworkConfiguration for qalog failed");
        return 0;
    }
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: After GetNetworkConfiguration for qalog");

    NB_NetworkNotifyEventCallback networkCallback = { NetworkNotifyEventCallback, 0 };

    NB_CacheConfiguration voiceCacheConfig = { 0 };
    voiceCacheConfig.cacheName = (char*)"voiceCache";
    voiceCacheConfig.maximumItemsInMemoryCache = 64;
    voiceCacheConfig.maximumItemsInPersistentCache = 64;
    voiceCacheConfig.obfuscateKey = (byte*) VOICE_CACHE_OBFUSCATE_KEY;
    voiceCacheConfig.obfuscateKeySize = sizeof(VOICE_CACHE_OBFUSCATE_KEY);
    voiceCacheConfig.configuration = NULL;

    // Voice cache settings
    NB_CacheConfiguration rasterTileCacheConfig = { 0 };
    rasterTileCacheConfig.cacheName = (char*)"mapCache";
    rasterTileCacheConfig.maximumItemsInMemoryCache = 1000;
    rasterTileCacheConfig.maximumItemsInPersistentCache = 2000;
    rasterTileCacheConfig.obfuscateKey = (byte*) RASTER_TILE_CACHE_OBFUSCATE_KEY;
    rasterTileCacheConfig.obfuscateKeySize = sizeof(RASTER_TILE_CACHE_OBFUSCATE_KEY);
    rasterTileCacheConfig.configuration = NULL;

    const char *tmpWorkFolder = env->GetStringUTFChars(workFolder, NULL);
    if(tmpWorkFolder)
    {
        persistentDataFullPath.append(tmpWorkFolder);
        persistentDataFullPath.append("/");
        persistentDataFullPath.append(PERSISTENT_DATA_FILENAME);
    }

    NB_PersistentData* persistentData = GetPersistentData(palInstance);
    env->ReleaseStringUTFChars(workFolder, tmpWorkFolder);

    NativeLTKContext* context = new NativeLTKContext;
    context->pal = palInstance;
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: Before NB_ContextCreate");
    NB_ContextCreate(palInstance, networkConfiguration, &networkCallback, &voiceCacheConfig, &rasterTileCacheConfig, persistentData, &context->nbContext);
    PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----createNativeContext: After NB_ContextCreate");

    if(!context->nbContext)
    {
        return 0;
    }

    NB_TargetMapping targets[] =
    {
    { "geocode",            "geocode,global" },
    { "map",                "map,global" },
    { "nav",                "nav,global" },
    { "proxpoi",            "proxpoi,global" },
    { "reverse-geocode",    "reverse-geocode,global" },
    { "traffic-notify",     "traffic-notify,global" },
    { "vector-tile",        "vector-tile,global" },
    { "coupon",             "coupons" },
    { "metadata-source",    "metadata" },
    { "metadata-manifest",  "metadata" },
    { "traffic-notify",     "traffic-notify,global" }
    };
    NB_ContextSetTargetMappings(context->nbContext, targets, sizeof(targets) / sizeof(targets[0]));

    /*
    QaLogPtr qaLogPtr = ContextBasedSingleton<QaLog>::getInstance(context->nbContext);
    if(qaLogPtr)
    {
        if(!qaLogPtr->Initialization(qaLogNetworkConfiguration))
        {
            return 0;
        }
        LOGE("Initialization QA log instance success.");
    }
    */

    //The code will be delete after the sample APP UI done.
    /*
    TaskId taskId = 0;
    PAL_EventTaskQueueAdd(palInstance,
                          (PAL_TaskQueueCallback) CCC_InternalTestQaLog,
                          (void*) context->nbContext,
                          &taskId);*/

    if (NB_ContextInitializeGenericHttpDownloadManager(context->nbContext, 10) != NE_OK)
    {
        //ShutdownCCC();
        LOGE("-----CreateNativeContext: NB_ContextInitializeGenericHttpDownloadManager failed");
        return 0;
    }

    LOGE("-----CreateNativeContext: Succeeded %p", context);
    return (jint)context;
}

static void JNI_DestoryNativeContext(JNIEnv* env, jobject thiz, jint context)
{
    NB_ContextDestroy(reinterpret_cast<NativeLTKContext*>(context)->nbContext);
    PAL_Destroy(reinterpret_cast<NativeLTKContext*>(context)->pal);
}

static void JNI_UpdateToken(JNIEnv* env, jobject thiz, jlong ltkContext, jobject networkOptions)
{
    NativeLTKContext* nativeLTKContext = static_cast<NativeLTKContext*>((void*)ltkContext);
    if(!nativeLTKContext || !(nativeLTKContext->nbContext))
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----JNI_UpdateToken: parameter is wrong.");
        return;
    }

    NB_NetworkConfiguration* networkConfiguration = GetNetworkConfiguration(env, networkOptions, false);
    if(!networkConfiguration)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----JNI_UpdateToken: GetNetworkConfiguration failed.");
        return;
    }

    NetworkConfigTask* task = new NetworkConfigTask(nativeLTKContext->nbContext, networkConfiguration);
    if (!task)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----JNI_UpdateToken: no memory.");
        NB_NetworkConfigurationDestroy(networkConfiguration);
        return;
    }

    PAL_Instance* pal = NB_ContextGetPal(nativeLTKContext->nbContext);
    if (!pal)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----JNI_UpdateToken: pal is null. ");
        NB_NetworkConfigurationDestroy(networkConfiguration);
        delete task;
        return;
    }

    TaskId taskId = 0;
    if (PAL_EventTaskQueueAdd(pal, (PAL_TaskQueueCallback)CCC_ResetNetwork, (void*)task, &taskId) != PAL_Ok)
    {
        PAL_TestLog(PAL_LogBuffer_1, PAL_LogLevelInfo, "-----JNI_UpdateToken: add task fail. ");
        NB_NetworkConfigurationDestroy(networkConfiguration);
        delete task;
        return;
    }
}

static jstring JNI_GetDomainName(JNIEnv* env, jobject thiz)
{
    jstring str = env->NewStringUTF(g_domian.c_str());
    return str;
}

static jint JNI_GetSslTrust(JNIEnv* env, jobject thiz)
{
    return g_sslTrust;
}

static JNINativeMethod gMethods[] =
{
    { "createNativeContext",     "(Ljava/lang/Object;FLjava/lang/String;)I",          (void *) &JNI_CreateNativeContext      },
    { "destroyNativeContext",    "(I)V",                                              (void *) &JNI_DestoryNativeContext     },
    { "createQalog",             "(JLjava/lang/Object;)V",                            (void *) &JNI_CreateQalog              },
    { "setQaLogConfiguration",   "(JIIIIJLjava/lang/String;ILjava/lang/String;)V",    (void *) &JNI_SetQaLogConfiguration    },
    { "startLogging",            "(JZ)V",                                             (void *) &JNI_StartLogging             },
    { "uploadLogFile",           "(J)V",                                              (void *) &JNI_UploadLogFile            },
    { "cancelUpload",            "(J)V",                                              (void *) &JNI_CancelUpload             },
    { "updateToken",             "(JLjava/lang/Object;)V",                            (void *) &JNI_UpdateToken              }
};

static JNINativeMethod gSslTrustMethods[] =
{
    { "GetDomainName",           "()Ljava/lang/String;",                              (void *) &JNI_GetDomainName            },
    { "GetSslTrust",             "()I",                                               (void *) &JNI_GetSslTrust              }
};

int register_nbcontext(JavaVM* vm, JNIEnv* env)
{
    jint result = JNI_ERR;
    if(env == NULL)
    {
        return result;
    }

    jclass nativeNBContextClazz = env->FindClass("com/locationtoolkit/common/jni/NativeNBContext");
    jclass clazz = env->FindClass("com/locationtoolkit/common/jni/NetworkOptions");
    jclass nativeSslFactoryClazz = env->FindClass("com/navbuilder/pal/android/ndk/CustomSSLSocketFactory");

    if(nativeNBContextClazz == NULL)
    {
        goto fail;
    }

    if(clazz == NULL)
    {
        goto fail;
    }

    if(nativeSslFactoryClazz == NULL)
    {
        goto fail;
    }

    fields.nativeNBContextClazz = (jclass)env->NewGlobalRef(nativeNBContextClazz);
    env->DeleteLocalRef(nativeNBContextClazz);
    if (fields.nativeNBContextClazz == NULL)
    {
        goto fail;
    }
    if (env->RegisterNatives(fields.nativeNBContextClazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        goto fail;
    }

    fields.nativeSslFactoryClazz = (jclass)env->NewGlobalRef(nativeSslFactoryClazz);
    env->DeleteLocalRef(nativeSslFactoryClazz);
    if (fields.nativeSslFactoryClazz == NULL)
    {
        goto fail;
    }
    if (env->RegisterNatives(fields.nativeSslFactoryClazz, gSslTrustMethods, sizeof(gSslTrustMethods) / sizeof(gSslTrustMethods[0])) != JNI_OK)
    {
        goto fail;
    }

    fields.onUploadComplete = env->GetMethodID(fields.nativeNBContextClazz, "onUploadComplete", "(Ljava/lang/String;)V");
    if (fields.onUploadComplete == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBContext.onUploadComplete"));
        goto fail;
    }

    fields.onRequestCancelled = env->GetMethodID(fields.nativeNBContextClazz, "onRequestCancelled", "()V");
    if (fields.onRequestCancelled == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBContext.onRequestCancelled"));
        goto fail;
    }

    fields.onRequestError = env->GetMethodID(fields.nativeNBContextClazz, "onRequestError", "(I)V");
    if (fields.onRequestError == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBContext.onRequestError"));
        goto fail;
    }

    fields.onRequestStart = env->GetMethodID(fields.nativeNBContextClazz, "onRequestStart", "()V");
    if (fields.onRequestStart == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBContext.onRequestStart"));
        goto fail;
    }

    fields.onRequestProgress = env->GetMethodID(fields.nativeNBContextClazz, "onRequestProgress", "(I)V");
    if (fields.onRequestProgress == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNBContext.onRequestProgress"));
        goto fail;
    }

    fields.clazz = (jclass)env->NewGlobalRef(clazz);
    env->DeleteLocalRef(clazz);
    if (fields.clazz == NULL)
    {
        goto fail;
    }
    fields.adsAppId = env->GetFieldID(fields.clazz, "adsAppId", "Ljava/lang/String;");
    if(fields.adsAppId == NULL)
    {
        goto fail;
    }
    fields.credential = env->GetFieldID(fields.clazz, "credential", "Ljava/lang/String;");
    if(fields.credential == NULL)
    {
        goto fail;
    }
    fields.device = env->GetFieldID(fields.clazz, "device", "Ljava/lang/String;");
    if(fields.device == NULL)
    {
        goto fail;
    }
    fields.deviceSerialNumber = env->GetFieldID(fields.clazz, "deviceSerialNumber", "Ljava/lang/String;");
    if(fields.deviceSerialNumber == NULL)
    {
        goto fail;
    }
    fields.model = env->GetFieldID(fields.clazz, "model", "Ljava/lang/String;");
    if(fields.model == NULL)
    {
        goto fail;
    }

    fields.domain = env->GetFieldID(fields.clazz, "domain", "Ljava/lang/String;");
    if(fields.domain == NULL)
    {
        goto fail;
    }
    fields.firmwareVersion = env->GetFieldID(fields.clazz, "firmwareVersion", "Ljava/lang/String;");
    if(fields.firmwareVersion == NULL)
    {
        goto fail;
    }
    fields.hostname = env->GetFieldID(fields.clazz, "hostname", "Ljava/lang/String;");
    if(fields.hostname == NULL)
    {
        goto fail;
    }
    fields.language = env->GetFieldID(fields.clazz, "language", "Ljava/lang/String;");
    if(fields.language == NULL)
    {
        goto fail;
    }
    fields.mdn = env->GetFieldID(fields.clazz, "mdn", "Ljava/lang/String;");
    if(fields.mdn == NULL)
    {
        goto fail;
    }
    fields.min = env->GetFieldID(fields.clazz, "min", "Ljava/lang/String;");
    if(fields.min == NULL)
    {
        goto fail;
    }
    fields.pin = env->GetFieldID(fields.clazz, "pin", "Ljava/lang/String;");
    if(fields.pin == NULL)
    {
        goto fail;
    }
    fields.countryCode = env->GetFieldID(fields.clazz, "countryCode", "Ljava/lang/String;");
    if(fields.countryCode == NULL)
    {
        goto fail;
    }
    fields.carrier = env->GetFieldID(fields.clazz, "carrier", "Ljava/lang/String;");
    if(fields.carrier == NULL)
    {
        goto fail;
    }

    fields.deviceUniqueIdentifier = env->GetFieldID(fields.clazz, "deviceUniqueIdentifier", "Ljava/lang/String;");
    if(fields.deviceUniqueIdentifier == NULL)
    {
        goto fail;
    }

    fields.pushMessageGuid = env->GetFieldID(fields.clazz, "pushMessageGuid", "Ljava/lang/String;");
    if(fields.pushMessageGuid == NULL)
    {
        goto fail;
    }

    fields.manufacturer = env->GetFieldID(fields.clazz, "manufacturer", "Ljava/lang/String;");
    if(fields.manufacturer == NULL)
    {
        goto fail;
    }

    fields.platformId = env->GetFieldID(fields.clazz, "platformId", "I");
    if(fields.platformId == NULL)
    {
        goto fail;
    }
    fields.priceType = env->GetFieldID(fields.clazz, "priceType", "I");
    if(fields.priceType == NULL)
    {
        goto fail;
    }
    fields.port = env->GetFieldID(fields.clazz, "port", "I");
    if(fields.port == NULL)
    {
        goto fail;
    }
    fields.protocol = env->GetFieldID(fields.clazz, "protocol", "I");
    if(fields.protocol == NULL)
    {
        goto fail;
    }
    fields.mcc = env->GetFieldID(fields.clazz, "mcc", "I");
    if(fields.mcc == NULL)
    {
        goto fail;
    }
    fields.mnc = env->GetFieldID(fields.clazz, "mnc", "I");
    if(fields.mnc == NULL)
    {
        goto fail;
    }
    fields.isEmulator = env->GetFieldID(fields.clazz, "isEmulator", "I");
    if(fields.isEmulator == NULL)
    {
        goto fail;
    }
    fields.tps = env->GetFieldID(fields.clazz, "tps", "[B");
    if(fields.tps == NULL)
    {
        goto fail;
    }
    fields.guid = env->GetFieldID(fields.clazz, "guid", "[B");
    if(fields.guid == NULL)
    {
        goto fail;
    }

    fields.sslTrust = env->GetFieldID(fields.clazz, "sslTrust", "Z");
    if(fields.sslTrust == NULL)
    {
        goto fail;
    }

    fields.pushGuidEncode = env->GetFieldID(fields.clazz, "pushGuidEncode", "I");
    if(fields.pushGuidEncode == NULL)
    {
        goto fail;
    }

    cachedVM = vm;

    result = JNI_OK;

fail:
    if (result != JNI_OK)
    {
        __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, "Failed to register nbcontext!");
    }

    return result;
}

int unregister_nbcontext(JNIEnv* env)
{
   if(env == NULL)
   {
       return JNI_ERR;
   }
   if (fields.clazz != NULL)
   {
       env->DeleteGlobalRef(fields.clazz);
       fields.clazz = NULL;
   }
   return JNI_OK;
}
