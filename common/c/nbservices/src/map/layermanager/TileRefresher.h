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
   @file        TileRefresher.h
   @defgroup    nbmap

   Description: TileRefresher watches tiles that needs to be refreshed
   frequently (such as tiles from Traffic Layer). It will scan tiles stored in
   the cache, and notify the listeners if some tiles are out dated.

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

#ifndef _TILEREFRESHER_H_
#define _TILEREFRESHER_H_


extern "C"
{
#include "nbcontext.h"
}
#include <map>
#include <set>
#include "TileKey.h"
#include "Tile.h"
#include "TileInternal.h"

namespace nbmap
{
/*! The index saved the information used to refresh the tile */
class RefreshTileIndex : public Base
{
public:
    // Public functions .........................................................................

    /*! RefreshTileIndex constructor */
    RefreshTileIndex(TileKeyPtr tileKey,                /*!< Tile key of the tile */
                     shared_ptr<string> contentId, /*!< Content ID of the tile */
                     shared_ptr<string> cachingName/*!< Caching name of the tile */
                     );

    /*! RefreshTileIndex destructor */
    virtual ~RefreshTileIndex();


    // Public members ...........................................................................

    TileKeyPtr m_tileKey;                   /*!< Tile key of the tile */
    shared_ptr<string> m_contentId;    /*!< Content ID of the tile */
    shared_ptr<string> m_cachingName;  /*!< Caching name of the tile */


private:
    // Private functions ........................................................................

    // Copy constructor and assignment operator are not supported.
    RefreshTileIndex(const RefreshTileIndex& index);
    RefreshTileIndex& operator=(const RefreshTileIndex& index);
};

typedef shared_ptr<RefreshTileIndex> RefreshTileIndexPtr;

class TileExpireListener
{
public:
    TileExpireListener();
    virtual ~TileExpireListener();

    /*! Tell listener that some tiles expired.

        The listener may or may not be interested in these tile keys.

        @return None.
    */
    virtual void TilesExpired(const vector<TileKeyPtr>& tileKeys) = 0;

private:
    TileExpireListener(const TileExpireListener& instance);
    TileExpireListener& operator=(const TileExpireListener& instance);
};

class TileRefresher : public Base
{
public:
    TileRefresher(TileLayerInfoPtr info, NB_Context* context);
    virtual ~TileRefresher();

    /*! Register TileExpireListener to TileRefresher.

        @return None.
    */
    void RegisterListener(TileExpireListener* listener);

    /*! Unregister TileExpireListener from TileRefresher.

        @return None.
    */
    void UnregisterListener(TileExpireListener* listener);

    /*! Add a tile for TileRefresher to watch.

        @return NB_Error
    */
    NB_Error
    AddTileToWatch(TilePtr tile);

    /*! Set refresh interval of this refresher.

      @return void
    */
    void SetRefreshInterval(uint32 secondsInterval);

private:
    // Private functions ...................................................................

    // Copy constructor and assignment operator are not supported.
    TileRefresher(const TileRefresher& instance);
    TileRefresher& operator=(const TileRefresher& instance);

    void StartRefreshTimer();

    static void RefreshTimerCallback(PAL_Instance *pal,
                                     TileRefresher* layer,
                                     PAL_TimerCBReason reason);
    /*! Checks outdated tiles, and notify listener.

        @return None.
    */
    void CheckOutdatedTiles();

    // Private member fields ...............................................................
    set<TileExpireListener*> m_listeners;/*!< Listeners who will be notified when
                                                   tiles expire. */

    bool        m_timerActive;          /*!< Flag to identify if the refresh timer is active */
    uint32      m_secondsInterval;      /*!< Interval (in seconds) used to update tiles . */
    NB_Context* m_pContext;             /*!< Pointer of NB_Context instance. */

    TileLayerInfoPtr m_tileLayerInfo;

    typedef map<TileKey, RefreshTileIndexPtr> TILE_REFRESH_MAP;

    /*!< Tile indexes associated with the tiles contained valid data. These tile indexes
      will be moved to m_expiredTileIndexes when the expired timer is fired. */
    TILE_REFRESH_MAP m_validTileIndexes;

    /*!< Tile indexes will be expired when the timer is fired. The tiles associated with
      these tile indexes will be removed from cache */
    TILE_REFRESH_MAP m_expiredTileIndexes;
};

typedef shared_ptr<TileRefresher> TileRefresherPtr;

}

#endif /* _TILEREFRESHER_H_ */

/*! @} */
