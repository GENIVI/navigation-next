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

    @file nbrelog.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_LOG_H_
#define _NBRE_LOG_H_
#include "palerror.h"
#include "nbretypes.h"
#include "pallog.h"

class NBRE_IOStream;

#define NBRE_LOG_MAX_IDS          32
#define NBRE_LOG_INDENT_SPACES    4

typedef unsigned long   NBRE_LogComponentID;

/*! Logging output options */
typedef enum
{
    NBRE_LO_TIME        = (1<<0),          /*!< Show the time in the log entry */
    NBRE_LO_DATE        = (1<<1),          /*!< Show the date in the log entry */
    NBRE_LO_SEVERITY    = (1<<2),          /*!< Show the severity in the log entry */
    NBRE_LO_SOURCE      = (1<<3),          /*!< Show the source filename in the log entry */
    NBRE_LO_FUNCTION    = (1<<4),          /*!< Show the function name in the log entry */
    NBRE_LO_LINE        = (1<<5),          /*!< Show the line number in the log entry */
    NBRE_LO_COMPONENT   = (1<<6),          /*!< Show the name of the component */
    NBRE_LO_ALL         = 0xFFFF          /*!< Show all information in the log entry */
}NBRE_LogOptions;

/*! Logging output options */
typedef enum
{
    NBRE_LSS_MEMORY     = (1<<0),
    NBRE_LSS_FILE       = (1<<1),
}NBRE_LogSubSystem;

class NBRE_Log
{
public:
    /*! Creates an instance of the NBRE_Log

    Call this function first before any other NBRE_LogXXX API functions to create the NBRE_Log instance. 
    The caller is responsible for saving the pointer pointed to by the OUT parameter log.
    This NBRE_Log pointer is used on all subsequent calls to NBRE_Log API calls.
    When finished with the logger, the user must call NBRE_LogDestroy to close and free the resources used 
    by the NBRE_Log.

    @return PAL_Error on error or NBRE_Ok if the NBRE_Log is created successfully
    
    @see NBRE_LogDestroy
    */
    NBRE_Log(
            NBRE_IOStream* istream,                           /*!< The stream used to store log content */
            const char* logName,                              /*!< Log name, typecally it is the log file name */
            NBRE_LogSubSystem subSystem
            );


    /*! Release the Log instance

    This function closes the Log file and releases all associated resources.

    @return nothing

    @see NBRE_LogCreate
    */
    ~NBRE_Log();

public:
    /*! Register the component

    This function registers the component and returns the NBRE_LogComponentID for this component.
    There are a total of 32 NBRE_LogComponentID entries available.  If an attempt is made to
    register a component after 32 have already been issued, the function will return NBRE_Failed.

    @return NBRE_Ok on success, PAL_ErrBadParam if invalid component name, NBRE_Failed if out of IDs

    @see NBRE_LogCreate
    */
    PAL_Error 
    RegisterComponent(
                    const char* componentName,       /*!< The name of the component */
                    NBRE_LogComponentID* componentId  /*!< The bitmask assigned for the component specified */
                    );


    /*! Opens the log

    This function opens the log file with the specified filename.  All log entries will be appended to this log.

    @return NBRE_Ok on success, PAL_ErrFileFailed or  NBRE_ErrFileNoPermission on error

    @see NBRE_LogCreate
    @see NBRE_LogClose
    */
    PAL_Error 
    Open(
        const char* filename            /*!< The filename of the log */
        );

    /*! Closes the log

    This function closes the log file.  No further log entries will be appended to this log after this call.

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    */
    PAL_Error Close();

    /*! Turn on the log

    This function turns the logging function on.  All log entries will be appended to the log after this call.

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    */
    PAL_Error 
    On(
        uint16 optionMask                /*!< The options to turn on for this component.  The options are detailed
                                        in the NBRE_LOG_OPTION section. */
        );


    /*! Turn off the log

    This function turns the logging function off.  No log entries will be logged after this call.

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    @see NBRE_LogOn
    */
    PAL_Error Off();

    /*! Turn on the logging for specific component

    This function turns the logging function on for the specified component.  All log entries will be appended to the log after this call.

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOn
    @see NBRE_LogOff
    @see NBRE_LogComponentOff
    */
    PAL_Error 
    ComponentOn(
                NBRE_LogComponentID componentId,  /*!< The LogComponentID returned by NBRE_LogRegisterComponent */
                PAL_LogSeverity  severity        /*!< The severity of events to log */
                );



