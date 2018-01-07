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
#include "nbretexture.h"
#include "nbreitextureimage.h"
#include "nbrepngtextureimage.h"
#include "nbrelog.h"

NBRE_Texture::NBRE_Texture(NBRE_IRenderPal *renderPal,NBRE_ITextureImage *image, TextureType type, const NBRE_ImageInfo *info, nb_boolean isUsingMipmap, const NBRE_String& name):
    mRenderPal(renderPal),
    mTextureImage(image),
    mType(type),
    mUsage(0),
    mLoadingState(TS_UNLOADED),
    mUsingMipmap(isUsingMipmap),
    mName(name),
    mTextureListener(NULL)
{
    if (info != NULL)
    {
        mTextureInfo = *info;
        mOriginalInfo = mTextureInfo;
    }
    else
    {
        nbre_assert(image);
        image->ReadInfo(mTextureInfo, mOriginalInfo);
    }
}

NBRE_Texture::~NBRE_Texture()
{
    if (mTextureImage != NULL)
    {
        mTextureImage->Release();
    }
    if (mTextureListener)
    {
        mTextureListener->OnDestroyed(this);
    }
}

void
NBRE_Texture::Unload(void)
{
    if (mLoadingState == TS_UNLOADED)
    {
        return;
    }
    FreeInternalTextureImpl();
    mLoadingState = TS_UNLOADED;
}

PAL_Error
NBRE_Texture::Load()
{
    if (mLoadingState == TS_LOADED)
    {
        return PAL_ErrAlreadyExist;
    }
    if(mTextureImage == NULL)
    {
        mTextureImage = NBRE_NEW NBRE_PngTextureImage(mTextureInfo);
    }
    PAL_Error err = GenerateInternalTextureImpl();
    if (err == PAL_Ok)
    {
        mLoadingState = TS_LOADED;
        if (mTextureImage != NULL)
        {
            mTextureImage->Release();
            mTextureImage = NULL;
        }
    }
    return err;
}

void
NBRE_Texture::SetEventListener(NBRE_ITextureEventListener* listener)
{
    if (mTextureListener != listener)
    {
        mTextureListener = listener;
    }
}