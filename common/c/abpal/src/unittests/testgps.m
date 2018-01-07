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

@defgroup TestGps_c Unit Tests for PAL GPS

Unit Tests for PAL GPS

This file contains all unit tests for the PAL audio component
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testgps.h"
#include "main.h"
#include "abpalgps.h"
#include "platformutil.h"
#include "palfile.h"
#include "palclock.h"
#ifdef WINCE
#include "winbase.h"
#include "windows.h"
#endif


// Constants ................................................................

const int INITIALIZE_TIMEOUT_MSEC =  45 * 1000;
const int GET_LOCATION_TIMEOUT_MSEC = 30 * 1000;
const int MSA_TIMEOUT_MSEC = 30 * 1000;
const int TRACKING_TIME_MSEC = 2 * 60 * 1000;

const char* LOG_FILE_NAME = "testgps.txt";

// Global variables .........................................................

static void* g_deviceStateEvent = 0;
static void* g_locationEvent = 0;
static PAL_Instance* g_pal = 0;

typedef void* HANDLE;
// Static helper functions ..................................................

void
WriteLogFileMessage(const char* format, ...)
{
    PAL_File* file = 0;
    PAL_ClockDateTime dateTime = {0};
    uint32 written = 0;
    char dateTimeStamp[80] = {0};

    char buffer[256] = {0};
    va_list args;

    (void)PAL_FileOpen(g_pal, LOG_FILE_NAME, PFM_Append, &file);
    (void)PAL_ClockGetDateTime(&dateTime);
    sprintf(dateTimeStamp, "%02d/%02d/%04d %02d:%02d:%02d ",
        dateTime.month, dateTime.day, dateTime.year, dateTime.hour, dateTime.minute, dateTime.second);
    (void)PAL_FileWrite(file, (const uint8 *)dateTimeStamp, strlen(dateTimeStamp) * sizeof(char), &written);

    va_start(args, format);
    vsprintf(buffer, format, args);
    strcat(buffer, "\r\n");

    (void)PAL_FileWrite(file, (const uint8 *)buffer, strlen(buffer) * sizeof(char), &written);
    (void)PAL_FileClose(file);
}

void
GpsInitializeCallback(const void* userData, PAL_Error error, const char* errorInfoXml)
{
    WriteLogFileMessage("GpsInitializeCallback() %s", errorInfoXml);

    if (g_deviceStateEvent && error == PAL_Ok)
    {
        SetCallbackCompletedEvent(g_deviceStateEvent);
    }
}

void
GpsDeviceStateCallback(const void* userData, const char* stateChangeXml, PAL_Error error, const char* errorInfoXml)
{
    WriteLogFileMessage("GpsDeviceStateCallback() %X", error);
}

void
GpsLocationCallback(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml)
{
    //WriteLogFileMessage("GpsLocationCallback(%f, %f)", location->latitude, location->longitude);

    if (g_locationEvent)
    {
        SetCallbackCompletedEvent(g_locationEvent);
    }
}


// Test functions ...........................................................

void
TestGpsCreateDestroy(void)
{
    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
}

void
TestGpsIdGetLocation(void)
{
    const ABPAL_GpsConfig config[] =
    {
       "providertype",     "gpsid",
    };

    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestGpsIdGetLocation test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestGpsIdGetLocation timed out on initialize");
    }

    criteria.desiredAccuracy = 150;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 16;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 30;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsGetLocation(gpsContext, &criteria, GpsLocationCallback, g_locationEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_locationEvent, GET_LOCATION_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsGetLocation timed out!");
        CU_FAIL("TestGpsIdGetLocation timed out on get location");
    }

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestGpsIdGetLocation test finished.........");

    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}


void
TestGpsIdTracking(void)
{
    const char *path = [[[NSBundle mainBundle] pathForResource:@"TEST" ofType:@"GPS"] cStringUsingEncoding:NSUTF8StringEncoding];
    const ABPAL_GpsConfig config[] =
    {
        "gpsfilename", path,
    };
    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };
    ABPAL_GpsTrackingInfo trackingInfo = { 0 };

    HANDLE waitEvent = CreateCallbackCompletedEvent();

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestGpsIdTracking test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestGpsIdTracking timed out on initialize");
    }

    criteria.desiredAccuracy = 150;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 16;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 60;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsBeginTracking(gpsContext, FALSE, &criteria, GpsLocationCallback, g_locationEvent, &trackingInfo);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    WaitForCallbackCompletedEvent(waitEvent, TRACKING_TIME_MSEC);

    err = ABPAL_GpsEndTracking(gpsContext, &trackingInfo);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestGpsIdTracking test finished.........");

    DestroyCallbackCompletedEvent(waitEvent);
    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}


