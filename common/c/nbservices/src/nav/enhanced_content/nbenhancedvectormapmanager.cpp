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

    @file nbenhancedvectormapmanager.cpp
    @date 02/21/2012

*/
/*
    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#pragma warning(disable : 4251)

#include "nbenhancedvectormapmanager.h"
#include "nbenhancedvectormapprivatetypes.h"
#include "LayerManager.h"
#include "TileServicesConfiguration.h"

#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "palstdlib.h"
#include "palfile.h"
#include "TileInternal.h"
#include "StringUtility.h"

#include "MetadataConfiguration.h"

static const char* s_metadataPersistentName = "_METADATA_";
static const char* s_dayMaterialKeyName = "DMAT";
static const char* s_nightMaterialKeyName = "NMAT";

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    MetadataAsyncCallback is used for adapting low layer interface

    To handle asynchronized network request event.

*/
class MetadataAsyncCallback
    : public nbmap::AsyncCallback<const std::vector<shared_ptr<nbmap::Layer > >& >
{
public:
    MetadataAsyncCallback();
    virtual ~MetadataAsyncCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(const std::vector<shared_ptr<nbmap::Layer > >& response);
	virtual void Error(NB_Error error);
	virtual bool Progress(int percentage);

    inline void SetEnhancedVectorMapManager(NB_EnhancedVectorMapManager* pManager) { m_pEnhancedVectorMapManager = pManager; }
private:
    MetadataAsyncCallback(const MetadataAsyncCallback&);
    MetadataAsyncCallback& operator=(const MetadataAsyncCallback&);

    NB_EnhancedVectorMapManager* m_pEnhancedVectorMapManager;
};

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    CommonMaterialAsyncCallback is used for adapting low layer interface

    To handle asynchronized network request event.

*/
class CommonMaterialAsyncCallback
    : public nbmap::AsyncCallbackWithRequest<nbmap::TileKeyPtr, nbmap::TilePtr>
{
public:
    CommonMaterialAsyncCallback();
    virtual ~CommonMaterialAsyncCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(nbmap::TileKeyPtr request, nbmap::TilePtr response);
	virtual void Error(nbmap::TileKeyPtr request, NB_Error error);
	virtual bool Progress(int percentage);

    inline void SetEnhancedVectorMapManager(NB_EnhancedVectorMapManager* pManager) { m_pEnhancedVectorMapManager = pManager; }
    inline void SetDayTime(bool bDayTime) { m_bDayTime = bDayTime; }
private:
    CommonMaterialAsyncCallback(const CommonMaterialAsyncCallback&);
    CommonMaterialAsyncCallback& operator=(const CommonMaterialAsyncCallback&);

    NB_EnhancedVectorMapManager* m_pEnhancedVectorMapManager;
    bool                         m_bDayTime;
};

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    NB_EnhancedVectorMapManager interface implementation
*/

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerCreate(NB_Context* pContext, const NB_EnhancedVectorMapManagerConfig* pConfig, NB_EnhancedVectorMapManager** pManager)
{
    NB_EnhancedVectorMapManager* result = (NB_EnhancedVectorMapManager*)nsl_malloc(sizeof(NB_EnhancedVectorMapManager));
    if (!result)
    {
        return NE_NOMEM;
    }
    nsl_memset(result, 0, sizeof(NB_EnhancedVectorMapManager));

    result->context = pContext;
    result->enabledLayers = 0xFFFFFFFF;
    result->screenWidth = pConfig->screenWidth;
    result->screenHeight = pConfig->screenHeight;

    // Don't create instance directly. We have to make sure all ptr is initiallized correctly.
    new(&result->workPath) shared_ptr<std::string>();
    new(&result->language) shared_ptr<std::string>();
    new(&result->layerManager) shared_ptr<nbmap::LayerManager>();
    new(&result->metadataAsyncCallback) shared_ptr<MetadataAsyncCallback>();
    new(&result->dayMaterialAsyncCallback) shared_ptr<CommonMaterialAsyncCallback>();
    new(&result->nightMaterialAsyncCallback) shared_ptr<CommonMaterialAsyncCallback>();
    new(&result->layers) std::vector<shared_ptr<nbmap::Layer> >();
    new(&result->layerManagerListener) RefreshMaterialListener(result);
    new(&result->dayMaterial) nbcommon::DataStreamPtr();
    new(&result->nightMaterial) nbcommon::DataStreamPtr();

    // Initiallize work path. This string is used by low layer logic, so we have to use AutoSharedPtr
    result->workPath.reset(new std::string(pConfig->vectorTileDataPath));
    if (!result->workPath)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }

    result->language.reset(new std::string(pConfig->language));
    if (!result->language)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }

    // Initiallize LayerManager
    result->layerManager = nbmap::TileServicesConfiguration::GetLayerManager(pContext,
                                                                             pConfig->maxCachingTileCount,
                                                                             pConfig->maximumTileRequestCountPerLayer,
                                                                             shared_ptr<nbmap::MetadataConfiguration>(new nbmap::MetadataConfiguration())
                                                                             );
    if (!result->layerManager)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }
    result->layerManager->SetCachePath(result->workPath);
    char metadataPathString[MAX_ENHANCED_VECTOR_MAP_PATH_LENGTH + 1] = {0};
    nsl_strcpy(metadataPathString, result->workPath->c_str());
    if (PAL_FileAppendPath(NB_ContextGetPal(pContext), metadataPathString, MAX_ENHANCED_VECTOR_MAP_PATH_LENGTH + 1, s_metadataPersistentName) != PAL_Ok)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }
    shared_ptr<std::string> metadataPath(new std::string(metadataPathString));
    if (!metadataPath)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }
    result->layerManager->SetPersistentMetadataPath(metadataPath);
    result->layerManager->RegisterListener(&result->layerManagerListener);

    // Initiallize internal callback objects
    result->metadataAsyncCallback.reset(new MetadataAsyncCallback());
    if (!result->metadataAsyncCallback)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }
    result->metadataAsyncCallback->SetEnhancedVectorMapManager(result);

    result->dayMaterialAsyncCallback.reset(new CommonMaterialAsyncCallback());
    if (!result->dayMaterialAsyncCallback)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }
    result->dayMaterialAsyncCallback->SetEnhancedVectorMapManager(result);
    result->dayMaterialAsyncCallback->SetDayTime(true);

    result->nightMaterialAsyncCallback.reset(new CommonMaterialAsyncCallback());
    if (!result->nightMaterialAsyncCallback)
    {
        NB_EnhancedVectorMapManagerDestroy(result);
        return NE_NOMEM;
    }
    result->nightMaterialAsyncCallback->SetEnhancedVectorMapManager(result);
    result->nightMaterialAsyncCallback->SetDayTime(false);

    NB_ContextSetEnhancedVectorMapManagerNoOwnershipTransfer(pContext, result);
    *pManager = result;

    return NE_OK;
}

