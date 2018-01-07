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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "nbrelog.h"
#include "paldebuglog.h"
#include "palclock.h"
#include "nbreiostream.h"
#include "paltestlog.h"

#define REMOVE_SOURCE_PATH 1
#define DEBUG_LOG_BUFFER_SIZE 1024

NBRE_Log::NBRE_Log(NBRE_IOStream* istream, const char* logname, NBRE_LogSubSystem ss):
    pFile(NULL),
    power(0),
    options(0),
    indent(0),
    subSystem(ss)
{
    nsl_memset(pComponentName, 0, sizeof(pComponentName));
    nsl_memset(componentPower, 0, sizeof(componentPower));
    nsl_memset(componentSeverity, 0, sizeof(componentSeverity));
    uint32 written = 0;
    const char *pHeader = 
        "/---------------------------------------------------\\\r\n" \
        "|  Networks in Motion Diagnostic Log File V. 1.00   |\r\n" \
        "|---------------------------------------------------|\r\n" \
        "| Key: [B]=Begin block    [E]= End block            |\r\n" \
        "|      [#]=Critical       [X]=Error                 |\r\n" \
        "|      [!]=Warning        [i]=Info                  |\r\n" \
        "|      [d]=Debug                                    |\r\n" \
        "\\---------------------------------------------------/\r\n\n";
    // Set the filename for the log
    logName = nsl_strdup(logname);
    pFile = istream;

    // Write the log header information block
    if(pFile)
    {
        pFile->Write((uint8 *)pHeader, nsl_strlen(pHeader), &written);
    }
}

NBRE_Log::~NBRE_Log()
{
    int i;

    // Free all of the component names
    for(i = 0; i < NBRE_LOG_MAX_IDS; i++)
    {
        if(pComponentName[i])
        {
            nsl_free(pComponentName[i]);
            pComponentName[i] = NULL;
        }
    }

    if (logName != NULL)
    {
        nsl_free(logName);
    }
}

PAL_Error 
NBRE_Log::RegisterComponent(const char* componentName, NBRE_LogComponentID* componentId)
{
    PAL_Error err = PAL_Failed;
    int i;

    if(!componentName || !componentId)
    {
        return PAL_ErrBadParam;
    }

    for(i = 0; i < NBRE_LOG_MAX_IDS; i++)
    {
        if(!pComponentName[i])
        {
            pComponentName[i] = (char *)nsl_malloc(nsl_strlen(componentName)+1);
            nsl_strcpy(pComponentName[i], componentName);
            *componentId = i + 1;
            return PAL_Ok;
        }
    }

    return err;
}

PAL_Error 
NBRE_Log::On(uint16 optionMask)
{
    power      = TRUE;
    options    = optionMask;

    return PAL_Ok;
}

PAL_Error 
NBRE_Log::Off()
{
    power = FALSE;
    return PAL_Ok;
}

PAL_Error 
NBRE_Log::ComponentOn(NBRE_LogComponentID componentId, 
                   PAL_LogSeverity  severity)
{
    PAL_Error err = PAL_Ok;

    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }

    // Set the severity level for this component
    componentSeverity[componentId-1] = severity;

    // Turn this component's logging on
    componentPower[componentId-1] = TRUE;

    return err;
}

PAL_Error 
NBRE_Log::ComponentOff( NBRE_LogComponentID componentId)
{
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }

    // Turn this component's logging off
    componentPower[componentId-1] = FALSE;

    componentSeverity[componentId-1] = PAL_LogSeverityNone;

    return PAL_Ok;
}

PAL_Error 
NBRE_Log::ComponentGetStatus(NBRE_LogComponentID componentId, 
                   PAL_LogSeverity  *severity)
{
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }

    // Return the severity level for this component
    *severity = componentSeverity[componentId-1];

    return PAL_Ok;
}

PAL_Error
NBRE_Log::WriteF(
                 NBRE_LogComponentID   componentId,    /*!< The component for this event */
                 PAL_LogSeverity  severity,           /*!< The severity of this event */
                 const char *fmt, ...                 /*!< The text to place in the log */
                 )
{
    PAL_Error err = PAL_Ok;
    va_list ap;
    char *pBuf = NULL;
    uint16 len = 2048;
    
    if(!fmt)
    {
        return PAL_ErrBadParam;
    }
    
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }
    
    // Ensure that the master log switch is on
    if(power == FALSE)
    {
        return PAL_ErrLogSeverity;
    }
    
    // Check if the severity level is high enough to log this event
    if(severity > componentSeverity[componentId-1])
    {
        return PAL_ErrLogSeverity;
    }
    
    if(err == PAL_Ok)
    {
        // Allocate memory for our variable argument manipulation
        pBuf = (char *)nsl_malloc(len);
        if(pBuf == NULL)
        {
            return PAL_ErrNoMem;
        }
        
        // Clear the buffer
        nsl_memset(pBuf, 0, len);
        
        // Convert the variable arguments into a string
        va_start(ap, fmt);
        vsprintf(pBuf, fmt, ap);
        va_end(ap);
        
        err = LogWrite(componentId, NULL, NULL, 0, pBuf, (LogType)severity);
        
        // Free our temporary buffer
        nsl_free(pBuf);
    }
    
    return err;
}

