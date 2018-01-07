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
@file     CustomLayerManager.h
@defgroup nbmap

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef __CUSTOM_LAYER_MANAGER__
#define __CUSTOM_LAYER_MANAGER__

#include "paltypes.h"
#include "nbexp.h"
#include "navpublictypes.h"
#include "smartpointer.h"
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "TileProvider.h"
#include "CustomNBMTile.h"

namespace nbmap
{

// Seperator between layer ID and pin ID
//const char ID_SEPERATOR[] = ":";

class CustomLayer;

/*! Provides a definition of a PinManager within mapview. Allows a user to
    add and remove Pin Layers.
*/
class CustomLayerManager
{
public:

    CustomLayerManager() {}

    virtual ~CustomLayerManager() {}

    /*! Add a Custom Layer

        This function will allow the user to add a Custom Layer to the Custom Layer Manager.
        Creates a Custom Layer and returns a handle ID.

        @param provider
        @param mainOrder Custom layer draw order.
        @param subOrder Custom layer sub draw order.
        @param refZoom reference grid level.
        @param minZoom min zoom level of this layer.
        @param maxZoom max zoom level of this layer.
        @param visible set the layer visible or not.
        @return handle ID
    */
    virtual uint32 AddCustomLayer(TileProviderPtr provider, int mainOrder, int subOrder, int refZoom, int minZoom, int maxZoom, bool visible) = 0;

    /*! Remove a Custom Layer

        Removes the layer to sepcified by the layer ID.

        @param layerID The handle ID of this layer.
        @return None
    */
    virtual void RemoveCustomLayer(int layerID) = 0;

    /*! Set file path of material config information

        This file is a XML file contained material config information.

        @return None
    */
    virtual void SetMaterialConfigPath(shared_ptr<std::string> materialConfigPath) = 0;

    /*! Get the coordinates of all pins across all custom layers

        This function will check if the custom layers are enabled internal and only return
        the coordinates of the pin layers which are enabled.

        @return A vector containing the coordinates of all pins across all custom layers
    */
    virtual shared_ptr<std::vector<NB_LatitudeLongitude> > GetAllPinCoordinates() = 0;

    /*! Remove all the pins.
        @return None
    */
    virtual void RemoveAllPins() = 0;

    /*! Adding the custom tile data via user notify the cutom layer manager*/
    virtual void LoadTile(uint32 layerID, CustomNBMTilePtr tile) = 0;

    /*! Get the user marker ID by the internal marker ID that create by TileFactory
        @param internalMarkerID(INPUT) create by tile factory.
        @param userMarkerID(OUTPUT) create by user.
        @return true, meaning that the marker ID exist, user can get it from 'userMarkerID' param.
    */
    virtual bool GetMarkerID(const std::string& internalMarkerID, int& userMarkerID) = 0;

    /*! Return a CustomLayer whose layer ID is layerID;

        @return shared pointer of layer instance.
    */
    virtual shared_ptr<CustomLayer> GetCustomLayer(uint32 layerID) = 0;

    /*! Make the custom layer visible or not*/
    virtual void SetVisible(uint32 layerID, bool isVisible) = 0;
};

typedef shared_ptr<CustomLayerManager> CustomLayerManagerPtr;

}

#endif
