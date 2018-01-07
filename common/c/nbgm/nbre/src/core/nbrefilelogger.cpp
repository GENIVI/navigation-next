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
#include "nbrefilelogger.h"

#define ENABEL_VERBOSE_LOG_FILE 0

NBRE_FileLogger::NBRE_FileLogger( PAL_Instance* pal, const char* file ):
    mLogFile(NULL),
    mBuffer(NULL),
    mCurrentBufferSize(0)
{
    mLogFile = NULL;
    mBuffer = NULL;
    mCurrentBufferSize = 0;
#if ENABEL_VERBOSE_LOG_FILE == 1
    if(pal!=NULL && file!=NULL)
    {
        PAL_FileOpen(pal, file, PFM_Create, &mLogFile);
    }
    mBuffer = (uint8*)nsl_malloc(NBRE_LOG_BUFFER_SIZE);
    if(mBuffer != NULL)
    {
        nsl_memset(mBuffer, 0, NBRE_LOG_BUFFER_SIZE);
    }
#else
#endif
}

NBRE_FileLogger::~NBRE_FileLogger()
{
#if ENABEL_VERBOSE_LOG_FILE == 1
    Flush();
    if(mLogFile != NULL)
    {
        PAL_FileClose(mLogFile);
    }
    if(mBuffer != NULL)
    {
        nsl_free(mBuffer);
    }
#else
#endif
}

void NBRE_FileLogger::Write( const char* fmt, ... )
{
#if ENABEL_VERBOSE_LOG_FILE == 1
    va_list ap;
    static const uint16 len = 1024;
    char buf[len] ;
    nsl_memset(buf, 0, len);

    va_start(ap, fmt);
    nsl_vsnprintf(buf, len-1, fmt, ap);
    va_end(ap);
    WriteBuffer(buf);
#else
#endif
}

void NBRE_FileLogger::WriteBuffer( const char* buf )
{
#if ENABEL_VERBOSE_LOG_FILE == 1
    if(mLogFile == NULL ||
        mBuffer == NULL)
    {
        return;
    }

    uint32 len = nsl_strlen(buf);
    if(len > NBRE_LOG_BUFFER_SIZE || len + mCurrentBufferSize > NBRE_LOG_BUFFER_SIZE)
    {
        Flush();
    }

    if(len > NBRE_LOG_BUFFER_SIZE)
    {
        uint32 bytesWriten=0;
        PAL_FileWrite(mLogFile, (uint8*)buf, len, &bytesWriten);
        len = 0;
    }
    else
    {
        nsl_memcpy(mBuffer+mCurrentBufferSize, buf, len);
        mCurrentBufferSize += len;
    }
#else
#endif
}

void NBRE_FileLogger::Flush()
{
#if ENABEL_VERBOSE_LOG_FILE == 1
    if(mLogFile != NULL && mBuffer != NULL && mCurrentBufferSize>0)
    {
        uint32 bytesWriten=0;
        PAL_FileWrite(mLogFile, mBuffer, mCurrentBufferSize, &bytesWriten);
        nsl_memset(mBuffer, 0, NBRE_LOG_BUFFER_SIZE);
        mCurrentBufferSize = 0;
    }
#else
#endif
}
