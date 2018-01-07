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

    @file nbreiostream.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_IO_STREAM_H_
#define _NBRE_IO_STREAM_H_

#include "paltypes.h"
#include "palerror.h"
#include "palfile.h"
#include "nbrecommon.h"
#include "nbretypes.h"
#include "nbrelog.h"

class NBRE_IOStream
{
public:
    virtual ~NBRE_IOStream();

public:
    PAL_Error Read(uint8* recvBuff, uint32 count, uint32* readBytes);
    PAL_Error Write(const uint8* buffer, uint32 bufferSize, uint32* bytesWritten);
    PAL_Error Seek(PAL_FileSetOrigin type, int32 offset);
    uint32 GetPos() const;
    nb_boolean Eof() const;
    uint32 GetSize() const;
    PAL_Error Flush();

    inline NBRE_IOStream& operator>>(uint16&);
    inline NBRE_IOStream& operator>>(uint32&);
    inline NBRE_IOStream& operator>>(int16&);
    inline NBRE_IOStream& operator>>(int32&);
    inline NBRE_IOStream& operator>>(float&);

protected:
    NBRE_IOStream(uint32 dataSize, uint32 cacheSize);
    NBRE_IOStream(uint32 cacheSize);

    NBRE_IOStream(uint32 dataSize, uint32 cacheSize, PAL_FileMode mMode);
    NBRE_IOStream(uint32 cacheSize, PAL_FileMode mMode);

    void Init(uint32 cacheSize);
    void SetDataSize(uint32 dataSize);

private:
    virtual PAL_Error ReadImpl(uint8* recvBuff, uint32 count, uint32* readBytes) = 0;
    virtual PAL_Error WriteImpl(const uint8* buffer, uint32 bufferSize, uint32* bytesWritten) = 0;
    virtual PAL_Error SeekImpl(uint32 offset) = 0;

    struct IOStreamBuffer
    {
        uint8*              mBuffer;
        uint32              mSize;
        uint32              mHead;
        uint32              mTail;
    };
    void InitStreamBuffer(IOStreamBuffer& buffer, uint32 cacheSize);

private:
    IOStreamBuffer          mReadStreamBuffer;
    IOStreamBuffer          mWriteStreamBuffer;
    uint32                  mDataTotalSize;
    uint32                  mReadBytesCounter;

    PAL_FileMode            mMode;
};

#include "nbreiostream.inl"
#endif

