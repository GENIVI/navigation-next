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
 @file     NativeNetworkAndroid.cpp
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

#include "NativeNetworkAndroid.h"
#include "nbnetwork.h"
#include "cslnetwork.h"
#include "nbcontextprotected.h"
#include "paldebuglog.h"
#include <assert.h>
#include <jni.h>
#include <android/log.h>
#include "JNIUtility.h"
#include "Network.h"
#include "MutableTPSElement.h"
#include "NetworkRequest.h"
#include "nativeltkcontext.h"
#include <map>
#include <android/log.h>

//#define VERBOSE_TPS_ELEMENT_OUTPUT

#define  LOG_TAG    "AndroidNDK"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOG_LINE   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s, %d", __func__, __LINE__)

using namespace std;

struct fields_t
{
    jclass network_clazz;
    std::map<JNINativeNetworkCallback*, jobject> networkObjMap;
    jfieldID network;
    jmethodID networkOnErrorMethodID;
    jmethodID networkConnectionOpenMethodID;
    jmethodID networkConnectionCloseMethodID;
    jmethodID networkOnIdenMethodID;
    jmethodID requestOnProgressMethodID;
    jmethodID requestOnCompleteMethodID;
    jmethodID requestOnErrorMethodID;

    jclass tps_clazz;
    jmethodID tpsConstructor;
    jfieldID tpsElement;

    jclass request_clazz;
    jmethodID requestGetTarget;
    jmethodID requestGetTpsQuery;
    jmethodID requestGetInvocationContextTPSElement;
    jmethodID requestGetID;
};

static fields_t fields;
static JavaVM* cachedVM = NULL;

JNIEnv* getJNIEnv()
{
    JNIEnv* env = NULL;
    if (cachedVM)
    {
        int status = cachedVM->GetEnv((void **) &env, JNI_VERSION_1_4);
        if (status < 0)
        {
            return NULL;
        }
    }
    return env;
}

void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData);

struct NetworkCombination
{
    shared_ptr<JNINativeNetworkCallback> m_Callback;
    NB_NetworkNotifyEventCallback m_NetworkCallback;

    shared_ptr<Network> m_Network;
};

static MutableTpsElement* GetTpsElement(JNIEnv* env, jobject thiz)
{
    if (!env || !thiz || !fields.tpsElement)
    {
        return NULL;
    }
    return reinterpret_cast<MutableTpsElement*>(env->GetLongField(thiz, fields.tpsElement));
}

static NetworkCombination* GetNativeNetwork(JNIEnv* env, jobject thiz)
{
    if (!env || !thiz || !fields.network)
    {
        return NULL;
    }
    return reinterpret_cast<NetworkCombination*>(env->GetLongField(thiz, fields.network));
}

static void JNI_InitNetwork(JNIEnv* env, jobject thiz, jint ltkContext)
{
    NativeLTKContext* nativenbcontext = reinterpret_cast<NativeLTKContext*>(ltkContext);
    if (nativenbcontext)
    {
        NetworkCombination* networkCombination = new NetworkCombination();
        if (networkCombination)
        {
            networkCombination->m_Callback = shared_ptr<JNINativeNetworkCallback>(new JNINativeNetworkCallback);
            networkCombination->m_NetworkCallback.callback = NetworkNotifyEventCallback;
            networkCombination->m_NetworkCallback.callbackData = networkCombination->m_Callback.get();

            CSL_NetworkAddEventNotifyCallback(NB_ContextGetNetwork(nativenbcontext->nbContext), &networkCombination->m_NetworkCallback);

            networkCombination->m_Network = shared_ptr<Network>(new Network(nativenbcontext->nbContext));

            env->SetLongField(thiz, fields.network, (long)networkCombination);

            fields.networkObjMap[networkCombination->m_Callback.get()] = env->NewGlobalRef(thiz);
        }
    }
}

