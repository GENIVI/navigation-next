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
  @file        AggregateLayerProvider.h
  @defgroup    map

  Description: AggregateLayerProvider puts all layer provider together, and
  exports to the LayerManger.

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/
 /*! @{ */

#ifndef __AGGREGATE_LAYER_PROVIDER__
#define __AGGREGATE_LAYER_PROVIDER__

extern "C"
{
#include "paltypes.h"
#include "pal.h"
#include "paltimer.h"
#include "palclock.h"
#include "pallock.h"
#include "nberror.h"
}

#include "base.h"
#include "LayerProvider.h"
#include "AsyncCallback.h"
#include "nbcontext.h"
#include "UnifiedLayer.h"
#include "Handler.h"
#include "UnifiedLayerListener.h"

namespace nbmap
{

/*! Aggregate Implementation for Layer Provider.

   This will contain a collection of LayerProviders.

*/
class LayerCallback;

/*! Aggregate LayerProvider. */
class AggregateLayerProvider : public Base,
                               public LayerProvider
{
public:
    AggregateLayerProvider(NB_Context* context,     /*!< Pointer to current context      */
                           uint32 nbgmVersion = 0   /*!< NBGM version to format requesting
                                                         URL. And it should be checked with
                                                         minimum and maximum version from
                                                         metadata protocol.              */
                          );

    virtual ~AggregateLayerProvider();

    /*See Description in LayerProvider.h */
    virtual void GetLayers(shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > > callback,
                           shared_ptr<MetadataConfiguration> metadataConfig);
    virtual void ClearLayers();
    virtual const LayerPtr GetLayerAvailabilityMatrix() const;
    virtual const std::vector<LayerPtr>& GetLayerPtrVector() const;
    virtual const std::vector<LayerPtr>& GetCommonMaterialLayers() const;
    virtual const std::vector<LayerPtr>& GetOptionalLayerPtrVector() const;
    virtual const LayerPtr GetDAMLayer() const;

    /*! Please refer LayerManager.h */
    virtual NB_Error SetCachePath(shared_ptr<std::string> cachePath);
    virtual NB_Error GetBackgroundRasterRange(uint32& minZoomLevel,
                                              uint32& maxZoomLevel);
    virtual void SetPreferredLanguageCode(uint8 languageCode);
    virtual void SetTheme(MapViewTheme theme);
    virtual void SetMetadataRetryTimes(int number);
    virtual NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath);
    virtual void CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback, shared_ptr<MetadataConfiguration> config, bool skipTimeInterval, bool forceUpdate);
    NB_Context* GetContext();

    NB_Error AddLayerProvider(LayerProviderPtr layerProvider);
    vector<LayerProviderPtr> GetLayerProviderVector();
    void Initialize();
    bool IsInitialized();
    void CallbackSuccess();
    void CallbackError(NB_Error error);

    /*! Clear cached layers and replace them with layers from sub-providers.

        @return None
    */
    void UpdateLayersFromSubProviders();

    /*! Add a layer

        @return true if need to notify user to update layers, false otherwise.
    */
    bool
    AddLayer(LayerPtr layer     /*!< A layer to add */
             );

    /*! Remove a layer

        @return true if need to notify user to update layers, false otherwise.
    */
    bool
    RemoveLayer(LayerPtr layer  /*!< A layer to remove */
                );

private:

    /*! Updates layers from specified layerProvider.

            @return void
    */
    void UpdateLayersFromSingleProvider(LayerProviderPtr layerProvider);

    bool m_isInitialized;                           /*!< Is this object initialized? */
    NB_Context* m_pContext;                         /*!< Pointer to current context */
    std::vector<LayerProviderPtr> m_LayerProviders;

    std::vector<LayerPtr> m_pLayers;                /*!< cached layer list */
    std::vector<LayerPtr> m_pCommonMaterialLayers;  /*!< cached common materials layer list */
    std::vector<LayerPtr> m_pOptionalLayers;
    LayerPtr m_pLayerAvailabilityMatrix;            /*!< Layer Availability Matrix Layer */
    LayerPtr m_pDAMLayer;                           /*!< Data Availability Matrix Layer */
    shared_ptr<LayerCallback> m_pLayerCallback;     /*! Callback class to handle layer callbacks. */
    shared_ptr<AsyncCallback<const std::vector<LayerPtr>& > > m_pCallbackFinal;

};


/*! Callback for AggregateLayerProvider. */
class LayerCallback : public AsyncCallback<const vector<LayerPtr>& >
{
public:
    LayerCallback();
    ~LayerCallback();

    // Refer to AsyncCallback for description.
    void Initialize(AggregateLayerProvider* aggregateLayerProvider);
    virtual void Success(const vector<LayerPtr>& response);
    virtual void Error(NB_Error error);

    /*! Check whether we can start new requests or not.

        @return true if we can.
    */
    bool CanStartRequest();

    /*! Update callback count we should wait.

        @return None.
    */
    void SetCallbackCount(unsigned int number  /*!< Callback number to wait. */
                          );

    /*! Remove a layer in temporary vector for callback

        @return None
    */
    void
    RemoveLayer(LayerPtr layer  /*!< A layer to remove */
                );

    /*! Set a  new AggregateLayerProvider for callback

     @return None
     */
    void
    SetAggregateLayerProvider(AggregateLayerProvider* aggregateLayerProvider  /*!< New provider */
                );

private:
    AggregateLayerProvider*  m_pAggregateLayerProvider; /*!< Instance of AggregateLayerProvider*/
    vector<LayerProviderPtr> m_LayerProviders;          /*!< Copy of available layer providers */
    std::vector<LayerPtr>    m_pLayers; /*!< vector of created layers. */

    size_t    m_callbackCount;          /*!< registered callback  common count  */
    size_t    m_remainedCallbackCount;  /*<  count of unfinished callback */
    bool      hasErrorOccured;          /*!< Flag indicate error occurred. */
};

}

#endif

/*! @} */
