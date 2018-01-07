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
#include "nbregles2gpuprogramfactory.h"
#include "nbregles2gpuprogram.h"
#include "nbregles2vertexdeclaration.h"
#include "nbregles2shader.h"
#include "nbrememorystream.h"
#include "nbrefilestream.h"

class ShaderCacheStratge:public NBRE_ICacheStrategy<NBRE_String, NBRE_GLES2GpuProgramPtr>
{
private:
    virtual PAL_Error GetRemovedKeys (NBRE_Vector<NBRE_String>& removedKeys)const;
    virtual void OnPut(const NBRE_String& key, const NBRE_GLES2GpuProgramPtr& val);
    virtual void OnGet (const NBRE_String& /*key*/){;}
    virtual void OnTouch (const NBRE_String& /*key*/){;}
    virtual nb_boolean OnRealse(const NBRE_String& key);
    virtual void OnRemoveAll();
    virtual void Destroy(){ NBRE_DELETE this; }
    virtual void OnLock (const NBRE_String& key);
    virtual void OnUnlock (const NBRE_String& key);
    virtual void OnRemove(const NBRE_String& key);
    virtual void OnDestroy(NBRE_GLES2GpuProgramPtr& /*val*/){};
    virtual void OnUpdate(const NBRE_String& key, const NBRE_GLES2GpuProgramPtr& val, NBRE_GLES2GpuProgramPtr& oldVal);

private:
    struct ShaderItem
    {
        ShaderItem():mPtr(NULL), mLocked(FALSE){}
        ShaderItem(const NBRE_GLES2GpuProgramPtr* ptr, nb_boolean locked):mPtr(ptr),mLocked(locked){;}
        const NBRE_GLES2GpuProgramPtr* mPtr;
        nb_boolean mLocked;
    };
    typedef NBRE_Map<NBRE_String, ShaderItem> ShaderItemMap;

private:
    ShaderItemMap mShaderItems;
};

void
ShaderCacheStratge::OnRemove(const NBRE_String& key)
{
    mShaderItems.erase(key);
}

void
ShaderCacheStratge::OnRemoveAll()
{
    mShaderItems.clear();
}


void
ShaderCacheStratge::OnLock (const NBRE_String& key)
{
    nbre_assert(!key.empty());
    ShaderItemMap::iterator it = mShaderItems.find(key);
    if (it != mShaderItems.end())
    {
        it->second.mLocked = TRUE;
    }
}

void
ShaderCacheStratge::OnUnlock (const NBRE_String& key)
{
    nbre_assert(!key.empty());
    ShaderItemMap::iterator it = mShaderItems.find(key);
    if (it != mShaderItems.end())
    {
        it->second.mLocked = FALSE;
    }
}