/*! See header file for detail */
NB_DEF void
NB_EnhancedVectorMapManagerDestroy(NB_EnhancedVectorMapManager* pManager)
{
    if (pManager->layerManager)
    {
        pManager->layerManager->UnregisterListener(&pManager->layerManagerListener);
    }
    // In case that NB_EnhancedVectorMapManager has been destroyed, but request still in progress,
    // we have to tell callback objects about this.
    if (pManager->metadataAsyncCallback)
    {
        pManager->metadataAsyncCallback->SetEnhancedVectorMapManager(NULL);
    }
    if (pManager->dayMaterialAsyncCallback)
    {
        pManager->dayMaterialAsyncCallback->SetEnhancedVectorMapManager(NULL);
    }
    if (pManager->nightMaterialAsyncCallback)
    {
        pManager->nightMaterialAsyncCallback->SetEnhancedVectorMapManager(NULL);
    }
    NB_ContextSetEnhancedVectorMapManagerNoOwnershipTransfer(pManager->context, NULL);
    // free all ptr
    pManager->workPath.~shared_ptr();
    pManager->language.~shared_ptr();
    pManager->layerManager.~shared_ptr();
    pManager->metadataAsyncCallback.~shared_ptr();
    pManager->dayMaterialAsyncCallback.~shared_ptr();
    pManager->nightMaterialAsyncCallback.~shared_ptr();
    pManager->layers.~vector();
    pManager->layerManagerListener.~RefreshMaterialListener();
    pManager->dayMaterial.~shared_ptr();
    pManager->nightMaterial.~shared_ptr();
    nsl_free(pManager);
}

