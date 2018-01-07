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

#include "com_navbuilder_nb_internal_NBInternal.h"
#include "jni.h"
#include "jninbgeocodehandler.h"
#include "logutil.h"
#include "jninbgeocodeinformation.h"

extern "C"{
#include "nbgeocodehandler.h"
#include "paltaskqueue.h"
}

struct fields_t {
    jfieldID    context;
    jmethodID   post_event;
};
static fields_t fields;

// ----------------------------------------------------------------------------
class JNINBGeocodeHandlerListener: public JNINBHandlerListener
{
public:
    JNINBGeocodeHandlerListener(JNIEnv* env, jobject thiz, jobject weak_thiz);
    ~JNINBGeocodeHandlerListener();
    void notify(request_event_type, jobject);
private:
    JNINBGeocodeHandlerListener();
    jclass      mClass;     // Reference to NativeGeocodeHandler class
    jobject     mObject;    // ref to NativeGeocodeHandler Java object to call on
	JavaVM*     mJavaVM;
};

JNINBGeocodeHandlerListener::JNINBGeocodeHandlerListener(JNIEnv* env, jobject thiz, jobject weak_thiz)
{
	jint err = env->GetJavaVM(&mJavaVM);
    // Hold onto the NativeGeocodeHandler class for use in calling the static method
    // that posts events to the application thread.
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        printf("Can't find com/navbuilder/nb/geocode/internal/NativeGeocodeHandler");
        //jniThrowException(env, "java/lang/Exception", NULL);
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);

    mObject  = env->NewGlobalRef(thiz);
}

JNINBGeocodeHandlerListener::~JNINBGeocodeHandlerListener()
{
	JNIEnv* env;
	jint res;
#if !defined(JNI_VERSION_1_2) || defined(ANDROID_NDK)
	res = mJavaVM->AttachCurrentThread(&env, NULL);
#else
	res = mJavaVM->AttachCurrentThread((void **)&env, NULL);
#endif

	//jint res = mJavaVM->AttachCurrentThread(&env, NULL);

	env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);

	if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }

    mJavaVM->DetachCurrentThread();
}

void JNINBGeocodeHandlerListener::notify(request_event_type type, jobject data)
{
	JNIEnv* env;

	//TODO check value
	jint res;
#if !defined(JNI_VERSION_1_2) || defined(ANDROID_NDK)
	res = mJavaVM->AttachCurrentThread(&env, NULL);
#else
	res = mJavaVM->AttachCurrentThread((void **)&env, NULL);
#endif
	//jint res = mJavaVM->AttachCurrentThread(env, NULL);

	env->CallStaticVoidMethod(mClass, fields.post_event, mObject, type, data);

	if (env->ExceptionOccurred()) {
        env->ExceptionDescribe();
    }

    mJavaVM->DetachCurrentThread();
}
// ----------------------------------------------------------------------------

static JNINBGeocodeHandler* setGeocodeHandler(JNIEnv* env, jobject thiz, const JNINBGeocodeHandler* handler)
{
    //Mutex::Autolock l(sLock);
	JNINBGeocodeHandler* old = (JNINBGeocodeHandler*)env->GetIntField(thiz, fields.context);
/*    if (player.get()) {
        player->incStrong(thiz);
    }
    if (old != 0) {
        old->decStrong(thiz);
    }*/

    env->SetIntField(thiz, fields.context, (int)handler);
    return old;
}

static JNINBGeocodeHandler* getGeocodeHandler(JNIEnv* env, jobject thiz)
{
    // Mutex::Autolock l(sLock);
    JNINBGeocodeHandler* const p = (JNINBGeocodeHandler*)env->GetIntField(thiz, fields.context);
    return p;
}

static void com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeCreate
  (JNIEnv* env, jobject thiz, jint contextP)
{
	LOGI("---> NativeGeocodeHandler.nativeCreate\n");

	JNINBContext* jniNBContext = (JNINBContext*) contextP;
	JNINBGeocodeHandler* jniNBGeocodeHandler = new JNINBGeocodeHandler(jniNBContext);

	JNINBGeocodeHandlerListener* listener = new JNINBGeocodeHandlerListener(env, thiz, 0);
	jniNBGeocodeHandler->setListener(listener);

	setGeocodeHandler(env, thiz, jniNBGeocodeHandler);

	LOGI("<--- NativeGeocodeHandler.nativeCreate\n");
}

