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

#include "nbgmtestsuitefactory.h"
#include "nbgmtestsuite.h"
#include "utility.h"
#include "palfile.h"
#include "tchar.h"
#include "json.h"
#include "winutility.h"

NBRE_TestCase* finalSuites[255];
int finalSuitCount = 0;
static int testLevel = 0;
static int groupCount = 0;
static char** suitGroupeNames;
static CU_BasicRunMode testRunMode = CU_BRM_VERBOSE;


static std::string readInputTestFile( const char *path )
{
    FILE *file = fopen( path, "rb" );
    if ( !file )
        return std::string("");
    fseek( file, 0, SEEK_END );
    long size = ftell( file );
    fseek( file, 0, SEEK_SET );
    std::string text;
    char *buffer = new char[size+1];
    buffer[size] = 0;
    if ( fread( buffer, 1, size, file ) == (unsigned long)size )
        text = buffer;
    fclose( file );
    delete[] buffer;
    return text;
}

void GetConfigInfo()
{
    char* exeDir = GetExePath();
    const char* temp = NULL;
    const char* configFolder = "config\\config.json";
    char* configFullPath = (char*)nsl_malloc((nb_usize)(strlen(exeDir)+strlen(configFolder)+1));
    nsl_strcpy(configFullPath, exeDir);
    nsl_strcat(configFullPath, configFolder);
    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;
    std::string json_document = readInputTestFile(configFullPath);
    bool parsingSuccessful = reader.parse(json_document, root); 
    if ( !parsingSuccessful )
    {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse configuration\n" << reader.getFormatedErrorMessages();
        return;
    }

    const Json::Value sList = root["SUITE_LIST"];

    suitGroupeNames = new char*[sList.size()];
    for (size_t i=0; i < sList.size(); i++){
        suitGroupeNames[i] = new char[50];
        memset(suitGroupeNames[i], 0, 50);
    }
    //get suite name
    for (size_t index = 0; index < sList.size(); ++index)
    {
        nsl_strcpy(suitGroupeNames[index],sList[index].asString().c_str());
    }
    temp = root["TEST_LEVEL"].asString().c_str();
    //get test level
    if (nsl_strcmp(temp,"SMOKE")== 0)
        testLevel = 0;
    if (nsl_strcmp(temp,"INTERMIDATE")== 0)
        testLevel = 1;
    if (nsl_strcmp(temp,"REGRESSION")== 0)
        testLevel = 2;
    //get suite count
    groupCount = sList.size();
    //get run mode
    temp = root["TEST_RUN_MODE"].asString().c_str();
    if (nsl_strcmp(temp,"NORMAL")== 0)
        testRunMode = CU_BRM_NORMAL;
    if (nsl_strcmp(temp,"SILENT")== 0)
        testRunMode = CU_BRM_NORMAL;
    if (nsl_strcmp(temp,"VERBOSE")== 0)
        testRunMode = CU_BRM_VERBOSE;

}

int main(int /*argc*/, char* /*argv*/[])
{
    PAL_Instance* pal = CreatePal();
    PAL_File* configFile = NULL;
    PAL_File* palFileResult = NULL;

    //test suite configuration
    //create a new test log file
    PAL_FileOpen(pal, RESULT_FILE_PATH, PFM_Create, &palFileResult);
    PAL_FileClose(palFileResult);
    palFileResult = NULL;

    
    //setup predefined test suites
    //run test cases
    NBRE_TestSuiteFactory suiteFactory;
    //get config information
    GetConfigInfo();
    CU_initialize_registry();
    for(int i=0; i<groupCount; ++i)
    {
        NBRE_TestSuite*  nbgmSuite = suiteFactory.GetTestSuite(suitGroupeNames[i]);
        if(nbgmSuite != NULL)
        {
            nbgmSuite->AddToCUint();
        }
    }

    CU_basic_set_mode(testRunMode);
    CU_basic_run_tests();
    CU_cleanup_registry();

    PAL_FileClose(configFile);
    DestroyPal(pal);
    return CU_get_error();
}

