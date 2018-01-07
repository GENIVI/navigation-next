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

    @file     realgpsprovider.cpp
    @date     01/27/2013
    @defgroup REALGPSPROVIDER_H Real Gps

    @brief    Real Gps

*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "realgpsprovider.h"
#include "palclock.h"
#include "palstdlib.h"

extern JNIEnv* getJNIEnv();

struct fields_t {
    jclass clazz;
    jmethodID init;
    jmethodID startLocationUpdates;
    jmethodID stopLocationUpdates;
    jfieldID  gpsContext;
};
static fields_t fields;

static jobject gpsProviderRef = NULL;

RealGpsProvider::RealGpsProvider(PAL_Instance* pal):
    GpsProvider(pal),
    m_pPal(pal)
{
	JNIEnv* env = getJNIEnv();
	jobject gpsProvider = env->NewObject(fields.clazz, fields.init);

    if(gpsProvider == NULL){
    	return;
    }

	gpsProviderRef = env->NewGlobalRef(gpsProvider);
	env->DeleteLocalRef(gpsProvider);
}

RealGpsProvider::~RealGpsProvider()
{
    if (gpsProviderRef != NULL)
    {
    	JNIEnv* env = getJNIEnv();
		env->DeleteGlobalRef(gpsProviderRef);
		gpsProviderRef = NULL;
    }
}

PAL_Error RealGpsProvider::Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 configCount)
{
	JNIEnv* env = getJNIEnv();
	env->SetIntField(gpsProviderRef, fields.gpsContext, (int)this);

    return PAL_Ok;
}

void RealGpsProvider::start()
{
	if (gpsProviderRef != NULL)
	{
        JNIEnv* env = getJNIEnv();
  	    env->CallVoidMethod(gpsProviderRef, fields.startLocationUpdates);
	}
}

void RealGpsProvider::stop()
{
	if (gpsProviderRef != NULL)
	{
	    JNIEnv* env = getJNIEnv();
	    env->CallVoidMethod(gpsProviderRef, fields.stopLocationUpdates);
	}
}

void RealGpsProvider::Notify(double latitude, double longitude, double altitude, double heading, long time, int satellitesCount)
{
    //convert latitude and longitude to the AbpalGpsLocation
	ABPAL_GpsLocation location = {0};

    location.status = 0;
    location.gpsTime = time;
    location.valid = PGV_Latitude | PGV_Longitude | PGV_Altitude | PGV_Heading | PGV_SatelliteCount;
    location.latitude = latitude;
    location.longitude = longitude;
    location.heading = heading;
    location.horizontalVelocity = 0;//gpsFileRecord->wVelocityHor * Gps_Horizontal_Vel_Constant;
    location.altitude = altitude;
    location.verticalVelocity = 0;//gpsFileRecord->bVelocityVer * Gps_Vertical_Vel_Constant;
    location.horizontalUncertaintyAngleOfAxis = 0;//gpsFileRecord->bHorUncAngle * Gps_Horizontal_Unc_Angle_Of_AxisC_onstant;
    location.horizontalUncertaintyAlongAxis = 0;//uncdecode(gpsFileRecord->bHorUnc);
    location.horizontalUncertaintyAlongPerpendicular = 0;//uncdecode(gpsFileRecord->bHorUncPerp);
    location.verticalUncertainty = 0;//uncdecode(gpsFileRecord->wVerUnc);
    location.utcOffset = 0;
    location.numberOfSatellites = satellitesCount;

    NotifyListeners(PAL_Ok, location);
}

static void jni_GPSLocationChanged(JNIEnv* env, jobject thiz, jdouble latitude, jdouble longitude, jdouble altitude, jdouble heading, jlong time, jint satNum)
{
	RealGpsProvider* provider = (RealGpsProvider*)env->GetIntField(thiz , fields.gpsContext);

    provider->Notify(latitude, longitude, altitude, heading, time, satNum);
}

static JNINativeMethod gMethods[] = {
    { "nativeGPSLocationChanged",  "(DDDDJI)V", (void *) &jni_GPSLocationChanged},
};

static void release_fields(JNIEnv* env)
{
    if (fields.clazz)
    {
        env->DeleteGlobalRef(fields.clazz);
        fields.clazz = NULL;
    }
}

int register_realgpsprovider(JNIEnv* env)
{
    uint32 errorLine = 0;

    jclass clazz = env->FindClass("com/navbuilder/pal/android/ndk/GPSLocationManager");
    if (clazz != NULL)
    {
        fields.clazz = (jclass) env->NewGlobalRef(clazz);
        if (fields.clazz == NULL)
        {
            errorLine = __LINE__;
            goto HandleError;
        }

        env->DeleteLocalRef(clazz);
    }
    else
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.gpsContext = env->GetFieldID(clazz, "mGPSContext", "I");
    if (fields.gpsContext == NULL) {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.init = env->GetMethodID(fields.clazz, "<init>", "()V");
    if (fields.init == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.startLocationUpdates = env->GetMethodID(fields.clazz, "startLocationUpdates", "()V");
    if (fields.startLocationUpdates == NULL)
    {
        errorLine = __LINE__;
        goto HandleError;
    }

    fields.stopLocationUpdates = env->GetMethodID(fields.clazz, "stopLocationUpdates", "()V");
    if (fields.stopLocationUpdates == NULL)
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

int unregister_realgpsprovider(JNIEnv* env)
{
    release_fields(env);
    return JNI_OK;
}