void
TestGpsLpsGetLocation(void)
{
    const ABPAL_GpsConfig config[] =
    {
        "ProviderType",     "LPSGPSID",
        "ClientID",         "12569644",
        "ClientName",       "VZ Navigator",
        "ClientPWD",        "0E71199A9D8E9E0A67F6C1B84A47314B",
        "SMSPrefix",        "//VZW-NAV00001",
    };
    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestGpsLpsGetLocation test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestGpsLpsGetLocation timed out on initialize");
    }

    criteria.desiredAccuracy = 50;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 16;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 0;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsGetLocation(gpsContext, &criteria, GpsLocationCallback, g_locationEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_locationEvent, GET_LOCATION_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsGetLocation timed out!");
        CU_FAIL("TestGpsLpsGetLocation timed out on get location");
    }

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestGpsLpsGetLocation test finished.........");

    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}


void
TestGpsLpsTracking(void)
{
    const ABPAL_GpsConfig config[] =
    {
        "ProviderType",     "LPSGPSID",
        "ClientID",         "12569644",
        "ClientName",       "VZ Navigator",
        "ClientPWD",        "0E71199A9D8E9E0A67F6C1B84A47314B",
        "SMSPrefix",        "//VZW-NAV00001",
    };
    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };
    ABPAL_GpsTrackingInfo trackingInfo = { 0 };

    HANDLE waitEvent = CreateCallbackCompletedEvent();

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestGpsLpsTracking test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestGpsLpsTracking timed out on initialize");
    }

    criteria.desiredAccuracy = 50;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 16;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 0;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsBeginTracking(gpsContext, FALSE, &criteria, GpsLocationCallback, g_locationEvent, &trackingInfo);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    WaitForCallbackCompletedEvent(waitEvent, TRACKING_TIME_MSEC);

    err = ABPAL_GpsEndTracking(gpsContext, &trackingInfo);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestGpsLpsTracking test finished.........");

    DestroyCallbackCompletedEvent(waitEvent);
    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}

void
TestAGpsIdGetLocation(void)
{
    const ABPAL_GpsConfig config[] =
    {
        "ProviderType",     "AGPSID",
        //"PDEIPAddress",        "207.114.133.79",
        //"PDEPort",            "8009",
        "PDEIPAddress",        "216.198.139.92", //TELUS PDE
        "PDEPort",            "8889",
        "DeviceName",        "LGE-GW825",
        "RoamingModes",        "MSB",
    };
    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestAGpsIdGetLocation test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestAGpsIdGetLocation timed out on initialize");
    }

    criteria.desiredAccuracy = 1000;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 20;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 0;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsGetLocation(gpsContext, &criteria, GpsLocationCallback, g_locationEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_locationEvent, GET_LOCATION_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsGetLocation timed out!");
        CU_FAIL("TestAGpsIdGetLocation timed out on get location");
    }

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestAGpsIdGetLocation test finished.........");

    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}

void
TestAGpsIdGetLocationMSA(void)
{
    const ABPAL_GpsConfig config[] =
    {
        "ProviderType",     "AGPSID",
        //"PDEIPAddress",        "207.114.133.79",
        //"PDEPort",            "8009",
        "PDEIPAddress",        "216.198.139.92", //TELUS PDE
        "PDEPort",            "8889",
        "DeviceName",        "LGE-GW825",
        "testmodes",        "forcemsa",
    };
    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestAGpsIdGetLocationMSA test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestAGpsIdGetLocationMSA timed out on initialize");
    }

    criteria.desiredAccuracy = 1000;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 20;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 0;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsGetLocation(gpsContext, &criteria, GpsLocationCallback, g_locationEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_locationEvent, MSA_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsGetLocation timed out!");
        CU_FAIL("TestAGpsIdGetLocationMSA timed out on get location");
    }

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestAGpsIdGetLocationMSA test finished.........");

    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}


