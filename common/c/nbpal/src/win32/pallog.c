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
@defgroup LogImpl for Core
Debug Logging functions
This file contains logging functions for the Core
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
#include "pallog.h"
#include "paldebuglog.h"
#include "palstdlib.h"
#include "palfile.h"
#include "palclock.h"
// Set this to 1 to remove the source path from any source file string
#define REMOVE_SOURCE_PATH  1
/*! Log type. */
typedef enum {
    PAL_LogTypeNone = 0,                            /*!< No events logged */
    PAL_LogTypeCritical,                            /*!< Critical event */
    PAL_LogTypeMajor,                               /*!< Error event */
    PAL_LogTypeImportant,                           /*!< Error event */
    PAL_LogTypeMinor,                               /*!< Warning event */
    PAL_LogTypeInfo,                                /*!< Informational event */
    PAL_LogTypeMinorInfo,                           /*!< Informational event */
    PAL_LogTypeDebug,                               /*!< all events logged */
    PAL_LogTypeBlockBegin,                          /*!< Block begin event */
    PAL_LogTypeBlockEnd,                            /*!< Block end event */
} PAL_LogType;
struct PAL_Log
{
    PAL_Instance*   pPal;                                   /*!< Pointer to the PAL Instance */
    PAL_File*       pFile;                                  /*!<  Log file */
    nb_boolean      power;                                  /*!< On/Off toggle for global logging */
    uint32          options;                                /*!< The logging options */
    uint16          indent;                                 /*!< The current indent level */
    char*           pComponentName[PALLOG_MAX_IDS];         /*!< Component name registered by client */
    nb_boolean      componentPower[PALLOG_MAX_IDS];         /*!< Component level power on/off toggle */
    PAL_LogSeverity componentSeverity[PALLOG_MAX_IDS];      /*!< The log severity for each component */
    char            pFilename[PAL_FILE_MAX_NAME_LENGTH];    /*!< Log filename */
};
static PAL_Error LogOpen(PAL_Log* log);
static PAL_Error LogClose(PAL_Log* log);
static nb_boolean IsValidComponent(PAL_Log* log, PAL_LogComponentID id);
static PAL_Error LogWrite(PAL_Log* pLog, PAL_LogComponentID componentId, const char* pSrc, 
                          const char* pFunc,  uint32 line, const char* pText, PAL_LogType type);
static PAL_Error LogWriteIndent(PAL_Log* pLog, char *pBuf);
PAL_DEF PAL_Error		
PAL_LogCreate(PAL_Instance* pal, const char *pFilename, PAL_Log** log)
{
    PAL_Error err = PAL_Ok;
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
    PAL_Log* pLog = (PAL_Log *)nsl_malloc(sizeof(PAL_Log));
    if(!pLog)
    {
        return PAL_ErrNoMem;
    }
    // Clear the structure
    nsl_memset(pLog, 0, sizeof(PAL_Log));
    // Save the PAL_Instance pointer
    pLog->pPal = pal;
    // Set the filename for the log
    nsl_strcpy(pLog->pFilename, pFilename);
    // Check if the file exists, if not create it
    err = PAL_FileExists(pal, pFilename);
    if(err != PAL_Ok)
    {
        // Create the file
        err = LogOpen(pLog);
        if(err == PAL_Ok)
        {
            // Write the log header information block
            err = PAL_FileWrite(pLog->pFile, (uint8 *)pHeader, nsl_strlen(pHeader), &written);
            LogClose(pLog);
        }
    }
    *log = pLog;
    return err;
}