static void com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeFinalize
  (JNIEnv *env, jobject thiz)
{
	LOGI("---> NativeGeocodeHandler.nativeFinalize\n");
	JNINBGeocodeHandler* handler = getGeocodeHandler(env, thiz);
	delete handler;
	LOGI("<--- NativeGeocodeHandler.nativeFinalize\n");
}

static void com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeStartRequest
  (JNIEnv *env, jobject thiz, jobject jparameters)
{
	LOGI("---> NativeGeocodeHandler.nativeStartRequest\n");

	JNINBGeocodeHandler* handler = getGeocodeHandler(env, thiz);
	JNINBGeocodeParameters* parameters = new JNINBGeocodeParameters(env, jparameters);
	handler->startRequest(parameters);

	LOGI("<--- NativeGeocodeHandler.nativeStartRequest\n");
}

static void com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeCancelRequest
  (JNIEnv *, jobject)
{
	LOGI("---> NativeGeocodeHandler.nativeCancelRequest\n");
	LOGI("<--- NativeGeocodeHandler.nativeCancelRequest\n");
}

static JNINativeMethod gMethods[] = {
    {"nativeCreate",        "(I)V",                    (void *)&com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeCreate},
    {"nativeFinalize",      "()V",                     (void *)&com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeFinalize},
    {"nativeStartRequest",  "(Lcom/navbuilder/nb/geocode/GeocodeParameters;)V", (void *)&com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeStartRequest},
    {"nativeCancelRequest", "()V",                     (void *)&com_navbuilder_nb_geocode_internal_NativeGeocodeHandler_nativeCancelRequest},
};

extern int register_JNINBGeocodeInformation(JNIEnv*);
extern int register_JNIAddress(JNIEnv*);
extern int register_JNINBGeocodeParameters(JNIEnv*);
extern int register_JNINBLocation(JNIEnv*);
extern int register_JNIResultSlice(JNIEnv*);
extern int register_JNINBNetworkConfiguration(JNIEnv*);

static int register_com_navbuilder_nb_geocode_internal_NativeGeocodeHandler(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass("com/navbuilder/nb/geocode/internal/NativeGeocodeHandler");
    if (clazz == NULL) {
        printf("Can't find com/navbuilder/nb/geocode/internal/NativeGeocodeHandler");
        return -1;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        printf("Can't find NativeGeocodeHandler.mNativeContext");
        return -1;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;ILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        printf("Can't find NativeGeocodeHandler.postEventFromNative");
        return -1;
    }
	
    return env->RegisterNatives(clazz, gMethods, sizeof(gMethods)/sizeof(gMethods[0]));
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_com_navbuilder_nb_geocode_internal_NativeGeocodeHandler(env) < 0) {
        LOGE("ERROR: NativeGeocodeHandler native registration failed\n");
        goto bail;
    }

	if (register_JNINBGeocodeInformation(env) < 0) {
        LOGE("ERROR: JNINBGeocodeInformation native registration failed\n");
        goto bail;
	}

	if (register_JNIAddress(env) < 0) {
        LOGE("ERROR: JNIAddress native registration failed\n");
        goto bail;
	}

	if (register_JNINBGeocodeParameters(env) < 0) {
        LOGE("ERROR: JNINBGeocodeParameters native registration failed\n");
        goto bail;
	}

	if (register_JNINBLocation(env) < 0) {
        LOGE("ERROR: JNINBLocation native registration failed\n");
        goto bail;
	}

	if (register_JNIResultSlice(env) < 0) {
        LOGE("ERROR: JNIResultSlice native registration failed\n");
        goto bail;
	}

	if (register_JNINBNetworkConfiguration(env) < 0) {
        LOGE("ERROR: JNINBNetworkConfiguration native registration failed\n");
        goto bail;
	}

	/* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}