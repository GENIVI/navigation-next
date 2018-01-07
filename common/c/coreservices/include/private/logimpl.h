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

@file     LogImpl.h
@date     03/11/2009
@defgroup LOG_API Core Log API

@brief    Log Class

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

#ifndef PALLOG_H
#define PALLOG_H

#pragma once

#include "pal.h"
#include "palerror.h"
#include "paltypes.h"
#include "palfile.h"

#define PALLOG_MAX_IDS          32
#define PALLOG_INDENT_SPACES    4

typedef unsigned long   PAL_LogComponentID;

/*! Logging severity. */
typedef enum {
    PAL_LogSeverityNone = 0,                            /*!< No events logged */
    PAL_LogSeverityCritical,                            /*!< Critical event */
    PAL_LogSeverityMajor,                               /*!< Error event */
    PAL_LogSeverityImportant,                           /*!< Error event */
    PAL_LogSeverityMinor,                               /*!< Warning event */
    PAL_LogSeverityInfo,                                /*!< Informational event */
    PAL_LogSeverityMinorInfo,                           /*!< Minor Informational event */
    PAL_LogSeverityDebug,                               /*!< all events logged */
    PAL_LogSeverityBlockBegin,                          /*!< Block begin event */
    PAL_LogSeverityBlockEnd,                            /*!< Block end event */
} PAL_LogSeverity;

/*! Logging structure. */
typedef struct
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

}PAL_Log;

/*! Logging output options */
#define PAL_LOG_OPTION_TIME         (1<<0)          /*!< Show the time in the log entry */
#define PAL_LOG_OPTION_DATE         (1<<1)          /*!< Show the date in the log entry */
#define PAL_LOG_OPTION_SEVERITY     (1<<2)          /*!< Show the severity in the log entry */
#define PAL_LOG_OPTION_SOURCE       (1<<3)          /*!< Show the source filename in the log entry */
#define PAL_LOG_OPTION_FUNCTION     (1<<4)          /*!< Show the function name in the log entry */
#define PAL_LOG_OPTION_LINE         (1<<5)          /*!< Show the line number in the log entry */
#define PAL_LOG_OPTION_COMPONENT    (1<<6)          /*!< Show the name of the component */
#define PAL_LOG_OPTION_ALL          0xFFFF          /*!< Show all information in the log entry */

/*! Creates an instance of the PAL_Log

Call this function first before any other PAL_LogXXX API functions to create the PAL_Log instance. 
The caller is responsible for saving the pointer pointed to by the OUT parameter log.
This PAL_Log pointer is used on all subsequent calls to PAL_Log API calls.
When finished with the logger, the user must call PAL_LogDestroy to close and free the resources used 
by the PAL_Log.

@return PAL_Error on error or PAL_Ok if the PAL_Log is created successfully
    
@see PAL_LogDestroy
*/
PAL_Error		
PAL_LogCreate(
              PAL_Instance* pal,						        /*!< PAL instance */
              const char* filename,                             /*!< Full path of the log filename */
              PAL_Log** log							            /*!< [OUT parameter] that will return a pointer to the PAL_Log.
                                                                The caller must pass this log to every PAL_Log API */
              );


/*! Release the Log instance

This function closes the Log file and releases all associated resources.

@return nothing

@see PAL_LogCreate
*/
void				
PAL_LogDestroy(
               PAL_Log* log			/*!< Pointer to the logger returned on call to PAL_LogCreate */
               );

/*! Register the component

This function registers the component and returns the PAL_LogComponentID for this component.
There are a total of 32 PAL_LogComponentID entries available.  If an attempt is made to
register a component after 32 have already been issued, the function will return PAL_Failed.

@return PAL_Ok on success, PAL_ErrBadParam if invalid component name, PAL_Failed if out of IDs

@see PAL_LogCreate
*/
PAL_Error 
PAL_LogRegisterComponent(
                         PAL_Log* log,		            /*!< Pointer to the logger returned on call to PAL_LogCreate */
                         const char* componentName,       /*!< The name of the component */
                         PAL_LogComponentID* componentId  /*!< The bitmask assigned for the component specified */
                         );


