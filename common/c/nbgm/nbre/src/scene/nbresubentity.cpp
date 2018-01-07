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
#include "nbresubentity.h"
#include "nbresurfacemanager.h"
#include "nbrevertexanimation.h"

NBRE_SubEntity::NBRE_SubEntity(NBRE_Context& context, NBRE_SubModel* subMesh): 
                mContext(context),
                mSubModel(subMesh),
                mVisible(TRUE),
                mShaderLastUpdated(0),
                mTextureAnimation(NULL),
                mTextureAnimationLastUpdated(0)
{
}

NBRE_SubEntity::~NBRE_SubEntity()
{
}

const NBRE_ShaderPtr&
NBRE_SubEntity::GetShader() const
{
    if(mShaderLastUpdated != mContext.mShaderManager->LatestUpdated())
    {
        mShaderLastUpdated = mContext.mShaderManager->LatestUpdated();
        if(mShaderName.empty())
        {
            if(!mSubModel->ShaderName().empty())
            {
                mShader = mContext.mShaderManager->GetShader(mSubModel->ShaderName());
            }
        }
        else
        {
            mShader = mContext.mShaderManager->GetShader(mShaderName);
        }
    }

    if(mShader)
    {
        return mShader;
    }
    else
    {
        return mSubModel->GetShader();
    }
}

void
NBRE_SubEntity::SetTextureAnimation(NBRE_ITextureAnimation* textureAnimation)
{
    mTextureAnimation = textureAnimation;
}

const NBRE_ITextureAnimation*
NBRE_SubEntity::TextureAnimation() const
{
    return mTextureAnimation;
}

uint32
NBRE_SubEntity::GetTextureAnimationLastUpdated() const
{
    return mTextureAnimationLastUpdated;
}

void NBRE_SubEntity::SetTextureAnimationLastUpdated(uint32 number)
{
    mTextureAnimationLastUpdated = number;
}
