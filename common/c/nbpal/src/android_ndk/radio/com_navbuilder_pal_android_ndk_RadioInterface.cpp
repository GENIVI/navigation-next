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
#include <cstring>
#include "palradio.h"
#include "paldebuglog.h"

struct fields_t {
    jclass clazz;
    jobject radioActivity;
    jmethodID setSystemMode;
    jmethodID setGSMNetworkInfo;
    jmethodID setCDMANetworkInfo;
    jmethodID setWifiNetworkInfo;
    jmethodID setAllWifiNetworkInfo;
    PAL_RadioContext* radioContext;
};
static fields_t fields;

extern JNIEnv* getJNIEnv();

static void JniRadioSystemModeCallback(PAL_Error status, PAL_RadioSystemMode systemMode, void* userData);
static void JniRadioGSMNetworkInfoCallback(PAL_Error status, PAL_RadioGSMInfo* gsmInfo, void* userData);
static void JniRadioCDMANetworkInfoCallback(PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo, void* userData);
static void JniRadioWifiNetworkInfoCallback(PAL_Error status, PAL_RadioWifiInfo* wifiInfo, void* userData);
static void JniRadioAllWifiNetworksInfoCallback(PAL_Error status, PAL_RadioCompleteWifiInfo** allWifiInfo, unsigned long wifisInfoCount, void* userData);

static void jni_CreateContext(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_CreateContext"));
	fields.radioActivity = env->NewGlobalRef(thiz);
	int fakePalPtr = 0;

	PAL_RadioContextCreate((PAL_Instance*)&fakePalPtr, PRST_All, &(fields.radioContext));

	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG,
			("jni_CreateContext radioContext=%d", fields.radioContext));
}

static void jni_GetSystemMode(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_GetSystemMode"));

	PAL_RadioGetSystemMode(fields.radioContext, JniRadioSystemModeCallback, NULL);
}

static void jni_GetGSMNetworkInfo(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_GetGSMNetworkInfo"));
	PAL_RadioGetGSMNetworkInfo(fields.radioContext, JniRadioGSMNetworkInfoCallback, NULL);
}

static void jni_GetCDMANetworkInfo(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_GetCDMANetworkInfo"));
	PAL_RadioGetCDMANetworkInfo(fields.radioContext, JniRadioCDMANetworkInfoCallback, NULL);
}

static void jni_GetWifiNetworkInfo(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_GetWifiNetworkInfo"));
	PAL_RadioGetWifiNetworkInfo(fields.radioContext, JniRadioWifiNetworkInfoCallback, NULL);
}

static void jni_GetAllWifiNetworksInfo(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_GetAllWifiNetworksInfo"));
	PAL_RadioCancelAllWifiNetworksInfo(fields.radioContext, JniRadioAllWifiNetworksInfoCallback);
	PAL_RadioGetAllWifiNetworksInfo(fields.radioContext, JniRadioAllWifiNetworksInfoCallback, NULL);
}

static void jni_DestroyContext(JNIEnv* env, jobject thiz)
{
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("jni_DestroyContext"));
	PAL_RadioCancelAllWifiNetworksInfo(fields.radioContext, JniRadioAllWifiNetworksInfoCallback);
	PAL_RadioContextDestroy(fields.radioContext);
	fields.radioContext = NULL;
}

//CALLBACKS
static void JniRadioSystemModeCallback(PAL_Error status, PAL_RadioSystemMode systemMode, void* userData){
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("JniRadioSystemModeCallback"));

	JNIEnv* env = getJNIEnv();
  	env->CallVoidMethod(fields.radioActivity,
  			fields.setSystemMode, (int)systemMode);
}

static void JniRadioGSMNetworkInfoCallback(PAL_Error status, PAL_RadioGSMInfo* gsmInfo, void* userData){
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("JniRadioGSMNetworkInfoCallback"));

	JNIEnv* env = getJNIEnv();
	env->CallVoidMethod(fields.radioActivity,
						fields.setGSMNetworkInfo,
						(int)gsmInfo->cellId,
						(int)gsmInfo->locationAreaCode,
						(int)gsmInfo->mobileCountryCode,
						(int)gsmInfo->mobileNetworkCode,
						(int)gsmInfo->signalStrength);
}

