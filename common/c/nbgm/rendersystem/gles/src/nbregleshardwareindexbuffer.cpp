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
#include "nbregleshardwareindexbuffer.h"
#include "nbrecommon.h"
#include "nbreglescommon.h"

NBRE_GLESHardwareIndexBuffer::NBRE_GLESHardwareIndexBuffer(IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, Usage usage):
    NBRE_HardwareIndexBuffer(idxType, numIndexes, useShadowBuffer, usage),
    mBufferData(NULL),
    mVBOBufferId(0),
    mSizeChanged(FALSE)
{
    mBufferData = NBRE_NEW uint8[mSizeInBytes];
    nsl_memset(mBufferData, 0, mSizeInBytes);
}

NBRE_GLESHardwareIndexBuffer::~NBRE_GLESHardwareIndexBuffer()
{
    NBRE_DELETE_ARRAY mBufferData;
    if (mVBOBufferId != 0)
    {
        glDeleteBuffers(1, &mVBOBufferId);
    }
}

void
NBRE_GLESHardwareIndexBuffer::ReadData( uint32 offset, uint32 length, void* pDest ) const
{
    nbre_assert((offset + length) <= mSizeInBytes);
    nsl_memcpy(pDest, mBufferData + offset, length);
}

void
NBRE_GLESHardwareIndexBuffer::WriteData( uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer)
{
    nbre_assert((offset + length) <= mSizeInBytes);

    LockOptions lockOptions = HBL_WRITE_ONLY;

    if (discardWholeBuffer)
    {
        lockOptions = HBL_WRITE_DISCARD;
    }

    void* bufferAddr = Lock(offset, length, lockOptions);

    nsl_memcpy(bufferAddr, pSource, length);

    UnLock();
}

void
NBRE_GLESHardwareIndexBuffer::Resize( uint32 length )
{
    if(mSizeInBytes < length)
    {
        mSizeInBytes = length;
        NBRE_DELETE_ARRAY mBufferData;
        mBufferData = NBRE_NEW uint8[mSizeInBytes];
        if (mUseShadowBuffer)
        {
            mSizeChanged = TRUE;
            mShadowUpdate = TRUE;
        }
    }
}

void*
NBRE_GLESHardwareIndexBuffer::Lock( uint32 offset, LockOptions lockOptions )
{
    nbre_assert(offset <= mSizeInBytes);
    return Lock(offset, mSizeInBytes - offset, lockOptions);
}

void*
NBRE_GLESHardwareIndexBuffer::Lock( uint32 offset, uint32 length, LockOptions lockOption )
{
    nbre_assert((offset + length) <= mSizeInBytes);

    switch(lockOption)
    {
    case HBL_WRITE_ONLY:
        if (mUseShadowBuffer)
        {
            mShadowUpdate = TRUE;
        }
        break;

    case HBL_READ_ONLY:
        break;

    case HBL_WRITE_DISCARD:
        if (mUseShadowBuffer)
        {
            mShadowUpdate = TRUE;
            mDiscardable = TRUE;
        }
        break;

    default:
        return NULL;
    }

    mLockStart = offset;
    mLockSize = length;

    return mBufferData + offset;
}

void
NBRE_GLESHardwareIndexBuffer::UnLock() const
{
}

GLuint
NBRE_GLESHardwareIndexBuffer::GenerateVBOBuffer(GLenum usage)
{
    if (mSizeChanged && mVBOBufferId != 0)
    {
        glDeleteBuffers(1, &mVBOBufferId);
        mVBOBufferId = 0;
        mSizeChanged = FALSE;
    }

    if (mVBOBufferId == 0)
    {
        glGenBuffers(1, &mVBOBufferId);
        GL_CHECK_ERROR;
        if(mVBOBufferId != 0)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOBufferId);
            GL_CHECK_ERROR;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, mBufferData, usage);
            GL_CHECK_ERROR;
            mShadowUpdate = FALSE;
        
            if (mDiscardable)
            {
                NBRE_DELETE_ARRAY mBufferData;
                mBufferData = NULL;
                mDiscardable = FALSE;
            }
        }
    }

    return mVBOBufferId;
}

void
NBRE_GLESHardwareIndexBuffer::UpdateFromShadow()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOBufferId);

    if (mShadowUpdate)
    {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, mLockStart, mLockSize, mBufferData);
        GL_CHECK_ERROR;
        mShadowUpdate = FALSE;

        if (mDiscardable)
        {
            NBRE_DELETE_ARRAY mBufferData;
            mBufferData = NULL;
            mDiscardable = FALSE;
        }
    }
}
