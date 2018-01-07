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

    @file nbregles2hardwareindexbuffer.h
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
#ifndef _NBRE_GLES2_HARDWARE_INDEXBUFFER_H_
#define _NBRE_GLES2_HARDWARE_INDEXBUFFER_H_

#include "nbrehardwareindexbuffer.h"
#include "nbregles2common.h"

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/*! \addtogroup NBRE_GLES2RenderPal
*  @{
*/
/** Specialization of NBRE_HardwareIndexBuffer for OpenGLES2 */
class NBRE_GLES2HardwareIndexBuffer: public NBRE_HardwareIndexBuffer
{
public:
    NBRE_GLES2HardwareIndexBuffer(IndexType idxType, uint32 numIndexes, nb_boolean useShadowBuffer, Usage usage);
    ~NBRE_GLES2HardwareIndexBuffer();

public:
    // From NBRE_HardwareBuffer
    virtual void Resize(uint32 length);
    virtual void ReadData( uint32 offset, uint32 length, void* pDest ) const;
    virtual void WriteData( uint32 offset, uint32 length, const void* pSource, nb_boolean discardWholeBuffer);

    virtual void* Lock(uint32 offset, LockOptions lockOptions);
    virtual void* Lock(uint32 offset, uint32 length, LockOptions lockOptions);
    virtual void UnLock()const;

    /// These functions must be called in render thread.
    virtual void UpdateFromShadow();
    GLuint GetBufferId();
    uint8* GetBuffer();
    nb_boolean IsModified()const { return mShadowUpdate; }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_GLES2HardwareIndexBuffer);

private:
    uint8* mBufferData;
    mutable GLuint mVBOBufferId;
    nb_boolean mSizeChanged;
};

/*! @} */
/*! @} */
#endif