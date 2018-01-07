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

@file     ArgTableUtil.c
@date     04/20/09
@defgroup ArgTable-specific Utility Functions

Implementation of platform-specific utilities used for system test.

This file contains the different platform implementation of the platform-
specific utilities needed for system testing such as Argtable.
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

#include "argtableutil.h"
#include "argtable2.h"
#include "main.h"
#include "namebitmaskmap.h"

#define BUFSIZE         19
#define ARGTABLESIZE    20

static void DisplayHelp(void* argtable, const char* progname);
static nb_boolean ValidateInput();
static nb_boolean IsCorrectFileName(const char* file);

nb_boolean ProcessArguments(int argc, char* argv[])
{   
    const char*     progname    = "systemtests";
    nb_boolean      exitcode    = 0;
    int             errorCount  = 0;
    int             i           = 0;
    char*           pSuites     = NULL;
    struct arg_int* networkType = NULL;
    struct arg_int* verbose     = NULL;
    struct arg_int* start       = NULL;
    struct arg_int* step        = NULL;
    struct arg_lit* help        = NULL;
    struct arg_int* coverage    = NULL;
    struct arg_lit* overwrite   = NULL;
    struct arg_str* token       = NULL;
    struct arg_str* tpslib      = NULL;
    struct arg_str* host        = NULL;
    struct arg_str* suite       = NULL;
    struct arg_str* groups      = NULL;
    struct arg_lit* verboseQa   = NULL;
    struct arg_lit* listSuites    = NULL;
    struct arg_lit* listTests   = NULL;
    struct arg_str* domain      = NULL;
    struct arg_str* carrier     = NULL;
    struct arg_end* end         = NULL;
    struct arg_str* logFileName   = NULL;
    struct arg_str* testNameStart = NULL;
    void*           argtable[ARGTABLESIZE];
    Test_Options*   testOptions = Test_OptionsCreate();

    start       = arg_int0("P", "numstart",  "Number of first suite(start point) to run", "-//-");
    step        = arg_int0("S", "step",      "Number of suites per this process", "-//-");
    networkType = arg_int0("n", "network",   "1/2/3", "Set the network type for the TPS");
    verbose     = arg_int0("v", "verbose",   "1/2/3", "Control Output for the logging");
    coverage    = arg_int0("c", "coverage",  "0/1/2", "Coverage of testing");
    overwrite   = arg_lit0("O", "overwrite", "Enable free memory overwrite testing (consumes lots of memory)");
    help        = arg_lit0(NULL, "help",     "display this help and exit");
    token       = arg_str0("t", "token",     "<token>", "server token");
    tpslib      = arg_str0("l", "library",   "<tplfile>", "tpslib file");
    suite       = arg_str0("s", "suite",     "<suites>", "suites to test");
    groups      = arg_str0("g", "group",     "<groups>", "suite groups to test");
    host        = arg_str0("m", "machine",   "<hostname>", "dev server hostname");
    verboseQa   = arg_lit0("Q", "qaverbose", "Enable verbose QA Logging");
    listSuites    = arg_lit0("L", "list suites", "List suites and exit");
    listTests     = arg_lit0("T", "list tests", "Save test list in file and exit");
    domain      = arg_str0("d", "domain",    "<domain id>", "Domain name id");
    carrier     = arg_str0("C", "carrier",   "<carrier id>", "Carrier id");
    logFileName   = arg_str0("x", "xml log file name", "<file name>", "file name, xml  CUnit output");
    testNameStart = arg_str0("N", "test name start", "<test start>", "start test name for run");
    end         = arg_end(BUFSIZE);

    argtable[0] = networkType;
    argtable[1] = verbose;
    argtable[2] = help;
    argtable[3] = coverage;
    argtable[4] = overwrite;
    argtable[5] = token;
    argtable[6] = tpslib;
    argtable[7] = suite;
    argtable[8] = groups;
    argtable[9] = host;
    argtable[10] = verboseQa;
    argtable[11] = listSuites;
    argtable[12] = listTests;
    argtable[13] = domain;
    argtable[14] = carrier;
    argtable[15] = start;
    argtable[16] = step;
    argtable[17] = logFileName;
    argtable[18] = testNameStart;
    argtable[19] = end;

    if (arg_nullcheck(argtable) != 0)
    {
        PRINTF("%s: insufficient memory\n", progname);
        exitcode = FALSE;
        goto exit;
    }

    errorCount = arg_parse(argc, argv, argtable);

    if (help->count > 0)
    {
        DisplayHelp(argtable, progname);
        exitcode = FALSE;
        goto exit;
    }

    if (errorCount > 0)
    {
        arg_print_errors(stdout, end, progname);
        PRINTF("Try '%s --help' for more information.\n", progname);
        exitcode = FALSE;
        goto exit;
    }

    if (verbose->count > 0)
    {
        testOptions->logLevel = verbose->ival[0];
    }

    if (start->count > 0)
    {
        testOptions->numStart = start->ival[0];
    }

    if (step->count > 0)
    {
        testOptions->numStep = step->ival[0];
    }

    if (coverage->count > 0)
    {
        testOptions->testLevel = coverage->ival[0];
    }

    if (networkType->count > 0)
    {
        testOptions->networkType = networkType->ival[0];
    }

    if (overwrite->count > 0)
    {
        testOptions->overwriteFlag = TestOverwriteFlagTrue;
    }

    if (token->count > 0)
    {
        nsl_strncpy(testOptions->token, *token->sval, MAX_TOKEN_LENGTH);
    }

    if (tpslib->count > 0)
    {
        nsl_strncpy(testOptions->tpslib, *tpslib->sval, MAX_TPSLIB_NAME_LENGTH);
    }

    if (testNameStart->count > 0)
    {
        char* test = (char*)*testNameStart->sval;

        if (test)
        {
            nsl_strncpy(testOptions->testName, test, sizeof(testOptions->testName)/sizeof(char));
            testOptions->startFromParticularTest = TRUE;
        }
    }

    if (suite->count > 0)
    {
        pSuites = (char *)*suite->sval;
        while(pSuites)
        {
            nb_boolean found = FALSE;

            for(i = 0; i < suiteCount; i++)
            {
                int len = 0;

                if (nsl_strlen(suiteMap[i].name) >= nsl_strlen(pSuites))
                {
                    len = nsl_strlen(suiteMap[i].name);
                }
                else
                {
                    len = nsl_strlen(pSuites);
                }

                if (nsl_strnicmp(pSuites, suiteMap[i].name, len) == 0)
                {
                    testOptions->suites |= suiteMap[i].bitmask;
                    found = TRUE;
                    break;
                }
            }

            if (!found)
            {
                arg_print_errors(stdout, end, progname);
                PRINTF("Invalid suite name found at: %s\n", pSuites);
                exitcode = FALSE;
                goto exit;
            }

            pSuites = nsl_strchr(pSuites, ',');
            if(pSuites)
            {
                pSuites++;
            }
        }
    }
    else if (groups->count == 0)
    {
        //testOptions->suites = SUITE_ALL;
        testOptions->suites =
        0
//        | SUITE_NETWORK           // current OK
//        | SUITE_RASTERMAP         // current OK
//        | SUITE_RASTERTILE        // current OK
//        | SUITE_VECTORMAP         // current OK
//        | SUITE_GEOCODE           // current OK
//        | SUITE_SEARCHBUILDER     // current error // TestSearchBuilderSearchSpeedCameras
//        | SUITE_POI               // current OK
//        | SUITE_NAVIGATION        // current error // SwedenFerries CarAustria CarBrussels
//        | SUITE_DIRECTIONS        // current OK
//        | SUITE_SPATIAL           // current OK
//        | SUITE_SPEECH            // current OK
//        | SUITE_FILESET           // current error  // NESERVERX_UNSUPPORTED_FILESET, server report: TPSUnsupportedFileset: no package reference for hash, '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
//        | SUITE_MOTD              // current OK
//        | SUITE_PLACEMESSAGE      // current OK
//        | SUITE_SYNC              // current OK
//        | SUITE_PROFILE           // current OK
//        | SUITE_DATASTORE         // current OK
//        | SUITE_SMS               // current OK
//        | SUITE_REVERSEGEOCODE    // current OK
//        | SUITE_ROUTE             // current OK
//        | SUITE_QALOG             // current OK
//        | SUITE_SUBSCRIPTION      // current OK
//        | SUITE_GPSTYPES          // current OK
//        | SUITE_TRISTRIP          // current OK
//        | SUITE_ANALYTICS         // current OK
//        | SUITE_ERS               // current OK
//        | SUITE_LICENSE           // current OK
//        | SUITE_LOCATION          // current OK
//        | SUITE_ENHANCEDCONTENT   // current OK
//        | SUITE_LOCATIONSERVICES  // current OK
//        | SUITE_SINGLESEARCH      // current OK
//        | SUITE_TILESERVICE       // current error // TestTileServiceReceiveSatelliteMultiple TestPOITiles time out
//        | SUITE_SPEEDCAMERAS      // current error // TestSpeedCamerasGetNextCamera
//        | SUITE_PUBLICTRANSIT     // current OK
        | SUITE_ALL
        ;
    }

    if (groups->count > 0)
    {
        pSuites = (char *)*groups->sval;
        while(pSuites)
        {
            nb_boolean found = FALSE;

            for(i = 0; i < groupCount; i++)
            {
                if(nsl_strnicmp(pSuites, groupMap[i].name, nsl_strlen(groupMap[i].name)) == 0)
                {
                    testOptions->suites |= groupMap[i].bitmask;
                    found = TRUE;
                    break;
                }
            }

            if (!found)
            {
                arg_print_errors(stdout, end, progname);
                PRINTF("Invalid group name found at: %s\n", pSuites);
                exitcode = FALSE;
                goto exit;
            }

            pSuites = nsl_strchr(pSuites, ',');
            if(pSuites)
            {
                pSuites++;
            }
        }
    }

    if (host->count > 0)
    {
        nsl_strncpy(testOptions->hostname, *host->sval, MAX_HOSTNAME_LENGTH);
    }

    testOptions->verboseQaLog = (verboseQa->count > 0) ? TRUE : FALSE;

    testOptions->listSuitesAndExit = (listSuites->count > 0) ? TRUE : FALSE;
    testOptions->listTestsAndExit = (listTests->count > 0) ? TRUE : FALSE;

    if (domain->count > 0)
    {
        nb_boolean valid = FALSE;

        if (nsl_strlen(domain->sval[0]) == 1)
        {
            switch (*(domain->sval[0]))
            {
            case 'd':
                testOptions->domain = TestNetworkDomainDevLocal;
                valid = TRUE;
                break;
            case 'n':
                testOptions->domain = TestNetworkDomainNavBuilder;
                valid = TRUE;
                break;
            case 'v':
                testOptions->domain = TestNetworkDomainVerizon;
                valid = TRUE;
                break;
            }
        }

        if (!valid)
        {
            PRINTF("Invalid domain specifier: %s", domain->sval[0]);
            exitcode = FALSE;
            goto exit;
        }
    }

    if (carrier->count > 0)
    {
        nb_boolean valid = FALSE;

        if (nsl_strlen(carrier->sval[0]) == 1)
        {
            switch (*(carrier->sval[0]))
            {
            case 'v':
                testOptions->carrier |= TestCarrierVerizon;
                valid = TRUE;
                break;
            case 'a':
                testOptions->carrier |= TestCarrierAtlasbook;
                valid = TRUE;
                break;
            }
        }

        if (!valid)
        {
            PRINTF("Invalid carrier specifier: %s", carrier->sval[0]);
            exitcode = FALSE;
            goto exit;
        }
    }

    if (logFileName->count > 0)
    {
        if (IsCorrectFileName(*(logFileName->sval)))
        {
            nsl_strncpy(testOptions->logfilename, *(logFileName->sval), nsl_strlen(*(logFileName->sval)));
        } else {
            PRINTF("\nInvalid name of file!");
            exitcode = FALSE;
            goto exit;
        }
    }

    exitcode = ValidateInput();
    if(exitcode == FALSE)
    {
        PRINTF("\nTo view Help use --help\n");
    }

exit:

    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));

    return exitcode;
}