static void JNI_DestroyNetwork(JNIEnv* env, jobject thiz)
{
    NetworkCombination* nativeNetwork = GetNativeNetwork(env, thiz);
    if (nativeNetwork)
    {
        CSL_NetworkRemoveEventNotifyCallback(NB_ContextGetNetwork(nativeNetwork->m_Network->GetNbContext()), &nativeNetwork->m_NetworkCallback);

        env->SetLongField(thiz, fields.network, 0);

        if (fields.networkObjMap[nativeNetwork->m_Callback.get()])
        {
            env->DeleteGlobalRef(fields.networkObjMap[nativeNetwork->m_Callback.get()]);
        }

        nativeNetwork->m_Callback.reset();
        nativeNetwork->m_Network.reset();
        delete nativeNetwork;
        nativeNetwork = NULL;
    }
}

static void JNI_Request(JNIEnv* env, jobject thiz, jobject nativeRequest)
{
    NetworkCombination* nativeNetwork = GetNativeNetwork(env, thiz);
    if ((!nativeNetwork) || (!nativeNetwork->m_Callback))
    {
        return;
    }

    jstring targetName = (jstring)env->CallObjectMethod(nativeRequest, fields.requestGetTarget);
    shared_ptr<string> target(new string(NewStdString(env, targetName)));

    jobject object = env->CallObjectMethod(nativeRequest, fields.requestGetTpsQuery);
    MutableTpsElement* tpsElement = GetTpsElement(env, object);
    if (tpsElement)
    {
        tpsElement->AttachToRequest(true);
        jint requestID = env->CallIntMethod(nativeRequest, fields.requestGetID);
        uint32 id = static_cast<uint32>(requestID);
        jobject tmpObject = env->CallObjectMethod(nativeRequest, fields.requestGetInvocationContextTPSElement);
        MutableTpsElement* invocationContextTPSElement = GetTpsElement(env, tmpObject);
        // mark it as attached to avoid auto release..
        // @todo: there might be memory leak, needs investigation...

        if (invocationContextTPSElement)
        {
            invocationContextTPSElement->AttachToRequest(true);
        }
        shared_ptr<NetworkRequest> request(new NetworkRequest(target, tpsElement, invocationContextTPSElement, id));
        nativeNetwork->m_Network->Request(request, nativeNetwork->m_Callback);
    }
}

static jboolean JNI_Cancel(JNIEnv* env, jobject thiz, jint requestID)
{
    NetworkCombination* nativeNetwork = GetNativeNetwork(env, thiz);
    if (nativeNetwork)
    {
        return nativeNetwork->m_Network->Cancel((int)requestID) ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

static void JNI_CancelAll(JNIEnv* env, jobject thiz)
{
    NetworkCombination* nativeNetwork = GetNativeNetwork(env, thiz);
    if (nativeNetwork)
    {
       nativeNetwork->m_Network->CancelPendingRequests();
    }
}

static void JNI_TPSElementInit(JNIEnv* env, jobject thiz, jstring name)
{
    shared_ptr<string> elementName(new string(NewStdString(env, name)));
    MutableTpsElement* tpsElement = new (std::nothrow) MutableTpsElement(elementName);

    if (!tpsElement)
    {
        return;
    }

    env->SetLongField(thiz, fields.tpsElement, (long)tpsElement);
}

static void JNI_TPSElementDestroy(JNIEnv* env, jobject thiz)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        env->SetLongField(thiz, fields.tpsElement, 0);
        /* If the tps element is attach to qequest, do not need delete it.
         * It will be destroyed when request is destoryed.
         */
        if (!tpsElement->IsAttached())
        {
            delete tpsElement;
            tpsElement = NULL;
        }
        else
        {
            tpsElement->SetCanDestory(true);
        }
    }
}

static jstring JNI_TPSElementGetName(JNIEnv* env, jobject thiz)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        return env->NewStringUTF(tpsElement->GetElementName()->c_str());
    }

    return env->NewStringUTF("");
}

static jobject JNI_TPSElementGetParent(JNIEnv* env, jobject thiz)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        MutableTpsElement* tpsParent = tpsElement->GetParent();
        if (tpsParent)
        {
            jstring elementName = env->NewStringUTF(tpsParent->GetElementName()->c_str());
            jobject object = env->NewObject(fields.tps_clazz, fields.tpsConstructor, elementName);

            MutableTpsElement* tpsElement = GetTpsElement(env, object);
            if (tpsElement)
            {
                delete tpsElement;
                tpsElement = NULL;
            }

            env->SetLongField(object, fields.tpsElement, (long)tpsParent);
            env->DeleteLocalRef(elementName);

            return object;
        }
    }

    return 0;
}