PAL_Error 
NBRE_Log::WriteF(
              NBRE_LogComponentID   componentId,    /*!< The component for this event */
              PAL_LogSeverity  severity,           /*!< The severity of this event */
              const char *pSource,                 /*!< The name of the source module */
              const char *pFunc,                   /*!< The name of the function where the event occured */
              int line,                            /*!< The line number of the source where the event occured */
              const char *fmt, ...                 /*!< The text to place in the log */
              )
{
    PAL_Error err = PAL_Ok;
    va_list ap;
    char *pBuf = NULL;
    uint16 len = 2048;

    if(!fmt)
    {
        return PAL_ErrBadParam;
    }

    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }
    
    // Ensure that the master log switch is on
    if(power == FALSE)
    {
        return PAL_ErrLogSeverity;
    }

    // Check if the severity level is high enough to log this event
    if(severity > componentSeverity[componentId-1])
    {
        return PAL_ErrLogSeverity;
    }

    if(err == PAL_Ok)
    {
        // Allocate memory for our variable argument manipulation
        pBuf = (char *)nsl_malloc(len);
        if(pBuf == NULL)
        {
            return PAL_ErrNoMem;
        }

        // Clear the buffer
        nsl_memset(pBuf, 0, len);

        // Convert the variable arguments into a string
        va_start(ap, fmt);
        vsprintf(pBuf, fmt, ap);
        va_end(ap);

        err = LogWrite(componentId, pSource, pFunc, line, pBuf, (LogType)severity); 

        // Free our temporary buffer
        nsl_free(pBuf);
    }

    return err;
}

PAL_Error
NBRE_Log::Write(
                NBRE_LogComponentID   componentId,    /*!< The component for this event */
                PAL_LogSeverity  severity,           /*!< The severity of this event */
                const char *pText                    /*!< The text to place in the log */
                )
{
    return Write(componentId, severity, NULL, NULL, 0, pText);
}

PAL_Error 
NBRE_Log::Write(
               NBRE_LogComponentID   componentId,    /*!< The component for this event */
               PAL_LogSeverity  severity,           /*!< The severity of this event */
               const char *pSource,                       /*!< The name of the source module */
               const char *pFunc,                         /*!< The name of the function where the event occured */
               int line,                            /*!< The line number of the source where the event occured */
               const char *pText                    /*!< The text to place in the log */
               )
{
    PAL_Error err = PAL_Ok;

    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }

    // Ensure that the master log switch is on
    if(power == FALSE)
    {
        return PAL_ErrLogSeverity;
    }

    // Check if the severity level is high enough to log this event
    if(severity > componentSeverity[componentId-1] &&
        (int)severity != (int)LogTypeBlockBegin && (int)severity != (int)LogTypeBlockEnd)
    {
        return PAL_ErrLogSeverity;
    }

    if(err == PAL_Ok)
    {
        err = LogWrite(componentId, (char *)pSource, (char *)pFunc, line, (char *)pText, 
            (LogType)severity);
    }

    return err;
}

PAL_Error
NBRE_Log::WriteBlockBegin(
                          NBRE_LogComponentID   componentId,    /*!< The component for this event */
                          const char *pText                    /*!< The text to place in the log */
                          )
{
    return WriteBlockBegin(componentId, NULL, NULL, 0, pText);
}

PAL_Error 
NBRE_Log::WriteBlockBegin(
                       NBRE_LogComponentID   componentId,    /*!< The component for this event */
                       const char *pSource,                 /*!< The name of the source module */
                       const char *pFunc,                   /*!< The name of the function where the event occured */
                       int line,                            /*!< The line number of the source where the event occured */
                       const char *pText                    /*!< The text to place in the log */
                       )
{
    PAL_Error err = PAL_Ok;
    
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }

    err = Write(componentId, (PAL_LogSeverity)LogTypeBlockBegin, pSource, pFunc, line, pText);
        
    indent++;

    return err;
}