/*! See header file for detail */
NB_DEF void
NB_EnhancedVectorMapManagerClear(NB_EnhancedVectorMapManager* pManager)
{
    if (pManager->layerManager)
    {
        // 'ClearLayers' will remove all request processing, callback will never be invoked.
        pManager->metadataCallback.callback = NULL;
        pManager->commonMaterialCallback.callback = NULL;
        pManager->layerManager->RemoveAllTiles();
        pManager->dataStatusFlags = 0;
        pManager->layers.clear();
        pManager->dayMaterial.reset();
        pManager->nightMaterial.reset();
    }
}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerRequestMetadata(NB_EnhancedVectorMapManager* pManager, EnhancedVectorMapMetadataRequestCallback callback)
{
    if (!pManager->layerManager)
    {
        return NE_UNEXPECTED;
    }
    if (pManager->metadataCallback.callback || pManager->commonMaterialCallback.callback)
    {
        // a request is already in progress.
        return NE_BUSY;
    }
    pManager->metadataCallback = callback;
    pManager->dataStatusFlags = 0;

    nbmap::MetadataConfigurationPtr config;
    //@note: Height and width of screen are discarded, are they really useful for navigation?
    config->m_pLanguage = pManager->language;

    return pManager->layerManager->GetLayers(pManager->metadataAsyncCallback,
                                             config);


}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerCancelMetadataRequest(NB_EnhancedVectorMapManager* pManager)
{
    return NE_NOSUPPORT;
}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerRequestCommonMaterial(NB_EnhancedVectorMapManager* pManager, EnhancedVectorMapCommonMaterialRequestCallback callback)
{
    if (!pManager->layerManager || !(pManager->dataStatusFlags & NB_EnhancedVectorMapData_MetaData))
    {
        return NE_UNEXPECTED;
    }
    if (pManager->commonMaterialCallback.callback)
    {
        // a request is already in progress.
        return NE_BUSY;
    }
    pManager->dataStatusFlags &= ~(NB_EnhancedVectorMapData_DayMaterial | NB_EnhancedVectorMapData_NightMaterial);
    pManager->commonMaterialCallback = callback;
    shared_ptr<string> dayType(new string("day"));
    shared_ptr<string> nightType(new string("night"));
    NB_Error err = pManager->layerManager->GetCommonMaterials(pManager->dayMaterialAsyncCallback, dayType);
    err = ( err == NE_OK ? pManager->layerManager->GetCommonMaterials(pManager->nightMaterialAsyncCallback, nightType) : err );
    return err;
}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerCancelCommonMaterialRequest(NB_EnhancedVectorMapManager* pManager)
{
    return NE_NOSUPPORT;
}

