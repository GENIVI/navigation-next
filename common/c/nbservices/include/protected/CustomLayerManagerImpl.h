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

    @file       CustomLayerManagerImpl.h

    Class CustomLayerManagerImpl inherits from CustomLayerManager interface.

    @todo: Add description of this class.

*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef CUSTOMLAYERMANAGERIMPL_H
#define CUSTOMLAYERMANAGERIMPL_H

extern "C"
{
}

#include "base.h"
#include "CustomLayerManager.h"
#include "LayerManager.h"

namespace nbmap
{

// Types ........................................................................................

class CustomLayer;
class LayerIdGenerator;
class LayerProvider;
class Pin;

/*! */
class CustomLayerManagerImpl : public Base,
                               public CustomLayerManager
{
public:
    // Public functions .........................................................................

    /*! CustomLayerManagerImpl constructor
     *
     *  Will create and Initialize all variables.
     */
    CustomLayerManagerImpl(NB_Context* context,
                           shared_ptr<LayerProvider> layerProvider,
                           LayerManagerPtr layerManager,
                           shared_ptr<LayerIdGenerator> idGenerator);

    /*! Light weight constructor
     *
     *  Will create CustomLayerManagerImpl, but will not fully initialized it.
     */
    CustomLayerManagerImpl(NB_Context* context);

    /*! CustomLayerManagerImpl destructor */
    virtual ~CustomLayerManagerImpl();

    /* See description in PinManager.h */
    virtual uint32 AddCustomLayer(TileProviderPtr provider, int mainOrder, int subOrder, int refZoom, int minZoom, int maxZoom, bool visible);
    virtual void RemoveCustomLayer(int layerID);

    virtual void SetMaterialConfigPath(shared_ptr<std::string> materialConfigPath);
    virtual shared_ptr<std::vector<NB_LatitudeLongitude> > GetAllPinCoordinates();
    virtual void RemoveAllPins();
    virtual void LoadTile(uint32 layerID, CustomNBMTilePtr tile);
    virtual bool GetMarkerID(const std::string& internalMarkerID, int& userMarkerID);
    virtual shared_ptr<CustomLayer> GetCustomLayer(uint32 layerID);
    virtual void SetVisible(uint32 layerID, bool isVisible);

    void Initialize(shared_ptr<LayerProvider> layerProvider,
                    LayerManagerPtr layerManager,
                    shared_ptr<LayerIdGenerator> idGenerator);

    /*! Notify to update all pins of specified custom layer

        @return None
    */
    void
    RefreshAllPinsOfLayer(uint32 layerID       /*!< Custom layer to refresh pins */
                          );

    /*! Notify to update a tile of specified custom layer

        @return None
    */
    void
    RefreshTilesOfLayer(const std::vector<TileKeyPtr>& tileKeys,/*!< Tile keys to refresh */
                        uint32 layerID         /*!< Custom layer to refresh tiles */
                        );

    /*! Notify that the pins are removed

        @return None
    */
    void
    NotifyPinsRemoved(shared_ptr<std::vector<shared_ptr<Pin> > > pins /*!< The removed pins */
                      );

private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    CustomLayerManagerImpl(const CustomLayerManagerImpl& manager);
    CustomLayerManagerImpl& operator=(const CustomLayerManagerImpl& manager);

    // Private members ..........................................................................
    int m_currentLayerIdDigital;
    NB_Context*      m_pContext;                            /*!< NB_Context instance. */
    LayerManagerPtr  m_pLayerManager;
    shared_ptr<LayerProvider> m_layerProvider;              /*!< A layer provider */

    shared_ptr<LayerIdGenerator> m_pLayerIdGenerator;
    shared_ptr<std::string>      m_materialConfigPath;      /*!< Path of a XML file contained material
                                                                 config information */
    std::map<uint32, shared_ptr<CustomLayer> > m_layerMap;  /*!< A map contained all custom layers.
                                                                 Layer IDs are keys of this map.*/
};

}  // namespace nbmap

/*! @} */

#endif  // PINMANAGERIMPL_H
