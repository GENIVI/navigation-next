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

    @file     testbatchtool.cpp
*/
/*
    See description in header file.

    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testbatchtoolprocessor.h"

extern "C"
{
    #include "main.h"
    #include "testbatchtool.h"
    #include "palfile.h"
    #include "palclock.h"
}

typedef struct {
    bool makeGolden;
    bool addTimeStamp;
    bool enablePerformanceMeasurement;
    char folderHome[BATCH_STRING_MAX_LEN];
    char fileLog[BATCH_STRING_MAX_LEN];
    char fileGeocodeOut[BATCH_STRING_MAX_LEN];
    char fileGeocodeIn[BATCH_STRING_MAX_LEN];
    char fileGeocodeGolden[BATCH_STRING_MAX_LEN];
    char fileGeocodeComparison[BATCH_STRING_MAX_LEN];
    char fileReverseGeocodeOut[BATCH_STRING_MAX_LEN];
    char fileReverseGeocodeIn[BATCH_STRING_MAX_LEN];
    char fileReverseGeocodeGolden[BATCH_STRING_MAX_LEN];
    char fileReverseGeocodeComparison[BATCH_STRING_MAX_LEN];
    char filePOISearchOut[BATCH_STRING_MAX_LEN];
    char filePOISearchIn[BATCH_STRING_MAX_LEN];
    char filePOISearchGolden[BATCH_STRING_MAX_LEN];
    char filePOIComparison[BATCH_STRING_MAX_LEN];
    char filePOICatDictionary[BATCH_STRING_MAX_LEN];
    char fileRouteOut[BATCH_STRING_MAX_LEN];
    char fileRouteIn[BATCH_STRING_MAX_LEN];
    char fileRouteGolden[BATCH_STRING_MAX_LEN];
    char fileNavigationSessionOut[BATCH_STRING_MAX_LEN];
    char fileNavigationSessionIn[BATCH_STRING_MAX_LEN];
    char fileNavigationSessionGolden[BATCH_STRING_MAX_LEN];
} BatchConf;

const uint32 CALLBACK_TIMEOUT = 240 * 1000;
const uint32 DESTROY_TIMEOUT = 3 * 1000;

#define BATCH_CONF_FILE_NAME "bt_config.ini"

static BatchConf g_batchConf = {0};

static void TestBatchToolGeocode(void);
static void TestBatchToolReverseGeocode(void);
static void TestBatchToolPOI(void);
static void TestBatchToolRoute(void);
static void TestBatchToolNavigationSession(void);
static void TestBatchToolSwitchToOnBoard(void);

static char* AddStringPrefix(char* dest, char* prefix);
static bool AddTimeStampToFile(char* fileName);
static bool BatchFileOpen(char* FileName, PAL_File** fpFile, PAL_FileMode mode);
static bool BatchFileClose(PAL_File* fpFile);
static bool BatchFileGetNextString(PAL_File* fpFile, BatchFileString* batchParams);
static bool BatchFileGetConf(char* ConfFileName, BatchConf* batchParams);
static bool BatchFileCreateBackup(char* FileName);

void TestBatchTool_AddTests(CU_pSuite pTestSuite, int level)
{
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestBatchToolGeocode", TestBatchToolGeocode);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestBatchToolReverseGeocode", TestBatchToolReverseGeocode);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestBatchToolPOI", TestBatchToolPOI);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestBatchToolRoute", TestBatchToolRoute);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestBatchToolNavigationSession", TestBatchToolNavigationSession);
};

int TestBatchTool_SuiteSetup()
{
    BatchFileGetConf(BATCH_CONF_FILE_NAME, &g_batchConf);
    PRINTF("*******************\n");
    PRINTF("*  CCC BatchTool  *\n");
    PRINTF("*******************\n\n");

    PRINTF("******************************************\n");
    PRINTF("geocode    in                  :%s\n", g_batchConf.fileGeocodeIn);
    PRINTF("geocode    out                 :%s\n", g_batchConf.fileGeocodeOut);
    PRINTF("geocode    gold                :%s\n", g_batchConf.fileGeocodeGolden);
    PRINTF("rgeocode   in                  :%s\n", g_batchConf.fileReverseGeocodeIn);
    PRINTF("rgeocode   out                 :%s\n", g_batchConf.fileReverseGeocodeOut);
    PRINTF("rgeocode   gold                :%s\n", g_batchConf.fileReverseGeocodeGolden);
    PRINTF("POISearch  in                  :%s\n", g_batchConf.filePOISearchIn);
    PRINTF("POISearch  out                 :%s\n", g_batchConf.filePOISearchOut);
    PRINTF("POISearch  gold                :%s\n", g_batchConf.filePOISearchGolden);
    PRINTF("Make golden                    :%s",   g_batchConf.makeGolden ? "yes\n" : "no\n");
    PRINTF("Add Time Stamp                 :%s",   g_batchConf.addTimeStamp ? "yes\n" : "no\n");
    PRINTF("Enable Performance Measurement :%s",   g_batchConf.enablePerformanceMeasurement ? "yes\n" : "no\n");
    PRINTF("******************************************\n\n");

    return 0;
}


int TestBatchTool_SuiteCleanup()
{
    return 0;
}


void TestBatchToolGeocode(void)
{
    PAL_File *fpFile=NULL;
    BatchFileString batchParams = {0};
    bool result=false;

    if (nsl_strlen(g_batchConf.fileGeocodeIn)==0) {
        PRINTF("\nERROR: Parameters missing. Geocode canceled.\n");
        return;
    }
    if (nsl_strlen(g_batchConf.fileGeocodeOut)==0) {
        PRINTF("\nERROR: Parameters missing. Geocode canceled.\n");
        return;
    }
    if (g_batchConf.makeGolden && nsl_strlen(g_batchConf.fileGeocodeGolden)==0) {
        PRINTF("\nERROR: Parameters missing. Geocode canceled.\n");
        return;
    }

	if (g_batchConf.makeGolden) {
		nsl_strcpy(batchParams.outputFileName, g_batchConf.fileGeocodeGolden);
	} else {
		nsl_strcpy(batchParams.outputFileName, g_batchConf.fileGeocodeOut);
	}

    if (g_batchConf.addTimeStamp) AddTimeStampToFile(batchParams.outputFileName);

    // Create backup of prev file
    BatchFileCreateBackup(batchParams.outputFileName);

    result=BatchFileOpen(g_batchConf.fileGeocodeIn, &fpFile, PFM_Read);
    if (result)
    {
        // Create processor instance
        void* completeEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL_FATAL(completeEvent);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_Begin();
        }
        TestBatchToolProcessor* processor = new TestBatchToolProcessor(completeEvent, &batchParams);
        CU_ASSERT_PTR_NOT_NULL(processor);

        do {
            result=BatchFileGetNextString(fpFile, &batchParams);

            // Skip comment line
            if (batchParams.buf[0]=='#' && result) continue;

            if (result) {
                if (processor->IsInitialized()) {
                    processor->TestBatchToolGeocode();
                    WaitForCallbackCompletedEvent(completeEvent, CALLBACK_TIMEOUT);
                }
            }
        } while (result);

        // Delete processor instanse
        processor->Destroy();
        WaitForCallbackCompletedEvent(completeEvent, DESTROY_TIMEOUT);
        delete processor;
        DestroyCallbackCompletedEvent(completeEvent);
        completeEvent=NULL;

        BatchFileClose(fpFile);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_End();
        }
    } else {
        PRINTF("\nERROR: Can't open input file. Geocode canceled.\n");
    }
}

void TestBatchToolReverseGeocode(void)
{
    PAL_File *fpFile=NULL;
    BatchFileString batchParams = {0};
    bool result=false;

    if (nsl_strlen(g_batchConf.fileReverseGeocodeIn)==0) {
        PRINTF("\nERROR: Parameters missing. ReverseGeocode canceled.\n");
        return;
    }
    if (nsl_strlen(g_batchConf.fileReverseGeocodeOut)==0) {
        PRINTF("\nERROR: Parameters missing. ReverseGeocode canceled.\n");
        return;
    }
    if (g_batchConf.makeGolden && nsl_strlen(g_batchConf.fileReverseGeocodeGolden)==0) {
        PRINTF("\nERROR: Parameters missing. ReverseGeocode canceled.\n");
        return;
    }

	if (g_batchConf.makeGolden) {
		nsl_strcpy(batchParams.outputFileName, g_batchConf.fileReverseGeocodeGolden);
	} else {
		nsl_strcpy(batchParams.outputFileName, g_batchConf.fileReverseGeocodeOut);
	}

    if (g_batchConf.addTimeStamp) AddTimeStampToFile(batchParams.outputFileName);

    // Create backup of prev file
    BatchFileCreateBackup(batchParams.outputFileName);

    result=BatchFileOpen(g_batchConf.fileReverseGeocodeIn, &fpFile, PFM_Read);
    if (result) {
        // Create processor instance
        void* completeEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL_FATAL(completeEvent);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_Begin();
        }
        TestBatchToolProcessor* processor = new TestBatchToolProcessor(completeEvent, &batchParams);
        CU_ASSERT_PTR_NOT_NULL(processor);

        do {
            result=BatchFileGetNextString(fpFile, &batchParams);

            // Skip comment line
            if (batchParams.buf[0]=='#' && result) continue;

            if (result) {
                if (processor->IsInitialized()) {
                    processor->TestBatchToolReverseGeocode();
                    WaitForCallbackCompletedEvent(completeEvent, CALLBACK_TIMEOUT);
                }
            }
        } while (result);

        // Delete processor instanse
        processor->Destroy();
        WaitForCallbackCompletedEvent(completeEvent, DESTROY_TIMEOUT);
        delete processor;
        DestroyCallbackCompletedEvent(completeEvent);
        completeEvent=NULL;

        BatchFileClose(fpFile);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_End();
        }
    } else {
        PRINTF("\nERROR: Can't open input file. ReverseGeocode canceled.\n");
    }
}

void TestBatchToolPOI(void)
{
    PAL_File*       fpFile = NULL;
    BatchFileString batchParams = {0};
    bool            result = false;

    if (nsl_strlen(g_batchConf.filePOISearchIn)==0) {
        PRINTF("\nERROR: Parameters missing. POISearch canceled.\n");
        return;
    }
    if (nsl_strlen(g_batchConf.filePOISearchOut)==0) {
        PRINTF("\nERROR: Parameters missing. POISearch canceled.\n");
        return;
    }
    if (g_batchConf.makeGolden && nsl_strlen(g_batchConf.filePOISearchGolden)==0) {
        PRINTF("\nERROR: Parameters missing. POISearch canceled.\n");
        return;
    }

	if (g_batchConf.makeGolden) {
		nsl_strcpy(batchParams.outputFileName, g_batchConf.filePOISearchGolden);
	} else {
		nsl_strcpy(batchParams.outputFileName, g_batchConf.filePOISearchOut);
	}

    if (g_batchConf.addTimeStamp) AddTimeStampToFile(batchParams.outputFileName);

    // Create backup of prev file
    BatchFileCreateBackup(batchParams.outputFileName);

    result = BatchFileOpen(g_batchConf.filePOISearchIn, &fpFile, PFM_Read);

    if (result)
    {
        // Create processor instance
        void* completeEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL_FATAL(completeEvent);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_Begin();
        }
        TestBatchToolProcessor* processor = new TestBatchToolProcessor(completeEvent, &batchParams);
        CU_ASSERT_PTR_NOT_NULL(processor);

        do
        {
            result=BatchFileGetNextString(fpFile, &batchParams);

            // Skip comment line
            if (batchParams.buf[0]=='#' && result) continue;

            if (result)
            {
                if (processor->IsInitialized()) {
                    processor->TestBatchToolPOI();
                    WaitForCallbackCompletedEvent(completeEvent, CALLBACK_TIMEOUT);
                }
            }
        } while (result);

        // Delete processor instanse
        processor->Destroy();
        WaitForCallbackCompletedEvent(completeEvent, DESTROY_TIMEOUT);
        delete processor;
        DestroyCallbackCompletedEvent(completeEvent);
        completeEvent=NULL;

        BatchFileClose(fpFile);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_End();
        }
    }
    else
    {
        PRINTF("\nERROR: Can't open input file. POI canceled.\n");
    }
}

void TestBatchToolRoute(void)
{
    PAL_File*       fpFile = NULL;
    BatchFileString batchParams = {0};
    bool            result = false;

    if (nsl_strlen(g_batchConf.fileRouteIn)==0) {
        PRINTF("\nERROR: Parameters missing. Route canceled.\n");
        return;
    }
    if (nsl_strlen(g_batchConf.fileRouteOut)==0) {
        PRINTF("\nERROR: Parameters missing. Route canceled.\n");
        return;
    }
    if (g_batchConf.makeGolden && nsl_strlen(g_batchConf.fileRouteGolden)==0) {
        PRINTF("\nERROR: Parameters missing. Route canceled.\n");
        return;
    }

        if (g_batchConf.makeGolden) {
            nsl_strcpy(batchParams.outputFileName, g_batchConf.fileRouteGolden);
        } else {
            nsl_strcpy(batchParams.outputFileName, g_batchConf.fileRouteOut);
        }

    if (g_batchConf.addTimeStamp) AddTimeStampToFile(batchParams.outputFileName);

    // Create backup of prev file
    BatchFileCreateBackup(batchParams.outputFileName);

    result = BatchFileOpen(g_batchConf.fileRouteIn, &fpFile, PFM_Read);

    if (result)
    {
        // Create processor instance
        void* completeEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL_FATAL(completeEvent);

        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_Begin();
        }

        TestBatchToolProcessor* processor = new TestBatchToolProcessor(completeEvent, &batchParams);
        CU_ASSERT_PTR_NOT_NULL(processor);

        do
        {
            result=BatchFileGetNextString(fpFile, &batchParams);

            // Skip comment line
            if (batchParams.buf[0]=='#' && result) continue;

            if (result)
            {
                if (processor->IsInitialized()) {
                    processor->TestBatchToolRoute();
                    WaitForCallbackCompletedEvent(completeEvent, CALLBACK_TIMEOUT);
                }
            }
        } while (result);

        // Delete processor instanse
        processor->Destroy();
        WaitForCallbackCompletedEvent(completeEvent, DESTROY_TIMEOUT);
        delete processor;
        DestroyCallbackCompletedEvent(completeEvent);
        completeEvent=NULL;

        BatchFileClose(fpFile);
        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_End();
        }
    }
    else
    {
        PRINTF("\nERROR: Can't open input file. Route canceled.\n");
    }
}

void TestBatchToolNavigationSession(void)
{
    PAL_File*       fpFile = NULL;
    BatchFileString batchParams = {0};
    bool            result = false;

    if (nsl_strlen(g_batchConf.fileNavigationSessionIn)==0) {
        PRINTF("\nERROR: fileNavigationSessionIn parameters missing. NavigationSession canceled.\n");
        return;
    }
    if (nsl_strlen(g_batchConf.fileNavigationSessionOut)==0) {
        PRINTF("\nERROR: fileNavigationSessionOut parameters missing. NavigationSession canceled.\n");
        return;
    }
    if (g_batchConf.makeGolden && nsl_strlen(g_batchConf.fileNavigationSessionGolden)==0) {
        PRINTF("\nERROR: fileNavigationSessionGolden parameters missing. NavigationSession canceled.\n");
        return;
    }

	if (g_batchConf.makeGolden) {
		nsl_strncpy(batchParams.outputFileName, g_batchConf.fileNavigationSessionGolden, sizeof(batchParams.outputFileName));
	} else {
		nsl_strncpy(batchParams.outputFileName, g_batchConf.fileNavigationSessionOut, sizeof(batchParams.outputFileName));
	}

    if (g_batchConf.addTimeStamp)
    {
        AddTimeStampToFile(batchParams.outputFileName);
    }

    // Create backup of prev file
    BatchFileCreateBackup(batchParams.outputFileName);

    result = BatchFileOpen(g_batchConf.fileNavigationSessionIn, &fpFile, PFM_Read);

    if (result)
    {
        // Create processor instance
        void* completeEvent = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL_FATAL(completeEvent);

        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_Begin();
        }

        TestBatchToolProcessor* processor = new TestBatchToolProcessor(completeEvent, &batchParams);
        CU_ASSERT_PTR_NOT_NULL(processor);

        do
        {
            result = BatchFileGetNextString(fpFile, &batchParams);

            // Skip comment line
            if (batchParams.buf[0]=='#' && result) continue;

            if (result)
            {
                if (processor->IsInitialized()) {
                    processor->TestBatchToolNavigationSession();
                    WaitForCallbackCompletedEvent(completeEvent, CALLBACK_TIMEOUT);
                }
            }
        } while (result);

        // Delete processor instanse
        processor->Destroy();
        WaitForCallbackCompletedEvent(completeEvent, DESTROY_TIMEOUT);
        delete processor;
        DestroyCallbackCompletedEvent(completeEvent);
        completeEvent=NULL;

        BatchFileClose(fpFile);

        if (g_batchConf.enablePerformanceMeasurement)
        {
            PerformanceMeasurement_End();
        }
    }
    else
    {
        PRINTF("\nERROR: Can't open input file. NavigationSession canceled.\n");
    }
}

// ==================================================
// Utility functions
// ==================================================
bool BatchFileOpen(char* FileName, PAL_File** fpFile, PAL_FileMode mode)
{
    PAL_Error fileErr = PAL_Ok;

    PAL_Instance *pal = PAL_CreateInstance();
    if (pal) {
        char fullFileName[BATCH_STRING_MAX_LEN] = {0};
        nsl_strlcat(fullFileName, GetDocumentsDirectoryPath(), sizeof(fullFileName));
        nsl_strlcat(fullFileName, FileName, sizeof(fullFileName));
        fileErr=PAL_FileOpen(pal, fullFileName, mode, fpFile);
        PAL_Destroy(pal);
        if (fileErr==PAL_Ok) return true;
    }
    return false;
}

bool BatchFileClose(PAL_File* fpFile)
{
    PAL_Error fileErr = PAL_Ok;
    fileErr = PAL_FileClose(fpFile);
    if (fileErr==PAL_Ok) return true;
    else return false;
}

bool AddTimeStampToFile(char* fileName)
{
    char buf[BATCH_STRING_MAX_LEN]={0};
    PAL_Error result=PAL_Ok;
    PAL_ClockDateTime date={0};
    char* point=NULL;

    result=PAL_ClockGetDateTime(&date);
    if (result!=PAL_Ok) return false;

    nsl_sprintf(buf,"_%04d%02d%02d-%02d%02d%02d",date.year, date.month, date.day, date.hour, date.minute, date.second);
    point=nsl_strrchr(fileName,'.');
    if (point)
    {
        nsl_strcat(buf,point);
        *point='\0';
    }
    nsl_strcat(fileName,buf);
    return true;
}

bool BatchFileGetConf(char* ConfFileName, BatchConf* batchConf)
{
    PAL_File* fp=NULL;
    bool res=false;
    PAL_Error result = PAL_Ok;
    char buf[BATCH_STRING_MAX_LEN] = {0};
    uint8 bufChar = 0;
    uint32 bytesRead = 0;
    int bufIndex=0;
    unsigned int valueStart=0;

    res = BatchFileOpen(ConfFileName, &fp, PFM_Read);

    if (res) {

        do {
            bufIndex=0;
            nsl_memset(buf,0,sizeof(buf));
            do {
                result = PAL_FileRead(fp, &bufChar, 1, &bytesRead);
            } while (bytesRead!=0 && (bufChar=='\n' || bufChar=='\r' || bufChar==' ' || bufChar=='\t'));

            while (bytesRead!=0 && bufChar!='\n' && bufChar!='\r') {
                buf[bufIndex]=bufChar;
                bufIndex++;
                result = PAL_FileRead(fp, &bufChar, 1, &bytesRead);
            }
            buf[bufIndex]='\0';

            // Line analysis
            if (buf[0]!='#') {
                char* separator=NULL;
                separator = nsl_strchr(buf,'=');
                if (separator) {
                    *separator = '\0';
                    valueStart=nsl_strlen(buf)+1;

                    if (!nsl_strcmp(buf,"GeocodeIn")) {
                        nsl_strncpy(batchConf->fileGeocodeIn, &buf[valueStart], sizeof(batchConf->fileGeocodeIn));
                    }
                    if (!nsl_strcmp(buf,"GeocodeOut")) {
                        nsl_strncpy(batchConf->fileGeocodeOut, &buf[valueStart], sizeof(batchConf->fileGeocodeOut));
                    }
                    if (!nsl_strcmp(buf,"GeocodeGolden")) {
                        nsl_strncpy(batchConf->fileGeocodeGolden, &buf[valueStart], sizeof(batchConf->fileGeocodeGolden));
                    }
                    if (!nsl_strcmp(buf,"ReverseGeocodeIn")) {
                        nsl_strncpy(batchConf->fileReverseGeocodeIn, &buf[valueStart], sizeof(batchConf->fileReverseGeocodeIn));
                    }
                    if (!nsl_strcmp(buf,"ReverseGeocodeOut")) {
                        nsl_strncpy(batchConf->fileReverseGeocodeOut, &buf[valueStart], sizeof(batchConf->fileReverseGeocodeOut));
                    }
                    if (!nsl_strcmp(buf,"ReverseGeocodeGolden")) {
                        nsl_strncpy(batchConf->fileReverseGeocodeGolden, &buf[valueStart], sizeof(batchConf->fileReverseGeocodeGolden));
                    }
                    if (!nsl_strcmp(buf,"POIIn")) {
                        nsl_strncpy(batchConf->filePOISearchIn, &buf[valueStart], sizeof(batchConf->filePOISearchIn));
                    }
                    if (!nsl_strcmp(buf,"POIOut")) {
                        nsl_strncpy(batchConf->filePOISearchOut, &buf[valueStart], sizeof(batchConf->filePOISearchOut));
                    }
                    if (!nsl_strcmp(buf,"POIGolden")) {
                        nsl_strncpy(batchConf->filePOISearchGolden, &buf[valueStart], sizeof(batchConf->filePOISearchGolden));
                    }
                    if (!nsl_strcmp(buf,"RouteIn")) {
                        nsl_strncpy(batchConf->fileRouteIn, &buf[valueStart], sizeof(batchConf->fileRouteIn));
                    }
                    if (!nsl_strcmp(buf,"RouteOut")) {
                        nsl_strncpy(batchConf->fileRouteOut, &buf[valueStart], sizeof(batchConf->fileRouteOut));
                    }
                    if (!nsl_strcmp(buf,"RouteGolden")) {
                        nsl_strncpy(batchConf->fileRouteGolden, &buf[valueStart], sizeof(batchConf->fileRouteGolden));
                    }
                    if (!nsl_strcmp(buf,"NavigationSessionIn")) {
                        nsl_strncpy(batchConf->fileNavigationSessionIn, &buf[valueStart], sizeof(batchConf->fileNavigationSessionIn));
                    }
                    if (!nsl_strcmp(buf,"NavigationSessionOut")) {
                        nsl_strncpy(batchConf->fileNavigationSessionOut, &buf[valueStart], sizeof(batchConf->fileNavigationSessionOut));
                    }
                    if (!nsl_strcmp(buf,"NavigationSessionGolden")) {
                        nsl_strncpy(batchConf->fileNavigationSessionGolden, &buf[valueStart], sizeof(batchConf->fileNavigationSessionGolden));
                    }
                    if (!nsl_strcmp(buf,"HomeFolder")) {
                        nsl_strncpy(batchConf->folderHome, &buf[valueStart], sizeof(batchConf->folderHome));
                    }
                    if (!nsl_strcmp(buf,"LogFileName")) {
                        nsl_strncpy(batchConf->fileLog, &buf[valueStart], sizeof(batchConf->fileLog));
                    }
                    if (!nsl_strcmp(buf,"MakeGolden")) {
                        if (nsl_strstr(&buf[valueStart],"yes")) {
                            batchConf->makeGolden=true;
                        } else {
                            batchConf->makeGolden=false;
                        }
                    }
                    if (!nsl_strcmp(buf,"AddTimeStamp")) {
                        if (nsl_strstr(&buf[valueStart],"yes")) {
                            batchConf->addTimeStamp=true;
                        } else {
                            batchConf->addTimeStamp=false;
                        }
                    }
                    if (!nsl_strcmp(buf,"EnablePerformanceMeasurement")) {
                        if (nsl_strstr(&buf[valueStart],"yes")) {
                            batchConf->enablePerformanceMeasurement=true;
                        } else {
                            batchConf->enablePerformanceMeasurement=false;
                        }
                    }
                }
            }
            nsl_strchr(buf,'#');

        } while (bytesRead!=0);

        BatchFileClose(fp);
    }
    else
    {
        return false;
    }

    // Add home path to all file names
    if (nsl_strlen(batchConf->folderHome))
    {
        if (batchConf->fileGeocodeIn[0]) AddStringPrefix(batchConf->fileGeocodeIn, batchConf->folderHome);
        if (batchConf->fileGeocodeOut[0]) AddStringPrefix(batchConf->fileGeocodeOut, batchConf->folderHome);
        if (batchConf->fileGeocodeGolden[0]) AddStringPrefix(batchConf->fileGeocodeGolden, batchConf->folderHome);

        if (batchConf->fileReverseGeocodeIn[0]) AddStringPrefix(batchConf->fileReverseGeocodeIn, batchConf->folderHome);
        if (batchConf->fileReverseGeocodeOut[0]) AddStringPrefix(batchConf->fileReverseGeocodeOut, batchConf->folderHome);
        if (batchConf->fileReverseGeocodeGolden[0]) AddStringPrefix(batchConf->fileReverseGeocodeGolden, batchConf->folderHome);

        if (batchConf->filePOISearchIn[0]) AddStringPrefix(batchConf->filePOISearchIn, batchConf->folderHome);
        if (batchConf->filePOISearchOut[0]) AddStringPrefix(batchConf->filePOISearchOut, batchConf->folderHome);
        if (batchConf->filePOISearchGolden[0]) AddStringPrefix(batchConf->filePOISearchGolden, batchConf->folderHome);

        if (batchConf->fileRouteIn[0]) AddStringPrefix(batchConf->fileRouteIn, batchConf->folderHome);
        if (batchConf->fileRouteOut[0]) AddStringPrefix(batchConf->fileRouteOut, batchConf->folderHome);
        if (batchConf->fileRouteGolden[0]) AddStringPrefix(batchConf->fileRouteGolden, batchConf->folderHome);

        if (batchConf->fileNavigationSessionIn[0]) AddStringPrefix(batchConf->fileNavigationSessionIn, batchConf->folderHome);
        if (batchConf->fileNavigationSessionOut[0]) AddStringPrefix(batchConf->fileNavigationSessionOut, batchConf->folderHome);
        if (batchConf->fileNavigationSessionGolden[0]) AddStringPrefix(batchConf->fileNavigationSessionGolden, batchConf->folderHome);

        if (batchConf->fileLog[0]) AddStringPrefix(batchConf->fileLog, batchConf->folderHome);
    }
    return true;
}

char* AddStringPrefix(char* dest, char* prefix)
{
    char buf[BATCH_STRING_MAX_LEN] = {0};

    nsl_strcpy(buf, prefix);
    nsl_strcat(buf,dest);
    nsl_strcpy(dest,buf);

    return dest;
}

bool BatchFileCreateBackup(char* FileName) {
    char BackupFileName[BATCH_STRING_MAX_LEN]={0};
    nsl_strcpy(BackupFileName,FileName);
    nsl_strcat(BackupFileName,".bac");

    PAL_Error result = PAL_Ok;

    PAL_Instance *pal = PAL_CreateInstance();
    if (pal) {
        result=PAL_FileExists(pal, FileName);

        if (result==PAL_Ok) {
            result=PAL_FileExists(pal, BackupFileName);
            if (result==PAL_Ok) PAL_FileRemove(pal,BackupFileName);
            result=PAL_FileRename(pal,FileName,BackupFileName);
        }
        PAL_Destroy(pal);
        return (result==PAL_Ok);
    }
    return false;
}

int BatchFileGetCaseIDFromString(char* stringInput)
{
    int i = 0, j = 0;
    char caseId[10] = {0};

    while (stringInput[i] != ',')
    {
        if (stringInput[i] != '"')
        {
            caseId[j] = stringInput[i];
            j++;
        }

        i++;
    }

    if (nsl_strlen(caseId) != 0)
    {
        return nsl_atoi(caseId);
    }
    else
    {
        return -1;
    }
}

///////////////////////////////////////////////////////////////
///CSV PARSER
///////////////////////////////////////////////////////////////
bool BatchFileGetNextString(PAL_File* fpFile, BatchFileString* batchParams)
{
    char buffer[BATCH_STRING_MAX_LEN+1] = {0};
    uint8 bufChar = 0;
    uint32 bytesRead = 0;
    int bufIndex=0;
    unsigned short j=0;
    unsigned short i=0;
    unsigned short iStartParam=0;
    unsigned short iParamNum=0;
    PAL_Error result = PAL_Ok;

    // Clear params
    nsl_memset(batchParams->buf,0,sizeof(batchParams->buf));
    nsl_memset(batchParams->params,0,sizeof(batchParams->params));

    // Get string from file
    bufIndex=0;
    nsl_memset(buffer,0,sizeof(buffer));
    do {
        result = PAL_FileRead(fpFile, &bufChar, 1, &bytesRead);
    } while (bytesRead!=0 && (bufChar=='\n' || bufChar=='\r' || bufChar==' ' || bufChar=='\t'));

    if (bytesRead==0) return false;

    while (bytesRead!=0 && bufChar!='\n' && bufChar!='\r') {
        buffer[bufIndex]=bufChar;
        bufIndex++;
        result = PAL_FileRead(fpFile, &bufChar, 1, &bytesRead);
    }
    buffer[bufIndex]='\0';

    for (i=0,j=0; i<nsl_strlen(buffer); i++) {
        // Param in quotes
        if (buffer[i]=='\"') {
            iStartParam=j;
            do {
                i++;
                if (buffer[i]=='\"') {
                    i++;
                    if (buffer[i]!='\"') break;
                }
                batchParams->buf[j]=buffer[i];
                j++;
            } while (i<nsl_strlen(buffer));
            batchParams->buf[j]='\0';
            j++;
            batchParams->params[iParamNum]=iStartParam;
        }

        // Separator
        if (buffer[i]==',' && i!=0) {
            iParamNum++;
            if (buffer[i+1]==',' || buffer[i+1]=='\0') {
                batchParams->buf[j]='\0';
                batchParams->params[iParamNum]=j;
                j++;
            }
        }

        // Param without quotes
        if ((buffer[i]==',' && buffer[i+1]!='\"' && buffer[i+1]!=',' && buffer[i+1]!='\0') ||
            (i==0 && buffer[i]!='\"' && buffer[i]!=',')) {
            iStartParam=j;
            if (i==0) {
                batchParams->buf[j]=buffer[i];
                j++;
            }
            do {
                if (buffer[i+1]==',') break;
                i++;
                batchParams->buf[j]=buffer[i];
                j++;
            } while (i<nsl_strlen(buffer));
            batchParams->buf[j]='\0';
            j++;
            batchParams->params[iParamNum]=iStartParam;
        }
    }

    // Make all unused parameters position to null chars in buf
    for (i=iParamNum+1; i<BATCH_STRING_MAX_PARAMS; i++) {
        batchParams->params[i]=(unsigned short)nsl_strlen(batchParams->buf);
    }
    return true;
}