/*! Opens the log

This function opens the log file with the specified filename.  All log entries will be appended to this log.

@return PAL_Ok on success, PAL_ErrFileFailed or  PAL_ErrFileNoPermission on error

@see PAL_LogCreate
@see PAL_LogClose
*/
PAL_Error 
PAL_LogOpen(
            PAL_Log* log,		            /*!< Pointer to the logger returned on call to PAL_LogCreate */
            const char* filename            /*!< The filename of the log */
            );

/*! Closes the log

This function closes the log file.  No further log entries will be appended to this log after this call.

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
*/
PAL_Error 
PAL_LogClose(
             PAL_Log* log			            /*!< Pointer to the logger returned on call to PAL_LogCreate */
             );

/*! Turn on the log

This function turns the logging function on.  All log entries will be appended to the log after this call.

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
*/
PAL_Error 
PAL_LogOn(
          PAL_Log* log,		            /*!< Pointer to the logger returned on call to PAL_LogCreate */
          uint16 optionMask                /*!< The options to turn on for this component.  The options are detailed
                                           in the PAL_LOG_OPTION section. */
                                           );


/*! Turn off the log

This function turns the logging function off.  No log entries will be logged after this call.

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
@see PAL_LogOn
*/
PAL_Error 
PAL_LogOff(
           PAL_Log* log			            /*!< Pointer to the logger returned on call to PAL_LogCreate */
           );

/*! Turn on the logging for specific component

This function turns the logging function on for the specified component.  All log entries will be appended to the log after this call.

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOn
@see PAL_LogOff
@see PAL_LogComponentOff
*/
PAL_Error 
PAL_LogComponentOn(
                   PAL_Log* log,		            /*!< Pointer to the logger returned on call to PAL_LogCreate */
                   PAL_LogComponentID componentId,  /*!< The LogComponentID returned by PAL_LogRegisterComponent */
                   PAL_LogSeverity  severity        /*!< The severity of events to log */
                   );


/*! Turn off the logging for a specific component

This function turns the logging function off for the specified component.  No log entries will be logged after this call.

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
@see PAL_LogOn
@see PAL_LogOff
@see PAL_LogComponentOn
*/
PAL_Error 
PAL_LogComponentOff(
                    PAL_Log* log,		            /*!< Pointer to the logger returned on call to PAL_LogCreate */
                    PAL_LogComponentID componentId   /*!< The LogComponentID returned by PAL_LogRegisterComponent */
                    );

/*! Get status for a specific component

This function returns status information for the specified component. 

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOn
@see PAL_LogOff
@see PAL_LogComponentOn
@see PAL_LogComponentOff
*/
PAL_Error 
PAL_LogComponentGetStatus(
                   PAL_Log* log,		            /*!< Pointer to the logger returned on call to PAL_LogCreate */
                   PAL_LogComponentID componentId,  /*!< The LogComponentID returned by PAL_LogRegisterComponent */
                   PAL_LogSeverity  *severity       /*!< The severity of events that are currently being logged */
                   );

