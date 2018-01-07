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
#include "nbgm.h"
#include "nbgmprotected.h"
#include "nbrelog.h"
#include "palstdlib.h"
#include "nbrerenderengine.h"
#include "nbrememorystream.h"
#include "nbgmmapviewprotected.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmcontext.h"

#ifdef SUPPORT_D3D9
#include "nbred3d9renderpal.h"
#endif

#ifdef SUPPORT_D3D11
#include "nbred3d11renderpal.h"
#endif

#ifdef SUPPORT_GL
#include "nbreglrenderpal.h"
#endif

#ifdef SUPPORT_GLES20
#include "nbregles2renderpal.h"
#endif

#ifdef SUPPORT_GLES
#include "nbreglesrenderpal.h"
#endif



#include "nbgmmapviewprotected.h"
#include "nbgmnavviewprotected.h"
#include "nbgmconst.h"
#include "nbgmrendercontext.h"
#include "nbrefilestream.h"
#include "nbgmresourcemanager.h"
#include "nbgmmapcamerahelperimpl.h"

struct NBGM_Instance
{
    NBGM_Config                     config;
    NBRE_Log*                       log;
    NBRE_FileStream*                logFile;
    NBRE_String                     workspace;
};

static NBGM_Instance gNbgmInstance;
static nb_boolean gInitialized = FALSE;

NBGM_DEF PAL_Error
NBGM_Initialize(const NBGM_Config* config)
{
    PAL_Error err = PAL_Ok;
    if(gInitialized)
    {
        return PAL_Ok;
    }

    if(config == NULL)
    {
        goto Error;
    }

    nsl_memset(&gNbgmInstance.config, 0, sizeof(NBGM_Config));

    //1. pal
    gNbgmInstance.config = *config;
    gNbgmInstance.workspace = config->workSpace;
    gNbgmInstance.config.workSpace = gNbgmInstance.workspace.c_str();

    nbre_assert(config->dpi >= 1.0f && config->dpi <= 1000000.0f);

    NBRE_DebugLog(PAL_LogSeverityInfo, "NBGM_Create succeeded");
    gInitialized = TRUE;
    return err;

Error:
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_Create failed, err = 0X%08x", err);

    return err;
}

PAL_Instance*
NBGM_GetPal()
{
    if(gInitialized)
    {
        return gNbgmInstance.config.pal;
    }
    return NULL;
}

PAL_Error
NBGM_CreateInternalResourceManager(const NBGM_MapViewConfig* config, PAL_Instance* pal, NBGM_ResourceManager** resourceManager)
{
    PAL_Error err = PAL_Ok;
    if(config == NULL || pal == NULL || resourceManager == NULL)
    {
        return PAL_ErrBadParam;
    }

    NBGM_ResourceManagerConfig resourceConfig;
    resourceConfig.renderContext = config->renderContext;

    err = NBGM_CreateResourceManager(&resourceConfig, resourceManager);

    return err;
}

