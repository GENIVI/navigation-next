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
@file     LayerProvider.h
@defgroup nbmap

*/
/*
(C) Copyright 2011 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __LAYER_PROVIDER__
#define __LAYER_PROVIDER__

#include "nbexp.h"
#include "nbcontext.h"
#include "smartpointer.h"
#include <map>
#include <vector>
#include <list>
#include "Layer.h"
#include "AsyncCallback.h"
#include "LayerManager.h"
#include "HybridStrategy.h"

namespace nbmap
{

class MetadataConfiguration;
class UnifiedLayerListener;

/*! Providers the list of supported layers.

The layers are all returned as UnifiedLayer instances.
*/

class LayerProvider
{
public:

    LayerProvider() {}

    virtual ~LayerProvider() {}

    /*! Returns the array of supported layers.

    Each time this method is called, the data is refreshed (re-downloaded or re-generated).
    LayerManager should cache this data.

    @return Array of the supported layers as obtained from Metadata request.
    */
    virtual void GetLayers(shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback,
                           shared_ptr<MetadataConfiguration> metadataConfig
                           ) = 0;

    /*! Clear layers

        This function clears all layers and persistent information about layers.

        @return None
    */
    virtual void ClearLayers() = 0;

    /*! Return the LAM layer pointer.

        This method will return a pointer to the Layer
        Avialability Matrix Layer.

       @return Shared pointer of LAM layer.
    */
    virtual const LayerPtr GetLayerAvailabilityMatrix() const = 0;

    /*! Return Stack of Layers to the caller

        This call will return the layers created based on Metadata
        and Maptile reply. This stack will not include Common Materials Layer
        and LAM Layer.

        @ return vector of pointers to Layer Interface.
    */
    virtual const std::vector<LayerPtr>& GetLayerPtrVector() const = 0;

    /*! Return Stack of Optional Layers to the caller

      This call will return the optional layers created based on Metadata
      and Maptile reply. This stack will not include Common Materials Layer
      and LAM Layer.

      @ return vector of pointers to Layer Interface.
    */
    virtual const std::vector<LayerPtr>& GetOptionalLayerPtrVector() const = 0;


    /*! Return Stack of Common Materials Layer

        This method will return stack of common materials.
        Common materials are only requested once during startup and
        persisted thereafter.

        @return vector of pointer to common materials.
    */
    virtual const std::vector<LayerPtr>& GetCommonMaterialLayers() const = 0;

    /*! Get minimum and maximum zoom levels of raster layers

        @return NE_OK if success,
        NE_NOENT if there is no zoom levels specified.
    */
    virtual NB_Error
    GetBackgroundRasterRange(uint32& minZoomLevel,    /*!< On return minimum zoom level for all raster layers */
                             uint32& maxZoomLevel     /*!< On return maximum zoom level for all raster layers */
                            ) = 0;

    /*! Please refer LayerManager.h */
    virtual NB_Error SetCachePath(shared_ptr<std::string> cachePath) = 0;

    /*! Return the data availability matrix layer pointer.

        This method will return a pointer to the Data
        Avialability Matrix Layer.

       @return Shared pointer of data availability matrix layer.
    */
    virtual const LayerPtr GetDAMLayer() const = 0;

    /*! Set preferred language code for LayerProvider.

      Layers belongs to this layer provider should use this language to generate tiles for
      MapView to display.

      @param preferredLanguageCode Preferred language code for labels. 0 means local
      language. See NBM format doc for full code list.

      @return None
    */
    virtual void SetPreferredLanguageCode(uint8 languageCode) = 0;

    /*! Set day & night theme for LayerProvider.
      @return None
    */
    virtual void SetTheme(MapViewTheme theme) = 0;

    /*! Set retry time of Metadata.

        @return None.
    */
    virtual void SetMetadataRetryTimes(int number) = 0;

    /*! Set persistent metadata path.

        This path will be used to store serialized metadata.

        @return NE_OK if succeeded.
    */
    virtual NB_Error SetPersistentMetadataPath(shared_ptr<string> metadataPath) = 0;

    /*! Checks whether metadata is changed.
        invoke the CheckMetadataChanges function of metadataprocessor.
        @return None.
    */
    virtual void
    CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback,     /*!< callback to be invoked */
                         shared_ptr<MetadataConfiguration> config,      /*!< Configuration used to retrieve metadtadata from server.
                                                                            Set it to empty to use default values. */
                         bool skipTimeInterval,                          /*!< Flag, when set to true, will
                                                                            check with server no matter the
                                                                            tile stamp of last sync time.  */
                         bool forceUpdate                                /*!< Flag to force update */
                         );

    /*! Register a Listener into UnifiedLayerProvider.

        @return None.
    */
    void
    RegisterUnifiedLayerListener(UnifiedLayerListener* listener, /*!< listener to be registered. */
                                 bool higherPriority /*!< Flag to indicate if the listener has
                                                          higher priority. The listener will be
                                                          place at the beginning of listener queue
                                                          if it has higher priority. Listeners
                                                          registered with higher priority get
                                                          notified earlier than the ones with lower
                                                          priority */
                                  );

    /*! Unregister a Listener into UnifiedLayerProvider.

      @return None.
    */
    void UnregisterUnifiedLayerListener(UnifiedLayerListener* listener /*!< listener to be
                                                                                unregistered. */
                                       );

    inline nbcommon::HybridMode GetOnOffBoardMode() const {return m_onoffBoardMode;}
protected:
    /*! Invoke callbacks in registered UnifiedLayerListener.

        @return None.
    */
    void NotifyLayersUpdated(const vector<LayerPtr>& newLayers);
    nbcommon::HybridMode m_onoffBoardMode;               /*!< onboard or offboard */
private:
    list<UnifiedLayerListener*> m_pListeners;       /*!< Listeners register to this provider */
};

typedef shared_ptr<LayerProvider > LayerProviderPtr;
}
#endif
/*! @} */
