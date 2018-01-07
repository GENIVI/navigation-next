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
 @file     nblocationandroid.cpp
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
 /*! @{ */

#include "nblocationandroid.h"
#include "palstdlib.h"
#include <android/log.h>
#define LOG_TAG "NBLocation"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct fields_location
{
    jmethodID getAltitude;
    jmethodID getGpsTime;
    jmethodID getHeading;
    jmethodID getHorizontalVelocity;
    jmethodID getLatitude;
    jmethodID getLongitude;
    jmethodID getNumberOfSatellites;
    jmethodID getStatus;
    jmethodID getValid;
    jmethodID getAccuracy;
    jmethodID getLocationType;
};
static fields_location fields;

bool
GetNativeNBGpsLocation(JNIEnv* env, jobject locationObj, NB_GpsLocation &location)
{
    if(env == NULL || locationObj == NULL)
    {
        return false;
    }
    location.altitude = env->CallDoubleMethod(locationObj, fields.getAltitude);
    location.gpsTime = env->CallLongMethod(locationObj, fields.getGpsTime);
    location.heading = env->CallDoubleMethod(locationObj, fields.getHeading);
    location.horizontalVelocity = env->CallDoubleMethod(locationObj, fields.getHorizontalVelocity);
    location.latitude = env->CallDoubleMethod(locationObj, fields.getLatitude);
    location.longitude = env->CallDoubleMethod(locationObj, fields.getLongitude);
    location.numberOfSatellites = env->CallIntMethod(locationObj, fields.getNumberOfSatellites);
    location.status = static_cast<NB_Error>(env->CallIntMethod(locationObj, fields.getStatus));
    location.valid = env->CallIntMethod(locationObj, fields.getValid);
    location.horizontalUncertaintyAlongAxis = static_cast<double>(env->CallIntMethod(locationObj, fields.getAccuracy));
	// To fix accuracy issue,
	// we set NGV_HorizontalUncertainty the same as NGV_AxisUncertainty.
	if(location.valid & NGV_AxisUncertainty)
	{
		location.valid |= NGV_HorizontalUncertainty;
	}
	else
	{
		location.valid &= (~NGV_HorizontalUncertainty);
	}
    return true;
}

int
register_nblocation(JNIEnv* env)
{
    if(env == NULL)
    {
        return JNI_ERR;
    }
    jclass clazz = env->FindClass("com/locationtoolkit/common/data/Location");
    if(clazz == NULL)
    {
        LOGE("Cannot find class Location!");
        return JNI_ERR;
    }
    fields.getAltitude = env->GetMethodID(clazz, "getAltitude", "()D");
    if(fields.getAltitude == NULL)
    {
        LOGE("Cannot find Location.getAltitude!");
        return JNI_ERR;
    }
    fields.getGpsTime = env->GetMethodID(clazz, "getGpsTime", "()J");
    if(fields.getGpsTime == NULL)
    {
        LOGE("Cannot find Location.getGpsTime!");
        return JNI_ERR;
    }
    fields.getHeading = env->GetMethodID(clazz, "getHeading", "()D");
    if(fields.getHeading == NULL)
    {
        LOGE("Cannot find Location.getHeading!");
        return JNI_ERR;
    }
    fields.getHorizontalVelocity = env->GetMethodID(clazz, "getHorizontalVelocity", "()D");
    if(fields.getHorizontalVelocity == NULL)
    {
        LOGE("Cannot find Location.getHorizontalVelocity!");
        return JNI_ERR;
    }
    fields.getLatitude = env->GetMethodID(clazz, "getLatitude", "()D");
    if(fields.getLatitude == NULL)
    {
        LOGE("Cannot find Location.getLatitude!");
        return JNI_ERR;
    }
    fields.getLongitude = env->GetMethodID(clazz, "getLongitude", "()D");
    if(fields.getLongitude == NULL)
    {
        LOGE("Cannot find Location.getLongitude!");
        return JNI_ERR;
    }
    fields.getNumberOfSatellites = env->GetMethodID(clazz, "getNumberOfSatellites", "()I");
    if(fields.getNumberOfSatellites == NULL)
    {
        LOGE("Cannot find Location.getNumberOfSatellites!");
        return JNI_ERR;
    }
    fields.getStatus = env->GetMethodID(clazz, "getStatus", "()I");
    if(fields.getStatus == NULL)
    {
        LOGE("Cannot find Location.getStatus!");
        return JNI_ERR;
    }
    fields.getValid = env->GetMethodID(clazz, "getValid", "()I");
    if(fields.getValid == NULL)
    {
        LOGE("Cannot find Location.getValid!");
        return JNI_ERR;
    }
    fields.getAccuracy = env->GetMethodID(clazz, "getAccuracy", "()I");
    if(fields.getAccuracy == NULL)
    {
        LOGE("Cannot find Location.getAccuracy!");
        return JNI_ERR;
    }
    fields.getLocationType = env->GetMethodID(clazz, "getLocationType", "()I");
    if(fields.getLocationType == NULL)
    {
        LOGE("Cannot find Location.getLocationType!");
        return JNI_ERR;
    }
    return JNI_OK;
}

int
unregister_nblocation(JNIEnv* env)
{
    return JNI_OK;
}

 /*! @} */