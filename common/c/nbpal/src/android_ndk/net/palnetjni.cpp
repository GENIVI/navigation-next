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
 * (C) Copyright 2013 by TeleCommunication Systems, Inc.
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

#include "palnetjni.h"
#include "netconnection.h"
#include "ndkhttpconnection.h"
#include "ndktcpconnection.h"
#include "palerror.h"
#include "paldebuglog.h"

extern JNIEnv* getJNIEnv();

typedef struct fields_t {
    jclass baseNetClass;
    jclass netTcpClazz;
    jclass netTlsClazz;
    jclass httpClazz;
    jobject tcpInstanceRef;
    jmethodID midTcpCreate;
    jmethodID midHttpCreate;
    jmethodID midTcpConnect;
    jmethodID midHttpConnect;
    jmethodID midTcpSend;
    jmethodID midHttpSend;
    jmethodID midClose;
    void* tcpUserData;
    bool isSecurityTcpNetwork;
} fields;
static fields netFields;

namespace nimpal
{
    namespace network
    {
        map<uint32, void*> NetworkInstancesMappingTable::m_networkInstanceMap;
        uint32 NetworkInstancesMappingTable::handleID = 0;
        PAL_Instance* NetworkInstancesMappingTable::m_pal = NULL;
        PAL_Lock* NetworkInstancesMappingTable::m_lock = NULL;
        void NetworkInstancesMappingTable::SetPalInstance(PAL_Instance* pal)
        {
            m_pal = pal;
            PAL_LockCreate(m_pal, &m_lock);
        }

        uint32 NetworkInstancesMappingTable::GenNetworkHandleID()
        {
            return ((handleID == 0xffffffff) ? 0 : handleID++);
        }

        bool NetworkInstancesMappingTable::AddNetworkInstance(void* networkInstance, uint32& handleID)
        {
            if(!networkInstance)
            {
                return false;
            }
            PAL_LockLock(m_lock);
            handleID = GenNetworkHandleID();
            m_networkInstanceMap[handleID] = networkInstance;
            PAL_LockUnlock(m_lock);
            return true;
        }

        void* NetworkInstancesMappingTable::GetNetworkInstance(uint32 handleID)
        {
            void* instance = NULL;
            PAL_LockLock(m_lock);
            map<uint32, void*>::iterator it = m_networkInstanceMap.find(handleID);
            if(it != m_networkInstanceMap.end())
            {
                instance = it->second;
            }
            PAL_LockUnlock(m_lock);
            return instance;
        }

        bool NetworkInstancesMappingTable::RemoveNetworkInstance(uint32 handleID)
        {
            bool ret = false;
            PAL_LockLock(m_lock);
            map<uint32, void*>::iterator it = m_networkInstanceMap.find(handleID);
            if(it != m_networkInstanceMap.end())
            {
                m_networkInstanceMap.erase(it);
                ret = true;
            }
            PAL_LockUnlock(m_lock);
            return ret;
        }

        uint32 NetworkInstancesMappingTable::Size()
        {
            return m_networkInstanceMap.size();
        }

        void PalNetJNI::SetPalInstance(PAL_Instance* pal)
        {
            NetworkInstancesMappingTable::SetPalInstance(pal);
        }

        void* PalNetJNI::CreateSocket(void* userData, uint32& handleID, bool isSecurityTcpNetwork)
        {
            JNIEnv* env = getJNIEnv();
            if(!env || !userData)
            {
                return NULL;
            }

            jclass clazz = NULL;
            netFields.isSecurityTcpNetwork = isSecurityTcpNetwork;
            clazz = isSecurityTcpNetwork ? netFields.netTlsClazz : netFields.netTcpClazz;

            jmethodID constructor;
            constructor = env->GetMethodID(clazz, "<init>", "()V");
            jobject instance;
            instance = env->NewObject(clazz, constructor);
            if (instance == NULL)
            {
                return NULL;
            }
            netFields.tcpInstanceRef = env->NewGlobalRef(instance);
            env->DeleteLocalRef(instance);

            uint32 tmpHandleID = 0;
            if(!NetworkInstancesMappingTable::AddNetworkInstance(userData, tmpHandleID))
            {
                return NULL;
            }
            handleID = tmpHandleID;
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI::Tcp create[%p] handle[%d], Map size[%d]", userData, tmpHandleID, NetworkInstancesMappingTable::Size()));

            env->CallVoidMethod(netFields.tcpInstanceRef, netFields.midTcpCreate, (jlong)handleID);
            return (void*)(netFields.tcpInstanceRef);
        }

