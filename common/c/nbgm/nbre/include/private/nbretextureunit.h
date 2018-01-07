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

    @file nbretextureunit.h
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
#ifndef _NBRE_TEXTURE_UNIT_H_
#define _NBRE_TEXTURE_UNIT_H_
#include "nbretexturestate.h"
#include "nbrematrix4x4.h"
#include "nbretexture.h"
#include "nbretexturemanager.h"
#include "nbrecontext.h"

/** \addtogroup NBRE_Resource
*  @{
*/
/** \addtogroup NBRE_Shader
*  @{
*/

class NBRE_TextureUnit
{
public:
    NBRE_TextureUnit():mIsUsingMatrix(FALSE), mTextureCoordSetIndex(0){}
    ~NBRE_TextureUnit(){}

public:
    NBRE_TextureState GetState() const { return mState; }
    void SetState(const NBRE_TextureState& val) { mState = val; }
    const NBRE_Matrix4x4f& GetTransform() const { return mTransform; }
    void SetTransform(const NBRE_Matrix4x4f& val) { mTransform = val; mIsUsingMatrix = TRUE; }
    NBRE_TexturePtr GetTexture(NBRE_Context& context) const;
    NBRE_TexturePtr GetTexture() const { return mTexture; }
    void SetTexture(NBRE_TexturePtr val)
    {
    	nbre_assert(val.get());
    	mTexture = val;
    }
    nb_boolean  IsUsingMatrix() const { return mIsUsingMatrix; }

    /** Gets the index of the set of texture co-ords this layer uses.
    @note
    Applies to both fixed-function and programmable pipeline.
    */
    uint32 TextureCoordSet() const { return mTextureCoordSetIndex; }

    /** Sets the index of the set of texture co-ords this layer uses.
    @note
    Default is 0 for all layers. Only change this if you have provided multiple texture co-ords per
    vertex.
    @note
    Applies to both fixed-function and programmable pipeline.
    */
    void SetTextureCoordSet(uint32 set) { mTextureCoordSetIndex = set; }

private:
    NBRE_TextureState mState;    
    NBRE_Matrix4x4f mTransform;    
    NBRE_TexturePtr mTexture;
    nb_boolean mIsUsingMatrix;
    uint32 mTextureCoordSetIndex;
};

inline NBRE_TexturePtr
NBRE_TextureUnit::GetTexture(NBRE_Context& context) const
{
    if (mTexture.get() == NULL)
    {
        return context.mDefaultTexture;
    }
    return mTexture;
}

typedef NBRE_Vector<NBRE_TextureUnit> NBRE_TextureUnitList;

/** @} */
/** @} */
#endif
