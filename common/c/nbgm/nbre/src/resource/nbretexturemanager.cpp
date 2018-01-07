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
#include "nbretexturemanager.h"
#include "nbreitextureimage.h"
#include "nbrerenderpal.h"

#define  DEFAULT_TEXTURE_NAME  "default_tex"

class TextureCacheStratge:public NBRE_ICacheStrategy<NBRE_String, NBRE_TexturePtr>
{
private:
    virtual PAL_Error GetRemovedKeys (NBRE_Vector<NBRE_String>& removedKeys)const;
    virtual void OnPut(const NBRE_String& key, const NBRE_TexturePtr& val);
    virtual void OnGet (const NBRE_String& /*key*/){;}
    virtual void OnTouch (const NBRE_String& /*key*/){;}
    virtual nb_boolean OnRealse(const NBRE_String& key);
    virtual void OnRemoveAll();
    virtual void Destroy(){ NBRE_DELETE this; }
    virtual void OnLock (const NBRE_String& key);
    virtual void OnUnlock (const NBRE_String& key);
    virtual void OnRemove(const NBRE_String& key);
    virtual void OnDestroy(NBRE_TexturePtr& /*val*/){};
    virtual void OnUpdate(const NBRE_String& key, const NBRE_TexturePtr& val, NBRE_TexturePtr& oldVal);

private:
    struct TextureItem
    {
        TextureItem():mPtr(NULL), mLocked(FALSE){}
        TextureItem(const NBRE_TexturePtr* ptr, nb_boolean locked):mPtr(ptr),mLocked(locked){;}
        const NBRE_TexturePtr* mPtr;
        nb_boolean mLocked;
    };
    typedef NBRE_Map<NBRE_String, TextureItem> TextureItemMap;

private:
    TextureItemMap mTextureItems;
};

void
TextureCacheStratge::OnRemove(const NBRE_String& key)
{
    mTextureItems.erase(key);
}

void
TextureCacheStratge::OnRemoveAll()
{
    mTextureItems.clear();
}

void
TextureCacheStratge::OnLock (const NBRE_String& key)
{
    nbre_assert(!key.empty());
    TextureItemMap::iterator it = mTextureItems.find(key);
    if (it != mTextureItems.end())
    {
        it->second.mLocked = TRUE;
    }
}

void
TextureCacheStratge::OnUnlock (const NBRE_String& key)
{
    nbre_assert(!key.empty());
    TextureItemMap::iterator it = mTextureItems.find(key);
    if (it != mTextureItems.end())
    {
        it->second.mLocked = FALSE;
    }
}

nb_boolean
TextureCacheStratge::OnRealse (const NBRE_String& key)
{
    nbre_assert(!key.empty());
    TextureItemMap::const_iterator it = mTextureItems.find(key);
    if (it != mTextureItems.end())
    {
         nbre_assert(it->second.mPtr);
         if (!it->second.mLocked  && it->second.mPtr->unique())
         {
             return TRUE;
         }
         else
         {
             return FALSE;
         }
    }
    else
    {
        return TRUE;
    }
}

PAL_Error
TextureCacheStratge::GetRemovedKeys(NBRE_Vector<NBRE_String>& removedKeys)const
{
    removedKeys.clear();
    TextureItemMap::const_iterator it = mTextureItems.begin();
    for (; it != mTextureItems.end(); it++)
    {
        nbre_assert(it->second.mPtr);
        if (!it->second.mLocked && it->second.mPtr->unique())
        {
            removedKeys.push_back(it->first);
        }
    }
    return PAL_Ok;
}

void
TextureCacheStratge::OnPut(const NBRE_String& key, const NBRE_TexturePtr& val)
{
    nbre_assert(!key.empty());
    mTextureItems.insert(TextureItemMap::value_type(key, TextureItem(&val, FALSE)));
}

void
TextureCacheStratge::OnUpdate(const NBRE_String& key, const NBRE_TexturePtr& val, NBRE_TexturePtr& oldVal)
{
    nbre_assert(!key.empty());
    mTextureItems.erase(key);
    mTextureItems.insert(TextureItemMap::value_type(key, TextureItem(&val, FALSE)));
}

NBRE_TextureManager::NBRE_TextureManager(NBRE_IRenderPal* renderPal) : mRenderPal(renderPal), mTextureCacheStratege(NULL), mTextures(NBRE_NEW TextureCacheStratge())
{
}

NBRE_TextureManager::~NBRE_TextureManager()
{
    RemoveAllTextures();
}

void
NBRE_TextureManager::Initialize()
{
    CreateDefaultTexture();
}

NBRE_TexturePtr
NBRE_TextureManager::CreateRenderTexture(const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap)
{
    return CreateRenderTexture("", info, texType, isUsingMipmap);
}

NBRE_TexturePtr
NBRE_TextureManager::CreateRenderTexture(const NBRE_String &name, const NBRE_ImageInfo& info, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap)
{
    NBRE_TexturePtr pTexture;
    if (info.mHeight == 0 || info.mWidth == 0 || info.mFormat == NBRE_PF_NOTSUPPORT)
    {
        return pTexture;
    }

    NBRE_IRenderPal &renderPal = *(mRenderPal);
    if (name.empty())
    {
        NBRE_Texture *tex = renderPal.CreateTexture(info, texType, isUsingMipmap, name);
        if (tex)
        {
            pTexture.reset(tex);
        }
    }
    else
    {
        if (!mTextures.Contains(name))
        {
            NBRE_Texture *tex = renderPal.CreateTexture(info, texType, isUsingMipmap, name);
            if (tex)
            {
                pTexture.reset(tex);
                mTextures.Put(name, pTexture);
            }
        }
    }
    return pTexture;
}