static void JNI_TPSElementAttach(JNIEnv* env, jobject thiz, jobject tpsObject)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    MutableTpsElement* child = GetTpsElement(env, tpsObject);
    if (tpsElement && child)
    {
        tpsElement->Attach(child);
    }
}

static void JNI_TPSElementUnlink(JNIEnv* env, jobject thiz)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        tpsElement->UnlinkFromParent();
    }
}

static void JNI_TPSElementSetAttr(JNIEnv* env, jobject thiz, jstring name, jbyteArray value, jint size)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        shared_ptr<string> attrName(new string(NewStdString(env, name)));
        char* buf = (char*)nsl_malloc((int)size +1);
        if (attrName && buf)
        {
            nsl_memset(buf, '\0', size+1);
            if (value && (size > 0))
            {
                env->GetByteArrayRegion(value, 0, size, (jbyte*)buf);
            }

            tpsElement->SetAttribute(attrName, (const char*)buf, (int)size);

            nsl_free(buf);
        }
    }
}

static jbyteArray JNI_TPSElementGetAttr(JNIEnv* env, jobject thiz, jstring name)
{
    jbyteArray array;
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        char* data = NULL;
        uint32 size = 0;
        shared_ptr<string> attrName(new string(NewStdString(env, name)));
        tpsElement->GetAttribute(attrName, &data, &size);

        array = env->NewByteArray(size);
        env->SetByteArrayRegion(array, 0, size, (jbyte*)data);
    }
    return array;
}

static jstring JNI_TPSElementGetAttrNameFromIndex(JNIEnv* env, jobject thiz, jint index)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        char* data = NULL;

        tpsElement->GetAttributeNameFromIndex((int)index, &data);

        return env->NewStringUTF(data);
    }
    return env->NewStringUTF("");
}

static jint JNI_TPSElementgetAttrCount(JNIEnv* env, jobject thiz)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        return (jint)tpsElement->GetAttrCount();
    }
    return 0;
}

static jint JNI_TPSElementGetChildCount(JNIEnv* env, jobject thiz)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        return (jint)tpsElement->GetChildCount();
    }

    return 0;
}

static jobject JNI_TPSElementGetChild(JNIEnv* env, jobject thiz, jint index)
{
    MutableTpsElement* tpsElement = GetTpsElement(env, thiz);
    if (tpsElement)
    {
        MutableTpsElement* tpsChild = tpsElement->GetChildByIndex((int)index);
        if (tpsChild)
        {
            jstring elementName = env->NewStringUTF(tpsChild->GetElementName()->c_str());
            jobject object = env->NewObject(fields.tps_clazz, fields.tpsConstructor, elementName);

            MutableTpsElement* tpsElement = GetTpsElement(env, object);
            if (tpsElement)
            {
                delete tpsElement;
                tpsElement = NULL;
            }

            env->SetLongField(object, fields.tpsElement, (long)tpsChild);
            env->DeleteLocalRef(elementName);

            return object;
        }
    }

    return 0;
}

static JNINativeMethod gMethods[] =
{
    { "createNativeNetwork",   "(I)V",                                    (void *)&JNI_InitNetwork},
    { "destroyNativeNetwork",   "()V",                                    (void *)&JNI_DestroyNetwork},
    { "request",               "(Lcom/navbuilder/jni/NativeRequest;)V",   (void *)&JNI_Request},
    { "cancel",                "(I)Z",                                    (void *)&JNI_Cancel},
    { "cancelPendingRequests", "()V",                                     (void *)&JNI_CancelAll},
};

