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
  @file     Logger.cpp
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

#include "Logger.h"
#include "paldebuglog.h"
#include "palclock.h"
#include "FileOperatingTaskManager.h"
#include "nbmacros.h"
#include "nbcontextprotected.h"
#include "palatomic.h"

using namespace nbnav;

static const int32 LOG_BUFFER_SIZE   = 1024;
static const char* LOG_DEBUG_FORMAT  = "%02d/%02d %02d:%02d:%02d.%d: (%s): ";


Logger::Logger(NB_Context* nbContext, const char* fileName, LoggerLevel level)
    : m_level(level),
      m_valid(CCC_NEW bool(true)),
      m_stream(fileName ? CCC_NEW ofstream(fileName, ios_base::trunc|ios_base::out|ios_base::binary) : NULL)
{
    m_fileOperatingTaskManager = nbcommon::FileOperatingTaskManager::GetInstance(nbContext);

    if (fileName && !m_stream->is_open())
    {
        delete m_stream;
        m_stream = NULL;
    }
}

Logger::~Logger()
{
    delete m_stream;
}

Logger* Logger::GetInstance(NB_Context* nbContext, const char* workPath, LoggerLevel level)
{
#ifndef _DEBUG
    return NULL;
#else
    static Logger* instance = NULL;
    if (!instance)
    {
        Logger* newInstance = CCC_NEW Logger(nbContext, workPath, level);
        PAL_AtomicCompareExchangePointer((void* volatile *)&instance, newInstance, NULL);
    }
    return instance;
#endif
}

void Logger::WriteLog(LoggerLevel level, const char* componentName, const char *format, ...)
{
#ifndef _DEBUG
        return;
#else
    if (level >= m_level)
    {
        char formatedString[LOG_BUFFER_SIZE] = {'\0'};
        PAL_ClockDateTime dateTime = {0};
        PAL_ClockGetDateTime(&dateTime);

        (void)nsl_snprintf(formatedString, LOG_BUFFER_SIZE, LOG_DEBUG_FORMAT,
                          dateTime.day, dateTime.month, dateTime.hour,
                          dateTime.minute, dateTime.second, dateTime.milliseconds,
                          componentName);

        va_list argumentList;
        va_start(argumentList, format);
        int stringLength = nsl_vsnprintf(formatedString+strlen(formatedString),
                                         LOG_BUFFER_SIZE, format, argumentList);
        va_end(argumentList);

        // Add to FileOperating Thread to output log.
        if (m_stream && m_stream->is_open())
        {
            m_fileOperatingTaskManager->AddCustomizedTask(
                CCC_NEW LoggerTask(this, formatedString));
        }
        else
        {
            fprintf(stderr, "%s", formatedString);
        }
    }
#endif
}


/* See description in header file. */
void Logger::SetLogLevel(LoggerLevel level)
{
    PAL_AtomicCompareExchange((int32 volatile*) &m_level, (int)m_level, (int)level);
}

/* See description in header file. */
void Logger::AppendContentToStream(const string& content)
{
    if (m_stream->is_open())
    {
        *m_stream << content << endl;
    }

    // always output to stderr if non-silent.
    if (m_level != LL_Silent)
    {
        fprintf(stderr, "%s\n", content.c_str());
    }
}

Logger::LoggerTask::LoggerTask(Logger* logger, const char* content)
    : m_logger(logger)
{
    if (content)
    {
        m_content.assign(content);
    }

    if (logger && logger->m_valid)
    {
        m_valid = logger->m_valid;
    }
}

void Logger::LoggerTask::Execute()
{
    if (m_logger && m_valid && *m_valid && !m_content.empty())
    {
        m_logger->AppendContentToStream(m_content);
    }
    delete this;
}
