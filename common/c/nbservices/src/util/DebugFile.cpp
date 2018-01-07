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

	@file     DebugFile.cpp
	@date     02/22/2011

*/
/*
    See header file for description.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/


#include "DebugFile.h"
#include "stdio.h"
#include "stdarg.h"
#include "palclock.h"
#include "pallock.h"
#include "smartpointer.h"


//static PAL_File* pDebugFile = NULL;
static const char DEBUG_FILE_NAME[] = "DebugFile.txt";
static bool firstOutput = true;

static PAL_Lock* pLock = NULL;

// KLUDGE:
// Pal is not used in file functions but is checked against NULL! Just pass in some random value!
static shared_ptr<int> pseudoPAL(new int);


/* See header file for description */
void 
WriteDebugToFile(const char* output, ...)
{
    PAL_File* pDebugFile = NULL;
    uint32 bytesWritten = 0;

    if (pLock == NULL)
    {
        PAL_LockCreate((PAL_Instance*)pseudoPAL.get(), &pLock);
    }
    
    {
        PAL_LockLock(pLock);

        // Open existing file, create if it doesn't exist. Uncomment "PFM_Create" if you want to create a new file each time the app starts.
        PAL_Error result = PAL_FileOpen((PAL_Instance*)pseudoPAL.get(), DEBUG_FILE_NAME, firstOutput ? PFM_Append /* PFM_Create */ : PFM_Append, &pDebugFile);
        if ((result != PAL_Ok) || (pDebugFile == NULL))
        {
            return;
        }
        firstOutput = false;

        // Preprend date and time
        PAL_ClockDateTime time = {0};
        if (PAL_ClockGetDateTime(&time) == PAL_Ok)
        {
            char timeString[128] = {0};
            nsl_sprintf(timeString, "%d/%d/%d %02d:%02d:%02d %03d:\t", time.month, time.day, time.year, time.hour, time.minute, time.second, time.milliseconds);
            PAL_FileWrite(pDebugFile, (uint8*)timeString, (uint32)nsl_strlen(timeString), &bytesWritten);
            
#ifdef DEBUG_TO_CONSOLE
            // Also output to debug console. I don't know why we need to "" at the end, but otherwise we get compile errors.
            printf(timeString, "");
#endif
        }

        char formattedString[512] = {0};
        
        // Format argument list 
        va_list argumentList;
        va_start(argumentList, output);
        vsprintf(formattedString, output, argumentList);
        
        PAL_FileWrite(pDebugFile, (uint8*)formattedString, (uint32)nsl_strlen(formattedString), &bytesWritten);
        
#ifdef DEBUG_TO_CONSOLE
        // Also output to debug console. I don't know why we need to "" at the end, but otherwise we get compile errors.
        printf(formattedString, "");
#endif
        const char lineBreak[] = "\r\n";
        PAL_FileWrite(pDebugFile, (uint8*)lineBreak, (uint32)nsl_strlen(lineBreak), &bytesWritten);
#ifdef DEBUG_TO_CONSOLE
        // Also output to debug console. 
        printf("%s",lineBreak);
#endif
        PAL_FileClose(pDebugFile);
        
        PAL_LockUnlock(pLock);
    }
}



