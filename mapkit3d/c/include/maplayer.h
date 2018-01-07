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
 @file     maplayer.h
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

#ifndef MAPLAYER_H_
#define MAPLAYER_H_

#include "smartpointer.h"
#include "mapcontroller.h"
#include "maptile.h"

namespace nbmap
{

/*! a simple wrapper class to control custom layers
 */
class MapLayer
{
public:
    /**
     * Checks whether the layer is visible.
     * This method must be called on the main thread.
     *
     * @return True if the layer is visible; false otherwise.
     */
    bool IsVisible()const;

    /**
     * Sets the visibility of the layer.
     * This method must be called on the main thread.
     *
     * @param visible True to make this layer visible.
     * @return None.
     */
    void SetVisible(bool visible);

    /**
     * Notifies the map the tile in this specified location is invalidate.
     * The map will retrieve this tile again.
     * This method must be called on the main thread.
     *
     * @param x The x coordinate of the tile. This will be in the range [0, 2^zoom - 1] inclusive.
     * @param y The y coordinate of the tile. This will be in the range [0, 2^zoom - 1] inclusive.
     * @param zoom The zoom level of the tile. This will be in the range (2, 22] inclusive.
     * @return None.
     */
    void Invalidate(int x, int y, int zoom);

    /**
     * Notifies the map the tiles of this specified zoom lever are invalidate.
     * The map will retrieve the tiles of this zoom level again.
     * This method must be called on the main thread.
     *
     * @param zoom The zoom level of the tile. This will be in the range (2, 22] inclusive.
     * @return None.
     */
    void Invalidate(int zoom);

    /**
     * Notifies the map all the custom tiles are invalidate.
     * The map will retrieve all the custom tiles again.
     * This method must be called on the main thread.
     *
     * @return None.
     */
    void Invalidate();

    /**
     * Gets the id.
     * This method must be called on the main thread.
     *
     * @return The id of this layer.
     */
    uint32 GetLayerId() const;

    /**
     * Sets the id.
     * This method must be called on the main thread.
     * @param id
     * @return None.
     */
    void SetLayerId(uint32 id);

    /**
     * Loads a custom nbm tile.
     * This method must be called on the main thread.
     *
     * @param tile
     * @return None.
     */
    void LoadTile(MapTile* tile);

protected:
    MapLayer(PAL_Instance& pal, MapController& controller, bool visible);
    ~MapLayer();

private:
    static void LoadTileFunc(PAL_Instance* pal, void* userData);
    static void SetVisibleFunc(PAL_Instance* pal, void* userData);
    static void InvalidateFunc(PAL_Instance* pal, void* userData);
    static void InvalidateZoomFunc(PAL_Instance* pal, void* userData);
    static void InvalidateAllFunc(PAL_Instance* pal, void* userData);

    PAL_Instance &m_pal;
    MapController &m_mapController;
    uint32 m_layerId;
    bool m_visible;

    friend class MapController;
};

}

#endif /* MAPLAYER_H_ */
