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

#include <stdio.h>
#include "gpssm.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef WINCE
#include <windows.h>
#include <winbase.h>
#endif

int test_main()
{
    GpsSecurityModule* securityModule = 0;
    ABPAL_GpsLocation wgsLocations[10] = {{0}};
    ABPAL_GpsLocation chinaLocations[10] = {{0}};
    int i=0;

    wgsLocations[0].gpsTime = 949188050;
    wgsLocations[0].latitude = 39.905994;
    wgsLocations[0].longitude = 116.386562;
    wgsLocations[0].altitude = 16;

    wgsLocations[1].gpsTime = 949188051;
    wgsLocations[1].latitude = 39.906101;
    wgsLocations[1].longitude = 116.38698;
    wgsLocations[1].altitude = 16;

    wgsLocations[2].gpsTime = 949188052;
    wgsLocations[2].latitude = 39.906068;
    wgsLocations[2].longitude = 116.387731;
    wgsLocations[2].altitude = 16;
    
    wgsLocations[3].gpsTime = 949188053;
    wgsLocations[3].latitude = 39.906035;
    wgsLocations[3].longitude = 116.388482;
    wgsLocations[3].altitude = 16;

    wgsLocations[4].gpsTime = 949188054;
    wgsLocations[4].latitude = 39.90606;
    wgsLocations[4].longitude = 116.389394;
    wgsLocations[4].altitude = 16;

    wgsLocations[5].gpsTime = 949188055;
    wgsLocations[5].latitude = 39.906134;
    wgsLocations[5].longitude = 116.390446;
    wgsLocations[5].altitude = 16;

    wgsLocations[6].gpsTime = 949188056;
    wgsLocations[6].latitude = 39.906175;
    wgsLocations[6].longitude = 116.391197;
    wgsLocations[6].altitude = 16;

    wgsLocations[7].gpsTime = 949188057;
    wgsLocations[7].latitude = 39.906159;
    wgsLocations[7].longitude = 116.392366;
    wgsLocations[7].altitude = 16;

    wgsLocations[8].gpsTime = 949188058;
    wgsLocations[8].latitude = 39.906183;
    wgsLocations[8].longitude = 116.393793;
    wgsLocations[8].altitude = 16;

    wgsLocations[9].gpsTime = 949188059;
    wgsLocations[9].latitude = 39.906208;
    wgsLocations[9].longitude = 116.394823;
    wgsLocations[9].altitude = 16;

    if (GpsSecurityModuleCreate(&securityModule) != PAL_Ok)
    {
        printf("failed to create security module!");
        return 1;
    }

    for (i=0; i<10; i++)
    {
        printf("%d) Latitude: %f, Longitude: %f\n", i+1, wgsLocations[i].latitude, wgsLocations[i].longitude);

        if (GpsSecurityModuleEncrypt(securityModule, &wgsLocations[i], &chinaLocations[i]) != PAL_Ok)
        {
            printf("failed to destroy security module!");
            return 2;
        }
        else 
        {
            printf("(%f, %f) -> (%f, %f)\n", wgsLocations[i].latitude, wgsLocations[i].longitude,
                                           chinaLocations[i].latitude, chinaLocations[i].longitude);
        }
    }

    if (GpsSecurityModuleDestroy(securityModule) != PAL_Ok)
    {
        printf("failed to destroy security module!");
        return 3;
    }

    return 0;
}

#ifdef WINCE
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    int ret = 0;
    HRESULT hr = E_FAIL;
    
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        ret = test_main();
        CoUninitialize();
    }
    else
    {
        ret = -1;
    }
    return ret;
}
#elif defined(WIN32)
#include "tchar.h"
int _tmain(int argc, _TCHAR* argv[])
{
    int ret = 0;
    HRESULT hr = E_FAIL;

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        ret = test_main();
        CoUninitialize();
    }
    else
    {
        ret = -1;
    }
    return ret;
}
#else
int main(int argc, char* argv[])
{
    return test_main();
}
#endif


