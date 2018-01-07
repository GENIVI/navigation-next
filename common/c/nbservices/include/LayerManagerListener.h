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
@file     LayerManagerListener.h
@defgroup nbmap

*/
/*
(C) Copyright 2012 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __LAYER_MANAGER_LISTENER__
#define __LAYER_MANAGER_LISTENER__

#include <vector>
#include "smartpointer.h"
#include "ExternalMaterial.h"

namespace nbmap
{

class TileKey;
class Layer;
class Pin;
class MapLegendInfo;

/*!
    This class provides a way for clients of Layer Manager to
    be notified when they need to update tiles and pins.
*/
class LayerManagerListener
{

public:

    LayerManagerListener() {}

    virtual ~LayerManagerListener() {}

    /*! This function will be called when
        Layer Manager wants to notify that the users of
        Layer Manager should update all tiles.

        @return None
    */
    virtual void
    RefreshAllTiles() = 0;

    /*! This function will be called when
        Layer Manager wants to notify that the users of
        Layer Manager should update tiles of specifed layer.

        @todo: The parameter 'layer' could be replaced with a layer ID. But I want to
               use this layer pointer in 'ignoreList' of LayerManager::GetTiles.

        @return None
    */
    virtual void
    RefreshTilesOfLayer(shared_ptr<Layer> layer     /*!< Specified layer to refresh tiles */
                        ) = 0;


    /*! This function will be called when
        Layer Manager wants to notify that the users of
        Layer Manager should update tiles of specified layer.

        @todo: The parameter 'layer' could be replaced with a layer ID. But I want to
               use this layer pointer in 'ignoreList' of LayerManager::GetTiles.

        @return None
    */
    virtual void
    RefreshTiles(const std::vector<shared_ptr<TileKey> >& tileKeys, /*!< Tile keys to refresh */
                 shared_ptr<Layer> layer                            /*!< Specified layer to refresh tiles */
                 ) = 0;

    /*! This function will be called when layer manager wants to notify the user of
        layer manager that layers in layer provider is updated, and user should clear all
        existing requests and use these new layers.

        @return None
    */
    virtual void
    LayersUpdated(const vector<shared_ptr<Layer> >& layers  /*!< Updated layers */
                  ) = 0;

    /*! Notify listener that new layers are added.

        @return None.
    */
    virtual void
    LayersAdded(const std::vector<shared_ptr<Layer> >& layers       /*!< Newly added layers */
                ) = 0;

    /*! Notify listener that some layers are removed.

        @return None.
    */
    virtual void
    LayersRemoved(const std::vector<shared_ptr<Layer> >& layers     /*!< Newly removed layers */
                  ) = 0;


    /*! Notify Listener that an animation layer is enabled.

        The listener will get a shared pointer to the animation layer
        and the initial frame list. It is the listeners responsibilty
        to call GetAnimationTiles using the layer pointer and the frame list.

        @return None.

    */
    virtual void
    AnimationLayerAdded(shared_ptr<Layer> layer /*!<animation layer>*/
                        ) = 0;


    /*! Notify Listener that frame list associated with the animation layer is updated.

        When the framelist associated with the animation layer gets updated. Layer Manager
        will notify its listeners via this interface. The layer pointer will be used to identify
        which animation layer the framelist belongs to. Its the clients responsibity to update the
        tiles. Clients are expected to diff the new framelist with the old one and ask layer manager
        to fetch the new tiles using GetAnimationTiles interface on layer manager.

        @return None.

    */
    virtual void
    AnimationLayerUpdated(shared_ptr<Layer> layer, /*!<Shared pointer to the animation layer>*/
                          const vector<uint32>& frameList /*!<Each int represents a time stamp>*/
                         ) = 0;

    /*! Notify Listener that animation layer has been removed.

        Listener should unload all tiles associated with this layer.

        @return None
     */
    virtual void
    AnimationLayerRemoved(shared_ptr<Layer> layer/*!<Shared pointer to the animation layer>*/
                          ) = 0;

    /*! Notify listener that pins are removed.

        TRICKY: This function is only used to remove the pin bubble when pins are removed so far.

        @return None
    */
    virtual void
    PinsRemoved(shared_ptr<std::vector<shared_ptr<Pin> > > pins     /*!< The removed pins */
                ) = 0;

    /*! Notify listener to enable or disable map legend.

        @todo: Change data to proper data struct.
        @return None.
    */
    virtual void
    EnableMapLegend(bool enabled, shared_ptr <MapLegendInfo> mapLegend) = 0;

    /*! Notify listener to add/remove/active Material. */
    virtual void MaterialAdded(const ExternalMaterialPtr& /*material*/){}
    virtual void MaterialRemoved(const ExternalMaterialPtr& /*material*/){}
    virtual void MaterialActived(const ExternalMaterialPtr& /*material*/){}

    /*! This function will be called when Layer Manager wants to notify that the users
      of Layer Manager should reload tiles of specifed layer.

      @return None
    */
    virtual void
    ReloadTilesOfLayer(shared_ptr<Layer> /*layer*/){}
};

}

#endif  // __LAYER_MANAGER_LISTENER__

/*! @} */