PAL_Error
NBRE_Log::WriteBlockEnd(
                        NBRE_LogComponentID   componentId,    /*!< The component for this event */
                        const char *pText                    /*!< The text to place in the log */
                        )
{
    return WriteBlockEnd(componentId, NULL, NULL, 0, pText);
}

PAL_Error 
NBRE_Log::WriteBlockEnd(
                     NBRE_LogComponentID   componentId,    /*!< The component for this event */
                     const char *pSource,                 /*!< The name of the source module */
                     const char *pFunc,                   /*!< The name of the function where the event occured */
                     int line,                            /*!< The line number of the source where the event occured */
                     const char *pText                    /*!< The text to place in the log */
                     )
{
    PAL_Error err = PAL_Ok;

    // Check if the componentID is  valid and registered
    if(!IsValidComponent(componentId))
    {
        return PAL_ErrBadParam;
    }

    indent--;

    err = Write(componentId, (PAL_LogSeverity)LogTypeBlockEnd, pSource, pFunc, line, pText);

    return err;
}

nb_boolean
NBRE_Log::IsValidComponent(NBRE_LogComponentID id)
{
    NBRE_LogComponentID lid = id - 1;

    // Check that this component has been registered and is within the range of IDs
    if(lid < NBRE_LOG_MAX_IDS && pComponentName[lid] != NULL)
    {
        return TRUE;
    }

    return FALSE;
}

PAL_Error
NBRE_Log::LogWrite(NBRE_LogComponentID componentId, const char* pSrc, 
                   const char* pFunc,  uint32 line, const char* pText, LogType type)
{
#define BUFFER_EXTRA    256 // Extra space to ensure no overlap of buffer due to date/time/line nums
    PAL_Error err = PAL_Ok;
    char* pBuf = NULL;
    char tmpBuf[128];
    size_t len = 0;
    size_t lenSrc=0, lenText = 0, lenFunc = 0;
    uint32 written = 0;
    PAL_ClockDateTime curTime;

    if(!pFile && subSystem == NBRE_LSS_FILE)
    {
        return PAL_Failed;
    }

    // Allocate enough memory locally to build our entire log line so that we can
    // avoid many writes to disk
    if(pSrc)
    {
        lenSrc = nsl_strlen(pSrc);
    }

    if(pFunc)
    {
        lenFunc = nsl_strlen(pFunc);
    }

    if(pText)
    {
        lenFunc = nsl_strlen(pText);
    }

    len = lenSrc + lenFunc + lenText + BUFFER_EXTRA;
    pBuf = (char *)nsl_malloc(len);
    if(!pBuf)
    {
        return PAL_ErrNoMem;
    }

    // Clear the buffer
    nsl_memset(pBuf, 0, len);

    // Indent according to the number of BlockIndents
    if(indent)
    {
        LogWriteIndent(pBuf);
    }

    // Add the component name to the buffer
    if(options & NBRE_LO_COMPONENT)
    {
        nsl_sprintf(tmpBuf, "[%s]", pComponentName[componentId-1]);
        nsl_strcat(pBuf, tmpBuf);
    }

    // Add the severity to the buffer
    if(options & NBRE_LO_SEVERITY)
    {
        switch(type)
        {
        case LogTypeBlockBegin:
            nsl_strcat(pBuf, "[B]");
            break;

        case LogTypeBlockEnd:
            nsl_strcat(pBuf, "[E]");
            break;

        case LogTypeMajor:
        case LogTypeImportant:
            nsl_strcat(pBuf, "[X]");
            break;

        case LogTypeCritical:
            nsl_strcat(pBuf, "[#]");
            break;

        case LogTypeMinor:
            nsl_strcat(pBuf, "[!]");
            break;

        case LogTypeInfo:
        case LogTypeMinorInfo:
            nsl_strcat(pBuf, "[i]");
            break;

        case LogTypeDebug:
            nsl_strcat(pBuf, "[d]");
            break;

        case LogTypeNone:
        default:
            break;
        }
    }

    // Add the date/time to the buffer
    if(options & NBRE_LO_DATE || options & NBRE_LO_TIME)
    {
        PAL_ClockGetDateTime(&curTime);

        nsl_strcat(pBuf, "[");

        if(options & NBRE_LO_DATE)
        {
            nsl_sprintf(tmpBuf, "%02d/%02d/%02d", curTime.month, curTime.day, curTime.year);
            nsl_strcat(pBuf, tmpBuf);
        }

        if(options & NBRE_LO_TIME)
        {
            if(options & NBRE_LO_DATE)
            {
                nsl_strcat(pBuf, " ");
            }

            nsl_sprintf(tmpBuf, "%02d:%02d:%02d.%03d", curTime.hour, curTime.minute,
                curTime.second, curTime.milliseconds%1000);
            nsl_strcat(pBuf, tmpBuf);
        }

        nsl_strcat(pBuf, "]");
    }

    if((options & NBRE_LO_SOURCE && pSrc) || (options & NBRE_LO_FUNCTION && pFunc))
    {        
        nsl_strcat(pBuf, "[");

        if(options & NBRE_LO_SOURCE && pSrc)
        {
#if REMOVE_SOURCE_PATH
            // See if the path has a standard windows path separator
            const char* pTemp = nsl_strrchr(pSrc, '\\');
            if(pTemp == NULL)
            {
                // Try other path separator type
                pTemp = nsl_strrchr(pSrc, '/');
            }
            if(pTemp != NULL)
            {
                pTemp++;
                nsl_sprintf(tmpBuf, "%s", pTemp);
            }
            else
            {
                nsl_sprintf(tmpBuf, "%s", pSrc);
            }
#else
            nsl_sprintf(tmpBuf, "%s", pSrc);
#endif
            nsl_strcat(pBuf, tmpBuf);
            if(options & NBRE_LO_LINE)
            {
                nsl_sprintf(tmpBuf, ":%d", line);
                nsl_strcat(pBuf, tmpBuf);
            }
        }

        if(options & NBRE_LO_FUNCTION && pFunc)
        {
            if(options & NBRE_LO_SOURCE)
            {
                nsl_strcat(pBuf, "|");
            }
            nsl_sprintf(tmpBuf,  "%s", pFunc);
            nsl_strcat(pBuf, tmpBuf);
        }

        nsl_strcat(pBuf, "]");
    }

    if(pText)
    {
        nsl_strcat(pBuf, " ");
        nsl_strcat(pBuf, pText);
    }

    if(subSystem == NBRE_LSS_FILE)
    {
        // If there is no end of line character, add it
        if(pBuf[nsl_strlen(pBuf)-1] != '\n')
        {
            nsl_strcat(pBuf, "\r\n");
        }
        pFile->Write((uint8 *)pBuf, nsl_strlen(pBuf), &written);
    }
    else
    {
        debugf(pBuf);
    }

    // Free our buffer memory
    nsl_free(pBuf);

    return(err);
}

