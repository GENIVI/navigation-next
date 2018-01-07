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

    @file     PlatformUtil.c
    @date     02/06/2009
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the Windows platform implementation of the platform-
    specific utilities needed for system testing.
    */
    /*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "PlatformUtil.h"
#include "palstdlib.h"
#include "palconfig.h"
#include "cunit.h"
#include <windows.h>
#include "tapi.h"


#define TAPI_API_LOW_VERSION    0x00020000
#define TAPI_API_HIGH_VERSION   0x00020000

#define CAPS_BUFFER_SIZE        512
/*! Test base path */
#define TEST_BASE_PATH                      "NB_UNITTEST_DATA"

const wchar_t DELIMITER = L'\\';
const wchar_t DELIMITER_STRING[] = L"\\";

Test_Options g_TestOptions;


static HRESULT NIM_GetPhoneNumber(char *szNumber, UINT cchNumber);


void*
CreateCallbackCompletedEvent(void)
{
    return CreateEvent(NULL, FALSE, FALSE, NULL);
}

void
ProcessPendingMessages(void)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

boolean
WaitForCallbackCompletedEvent(void* callbackCompletedEvent, uint32 timeoutMsec)
{
    const uint32 waitTimeSlice = 250;
    DWORD startTime = GetTickCount();

    boolean success = TRUE;
    
    while (success)
    {
        // wait for signalled callback event, timeout or message to process
        DWORD waitResult = MsgWaitForMultipleObjects(1, &callbackCompletedEvent, FALSE, waitTimeSlice, QS_ALLINPUT);
        
        ProcessPendingMessages();

        if (waitResult == WAIT_OBJECT_0)
        {
            // exit successfully if callback event signalled
            break;
        }
        else if (waitResult == WAIT_FAILED)
        {
            // exit unsuccesfully if wait failed
            success = FALSE;
        }
        else
        {
            // exit unsuccesfully if timeout time elapsed
            success = (boolean)((GetTickCount() - startTime) < timeoutMsec);
        }
    }
    
    return success;
}

void
SetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    SetEvent(callbackCompletedEvent);
}

void
ResetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    ResetEvent(callbackCompletedEvent);
}

void
DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    CloseHandle(callbackCompletedEvent);
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = { 0 };
    PAL_Instance* pal = NULL;    
    pal = PAL_Create(&palConfig);
    return pal;
}

void PAL_DestroyInstance(
    PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }
    
    PAL_Destroy(pal);   
}

Test_Options* Test_OptionsCreate()
{
    nsl_memset(&g_TestOptions, 0, sizeof(g_TestOptions));

    g_TestOptions.logLevel = LogLevelNone;
    g_TestOptions.networkType = TestNetworkTcp;
    g_TestOptions.testLevel = TestLevelIntermediate;
    g_TestOptions.overwriteFlag = TestOverwriteFlagFalse;

    return &g_TestOptions;
}

Test_Options* Test_OptionsGet()
{
    return &g_TestOptions;
}

void Test_OptionsDestroy()
{

}


void InitializeHeapCheck()
{
}


