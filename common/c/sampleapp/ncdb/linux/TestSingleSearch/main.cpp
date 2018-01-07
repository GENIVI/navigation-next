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

#include <iostream>
#include <getopt.h>
#include <fstream>
#include <cstring>
#include "main.h"
//#include "metrics.h"
#include "test_single_search.h"

using namespace std;

bool g_bSuggestList = false;
bool g_bVerbose = false;
bool g_bDumpFile = false;
string g_sLangCode("XXX");
unsigned int g_LiteMode = LITEMODE_OFF;
bool g_bFreeform = false;
bool g_bAirportComp = false;
Ncdb::GeoReqType g_eGeoReqType = Ncdb::GeoReqComponent;
string g_sCountry("USA");
unsigned int g_WantFlags = WANT_SINGLELINE_ADDRESS;
MemUsageLogger* memUsageLogger = 0;

ofstream outfile;

struct LiteModeMapping
{
    const char      *name;
    unsigned int    value;
};

LiteModeMapping liteMappings[] = {
    {"off", LITEMODE_OFF},
    {"medium", LITEMODE_MEDIUM},
    {"extreme", LITEMODE_EXTREME},
    {"nav", LITEMODE_NAV}
};

unsigned int GetLiteModeFromString(char* str)
{
    int entries = sizeof(liteMappings) / sizeof(LiteModeMapping);
    unsigned int litemode = LITEMODE_OFF;

    char* ptr;

    do
    {
        ptr = strstr(str, "/");

        if(ptr != NULL)
        {
            *ptr = 0;
            ++ptr;
        }

        // Now find the entry
        int idx = -1;
        for(int i = 0; i < entries; i++)
        {
            if(strcasecmp(liteMappings[i].name, str)==0)
            {
                idx = i;
                break;
            }
        }

        if(idx < 0)
        {
            cout << "Invalid Lite Mode(" << str << ") Entered. Defaulting to Off for this option" << endl;
            idx = 0;
        }

        litemode |= liteMappings[idx].value;

        str = ptr;
    }while(str);

    return litemode;
}

void printUsage(const char* program_name)
{
    cout << "Usage: "<< program_name << "-i <map ini> -f <input test cases fname>" << endl << endl;
    cout << "Required Flags" << endl;
    cout << "\t-i <map ini>: Full path to the ini file of the map data" << endl;
    cout << "\t-f <input case fname>: Full path to the file containing the input test cases" << endl;
    cout << "\t-W want flags: Geocoding WANT Flags. Controls Formatted Outputs" <<endl;
    cout << "\t-S Print only suggest List" <<endl;
    cout << endl << "Optional Flags" << endl;
	/*
    cout << "\t-n numresults: Max number of results" <<endl;
    cout << "\t-F flags: Geocoding Flags" <<endl;
    cout << "\t-V: Verbose Output" <<endl;
    cout << "\t-D: Dump results in results.out file" <<endl;
    cout << "\t-L Lang Code: The language Code" <<endl;
    cout << "\t-l litemode value: LiteMode value. 'off', 'medium' or 'extreme'" <<endl;
    cout << "\t-C Country name or ISO code. Ex. \"United States\" or USA " <<endl;
    cout << "\t-g GeoReqType like GeoReqComponent = 0, GeoReqLine1_2 = 1, GeoReqFreeform = 2, GeoReqAirport = 4 and options 2 and 4 ORed" <<endl;
    cout << "\t-H: Print this help and exit" <<endl;
	*/
    
    cout << endl;
}

int main(int argc, char * argv[])
{
    int c;
    extern char* optarg;
    extern int   optopt;

    const char* inipath     = NULL;
    char*       filename    = NULL;
    float       radius     = 10.0;
    int         numresults  = GEOCODE_MAX_RESULT_COUNT;
    int         flags       = GEOCODE_DEFAULT;
    int         help = 0;

    // Create object for MemUsageLogger();
    memUsageLogger = new MemUsageLogger();

    while((c = getopt (argc, argv, "i:f:r:n:F:W:SVDL:l:hC:g:")) != -1)
    {
        switch(c)
        {
        case 'i':
            inipath = optarg;
            break;
        case 'f':
            filename = optarg;
            break;
        case 'r':
            radius = atof(optarg);
            break;
        case 'n':
            numresults = atoi(optarg);
            break;
        case 'F':
            if (optarg[0] == '0' && (optarg[1] == 'x' || optarg[1] == 'X'))
                flags = strtol(optarg, NULL, 16);
            else
                flags = atoi(optarg);
            break;
        case 'W':
            if (optarg[0] == '0' && (optarg[1] == 'x' || optarg[1] == 'X'))
                g_WantFlags = strtol(optarg, NULL, 16);
            else
                g_WantFlags = atoi(optarg);
            break;
        case 'S':
            g_bSuggestList = true;
            break;
        case 'V':
            g_bVerbose = true;
            break;
        case 'D':
            g_bDumpFile = true;
            break;
        case 'L':
            g_sLangCode.assign(optarg, 3);
            break;
        case 'l':
            g_LiteMode = GetLiteModeFromString(optarg);
            break;
        case 'C':
            if (optarg != 0 && optarg[0] != '\0')
                g_sCountry.assign(optarg);
            break;
        case 'g':
            if (optarg != 0 && optarg[0] != '\0')
                g_eGeoReqType = (Ncdb::GeoReqType)atoi(optarg);
            break;
        case 'h':
            help = 1;
            break;
        case '?':
            if(optopt == 'F' || optopt == 'W' || optopt =='i' || optopt == 't' ||
               optopt == 'n' || optopt == 'f' || optopt == 'L' ||
               optopt == 'l' || optopt == 'C' || optopt == 'g')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if(isprint(optopt))
            {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            }
            return 1;
        default:
            return 1;
        }
    }

    if(help)
    {
        printUsage(argv[0]);
        return 0;
    }

    if(!inipath || !filename)
    {
        cout << "Need the Map data ini path and input filename" << endl;
        printUsage(argv[0]);
        return 1;
    }

    cout << "Loading: " << filename << endl << endl;

    if(g_bDumpFile)
    {
        // open the outfile
        outfile.open("results.out");
        if(!outfile.is_open())
        {
            printf("Error opening dumpfile 'results.out'\n");
            g_bDumpFile = false;
        }
    }

    SingleSearchStringsOfFlatfile(inipath, filename);

    if(g_bDumpFile && outfile.is_open())
    {
        outfile.close();
    }

//    cout << GLOBAL_TIMER.numberOfEntries() << " timing entries (ms)" << endl;
//    cout << "  min: " << GLOBAL_TIMER.min_time_ms() << endl;
//    cout << "  avg: " << GLOBAL_TIMER.avg_time_ms() << endl;
//    cout << "  max: " << GLOBAL_TIMER.max_time_ms() << endl;
//    cout << "  total: " << GLOBAL_TIMER.total_time_ms() << endl;
}
