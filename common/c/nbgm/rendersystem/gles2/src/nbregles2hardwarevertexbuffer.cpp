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
#include "nbregles2hardwarevertexbuffer.h"
#include "nbrelog.h"

NBRE_GLES2HardwareVertexBuffer::NBRE_GLES2HardwareVertexBuffer(uint32 vertexSize,
                                                               uint32 numVertices,
                                                               nb_boolean useShadowBuffer,
                                                               Usage usage)
    :NBRE_HardwareVertexBuffer(vertexSize, numVertices, useShadowBuffer, usage),
    mBufferData(NULL),
    mVBOBufferId(0)
{
}

NBRE_GLES2HardwareVertexBuffer::~NBRE_GLES2HardwareVertexBuffer()
{
    if(mBufferData!=NULL)
    {
        nsl_free(mBufferData);
    }
    if(mVBOBufferId != 0)
    {
        glDeleteBuffers(1, &mVBOBufferId);
        mVBOBufferId = 0;
    }
}

void
NBRE_GLES2HardwareVertexBuffer::Allocate()
{
    mBufferData = (uint8*) nsl_malloc(mSizeInBytes);
    if(mBufferData!=NULL)
    {
        nsl_memset(mBufferData, 0, mSizeInBytes);
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLES2HardwareVertexBuffer::Allocate, not enough memory! allocate size=%d",
                      mSizeInBytes);
    }
}

void
NBRE_GLES2HardwareVertexBuffer::WriteData( uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer)
{
    if(mBufferData!=NULL)
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
    else
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLES2HardwareVertexBuffer::WriteData, Allocate failed!",
                      mSizeInBytes);
    }
}

void
NBRE_GLES2HardwareVertexBuffer::ReadData( uint32 offset, uint32 length, void* pDest ) const
{
    if(mBufferData!=NULL)
    {
        nbre_assert((offset + length) <= mSizeInBytes);
        nsl_memcpy(pDest, mBufferData + offset, length);
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLES2HardwareVertexBuffer::ReadData, Allocate failed!",
                      mSizeInBytes);
    }
}

void
NBRE_GLES2HardwareVertexBuffer::Resize( uint32 length )
{
    if(mSizeInBytes < length)
    {
        if(mBufferData!=NULL)
        {
            mSizeInBytes = length;
            uint8* newBufferData = (uint8*)nsl_realloc(mBufferData, mSizeInBytes);
            if(newBufferData != NULL)
            {
                mBufferData = newBufferData;
            }
            else
            {
                nsl_free(mBufferData);
                mBufferData = NULL;
            }
        }
        else
        {
            Allocate();
        }

        if(mVBOBufferId != 0)
        {
            glDeleteBuffers(1, &mVBOBufferId);
            mVBOBufferId = 0;
        }
    }
}

void*
NBRE_GLES2HardwareVertexBuffer::Lock( uint32 offset, LockOptions lockOptions )
{
    if(mBufferData!=NULL)
    {
        nbre_assert(offset <= mSizeInBytes);
        return Lock(offset, mSizeInBytes - offset, lockOptions);
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLES2HardwareVertexBuffer::Lock, Allocate failed!",
                      mSizeInBytes);
    }
    return NULL;
}

void*
NBRE_GLES2HardwareVertexBuffer::Lock( uint32 offset, uint32 length, LockOptions lockOption )
{
    nbre_assert((offset + length) <= mSizeInBytes);
    if(mBufferData!=NULL)
    {
        switch(lockOption)
        {
            case HBL_WRITE_ONLY:
                mShadowUpdate = TRUE;
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
    else
    {
        NBRE_DebugLog(PAL_LogSeverityMajor,
                      "NBRE_GLES2HardwareVertexBuffer::Lock, Allocate failed!",
                      mSizeInBytes);
    }
    return NULL;
}

void
NBRE_GLES2HardwareVertexBuffer::UnLock()const
{
}

GLuint
NBRE_GLES2HardwareVertexBuffer::GetBufferId()
{

    if(mVBOBufferId == 0)
    {
        nbre_assert(mBufferData!=NULL);
        glGenBuffers(1, &mVBOBufferId);
        if(mVBOBufferId != 0)
        {
            nbre_assert(mBufferData!=NULL);
            glBindBuffer(GL_ARRAY_BUFFER, mVBOBufferId);
            GLenum usage = GL_STATIC_DRAW;
            if(mUsage == NBRE_HardwareBuffer::HBU_DYNAMIC)
           {
               usage = GL_DYNAMIC_DRAW;
           }
           else if(mUsage == NBRE_HardwareBuffer::HBU_STREAM)
           {
               usage = GL_STREAM_DRAW;
           }
           glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, mBufferData, usage);
           mShadowUpdate = FALSE;
		   
		   if(mDiscardable)
           {
               NBRE_DELETE_ARRAY mBufferData;
               mBufferData = NULL;
               mDiscardable = FALSE;
           }
       }
       else
       {
           NBRE_DebugLog(PAL_LogSeverityInfo,
                         "NBRE_GLES2HardwareVertexBuffer::GetBufferId: Create vertex buffer object failed! glError = %d",
                         glGetError());
       }
    }
    else
    {
        if(mShadowUpdate)
        {
            nbre_assert(mBufferData!=NULL);
            glBindBuffer(GL_ARRAY_BUFFER, mVBOBufferId);
            glBufferSubData(GL_ARRAY_BUFFER, mLockStart, mLockSize, mBufferData);
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
NBRE_GLES2HardwareVertexBuffer::UpdateFromShadow()
{

}

uint8*
NBRE_GLES2HardwareVertexBuffer::GetBuffer()
{
	return mBufferData;
}
