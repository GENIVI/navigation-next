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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*
 * nbgmandroidrendercontext.cpp
 *
 *  Created on: May 2, 2013
 *      Author: btian
 */
#include "nbgmandroidrendercontext.h"
#include "nbgmandroidegl.h"
#include "stdlib.h"
#include <android/log.h>
#include "palstdlib.h"

#define  LOG_TAG    "NBGM_AndroidRenderContext"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOG_LINE __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s, %d", __func__, __LINE__)

NBGM_AndroidRenderContext::NBGM_AndroidRenderContext(ANativeWindow* window, PAL_Instance *pal, const NBGM_RenderConfig &config)
    : mRenderSystemType(NBGM_RS_GLES20), mConfig(config)
{
    mEGL = new NBGMAndroidEGL(window, pal, config);
}

NBGM_AndroidRenderContext::~NBGM_AndroidRenderContext()
{
}

void NBGM_AndroidRenderContext::Initialize()
{
	bool ret = false;
   switch (mRenderSystemType)
    {
    case NBGM_RS_GLES:
        {
        	ret = mEGL->Initilize(EGL_OPENGL_ES_BIT);
        }
        break;
    case NBGM_RS_GLES20:
        {
        	ret = mEGL->Initilize(EGL_OPENGL_ES2_BIT);
        }
        break;
    default:
        break;
    }
   LOGI("mEGL->Initilize = %d", ret);
}

void NBGM_AndroidRenderContext::Finalize()
{
    mEGL->Finish();
    delete mEGL;
}

bool NBGM_AndroidRenderContext::BeginRender()
{
    return mEGL->MakeCurrent();
}

bool NBGM_AndroidRenderContext::EndRender()
{
    return mEGL->Swap();
}

void NBGM_AndroidRenderContext::Reset()
{
    //nothing for now
}

bool NBGM_AndroidRenderContext::SwitchTo()
{
    return mEGL->MakeCurrent();
}

const void* NBGM_AndroidRenderContext::GetConfiguration(const char* name)
{
    if(name ==  NULL)
    {
        return NULL;
    }
    if(nsl_strcmp(name, "RENDER_SYSTEM_TYPE") == 0)
    {
        return &mRenderSystemType;
    }
    if(nsl_strcmp(name, "ANTI-ALIASING") == 0)
    {
        return &(mConfig.enableFullScreenAntiAliasing);
    }
    if(nsl_strcmp(name, "ANISOTROPICFILTERING") == 0)
    {
        return &(mConfig.enableAnisotropicFiltering);
    }
    return NULL;
}

void NBGM_AndroidRenderContext::UpdateSurface(ANativeWindow* window)
{
    mEGL->UpdateSurface(window);
}

void NBGM_AndroidRenderContext::DestroySurface()
{
    mEGL->DestroySurface();
}