nb_boolean ValidateInput()
{
    Test_Options* testOptions = Test_OptionsGet();

    if (testOptions->logLevel < LogLevelNone || testOptions->logLevel > LogLevelHigh) 
    {
        PRINTF("\nPlease use 1 for Low, 2 for Medium and 3 for High level Logging\n\n");
        return FALSE;
    }

    if (testOptions->testLevel < TestLevelSmoke || testOptions->testLevel > TestLevelFull)
    {
        PRINTF("\nPlease use 0 for Smoke, 1 for Intermediate and 2 for Full testing\n\n");
        return FALSE;
    }        

    return TRUE;
}

void DisplayHelp(void* argtable, const char* progname)
{
    PRINTF("\nUsage: %s", progname);
    arg_print_syntax(stdout,argtable,"\n");
    arg_print_glossary(stdout,argtable,"  %-25s %s\n");
    PRINTF("\n");
	PRINTF("Use -n or --network to specify a network type: 0 = TCP, 1 = TLS, 2 = HTTP, 3 = HTTPS (the default is TCP)\n\n");
    PRINTF("Use -v or --verbose to specify a verbosity level: 0 = None, 1 = Low, 2 = Medium, 3 = High\n\n");
    PRINTF("Use -c or --coverage to specify a test coverage level: 0 = Smoke, 1 = Intermediate (default), and 2 = Full\n");
    PRINTF("\nFor Win32 debug builds, the -O or --overwrite flag sets the debug heap to reserve freed memory blocks; this will find any stale pointer writes\n\n");
    PRINTF("Use -s or --suite to specify one or more comma seperated test suites (-L or --list lists all available test suites)\n");
    PRINTF("Use -g or --group to specify groups of tests to run\n\n");
    PRINTF("Use -m or --machine to specify a server that is not derived from the token\n");
    PRINTF("Use -d or --domain to specify a network domain: d = dev.local, n = navbuilder.nimlbs.net (default), and v = mobile.vznavigator.net\n\n");
    PRINTF("Use -C or --carrier to specify tests for a specific carrier: v = VZW\n");
    PRINTF("Use -P or --numstart with -S or --step to set first suite to run and number of suites for this process\n");
    PRINTF("Use -x  to specified xml log file name\n");
}

nb_boolean IsCorrectFileName(const char* file)
{
    if (file == NULL)
    {
        return FALSE;
    }

    if (nsl_strlen(file) > MAX_LOGFILENAME_LENGTH)
    {
        return FALSE;
    }

    if ((nsl_strstr(file, "\\"))||(nsl_strstr(file, "/"))||(nsl_strstr(file, "|")))
    {
        return FALSE;
    }

    if ((nsl_strstr(file, ":"))||(nsl_strstr(file, "*"))||(nsl_strstr(file, "\"")))
    {
        return FALSE;
    }

    if ((nsl_strstr(file, ">"))||(nsl_strstr(file, "<"))||(nsl_strstr(file, "?")))
    {
        return FALSE;
    }
    return TRUE;
}

/*! @} */