    /*! Turn off the logging for a specific component

    This function turns the logging function off for the specified component.  No log entries will be logged after this call.

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    @see NBRE_LogOn
    @see NBRE_LogOff
    @see NBRE_LogComponentOn
    */
    PAL_Error 
    ComponentOff(
                NBRE_LogComponentID componentId   /*!< The LogComponentID returned by NBRE_LogRegisterComponent */
                );

    /*! Get status for a specific component

    This function returns status information for the specified component. 

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOn
    @see NBRE_LogOff
    @see NBRE_LogComponentOn
    @see NBRE_LogComponentOff
    */
    PAL_Error 
    ComponentGetStatus(
                       NBRE_LogComponentID componentId,  /*!< The LogComponentID returned by NBRE_LogRegisterComponent */
                       PAL_LogSeverity  *severity       /*!< The severity of events that are currently being logged */
                       );

    /*! Write a log entry to the log

    This function writes a log entry to the log.  The severity will be the first thing on the log line followed by time, component
    and text.

    The line would look as follows if the NBRE_LOG_OPTION_SEVERITY, NBRE_LOG_OPTION_TIME, NBRE_LOG_OPTION_DATE and 
    NBRE_LOG_OPTION_MODULE options were on for this component:

    [i] [03/15/2009 08:28:12.1234] [Memory] 12560 bytes of memory have been successfully allocated.

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    @see NBRE_LogWriteEx
    @see NBRE_LogWriteBlockBegin
    */
    PAL_Error 
    Write(
        NBRE_LogComponentID   componentId,    /*!< The component for this event */
        PAL_LogSeverity  severity,           /*!< The severity of this event */
        const char *pSource,                 /*!< The name of the source module */
        const char *pFunc,                   /*!< The name of the function where the event occured */
        int line,                            /*!< The line number of the source where the event occured */
        const char *pText                    /*!< The text to place in the log */
        );

    PAL_Error
    Write(
          NBRE_LogComponentID   componentId,    /*!< The component for this event */
          PAL_LogSeverity  severity,           /*!< The severity of this event */
          const char *pText                    /*!< The text to place in the log */
          );

    /*! Write a variable format log entry to the log with source file information

    This function writes a log entry to the log and includes source file information.  The severity will be the first thing on the 
    log line followed by time, component and text.
    Each log entry will start with the severity of the event if this option is enabled.  The severity log prefixes are show below.

    [i] Information
    [B] Begin formatted block
    [E] End formatted block
    [!] Warning
    [X] Error
    [d] Debug
    [#] Critical error

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    */
    PAL_Error 
    WriteF(
        NBRE_LogComponentID   componentId,    /*!< The component for this event */
        PAL_LogSeverity  severity,           /*!< The severity of this event */
        const char *pSource,                 /*!< The name of the source module */
        const char *pFunc,                   /*!< The name of the function where the event occured */
        int line,                            /*!< The line number of the source where the event occured */
        const char *fmt, ...                 /*!< The text to place in the log */
        );

    PAL_Error
    WriteF(
           NBRE_LogComponentID   componentId,    /*!< The component for this event */
           PAL_LogSeverity  severity,           /*!< The severity of this event */
           const char *fmt, ...                 /*!< The text to place in the log */
           );

    /*! Start a new log entry block

    This function writes a log entry to the log and includes source file information.  This differs from other log writing 
    functions as it will tab all entries after this is called.  This allows the user to specify a begin at the start of a
    function and it can easily be seen when the function begins and ends.  Call NBRE_LogWriteBlockEnd to end this block.  
    Note that you may have multiple calls to NBRE_LogWriteBlockBegin which will tab another block for each call.  This allows
    for further isolation of sections of code that belong in their own block.

    Here is a sample of how the block structure formats the log output for easily readiblity.

    [B] NBRE_MemoryAlloc - Begin
        [i] Allocating 25K of memory
        [i] Clearing all memory to zero
        [i] Checking memory
        [B] NBRE_MemoryCheck - Begin
            [i] Checking the memory
            [i] Double-checking the memory
            [i] Memory checks out OK
        [E] NBRE_MemoryCheck - End
        [i] Memory allocated and cleared successfully
        [i] Returning NBRE_Ok
    [E] NBRE_MemoryAlloc - End

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    @see NBRE_LogWrite
    @see NBRE_LogWriteEx
    @see NBRE_LogWriteBlockEnd
    */
    PAL_Error 
    WriteBlockBegin(
                    NBRE_LogComponentID   componentId,    /*!< The component for this event */
                    const char *pSource,                 /*!< The name of the source module */
                    const char *pFunc,                   /*!< The name of the function where the event occured */
                    int line,                            /*!< The line number of the source where the event occured */
                    const char *pText                    /*!< The text to place in the log */
                    );

