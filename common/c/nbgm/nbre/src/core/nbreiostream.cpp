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

#include "nbreiostream.h"
#include "palmath.h"

void
NBRE_IOStream::InitStreamBuffer( IOStreamBuffer& buffer, uint32 cacheSize)
{
    buffer.mBuffer = NULL;
    buffer.mHead = buffer.mTail = 0;
    buffer.mSize = cacheSize;

    if (buffer.mSize != 0)
    {
        buffer.mBuffer = static_cast<uint8*>(nsl_malloc(cacheSize));
        nbre_assert(buffer.mBuffer);
        nsl_memset(buffer.mBuffer, 0, cacheSize);
    }
}

void
NBRE_IOStream::Init(uint32 cacheSize)
{
    switch(mMode)
    {
    case PFM_Read:
        InitStreamBuffer(mReadStreamBuffer, cacheSize);
        InitStreamBuffer(mWriteStreamBuffer, 0);
        break;

    case PFM_ReadWrite:
        InitStreamBuffer(mReadStreamBuffer,cacheSize);
        InitStreamBuffer(mWriteStreamBuffer,cacheSize);
        break;

    case PFM_Create:
    case PFM_Append:
        InitStreamBuffer(mReadStreamBuffer, 0);
        InitStreamBuffer(mWriteStreamBuffer,cacheSize);
        break;

    default:
        nsl_assert(FALSE);
    }

    mReadBytesCounter = 0;
}

NBRE_IOStream::NBRE_IOStream(uint32 dataSize, uint32 cacheSize):
mDataTotalSize(dataSize), mMode(PFM_Read)
{
    Init(cacheSize);
}

NBRE_IOStream::NBRE_IOStream(uint32 cacheSize):
mDataTotalSize(0), mMode(PFM_Read)
{
    Init(cacheSize);
}

NBRE_IOStream::NBRE_IOStream(uint32 dataSize, uint32 cacheSize, PAL_FileMode mode):
    mDataTotalSize(dataSize), mMode(mode)
{
    Init(cacheSize);
}

NBRE_IOStream::NBRE_IOStream(uint32 cacheSize, PAL_FileMode mode):
    mDataTotalSize(0), mMode(mode)
{
    Init(cacheSize);
}

NBRE_IOStream::~NBRE_IOStream()
{
    if (mReadStreamBuffer.mBuffer != NULL)
    {
        nsl_free(mReadStreamBuffer.mBuffer);
    }
    if (mWriteStreamBuffer.mBuffer != NULL)
    {
        nsl_free(mWriteStreamBuffer.mBuffer);
    }
}

void
NBRE_IOStream::SetDataSize(uint32 dataSize)
{
    mDataTotalSize = dataSize;
}

PAL_Error
NBRE_IOStream::Read(uint8* recvBuff, uint32 count, uint32* readBytes)
{
    if (recvBuff == NULL || readBytes == NULL)
    {
        return PAL_ErrBadParam;
    }

    if ((mDataTotalSize < mReadBytesCounter) || (mReadStreamBuffer.mTail < mReadStreamBuffer.mHead))
    {
        return PAL_ErrBadParam;
    }

    if(Eof())
    {
        *readBytes = 0;
        return PAL_ErrEOF;
    }

    if (mReadStreamBuffer.mSize == 0)
    {
        PAL_Error err = ReadImpl(recvBuff, count, readBytes);
        if (err != PAL_Ok)
        {
            return err;
        }
        mReadBytesCounter += *readBytes;
    }
    else
    {
        uint32 bufDataReminder = mReadStreamBuffer.mTail - mReadStreamBuffer.mHead;

        if (count > mReadStreamBuffer.mSize + bufDataReminder)
        {
            nsl_memcpy( recvBuff, mReadStreamBuffer.mBuffer + mReadStreamBuffer.mHead, bufDataReminder);

            if (ReadImpl(recvBuff + bufDataReminder, count - bufDataReminder, readBytes) != PAL_Ok)
            {
                return PAL_ErrFileFailed;
            }

            *readBytes += bufDataReminder;
            mReadBytesCounter += *readBytes;
            mReadStreamBuffer.mTail = mReadStreamBuffer.mHead = 0;
        }
        else if (count > bufDataReminder)
        {
            nsl_memcpy(recvBuff, mReadStreamBuffer.mBuffer + mReadStreamBuffer.mHead, bufDataReminder);

            if (ReadImpl(mReadStreamBuffer.mBuffer, mReadStreamBuffer.mSize, &mReadStreamBuffer.mTail) != PAL_Ok)
            {
                return PAL_ErrFileFailed;
            }

            uint32 copyData = (mReadStreamBuffer.mTail > count - bufDataReminder) ? count - bufDataReminder : mReadStreamBuffer.mTail;
            nsl_memcpy( recvBuff + bufDataReminder, mReadStreamBuffer.mBuffer, copyData );
            mReadStreamBuffer.mHead = copyData;
            *readBytes = bufDataReminder + copyData;
            mReadBytesCounter += *readBytes;
        }
        else
        {
            nsl_memcpy( recvBuff, mReadStreamBuffer.mBuffer + mReadStreamBuffer.mHead, count );
            mReadStreamBuffer.mHead += count;
            mReadBytesCounter += count;
            *readBytes = count;
        }
    }

    return PAL_Ok;
}

