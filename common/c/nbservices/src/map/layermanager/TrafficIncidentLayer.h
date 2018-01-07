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
    @file       TrafficIncidentLayer.h

    Class TrafficIncidentLayer inherits from class Layer. Class
    TrafficIncidentLayer is used to search pins of traffic incidents.
    It calls function PinManager::AddPinLayer to create a pin layer to
    display pins of traffic incidents. Traffic incidents are the search
    result by using bounding box. The bounding box is calculated by one
    tile of reference tile grid zoom level. The pins of traffic incidents
    are refreshed in 5 minutes.
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

#ifndef TRAFFICINCIDENTLAYER_H
#define TRAFFICINCIDENTLAYER_H

/*!
    @addtogroup nbmap
    @{
*/

extern "C"
{
#include "paltypes.h"
#include "paltimer.h"
#include "nbcontext.h"
#include "nbsinglesearchinformation.h"
#include "nbsinglesearchhandler.h"
}

#include "smartpointer.h"
#include "AsyncCallback.h"
#include "UnifiedLayer.h"
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace nbmap
{
// Types ........................................................................................

class PinManager;
class PinLayer;
class TrafficIncidentListener;
class TrafficBubbleProvider;
class PinCushion;

/*! The layer used to display pins of traffic incidents */
class TrafficIncidentLayer : public UnifiedLayer
{
public:
    // Public functions .........................................................................

    /*! TrafficIncidentLayer constructor */
    TrafficIncidentLayer(NB_Context* context,                   /*!< NB_Context instance */
                         uint32 digitalId,                      /*!< A digital layer ID */
                         shared_ptr<std::string> layerId,       /*!< An ID to identify the layer */
                         shared_ptr<PinManager> pinManager,     /*!< A pointer to a pin manager */
                         shared_ptr<PinCushion> pinCushion      /*!< pin cushion for getting traffic bubble resolver */
                         );

    /*! TrafficIncidentLayer destructor */
    virtual ~TrafficIncidentLayer();

    /* See description in Layer.h */
    virtual void RemoveAllTiles();

    /*! Register a listener

        @return None
    */
    void
    RegisterListener(TrafficIncidentListener* listener      /*!< A listener to register */
                     );

    /*! Unregister a listener

        User should always unregister the listener when the object of listener is destroyed.

        @return None
    */
    void
    UnregisterListener(TrafficIncidentListener* listener    /*!< A listener to unregister */
                       );

    /*! Set language to search traffic incidents

        @return None
    */
    void
    SetLanguage(shared_ptr<std::string> language    /*!< Language to set */
                );

    /*! Get the traffic incidents

        This function should be called to get the current traffic incidents. User should also
        register a listener to monitor the traffic incidents updated.

        @todo: This function returns all traffic incidents so far. I think it should return the
               visible traffic incidents on map.

        @return A vector of the traffic incidents
    */
    std::vector<NB_TrafficIncident>
    GetTrafficIncidents();

    void RefreshSearchResults(const vector<TileKeyPtr>& tileKeys);
    virtual std::string className() const { return "TrafficIncidentLayer"; }

private:
    // Private types ............................................................................

    /*! Search result of traffic incidents */
    class IncidentResult : public Base
    {
    public:
        /*! IncidentResult constructor */
        IncidentResult(NB_SingleSearchInformation* searchInformation, /*!< Single search information of traffic incidents */
                       TileKeyPtr tileKey                             /*!< A tile key of the reference tile grid zoom level */
                       );

        /*! IncidentResult destructor */
        virtual ~IncidentResult();

        NB_SingleSearchInformation* m_singleSearchInformation;  /*!< Single search information of traffic incidents */
        TileKeyPtr m_tileKey;                                   /*!< A tile key of the reference tile grid zoom level */
        std::vector<shared_ptr<string> > m_pinIds;              /*!< Pin IDs associated with this search result */


    private:
        // Copy constructor and assignment operator are not supported.
        IncidentResult(const IncidentResult& result);
        IncidentResult& operator=(const IncidentResult& index);
    };

    typedef shared_ptr<IncidentResult> IncidentResultPtr;


    // Private functions ...................................................................

    // See UnifiedLayer for description.
    virtual void GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                         AsyncTileRequestWithRequestPtr callback,
                         uint32 priority, bool cachedOnly = false);
    virtual void PostLayerToggled();

    // Copy constructor and assignment operator are not supported.
    TrafficIncidentLayer(const TrafficIncidentLayer& layer);
    TrafficIncidentLayer& operator=(const TrafficIncidentLayer& layer);

    /* See source file for description */

    void ProcessPendingSearch();
    void AddSearchResult(NB_SingleSearchInformation* searchInformation,
                         TileKeyPtr tileKey);
    void ShowPinsBySearchResult(NB_SingleSearchInformation* searchInformation,
                                std::vector<shared_ptr<std::string> >& pinIdsToReturn);

    NB_Error GetTrafficIncidentsFromSearchResults(const std::map<TileKey, IncidentResultPtr>& searchResults,
                                                  std::vector<NB_TrafficIncident>& trafficIncidentsToReturn);
    void NotifyToUpdateTrafficIncidents();
    static void SearchIncidentCallback(NB_SingleSearchHandler* handler,
                                       NB_RequestStatus status,
                                       NB_Error error,
                                       nb_boolean up,
                                       int percent,
                                       TrafficIncidentLayer* layer);


    // Private members ..........................................................................
    shared_ptr<PinCushion> m_pinCushion;         /*!< pin cushion for getting traffic bubble resolver */

    NB_Context* m_context;                                  /*!< Pointer to current context */
    NB_SingleSearchHandler* m_singleSearchHandler;          /*!< A single search handler to search traffic incidents */
    shared_ptr<std::string> m_language;                     /*!< Language used to search traffic incidents */
    shared_ptr<PinManager> m_pinManager;                    /*!< A pin manager used to create a pin layer for
                                                                 traffic incidents. */
    shared_ptr<PinLayer> m_pinLayer;                        /*!< A pin layer contained pins of traffic incidents */
    shared_ptr<TrafficBubbleProvider> m_bubbleProbider;     /*!< Bubble probider used to add pins of traffic incidents */
    std::set<TrafficIncidentListener*> m_listeners;         /*!< A set of listeners */
    std::list<TileKeyPtr> m_pendingList;                    /*!< A search pending list of tile keys of
                                                                 reference tile grid zoom level. The tile
                                                                 keys used to generate the bounding box to
                                                                 search traffic incidents. It is a
                                                                 doubly-linked list. Pop the back item and
                                                                 push it to front when getting an item to
                                                                 search. So this logic ensures that the
                                                                 recent item is searched first. And there
                                                                 are not many items in this list with
                                                                 appropriate reference tile grid zoom level. */
    std::map<TileKey, IncidentResultPtr> m_validResults;    /*!< Valid search results of traffic incidents
                                                                 associated with specified tile keys of
                                                                 reference tile grid zoom level. These
                                                                 search will be moved to m_expiredResults
                                                                 when the expired timer is fired. */
};

typedef shared_ptr<TrafficIncidentLayer> TrafficIncidentLayerPtr;

};  // namespace nbmap

/*! @} */

#endif  // TRAFFICINCIDENTLAYER_H
