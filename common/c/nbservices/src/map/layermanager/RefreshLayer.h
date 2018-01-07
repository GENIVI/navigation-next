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

    @file       RefreshLayer.h

    Class RefreshLayer inherits from class UnifiedLayer. A RefreshLayer
    refreshes tiles int the specified interval.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef REFRESHLAYER_H
#define REFRESHLAYER_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "nbcontext.h"
}

#include "base.h"
#include "smartpointer.h"
#include "TileManager.h"
#include "UnifiedLayer.h"
#include "TileRefresher.h"
#include <map>
#include <utility>
#include <set>
#include <vector>

namespace nbmap
{

// Types ........................................................................................

class RefreshLayer;

/*! */
class RefreshLayerListener
{
public:
    // Interface Functions ......................................................................

    /*! Notify to update tiles of a refresh layer

        @return None
    */
    virtual void
    RefreshTiles(const vector<TileKeyPtr>& tileKeys,   /*!< Tile keys to refresh */
                 Layer* layer                               /*!< Specified layer to refresh tiles */
                 ) = 0;


    // Public functions .........................................................................

    /*! RefreshLayerListener destructor */
    virtual ~RefreshLayerListener();
};

/*! The layer used to refreshes tiles int the specified interval */
class RefreshLayer : public UnifiedLayer,
                     public TileExpireListener
{
public:
    // Public functions .........................................................................

    /*! RefreshLayer constructor */
    RefreshLayer(TileManagerPtr tileManager,    /*!< A tile manager used for the layer */
                 uint32 layerIdDigital,         /*!< A digital layer ID */
                 NB_Context* context            /*!< Pointer to current context */
                 );

    /*! RefreshLayer constructor */
    RefreshLayer(TileManagerPtr tileManager,    /*!< A tile manager used for the layer */
                 uint32 layerIdDigital,         /*!< A digital layer ID */
                 uint32 secondsInterval,        /*!< Interval used to update tiles. The unit is second. */
                 NB_Context* context            /*!< Pointer to current context */
                 );

    /*! RefreshLayer destructor */
    virtual ~RefreshLayer();

    /*! Allocates new RefreshLayer and assign new layerId to it.

        @return Pointer of newly created RefreshLayer.
    */
    virtual UnifiedLayerPtr Clone();


    /*! Register a listener

        @return None
    */
    void
    RegisterListener(RefreshLayerListener* listener     /*!< A listener to register */
                     );

    /*! Unregister a listener

        User should always unregister the listener when the object of listener is destroyed.

        @return None
    */
    void
    UnregisterListener(RefreshLayerListener* listener   /*!< A listener to unregister */
                       );

    /*! Add a valid tile

        @return NE_OK if success
    */
    NB_Error
    AddValidTile(TilePtr tile       /*!< A valid tile to add */
                 );

    /*! Check whether this layer is created by OffboardLayerProvider.

        @return true if so.
    */
    bool IsCreatedByOffboardLayerManager();

    // See description in TileRefresher.h
    virtual void TilesExpired(const vector<TileKeyPtr>& tileKeys);

    /*! Copy constructor */
    RefreshLayer(const RefreshLayer& layer);

    virtual std::string className() const { return "RefreshLayer"; }

private:
    // Private functions ........................................................................
    // assignment operator are not supported.
    RefreshLayer& operator=(const RefreshLayer& layer);

    /* See source file for description */
    void RefreshTiles();

    /* See description in Layer.h */
    virtual void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority, bool cachedOnly = false);

    virtual void ProcessSpecialCharacteristicsOfLayer();

    // Private members ..........................................................................
    TileRefresherPtr m_tileRefresher;
    set<RefreshLayerListener*> m_listeners;                /*!< A set of listeners */
    shared_ptr<bool> m_thisValid;       /*!< The flag identified if this object is valid. It is
                                          initialized to true and set to false in
                                          RefreshLayer destructor. */
    bool m_createdByOffboardLayerProvider;    /*!< Flag to indicate whether this instance is
                                                   created by RouteManager or not. */
};

typedef shared_ptr<RefreshLayer> RefreshLayerPtr;

};  // namespace nbmap

/*! @} */

#endif  // REFRESHLAYER_H
