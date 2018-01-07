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

/*--------------------------------------------------------------------------

(C) Copyright 2010 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "palradio.h"
#include "palstdlib.h"
#include <vector>

extern JNIEnv* getJNIEnv();

struct fields_t {
    jclass clazz;
    jmethodID init;
    jmethodID radioGetSystemMode;
    jmethodID radioGetGSMNetworkInfo;
    jmethodID radioGetCDMANetworkInfo;
    jmethodID radioGetWifiNetworkInfo;
    jmethodID radioGetAllWifiNetworksInfo;
    jmethodID radioCancelAllWifiNetworksInfo;
    jfieldID  radioContext;
};
static fields_t fields;

typedef struct AllWifiNetworksInfoCallback
{
	PAL_RadioAllWifiNetworksInfoCallback callback;
	void* userData;

}AllWifiNetworksInfoCallback;

typedef struct PAL_RadioContext
{
	jobject palRadioAndroidClass;
	std::vector<AllWifiNetworksInfoCallback*> *allWifiCallbacks;
	PAL_RadioCompleteWifiInfo* allWifiInfoResults;
	int allWifiInfoResultsCount;
} PAL_RadioContext;

PAL_DEF PAL_Error PAL_RadioContextCreate(PAL_Instance* pal, PAL_RadioServiceType type, PAL_RadioContext** radioContext)
{

    if (pal == NULL || radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    JNIEnv* env = getJNIEnv();
    jobject palRadio = env->NewObject(fields.clazz, fields.init);

    if(palRadio == NULL){
    	return PAL_ErrBadParam;
    }

    *radioContext = NULL;
    PAL_RadioContext* context = new PAL_RadioContext;
    if (context != NULL)
    {
    	context->palRadioAndroidClass = env->NewGlobalRef(palRadio);

    	if(context->palRadioAndroidClass == NULL){
    		return PAL_ErrBadParam;
    	}
    	env->DeleteLocalRef(palRadio);
    }

    env->SetIntField(context->palRadioAndroidClass, fields.radioContext, (int)context);

    context->allWifiCallbacks = new std::vector<AllWifiNetworksInfoCallback*>;
    context->allWifiInfoResults = NULL;

    *radioContext = context;

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioContextDestroy(PAL_RadioContext* radioContext)
{
    if (radioContext != NULL)
    {
    	JNIEnv* env = getJNIEnv();
		env->DeleteGlobalRef(radioContext->palRadioAndroidClass);

        delete radioContext;
    }

    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioGetSystemMode(PAL_RadioContext* radioContext, PAL_RadioSystemModeCallback callback, void* userData)
{
	if(radioContext == NULL){
		return PAL_ErrBadParam;
	}

    JNIEnv* env = getJNIEnv();
  	env->CallVoidMethod(radioContext->palRadioAndroidClass, fields.radioGetSystemMode, callback, userData);

	return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioGetGSMNetworkInfo(PAL_RadioContext* radioContext, PAL_RadioGSMNetworkInfoCallback callback, void* userData)
{
	if(radioContext == NULL){
		return PAL_ErrBadParam;
	}

    JNIEnv* env = getJNIEnv();
   	env->CallVoidMethod(radioContext->palRadioAndroidClass, fields.radioGetGSMNetworkInfo, callback, userData);

	return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioGetCDMANetworkInfo(PAL_RadioContext* radioContext, PAL_RadioCDMANetworkInfoCallback callback, void* userData)
{
	if(radioContext == NULL){
		return PAL_ErrBadParam;
	}

    JNIEnv* env = getJNIEnv();
   	env->CallVoidMethod(radioContext->palRadioAndroidClass, fields.radioGetCDMANetworkInfo, callback, userData);

	return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioGetWifiNetworkInfo(PAL_RadioContext* radioContext, PAL_RadioWifiNetworkInfoCallback callback, void* userData)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    JNIEnv* env = getJNIEnv();
   	env->CallVoidMethod(radioContext->palRadioAndroidClass, fields.radioGetWifiNetworkInfo, callback, userData);

	return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioGetAllWifiNetworksInfo(PAL_RadioContext* radioContext, PAL_RadioAllWifiNetworksInfoCallback callback, void* userData)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    if(callback != NULL){
        std::vector<AllWifiNetworksInfoCallback*> *callbacks  = radioContext->allWifiCallbacks;
        std::vector<AllWifiNetworksInfoCallback*>::iterator current;
        bool isCallbackPresent = false;

        for (current = callbacks->begin(); current != callbacks->end(); current++)
        {
        	AllWifiNetworksInfoCallback *listener = *current;
            if (listener->callback == callback)
            {
            	isCallbackPresent = true;
            	break;
            }
        }

        if(isCallbackPresent == false){
			AllWifiNetworksInfoCallback* wififCallback = (AllWifiNetworksInfoCallback*)malloc(sizeof(AllWifiNetworksInfoCallback));
			wififCallback->callback = callback;
			wififCallback->userData = userData;

			//Lock
			radioContext->allWifiCallbacks->push_back(wififCallback);
			//Unlock
        }
    }

    JNIEnv* env = getJNIEnv();
	env->CallVoidMethod(radioContext->palRadioAndroidClass, fields.radioGetAllWifiNetworksInfo);

	return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioCancelWifiNetworkInfo(PAL_RadioContext* radioContext)
{
    return PAL_Ok;
}

PAL_DEF PAL_Error PAL_RadioCancelAllWifiNetworksInfo(PAL_RadioContext* radioContext,
                                                     PAL_RadioAllWifiNetworksInfoCallback callback)
{
    if (radioContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    std::vector<AllWifiNetworksInfoCallback*> *callbacks  = radioContext->allWifiCallbacks;
    std::vector<AllWifiNetworksInfoCallback*>::iterator current;

    for (current = callbacks->begin(); current != callbacks->end(); current++)
    {
    	AllWifiNetworksInfoCallback *listener = *current;
        if (listener->callback == callback)
        {
        	//Lock
            current = callbacks->erase(current);
            //Unlock
            free((void*)listener);
            break;
        }
    }


    if(radioContext->allWifiCallbacks->size() == 0){
		JNIEnv* env = getJNIEnv();
		env->CallVoidMethod(radioContext->palRadioAndroidClass, fields.radioCancelAllWifiNetworksInfo);
    }

    return PAL_Ok;
}

static void jni_GSMNetworkInfoCallback(JNIEnv* env, jobject thiz, jlong mobileCountryCode, jlong mobileNetworkCode,
        jlong locationAreaCode, jlong cellId, jint signalStrength, jint callbackPtr, jint userDataPtr)
{
	PAL_RadioGSMNetworkInfoCallback callback = (PAL_RadioGSMNetworkInfoCallback)callbackPtr;
	PAL_RadioGSMInfo gsmInfo;
    nsl_memset(&gsmInfo, 0, sizeof(PAL_RadioGSMInfo));

    gsmInfo.cellId = cellId;
    gsmInfo.locationAreaCode = locationAreaCode;
    gsmInfo.mobileCountryCode = mobileCountryCode;
    gsmInfo.mobileNetworkCode = mobileNetworkCode;
    gsmInfo.signalStrength = signalStrength;

    callback(PAL_Ok, &gsmInfo, (void*)userDataPtr);
}

static void jni_CDNAMNetworkInfoCallback(JNIEnv* env, jobject thiz, jlong systemId, jlong networkId,
        jlong cellId, jint signalStrength, jint callbackPtr, jint userDataPtr)
{
	PAL_RadioCDMANetworkInfoCallback callback = (PAL_RadioCDMANetworkInfoCallback)callbackPtr;
	PAL_RadioCDMAInfo cdmaInfo;
    nsl_memset(&cdmaInfo, 0, sizeof(PAL_RadioCDMAInfo));

    cdmaInfo.cellId = cellId;
    cdmaInfo.systemId = systemId;
    cdmaInfo.networkId = networkId;
    cdmaInfo.signalStrength = signalStrength;

    callback(PAL_Ok, &cdmaInfo, (void*)userDataPtr);
}

static void jni_SystemModeCallback(JNIEnv* env, jobject thiz, jint phoneType, jint callbackPtr, jint userDataPtr)
{
	PAL_RadioSystemModeCallback callback = (PAL_RadioSystemModeCallback)callbackPtr;
	PAL_RadioSystemMode systemMode;
	nsl_memset(&systemMode, 0, sizeof(PAL_RadioSystemMode));

	switch(phoneType){
		case 1://PHONE_TYPE_GSM
			systemMode = PRSM_GSM;
			break;
		case 2://PHONE_TYPE_CDMA
			systemMode = PRSM_CDMA;
			break;
		default://PHONE_TYPE_NONE - 0, PHONE_TYPE_SIP - 3
			systemMode = PRSM_NoService;
			break;
	}

    callback(PAL_Ok, systemMode, (void*)userDataPtr);
}

static void jni_WifiNetworkInfoCallback(JNIEnv* env, jobject thiz, jstring macAddress, jint signalStrength, jint callbackPtr, jint userDataPtr)
{
	PAL_RadioWifiNetworkInfoCallback callback = (PAL_RadioWifiNetworkInfoCallback)callbackPtr;
	PAL_RadioWifiInfo wifiInfo;
	nsl_memset(&wifiInfo, 0, sizeof(PAL_RadioWifiInfo));

	const char *mac = env->GetStringUTFChars(macAddress, NULL);

	wifiInfo.signalStrength = signalStrength;
	wifiInfo.macAddress = (unsigned char*)mac;

    callback(PAL_Ok, &wifiInfo, (void*)userDataPtr);

    env->ReleaseStringUTFChars(macAddress, mac);
}

static void jni_AllWifiNetworkInfoCallback(JNIEnv* env, jobject thiz, jstring macAddress, jstring ssid, jboolean privacy,jint frequency, jint signalStrength, jint resultNum, jint resultCount)
{
	PAL_RadioContext* context = (PAL_RadioContext*)env->GetIntField(thiz , fields.radioContext);
	if(context == NULL){
		return;
	}

	if(context->allWifiInfoResults == NULL && resultCount > 0){
		context->allWifiInfoResults = new PAL_RadioCompleteWifiInfo[resultCount];
		context->allWifiInfoResultsCount = resultCount;
	}

	const char *mac = env->GetStringUTFChars(macAddress, NULL);
	int macLen = nsl_strlen(mac);
	char *macBuf = (char*)malloc(macLen+1);
	nsl_memset(macBuf, 0, macLen+1);
	nsl_memcpy(macBuf,mac,macLen);

	const char *ssidstr = env->GetStringUTFChars(ssid, NULL);
	int ssidLen = nsl_strlen(ssidstr);
	char *ssidBuf = (char*)malloc(ssidLen+1);
	nsl_memset(ssidBuf, 0, ssidLen+1);
	nsl_memcpy(ssidBuf,ssidstr,ssidLen);

	context->allWifiInfoResults[resultNum].macAddress = (unsigned char*)macBuf;
	context->allWifiInfoResults[resultNum].ssid = (unsigned char*)ssidBuf;
	context->allWifiInfoResults[resultNum].frequency = frequency;
	context->allWifiInfoResults[resultNum].signalStrength = signalStrength;
	context->allWifiInfoResults[resultNum].isPrivacy = privacy;

	env->ReleaseStringUTFChars(macAddress, mac);
	env->ReleaseStringUTFChars(ssid, ssidstr);

	if((resultNum+1 == resultCount) && (resultCount > 0)){
		//Lock
		for(int i=0; i<(context->allWifiCallbacks->size());i++){
			AllWifiNetworksInfoCallback* cb = context->allWifiCallbacks->at(i);
			cb->callback(PAL_Ok, &(context->allWifiInfoResults),resultCount,cb->userData);
			//context->allWifiCallbacks->at(i)->callback(PAL_Ok, &(context->allWifiInfoResults),resultCount,context->allWifiCallbacks->at(i)->userData);
		}
		//Unlock
		for(int i=0;i<resultCount;i++){
			free((void*)context->allWifiInfoResults[i].macAddress);
			free((void*)context->allWifiInfoResults[i].ssid);
		}
		delete(context->allWifiInfoResults);
		context->allWifiInfoResults = NULL;
		context->allWifiInfoResultsCount = 0;
	}
}

static JNINativeMethod gMethods[] = {
    { "nativeGSMNetworkInfoCallback",        "(JJJJIII)V",               (void *) &jni_GSMNetworkInfoCallback},
    { "nativeCDMANetworkInfoCallback",       "(JJJIII)V",                (void *) &jni_CDNAMNetworkInfoCallback},
    { "nativeSystemModeCallback",            "(III)V",                   (void *) &jni_SystemModeCallback},
    { "nativeWifiNetworkInfoCallback",       "(Ljava/lang/String;III)V", (void *) &jni_WifiNetworkInfoCallback},
    { "nativeAllWifiNetworkInfoAddElement",  "(Ljava/lang/String;Ljava/lang/String;ZIIII)V", (void *) &jni_AllWifiNetworkInfoCallback},
};

static void release_fields(JNIEnv* env)
{
    if (fields.clazz)
    {
        env->DeleteGlobalRef(fields.clazz);
        fields.clazz = NULL;
    }
}

int register_palradio(JNIEnv* env)
{
    uint32 errorLine = 0;

    jclass clazz = env->FindClass("com/navbuilder/pal/android/ndk/PalRadio");
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

    fields.radioContext = env->GetFieldID(clazz, "mRadioContext", "I");
    if (fields.radioContext == NULL) {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.init = env->GetMethodID(fields.clazz, "<init>", "()V");
    if (fields.init == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.radioGetSystemMode = env->GetMethodID(fields.clazz, "RadioGetSystemMode", "(II)V");
    if (fields.radioGetSystemMode == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.radioGetGSMNetworkInfo = env->GetMethodID (fields.clazz, "RadioGetGSMNetworkInfo", "(II)V");
    if (fields.radioGetGSMNetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.radioGetCDMANetworkInfo = env->GetMethodID(fields.clazz, "RadioGetCDMANetworkInfo", "(II)V");
    if (fields.radioGetCDMANetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.radioGetWifiNetworkInfo = env->GetMethodID(fields.clazz, "RadioGetWifiNetworkInfo", "(II)V");
    if (fields.radioGetWifiNetworkInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.radioGetAllWifiNetworksInfo = env->GetMethodID(fields.clazz, "RadioGetAllWifiNetworksInfo", "()V");
    if (fields.radioGetAllWifiNetworksInfo == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.radioCancelAllWifiNetworksInfo = env->GetMethodID(fields.clazz, "RadioCancelAllWifiNetworksInfo", "()V");
    if (fields.radioCancelAllWifiNetworksInfo == NULL)
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

int unregister_palradio(JNIEnv* env)
{
    release_fields(env);
    return JNI_OK;
}
