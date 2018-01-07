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
 @file     contentmanagerandroid.cpp
 */

/*! @{ */

#include "contentmanagerandroid.h"
#include <assert.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include "paldebuglog.h"
#include "JNIUtility.h"
#include "nativeltkcontext.h"
#include "OnboardContentManager.h"

#define  LOG_TAG    "ContentManager"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace nbcommon;

#define  MAX_PATH_LENGTH 512

const uint32 TEST_MAX_PATH_LENGTH = 256;

const uint32 TEST_SCREEN_WIDTH = 780;
const uint32 TEST_SCREEN_HEIGHT = 585;

const string TEST_COUNTRY  = "USA";
const string TEST_NBGM_VERSION = "1";


struct fields_t
{
    jclass clazz;
	jobject contentManagerObj;
    jfieldID contentManager;
    jmethodID OnRegionsAvailableMethodID;
    jmethodID OnRegionalErrorMethodID;
    jmethodID OnDownloadErrorMethodID;
    jmethodID OnStatusChangedMethodID;
    jmethodID OnProgressMethodID;
};

static fields_t fields;
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

class JniRegionalMapDataListener:
    public RegionalMapDataListener
{
public:
    JniRegionalMapDataListener() {}

    void OnError(NB_Error error);
	void OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions);
};

void
JniRegionalMapDataListener::OnError(NB_Error error)
{
    LOGI("JniRegionalMapDataListener - OnError");
	JNIEnv* env = getJNIEnv();
    if(env)
    {
        env->CallVoidMethod(fields.contentManagerObj, fields.OnRegionalErrorMethodID, static_cast<jint>(error));
    }
}

void
JniRegionalMapDataListener::OnAvailableRegions(const std::vector<RegionalInformationPtr>& regions)
{
    //todo: call java interface
    LOGI("JniRegionalMapDataListener - OnAvailableRegions");
	JNIEnv* env = getJNIEnv();
    if(env)
    {
        jobjectArray mapRegions = 0;
        jint length = regions.size();

        jclass objClass = (env)->FindClass("java/lang/Object");
        mapRegions = (env)->NewObjectArray(length, objClass, 0);

        jclass objectClass = (env)->FindClass("com/locationtoolkit/contentui/RegionalInformation");

        jfieldID regionalName = (env)->GetFieldID(objectClass,"m_regionalName","Ljava/lang/String;");
        jfieldID version = (env)->GetFieldID(objectClass,"m_version","Ljava/lang/String;");
        jfieldID regionalID = (env)->GetFieldID(objectClass,"m_regionalID","Ljava/lang/String;");

        jfieldID downloadSize = (env)->GetFieldID(objectClass,"m_downloadSize","I");
        jfieldID uncompressedSize = (env)->GetFieldID(objectClass,"m_uncompressedSize","I");
        jfieldID downloadedProgress = (env)->GetFieldID(objectClass,"m_downloadedProgress","I");
        jfieldID downloadedSize = (env)->GetFieldID(objectClass,"m_downloadedSize","I");

        jfieldID updateAvailable = (env)->GetFieldID(objectClass,"m_updateAvailable","Z");
        jfieldID isInUse = (env)->GetFieldID(objectClass,"m_isInUse","Z");

        jfieldID downloadStatus = (env)->GetFieldID(objectClass,"m_downloadStatus","I");
        jfieldID errorCode = (env)->GetFieldID(objectClass,"m_errorCode","I");

        for(int i = 0; i < length; i++)
        {
		    jobject _obj = (env)->AllocObject(objectClass);
		    LOGI("JniRegionalMapDataListener - OnAvailableRegions input region i=%d,length=%d, regionID=%s, uncompressedsize=%lu, downloadSize=%lu, downloadedSize=%lu, downloadedProgress=%u", i, length, regions[i]->m_regionalID.c_str(), regions[i]->m_uncompressedSize, regions[i]->m_downloadSize, regions[i]->m_downloadedSize, regions[i]->m_downloadedProgress);
            jstring jregionalName = stoJstring(env, regions[i]->m_regionalName.c_str());
            jstring jversion = stoJstring(env, regions[i]->m_version.c_str());
            jstring jregionalID = stoJstring(env, regions[i]->m_regionalID.c_str());

            jint jdownloadSize = (jint)regions[i]->m_downloadSize;
            jint juncompressedSize = (jint)regions[i]->m_uncompressedSize;
            jint jdownloadedProgress = (jint)regions[i]->m_downloadedProgress;
            jint jdownloadedSize = (jint)regions[i]->m_downloadedSize;

            jboolean jupdateAvailable = (jboolean)regions[i]->m_updateAvailable;
            jboolean jisInUse = (jboolean)regions[i]->m_isInUse;

            jint jdownloadStatus = (jint)regions[i]->m_downloadStatus;
            jint jerrorCode = (jint)regions[i]->m_errorCode;

            (env)->SetObjectField(_obj, regionalName, jregionalName);
            (env)->SetObjectField(_obj, version, jversion);

            (env)->SetObjectField(_obj, regionalID, jregionalID);
            (env)->SetIntField(_obj, downloadSize, jdownloadSize);
            (env)->SetIntField(_obj, uncompressedSize, juncompressedSize);
            (env)->SetIntField(_obj, downloadedProgress, jdownloadedProgress);
            (env)->SetIntField(_obj, downloadedSize, jdownloadedSize);

            (env)->SetBooleanField(_obj, updateAvailable, jupdateAvailable);
            (env)->SetBooleanField(_obj, isInUse, jisInUse);

            (env)->SetIntField(_obj, downloadStatus, jdownloadStatus);
            (env)->SetIntField(_obj, errorCode, jerrorCode);

            (env)->SetObjectArrayElement(mapRegions, i, _obj);
        }

        LOGI("JniRegionalMapDataListener - OnAvailableRegions Call to Java");
        env->CallVoidMethod(fields.contentManagerObj, fields.OnRegionsAvailableMethodID, mapRegions);
    }
}