NBRE_TexturePtr
NBRE_TextureManager::CreateTexture(const NBRE_String &name, NBRE_Image** image, uint8 faceCount, uint8 numMipmaps, nb_boolean isUsingMipmap, NBRE_Texture::TextureType texType)
{
    NBRE_TexturePtr pTexture;
    if (image == NULL || numMipmaps == 0)
    {
        return pTexture;
    }

    NBRE_IRenderPal &renderPal = *(mRenderPal);
    if (name.empty())
    {
        NBRE_Texture *tex = renderPal.CreateTexture(image, 1, numMipmaps, isUsingMipmap, texType, name);
        image = NULL;
        if (tex)
        {
            pTexture.reset(tex);
        }
    }
    else
    {
        if (!mTextures.Contains(name))
        {
            NBRE_Texture *tex = renderPal.CreateTexture(image, 1, numMipmaps, isUsingMipmap, texType, name);
            image = NULL;
            if (tex)
            {
                pTexture.reset(tex);
                mTextures.Put(name, pTexture);
            }
        }
    }
    if(image != NULL)
    {
        for(uint32 i = 0; i< static_cast<uint32>(faceCount * numMipmaps); ++i)
        {
            NBRE_DELETE image[i];
        }
    }
    return pTexture;
}

NBRE_TexturePtr
NBRE_TextureManager::GetTexture(const NBRE_String &name)
{
    NBRE_TexturePtr texture;
    if (!name.empty())
    {
        mTextures.Get(name, texture);
    }
    return texture;
}

PAL_Error
NBRE_TextureManager::LockTexture(const NBRE_String &name)
{
    PAL_Error err = PAL_ErrNotFound;
    if (!name.empty())
    {
        if (mTextures.Contains(name))
        {
            mTextures.Lock(name);
            err = PAL_Ok;
        }
    }
    return err;
}

PAL_Error
NBRE_TextureManager::UnlockTexture(const NBRE_String &name)
{
    PAL_Error err = PAL_ErrNotFound;
    if (!name.empty())
    {
        if (mTextures.Contains(name))
        {
            mTextures.Unlock(name);
            err = PAL_Ok;
        }
    }
    return err;
}

NBRE_TexturePtr
NBRE_TextureManager::CreateTexture(const NBRE_String &name, NBRE_ITextureImage* textureImage, NBRE_Texture::TextureType texType, nb_boolean isUsingMipmap)
{
    NBRE_TexturePtr pTexture;
    nbre_assert(textureImage != NULL);

    NBRE_IRenderPal &renderPal = *(mRenderPal);
    if (name.empty())
    {
        NBRE_Texture *tex = renderPal.CreateTexture(textureImage, texType, isUsingMipmap, name);
        textureImage = NULL;
        if (tex)
        {
            pTexture.reset(tex);
        }
    }
    else
    {
        if (!mTextures.Contains(name))
        {
            NBRE_Texture *tex = renderPal.CreateTexture(textureImage, texType, isUsingMipmap, name);
            textureImage = NULL;
            if (tex)
            {
                pTexture.reset(tex);
                mTextures.Put(name, pTexture);
            }
        }
    }
    if(textureImage!=NULL)
    {
        textureImage->Release();
    }
    return pTexture;
}

void
NBRE_TextureManager::RemoveAllTextures()
{
    mTextures.RemoveAll();
}

void
NBRE_TextureManager::Merge(NBRE_TextureManager* manager)
{
    if(manager != NULL)
    {
        NBRE_Vector<NBRE_String> ks;
        manager->mTextures.GetAllKeys(ks);
        for(NBRE_Vector<NBRE_String>::iterator it = ks.begin(); it != ks.end(); ++it)
        {
            NBRE_TexturePtr texturePtr;
            manager->mTextures.Get(*it, texturePtr);
            mTextures.Put(*it, texturePtr);
        }
        mTextures.Refresh();
    }
}

void
NBRE_TextureManager::CreateDefaultTexture()
{
    NBRE_IRenderPal &renderPal = *(mRenderPal);
    NBRE_Image **images = NBRE_NEW NBRE_Image*[1];

    NBRE_Image *image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8);
    uint8 *buffer = image->GetImageData();
    buffer[0] = 255;
    buffer[1] = 255;
    buffer[2] = 255;
    buffer[3] = 255;

    images[0] = image;
    NBRE_TexturePtr texture(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "NBRE_TextureManager::DefaultTexture"));
    mTextures.Put(DEFAULT_TEXTURE_NAME, texture);
    mTextures.Lock(DEFAULT_TEXTURE_NAME);
}


NBRE_TexturePtr
NBRE_TextureManager::GetDefaultTexture()
{
    NBRE_TexturePtr defaultTex;
    mTextures.Get(DEFAULT_TEXTURE_NAME, defaultTex);
    nbre_assert(defaultTex);
    return defaultTex;
}