PAL_Error
NBRE_Log::LogWriteIndent(char *pBuf)
{
    int spaces = 0;
    int i = 0;

    if(!pBuf)
    {
        return PAL_ErrBadParam;
    }

    if(!pFile)
    {
        return PAL_Failed;
    }

    // Ensure that the indent never gets to an invalid value
    if((int16)indent < 0)
    {
        indent = 0;
    }

    // If there is no indent currently in affect, simply return
    if(indent == 0)
    {
        return PAL_Ok;
    }

    spaces = indent*NBRE_LOG_INDENT_SPACES;

    for(i = 0; i < spaces; i++)
    {
        nsl_strcat(pBuf, " ");
    }

    return(PAL_Ok);
}

static PAL_LogSeverity gLogSeverity = PAL_LogSeverityMajor;
void NBRE_DebugLog(PAL_LogSeverity severity, const char* fmt, ...)
{
    if(severity >= gLogSeverity)
    {
        return;
    }

    va_list ap;
    char buf[DEBUG_LOG_BUFFER_SIZE] = {0};
    va_start(ap, fmt);
    nsl_vsnprintf(buf, DEBUG_LOG_BUFFER_SIZE-1, fmt, ap);
    va_end(ap);

    PAL_LogLevel logLevel = PAL_LogLevelVerbose;
    switch(severity)
    {
    case PAL_LogSeverityNone:
        logLevel = PAL_LogLevelNone;
        break;
    case PAL_LogSeverityCritical:
        logLevel = PAL_LogLevelError;
        break;
    case PAL_LogSeverityMajor:
    case PAL_LogSeverityImportant:
        logLevel = PAL_LogLevelWarning;
        break;
    case PAL_LogSeverityMinor:
        logLevel = PAL_LogLevelDebug;
        break;
    case PAL_LogSeverityInfo:
    case PAL_LogSeverityMinorInfo:
        logLevel = PAL_LogLevelInfo;
    case PAL_LogSeverityDebug:
        logLevel = PAL_LogLevelVerbose;
    default:
        break;
    }
    PAL_TestLog(PAL_LogBuffer_2, logLevel, buf);
}

void NBRE_SetDebugLogSeverity(PAL_LogSeverity severity)
{
    gLogSeverity = severity;
}