class JniDownloadStatusListener:
    public DownloadStatusListener
{
public:
    JniDownloadStatusListener() {}

    void OnStatusChanged(const std::string& regionalId, NB_EnhancedContentDataType dataType, RegionDownloadStatus newStatus);
    void OnError(const std::string& regionalId, NB_Error error);
    void OnProgress(const std::string& regionalId, int percentage);
};

void
JniDownloadStatusListener::OnError(const std::string& regionalId, NB_Error error)
{
    LOGI("JniDownloadStatusListener::OnError");
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        LOGI("JniDownloadStatusListener - OnError Call to Java");
		jstring Id = env->NewStringUTF(regionalId.c_str());
        env->CallVoidMethod(fields.contentManagerObj, fields.OnDownloadErrorMethodID, Id, static_cast<jint>(error));
    }
}

void
JniDownloadStatusListener::OnStatusChanged(const std::string& regionalId, NB_EnhancedContentDataType dataType, RegionDownloadStatus newStatus)
{
    LOGI("JniDownloadStatusListener::OnStatusChanged");
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        LOGI("JniDownloadStatusListener - OnStatusChanged Call to Java");
		jstring Id = env->NewStringUTF(regionalId.c_str());
        env->CallVoidMethod(fields.contentManagerObj, fields.OnStatusChangedMethodID, Id, static_cast<jint>(dataType), static_cast<jint>(newStatus));
    }
}

void
JniDownloadStatusListener::OnProgress(const std::string& regionalId, int percentage)
{
    LOGI("JniDownloadStatusListener::OnProgress-%d", percentage);
    JNIEnv* env = getJNIEnv();
    if(env)
    {
        LOGI("JniDownloadStatusListener - OnProgress Call to Java");
		jstring Id = env->NewStringUTF(regionalId.c_str());
        env->CallVoidMethod(fields.contentManagerObj, fields.OnProgressMethodID, Id, static_cast<jint>(percentage));
    }
}