        bool PalNetJNI::Connect(void* jInstance, std::string host, int port)
        {
            JNIEnv* env = getJNIEnv();
            if((!env) || (host.empty()) || (port <= 0) || (!jInstance))
            {
                return false;
            }

            jstring jHostStr = env->NewStringUTF(host.c_str());
            env->CallVoidMethod((jobject)jInstance, netFields.midTcpConnect, jHostStr, port);
            env->DeleteLocalRef(jHostStr);
            return true;
        }

        bool PalNetJNI::Send(void* jInstance, char* buf, int len)
        {
            JNIEnv* env = getJNIEnv();
            if((!env) || (!buf) || (len <= 0) || (!jInstance))
            {
                return false;
            }

            jbyte *data = (jbyte*)buf;
            jbyteArray jarray = env->NewByteArray((jint)len);
            env->SetByteArrayRegion(jarray, 0, len, data);
            env->CallVoidMethod((jobject)jInstance, netFields.midTcpSend, jarray, len);
            env->DeleteLocalRef(jarray);
            return true;
        }

        void PalNetJNI::Close(void* jInstance, uint32 handleID)
        {
            JNIEnv* env = getJNIEnv();
            if((!env) || (!jInstance))
            {
                return;
            }
            NetworkInstancesMappingTable::RemoveNetworkInstance(handleID);
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI::Tcp Close, handle[%d], Map size[%d]", handleID, NetworkInstancesMappingTable::Size()));
            env->CallVoidMethod((jobject)jInstance, netFields.midClose);
            env->DeleteGlobalRef((jobject)jInstance);
            netFields.tcpUserData = NULL;
        }

        void* PalNetJNI::HttpCreate(void* userData, uint32& handleID, bool isSecurityHttpNetwork)
        {
            JNIEnv* env = getJNIEnv();
            if(!env || !userData)
            {
                return NULL;
            }

            jmethodID constructor;
            constructor = env->GetMethodID(netFields.httpClazz, "<init>", "()V");
            jobject instance;
            instance = env->NewObject(netFields.httpClazz, constructor);
            if (instance == NULL)
            {
                return NULL;
            }
            jobject httpInstanceRef = env->NewGlobalRef(instance);
            env->DeleteLocalRef(instance);
            instance = NULL;
            if(!httpInstanceRef)
            {
                return NULL;
            }
            uint32 tmpHandleID = 0;
            if(!NetworkInstancesMappingTable::AddNetworkInstance(userData, tmpHandleID))
            {
                return NULL;
            }
            handleID = tmpHandleID;
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI::Http create[%p] handle[%d], Map size[%d]", userData, tmpHandleID, NetworkInstancesMappingTable::Size()));

            env->CallVoidMethod(httpInstanceRef, netFields.midHttpCreate, (jlong)tmpHandleID);
            return (void*)(httpInstanceRef);
        }

        bool PalNetJNI::HttpConnect(void* jInstance, std::string host, int port, bool isSecurityConnection)
        {
            JNIEnv* env = getJNIEnv();
            if((!env) || (host.empty()) || (port <= 0) || (!jInstance))
            {
                return false;
            }

            jstring jHostStr = env->NewStringUTF(host.c_str());
            if (jHostStr == NULL)
            {
                return false;
            }
            env->CallVoidMethod((jobject)jInstance, netFields.midHttpConnect, jHostStr, port, (isSecurityConnection ? 1:0));
            env->DeleteLocalRef(jHostStr);
            return true;
        }