PAL_DEF void				
PAL_LogDestroy(PAL_Log* log)
{
    int i;
    if(!log)
    {
        return;
    }
    // Free all of the component names
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        if(log->pComponentName[i])
        {
            nsl_free(log->pComponentName[i]);
            log->pComponentName[i] = NULL;
        }
    }
    nsl_free(log);
}
PAL_DEF PAL_Error 
PAL_LogRegisterComponent(PAL_Log* log, const char* componentName, PAL_LogComponentID* componentId)
{
    PAL_Error err = PAL_Failed;
    int i;
    if(!log || !componentName || !componentId)
    {
        return PAL_ErrBadParam;
    }
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        if(!log->pComponentName[i])
        {
            log->pComponentName[i] = (char *)nsl_malloc(nsl_strlen(componentName)+1);
            nsl_strcpy(log->pComponentName[i], componentName);
            *componentId = i + 1;
            return PAL_Ok;
        }
    }
    return err;
}
PAL_DEF PAL_Error 
PAL_LogOn(PAL_Log* log, uint16 optionMask)
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    log->power      = TRUE;
    log->options    = optionMask;
    return err;
}
PAL_DEF PAL_Error 
PAL_LogOff(PAL_Log* log)
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    log->power = FALSE;
    return err;
}
PAL_DEF PAL_Error 
PAL_LogComponentOn(PAL_Log* log, PAL_LogComponentID componentId, 
                   PAL_LogSeverity  severity)
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    // Set the severity level for this component
    log->componentSeverity[componentId-1] = severity;
    // Turn this component's logging on
    log->componentPower[componentId-1] = TRUE;
    return err;
}
PAL_DEF PAL_Error 
PAL_LogComponentOff(PAL_Log* log,  PAL_LogComponentID componentId)
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    // Turn this component's logging off
    log->componentPower[componentId-1] = FALSE;
    log->componentSeverity[componentId-1] = PAL_LogSeverityNone;
    return err;
}
PAL_DEF PAL_Error 
PAL_LogComponentGetStatus(PAL_Log* log, PAL_LogComponentID componentId, 
                   PAL_LogSeverity  *severity)
{
    if(!log || !severity)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    // Return the severity level for this component
    *severity = log->componentSeverity[componentId-1];
    return PAL_Ok;
}
PAL_DEF PAL_Error 
PAL_LogWriteF(
              PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
              PAL_LogComponentID   componentId,    /*!< The component for this event */
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
    if(!log || !fmt)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    // Ensure that the master log switch is on
    if(log->power == FALSE)
    {
        return PAL_ErrLogSeverity;
    }
    // Check if the severity level is high enough to log this event
    if(severity > log->componentSeverity[componentId-1])
    {
        return PAL_ErrLogSeverity;
    }
    // Open the log file
    err = LogOpen(log);
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
        err = LogWrite(log, componentId, pSource, pFunc, line, pBuf, (PAL_LogType)severity); 
        LogClose(log);
        // Free our temporary buffer
        nsl_free(pBuf);
    }
    return err;
}
PAL_DEF PAL_Error 
PAL_LogWrite(
               PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
               PAL_LogComponentID   componentId,    /*!< The component for this event */
               PAL_LogSeverity  severity,           /*!< The severity of this event */
               const char *pSource,                       /*!< The name of the source module */
               const char *pFunc,                         /*!< The name of the function where the event occured */
               int line,                            /*!< The line number of the source where the event occured */
               const char *pText                    /*!< The text to place in the log */
               )
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    // Ensure that the master log switch is on
    if(log->power == FALSE)
    {
        return PAL_ErrLogSeverity;
    }
    // Check if the severity level is high enough to log this event
    if(severity > log->componentSeverity[componentId-1] &&
        severity != PAL_LogTypeBlockBegin && severity != PAL_LogTypeBlockEnd)
    {
        return PAL_ErrLogSeverity;
    }

    // Open the log file
    err = LogOpen(log);
    if(err == PAL_Ok)
    {
        err = LogWrite(log, componentId, (char *)pSource, (char *)pFunc, line, (char *)pText, 
            (PAL_LogType)severity);
        LogClose(log);
    }
    return err;
}
PAL_DEF PAL_Error 
PAL_LogWriteBlockBegin(
                       PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
                       PAL_LogComponentID   componentId,    /*!< The component for this event */
                       const char *pSource,                 /*!< The name of the source module */
                       const char *pFunc,                   /*!< The name of the function where the event occured */
                       int line,                            /*!< The line number of the source where the event occured */
                       const char *pText                    /*!< The text to place in the log */
                       )
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    err = PAL_LogWrite(log, componentId, (PAL_LogSeverity)PAL_LogTypeBlockBegin, pSource, pFunc, line, pText);
    log->indent++;
    return err;
}
PAL_DEF PAL_Error 
PAL_LogWriteBlockEnd(
                     PAL_Log* log,		                  /*!< Pointer to the logger returned on call to PAL_LogCreate */
                     PAL_LogComponentID   componentId,    /*!< The component for this event */
                     const char *pSource,                 /*!< The name of the source module */
                     const char *pFunc,                   /*!< The name of the function where the event occured */
                     int line,                            /*!< The line number of the source where the event occured */
                     const char *pText                    /*!< The text to place in the log */
                     )
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }
    // Check if the componentID is  valid and registered
    if(!IsValidComponent(log, componentId))
    {
        return PAL_ErrBadParam;
    }
    log->indent--;
    err = PAL_LogWrite(log, componentId, (PAL_LogSeverity)PAL_LogTypeBlockEnd, pSource, pFunc, line, pText);
    return err;
}
static PAL_Error LogOpen(PAL_Log* log)
{
    PAL_Error err = PAL_Ok;
    if(!log || !log->pFilename)
    {
        return PAL_ErrBadParam;
    }
    err = PAL_FileOpen(log->pPal, log->pFilename, PFM_Append, &log->pFile); 
    return err;
}
static PAL_Error LogClose(PAL_Log* log)
{
    PAL_Error err = PAL_Ok;
    if(!log)
    {
        return PAL_ErrBadParam;
    }   
    if(log->pFile)
    {
        err = PAL_FileClose(log->pFile);
        log->pFile = NULL;
    }
    return err;
}
static nb_boolean IsValidComponent(PAL_Log* log, PAL_LogComponentID id)
{
    PAL_LogComponentID lid = id - 1;
    if(!log)
    {
        return FALSE;
    }
    // Check that this component has been registered and is within the range of IDs
    if(lid >= 0 && lid < PALLOG_MAX_IDS && log->pComponentName[lid] != NULL)
    {
        return TRUE;
    }
    return FALSE;
}
PAL_DEF PAL_Error
LogWrite(PAL_Log* pLog, PAL_LogComponentID componentId, const char* pSrc, 
                   const char* pFunc,  uint32 line, const char* pText, PAL_LogType type)
{
#define BUFFER_EXTRA    256 // Extra space to ensure no overlap of buffer due to date/time/line nums
    PAL_Error err = PAL_Ok;
    char* pBuf = NULL;
    char *pTemp = NULL;
    char tmpBuf[128];
    size_t len = 0;
    size_t lenSrc=0, lenText = 0, lenFunc = 0;
    uint32 written = 0;
    PAL_ClockDateTime curTime;
    if(!pLog)
    {
        return PAL_ErrBadParam;
    }
    if(!pLog->pFile)
    {
        return PAL_ErrFileNotOpen;
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
    if(pLog->indent)
    {
        LogWriteIndent(pLog, pBuf);
    }
    // Add the component name to the buffer
    if(pLog->options & PAL_LOG_OPTION_COMPONENT)
    {
        nsl_sprintf(tmpBuf, "[%s]", pLog->pComponentName[componentId-1]);
        nsl_strcat(pBuf, tmpBuf);
    }
    // Add the severity to the buffer
    if(pLog->options & PAL_LOG_OPTION_SEVERITY)
    {
        switch(type)
        {
        case PAL_LogTypeBlockBegin:
            nsl_strcat(pBuf, "[B]");
            break;
        case PAL_LogTypeBlockEnd:
            nsl_strcat(pBuf, "[E]");
            break;
        case PAL_LogTypeMajor:
        case PAL_LogTypeImportant:
            nsl_strcat(pBuf, "[X]");
            break;
        case PAL_LogTypeCritical:
            nsl_strcat(pBuf, "[#]");
            break;
        case PAL_LogTypeMinor:
            nsl_strcat(pBuf, "[!]");
            break;
        case PAL_LogTypeInfo:
        case PAL_LogTypeMinorInfo:
            nsl_strcat(pBuf, "[i]");
            break;
        case PAL_LogTypeDebug:
            nsl_strcat(pBuf, "[d]");
            break;
        case PAL_LogTypeNone:
        default:
            break;
        }
    }
    // Add the date/time to the buffer
    if(pLog->options & PAL_LOG_OPTION_DATE || pLog->options & PAL_LOG_OPTION_TIME)
    {
        PAL_ClockGetDateTime(&curTime);
        nsl_strcat(pBuf, "[");
        if(pLog->options & PAL_LOG_OPTION_DATE)
        {
            nsl_sprintf(tmpBuf, "%02d/%02d/%02d", curTime.month, curTime.day, curTime.year);
            nsl_strcat(pBuf, tmpBuf);
        }
        if(pLog->options & PAL_LOG_OPTION_TIME)
        {
            if(pLog->options & PAL_LOG_OPTION_DATE)
            {
                nsl_strcat(pBuf, " ");
            }
            nsl_sprintf(tmpBuf, "%02d:%02d:%02d.%03d", curTime.hour, curTime.minute,	
                curTime.second, curTime.milliseconds%1000);	
            nsl_strcat(pBuf, tmpBuf);
        }
        nsl_strcat(pBuf, "]");
    }
    if((pLog->options & PAL_LOG_OPTION_SOURCE && pSrc) || (pLog->options & PAL_LOG_OPTION_FUNCTION && pFunc))
    {        
        nsl_strcat(pBuf, "[");
        if(pLog->options & PAL_LOG_OPTION_SOURCE && pSrc)
        {
#if REMOVE_SOURCE_PATH
            // See if the path has a standard windows path separator
            pTemp = nsl_strrchr(pSrc, '\\');
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
            if(pLog->options & PAL_LOG_OPTION_LINE)
            {
                nsl_sprintf(tmpBuf, ":%d", line);
                nsl_strcat(pBuf, tmpBuf);
            }
        }
        if(pLog->options & PAL_LOG_OPTION_FUNCTION && pFunc)
        {
            if(pLog->options & PAL_LOG_OPTION_SOURCE)
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
    // If there is no end of line character, add it
    if(pBuf[nsl_strlen(pBuf)-1] != '\n')
    {
        nsl_strcat(pBuf, "\r\n");
    }
    err = PAL_FileWrite(pLog->pFile, (uint8 *)pBuf, nsl_strlen(pBuf), &written);
    // Free our buffer memory
    nsl_free(pBuf);
    return(err);
}
PAL_DEF PAL_Error
LogWriteIndent(PAL_Log* pLog, char *pBuf)
{
    int spaces = 0;
    int i = 0;
    if(!pLog || !pBuf)
    {
        return PAL_ErrBadParam;
    }
    if(!pLog->pFile)
    {
        return PAL_ErrFileNotOpen;
    }
    // Ensure that the indent never gets to an invalid value
    if((int16)pLog->indent < 0)
    {
        pLog->indent = 0;
    }
    // If there is no indent currently in affect, simply return
    if(pLog->indent == 0)
    {
        return PAL_Ok;
    }
    spaces = pLog->indent*PALLOG_INDENT_SPACES;
    for(i = 0; i < spaces; i++)
    {
        nsl_strcat(pBuf, " ");
    }
    return(PAL_Ok);
}

/*! @} */