nb_boolean
ShaderCacheStratge::OnRealse (const NBRE_String& key)
{
    nbre_assert(!key.empty());
    ShaderItemMap::const_iterator it = mShaderItems.find(key);
    if (it != mShaderItems.end())
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
ShaderCacheStratge::GetRemovedKeys(NBRE_Vector<NBRE_String>& removedKeys)const
{
    removedKeys.clear();
    ShaderItemMap::const_iterator it = mShaderItems.begin();
    for (; it != mShaderItems.end(); it++)
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
ShaderCacheStratge::OnPut(const NBRE_String& key, const NBRE_GLES2GpuProgramPtr& val)
{
    nbre_assert(!key.empty());
    mShaderItems.insert(ShaderItemMap::value_type(key, ShaderItem(&val, FALSE)));
}

void
ShaderCacheStratge::OnUpdate(const NBRE_String& key, const NBRE_GLES2GpuProgramPtr& val, NBRE_GLES2GpuProgramPtr& /*oldVal*/)
{
    nbre_assert(!key.empty());
    mShaderItems.erase(key);
    mShaderItems.insert(ShaderItemMap::value_type(key, ShaderItem(&val, FALSE)));
}

NBRE_GLES2GPUPorgramFactory::NBRE_GLES2GPUPorgramFactory(PAL_Instance* pal):
    mPalInstance(pal),
    mShaders(NBRE_NEW ShaderCacheStratge())
{
}

NBRE_GLES2GPUPorgramFactory::~NBRE_GLES2GPUPorgramFactory()
{
    RemoveAll();
}

void
NBRE_GLES2GPUPorgramFactory::Initialize()
{
     CreateAllDefaultPrograms();
}

NBRE_GLES2GpuProgramPtr
NBRE_GLES2GPUPorgramFactory::GetByName(const NBRE_String &name)
{
    NBRE_GLES2GpuProgramPtr texture;
    if (!name.empty())
    {
        mShaders.Get(name, texture);
    }
    return texture;
}

PAL_Error
NBRE_GLES2GPUPorgramFactory::Lock(const NBRE_String &name)
{
    PAL_Error err = PAL_Failed;
    if (!name.empty())
    {
        if (mShaders.Contains(name))
        {
            mShaders.Lock(name);
            err = PAL_Ok;
        }
    }
    return err;
}

PAL_Error
NBRE_GLES2GPUPorgramFactory::Unlock(const NBRE_String &name)
{
    PAL_Error err = PAL_Failed;
    if (!name.empty())
    {
        if (mShaders.Contains(name))
        {
            mShaders.Unlock(name);
            err = PAL_Ok;
        }
    }
    return err;
}

void
NBRE_GLES2GPUPorgramFactory::RemoveAll()
{
    mShaders.RemoveAll();
}

void
NBRE_GLES2GPUPorgramFactory::Refresh()
{
   mShaders.Refresh();
}

void
NBRE_GLES2GPUPorgramFactory::CreateAllDefaultPrograms()
{
    NBRE_String key = DEFAULT_SHADER;
    if (!mShaders.Contains(key))
    {
        NBRE_MemoryStream defaultVertexShaderSrc((const uint8*)DefualtVertexShader, sizeof(DefualtVertexShader), FALSE);
        NBRE_MemoryStream defaultPixelShaderSrc((const uint8*)DefualtPixelShader, sizeof(DefualtPixelShader), FALSE);
        NBRE_GLES2GPUPorgram *program = NBRE_NEW NBRE_GLES2GPUPorgram(defaultVertexShaderSrc, defaultPixelShaderSrc);
        if(program->Load() == PAL_Ok)
        {
            NBRE_GLES2GpuProgramPtr pProgram(program);
            mShaders.Put(key, pProgram);
			mShaders.Lock(key);
        }
    }

    CreateProgram("ao", AOVertexShader, AOPixelShader);
    CreateProgram("glow", glowVertexShader, glowPixelShader);
    CreateProgram("light3d", light3dVertexShader, light3dPixelShader);
}

NBRE_GLES2GpuProgramPtr
NBRE_GLES2GPUPorgramFactory::GetDefaultProgram()
{
    return GetByName(DEFAULT_SHADER);
}

static NBRE_String
GetKeyName(const NBRE_String & vs, const NBRE_String &ps)
{
    return vs+ps;
}

NBRE_GLES2GpuProgramPtr
NBRE_GLES2GPUPorgramFactory::CreateProgramFromFiles(const NBRE_String& vs, const NBRE_String& ps)
{
    nbre_assert(!vs.empty() && !ps.empty());
    NBRE_String key = GetKeyName(vs, ps);
    NBRE_GLES2GpuProgramPtr pProgram;
    if (!mShaders.Contains(key))
    {
        NBRE_FileStream vsSrc(mPalInstance, vs.c_str(), 1024);
        NBRE_FileStream psSrc(mPalInstance, ps.c_str(), 1024);
        NBRE_GLES2GPUPorgram *program = NBRE_NEW NBRE_GLES2GPUPorgram(vsSrc, psSrc);
        if(program->Load())
        {
            pProgram.reset(program);
            mShaders.Put(key, pProgram);
        }
    }
    return pProgram;
}

NBRE_GLES2GpuProgramPtr
NBRE_GLES2GPUPorgramFactory::CreateProgram(const NBRE_String& name, const NBRE_String& vs, const NBRE_String& ps)
{
    nbre_assert(vs.size() && ps.size());
    NBRE_MemoryStream defaultVertexShaderSrc((const uint8*)vs.c_str(), vs.length() + 1, FALSE);
    NBRE_MemoryStream defaultPixelShaderSrc((const uint8*)ps.c_str(), ps.length() + 1, FALSE);
    NBRE_GLES2GPUPorgram *program = NBRE_NEW NBRE_GLES2GPUPorgram(defaultVertexShaderSrc, defaultPixelShaderSrc);
    if(program->Load() == PAL_Ok)
    {
        NBRE_GLES2GpuProgramPtr pProgram(program);
        mShaders.Put(name, pProgram);
        mShaders.Lock(name);
        return pProgram;
    }
    else
    {
        return NBRE_GLES2GpuProgramPtr();
    }
}
