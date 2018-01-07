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

    @file     threadandroid.cpp
    @date     06/29/2012
    @defgroup PAL_TASKQUEUE PAL task queue component

    @brief    UI Android Thread implementation.

    UI Android Thread definition for PAL thread.
*/
/*
    (C) Copyright 2012 by Telecommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "uithreadandroid.h"
#include "palstdlib.h"
#include "taskqueue.h"
#include "task.h"

extern JNIEnv* getJNIEnv();
extern jint callVoidMethod(jobject, jmethodID);

struct fields_t {
    jclass clazz;
    jmethodID init;
    jmethodID startThread;
    jmethodID stopThread;
    jmethodID newTaskNotify;
    jmethodID cancelRunningTaskNotify;
};
static fields_t fields;

namespace nimpal
{
    namespace taskqueue
    {
        UIThreadAndroid::UIThreadAndroid(PAL_Instance* pal, TaskQueue* taskQueue, ThreadStateListener* listener)
            : Thread(pal, taskQueue, listener),
              m_nativeThread(NULL),
              m_cbThreadId(0)
        {
        }

        UIThreadAndroid::~UIThreadAndroid(void)
        {
			if (m_nativeThread)
			{
				callVoidMethod(m_nativeThread, fields.stopThread);

	            JNIEnv* env = getJNIEnv();
				env->DeleteGlobalRef(m_nativeThread);
				m_nativeThread = NULL;
			}
        }

        void UIThreadAndroid::NewTaskNotify(void)
        {
            callVoidMethod((jobject) m_nativeThread, fields.newTaskNotify);
        }

        void UIThreadAndroid::CancelRunningTaskNotify(void)
        {
            callVoidMethod((jobject) m_nativeThread, fields.cancelRunningTaskNotify);
        }

        PAL_Error UIThreadAndroid::StartThread(void)
        {
            PAL_Error err = PAL_Ok;

            do {
                if (GetState() != Invalid)
                {
                    // Thread was already created and started
                    break;
                }

                if (m_pal == NULL)
                {
                    err = PAL_ErrNoInit;
                    break;
                }

                JNIEnv* env = getJNIEnv();
                jobject nativeThread = env->NewObject(fields.clazz, fields.init, (int) this);

                if (nativeThread == NULL)
                {
                    err = PAL_ErrNoInit;
                    break;
                }

                m_nativeThread = env->NewGlobalRef(nativeThread);

                if (m_nativeThread == NULL)
                {
                    err = PAL_ErrNoInit;
                    break;
                }
                env->DeleteLocalRef(nativeThread);

                if(callVoidMethod(m_nativeThread, fields.startThread) == JNI_OK)
                {
                    m_cbThreadId = pthread_self();
                    err = PAL_Ok;
                    SetState(Pending);
                }
                else
                {
                    err = PAL_ErrQueueCannotStartNewThread;
                }
            } while(0);

            return err;
        }

        PAL_Error UIThreadAndroid::StopThread(void)
        {
        	// noop
        	return PAL_Ok;
        }

        nb_threadId UIThreadAndroid::ThreadId(void) const
        {
            return m_cbThreadId;
        }

        void UIThreadAndroid::ExecuteTasks(void)
        {
            Thread::ExecuteTasks();
        }
    }
}

static void jni_executeTasks(JNIEnv* env, jobject thiz, jint ptr)
{
    nimpal::taskqueue::UIThreadAndroid* instance = (nimpal::taskqueue::UIThreadAndroid*) ptr;

    if (instance != NULL)
    {
        instance->ExecuteTasks();
    }
}

static JNINativeMethod gMethods[] = {
    { "nativeExecuteTasks",  "(I)V",   (void *) &jni_executeTasks},
};

static void release_fields(JNIEnv* env)
{
    if (fields.clazz)
    {
        env->DeleteGlobalRef(fields.clazz);
        fields.clazz = NULL;
    }
}

int register_uithreadandroid(JNIEnv* env)
{
    uint32 errorLine = 0;

    jclass clazz = env->FindClass("com/navbuilder/pal/android/ndk/UIThreadAndroid");
    if (clazz != NULL)
    {
        fields.clazz = (jclass) env->NewGlobalRef(clazz);
        if (fields.clazz == NULL)
        {
            errorLine = __LINE__;
            goto HandleError;
        }

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
    }
    else
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.init = env->GetMethodID(fields.clazz, "<init>", "(I)V");
    if (fields.init == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.startThread = env->GetMethodID(fields.clazz, "startThread", "()V");
    if (fields.startThread == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.stopThread = env->GetMethodID(fields.clazz, "stopThread", "()V");
    if (fields.stopThread == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.newTaskNotify = env->GetMethodID(fields.clazz, "newTaskNotify", "()V");
    if (fields.newTaskNotify == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.cancelRunningTaskNotify = env->GetMethodID(fields.clazz, "cancelRunningTaskNotify", "()V");
    if (fields.cancelRunningTaskNotify == NULL)
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

int unregister_uithreadandroid(JNIEnv* env)
{
    release_fields(env);
    return JNI_OK;
}
