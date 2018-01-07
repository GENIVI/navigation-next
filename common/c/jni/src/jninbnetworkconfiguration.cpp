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

#include "jninbnetworkconfiguration.h"
#include <stdlib.h>
#include <string.h>
#include "jnihelper.h"
#include "logutil.h"

struct fields_t {
	jclass clazz;
    jfieldID authenticationToken;
	jfieldID hostName;
	jfieldID locale;
	jfieldID carrier;
	jfieldID device;
	jfieldID deviceId;
	jfieldID mdn;
	jfieldID hostPort;
	jfieldID clientGuid;
};
static fields_t fields;

JNINBNetworkConfiguration::JNINBNetworkConfiguration(JNIEnv* env, jobject jnetconfig)
{
	if (jnetconfig != NULL) {
		//this->initFields(env, jnetconfig);

		jstring jstr = (jstring) env->GetObjectField(jnetconfig, fields.authenticationToken);
		this->authenticationToken = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jnetconfig, fields.hostName);
		this->hostName = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jnetconfig, fields.locale);
		this->locale = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jnetconfig, fields.carrier);
		this->carrier = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jnetconfig, fields.device);
		this->device = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jnetconfig, fields.deviceId);
		this->deviceId = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jnetconfig, fields.mdn);
		this->mdn = GetStringUTFChars(env, jstr);

		this->hostPort = env->GetIntField(jnetconfig, fields.hostPort);

		jbyteArray guidByteArray = (jbyteArray) env->GetObjectField(jnetconfig, fields.clientGuid);
		this->clientGuidSize = 0;
		this->clientGuid = jByteArrayToChars(env, guidByteArray, &this->clientGuidSize);
	}
}

JNINBNetworkConfiguration::~JNINBNetworkConfiguration()
{
	if (this->authenticationToken)
		delete this->authenticationToken;
	if (this->hostName)
		delete this->hostName;
	if (this->locale)
		delete this->locale;
	if (this->carrier)
		delete this->carrier;
	if (this->device)
		delete this->device;
	if (this->deviceId)
		delete this->deviceId;
	if (this->mdn)
		delete this->mdn;
	if (this->clientGuid)
		free(this->clientGuid);
}

int register_JNINBNetworkConfiguration(JNIEnv* env)
{
	jclass clazz = env->FindClass("com/navbuilder/nb/client/NetworkConfig");
	if (clazz != NULL) {
		fields.clazz = (jclass) env->NewGlobalRef(clazz);
		if (fields.clazz == NULL)
		{
			LOGE("Can't create global ref for com/navbuilder/nb/client/NetworkConfig");
			return -1;
		}

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
	}
	else
	{
		LOGE("Can't find com/navbuilder/nb/client/NetworkConfig");
		return -1;
	}

	fields.authenticationToken = env->GetFieldID(fields.clazz, "authenticationToken", "Ljava/lang/String;");
	fields.hostName = env->GetFieldID(fields.clazz, "hostName", "Ljava/lang/String;");
	fields.locale = env->GetFieldID(fields.clazz, "locale", "Ljava/lang/String;");
	fields.carrier = env->GetFieldID(fields.clazz, "carrier", "Ljava/lang/String;");
	fields.device = env->GetFieldID(fields.clazz, "device", "Ljava/lang/String;");
	fields.deviceId = env->GetFieldID(fields.clazz, "deviceId", "Ljava/lang/String;");
	fields.mdn = env->GetFieldID(fields.clazz, "mdn", "Ljava/lang/String;");
	fields.hostPort = env->GetFieldID(fields.clazz, "hostPort", "I");
	fields.clientGuid = env->GetFieldID(fields.clazz, "clientGuid", "[B");
}

void JNINBNetworkConfiguration::initFields(JNIEnv* env, jobject jnetconfig)
{
	//if (fields.number == NULL) {
		// init fields
		jclass clazz = env->GetObjectClass(jnetconfig);
		fields.authenticationToken = env->GetFieldID(clazz, "authenticationToken", "Ljava/lang/String;");
		fields.hostName = env->GetFieldID(clazz, "hostName", "Ljava/lang/String;");
		fields.locale = env->GetFieldID(clazz, "locale", "Ljava/lang/String;");
		fields.carrier = env->GetFieldID(clazz, "carrier", "Ljava/lang/String;");
		fields.device = env->GetFieldID(clazz, "device", "Ljava/lang/String;");
		fields.deviceId = env->GetFieldID(clazz, "deviceId", "Ljava/lang/String;");
		fields.mdn = env->GetFieldID(clazz, "mdn", "Ljava/lang/String;");
		fields.hostPort = env->GetFieldID(clazz, "hostPort", "I");
		fields.clientGuid = env->GetFieldID(clazz, "clientGuid", "[B");
	//}
}