PAL_Error
NBGM_CreateRenderPal(const NBGM_ResourceManagerConfig* config, PAL_Instance* pal, NBRE_IRenderPal** renderer)
{
    PAL_Error err = PAL_Ok;
    NBRE_IRenderPal* result = NULL;
    if(config == NULL || pal == NULL || renderer == NULL)
    {
        return PAL_ErrBadParam;
    }
    const NBGM_RenderSystemType* rendersystemType = static_cast<const NBGM_RenderSystemType* >(config->renderContext->GetConfiguration("RENDER_SYSTEM_TYPE"));
    switch(*rendersystemType)
    {
#ifdef SUPPORT_D3D9
    case NBGM_RS_D3D9:
        {
            IDirect3D9 *d3d9 = (IDirect3D9*)config->renderContext->GetConfiguration("D3D9DRIVER");
            IDirect3DDevice9 *device = (IDirect3DDevice9*)config->renderContext->GetConfiguration("D3D9DEVICE");;
            result = NBRE_NEW NBRE_D3D9RenderPal(d3d9, device);
        }
        break;
#endif

#ifdef SUPPORT_D3D11
    case NBGM_RS_D3D11:
        {
            NBRE_D3D11RenderEnv env;
            env.mD3D11Device = config.device;
            env.mImmediateContext = config.context;
            env.mDepthStecilView = config.depthStencilView;
            env.mRenderTargetView = config.renderTargetView;
            env.mWorkPath = NBGM_GetInstance()->workspace;
            env.mPal = pal;
            renderer = NBRE_NEW NBRE_D3D11RenderPal(env);
        }
        break;
#endif

#ifdef SUPPORT_GL
    case NBGM_RS_GL:
        {
            result =  NBRE_NEW NBRE_GlRenderPal();
        }
        break;
#endif

#ifdef SUPPORT_GLES
    case NBGM_RS_GLES:
        {
            result =  NBRE_NEW NBRE_GLESRenderPal();
        }
        break;
#endif

#ifdef SUPPORT_GLES20
    case NBGM_RS_GLES20:
        {
            result =  NBRE_NEW NBRE_GLES2RenderPal(pal);
        }
        break;
#endif

    default:
        err = PAL_ErrUnsupported;
        break;
    }

    bool enableAntiAliasing = false;
    bool *flag = (bool*)config->renderContext->GetConfiguration("ANTI-ALIASING");
    if(flag && *flag)
    {
        enableAntiAliasing = true;
    }
    bool enableAnisotropicFiltering = false;
    flag = (bool*)config->renderContext->GetConfiguration("ANISOTROPICFILTERING");
    if(flag && *flag)
    {
        enableAnisotropicFiltering = true;
    }
    NBRE_RenderConfig renderConfig;
    renderConfig.enableAntiAliasing = enableAntiAliasing;
    renderConfig.enableAnisotropicFiltering = enableAnisotropicFiltering;
    result->Configure(renderConfig);
    *renderer = result;
    return err;
}

NBRE_RenderEngine*
NBGM_CreateRenderEingne(NBRE_IRenderPal* renderPal, PAL_Instance* pal)
{
    if(renderPal == NULL || pal == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_CreateRenderEingne failed");
        nbre_assert(0);
        return NULL;
    }

    NBRE_RenderEngine* result = NBRE_NEW NBRE_RenderEngine(pal);
    result->SetRenderPal(renderPal);
    NBRE_RenderConfig config;
    config.fontFolderPath = NBRE_String(gNbgmInstance.config.workSpace);
    result->Initialize(renderPal, config);
    return result;
}

NBGM_DEF PAL_Error
NBGM_CreateMapView(NBGM_MapViewConfig* config, NBGM_MapView** mapView)
{
    PAL_Error err = PAL_Ok;
    if(!gInitialized)
    {
        return PAL_ErrNotReady;
    }
    if( mapView == NULL || config == NULL)
    {
        return PAL_ErrBadParam;
    }
    if((config->drawSky || config->drawFlag || config->drawAvatar) && (gNbgmInstance.workspace.empty()))
    {
        return PAL_ErrNotFound;
    }
    *mapView = NBRE_NEW NBGM_MapViewInternal(*config, gNbgmInstance.config.pal);
    return err;
}

NBGM_DEF PAL_Error
NBGM_CreateNavView(NBGM_NavViewConfig* config, NBGM_NavView** navView)
{
    PAL_Error err = PAL_Ok;
    if(!gInitialized)
    {
        return PAL_ErrNotReady;
    }

    if(navView == NULL || config == NULL)
    {
        return PAL_ErrBadParam;
    }
    if((config->mapViewConfig.drawSky || config->mapViewConfig.drawFlag || config->mapViewConfig.drawAvatar) && (gNbgmInstance.workspace.empty()))
    {
        return PAL_ErrNotFound;
    }
    *navView = NBRE_NEW NBGM_NavViewInternal(*config, gNbgmInstance.config.pal);
    return err;
}

NBGM_DEC PAL_Error
NBGM_CreateMapViewWithSharedResource(NBGM_MapViewConfig* config, NBGM_ResourceManager* resourceManager, NBGM_MapView** mapView)
{
    PAL_Error err = PAL_Ok;
    if(!gInitialized)
    {
        return PAL_ErrNotReady;
    }
    if( mapView == NULL || config == NULL || resourceManager == NULL)
    {
        return PAL_ErrBadParam;
    }
    if((config->drawSky || config->drawFlag || config->drawAvatar) && (gNbgmInstance.workspace.empty()))
    {
        return PAL_ErrNotFound;
    }
    *mapView = NBRE_NEW NBGM_MapViewInternal(*config, gNbgmInstance.config.pal, resourceManager);
    return err;
}

