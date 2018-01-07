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
    @file       AnimationLayer.h

    Class AnimationLayer inherits from class UnifiedLayer. Class
    AnimationLayer is used to request the frames of animation tiles.
    The current frame list saved in the AnimationLayer is used to
    request all frames of the requested tile keys when the function
    GetTiles is called. If the saved frame list is empty, call the
    function GetFrameList of class FrameListLayer to get the frame
    list. If the function GetFrameList returns NE_OK, use the got
    frame list to request tiles, if it returns NE_BUSY, save the
    requested tile keys and wait for the function FrameListUpdated of
    class FrameListListener called to request tiles.

    If the frame list is updated from class FrameListListener, the
    AnimationLayer cancels all requests which are downloading in
    progress and notify to refresh all tiles of this layer through
    the AnimationLayerListener.
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

#ifndef ANIMATIONLAYER_H
#define ANIMATIONLAYER_H

/*!
    @addtogroup nbmap
    @{
*/

#include "smartpointer.h"
#include "UnifiedLayer.h"
#include <set>
#include <string>
#include <vector>
#include "NBProtocolAnimationInfo.h"
#include "NBProtocolMapLegend.h"
#include "MapLegendInfo.h"

namespace nbmap
{
// Types ........................................................................................

class FrameListLayer;
class FrameListEntry;
class TileManager;
class MapLegendInfoImpl;

/*! The layer used to request the frames of animation tiles */
class AnimationLayer : public UnifiedLayer
{
public:
    // Public functions .........................................................................

    /*! AnimationLayer constructor */
    AnimationLayer(NB_Context* context,                 /*!< Pointer to current context */
                   uint32 digitalId,                    /*!< A digital layer ID */
                   shared_ptr<TileManager> tileManager  /*!< Tile manager to request the frame list */
                   );

    /*! AnimationLayer destructor */
    virtual ~AnimationLayer();

    /* See description in UnifiedLayer.h

       @todo: I will fix the type of the parameter tileKeys to const reference in UnifiedLayer.
    */
    virtual void GetTiles(std::vector<TileKeyPtr> tileKeys,
                          AsyncTileRequestWithRequestPtr callback,
                          uint32 priority);
    virtual shared_ptr<UnifiedLayer> Clone();

    /* See description in FrameListListener.h */
    void FrameListUpdated(shared_ptr<map<uint32, shared_ptr<FrameListEntry> > > frameList);
    void FrameListFailed(NB_Error error);

    /*! Request the tiles by the tile keys and timestamps

        @return NE_OK if success
    */
    void
    GetAnimationTiles(const std::vector<TileKeyPtr>& tileKeys,      /*!< Tile keys used to request tiles */
                      const std::vector<uint32>& timestamps,        /*!< Timestamps for each tile. If the vector is empty,
                                                                         all timestamps for each tile will be requested. */
                      shared_ptr<AsyncCallback<TilePtr> > callback, /*!< Callback used to return the tiles */
                      uint32 priority                               /*!< Priority of the tile requests */
                      );

    /*! Set the frame list layer

        This function saves a frame list layer and register a listener of this frame list
        layer. The frame list layer is used to get the frame list in the AnimationLayer.

        @return None
    */
    void
    SetFrameListLayer(shared_ptr<FrameListLayer> frameListLayer
                      );

    void SetMapLegend(shared_ptr<protocol::MapLegend> mapLegend);

    shared_ptr <MapLegendInfo> GetMapLegendInfo();

    void SetAnimationInfo(shared_ptr<protocol::AnimationInfo> animationInfo);

    shared_ptr<string> GetFrameListLayerID();

    /*! AnimationLayer copy constructor */
    AnimationLayer(const AnimationLayer& layer);

    virtual std::string className() const { return "AnimationLayer"; }

private:
    // Private types ............................................................................

    /*! The entry used to save the pending tile request waiting for the frame list updated */
    class PendingRequestEntry : public Base
    {
    public:
        /*! PendingRequestEntry constructor */
        PendingRequestEntry(const std::vector<TileKeyPtr>& tileKeys,                                /*!< Pending requested tile keys */
                            shared_ptr<AsyncCallback<TilePtr> > callback,    /*!< Callback for the tile request */
                            uint32 priority                                                         /*!< Priority of the tile request */
                            );

        /*! PendingRequestEntry destructor */
        virtual ~PendingRequestEntry();

        std::vector<TileKeyPtr> m_tileKeys;                                     /*!< Pending requested tile keys */
        shared_ptr<AsyncCallback<TilePtr> > m_callback;  /*!< Callback for the tile request */
        uint32 m_priority;                                                      /*!< Priority of the tile request */

    private:
        // Copy constructor and assignment operator are not supported.
        PendingRequestEntry(const PendingRequestEntry& entry);
        PendingRequestEntry& operator=(const PendingRequestEntry& entry);
    };

    typedef shared_ptr<PendingRequestEntry> PendingRequestEntryPtr;


    // Private functions ........................................................................

    // Assignment operator are not supported.
    AnimationLayer& operator=(const AnimationLayer& layer);

    /* See source file for description */
    void NotifyErrorForTileKeys(NB_Error error,
                                shared_ptr<AsyncCallback<TilePtr> > callback,
                                std::vector<TileKeyPtr> tileKeys);

    // Private members ..........................................................................

    shared_ptr<bool> m_thisValid;                               /*!< The flag identified if this object is valid.
                                                                     It is initialized to true and set to false in
                                                                     the destructor of class AnimationLayer. */
    shared_ptr<FrameListLayer> m_frameListLayer;                /*!< Frame list layer used to get the frame list */
    shared_ptr<map<uint32, shared_ptr<FrameListEntry> > > m_frameList;          /*!< Frame list got from the FrameListLayer. This
                                                                     shared pointer is used to identify if the
                                                                     frame list is valid. If the shared pointer is
                                                                     not empty (even if the vector is empty), the
                                                                     frame list is valid, otherwise it is not valid. */
    std::vector<PendingRequestEntryPtr> m_pendingRequests;      /*!< Pending tile request waiting for the frame list
                                                                     updated */
    shared_ptr<protocol::MapLegend> m_pMapLegend;     /*!< MapLegend  returned from server */
    shared_ptr<MapLegendInfoImpl>   m_pMapLegendInfo; /*!< Map legend info converted from
                                                           m_pMapLegend,  can be used out of
                                                           nbservices */
    shared_ptr<protocol::AnimationInfo> m_pAnimationInfo;

};

typedef shared_ptr<AnimationLayer> AnimationLayerPtr;

};  // namespace nbmap

/*! @} */

#endif  // ANIMATIONLAYER_H