NB_DEF void
NB_EnhancedVectorMapManagerSetEnabledLayers(NB_EnhancedVectorMapManager* pManager, uint32 enabledLayers)
{
    pManager->enabledLayers = enabledLayers;
    std::vector<shared_ptr<nbmap::Layer> >::iterator iter = pManager->layers.begin();
    std::vector<shared_ptr<nbmap::Layer> >::iterator iterEnd = pManager->layers.end();
    while (iter != iterEnd)
    {
        shared_ptr<nbmap::Layer>& pLayer = *iter;
        shared_ptr<string> dataType = pLayer->GetTileDataType();
        if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_DETAILED_VECTOR_AREAS))))
        {
            pLayer->SetEnabled((pManager->enabledLayers & NB_TileLayer_Areas) == NB_TileLayer_Areas);
        }
        else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_DETAILED_VECTOR_ROADS))))
        {
            pLayer->SetEnabled((pManager->enabledLayers & NB_TileLayer_Roads) == NB_TileLayer_Roads);
        }
        else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_UNTEXTURED_3D_BUILDINGS))))
        {
            pLayer->SetEnabled((pManager->enabledLayers & NB_TileLayer_UntexBuilding ) == NB_TileLayer_UntexBuilding);
        }
        else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_LANDMARKS_3D))))
        {
            pLayer->SetEnabled((pManager->enabledLayers & NB_TileLayer_Landmarks) == NB_TileLayer_Landmarks);
        }
        iter++;
    }
}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerGetLayerDrawOrder(NB_EnhancedVectorMapManager* pManager, NB_TileLayerType layerType, uint32* drawOrder, uint32* labelDrawOrder)
{
    //nbmap::TileDataType dataType = nbmap::TDT_INVALID;
    shared_ptr<std::string> dataType;
    switch (layerType)
    {
        case NB_TileLayer_Areas:
            dataType = shared_ptr<std::string>(new string(TDT_DETAILED_VECTOR_AREAS));
            break;
        case NB_TileLayer_Roads:
            dataType = shared_ptr<std::string>(new string(TDT_DETAILED_VECTOR_ROADS));
            break;
        case NB_TileLayer_UntexBuilding:
            dataType = shared_ptr<std::string>(new string(TDT_UNTEXTURED_3D_BUILDINGS));
            break;
        case NB_TileLayer_Landmarks:
            dataType = shared_ptr<std::string>(new string(TDT_LANDMARKS_3D));
            break;
        default:
            break;
    }

    std::vector<shared_ptr<nbmap::Layer> >::iterator iter = pManager->layers.begin();
    std::vector<shared_ptr<nbmap::Layer> >::iterator iterEnd = pManager->layers.end();
    while (iter != iterEnd)
    {
        shared_ptr<nbmap::Layer>& pLayer = *iter;
        if (nbcommon::StringUtility::IsStringEqual(pLayer->GetTileDataType(),dataType))
        {
            if (drawOrder)
            {
                *drawOrder = pLayer->GetDrawOrder();
            }
            if (labelDrawOrder)
            {
                *labelDrawOrder = pLayer->GetLabelDrawOrder();
            }
            return NE_OK;
        }
        ++iter;
    }
    return NE_NOENT;
}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerGetDayMaterial(NB_EnhancedVectorMapManager* pManager, void** data, uint32* size)
{
    if (pManager->dayMaterial)
    {
        uint32 dataSize = pManager->dayMaterial->GetDataSize();
        void* pData = nsl_malloc(dataSize);
        if (!pData)
        {
            return NE_NOMEM;
        }
        if (dataSize != pManager->dayMaterial->GetData((uint8*)pData, 0, dataSize))
        {
            nsl_free(pData);
            return NE_NOENT;
        }
        *data = pData;
        *size = dataSize;
        return NE_OK;
    }
    return NE_NOENT;
}

/*! See header file for detail */
NB_DEF NB_Error
NB_EnhancedVectorMapManagerGetNightMaterial(NB_EnhancedVectorMapManager* pManager, void** data, uint32* size)
{
    if (pManager->dayMaterial)
    {
        uint32 dataSize = pManager->nightMaterial->GetDataSize();
        void* pData = nsl_malloc(dataSize);
        if (!pData)
        {
            return NE_NOMEM;
        }
        if (dataSize != pManager->nightMaterial->GetData((uint8*)pData, 0, dataSize))
        {
            nsl_free(pData);
            return NE_NOENT;
        }
        *data = pData;
        *size = dataSize;
        return NE_OK;
    }
    return NE_NOENT;
}

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    MetadataAsyncCallback implementation
*/
MetadataAsyncCallback::MetadataAsyncCallback()
    : m_pEnhancedVectorMapManager(NULL)
{
}

MetadataAsyncCallback::~MetadataAsyncCallback()
{
}

