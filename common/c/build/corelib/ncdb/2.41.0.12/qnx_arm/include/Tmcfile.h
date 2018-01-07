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
#pragma once
#pragma warning(disable:4251)
#ifndef _TMC_FILE_H_
#define _TMC_FILE_H_
#include "NcdbTypes.h"
#include <string>
#include <map>
#include <set>
using namespace std;

#define MTS_PER_SEC_INTO_MILES_PER_HOUR (3600/1609.366)

namespace Ncdb 
{
    class NCDB_API TmcFile
    {
    public:
        TmcFile();
        int reset_traffic_data_reader();
        int get_curr_speed_by_tmc(const char* provider,const char* tmc_code, int tmc_code_len, double* pspeed);
        int get_speed_limit_by_tmc(const char* provider,const char* tmc_code, int tmc_code_len, double* pspeed);
        int get_speedratio_by_tmc(const char* provider,const char* tmc_code, int tmc_code_len, double* ratio);
        int get_incident_status_by_tmc(const char* provider,const char* tmc_code, int tmc_code_len);
        int get_closure_status_by_tmc(const char* provider,const char* tmc_code, int tmc_code_len);
        void set_curr_speed_by_tmc(const char* provider, const char* tmc_code, double speed);
        void set_speed_limit_by_tmc(const char* provider, const char* tmc_code, double speed);
        void set_closure_status_by_tmc(const char* provider, const char* tmc_code, bool status);
        void set_incident_status_by_tmc(const char* provider, const char* tmc_code, bool status);
        static TmcFile* GetTmcFile();
#ifdef WIN32
        bool AccessModifiedFileGroup(const char* provider);
#endif



    private:
        const char* pTrafficSpeedFile;
        const char* pTrafficIncidentsFile;
        const char* pTrafficClosuresFile;
        map<string, double> m_tmcCurrentSpeed;
        map<string, double> m_tmcLimitSpeed;
        set<string> m_tmcIncident;
        set<string> m_tmcClosure;
        void CreateMapTmcSpeed( const char* provider );
        void InitMap(const char* provider );
        static bool hasLoadIntoMemory;
        static TmcFile m_tmcFile;

#ifdef WIN32
        FILETIME trafficSpeedFileTime;
        FILETIME trafficClosuresFileTime;
        FILETIME trafficIncidentsFileTime;

        bool AccessModifiedFile(const char* pFileName, FILETIME& fileTime, bool access);

#endif
    };



};


#endif