        bool PalNetJNI::HttpSend(void* jInstance,
                                 const char* buf,
                                 int         len,
                                 const char* pVerb,
                                 const char* pObject,
                                 const char* pAcceptType,
                                 const char* pAdditionalHeaders,
                                 void*       pRequestData)
        {
            JNIEnv* env = getJNIEnv();
            if((!env) || !pVerb || (!jInstance))
            {
                return false;
            }

            jstring jVerb = NULL;
            jstring jObject = NULL;
            jstring jAcceptType = NULL;
            jstring jAdditionalHeaders = NULL;
            if(pVerb)
            {
                jVerb = env->NewStringUTF(pVerb);
            }

            if(pObject)
            {
                jObject = env->NewStringUTF(pObject);
            }

            if(pAcceptType)
            {
                jAcceptType = env->NewStringUTF(pAcceptType);
            }

            if(pAdditionalHeaders)
            {
                jAdditionalHeaders = env->NewStringUTF(pAdditionalHeaders);
            }

            if(buf && len > 0)
            {
                jbyte *data = (jbyte*)buf;
                jbyteArray jarray = env->NewByteArray((jint)len);
                env->SetByteArrayRegion(jarray, 0, len, data);
                env->CallVoidMethod((jobject)jInstance, netFields.midHttpSend, jarray, len, jVerb, jObject, jAcceptType, jAdditionalHeaders, pRequestData);
                env->DeleteLocalRef(jarray);
            }
            else
            {
                jlong temp = (jlong)pRequestData;
                env->CallVoidMethod((jobject)jInstance, netFields.midHttpSend, NULL, 0, jVerb, jObject, jAcceptType, jAdditionalHeaders, temp);
            }

            if(pVerb)
            {
                env->DeleteLocalRef(jVerb);
            }

            if(pObject)
            {
                env->DeleteLocalRef(jObject);
            }

            if(pAcceptType)
            {
                env->DeleteLocalRef(jAcceptType);
            }

            if(pAdditionalHeaders)
            {
                env->DeleteLocalRef(jAdditionalHeaders);
            }
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("**********************"));
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("***PalNetJNI::HttpSend, pRequestData:%ld", pRequestData));
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("***PalNetJNI::HttpSend, target:%s", pObject));
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("**********************"));
            return true;
        }

        void PalNetJNI::HttpClose(void* jInstance, uint32 handleID)
        {
            JNIEnv* env = getJNIEnv();
            if((!env) || (!jInstance))
            {
                return;
            }
            NetworkInstancesMappingTable::RemoveNetworkInstance(handleID);
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI::Http Close, handle[%d], Map size[%d]", handleID, NetworkInstancesMappingTable::Size()));
            env->CallVoidMethod((jobject)jInstance, netFields.midClose);
            env->DeleteGlobalRef((jobject)jInstance);
        }
    }
}

void notifyHttpNetworkStatus(JNIEnv* env, jobject thiz, jint status, jlong pHttpUserData)
{
    DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("!!!notifyHttpNetworkStatus, http connection: %ld", (void*)pHttpUserData));

    void* pNetworkInstance = nimpal::network::NetworkInstancesMappingTable::GetNetworkInstance(static_cast<uint32>(pHttpUserData));
    if(!pNetworkInstance)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI HandleID[%d] Check - notifyHttpNetworkStatus", static_cast<uint32>(pHttpUserData)));
        return;
    }

    network::NDKHttpConnection* conn = static_cast<network::NDKHttpConnection*>(pNetworkInstance);
    if(!conn)
    {
        return;
    }
    int temp = status;
    conn->onHttpNetworkStatus((PAL_NetConnectionStatus)temp);
}

void notifyNetworkStatus(JNIEnv* env, jobject thiz, jint status, jlong pTcpUserData)
{
    void* pNetworkInstance = nimpal::network::NetworkInstancesMappingTable::GetNetworkInstance(static_cast<uint32>(pTcpUserData));
    if(!pNetworkInstance)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI HandleID[%d] Check - notifyNetworkStatus", static_cast<uint32>(pTcpUserData)));
        return;
    }
    network::NDKTcpConnection* conn = static_cast<network::NDKTcpConnection*>(pNetworkInstance);
    if(!conn)
    {
        return;
    }
    int temp = status;
    conn->SetTcpStatus((PAL_NetConnectionStatus)temp);
}

void notifyNetWorkRecvData(JNIEnv* env1, jobject thiz, jbyteArray pData, jint len, jlong pTcpUserData)
{
    JNIEnv* env = getJNIEnv();
    if(!env || !pData || len <= 0)
    {
        return;
    }

    jbyte* recvData = env->GetByteArrayElements(pData, NULL);
    jsize recvDataLen = env->GetArrayLength(pData);
    if(!recvData || recvDataLen <= 0)
    {
        return;
    }

    void* pNetworkInstance = nimpal::network::NetworkInstancesMappingTable::GetNetworkInstance(static_cast<uint32>(pTcpUserData));
    if(!pNetworkInstance)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI HandleID[%d] Check - notifyNetWorkRecvData", static_cast<uint32>(pTcpUserData)));
        return;
    }

    network::NDKTcpConnection* conn = static_cast<network::NDKTcpConnection*>(pNetworkInstance);
    if(conn)
    {
        conn->NotifyTcpDataReceived((const byte*)recvData, recvDataLen);
    }

    if(pData && recvData)
    {
        env->ReleaseByteArrayElements(pData, recvData, 0);
    }
}

