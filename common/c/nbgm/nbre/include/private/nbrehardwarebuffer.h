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

    @file nbrehardwarebuffer.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_HARDWAREBUFFER_H_
#define _NBRE_HARDWAREBUFFER_H_
#include "nbretypes.h"
#include "nbrecommon.h"

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/** Abstract class defining common features of hardware buffers.
    @remarks
    A 'hardware buffer' is any area of memory held outside of core system ram,
    and in our case refers mostly to video ram, although in theory this class
    could be used with other memory areas such as sound card memory, custom
    coprocessor memory etc.
*/
class NBRE_HardwareBuffer
{
public:
    enum Usage
    {
        /** Static buffer which the application rarely modifies once created. Modifying 
        the contents of this buffer will involve a performance hit.
        */
        HBU_STATIC = 1,
        /** Indicates the application would like to modify this buffer with the CPU
        fairly often. 
        Buffers created with this flag will typically end up in AGP memory rather
        than video memory.
        */
        HBU_DYNAMIC = 2,
        /** Indicates the application would like to modify this buffer with the CPU
        fairly often and typically used for animation. 
        Buffers created with this flag will typically end up in AGP memory rather
        than video memory.
        */
        HBU_STREAM = 3,
    };

    /// Locking options
    enum LockOptions
    {
        HBL_READ_ONLY,
        HBL_WRITE_ONLY,
        HBL_WRITE_DISCARD
    };

public:
    NBRE_HardwareBuffer(nb_boolean useShadowBuffer, Usage usage)
        :mSizeInBytes(0), mUseShadowBuffer(useShadowBuffer), mUsage(usage), mShadowUpdate(FALSE), mDiscardable(FALSE), mLockStart(0), mLockSize(0){}
    virtual ~NBRE_HardwareBuffer(){}

public:
    uint32 GetSizeInBytes() const { return mSizeInBytes; }

    virtual void Resize(uint32 length) = 0;
    /*! Write data to this buffer
        @param offset The byte offset from the start of the buffer to lock
        @param length The size of the area to write, in bytes
        @param pSource Data source
        @return void 
    */
    virtual void WriteData(uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer) = 0;
    virtual void ReadData(uint32 offset, uint32 length, void* pDest) const = 0;
    virtual void* Lock(uint32 offset, LockOptions lockOptions) = 0;
    virtual void* Lock(uint32 offset, uint32 length, LockOptions lockOptions) = 0;
    virtual void UnLock() const = 0;
    virtual void UpdateFromShadow() = 0;

    Usage GetUsage() const { return mUsage; }
    nb_boolean UseShadowBuffer() const { return mUseShadowBuffer; }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_HardwareBuffer);

protected:
    uint32 mSizeInBytes;
    nb_boolean mUseShadowBuffer;
    Usage mUsage;

    nb_boolean mShadowUpdate;
    nb_boolean mDiscardable;
    uint32 mLockStart;
    uint32 mLockSize;
};

/** @} */
#endif