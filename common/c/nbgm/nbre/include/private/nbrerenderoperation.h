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

    @file nbrerenderoperation.h
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
#ifndef _NBRE_RENDER_OPERATION_H_
#define _NBRE_RENDER_OPERATION_H_

#include "nbrerenderpal.h"
#include "nbrematrix4x4.h"
#include "nbrecontext.h"
#include "nbretextureunit.h"

class NBRE_IndexData;
class NBRE_VertexData;
class NBRE_Pass;

/*! \addtogroup NBRE_Scene
*  @{
*/

struct NBRE_LastRenderState
{
    const NBRE_Pass* mLastPass;
    const NBRE_VertexData* mLastVertexData;
    const NBRE_Matrix4x4d* mLastTransform;
    NBRE_Color mLastColor;
    NBRE_BlendFunction mLastSrcBlend;
    NBRE_BlendFunction mLastDstBlend;
    const NBRE_TextureUnitList* mLastTextureUnits;
    float mLastLineWidth;
};

class NBRE_RenderOperation
{
public:
    NBRE_RenderOperation(NBRE_Context& context);
    ~NBRE_RenderOperation();

public:
    void Render();
    uint32 GetTriangleNum() const;

    void SetPass(const NBRE_Pass* pass) {mPass = pass;}
    void SetTransform(const NBRE_Matrix4x4d* transform) { mTransform = transform;}
    void SetOperationType(NBRE_PrimitiveType type);
    void SetIndexData(NBRE_IndexData* indexData);
    void SetVertexData(NBRE_VertexData* vertexData) {mVertexData = vertexData;}
    void SetGpuProgramParam(const NBRE_String& name, const NBRE_GpuProgramParam& param);

    static void ResetLastRenderState(NBRE_Context& context);
private:
    void ApplyPass();
    void CalculTriangleNum();

    nb_boolean Verify();
private:
    DISABLE_COPY_AND_ASSIGN(NBRE_RenderOperation);

private:
    NBRE_Context& mContext;
    /// The type of operation to perform
    NBRE_PrimitiveType mOperationType;

    /// Vertex source data
    NBRE_VertexData* mVertexData;

    /** Index data. If null, the vertices are simply read in sequence to define the primitives.
    If not null, indices are used instead to identify vertices anywhere in the buffer,
    and allowing vertices to be used more than once.*/
    NBRE_IndexData* mIndexData;

    const NBRE_Matrix4x4d* mTransform;

    const NBRE_Pass* mPass;

    uint32 mTriangleNum;
    NBRE_Map<NBRE_String, NBRE_GpuProgramParam> mGpuParams;
};

/*! @} */

#endif
