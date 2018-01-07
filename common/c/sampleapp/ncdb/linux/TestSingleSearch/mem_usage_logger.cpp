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
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

#if __linux__
#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include "mem_usage_logger.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#define _PAGE_SIZE_32_BIT_MC 1024.0
#define _PAGE_SIZE_64_BIT_MC 2048.0

bool
filePresent(const char *filename)
{
    struct stat statBuf;
    return stat(filename, &statBuf) == 0;
}

/**
 * Attempt to read the system-dependent data for a process' vmm and
 * rss using the /proc/self/stat file, and return the results in KB.
 * On failure, returns 0.0, 0.0
 *
 * @param vm_usage The returned value for the current virtual
 *                 memory usage for process.
 * @param resident_set The returned value for the current resident
 *                     set size for the process.
 */
void
MemUsageLogger::process_mem_usage(double& vm_usage, double& resident_set)
{
    using std::ios_base;
    using std::ifstream;
    using std::string;

    vm_usage     = 0.0;
    resident_set = 0.0;

    ifstream stat_stream("/proc/self/stat",ios_base::in);

    // dummy vars for leading entries in stat that we don't care about
    string pid, comm, state, ppid, pgrp, session, tty_nr;
    string tpgid, flags, minflt, cminflt, majflt, cmajflt;
    string utime, stime, cutime, cstime, priority, nice;
    string O, itrealvalue, starttime;

    // the two fields we want
    unsigned long vsize;
    long rss;

    stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                >> utime >> stime >> cutime >> cstime >> priority >> nice
                >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

    long page_size_kb = sysconf(_SC_PAGE_SIZE) / (long)_PAGE_SIZE_32_BIT_MC;
    vm_usage     = vsize / _PAGE_SIZE_32_BIT_MC;
    resident_set = rss * page_size_kb;
}


MemUsageLogger::MemUsageLogger(void)
    : _vmm(0.0), _rss(0.0) , dumpFile(NULL)
{
    _initialized = true;
// #ifdef ENABLE_MEM_LOG
    Init();
// #endif
}

void
MemUsageLogger::Init(void)
{
    //if (filePresent("doMemDump"))
    //{
        dumpFile = fopen("MemLogFile", "w+");
        process_mem_usage (this->_vmm, this->_rss);
        fprintf (dumpFile, "Initialising MemLogger........\n");
        _logged_time = time(0);
        fprintf (dumpFile, "\n%s", ctime(&_logged_time));
        fprintf (dumpFile, "Init Memory Usage: vmm: %0.2fK rss: %0.2fK\n", _vmm, _rss);
    //}
}

void
MemUsageLogger::DumpMemUsage (const char* msg, const char* filename, const char* func, int line)
{
    assert (isInitialized ());
    double curVmm, curRss;
    //if (dumpFile)
    //{
        process_mem_usage (curVmm, curRss);
//        fprintf (dumpFile,
//                 "\n\n<%s: %s(): %d>[%s]\nCurrent Memory Usage: vmm: %0.2fK rss: %0.2fK | ::difference:: vmm: %0.2fK rss: %0.2fK",
//                 filename, func, line, msg, curVmm, curRss, curVmm - _vmm, curRss - _rss);
/*        if ( line > 0 && func != 0 && filename != 0 )
        {
            fprintf (dumpFile, "\n[ %s ] : %d : %s()", filename, line, func);
        }*/
        time_t curr_time = time(0);
        double seconds = difftime(curr_time, _logged_time);
        fprintf (dumpFile, "\n%s", ctime(&_logged_time));
        fprintf (dumpFile, "Time Difference is %f seconds", seconds);
        fprintf (dumpFile, "\n%s : vmm: %0.2fK rss: %0.2fK | ::difference:: vmm: %0.2fK rss: %0.2fK\n",
            msg, curVmm, curRss, curVmm - _vmm, curRss - _rss);
        _vmm = curVmm;
        _rss = curRss;
        fflush(dumpFile);
        _logged_time = curr_time;
    //}
}

#endif