NBGM_DEC PAL_Error
NBGM_CreateNavViewWithSharedResource(NBGM_NavViewConfig* config, NBGM_ResourceManager* resourceManager, NBGM_NavView** navView)
{
    PAL_Error err = PAL_Ok;
    if(!gInitialized)
    {
        return PAL_ErrNotReady;
    }

    if(navView == NULL || config == NULL || resourceManager == NULL)
    {
        return PAL_ErrBadParam;
    }
    if((config->mapViewConfig.drawSky || config->mapViewConfig.drawFlag || config->mapViewConfig.drawAvatar) && (gNbgmInstance.workspace.empty()))
    {
        return PAL_ErrNotFound;
    }
    *navView = NBRE_NEW NBGM_NavViewInternal(*config, gNbgmInstance.config.pal, resourceManager);
    return err;
}

NBGM_DEF NBGM_Instance*
NBGM_GetInstance()
{
    return &gNbgmInstance;
}

NBGM_DEF const NBGM_Config*
NBGM_GetConfig()
{
    if (!gInitialized)
    {
        return NULL;
    }
    return &gNbgmInstance.config;
}

NBGM_DEF void
NBGM_SetDebugLogSeverity(PAL_LogSeverity severity)
{
    NBRE_SetDebugLogSeverity(severity);
}

NBGM_DEF uint32
NBGM_GetSupportedNBMFileVersion()
{
    return 24;
}

NBGM_DEF void
NBGM_EnableLayerFarNearVisibility(nb_boolean enable)
{
    if (gInitialized)
    {
    gNbgmInstance.config.useRoadBuildingVisibility = enable;
    }
}

NBGM_DEF void
NBGM_SetPreferredLanguageCode(uint8 preferredLanguageCode)
{
    if (gInitialized)
    {
    gNbgmInstance.config.preferredLanguageCode = preferredLanguageCode;
    }
}

NBGM_DEF void
NBGM_ShowLabelDebugInfo(nb_boolean enable)
{
    if (gInitialized)
    {
        gNbgmInstance.config.showLabelDebugInfo = enable;
    }
}

NBGM_DEF void
NBGM_SetRelativeZoomLevel(int8 zoomlevel)
{
    if (gInitialized)
    {
    	gNbgmInstance.config.relativeZoomLevel = zoomlevel;
    }
}

NBGM_DEF void
NBGM_SetRelativeCameraHeight(float distance)
{
    if (gInitialized)
    {
    	gNbgmInstance.config.relativeCameraDistance = distance;
    }
}

NBGM_DEF void
NBGM_SetResolutionRelativeZoomLevel(float resolution)
{
    if (gInitialized)
    {
    	gNbgmInstance.config.metersPerPixelOnRelativeZoomLevel = resolution;
    }
}

NBGM_DEF PAL_Error
NBGM_CreateResourceManager(const NBGM_ResourceManagerConfig* resourceManagerConfig, NBGM_ResourceManager** resourceManager)
{
    PAL_Error err = PAL_Ok;
    if(!gInitialized)
    {
        return PAL_ErrNotReady;
    }

    if(resourceManagerConfig == NULL || resourceManager == NULL)
    {
        return PAL_ErrBadParam;
    }

    *resourceManager = NBRE_NEW NBGM_ResourceManager(*resourceManagerConfig, gNbgmInstance.config.pal);

    return err;
}

NBGM_DEC PAL_Error
NBGM_DestroyResourceManager(NBGM_ResourceManager** resourceManager)
{
    if(resourceManager == NULL || *resourceManager == NULL)
    {
        return PAL_ErrBadParam;
    }

    NBRE_DELETE *resourceManager;

    return PAL_Ok;
}

NBGM_DEC PAL_Error
NBGM_CreateMapCameraHelper(NBGM_MapCameraHelper** mapCameraHelper)
{
    if(!gInitialized)
    {
        return PAL_ErrNotReady;
    }

    if(mapCameraHelper == NULL)
    {
        return PAL_ErrBadParam;
    }

    *mapCameraHelper = NBRE_NEW NBGM_MapCameraHelperImpl();

    return PAL_Ok;
}

NBGM_DEF void
NBGM_SetDPI(float dpi)
{
    gNbgmInstance.config.dpi = dpi;
}