void
TestAGpsIdTracking(void)
{
    const ABPAL_GpsConfig config[] =
    {
        "ProviderType",     "AGPSID",
        //"PDEIPAddress",        "207.114.133.79", //Qualcomm PDE
        //"PDEPort",            "8009",
        "PDEIPAddress",        "216.198.139.92", //TELUS PDE
        "PDEPort",            "8889",
        "DeviceName",        "LGE-GW825",
        "RoamingModes",        "all",
    };
    const int configSize = (sizeof(config) / sizeof(config[0]));

    PAL_Error err = PAL_Ok;
    PAL_Instance* pal = 0;

    ABPAL_GpsContext* gpsContext = NULL;
    ABPAL_GpsCriteria criteria = { 0 };
    ABPAL_GpsTrackingInfo trackingInfo = { 0 };

    HANDLE waitEvent = CreateCallbackCompletedEvent();

    g_deviceStateEvent = CreateCallbackCompletedEvent();
    g_locationEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);
    g_pal = pal;

    WriteLogFileMessage("TestAGpsIdTracking test starting.........");

    err = ABPAL_GpsCreate(pal, &gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_PTR_NOT_NULL(gpsContext);

    err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitializeCallback, GpsDeviceStateCallback, gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(g_deviceStateEvent, INITIALIZE_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        WriteLogFileMessage("GpsInitialize timed out!");
        CU_FAIL("TestAGpsIdTracking timed out on initialize");
    }

    criteria.desiredAccuracy = 1000;
    criteria.desiredInformation = PGI_Location | PGI_Velocity | PGI_Altitude;
    criteria.desiredInterval = 1 * 1000;
    criteria.desiredMaxAge = 0;
    criteria.desiredNetworkAssist = TRUE;
    criteria.desiredPerformance = 20;
    criteria.desiredSetting = PGC_Interval; // PGC_Accuracy | PGC_Performance | PGC_Interval | PGC_Timeout | PGC_MaxAge;
    criteria.desiredTimeout = 0;
    criteria.desiredTotalFixes = 14401;

    err = ABPAL_GpsBeginTracking(gpsContext, FALSE, &criteria, GpsLocationCallback, g_locationEvent, &trackingInfo);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    WaitForCallbackCompletedEvent(waitEvent, TRACKING_TIME_MSEC);

    err = ABPAL_GpsEndTracking(gpsContext, &trackingInfo);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_GpsDestroy(gpsContext);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);

    WriteLogFileMessage("TestAGpsIdTracking test finished.........");

    DestroyCallbackCompletedEvent(waitEvent);
    DestroyCallbackCompletedEvent(g_deviceStateEvent);
    DestroyCallbackCompletedEvent(g_locationEvent);
}


/*! Add all your test functions here
@return None
*/
void
TestGps_AddAllTests(CU_pSuite pTestSuite)
{
    //CU_add_test(pTestSuite, "TestGpsCreateDestroy", &TestGpsCreateDestroy);
    CU_add_test(pTestSuite, "TestGpsIdGetLocation", &TestGpsIdGetLocation);
    CU_add_test(pTestSuite, "TestGpsIdTracking", &TestGpsIdTracking);
    //CU_add_test(pTestSuite, "TestAGpsIdGetLocation", &TestAGpsIdGetLocation);
//    CU_add_test(pTestSuite, "TestAGpsIdGetLocationMSA", &TestAGpsIdGetLocationMSA);
//    CU_add_test(pTestSuite, "TestAGpsIdTracking", &TestAGpsIdTracking);
//    // NOTE: only run LPS test at a time... GpsInitialize for an LPS causes an app restart
//    CU_add_test(pTestSuite, "TestGpsLpsGetLocation", &TestGpsLpsGetLocation);
//    CU_add_test(pTestSuite, "TestGpsLpsTracking", &TestGpsLpsTracking);
}

/*! Add common initialization code here.

@return 0

@see TestGps_SuiteCleanup
*/
int
TestGps_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestGps_SuiteSetup
*/
int
TestGps_SuiteCleanup()
{
    return 0;
}

/*! @} */
