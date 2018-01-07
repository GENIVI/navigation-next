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

#include "nbrememorystream.h"

NBRE_MemoryStream::NBRE_MemoryStream(const uint8* srcData, uint32 srcDataSize, nb_boolean duplicateData)
: NBRE_IOStream(srcDataSize, 0),
  mDuplicateData(duplicateData),
  mDataStartAddr(NULL),
  mDataEndAddr(NULL),
  mDataCurrentAddr(NULL)
{
    if (duplicateData)
    {
        if (srcDataSize > 0)
        {
            mDataStartAddr = (uint8*)nsl_malloc(srcDataSize);
            nbre_assert(mDataStartAddr && srcData);
            nsl_memcpy(mDataStartAddr, srcData, srcDataSize);
        }
    }
    else
    {
        mDataStartAddr = const_cast<uint8*>(srcData);
    }

    mDataCurrentAddr = mDataStartAddr;
    mDataEndAddr = mDataStartAddr + srcDataSize;
}

NBRE_MemoryStream::~NBRE_MemoryStream()
{
    Flush();
    if (mDuplicateData)
    {
        if (mDataStartAddr != NULL)
        {
            nsl_free(mDataStartAddr);
        }
    }
}

PAL_Error NBRE_MemoryStream::ReadImpl(uint8* recvBuff, uint32 count, uint32* readBytes)
{
    if ((mDataEndAddr < mDataStartAddr) || (mDataCurrentAddr > mDataEndAddr) || (mDataCurrentAddr < mDataStartAddr))
    {
        return PAL_ErrBadParam;
    }

    uint32 dataReminder = static_cast<uint32>(mDataEndAddr - mDataCurrentAddr);

    if (count >= dataReminder)
    {
        nsl_memcpy(recvBuff, mDataCurrentAddr, dataReminder);
        mDataCurrentAddr = mDataEndAddr;
        *readBytes = dataReminder;
    }
    else
    {
        nsl_memcpy(recvBuff, mDataCurrentAddr, count);
        mDataCurrentAddr += count;
        *readBytes = count;
    }

    return PAL_Ok;
}

PAL_Error NBRE_MemoryStream::SeekImpl(uint32 offset)
{
    if ((mDataEndAddr < mDataStartAddr) || (mDataCurrentAddr > mDataEndAddr) || (mDataCurrentAddr < mDataStartAddr))
    {
        return PAL_ErrBadParam;
    }

    uint32 upperBounding = static_cast<uint32>(mDataEndAddr - mDataStartAddr);

    if (offset >= upperBounding)
    {
        mDataCurrentAddr = mDataEndAddr;
    }
    else
    {
        mDataCurrentAddr = mDataStartAddr + offset;
    }

    return PAL_Ok;
}

PAL_Error
NBRE_MemoryStream::WriteImpl( const uint8* /*buffer*/, uint32 /*bufferSize*/, uint32* /*bytesWritten*/ )
{
    return PAL_ErrUnsupported;
}
