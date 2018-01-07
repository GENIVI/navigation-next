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

/**
 * @file mem_logger.h
 *
 * ? 2004-2009, TeleCommunication Systems, Inc.
 *
 * This  material is protected by U.S. and international copyright laws and
 * may  not  be  reproduced,  modified, distributed, publicly displayed, or
 * used  to  create derivative works without the express written consent of
 * TeleCommunication Systems, Inc. This copyright notice may not be altered
 * or removed.
 */

#ifndef MEM_USAGE_LOGGER_H_
#define MEM_USAGE_LOGGER_H_
#if __linux__

class MemUsageLogger
{
public:
    /**
     * Constructor for MemUsageLogger.
     */
    MemUsageLogger(void);

    /**
     * Analyse and dump the Memory Usage status.
     * @param msg Description about the dump
     * @param filename The filename from where memory logger is called.
     * @param func The function name from where memory logger is called.
     * @param line The line number from where memory logger is called.
     */
    void DumpMemUsage (const char* msg, const char* filename=0, const char* func=0, const int line=0);

private:
    /**
     * Initialise the Memory Usage Context here.
     */
    void Init(void);

    /**
     * Attempt to read the system-dependent data for a process' vmm and
     * rss using the /proc/self/stat file, and return the results in KB.
     * On failure, returns 0.0, 0.0
     *
     * @param vm_usage The returned value for the current virtual
     *                 memory usage for process.
     * @param resident_set The returned value for the current resident
     *                 set size for the process.
     */
    void process_mem_usage(double& vm_usage, double& resident_set);

    /**
     * Function to check  the initialization status for MemUsageLogger.
     */
    bool isInitialized(void) const { return _initialized; }

    /// Variable indicating the initialization status for MemUsageLogger.
    bool _initialized;

    /// Variable to store latest Virtual Memory usage in KB.
    double _vmm;

    /// Variable to store latest Resident Set usage in KB.
    double _rss;

    /// File pointer to dump the Mem Log File
    FILE *dumpFile;

    /// Logged Time;
    time_t _logged_time;
};

//A Global variable for access to memory profile.
extern MemUsageLogger* memUsageLogger;

#endif
#endif /*MEM_USAGE_LOGGER_H_*/