PAL_Error
NBRE_IOStream::Write(const uint8* buffer, uint32 bufferSize, uint32* bytesWritten)
{
    if (buffer == NULL || bytesWritten == NULL)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error err = PAL_Ok;

    if (mWriteStreamBuffer.mSize == 0)
    {
        err = WriteImpl(buffer, bufferSize, bytesWritten);
        mDataTotalSize += *bytesWritten;
        return err;
    }

    uint32 writtenBytes = 0;
    if(bufferSize >= mWriteStreamBuffer.mSize)
    {
        err = WriteImpl(mWriteStreamBuffer.mBuffer, mWriteStreamBuffer.mTail, &writtenBytes);
        (err != PAL_Ok) ? err : WriteImpl(buffer, bufferSize, &writtenBytes);
        if ( err != PAL_Ok)
        {
            return PAL_ErrFileFailed;
        }

        *bytesWritten = bufferSize + writtenBytes;
        mWriteStreamBuffer.mTail = 0;
    }
    else if(mWriteStreamBuffer.mTail + bufferSize > mWriteStreamBuffer.mSize)
    {
        err = WriteImpl(mWriteStreamBuffer.mBuffer, mWriteStreamBuffer.mTail, &writtenBytes);
        if ( err != PAL_Ok)
        {
            return PAL_ErrFileFailed;
        }
        nsl_memcpy(mWriteStreamBuffer.mBuffer, buffer, bufferSize);
        mWriteStreamBuffer.mTail = bufferSize;
        *bytesWritten = bufferSize;
    }
    else
    {
        nsl_memcpy(mWriteStreamBuffer.mBuffer + mWriteStreamBuffer.mTail, buffer, bufferSize);
        mWriteStreamBuffer.mTail += bufferSize;
        *bytesWritten = bufferSize;
    }

     mDataTotalSize += *bytesWritten;

    return PAL_Ok;
}

PAL_Error
NBRE_IOStream::Seek(PAL_FileSetOrigin type, int32 offset)
{
    if (mDataTotalSize < mReadBytesCounter || mReadStreamBuffer.mTail < mReadStreamBuffer.mHead)
    {
        return PAL_ErrBadParam;
    }

    uint32 offsetCopy = static_cast<uint32>(nsl_fabs(static_cast<double>(offset)));

    switch(type)
    {
    case PFSO_Current:
        if (offset >= 0)
        {
            if (offsetCopy >= mDataTotalSize - mReadBytesCounter)
            {
                if (SeekImpl(mDataTotalSize) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadStreamBuffer.mHead = mReadStreamBuffer.mTail = 0;
                mReadBytesCounter = mDataTotalSize;
            }
            else if (offsetCopy > mReadStreamBuffer.mTail - mReadStreamBuffer.mHead)
            {
                uint32 step = mReadBytesCounter + offsetCopy;
                if (SeekImpl(step) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadStreamBuffer.mHead = mReadStreamBuffer.mTail = 0;
                mReadBytesCounter = step;
            }
            else
            {
                mReadStreamBuffer.mHead += offsetCopy;
                mReadBytesCounter += offsetCopy;
            }
        }
        else
        {
            if (offsetCopy >= mReadBytesCounter)
            {
                if (SeekImpl(0) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadStreamBuffer.mHead = mReadStreamBuffer.mTail = 0;
                mReadBytesCounter = 0;
            }
            else if (offsetCopy > mReadStreamBuffer.mHead)
            {
                uint32 step = mReadBytesCounter - offsetCopy;
                if (SeekImpl(step) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadStreamBuffer.mHead = mReadStreamBuffer.mTail = 0;
                mReadBytesCounter = step;
            }
            else
            {
                mReadStreamBuffer.mHead -= offsetCopy;
                mReadBytesCounter -= offsetCopy;
            }
        }
        break;

    case PFSO_Start:
        if (offset >= 0)
        {
            if (offsetCopy >= mDataTotalSize)
            {
                if (SeekImpl(mDataTotalSize) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadBytesCounter = mDataTotalSize;
            }
            else
            {
                if (SeekImpl(offsetCopy) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadBytesCounter = offsetCopy;
            }
        }
        mReadStreamBuffer.mHead = mReadStreamBuffer.mTail = 0;
        break;

    case PFSO_End:
        if (offset < 0)
        {
            if (offsetCopy >= mDataTotalSize)
            {
                if (SeekImpl(0) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadBytesCounter = 0;
            }
            else
            {
                if (SeekImpl(mDataTotalSize - offsetCopy) != PAL_Ok)
                {
                    return PAL_ErrFileFailed;
                }
                mReadBytesCounter =  mDataTotalSize - offsetCopy;
            }
        }
        mReadStreamBuffer.mHead = mReadStreamBuffer.mTail = 0;
        break;

    default:
        return PAL_ErrUnsupported;
    }

    return PAL_Ok;
}

uint32
NBRE_IOStream::GetPos() const
{
    return mReadBytesCounter;
}

nb_boolean
NBRE_IOStream::Eof() const
{
    if(mDataTotalSize == mReadBytesCounter)
    {
        return TRUE;
    }
    return FALSE;
}

uint32
NBRE_IOStream::GetSize() const
{
    return mDataTotalSize;
}

PAL_Error
NBRE_IOStream::Flush()
{
    PAL_Error err = PAL_Ok;

    if (mWriteStreamBuffer.mTail > 0)
    {
        uint32 writtenBytes = 0;
        err = WriteImpl(mWriteStreamBuffer.mBuffer, mWriteStreamBuffer.mTail, &writtenBytes);
        mDataTotalSize += writtenBytes;
    }

    return err;
}