void
MetadataAsyncCallback::Success(const std::vector<shared_ptr<nbmap::Layer > >& response)
{
    if (m_pEnhancedVectorMapManager)
    {
        m_pEnhancedVectorMapManager->dataStatusFlags = NB_EnhancedVectorMapData_MetaData;
        m_pEnhancedVectorMapManager->layers.clear();
        m_pEnhancedVectorMapManager->layers = response;

        // reset enable status
        std::vector<shared_ptr<nbmap::Layer> >::iterator iter = m_pEnhancedVectorMapManager->layers.begin();
        std::vector<shared_ptr<nbmap::Layer> >::iterator iterEnd = m_pEnhancedVectorMapManager->layers.end();
        while (iter != iterEnd)
        {
            shared_ptr<nbmap::Layer>& pLayer = *iter;
            shared_ptr<string> dataType = pLayer->GetTileDataType();
            if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_DETAILED_VECTOR_AREAS))))
            {
                pLayer->SetEnabled((m_pEnhancedVectorMapManager->enabledLayers & NB_TileLayer_Areas) == NB_TileLayer_Areas);
            }
            else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_DETAILED_VECTOR_ROADS))))
            {
                pLayer->SetEnabled((m_pEnhancedVectorMapManager->enabledLayers & NB_TileLayer_Roads) == NB_TileLayer_Roads);
            }
            else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_UNTEXTURED_3D_BUILDINGS))))
            {
            }
            else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_LANDMARKS_3D))))
            {
                pLayer->SetEnabled((m_pEnhancedVectorMapManager->enabledLayers & NB_TileLayer_Landmarks) == NB_TileLayer_Landmarks);
            }
            iter++;
        }

        if (m_pEnhancedVectorMapManager->metadataCallback.callback)
        {
            m_pEnhancedVectorMapManager->metadataCallback.callback(m_pEnhancedVectorMapManager,
                                                                   NE_OK,
                                                                   100,
                                                                   m_pEnhancedVectorMapManager->metadataCallback.callbackData
                                                                   );
            m_pEnhancedVectorMapManager->metadataCallback.callback     = NULL;
            m_pEnhancedVectorMapManager->metadataCallback.callbackData = NULL;
        }
    }
}
void
MetadataAsyncCallback::Error(NB_Error error)
{
    if (m_pEnhancedVectorMapManager)
    {
        if (m_pEnhancedVectorMapManager->metadataCallback.callback)
        {
            m_pEnhancedVectorMapManager->metadataCallback.callback(m_pEnhancedVectorMapManager,
                                                                   error,
                                                                   0,
                                                                   m_pEnhancedVectorMapManager->metadataCallback.callbackData
                                                                   );
            m_pEnhancedVectorMapManager->metadataCallback.callback = NULL;
            m_pEnhancedVectorMapManager->metadataCallback.callbackData = NULL;
        }
    }
}