static JNINativeMethod gTpsMethods[] =
{
    { "init",          "(Ljava/lang/String;)V",                           (void *)&JNI_TPSElementInit},
    { "getName",       "()Ljava/lang/String;",                            (void *)&JNI_TPSElementGetName},
    { "getParent",     "()Lcom/navbuilder/jni/NativeMutableTPSElement;",  (void *)&JNI_TPSElementGetParent},
    { "attach",        "(Lcom/navbuilder/jni/NativeMutableTPSElement;)V", (void *)&JNI_TPSElementAttach},
    { "unlink",        "()V",                                             (void *)&JNI_TPSElementUnlink},
    { "setAttr",       "(Ljava/lang/String;[BI)V",                        (void *)&JNI_TPSElementSetAttr},
    { "getAttr",       "(Ljava/lang/String;)[B",                          (void *)&JNI_TPSElementGetAttr},
    { "getAttrCount",  "()I",                                             (void *)&JNI_TPSElementgetAttrCount},
    { "getChildCount", "()I",                                             (void *)&JNI_TPSElementGetChildCount},
    { "getChild",      "(I)Lcom/navbuilder/jni/NativeMutableTPSElement;", (void *)&JNI_TPSElementGetChild},
    { "getAttrName",   "(I)Ljava/lang/String;",                           (void *)&JNI_TPSElementGetAttrNameFromIndex},
    { "destroy",       "()V",                                             (void *)&JNI_TPSElementDestroy},
};