static void JniRadioCDMANetworkInfoCallback(PAL_Error status, PAL_RadioCDMAInfo* cdmaInfo, void* userData){
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("JniRadioCDMANetworkInfoCallback"));

	JNIEnv* env = getJNIEnv();
	env->CallVoidMethod(fields.radioActivity,
						fields.setCDMANetworkInfo,
						(int)cdmaInfo->cellId,
						(int)cdmaInfo->systemId,
						(int)cdmaInfo->networkId,
						(int)cdmaInfo->signalStrength);
}

static void JniRadioWifiNetworkInfoCallback(PAL_Error status, PAL_RadioWifiInfo* wifiInfo, void* userData){
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("JniRadioWifiNetworkInfoCallback"));

	JNIEnv* env = getJNIEnv();
	env->CallVoidMethod(fields.radioActivity,
						fields.setWifiNetworkInfo,
						env->NewStringUTF((const char*)wifiInfo->macAddress),
						(int)wifiInfo->signalStrength);
}

static void JniRadioAllWifiNetworksInfoCallback(PAL_Error status, PAL_RadioCompleteWifiInfo** allWifiInfo, unsigned long wifisInfoCount, void* userData){
	DEBUGLOG(LOG_SS_JNI, LOG_SEV_DEBUG, ("JniRadioAllWifiNetworksInfoCallback"));

	JNIEnv* env = getJNIEnv();
	for(int i=0;i<wifisInfoCount;i++){
		env->CallVoidMethod(fields.radioActivity,
							fields.setAllWifiNetworkInfo,
							env->NewStringUTF((const char*)(*allWifiInfo)[i].macAddress),
							env->NewStringUTF((const char*)(*allWifiInfo)[i].ssid),
							(int)(*allWifiInfo)[i].frequency,
							(int)(*allWifiInfo)[i].signalStrength,
							(bool)(*allWifiInfo)[i].isPrivacy,
							(int)i,
							(int)wifisInfoCount);
	}
}


static JNINativeMethod gMethods[] = {
    { "nativeCreateContext", "()V", (void *) &jni_CreateContext},
    { "nativeGetSystemMode", "()V", (void *) &jni_GetSystemMode},
    { "nativeGetGSMNetworkInfo", "()V", (void *) &jni_GetGSMNetworkInfo},
    { "nativeCDMANetworkInfo", "()V", (void *) &jni_GetCDMANetworkInfo},
    { "nativeWififNetworkInfo", "()V", (void *) &jni_GetWifiNetworkInfo},
    { "nativeAllWififNetworkInfo", "()V", (void *) &jni_GetAllWifiNetworksInfo},
    { "nativeDestroyContext", "()V", (void *) &jni_DestroyContext},
};

static void release_fields(JNIEnv* env)
{
    if (fields.clazz)
    {
        env->DeleteGlobalRef(fields.clazz);
        fields.clazz = NULL;
    }
}

int register_com_navbuilder_pal_ndk_radiointerface(JNIEnv *env){
    uint32 errorLine = 0;

    jclass clazz = env->FindClass("com/tcs/jcc/testharness/RadioActivity");
    if (clazz != NULL)
    {
        fields.clazz = (jclass) env->NewGlobalRef(clazz);
        if (fields.clazz == NULL)
        {
            errorLine = __LINE__;
            goto HandleError;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.setSystemMode = env->GetMethodID(fields.clazz, "SetSystemMode", "(I)V");
    if (fields.setSystemMode == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.setGSMNetworkInfo = env->GetMethodID(fields.clazz, "SetGSMNetworkInfo", "(IIIII)V");
    if (fields.setGSMNetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.setCDMANetworkInfo = env->GetMethodID(fields.clazz, "SetCDMANetworkInfo", "(IIII)V");
    if (fields.setCDMANetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.setWifiNetworkInfo = env->GetMethodID(fields.clazz, "SetWifiNetworkInfo", "(Ljava/lang/String;I)V");
    if (fields.setWifiNetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.setAllWifiNetworkInfo = env->GetMethodID(fields.clazz, "SetAllWifiNetworkInfo", "(Ljava/lang/String;Ljava/lang/String;IIZII)V");
    if (fields.setAllWifiNetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    if (env->RegisterNatives(fields.clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    return JNI_OK;

HandleError:
    release_fields(env);
    return JNI_ERR;
}

int unregister_com_navbuilder_pal_ndk_radiointerface(JNIEnv* env)
{
    release_fields(env);
    return JNI_OK;
}
