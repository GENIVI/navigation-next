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

#include "jninblocation.h"
#include "jninbcontext.h"
#include "logutil.h"


struct fields_t {
	jclass clazz;
	jmethodID init;
	jfieldID type;
    jfieldID areaName;
    jfieldID address;
    jfieldID street1;
    jfieldID street2;
    jfieldID city;
    jfieldID county;
    jfieldID state;
    jfieldID postal;
    jfieldID country;
    jfieldID airport;
    jfieldID latitude;
    jfieldID longitude;
    jfieldID freeform;
};
static fields_t fields;

jstring JNU_NewStringNative(JNIEnv *env, const char *str)
{
	jstring result;
	jbyteArray bytes = 0;
	int len;

    static jclass stringClass = NULL;
	static jmethodID stringInit;
    if (stringClass == NULL) {
        jclass localRefCls =
            env->FindClass("java/lang/String");
        if (localRefCls == NULL) {
            return NULL; /* exception thrown */
        }
        /* Create a global reference */
        stringClass = (jclass) env->NewGlobalRef(localRefCls);

        /* The local reference is no longer useful */
        env->DeleteLocalRef(localRefCls);

        /* Is the global reference created successfully? */
        if (stringClass == NULL) {
            return NULL; /* out of memory exception thrown */
        }

		stringInit = env->GetMethodID(stringClass, "<init>", "([B)V");
    }

	if (env->EnsureLocalCapacity(2) < 0) {
		return NULL; /* out of memory error */
	}
	len = strlen(str);
	bytes = env->NewByteArray(len);
	if (bytes != NULL) {
		env->SetByteArrayRegion(bytes, 0, len, (jbyte *)str);
		result = (jstring) env->NewObject(stringClass, stringInit, bytes);
		env->DeleteLocalRef(bytes);
		return result;
	} /* else fall through */
	return NULL;
}

JNINBLocation::JNINBLocation(NB_Location *location)
{
	JNIEnv *env = 0;
	JNINBContext::attachToJVM(&env);

	//this->initFields(env);

	jObject = env->NewObject(fields.clazz, fields.init);
	env->SetObjectField(jObject, fields.areaName, JNU_NewStringNative(env, location->areaname));
	env->SetObjectField(jObject, fields.address, JNU_NewStringNative(env, location->streetnum));
	env->SetObjectField(jObject, fields.street1, JNU_NewStringNative(env, location->street1));
	env->SetObjectField(jObject, fields.street2, JNU_NewStringNative(env, location->street2));
	env->SetObjectField(jObject, fields.city, JNU_NewStringNative(env, location->city));
	env->SetObjectField(jObject, fields.county, JNU_NewStringNative(env, location->county));
	env->SetObjectField(jObject, fields.state, JNU_NewStringNative(env, location->state));
	env->SetObjectField(jObject, fields.postal, JNU_NewStringNative(env, location->postal));
	env->SetObjectField(jObject, fields.country, JNU_NewStringNative(env, location->country));
	env->SetObjectField(jObject, fields.airport, JNU_NewStringNative(env, location->airport));
	env->SetObjectField(jObject, fields.freeform, JNU_NewStringNative(env, location->freeform));
	
	env->SetIntField(jObject, fields.type, (jint) location->type);
	env->SetDoubleField(jObject, fields.latitude, (jdouble) location->latitude);
	env->SetDoubleField(jObject, fields.longitude, (jdouble) location->longitude);

	JNINBContext::detachFromJVM(env);
}

JNINBLocation::~JNINBLocation()
{
}

int register_JNINBLocation(JNIEnv* env)
{
	jclass clazz = env->FindClass("com/navbuilder/nb/data/Location");
	if (clazz != NULL) {
		fields.clazz = (jclass) env->NewGlobalRef(clazz);
		if (fields.clazz == NULL)
		{
			LOGE("Can't create global ref for com/navbuilder/nb/data/Location");
			return -1;
		}

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
	}
	else
	{
		LOGE("Can't find com/navbuilder/nb/data/Location");
		return -1;
	}

	fields.init = env->GetMethodID(fields.clazz, "<init>", "()V");
	fields.areaName = env->GetFieldID(fields.clazz, "areaName", "Ljava/lang/String;");
	fields.address = env->GetFieldID(fields.clazz, "address", "Ljava/lang/String;");
	fields.street1 = env->GetFieldID(fields.clazz, "street1", "Ljava/lang/String;");
	fields.street2 = env->GetFieldID(fields.clazz, "street2", "Ljava/lang/String;");
	fields.city = env->GetFieldID(fields.clazz, "city", "Ljava/lang/String;");
	fields.county = env->GetFieldID(fields.clazz, "county", "Ljava/lang/String;");
	fields.state = env->GetFieldID(fields.clazz, "state", "Ljava/lang/String;");
	fields.postal = env->GetFieldID(fields.clazz, "postal", "Ljava/lang/String;");
	fields.country = env->GetFieldID(fields.clazz, "country", "Ljava/lang/String;");
	fields.airport = env->GetFieldID(fields.clazz, "airport", "Ljava/lang/String;");
	fields.freeform = env->GetFieldID(fields.clazz, "freeform", "Ljava/lang/String;");

	fields.type = env->GetFieldID(fields.clazz, "type", "I");
	fields.latitude = env->GetFieldID(fields.clazz, "latitude", "D");
	fields.longitude = env->GetFieldID(fields.clazz, "longitude", "D");
}

void JNINBLocation::initFields(JNIEnv* env)
{
    fields.clazz = env->FindClass("com/navbuilder/nb/data/Location");
	if (fields.clazz == NULL) {
		printf("Can't find com/navbuilder/nb/data/Location");
		//return -1;
	}
	fields.init = env->GetMethodID(fields.clazz, "<init>", "()V");
	fields.areaName = env->GetFieldID(fields.clazz, "areaName", "Ljava/lang/String;");
	fields.address = env->GetFieldID(fields.clazz, "address", "Ljava/lang/String;");
	fields.street1 = env->GetFieldID(fields.clazz, "street1", "Ljava/lang/String;");
	fields.street2 = env->GetFieldID(fields.clazz, "street2", "Ljava/lang/String;");
	fields.city = env->GetFieldID(fields.clazz, "city", "Ljava/lang/String;");
	fields.county = env->GetFieldID(fields.clazz, "county", "Ljava/lang/String;");
	fields.state = env->GetFieldID(fields.clazz, "state", "Ljava/lang/String;");
	fields.postal = env->GetFieldID(fields.clazz, "postal", "Ljava/lang/String;");
	fields.country = env->GetFieldID(fields.clazz, "country", "Ljava/lang/String;");
	fields.airport = env->GetFieldID(fields.clazz, "airport", "Ljava/lang/String;");
	fields.freeform = env->GetFieldID(fields.clazz, "freeform", "Ljava/lang/String;");

	fields.type = env->GetFieldID(fields.clazz, "type", "I");
	fields.latitude = env->GetFieldID(fields.clazz, "latitude", "D");
	fields.longitude = env->GetFieldID(fields.clazz, "longitude", "D");
}