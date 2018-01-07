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
    @file       FrameListLayer.h

    Class FrameListLayer inherits from class UnifiedLayer. Class
    FrameListLayer is used to parse the frame IDs from the downloaded
    data and save the frame list in memory. The refresh time got from
    the HTTP header is used to request the frame list after this specific
    interval.

    If the frame list has not been requested yet, or it is downloading
    in progress, the function GetFrameList returns NE_BUSY to tell
    the caller to wait for the callback of FrameListListener.
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

#ifndef FRAMELISTLAYER_H
#define FRAMELISTLAYER_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "paltimer.h"
}

#include <set>
#include <string>
#include <vector>
#include "TileRefresher.h"
#include "UnifiedLayer.h"
#include "smartpointer.h"

namespace nbmap
{
// Types ........................................................................................

class TileManager;
class FrameListListener;
class FrameListTileCallback;
class FrameListEntry;

typedef shared_ptr<FrameListEntry>     FrameListEntryPtr;
typedef map<uint32, FrameListEntryPtr> FrameListMap;
typedef shared_ptr<FrameListMap>       FrameListMapPtr;

/*! The layer used to request the frame list for animation */
class FrameListLayer : public UnifiedLayer,
                       public TileExpireListener
{
public:
    // Public functions .........................................................................

    /*! FrameListLayer constructor */
    FrameListLayer(NB_Context* context,                 /*!< Pointer to current context */
                   uint32 digitalId,                    /*!< A digital layer ID */
                   shared_ptr<TileManager> tileManager  /*!< Tile manager to request the frame list */
                   );

    /*! FrameListLayer destructor */
    virtual ~FrameListLayer();

    /* See description in UnifiedLayer.h

       @todo: I will fix the type of the parameter tileKeys to const reference in UnifiedLayer.
    */
    virtual void GetTiles(vector<TileKeyPtr> tileKeys,
                          AsyncTileRequestWithRequestPtr callback,
                          uint32 priority);
    virtual UnifiedLayerPtr Clone();

    // See description in TileRefresher.h
    virtual void TilesExpired(const vector<TileKeyPtr>& tileKeys);

    /*! Get the frame list

        This function directly returns the frame list if it is valid, otherwise
        this function will try to request the frame list and user needs to wait
        for the callback of FrameListListener. It is identified by the returned
        value.

        @return NE_OK if the frame list is valid,
                NE_BUSY if the frame list has not been requested yet, or it is
                        downloading in progress. User needs to wait for the
                        callback of FrameListListener.
    */
    virtual NB_Error
    GetFrameList(FrameListMapPtr& frameList   /*!< On return the latest frame list */
                 );

    /*! Update the tile of frame list

        This function is called by the callback to request the frame list. The
        data is parsed to a vector of frame IDs and saved internal.

        @return None
    */
    void
    TileRequestSuccess(TilePtr tile         /*!< Tile of frame list to update */
                     );

    /*! Notify updating the frame list failed

        This function is called by the callback to request the frame list.

        @return None
    */
    void
    TileRequestError(NB_Error error                      /*!< Failed error value */
                     );

    /*! Register a listener

        @return None
    */
    void
    RegisterListener(FrameListListener* listener        /*!< A listener to register */
                     );

    /*! Unregister a listener

        User should always unregister the listener when the object of listener is destroyed.

        @return None
    */
    void
    UnregisterListener(FrameListListener* listener      /*!< A listener to unregister */
                       );

    virtual std::string className() const { return "FrameListLayer"; }

protected:

    /*! Notify all listeners to update frame list. */
    void NotifyFrameListUpdated();

    /*! Notify all listeners about error when requesting FrameList. */
    void NotifyFrameListError(NB_Error error);

    // Copy constructor.
    FrameListLayer(const FrameListLayer& layer);

    /*!< Frame list saved in memory.

      This shared pointer is used to identify if the frame list is valid. If the shared
      pointer is not empty (even if the vector is empty), the frame list is valid,
      otherwise it is not valid.

      It uses the timeStamp of the entry as key.
    */
    FrameListMapPtr m_frameList;

    shared_ptr<bool> m_thisValid;     /*!< A flag identified if this object is valid. */

private:
    // Private functions ........................................................................

    // Assignment operator are not supported.
    FrameListLayer& operator=(const FrameListLayer& layer);

    /* See source file for description */

    uint8* FindStringInMemory(const char* string,
                              uint8* begin,
                              uint32 size);
    NB_Error RequestFrameList();
    void UpdateFrameList(FrameListMap& newFrameList);
    bool IsFrameListEqual(const FrameListMap& frameList1,
                          const FrameListMap& frameList2) const;
    // Private members ..........................................................................

    bool m_requestInProgress;                            /*!< Flag to identify if the frame list is
                                                              requesting in progress */
    shared_ptr<FrameListTileCallback> m_tileCallback;    /*!< Callback for the request of the frame list */
    set<FrameListListener*> m_listeners;            /*!< A set of listeners */

    TileRefresherPtr m_tileRefresher; /*!< TileRefresher controls when to refresh tiles. */
};

typedef shared_ptr<FrameListLayer> FrameListLayerPtr;

/*! An entry of FrameList. */
class FrameListEntry
{
public:

    /*! Feed with string and generate one FrameListEntry.

      @return shared pointer of FrameListEntry.
    */
    static shared_ptr<FrameListEntry>
    FeedWithCharArray(const vector<const char*>& charArray);

    /*! Destructor */
    virtual ~FrameListEntry();

    /*! Return timeStamp of this entry.

      @return timeStamp
    */
    uint32 GetTimeStamp();

    /*! Return timeStamp of this entry.

      @return timeStamp
    */
    shared_ptr<string> GetTimeStampString();

    /*! Return FrameId of this entry.

      @return shared pointer of the frame id.
    */
    shared_ptr<string> GetFrameId();

    /*! Not equal operator

      @return true if not equal
    */
    bool operator == (const FrameListEntry& entry) const;
private:
    /*! Default Constructor */
    FrameListEntry();

    uint32             m_timeStamp;      /*!< Time stamp when this frame is captured. */
    shared_ptr<string> m_timeStampString;/*!< TimeStamp of string form. */
    shared_ptr<string> m_frameID;        /*!< frameId that can be used to download tile. */

    //@note: According to SDS, the entry can be extended, add more member fields here if needed.
};

};  // namespace nbmap

/*! @} */

#endif  // FRAMELISTLAYER_H