bool
MetadataAsyncCallback::Progress(int percentage)
{
    if (m_pEnhancedVectorMapManager)
    {
        if (m_pEnhancedVectorMapManager->metadataCallback.callback)
        {
            m_pEnhancedVectorMapManager->metadataCallback.callback(m_pEnhancedVectorMapManager,
                                                                   NE_OK,
                                                                   percentage,
                                                                   m_pEnhancedVectorMapManager->metadataCallback.callbackData
                                                                   );
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    MetadataAsyncCallback implementation
*/
CommonMaterialAsyncCallback::CommonMaterialAsyncCallback()
    : m_pEnhancedVectorMapManager(NULL)
{
}

CommonMaterialAsyncCallback::~CommonMaterialAsyncCallback()
{
}

void
CommonMaterialAsyncCallback::Success(nbmap::TileKeyPtr request, nbmap::TilePtr response)
{
    if (m_pEnhancedVectorMapManager)
    {
        shared_ptr<std::string> dataType = response->GetDataType();
        nbcommon::DataStreamPtr dataStream = response->GetData();
        if ((dataType == shared_ptr<string>()) || (!dataStream))
        {
            // data is invalid
            if (m_pEnhancedVectorMapManager->commonMaterialCallback.callback)
            {
                m_pEnhancedVectorMapManager->commonMaterialCallback.callback(m_pEnhancedVectorMapManager,
                                                                             NE_BADDATA,
                                                                             0,
                                                                             NULL,
                                                                             m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData
                                                                             );
            }
            m_pEnhancedVectorMapManager->commonMaterialCallback.callback = NULL;
            m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData = NULL;
            return;
        }

        // remember common material file name
        if ( m_bDayTime )
        {
            m_pEnhancedVectorMapManager->dataStatusFlags |= NB_EnhancedVectorMapData_DayMaterial;
            m_pEnhancedVectorMapManager->dayMaterial = dataStream;
        }
        else
        {
            m_pEnhancedVectorMapManager->dataStatusFlags |= NB_EnhancedVectorMapData_NightMaterial;
            m_pEnhancedVectorMapManager->nightMaterial = dataStream;
        }

        if (m_pEnhancedVectorMapManager->dataStatusFlags & NB_EnhancedVectorMapData_DayMaterial
            && m_pEnhancedVectorMapManager->dataStatusFlags & NB_EnhancedVectorMapData_NightMaterial
            && m_pEnhancedVectorMapManager->commonMaterialCallback.callback)
        {
            m_pEnhancedVectorMapManager->commonMaterialCallback.callback(m_pEnhancedVectorMapManager,
                                                                         NE_OK,
                                                                         100,
                                                                         NULL,
                                                                         m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData
                                                                         );
            m_pEnhancedVectorMapManager->commonMaterialCallback.callback = NULL;
            m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData = NULL;
        }
    }
}

void
CommonMaterialAsyncCallback::Error(nbmap::TileKeyPtr request, NB_Error error)
{
    if (m_pEnhancedVectorMapManager)
    {
        if (m_pEnhancedVectorMapManager->commonMaterialCallback.callback)
        {
            m_pEnhancedVectorMapManager->commonMaterialCallback.callback(m_pEnhancedVectorMapManager,
                                                                         NE_OK,
                                                                         0,
                                                                         NULL,
                                                                         m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData
                                                                         );
            m_pEnhancedVectorMapManager->commonMaterialCallback.callback = NULL;
            m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData = NULL;
        }
    }
}

bool
CommonMaterialAsyncCallback::Progress(int percentage)
{
    if (m_pEnhancedVectorMapManager)
    {
        if (m_pEnhancedVectorMapManager->commonMaterialCallback.callback)
        {
            m_pEnhancedVectorMapManager->commonMaterialCallback.callback(m_pEnhancedVectorMapManager,
                                                                         NE_OK,
                                                                         percentage,
                                                                         NULL,
                                                                         m_pEnhancedVectorMapManager->commonMaterialCallback.callbackData
                                                                         );
        }
    }
    return true;
}

void
RefreshMaterialListener::RefreshAllTiles()
{
    NB_EnhancedVectorMapManagerRequestCommonMaterial(m_pEnhancedVectorMapManager, m_pEnhancedVectorMapManager->commonMaterialCallback);
}

void
RefreshMaterialListener::RefreshTilesOfLayer(shared_ptr<nbmap::Layer> layer)
{
}
void
RefreshMaterialListener::RefreshTiles(const std::vector<shared_ptr<nbmap::TileKey> >& tileKeys, shared_ptr<nbmap::Layer> layer)
{
}

void
RefreshMaterialListener::LayersUpdated(const vector<shared_ptr<nbmap::Layer> >& layers)
{
    m_pEnhancedVectorMapManager->layers.clear();
    m_pEnhancedVectorMapManager->layers = layers;

    // reset enable status
    std::vector<shared_ptr<nbmap::Layer> >::iterator iter = m_pEnhancedVectorMapManager->layers.begin();
    std::vector<shared_ptr<nbmap::Layer> >::iterator iterEnd = m_pEnhancedVectorMapManager->layers.end();
    while (iter != iterEnd)
    {
        shared_ptr<nbmap::Layer>& pLayer = *iter;
        shared_ptr<string> dataType = pLayer->GetTileDataType();
        if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_DETAILED_VECTOR_AREAS))))
        {
            pLayer->SetEnabled((m_pEnhancedVectorMapManager->enabledLayers & NB_TileLayer_Areas) == NB_TileLayer_Areas);
        }
        else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_DETAILED_VECTOR_ROADS))))
        {
            pLayer->SetEnabled((m_pEnhancedVectorMapManager->enabledLayers & NB_TileLayer_Roads) == NB_TileLayer_Roads);
        }
        else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_UNTEXTURED_3D_BUILDINGS))))
        {
        }
        else if (nbcommon::StringUtility::IsStringEqual(dataType, shared_ptr<std::string>(new std::string(TDT_LANDMARKS_3D))))
        {
            pLayer->SetEnabled((m_pEnhancedVectorMapManager->enabledLayers & NB_TileLayer_Landmarks) == NB_TileLayer_Landmarks);
        }
        iter++;
    }
}

void
RefreshMaterialListener::LayersAdded(const std::vector<shared_ptr<nbmap::Layer> >& layers)
{
}

void
RefreshMaterialListener::LayersRemoved(const std::vector<shared_ptr<nbmap::Layer> >& layers)
{
}

void
RefreshMaterialListener::AnimationLayerAdded(shared_ptr<nbmap::Layer> layer)
{
}

void
RefreshMaterialListener::AnimationLayerUpdated(shared_ptr<nbmap::Layer> layer, const vector<uint32>& frameList)
{
}

void
RefreshMaterialListener::AnimationLayerRemoved(shared_ptr<nbmap::Layer> layer)
{
}

void
RefreshMaterialListener::PinsRemoved(shared_ptr<std::vector<shared_ptr<nbmap::Pin> > > pins)
{
}

void
RefreshMaterialListener::EnableMapLegend(bool enabled, shared_ptr <nbmap::MapLegendInfo> mapLegend)
{
}

#pragma warning(default : 4251)

/*! @} */