static void JNI_CreateContentManager(JNIEnv* env, jobject thiz, jint ltkContext, jstring mapDataStoragePath, jstring persistentMetadataPath)
{
    LOGI("JNI_CreateContentManager");
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(ltkContext);
    if(nativenbcontext)
    {
		std::string mapdataPath = NewStdString(env, mapDataStoragePath);
	    std::string metadataPath = NewStdString(env, persistentMetadataPath);
        std::string clientGuid("1234567890");
        std::string language("EN-US");
		LOGI("JNI_CreateContentManager mapdataPath=%s,metadataPath=%s", mapdataPath.c_str(), metadataPath.c_str());

        OnboardContentManagerConfigPtr config(new OnboardContentManagerConfig(
                                                    TEST_SCREEN_WIDTH,
                                                    TEST_SCREEN_HEIGHT,
                                                    TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT,
                                                    TEST_COUNTRY,
                                                    language,
                                                    TEST_NBGM_VERSION,
                                                    mapdataPath,
                                                    metadataPath,
                                                    clientGuid,
                                                    NB_ECDT_MAP_ALL));

		ContentManager* contentManager = new ContentManager();
        contentManager->onboardContentManager = OnboardContentManager::CreateOnboardContentManager(nativenbcontext->nbContext,config);

		contentManager->jniRegionalMapDataListener = new JniRegionalMapDataListener();
		contentManager->jniDownloadStatusListener = new JniDownloadStatusListener();

        contentManager->onboardContentManager->RegisterDownloadStatusListener(contentManager->jniDownloadStatusListener);
        contentManager->onboardContentManager->RegisterRegionalMapDataListener(contentManager->jniRegionalMapDataListener);

		fields.contentManagerObj = env->NewGlobalRef(thiz);
		env->SetIntField(thiz, fields.contentManager, (int)contentManager);
		LOGI("JNI_CreateContentManager finished");
    }
}

static void JNI_DestroyContentManager(JNIEnv* env, jobject thiz)
{
    LOGI("JNI_DestroyContentManager");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
        contentManager->onboardContentManager->UnregisterRegionalMapDataListener(contentManager->jniRegionalMapDataListener);
        contentManager->onboardContentManager->UnregisterDownloadStatusListener(contentManager->jniDownloadStatusListener);
        delete contentManager;
    }
}

static jint JNI_CheckAvaliableRegions(JNIEnv* env, jobject thiz)
{
    LOGI("JNI_CheckAvaliableRegions");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
	    if(contentManager->onboardContentManager)
	    {
		    LOGI("JNI_CheckAvaliableRegions call CCC content manager check avaliable regions");
            return (jint)contentManager->onboardContentManager->CheckAvaliableRegions();
		}else{
		    return 0;
		}
    }
    return 0;
}

static jint JNI_RequestRegionalData(JNIEnv* env, jobject thiz, jstring regionalId)
{
    LOGI("JNI_RequestRegionalData");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
		std::string id = NewStdString(env, regionalId);
		LOGI("JNI_RequestRegionalData-regionID=%s", id.c_str());
        return (jint)contentManager->onboardContentManager->RequestRegionalData(id);
    }
    return 0;
}

static jint JNI_RemoveRegionalData(JNIEnv* env, jobject thiz, jstring regionalId)
{
    LOGI("JNI_RemoveRegionalData");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
	    std::string id = NewStdString(env, regionalId);
		LOGI("JNI_RemoveRegionalData-regionID=%s", id.c_str());
        return (jint)contentManager->onboardContentManager->RemoveRegionalData(id);
    }
    return 0;
}

static jint JNI_PauseDownload(JNIEnv* env, jobject thiz, jstring regionalId, jint downloadType)
{
    LOGI("JNI_PauseDownload");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
	    std::string id = NewStdString(env, regionalId);
		LOGI("JNI_PauseDownload-regionID=%s", id.c_str());
        return (jint)contentManager->onboardContentManager->PauseDownload(id);
    }
    return 0;
}

