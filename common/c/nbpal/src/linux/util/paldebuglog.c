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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2008 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * nimdebuglog.h: created 2008/09/04 by Mark Goddard.
 */
#include "paldebuglog.h"
#include "palstdlib.h"
#include "paltypes.h"
#include "palfile.h"
#include "palclock.h"
#include "pallock.h"

//#ifndef NDEBUG
PAL_DEF void
debugf(const char *fmt, ...)
{
    char formattedString[1024] = {0};

    // Format argument list
    va_list argumentList;
    va_start(argumentList, fmt);
    vsprintf(formattedString, fmt, argumentList);
    
    int length = nsl_strlen(formattedString);
    formattedString[length] = '\n';
    /*formattedString[length] = '\r';
    formattedString[length+1] = '\n';*/

    // Ouput formatted string. We could also log to file here if desired.
    printf("%s", formattedString);
}

PAL_DEF void debuglog(const char* fmt, ...)
{
    static const char DEBUG_FILE_NAME[] = "PalDebugFile.txt";
    static PAL_Lock* pLock = NULL;
    // KLUDGE:
    // Pal is not used in file functions but is checked against NULL! Just pass in some random value!
    static PAL_Instance* pseudoPAL = (PAL_Instance*)0xDEADDEAD;

    PAL_File* pDebugFile = NULL;
    uint32 bytesWritten = 0;
    
    if (pLock == NULL)
    {
        PAL_LockCreate(pseudoPAL, &pLock);
    }
    
    {
        PAL_LockLock(pLock);
        
        PAL_Error result = PAL_FileOpen(pseudoPAL, DEBUG_FILE_NAME, PFM_Append, &pDebugFile);
        if ((result != PAL_Ok) || (pDebugFile == NULL))
        {
            return;
        }
        
        // Preprend date and time
        PAL_ClockDateTime time = {0};
        if (PAL_ClockGetDateTime(&time) == PAL_Ok)
        {
            char timeString[128] = {0};
            nsl_sprintf(timeString, "%04d-%02d-%02d %02d:%02d:%02d.%06d ", time.year, time.month, time.day, time.hour, time.minute, time.second, time.milliseconds);
            PAL_FileWrite(pDebugFile, (uint8*)timeString, nsl_strlen(timeString), &bytesWritten);
        }
        
        char formattedString[1024] = {0};
        
        // Format argument list
        va_list argumentList;
        va_start(argumentList, fmt);
        vsprintf(formattedString, fmt, argumentList);
        
        PAL_FileWrite(pDebugFile, (uint8*)formattedString, nsl_strlen(formattedString), &bytesWritten);
        
        const char lineBreak[] = "\r\n";
        PAL_FileWrite(pDebugFile, (uint8*)lineBreak, nsl_strlen(lineBreak), &bytesWritten);
        
        PAL_FileClose(pDebugFile);
        
        PAL_LockUnlock(pLock);
    }
}

/*! @} */
