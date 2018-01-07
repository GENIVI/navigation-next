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

#include "jninbgeocodeparameters.h"
#include "logutil.h"


struct fields_t {
	jclass clazz;
    jfieldID requestType;
	jfieldID iterationCommand;
	jfieldID airport;
	jfieldID freeformAddress;
	jfieldID address;
	jfieldID crossStreet;
	jfieldID country;
	jfieldID size;
};
static fields_t fields;

static char* GetStringUTFChars(JNIEnv* env, jstring jstr)
{
	jboolean isCopy;
	if (jstr != NULL)
	{
		int len = env->GetStringLength(jstr);
		const char* tmpStr = env->GetStringUTFChars(jstr, &isCopy);
		char *res = new char[strlen(tmpStr) + 1];
		//char *res = (char*)malloc(strlen(tmpStr) * sizeof(char));
		strcpy(res, tmpStr);
		env->ReleaseStringUTFChars(jstr, tmpStr);

		return res;
	}
	return 0;
}

JNINBGeocodeParameters::JNINBGeocodeParameters(JNIEnv* env, jobject jParameters)
{
/*	jclass clazz = env->GetObjectClass(jParameters);
	jfieldID locationFieldID = env->GetFieldID(clazz, "location", "Lcom/navbuilder/nb/data/Location;");
	jfieldID requestTypeFieldID = env->GetFieldID(clazz, "requestType", "I");
	jfieldID iterationCommandFieldID = env->GetFieldID(clazz, "iterationCommand", "I");

	jobject location = env->GetObjectField(jParameters, locationFieldID);
	jclass jLocationClass = env->GetObjectClass(location);

	// Location fields
	jfieldID address = env->GetFieldID(jLocationClass, "address", "Ljava/lang/String;");
	jfieldID street2 = env->GetFieldID(jLocationClass, "street2", "Ljava/lang/String;"); // cross street
	jfieldID street1 = env->GetFieldID(jLocationClass, "street1", "Ljava/lang/String;"); // street1
	jfieldID city = env->GetFieldID(jLocationClass, "city", "Ljava/lang/String;");
	jfieldID state = env->GetFieldID(jLocationClass, "state", "Ljava/lang/String;");
	jfieldID postal = env->GetFieldID(jLocationClass, "postal", "Ljava/lang/String;");
	jfieldID county = env->GetFieldID(jLocationClass, "county", "Ljava/lang/String;");
	jfieldID country = env->GetFieldID(jLocationClass, "country", "Ljava/lang/String;");
	jfieldID freeform = env->GetFieldID(jLocationClass, "freeform", "Ljava/lang/String;");
*/
	if (jParameters == NULL)
	{
		return;
	}

	//this->initFields(env, jParameters);

	this->iterationCommand = (NB_IterationCommand) env->GetIntField(jParameters, fields.iterationCommand);
	this->requestType = (GeocodeRequestType) env->GetIntField(jParameters, fields.requestType);
	this->size = (int) env->GetIntField(jParameters, fields.size);

	jstring jstr = (jstring) env->GetObjectField(jParameters, fields.freeformAddress);
	this->freeformAddress = GetStringUTFChars(env, jstr);

	jstr = (jstring) env->GetObjectField(jParameters, fields.airport);
	this->airport = GetStringUTFChars(env, jstr);

	jstr = (jstring) env->GetObjectField(jParameters, fields.country);
	this->country = GetStringUTFChars(env, jstr);

	jstr = (jstring) env->GetObjectField(jParameters, fields.crossStreet);
	this->crossStreet = GetStringUTFChars(env, jstr);

	jobject jaddress = env->GetObjectField(jParameters, fields.address);
	this->address = new JNIAddress(env, jaddress);
}

JNINBGeocodeParameters::~JNINBGeocodeParameters()
{
	if (this->freeformAddress)
		delete this->freeformAddress;
	if (this->airport)
		delete this->airport;
	if (this->country)
		delete this->country;
	if (this->crossStreet)
		delete this->crossStreet;

	if (this->address)
		delete this->address;
}

int register_JNINBGeocodeParameters(JNIEnv* env)
{
	jclass clazz = env->FindClass("com/navbuilder/nb/geocode/GeocodeParameters");
	if (clazz != NULL) {
		fields.clazz = (jclass) env->NewGlobalRef(clazz);
		if (fields.clazz == NULL)
		{
			LOGE("Can't create global ref for com/navbuilder/nb/geocode/GeocodeParameters");
			return -1;
		}

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
	}
	else
	{
		LOGE("Can't find com/navbuilder/nb/geocode/GeocodeParameters");
		return -1;
	}

	fields.requestType      = env->GetFieldID(fields.clazz, "requestType",      "I");
	fields.iterationCommand = env->GetFieldID(fields.clazz, "iterationCommand", "I");
	fields.size             = env->GetFieldID(fields.clazz, "size",             "I");
	fields.airport          = env->GetFieldID(fields.clazz, "airport",          "Ljava/lang/String;");
	fields.crossStreet      = env->GetFieldID(fields.clazz, "crossStreet",      "Ljava/lang/String;");
	fields.country          = env->GetFieldID(fields.clazz, "country",          "Ljava/lang/String;");
	fields.freeformAddress  = env->GetFieldID(fields.clazz, "freeformAddress",  "Ljava/lang/String;");
	fields.address          = env->GetFieldID(fields.clazz, "address",          "Lcom/navbuilder/nb/data/Address;");

}
void JNINBGeocodeParameters::initFields(JNIEnv* env, jobject jobj)
{
	//if (fields.number == NULL) {
		// init fields
		jclass clazz = env->GetObjectClass(jobj);
		fields.requestType      = env->GetFieldID(clazz, "requestType",      "I");
		fields.iterationCommand = env->GetFieldID(clazz, "iterationCommand", "I");
		fields.size             = env->GetFieldID(clazz, "size",             "I");
		fields.airport          = env->GetFieldID(clazz, "airport",          "Ljava/lang/String;");
		fields.crossStreet      = env->GetFieldID(clazz, "crossStreet",      "Ljava/lang/String;");
		fields.country          = env->GetFieldID(clazz, "country",          "Ljava/lang/String;");
		fields.freeformAddress  = env->GetFieldID(clazz, "freeformAddress",  "Ljava/lang/String;");
		fields.address          = env->GetFieldID(clazz, "address",          "Lcom/navbuilder/nb/data/Address;");
	//}
}