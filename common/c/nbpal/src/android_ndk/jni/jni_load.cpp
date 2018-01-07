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
#include <assert.h>

static JavaVM* sVM = NULL;

JavaVM* getJavaVM()
{
    if (sVM == NULL)
    {
        // Should never happen
        assert(false);
    }

    return sVM;
}


JNIEnv* getJNIEnv()
{
    JNIEnv *envnow = NULL;
    int status = (sVM)->GetEnv((void **) &envnow, JNI_VERSION_1_4);
    if (status < 0)
    {
        status = (sVM)->AttachCurrentThread(&envnow, NULL);
        if (status < 0)
        {
            return NULL;
        }
    }
    return envnow;
}

jint callVoidMethod(jobject clazz, jmethodID methodID)
{
    JNIEnv* env = getJNIEnv();
    env->CallVoidMethod(clazz, methodID);
    return JNI_OK;
}

extern int register_palnet(JNIEnv* );
extern int unregister_palnet(JNIEnv* );

extern int register_uithreadandroid(JNIEnv* );
extern int unregister_uithreadandroid(JNIEnv* );

extern int register_palradio(JNIEnv* );
extern int unregister_palradio(JNIEnv* );

extern int register_com_navbuilder_pal_ndk_radiointerface(JNIEnv *env);
extern int unregister_com_navbuilder_pal_ndk_radiointerface(JNIEnv *env);

extern int register_dispalyandroid(JNIEnv* env);
extern int unregister_displayandroid(JNIEnv* env);

extern int register_palcommon(JNIEnv* env);
extern int unregister_palcommon(JNIEnv* env);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jint ret = JNI_ERR;
    JNIEnv* env = NULL;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        return JNI_ERR;
    }

    if (register_uithreadandroid(env) != JNI_OK)
    {
        goto fail;
    }

    if (register_dispalyandroid(env) != JNI_OK)
    {
        goto fail;
    }

    if (register_palnet(env) != JNI_OK)
    {
        goto fail;
    }

    if (register_palcommon(env) != JNI_OK)
    {
        goto fail;
    }


    sVM = vm;
    ret = JNI_VERSION_1_4;
fail:
    return ret;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    JNIEnv *env;
    if (jvm->GetEnv((void **)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        return;
    }

    unregister_uithreadandroid(env);
    unregister_displayandroid(env);
    unregister_palnet(env);
    unregister_palcommon(env);
}