static jint JNI_ResumeDownload(JNIEnv* env, jobject thiz, jstring regionalId, jint downloadType)
{
    LOGI("JNI_ResumeDownload");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
	    std::string id = NewStdString(env, regionalId);
		LOGI("JNI_ResumeDownload-regionID=%s", id.c_str());
        return (jint)contentManager->onboardContentManager->ResumeDownload(id);
    }
    return 0;
}

static jint JNI_CancelDownload(JNIEnv* env, jobject thiz, jstring regionalId)
{
    LOGI("JNI_CancelDownload");
    ContentManager* contentManager = reinterpret_cast<ContentManager*>(env->GetIntField(thiz, fields.contentManager));
    if(contentManager)
    {
	    std::string id = NewStdString(env, regionalId);
		LOGI("JNI_CancelDownload-regionID=%s", id.c_str());
        return (jint)contentManager->onboardContentManager->CancelDownload(id);
    }
    return 0;
}

static JNINativeMethod gMethods[] =
{
    { "CreateContentManager",     "(ILjava/lang/String;Ljava/lang/String;)V",                     (void *) &JNI_CreateContentManager      },
    { "DestroyContentManager",    "()V",                                                          (void *) &JNI_DestroyContentManager         },
    { "CheckAvaliableRegions",    "()I",                                                          (void *) &JNI_CheckAvaliableRegions     },
    { "RequestRegionalData",      "(Ljava/lang/String;)I",                                        (void *) &JNI_RequestRegionalData       },
    { "RemoveRegionalData",       "(Ljava/lang/String;)I",                                        (void *) &JNI_RemoveRegionalData        },
    { "PauseDownload",            "(Ljava/lang/String;I)I",                                       (void *) &JNI_PauseDownload             },
    { "ResumeDownload",           "(Ljava/lang/String;I)I",                                       (void *) &JNI_ResumeDownload            },
    { "CancelDownload",           "(Ljava/lang/String;)I",                                        (void *) &JNI_CancelDownload            }
};

int register_contentmanager(JavaVM* vm, JNIEnv* env)
{
    LOGI("register_contentmanager");
    jint result = JNI_ERR;
    jclass clazz = env->FindClass("com/locationtoolkit/contentui/NativeContentManager");
    if (clazz != NULL)
    {
        fields.clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/locationtoolkit/contentui/NativeContentManager"));
            goto fail;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/locationtoolkit/contentui/NativeContentManager"));
        goto fail;
    }

    fields.contentManager = env->GetFieldID(fields.clazz, "mNativeContext", "I");

    if (env->RegisterNatives(fields.clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        goto fail;
    }

    fields.OnRegionsAvailableMethodID = env->GetMethodID(fields.clazz, "OnRegionsAvailable", "([Lcom/locationtoolkit/contentui/RegionalInformation;)V");
    if (fields.OnRegionsAvailableMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeContentManager.OnRegionsAvailableMethodID"));
        goto fail;
    }

    fields.OnRegionalErrorMethodID = env->GetMethodID(fields.clazz, "OnRegionalError", "(I)V");
    if (fields.OnRegionalErrorMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeContentManager.OnRegionalErrorMethodID"));
        goto fail;
    }

    fields.OnDownloadErrorMethodID = env->GetMethodID(fields.clazz, "OnDownloadError", "(Ljava/lang/String;I)V");
    if (fields.OnDownloadErrorMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeContentManager.OnDownloadErrorMethodID"));
        goto fail;
    }
    fields.OnStatusChangedMethodID = env->GetMethodID(fields.clazz, "OnStatusChanged", "(Ljava/lang/String;II)V");
    if (fields.OnStatusChangedMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeContentManager.OnStatusChangedMethodID"));
        goto fail;
    }
    fields.OnProgressMethodID = env->GetMethodID(fields.clazz, "OnProgress", "(Ljava/lang/String;I)V");
    if (fields.OnProgressMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeContentManager.OnProgressMethodID"));
        goto fail;
    }

    cachedVM = vm;
    result = JNI_OK;

fail:
    return result;
}

int unregister_contentmanager(JNIEnv* env)
{
    return JNI_OK;
}