void NotifyHttpDataReceived(JNIEnv* env1, jobject thiz,
        jlong pRequestData, jint responseErrorCode, jbyteArray pData, jint len, jlong pHttpUserData)
{
    JNIEnv* env = getJNIEnv();
    if(!env || pRequestData == 0)
    {
        return;
    }

    jbyte* recvData = NULL;
    jsize recvDataLen = 0;
    if(pData)
    {
        recvData = env->GetByteArrayElements(pData, NULL);
        recvDataLen = env->GetArrayLength(pData);
    }

    void* pNetworkInstance = nimpal::network::NetworkInstancesMappingTable::GetNetworkInstance(static_cast<uint32>(pHttpUserData));
    if(!pNetworkInstance)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI HandleID[%d] Check - NotifyHttpDataReceived", static_cast<uint32>(pHttpUserData)));
        return;
    }

    network::NDKHttpConnection* conn = static_cast<network::NDKHttpConnection*>(pNetworkInstance);
    if(conn)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("**********************"));
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("PalNetJNI::NotifyHttpDataReceived, pRequestData:%ld", (void*)pRequestData));
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("**********************"));
        conn->onHttpDataReceived((void*)pRequestData, (PAL_Error)responseErrorCode, (const byte*)recvData, recvDataLen);
    }

    if(pData && recvData)
    {
        env->ReleaseByteArrayElements(pData, recvData, 0);
    }
}

void NotifyHttpResponseStatus(JNIEnv* env1, jobject thiz,
        jlong pRequestData, jint status, jlong pHttpUserData)
{
    JNIEnv* env = getJNIEnv();
    if(!env || pRequestData == 0)
    {
        return;
    }

    void* pNetworkInstance = nimpal::network::NetworkInstancesMappingTable::GetNetworkInstance(static_cast<uint32>(pHttpUserData));
    if(!pNetworkInstance)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI HandleID[%d] Check - NotifyHttpResponseStatus", static_cast<uint32>(pHttpUserData)));
        return;
    }

    network::NDKHttpConnection* conn = static_cast<network::NDKHttpConnection*>(pNetworkInstance);
    if(!conn)
    {
        return;
    }
    conn->onHttpResponseStatus((void*)pRequestData, status);
}

void NotifyHttpResponseHeaders(JNIEnv* env1, jobject thiz,
        jlong pRequestData, jstring responseHeaders, jlong pHttpUserData)
{
    JNIEnv* env = getJNIEnv();
    if(!env || pRequestData == 0 || responseHeaders == NULL)
    {
        return;
    }

    void* pNetworkInstance = nimpal::network::NetworkInstancesMappingTable::GetNetworkInstance(static_cast<uint32>(pHttpUserData));
    if(!pNetworkInstance)
    {
        DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("###PalNetJNI HandleID[%d] Check - NotifyHttpResponseHeaders", static_cast<uint32>(pHttpUserData)));
        return;
    }

    network::NDKHttpConnection* conn = static_cast<network::NDKHttpConnection*>(pNetworkInstance);
    if(!conn)
    {
        return;
    }

    int len = env->GetStringLength(responseHeaders);
    if(len == 0)
    {
        return;
    }
    const char* tmpResponseHeaders = env->GetStringUTFChars(responseHeaders, NULL);
    unsigned int stringLength = (len + 1) * sizeof(char);
    //Don't mind the malloc will trigger the memory leak, this will free in the httpresponse object.
    char* str = (char*) nsl_malloc(stringLength);
    if(str)
    {
        nsl_memset(str, 0, stringLength);
        nsl_strcpy(str, tmpResponseHeaders);
        conn->onHttpResponseHeaders((void*)pRequestData, str);
    }
    env->ReleaseStringUTFChars(responseHeaders, tmpResponseHeaders);
}