void TestHeapCheck(void)
{
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPTSTR    lpCmdLine,
                   int       nCmdShow)
{
    char*       argvarTemp  = NULL;
    char**      argvar      = NULL;
    int         index       = 0;
    int         i           = 0;
    int         errorCount  = 0;
    char*       token       = NULL;
    size_t      qStrLen     = 0;
    const char* progname    = "systemtests";

    qStrLen = wcslen(lpCmdLine) + 1;
    argvarTemp = nsl_malloc(qStrLen);
    if (argvarTemp == NULL)
    {
        PRINTF("%s: insufficient memory\n", progname);
        goto exit;        
    }
    WideCharToMultiByte( CP_ACP, 0, lpCmdLine, qStrLen, argvarTemp, qStrLen, NULL, NULL ); 

    argvar = nsl_malloc(qStrLen);
    if (argvar == NULL)
    {
        PRINTF("%s: insufficient memory\n", progname);
        goto exit;
    }

    argvar[index] = nsl_malloc(nsl_strlen(progname));
    if(argvar[index] ==  NULL)
    {
        PRINTF("%s: insufficient memory\n", progname);
        goto exit;
    }
    nsl_strcpy(argvar[index++], progname);

    token = strtok(argvarTemp, " ");
    while ( token != '\0' )
    {
        argvar[index] = nsl_malloc(nsl_strlen(token));
        if (argvar == NULL)
        {
            PRINTF("%s: insufficient memory\n", progname);
            goto exit;
        }
        nsl_strcpy(argvar[index], token);

        token = strtok(NULL, " ");
        index++;
    }

    errorCount = test_main(index, argvar);

exit:
    for (i = 0; i < index; ++i)
    {
        nsl_free(argvar[i]);
    }
    nsl_free (argvar); 
    nsl_free(argvarTemp); 

    return errorCount;

}


uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    uint64 mdn = 0;

    if (!RunningInEmulator())
    {
        char mdnBuf[20] = { 0 };

        HRESULT hr = NIM_GetPhoneNumber(mdnBuf, sizeof(mdnBuf));
        if (!hr)
        {
            mdn = nsl_strtouint64(mdnBuf);
        }
    }

    if (mdn == 0)
    {
        mdn = GetMobileDirectoryNumberFromFile(pal);
    }

    return mdn;
}


