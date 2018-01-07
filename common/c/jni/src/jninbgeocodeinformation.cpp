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

#include "jninbgeocodeinformation.h"
#include "jniresultslice.h"
#include "jninblocation.h"
#include "logutil.h"

struct fields_t {
	// GeocodeInformationImpl related
	jclass clazz;
	jmethodID init;
    jfieldID resultSlice;
	jfieldID locations;
	// Others
	jclass locationClass;
};
static fields_t fields;

JNINBGeocodeInformation::JNINBGeocodeInformation(JNINBContext* context, NB_GeocodeInformation* g_information)
{
	LOGI("---> Creating JNINBGeocodeInformation");

	JNIEnv* env = 0;
	jint error = context->attachToJVM(&env);

	//this->initFields(env);

    uint32 start = 0;
    uint32 end = 0;
    int32 total = 0;

	NB_Error err = NB_GeocodeInformationGetSliceInformation(g_information, &total, &start, &end);
	LOGI("Get slice err=%u, total=%u, start=%u, end=%u\n", err, total, start, end);

	JNIResultSlice* jniResultSlice = new JNIResultSlice(start, end, total);
	LOGI("Created JNIResultSlice=%d\n", jniResultSlice);

	this->jObject = env->NewObject(fields.clazz, fields.init);
	LOGI("Created GeocodeInformationImpl object jObject=%d\n", this->jObject);

	env->SetObjectField(this->jObject, fields.resultSlice, jniResultSlice->getJResultSlice());
	LOGI("Set Result Slice\n");

	jobjectArray locations = env->NewObjectArray(total, fields.locationClass, NULL);
	LOGI("Created location array %d\n", locations);
	for(int i = 0; i < total; i++)
	{
        NB_Location location;
        uint32 accuracy = 0;
        err = NB_GeocodeInformationGetLocationWithAccuracy(g_information, i, &location, &accuracy);
		LOGI("Got NB_Location err=%u\n", err);

		JNINBLocation* jniNBLocation = new JNINBLocation(&location);
		LOGI("Created JNINBLocation jniNBLocation=%d\n", jniNBLocation);

		env->SetObjectArrayElement(locations, i, jniNBLocation->getJObject());
		LOGI("Added new Location\n");
	}

	env->SetObjectField(jObject, fields.locations, locations);
	LOGI("Added location array\n");

	delete jniResultSlice;
	context->detachFromJVM(env);

	LOGI("<--- JNINBGeocodeInformation");
}

JNINBGeocodeInformation::~JNINBGeocodeInformation()
{
	
}

int register_JNINBGeocodeInformation(JNIEnv* env)
{
    jclass clazz = env->FindClass("com/navbuilder/nb/geocode/internal/GeocodeInformationImpl");
	if (clazz != NULL) {
		fields.clazz = (jclass) env->NewGlobalRef(clazz);
		if (fields.clazz == NULL)
		{
			LOGE("Can't create global ref to for com/navbuilder/nb/geocode/internal/GeocodeInformationImpl");
			return -1;
		}

        /* The local reference is no longer useful */
        env->DeleteLocalRef(clazz);
	}
	else
	{
		LOGE("Can't find com/navbuilder/nb/geocode/internal/GeocodeInformationImpl");
		return -1;
	}

	fields.init = env->GetMethodID(fields.clazz, "<init>", "()V");
	fields.resultSlice = env->GetFieldID(fields.clazz, "resultSlice", "Lcom/navbuilder/nb/data/ResultSlice;");
	fields.locations = env->GetFieldID(fields.clazz, "locations", "[Lcom/navbuilder/nb/data/Location;");

	jclass locationClass = env->FindClass("com/navbuilder/nb/data/Location");
	if (locationClass != NULL) {
		fields.locationClass = (jclass) env->NewGlobalRef(locationClass);
		if (fields.locationClass == NULL)
		{
			LOGE("Can't create global ref to for com/navbuilder/nb/data/Location");
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
}

/*void JNINBGeocodeInformation::initFields(JNIEnv* env)
{
	LOGI("---> JNINBGeocodeInformation::initFields");
	//if (fields.number == NULL) {
		// init fields
	    fields.clazz = env->FindClass("com/navbuilder/nb/geocode/internal/GeocodeInformationImpl");
		if (fields.clazz == NULL) {
			LOGE("Can't find com/navbuilder/nb/geocode/internal/GeocodeInformationImpl");
			//return -1;
		}
		fields.init = env->GetMethodID(fields.clazz, "<init>", "()V");
		fields.resultSlice = env->GetFieldID(fields.clazz, "resultSlice", "Lcom/navbuilder/nb/data/ResultSlice;");

		fields.locations = env->GetFieldID(fields.clazz, "locations", "[Lcom/navbuilder/nb/data/Location;");

	LOGI("<--- JNINBGeocodeInformation::initFields");
}*/