int register_network(JavaVM* vm, JNIEnv* env)
{
    jint result = JNI_ERR;

    if (!env)
    {
        return result;
    }

    jclass clazz = env->FindClass("com/navbuilder/jni/NativeNetwork");
    if (clazz != NULL)
    {
        fields.network_clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.network_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/navbuilder/jni/NativeNetwork"));
            return result;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find com/navbuilder/jni/NativeNetwork"));
        return result;
    }

    fields.network = env->GetFieldID(fields.network_clazz, "nativePtr", "J");

    fields.networkOnErrorMethodID = env->GetMethodID(fields.network_clazz, "onError", "(I)V");
    if (fields.networkOnErrorMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNetwork.onError"));
        return result;
    }

    fields.networkConnectionOpenMethodID = env->GetMethodID(fields.network_clazz, "doConnectionOpen", "()V");
    if (fields.networkConnectionOpenMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNetwork.doConnectionOpen"));
        return result;
    }

    fields.networkConnectionCloseMethodID = env->GetMethodID(fields.network_clazz, "doConnectionClose", "()V");
    if (fields.networkConnectionCloseMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNetwork.doConnectionClose"));
        return result;
    }

    fields.networkOnIdenMethodID = env->GetMethodID(fields.network_clazz, "onIden", "([B)V");
    if (fields.networkOnIdenMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNetwork.onIden"));
        return result;
    }

    fields.requestOnProgressMethodID = env->GetMethodID(fields.network_clazz, "onPacketProgress", "(IF)V");
    if (fields.requestOnProgressMethodID == NULL)
    {
        LOGI("Can't find NativeNetwork.onPacketProgress");
        return result;
    }

    fields.requestOnCompleteMethodID = env->GetMethodID(fields.network_clazz, "onPacketComplete", "(I[B)V");
    if (fields.requestOnCompleteMethodID == NULL)
    {
        LOGI("Can't find NativeNetwork.onPacketComplete");
        return result;
    }

    fields.requestOnErrorMethodID = env->GetMethodID(fields.network_clazz, "onError", "(II)V");
    if (fields.requestOnErrorMethodID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeNetwork.onError - request"));
        return result;
    }

    if (env->RegisterNatives(fields.network_clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        return result;
    }

    clazz = env->FindClass("com/navbuilder/jni/NativeMutableTPSElement");
    if (clazz != NULL)
    {
        fields.tps_clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.tps_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/navbuilder/jni/NativeMutableTPSElement"));
            return result;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        return result;
    }

    fields.tpsElement = env->GetFieldID(fields.tps_clazz, "nativePtr", "J");

    fields.tpsConstructor = env->GetMethodID(fields.tps_clazz, "<init>", "(Ljava/lang/String;)V");
    if (fields.tpsConstructor == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeMutableTPSElement constructor"));
        return result;
    }

    if (env->RegisterNatives(fields.tps_clazz, gTpsMethods, sizeof(gTpsMethods) / sizeof(gTpsMethods[0])) != JNI_OK)
    {
        return result;
    }


    clazz = env->FindClass("com/navbuilder/jni/NativeRequest");
    if (clazz)
    {
        fields.request_clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
        if (fields.request_clazz == NULL)
        {
            DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("cannot't create global ref for com/navbuilder/jni/NativeRequest"));
            return result;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        return result;
    }

    fields.requestGetTarget = env->GetMethodID(fields.request_clazz, "getTarget", "()Ljava/lang/String;");
    if (fields.requestGetTarget == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeRequest getTarget"));
        return result;
    }

    fields.requestGetTpsQuery = env->GetMethodID(fields.request_clazz, "getTPSQuery", "()Lcom/navbuilder/jni/NativeMutableTPSElement;");
    if (fields.requestGetTpsQuery == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeRequest getTPSQuery"));
        return result;
    }

    fields.requestGetInvocationContextTPSElement = env->GetMethodID(fields.request_clazz, "getInvocationContextTPSElement", "()Lcom/navbuilder/jni/NativeMutableTPSElement;");
    if (fields.requestGetInvocationContextTPSElement == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeRequest getInvocationContextTPSElement"));
        return result;
    }

    fields.requestGetID = env->GetMethodID(fields.request_clazz, "getID", "()I");
    if (fields.requestGetID == NULL)
    {
        DEBUGLOG(LOG_SS_JNI, LOG_SEV_CRITICAL, ("Can't find NativeRequest getID"));
        return result;
    }

    cachedVM = vm;
    result = JNI_VERSION_1_4;

    return result;
}

int unregister_network(JNIEnv* env)
{
    return JNI_VERSION_1_4;
}

// network event callback
void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        if (event == NB_NetworkNotifyEvent_ConnectionOpened)
        {
            env->CallVoidMethod(fields.networkObjMap[(JNINativeNetworkCallback*)userData], fields.networkConnectionOpenMethodID);
        }
        else if (event == NB_NetworkNotifyEvent_ConnectionClosed)
        {
            env->CallVoidMethod(fields.networkObjMap[(JNINativeNetworkCallback*)userData], fields.networkConnectionCloseMethodID);
        }
        else if (data && (event == NB_NetworkNotifyEvent_IdenError))
        {
            NB_NetworkNotifyEventError* error = static_cast<NB_NetworkNotifyEventError*>(data);

            env->CallVoidMethod(fields.networkObjMap[(JNINativeNetworkCallback*)userData], fields.networkOnErrorMethodID, (jint)error->code);
        }
        else if (data && (event == NB_NetworkNotifyEvent_ClientGuidAssigned))
        {
            NB_NetworkNotifyEventClientGuid* guid = static_cast<NB_NetworkNotifyEventClientGuid*>(data);
            int length = guid->guidDataLength;

            jbyte* byte = (jbyte*)guid->guidData;
            jbyteArray buf = env->NewByteArray(length);
            env->SetByteArrayRegion(buf, 0, (jint)length, byte);

            env->CallVoidMethod(fields.networkObjMap[(JNINativeNetworkCallback*)userData], fields.networkOnIdenMethodID, buf);

            env->DeleteLocalRef(buf);
        }
    }
}

// native network callback
void JNINativeNetworkCallback::Success(uint32 id, BinaryDataPtr data)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        if (data && data->IsValid())
        {
            jbyteArray result = env->NewByteArray(data->GetSize());
            env->SetByteArrayRegion(result, 0, data->GetSize(), reinterpret_cast<const  jbyte*>(data->GetData()));
            env->CallVoidMethod(fields.networkObjMap[this], fields.requestOnCompleteMethodID, (jint)id, result);
            env->DeleteLocalRef(result);
        }
        else
        {
            Error(id, NESERVER_FAILED);
        }
    }
}

void JNINativeNetworkCallback::Error(uint32 id, NB_Error error)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(fields.networkObjMap[this], fields.requestOnErrorMethodID, (jint)id, (jint)error);
    }
}

bool JNINativeNetworkCallback::Progress(uint32 id, float percentage)
{
    JNIEnv* env = getJNIEnv();
    if (env)
    {
        env->CallVoidMethod(fields.networkObjMap[this], fields.requestOnProgressMethodID, id, (jfloat)percentage);
    }

    return true;
}
