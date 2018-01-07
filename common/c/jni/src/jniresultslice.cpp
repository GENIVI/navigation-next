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

#include "jniresultslice.h"
#include "jninbcontext.h"
#include "logutil.h"

struct fields_t {
    jclass clazz;
	jmethodID init;
};
static fields_t fields;

JNIResultSlice::JNIResultSlice(int start, int end, int total)
{
	JNIEnv* env = 0;
	JNINBContext::attachToJVM(&env);

	//this->initFields(env);

	jObject = env->NewObject(fields.clazz, fields.init, start, end, total);

	JNINBContext::detachFromJVM(env);
}

JNIResultSlice::~JNIResultSlice()
{
}

int register_JNIResultSlice(JNIEnv* env)
{
	jclass clazz = env->FindClass("com/navbuilder/nb/data/ResultSlice");
	if (clazz != NULL) {
		fields.clazz = (jclass) env->NewGlobalRef(clazz);
		if (fields.clazz == NULL)
		{
			LOGE("Can't create global ref for com/navbuilder/nb/data/ResultSlice");
			return -1;
		}

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
	}
	else
	{
		LOGE("Can't find com/navbuilder/nb/data/ResultSlice");
		return -1;
	}

	fields.init = env->GetMethodID(fields.clazz, "<init>", "(III)V");
}

void JNIResultSlice::initFields(JNIEnv* env)
{
	fields.clazz = env->FindClass("com/navbuilder/nb/data/ResultSlice");
	fields.init = env->GetMethodID(fields.clazz, "<init>", "(III)V");
}