/*! Write a log entry to the log

This function writes a log entry to the log.  The severity will be the first thing on the log line followed by time, component
and text.

The line would look as follows if the PAL_LOG_OPTION_SEVERITY, PAL_LOG_OPTION_TIME, PAL_LOG_OPTION_DATE and 
PAL_LOG_OPTION_MODULE options were on for this component:

[i] [03/15/2009 08:28:12.1234] [Memory] 12560 bytes of memory have been successfully allocated.

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
@see PAL_LogWriteEx
@see PAL_LogWriteBlockBegin
*/
PAL_Error 
PAL_LogWrite(
             PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
             PAL_LogComponentID   componentId,    /*!< The component for this event */
             PAL_LogSeverity  severity,           /*!< The severity of this event */
             const char *pSource,                 /*!< The name of the source module */
             const char *pFunc,                   /*!< The name of the function where the event occured */
             int line,                            /*!< The line number of the source where the event occured */
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

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
*/
PAL_Error 
PAL_LogWriteF(
              PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
              PAL_LogComponentID   componentId,    /*!< The component for this event */
              PAL_LogSeverity  severity,           /*!< The severity of this event */
              const char *pSource,                 /*!< The name of the source module */
              const char *pFunc,                   /*!< The name of the function where the event occured */
              int line,                            /*!< The line number of the source where the event occured */
              const char *fmt, ...                 /*!< The text to place in the log */
              );

/*! Start a new log entry block

This function writes a log entry to the log and includes source file information.  This differs from other log writing 
functions as it will tab all entries after this is called.  This allows the user to specify a begin at the start of a
function and it can easily be seen when the function begins and ends.  Call PAL_LogWriteBlockEnd to end this block.  
Note that you may have multiple calls to PAL_LogWriteBlockBegin which will tab another block for each call.  This allows
for further isolation of sections of code that belong in their own block.

Here is a sample of how the block structure formats the log output for easily readiblity.

[B] PAL_MemoryAlloc - Begin
    [i] Allocating 25K of memory
    [i] Clearing all memory to zero
    [i] Checking memory
    [B] PAL_MemoryCheck - Begin
        [i] Checking the memory
        [i] Double-checking the memory
        [i] Memory checks out OK
    [E] PAL_MemoryCheck - End
    [i] Memory allocated and cleared successfully
    [i] Returning PAL_Ok
[E] PAL_MemoryAlloc - End

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
@see PAL_LogWrite
@see PAL_LogWriteEx
@see PAL_LogWriteBlockEnd
*/
PAL_Error 
PAL_LogWriteBlockBegin(
                       PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
                       PAL_LogComponentID   componentId,    /*!< The component for this event */
                       const char *pSource,                 /*!< The name of the source module */
                       const char *pFunc,                   /*!< The name of the function where the event occured */
                       int line,                            /*!< The line number of the source where the event occured */
                       const char *pText                    /*!< The text to place in the log */
                       );

/*! Ends a log entry block

This function writes a log entry to the log and includes source file information.  This differs from other log writing 
functions as it will tab all entries after this is called.  This allows the user to specify a begin at the start of a
function and it can easily be seen when the function begins and ends.  Call PAL_LogWriteBlockBegine to start the block.  
Note that you may have multiple calls to PAL_LogWriteBlockBegin which will tab another block for each call.  This allows
for further isolation of sections of code that belong in their own block.

Here is a sample of how the block structure formats the log output for easily readiblity.

[B] PAL_MemoryAlloc - Begin
    [i] Allocating 25K of memory
    [i] Clearing all memory to zero
    [i] Checking memory
    [B] PAL_MemoryCheck - Begin
        [i] Checking the memory
        [i] Double-checking the memory
        [i] Memory checks out OK
    [E] PAL_MemoryCheck - End
    [i] Memory allocated and cleared successfully
    [i] Returning PAL_Ok
[E] PAL_MemoryAlloc - End

@return PAL_Ok on success, PAL_Error on failure

@see PAL_LogCreate
@see PAL_LogOpen
@see PAL_LogWrite
@see PAL_LogWriteEx
@see PAL_LogWriteBlockBegin
*/
PAL_Error 
PAL_LogWriteBlockEnd(
                     PAL_Log* log,		                /*!< Pointer to the logger returned on call to PAL_LogCreate */
                     PAL_LogComponentID   componentId,    /*!< The component for this event */
                     const char *pSource,                 /*!< The name of the source module */
                     const char *pFunc,                   /*!< The name of the function where the event occured */
                     int line,                            /*!< The line number of the source where the event occured */
                     const char *pText                    /*!< The text to place in the log */
                     );

#endif  //PALLOG_H

/*! @} */
