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
@file     Layer.h
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

#ifndef __LAYER__
#define __LAYER__

#include "paltypes.h"
#include "nbexp.h"
#include "smartpointer.h"
#include "Tile.h"
#include "TileKey.h"
#include "AsyncCallback.h"
#include <iostream>
#include <map>
#include <vector>
#include <string>

using namespace std;

namespace nbmap
{

/*! Provides a definition of a layer within mapview. Allows a client of mapview to get displayable name
and toggle the display of the layer.
*/
class Layer
{
public:

    Layer() {}

    virtual ~Layer() {}

    /*! Draw order for the layer

        Specifces the order in which the available layers should
        be drawn. Each layer has a particular draw order associated with it.
        Clients should display layers based on this draw order.

        @todo Remove this function from interface after MapView updated.
        @return draw order.
     */
    virtual uint32 GetDrawOrder() = 0;

    /*! Make this layer displayable

        @param true to enable the layer and false to disable the layer.
    */
    virtual void SetEnabled(bool isEnabled) = 0;

    /*! Data type of the tile for this layer

        @return A string of data type
    */
    virtual shared_ptr<std::string> GetTileDataType() = 0;

    /*! Minimum zoom level for this layer

       Gives the minimum zoom level at which this
       layer is available
    */
    virtual uint32 GetMinZoom() = 0;

    /*! Maximum zoom level for this layer

       Gives the maximum zoom level for this layer
    */
    virtual uint32 GetMaxZoom() = 0;

    /*! Get reference tile grid level

        @return Reference tile grid level
    */
    virtual uint32 GetReferenceTileGridLevel() = 0;

    /*! Get the index within the LAM (layer availability matrix) bitmask for this layer

        @return LAM index
    */
    virtual uint32 GetLayerAvailabilityMatrixIndex() = 0;

    /*! Cancel all tile request of the layer

        If parameter 'clearCache' is true, this function removes all tiles of this layer
        from cache.

        @return None
    */
    virtual void RemoveAllTiles() = 0;

    /*! Get LayerID of integer form.

        @return ID of this layer.
    */
    virtual uint32 GetID() const = 0;

    /*! Check whether this layer is enabled or not.

        @return true if enabled.
    */
    virtual bool IsEnabled() const = 0;

    /*! Check whether this is overlay layer.

        @return true if it is.
    */
    virtual bool IsOverlay() = 0;

    /*! Return draw order of lable.

        @return label draw order of this layer.
    */
    virtual uint32 GetLabelDrawOrder() = 0;

    /*! Check if this layer is raster layer.

        If it is not a raster layer, tiles from this layer should be applied to all layers
        whose zoom level is between Layer::GetMinZoom() & Layer::GetMaxZoom() without
        redownloading tiles. But if it is raster layer, then when zoom level changed but
        current zoom level is between Layer::GetMinZoom() & Layer::GetMaxZoom(), we need to
        download new tiles from server.

        @return true if it is.
    */
    virtual bool IsRasterLayer() = 0;

    virtual shared_ptr<string> GetLayerName() const = 0;

    virtual string str() const = 0;
};


typedef shared_ptr<Layer > LayerPtr;

}
#endif
/*! @} */
