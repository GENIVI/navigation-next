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
  @file     Logger.h
*/
/*
  (C) Copyright 2014 by TeleCommunication Systems, Inc.

  The information contained herein is confidential, proprietary
  to TeleCommunication Systems, Inc., and considered a trade secret as
  defined in section 499C of the penal code of the State of
  California. Use of this information by anyone other than
  authorized employees of TeleCommunication Systems, is granted only
  under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "nbcontext.h"
#include "commontaskqueue.h"
#include "smartpointer.h"
#include <string>
#include <fstream>

namespace nbcommon
{
class FileOperatingTaskManager;
}


namespace nbnav
{

class Logger
{
public:
    /*! The logger level enum. */
    typedef enum
    {
        LL_Unknown = 0,
        LL_Verbose,
        LL_Debug,
        LL_Info, // default one.
        LL_Warn,
        LL_Error,
        LL_Fatal,
        LL_Silent
    } LoggerLevel;

    /*! Add one text to logger

        @param level logger level, if level is lower then setting, this log will be ignored
        @param componentName the component name of log text
        @param format the format string of log text
        @param ... argument list
        @return none
    */
    void WriteLog(LoggerLevel level, const char* componentName, const char* format, ...);

    /*! Set log level of this logger.

        User of this logger can set this value at runtime.

        @param level -  level to be set.

        @return None
    */
    void SetLogLevel(LoggerLevel level);

    /*! Get the Logger instance

      It is used to get the instance and set initial parameters to logger instance,
      The Level parameter is used to set the level standard, every log content with
      level lower than the standard level will be ignored, please also reference the
      AddLog method.

      @param nbContext NB_Context instance.
      @param filename name of log files.
      @param level LoggerLevel enum value.
    */
    static Logger* GetInstance(NB_Context* nbContext,
                               const char* fileName,
                               LoggerLevel level);

private:
    Logger(NB_Context* nbContext, const char* workPath, LoggerLevel level);
    ~Logger();

    class LoggerTask : public Task
    {
    public:
        LoggerTask(Logger* logger, const char* content);

        virtual ~LoggerTask(){};
        virtual void Execute();

    private:
        Logger* m_logger;
        string  m_content;
        shared_ptr<bool> m_valid;
    };

    void AppendContentToStream(const string& content);

    LoggerLevel      m_level;
    shared_ptr<bool> m_valid;
    ofstream*        m_stream;

    shared_ptr<nbcommon::FileOperatingTaskManager> m_fileOperatingTaskManager;
};
}

#endif

/*! @} */
