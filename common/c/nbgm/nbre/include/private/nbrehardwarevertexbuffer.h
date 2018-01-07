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

    @file nbrehardwarevertexbuffer.h
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
#ifndef _NBRE_HARDWARE_VERTEXBUFFER_H_
#define _NBRE_HARDWARE_VERTEXBUFFER_H_

#include "nbrecommon.h"
#include "nbrehardwarebuffer.h"

/*! \addtogroup NBRE_RenderPal
*  @{
*/
/** Specialization of HardwareBuffer for a vertex buffer. still abstract.*/
class NBRE_HardwareVertexBuffer: public NBRE_HardwareBuffer
{
public:
    /*!Create a Hardware vertex buffer
        @param vertexSize The size in bytes of a single vertex in this buffer. 
        If a vertex is made up by three point3f, then vertexSize = sizeof(float)*3*3
        @param numVertices The number of vertices in this buffer
        @return  
    */
    NBRE_HardwareVertexBuffer(uint32 vertexSize, uint32 numVertices, nb_boolean useShadowBuffer, Usage usage)
                              :NBRE_HardwareBuffer(useShadowBuffer, usage), mNumVertices(numVertices), mVertexSize(vertexSize)
    {
        mSizeInBytes = mVertexSize * numVertices;
    }
    ~NBRE_HardwareVertexBuffer(){}

public:
    /// Gets the size in bytes of a single vertex in this buffer
    uint32 GetVertexSize(void) const { return mVertexSize; }
    /// Get the number of vertices in this buffer
    uint32 GetNumVertices(void) const { return mNumVertices; }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_HardwareVertexBuffer);

protected:
    uint32  mNumVertices;
    uint32  mVertexSize;
};

/*! @} */
#endif