/// @todo Verify that this actually works as it is a copy from wm4main/main/nimbrewsdk/main/nimbrewwm.c  
HRESULT NIM_GetPhoneNumber(char *szNumber, UINT cchNumber)
{
#define CELLTSP_LINENAME_STRING (L"Cellular Line")

    HRESULT  hr = E_FAIL;
    LRESULT  lResult = 0;
    HLINEAPP hLineApp;
    DWORD    dwNumDevs;  //number of line devices
    DWORD    dwAPIVersion = TAPI_API_HIGH_VERSION;
    LINEINITIALIZEEXPARAMS liep;

    DWORD dwTAPILineDeviceID;
    const DWORD dwAddressID =0 ; // Get first line

    liep.dwTotalSize = sizeof(liep);
    liep.dwOptions   = LINEINITIALIZEEXOPTION_USEEVENT;

    //initialize line before accessing
    if (SUCCEEDED(lineInitializeEx(&hLineApp, 0, 0, TEXT("ExTapi_Lib"), &dwNumDevs, &dwAPIVersion, &liep)))
    {

        BYTE* pCapBuf = NULL;
        DWORD dwCapBufSize = CAPS_BUFFER_SIZE;
        LINEEXTENSIONID  LineExtensionID;
        LINEDEVCAPS*     pLineDevCaps = NULL;
        LINEADDRESSCAPS* placAddressCaps = NULL;
        DWORD dwCurrentDevID = 0;
        pCapBuf = nsl_malloc(dwCapBufSize);
        if (pCapBuf == NULL)
            goto ExitErr;

        pLineDevCaps = (LINEDEVCAPS*)pCapBuf;
        pLineDevCaps->dwTotalSize = dwCapBufSize;

        // Get TSP Line Device ID
        dwTAPILineDeviceID = 0xffffffff;
        for (dwCurrentDevID = 0 ; dwCurrentDevID < dwNumDevs ; dwCurrentDevID++)
        {
            //ensure TAPI, service provider, and application are all using the same versions
            if (0 == lineNegotiateAPIVersion(hLineApp, dwCurrentDevID, TAPI_API_LOW_VERSION, TAPI_API_HIGH_VERSION,
                &dwAPIVersion, &LineExtensionID))
            {
                lResult = lineGetDevCaps(hLineApp, dwCurrentDevID, dwAPIVersion, 0, pLineDevCaps);

                //increase buffer size if too small to hold the device capabilities
                if (dwCapBufSize < pLineDevCaps->dwNeededSize)
                {
                    nsl_free(pCapBuf);
                    dwCapBufSize = pLineDevCaps->dwNeededSize;
                    pCapBuf = nsl_malloc(dwCapBufSize);
                    if (pCapBuf == NULL)
                        goto ExitErr;

                    pLineDevCaps = (LINEDEVCAPS*)pCapBuf;
                    pLineDevCaps->dwTotalSize = dwCapBufSize;

                    lResult = lineGetDevCaps(hLineApp, dwCurrentDevID, dwAPIVersion, 0, pLineDevCaps);
                }
                //lResult of 0 means the device capabilities were successfully returned
                if ((0 == lResult) &&
                    (0 == _tcscmp((TCHAR*)((BYTE*)pLineDevCaps+pLineDevCaps->dwLineNameOffset), CELLTSP_LINENAME_STRING)))
                {
                    dwTAPILineDeviceID = dwCurrentDevID;
                    break;
                }
            } 
        } 

        placAddressCaps = (LINEADDRESSCAPS*)pCapBuf;
        placAddressCaps->dwTotalSize = dwCapBufSize;

        lResult = lineGetAddressCaps(hLineApp, dwTAPILineDeviceID, dwAddressID, dwAPIVersion, 0, placAddressCaps);

        //increase buffer size if too small to hold the address capabilities
        if (dwCapBufSize < placAddressCaps->dwNeededSize)
        {
            nsl_free(pCapBuf);
            dwCapBufSize = placAddressCaps->dwNeededSize;
            pCapBuf = nsl_malloc(dwCapBufSize);
            if (pCapBuf == NULL)
                goto ExitErr;

            placAddressCaps = (LINEADDRESSCAPS*)pCapBuf;
            placAddressCaps->dwTotalSize = dwCapBufSize;

            lResult = lineGetAddressCaps(hLineApp, dwTAPILineDeviceID, dwAddressID, dwAPIVersion, 0, placAddressCaps);
        }
        //lResult of 0 means the address capabilities were successfully returned
        if (0 == lResult)
        {
            if (szNumber)
            {
                WCHAR *tsAddress = NULL;				
                szNumber[0] = TEXT('\0');

                if (placAddressCaps->dwAddressSize == 0){
                    hr = E_FAIL;
                    goto ExitErr;

                }

                // A non-zero dwAddressSize means a phone number was found
                ASSERT(0 != placAddressCaps->dwAddressOffset);
                tsAddress = (WCHAR*)(((BYTE*)placAddressCaps)+placAddressCaps->dwAddressOffset);
                
                //VZW - there is a limitation on DB to hold 10 digit phone no.
                //stripping off the leading 1
                if (wcslen(tsAddress)> 10 && wcsncmp (tsAddress, _T("1"), 1)==0)
                    wcstombs(szNumber, tsAddress + 1, cchNumber);
                else
                    wcstombs(szNumber, tsAddress, cchNumber);
            }

            hr = S_OK;
        } 

        nsl_free(pCapBuf);
    }

ExitErr:
    lineShutdown(hLineApp);

    return hr;
}


nb_boolean RunningInEmulator()
{
    wchar_t name[256];
    BOOL rc = SystemParametersInfo(SPI_GETPLATFORMNAME, sizeof(name) / sizeof(name[0]), name, 0);
    if (rc)
    {
        rc = (BOOL)(wcscmp(name, L"DeviceEmulator") == 0);
    }

    return (nb_boolean)rc;
}


uint64 GetMobileInformationNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_IDENTIFIER_NUMBER;
}


nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize)
{
    nb_boolean rc = FALSE;
    wchar_t value[256];

    if (SystemParametersInfo(SPI_GETPLATFORMNAME, sizeof(value) / sizeof(value[0]), value, 0))
    {
        rc = (nb_boolean)(WideCharToMultiByte(CP_ACP, 0, value, -1, buffer, bufferSize, 0, 0) != 0);
    }

    return rc;
}

const char* GetBasePath()
{
    return TEST_BASE_PATH;
}

/*! @} */
