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
#include "nbrefilestream.h"
#include "nbrelog.h"

NBRE_FileStream::NBRE_FileStream(PAL_Instance* pal, const char* filePath, uint32 cacheSize):
    NBRE_IOStream(0, cacheSize),
    mFile(NULL),
    mSelfFile(TRUE)
{
    PAL_Error err = PAL_Ok;
    uint32 fileSize = 0;
    err = PAL_FileGetSize(pal, filePath, &fileSize);
    err = (err != PAL_Ok)?err:PAL_FileOpen(pal, filePath, PFM_Read, &mFile);
    if(err == PAL_Ok)
    {
        NBRE_IOStream::SetDataSize(fileSize);
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_FileStream, create from file failed, filePath=%s, err=%x", filePath, err);
        nbre_assert(FALSE);
    }
}

NBRE_FileStream::NBRE_FileStream( PAL_Instance* pal, const char* filePath, PAL_FileMode mode, uint32 cacheSize )
:NBRE_IOStream(0, cacheSize, mode), mFile(NULL), mSelfFile(TRUE)
{
    PAL_Error err = PAL_Ok;
    uint32 fileSize = 0;

    err = PAL_FileGetSize(pal, filePath, &fileSize);
    err = (err!=PAL_Ok)?err:PAL_FileOpen(pal, filePath, mode, &mFile);

    if(err == PAL_Ok)
    {
        SetDataSize(fileSize);
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_FileStream, create from file failed, filePath=%s, err=%x", filePath, err);
        nbre_assert(FALSE);
    }
}

NBRE_FileStream::NBRE_FileStream(PAL_File* file, uint32 fileSize, uint32 cacheSize):
NBRE_IOStream(fileSize, cacheSize),
mFile(file),
mSelfFile(FALSE)
{

}

NBRE_FileStream::NBRE_FileStream(PAL_File* file, uint32 fileSize, PAL_FileMode mode, uint32 cacheSize):
NBRE_IOStream(fileSize, cacheSize, mode),
mFile(file),
mSelfFile(FALSE)
{

}

NBRE_FileStream::~NBRE_FileStream()
{
    Flush();

    if(mSelfFile)
    {
        PAL_FileClose(mFile);
    }
}

PAL_Error NBRE_FileStream::ReadImpl(uint8* recvBuff, uint32 count, uint32* readBytes)
{
    if (PAL_FileRead(mFile, recvBuff, count, readBytes) != PAL_Ok)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

PAL_Error NBRE_FileStream::SeekImpl(uint32 offset)
{
    if (PAL_FileSetPosition(mFile, PFSO_Start, offset) != PAL_Ok)
    {
        return PAL_ErrFileFailed;
    }
    return PAL_Ok;
}

PAL_Error
NBRE_FileStream::WriteImpl( const uint8* buffer, uint32 bufferSize, uint32* bytesWritten )
{
    return PAL_FileWrite(mFile, buffer, bufferSize, bytesWritten);
}
