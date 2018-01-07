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
#include <stdlib.h>
#include <string.h>
#include "jniaddress.h"
#include "logutil.h"

struct fields_t {
	jclass clazz;
    jfieldID number;
	jfieldID street;
	jfieldID city;
	jfieldID county;
	jfieldID state;
	jfieldID postal;
	jfieldID country;
};
static fields_t fields;

static char* GetStringUTFChars(JNIEnv* env, jstring jstr)
{
	if (jstr == NULL)
	{
		return 0;
	}

	const char* tmpStr = env->GetStringUTFChars(jstr, NULL);
	char *res = new char[strlen(tmpStr) + 1];
	strncpy(res, tmpStr, strlen(tmpStr));
	env->ReleaseStringUTFChars(jstr, tmpStr);

	return res;
}

JNIAddress::JNIAddress(JNIEnv* env, jobject jaddress)
{
	if (jaddress != NULL) {
		//this->initFields(env, jaddress);

		jstring jstr = (jstring) env->GetObjectField(jaddress, fields.number);
		this->number = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jaddress, fields.street);
		this->street = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jaddress, fields.city);
		this->city = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jaddress, fields.county);
		this->county = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jaddress, fields.state);
		this->state = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jaddress, fields.postal);
		this->postal = GetStringUTFChars(env, jstr);

		jstr = (jstring) env->GetObjectField(jaddress, fields.country);
		this->country = GetStringUTFChars(env, jstr);
	}
}

JNIAddress::~JNIAddress()
{
	if (this->number)
		delete this->number;
	if (this->street)
		delete this->street;
	if (this->city)
		delete this->city;
	if (this->county)
		delete this->county;
	if (this->state)
		delete this->state;
	if (this->postal)
		delete this->postal;
	if (this->country)
		delete this->country;
}

int register_JNIAddress(JNIEnv* env)
{
	jclass clazz = env->FindClass("com/navbuilder/nb/data/Address");
	if (clazz != NULL) {
		fields.clazz = (jclass) env->NewGlobalRef(clazz);
		if (fields.clazz == NULL)
		{
			LOGE("Can't create global ref for com/navbuilder/nb/data/Address");
			return -1;
		}

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
	}
	else
	{
		LOGE("Can't find com/navbuilder/nb/data/Address");
		return -1;
	}

	fields.number = env->GetFieldID(fields.clazz, "number", "Ljava/lang/String;");
	fields.street = env->GetFieldID(fields.clazz, "street", "Ljava/lang/String;");
	fields.city = env->GetFieldID(fields.clazz, "city", "Ljava/lang/String;");
	fields.county = env->GetFieldID(fields.clazz, "county", "Ljava/lang/String;");
	fields.state = env->GetFieldID(fields.clazz, "state", "Ljava/lang/String;");
	fields.postal = env->GetFieldID(fields.clazz, "postal", "Ljava/lang/String;");
	fields.country = env->GetFieldID(fields.clazz, "country", "Ljava/lang/String;");
}

void JNIAddress::initFields(JNIEnv* env, jobject jaddress)
{
	//if (fields.number == NULL) {
		// init fields
		jclass clazz = env->GetObjectClass(jaddress);
		fields.number = env->GetFieldID(clazz, "number", "Ljava/lang/String;");
		fields.street = env->GetFieldID(clazz, "street", "Ljava/lang/String;");
		fields.city = env->GetFieldID(clazz, "city", "Ljava/lang/String;");
		fields.county = env->GetFieldID(clazz, "county", "Ljava/lang/String;");
		fields.state = env->GetFieldID(clazz, "state", "Ljava/lang/String;");
		fields.postal = env->GetFieldID(clazz, "postal", "Ljava/lang/String;");
		fields.country = env->GetFieldID(clazz, "country", "Ljava/lang/String;");
	//}
}

NB_Address JNIAddress::toNB_Address()
{
	NB_Address nb_address = { { 0 } };

	if (this->city)
		strcpy(nb_address.city, this->city);
	if (this->country)
		strcpy(nb_address.country, this->country);
	if (this->county)
		strcpy(nb_address.county, this->county);
	if (this->number)
		strcpy(nb_address.number, this->number);
	if (this->postal)
		strcpy(nb_address.postal, this->postal);
	if (this->state)
		strcpy(nb_address.state, this->state);
	if (this->street)
		strcpy(nb_address.street, this->street);

	return nb_address;
}