    PAL_Error
    WriteBlockBegin(
                    NBRE_LogComponentID   componentId,    /*!< The component for this event */
                    const char *pText                    /*!< The text to place in the log */
                    );

    /*! Ends a log entry block

    This function writes a log entry to the log and includes source file information.  This differs from other log writing 
    functions as it will tab all entries after this is called.  This allows the user to specify a begin at the start of a
    function and it can easily be seen when the function begins and ends.  Call NBRE_LogWriteBlockBegine to start the block.  
    Note that you may have multiple calls to NBRE_LogWriteBlockBegin which will tab another block for each call.  This allows
    for further isolation of sections of code that belong in their own block.

    Here is a sample of how the block structure formats the log output for easily readiblity.

    [B] NBRE_MemoryAlloc - Begin
        [i] Allocating 25K of memory
        [i] Clearing all memory to zero
        [i] Checking memory
        [B] NBRE_MemoryCheck - Begin
            [i] Checking the memory
            [i] Double-checking the memory
            [i] Memory checks out OK
        [E] NBRE_MemoryCheck - End
        [i] Memory allocated and cleared successfully
        [i] Returning NBRE_Ok
    [E] NBRE_MemoryAlloc - End

    @return NBRE_Ok on success, PAL_Error on failure

    @see NBRE_LogCreate
    @see NBRE_LogOpen
    @see NBRE_LogWrite
    @see NBRE_LogWriteEx
    @see NBRE_LogWriteBlockBegin
    */
    PAL_Error 
    WriteBlockEnd(
                NBRE_LogComponentID   componentId,    /*!< The component for this event */
                const char *pSource,                 /*!< The name of the source module */
                const char *pFunc,                   /*!< The name of the function where the event occured */
                int line,                            /*!< The line number of the source where the event occured */
                const char *pText                    /*!< The text to place in the log */
                );

    PAL_Error
    WriteBlockEnd(
                  NBRE_LogComponentID   componentId,    /*!< The component for this event */
                  const char *pText                    /*!< The text to place in the log */
                  );

private:
    /*! Log type. */
    typedef enum {
        LogTypeNone = 0,                            /*!< No events logged */
        LogTypeCritical,                            /*!< Critical event */
        LogTypeMajor,                               /*!< Error event */
        LogTypeImportant,                           /*!< Error event */
        LogTypeMinor,                               /*!< Warning event */
        LogTypeInfo,                                /*!< Informational event */
        LogTypeMinorInfo,                           /*!< Informational event */
        LogTypeDebug,                               /*!< all events logged */
        LogTypeBlockBegin,                          /*!< Block begin event */
        LogTypeBlockEnd,                            /*!< Block end event */
    }LogType;

    nb_boolean IsValidComponent(NBRE_LogComponentID id);
    PAL_Error LogWrite(NBRE_LogComponentID componentId, const char* pSrc, 
                          const char* pFunc,  uint32 line, const char* pText, LogType type);
    PAL_Error LogWriteIndent(char *pBuf);

private:
    NBRE_IOStream*      pFile;                                  /*!<  Log file */
    nb_boolean          power;                                  /*!< On/Off toggle for global logging */
    uint32              options;                                /*!< The logging options */
    uint16              indent;                                 /*!< The current indent level */
    char*               pComponentName[NBRE_LOG_MAX_IDS];       /*!< Component name registered by client */
    nb_boolean          componentPower[NBRE_LOG_MAX_IDS];       /*!< Component level power on/off toggle */
    PAL_LogSeverity     componentSeverity[NBRE_LOG_MAX_IDS];    /*!< The log severity for each component */
    char*               logName;
    NBRE_LogSubSystem   subSystem;
};

void NBRE_SetDebugLogSeverity(PAL_LogSeverity);
void NBRE_DebugLog(PAL_LogSeverity, const char* fmt, ...);

#endif