static JNINativeMethod gMethods[] = {
    {
        "notifyNetWorkRecvData",
        "([BIJ)V",
        (void *) &notifyNetWorkRecvData
    },

    {
        "notifyNetworkStatus",
        "(IJ)V",
        (void *) &notifyNetworkStatus
    },

    {
        "notifyHttpNetworkStatus",
        "(IJ)V",
        (void *) &notifyHttpNetworkStatus
    },


    {
        "NotifyHttpDataReceived",
        "(JI[BIJ)V",
        (void *) &NotifyHttpDataReceived
    },

    {
        "NotifyHttpResponseStatus",
        "(JIJ)V",
        (void *) &NotifyHttpResponseStatus
    },

    {
        "NotifyHttpResponseHeaders",
        "(JLjava/lang/String;J)V",
        (void *) &NotifyHttpResponseHeaders
    }
};

void release_fields()
{
    JNIEnv* env = getJNIEnv();
    if (netFields.netTcpClazz &&
        netFields.netTlsClazz &&
        netFields.baseNetClass &&
        env)
    {
        env->DeleteGlobalRef(netFields.netTcpClazz);
        env->DeleteGlobalRef(netFields.netTlsClazz);
        env->DeleteGlobalRef(netFields.baseNetClass);
        env->DeleteGlobalRef(netFields.tcpInstanceRef);
        netFields.netTcpClazz = NULL;
        netFields.netTlsClazz = NULL;
        netFields.baseNetClass = NULL;
        netFields.tcpInstanceRef = NULL;
    }
}

int unregister_palnet(JNIEnv* )
{
    release_fields();
    return JNI_OK;
}

int register_palnet(JNIEnv* env)
{
    if(!env)
    {
        return JNI_ERR;
    }

    nsl_memset(&netFields, 0, sizeof(netFields));
    jclass clazz = env->FindClass("com/navbuilder/pal/android/ndk/NetworkConnection");
    if (clazz != NULL)
    {
        netFields.baseNetClass = (jclass) env->NewGlobalRef(clazz);
        if (netFields.baseNetClass == NULL)
        {
            return JNI_ERR;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        return JNI_ERR;
    }

    clazz = env->FindClass("com/navbuilder/pal/android/ndk/HttpConnection");

    if (clazz != NULL)
    {
        netFields.httpClazz = (jclass) env->NewGlobalRef(clazz);
        if (netFields.httpClazz == NULL)
        {
            return JNI_ERR;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        return JNI_ERR;
    }

    clazz = env->FindClass("com/navbuilder/pal/android/ndk/SSLConnection");
    if (clazz != NULL)
    {
        netFields.netTlsClazz = (jclass) env->NewGlobalRef(clazz);
        if (netFields.netTlsClazz == NULL)
        {
            return JNI_ERR;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        return JNI_ERR;
    }

    clazz = env->FindClass("com/navbuilder/pal/android/ndk/TCPConnection");
    if (clazz != NULL)
    {
        netFields.netTcpClazz = (jclass) env->NewGlobalRef(clazz);
        if (netFields.netTcpClazz == NULL)
        {
            return JNI_ERR;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        return JNI_ERR;
    }

    if (env->RegisterNatives(netFields.baseNetClass, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
    {
        goto HandleError;
    }

    netFields.midTcpCreate = env->GetMethodID(netFields.baseNetClass, "create", "(J)V");
    if (netFields.midTcpCreate == NULL)
    {
        goto HandleError;
    }

    netFields.midHttpCreate = env->GetMethodID(netFields.baseNetClass, "create", "(J)V");
    if (netFields.midHttpCreate == NULL)
    {
        goto HandleError;
    }

    netFields.midTcpConnect = env->GetMethodID(netFields.baseNetClass, "connect", "(Ljava/lang/String;I)V");
    if (netFields.midTcpConnect == NULL)
    {
        goto HandleError;
    }

    netFields.midHttpConnect = env->GetMethodID(netFields.baseNetClass, "connect", "(Ljava/lang/String;IZ)V");
    if (netFields.midHttpConnect == NULL)
    {
        goto HandleError;
    }

    netFields.midTcpSend = env->GetMethodID(netFields.baseNetClass, "sent", "([BI)V");
    if (netFields.midTcpSend == NULL)
    {
        goto HandleError;
    }

    netFields.midHttpSend = env->GetMethodID(netFields.baseNetClass, "sent", "([BILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V");
    if (netFields.midHttpSend == NULL)
    {
        goto HandleError;
    }

    netFields.midClose = env->GetMethodID(netFields.baseNetClass, "close", "()V");
    if (netFields.midClose == NULL)
    {
        goto HandleError;
    }
    return JNI_OK;

HandleError:
    release_fields();
    return JNI_ERR;
}
