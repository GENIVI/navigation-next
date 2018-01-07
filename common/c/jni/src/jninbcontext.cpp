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

#include "jninbcontext.h"
#include "logutil.h"

JNINBContext* JNINBContext::m_pInstance = NULL;
int JNINBContext::count = 0;

JNINBContext::JNINBContext()
{
	JNINBContext::m_pInstance = this;
	JNINBContext::count = 0;
}

JNINBContext::~JNINBContext()
{
	JNINBContext::m_pInstance = 0;
	JNINBContext::count = 0;
}

jint JNINBContext::attachToJVM(JNIEnv** env)
{
	LOGI("attachToJVM\n");
	if (JNINBContext::count != 0) {
		LOGI("already attached\n");
		*env = JNINBContext::m_pInstance->env;
		JNINBContext::count++;
		return 1;
	}

	LOGI("not yet attached\n");

	if (JNINBContext::m_pInstance != NULL)
	{
		jint res;
#if !defined(JNI_VERSION_1_2) || defined(ANDROID_NDK)
		res = JNINBContext::m_pInstance->mJavaVM->AttachCurrentThread(env, NULL);
#else
		res = JNINBContext::m_pInstance->mJavaVM->AttachCurrentThread((void **)env, NULL);
#endif
		LOGI("attached curretn thread res=%d, env=%d\n", res, env);
		JNINBContext::m_pInstance->env = *env;
		JNINBContext::count++;
		return res;
	}
	else
	{
		return 0;
	}
}

void JNINBContext::detachFromJVM(JNIEnv* env)
{
	LOGI("detachFromJVM env=%d\n", env);
	JNINBContext::count--;
	LOGI("detachFromJVM count=%d\n", JNINBContext::count);
	if (JNINBContext::count != 0) {
		return;
	}

	if (JNINBContext::m_pInstance != NULL)
	{
		if (env->ExceptionOccurred()) {
			env->ExceptionDescribe();
		}

		jint res =JNINBContext::m_pInstance->mJavaVM->DetachCurrentThread();
		JNINBContext::m_pInstance->env = 0;
		LOGI("detached current thread res=%d\n", res);
	}
}