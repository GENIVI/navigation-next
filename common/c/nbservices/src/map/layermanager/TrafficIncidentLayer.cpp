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
    @file       TrafficIncidentLayer.cpp

    See header file for description.
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

extern "C"
{
#include "nbcontextprotected.h"
#include "transformutility.h"
}

#include "TrafficIncidentLayer.h"
#include "datastream.h"
#include "UnifiedLayer.h"   // Need constants in this header file.
#include "PinManager.h"
#include "PinLayer.h"
#include "PinLayerImpl.h"
#include "RefreshLayer.h"
#include "TrafficBubbleProvider.h"
#include "TrafficIncidentListener.h"
#include <algorithm>
#include "PinCushion.h"
#include "nbmacros.h"

/*! @{ */

using namespace nbcommon;
using namespace nbmap;
using namespace std;

// Local Constants ..............................................................................

/* According to the previous requirement, traffic should not be displayed when the
   zoom level is lower than 9. Calculate an appropriate reference tile grid zoom level
   to make the distance of one tile about 100 miles, the result of zoom level is 9.
   The formula is:

   #define PI 3.14159265358979323846
   #define RADIUS_EARTH_METERS 6378137.0
   #define EXPECTED_LATITUDE 0.0    // For equator
   #define DEGREE_TO_RADIAN 0.017453292519943295769222222222222
   #define MILE_TO_METER 1609.34

   double expectedZoomLevel = ceil(log2((2 * PI * RADIUS_EARTH_METERS * cos(EXPECTED_LATITUDE * DEGREE_TO_RADIAN)) / (100 * MILE_TO_METER) / cos(45.0 * DEGREE_TO_RADIAN)));
*/
//zoom level 11 per Karma requirement
const uint32 INCIDENT_REFERENCE_TILE_GRID_LEVEL = 11;
const uint32 INCIDENT_MIN_ZOOM_LEVEL = 11;

// The draw order of traffic incidents is max value of draw order.
const uint32 INCIDENT_DRAW_ORDER = 216;

// Slice size used to search traffic incidents
const uint32 INCIDENT_SEARCH_SLICE_SIZE = 100;

// 1 second equals to 1000 milliseconds
//const uint32 MILLISECONDS_PER_SECOND = 1000;

// Layer ID of the pin layer for traffic incidents
const char INCIDENT_PIN_LAYER_ID[] = "traffic-incident-pin-layer";

// Local Types ..................................................................................

/*! Functor to find a pointer to the tile key */
class FindTileKeyPointerFunctor : public Base
{
public:
    /*! FindTileKeyPointerFunctor constructor */
    FindTileKeyPointerFunctor(TileKeyPtr tileKeyToFind)
    : m_tileKeyToFind(tileKeyToFind)
    {
        // Nothing to do here.
    }

    /*! FindTileKeyPointerFunctor destructor */
    virtual ~FindTileKeyPointerFunctor()
    {
        // Nothing to do here.
    }

    bool operator() (TileKeyPtr tileKey)
    {
        // Check if the pointers to both tile keys are equal including both NULL.
        if (m_tileKeyToFind.get() == tileKey.get())
        {
            return true;
        }

        // Check if one of the tile keys is NULL.
        if (!(m_tileKeyToFind && tileKey))
        {
            return false;
        }

        // Compare two tile keys.
        return ((*m_tileKeyToFind) == (*tileKey));
    }


private:
    TileKeyPtr m_tileKeyToFind;     /*!< A tile key to find */
};


// Public functions .............................................................................

/* See header file for description */
TrafficIncidentLayer::TrafficIncidentLayer(NB_Context* context,
                                           uint32 digitalId,
                                           shared_ptr<string> layerId,
                                           PinManagerPtr pinManager,
                                           shared_ptr<PinCushion> pinCushion)
        : UnifiedLayer(TileManagerPtr(), digitalId, context),
          m_pinCushion(pinCushion),
          m_context(context),
          m_singleSearchHandler(NULL),
          m_pinManager(pinManager)
{
    if (m_tileLayerInfo)
    {
        m_tileLayerInfo->minZoom          = INCIDENT_MIN_ZOOM_LEVEL;
        m_tileLayerInfo->maxZoom          = DEFAULT_MAX_SUPPORTED_ZOOM_LEVEL;
        m_tileLayerInfo->drawOrder        = INCIDENT_DRAW_ORDER;
        m_tileLayerInfo->tileDataType     = layerId;
        m_tileLayerInfo->refTileGridLevel = INCIDENT_REFERENCE_TILE_GRID_LEVEL;
        m_tileLayerInfo->labelDrawOrder   = INCIDENT_DRAW_ORDER;
    }

    // Get the traffic bubble provider.
    m_bubbleProbider = TrafficBubbleProvider::GetTrafficBubbleProvider();

    // Update characteristics.
    SetCharacteristics(TILE_ADDITIONAL_KEY_OPTIONAL, OPTIONAL_DEFAULT_OFF);
    SetCharacteristics(TILE_ADDITIONAL_KEY_NAME, *layerId);

    // manually calling porcess characteristics since the TrafficIncidentLayer is manually created vs from metadata_source.
    ProcessCharacteristics();

    // Traffic incidents should be requested at reference grid level 9, so set this flag false to enable zoom level convertion.
    m_isRasterLayer = false;
}

/* See header file for description */
TrafficIncidentLayer::~TrafficIncidentLayer()
{
    // Remove the pin layer for traffic incidents.
    if (m_pinManager && m_pinLayer)
    {
        m_pinManager->RemovePinLayer(m_pinLayer);
    }
    m_pinLayer.reset();
    m_pinManager.reset();

    // Destroy the search handler.
    if (m_singleSearchHandler)
    {
        // Ignore the returned error.
        NB_SingleSearchHandlerDestroy(m_singleSearchHandler);
        m_singleSearchHandler = NULL;
    }

    m_context = NULL;
}

/* See description in Layer.h */
void
TrafficIncidentLayer::RemoveAllTiles()
{
    // Nothing to do here.
}

/* See header file for description */
void
TrafficIncidentLayer::RegisterListener(TrafficIncidentListener* listener)
{
    if (listener)
    {
        m_listeners.insert(listener);
    }
}

/* See header file for description */
void
TrafficIncidentLayer::UnregisterListener(TrafficIncidentListener* listener)
{
    if (listener)
    {
        m_listeners.erase(listener);
    }
}

/* See header file for description */
void
TrafficIncidentLayer::SetLanguage(shared_ptr<string> language)
{
    m_language = language;
}

/* See header file for description */
vector<NB_TrafficIncident>
TrafficIncidentLayer::GetTrafficIncidents()
{
    vector<NB_TrafficIncident> trafficIncidents;

    // Get all traffic incidents from the current search results.
    NB_Error error = GetTrafficIncidentsFromSearchResults(m_validResults, trafficIncidents);

    if (error == NE_OK)
    {
        return trafficIncidents;
    }

    return vector<NB_TrafficIncident>();
}

/*! Process the search requests in pending list

    @return None
*/
void
TrafficIncidentLayer::ProcessPendingSearch()
{
    // Check if the pending list is empty.
    if (m_pendingList.empty())
    {
        return;
    }

    // Create the search handler if it is NULL.
    NB_Error error = NE_OK;
    if (!m_singleSearchHandler)
    {
        NB_RequestHandlerCallback handlerCallback = {0};
        handlerCallback.callback = (NB_RequestHandlerCallbackFunction) SearchIncidentCallback;
        handlerCallback.callbackData = this;
        error = NB_SingleSearchHandlerCreate(m_context, &handlerCallback, &m_singleSearchHandler);
        if ((error != NE_OK) || (!m_singleSearchHandler))
        {
            return;
        }
    }

    // Check if the search is requesting in progress.
    if (NB_SingleSearchHandlerIsRequestInProgress(m_singleSearchHandler))
    {
        return;
    }

    /* Get a pending tile key from back of the list. Pop it from back and push
       it to front after search start succeeds. This logic could ensure that
       the recent item is searched first. */
    TileKeyPtr tileKey = m_pendingList.back();
    if (!tileKey)
    {
        // Pop this empty tile key.
        m_pendingList.pop_back();
        return;
    }

    // Use the tile key to set the bounding box.
    NB_SearchRegion region;
    nsl_memset(&region, 0, sizeof(region));
    region.type = NB_ST_BoundingBox;

    // Convert the tile key of top left to coordinate.
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    NB_SpatialConvertTileToMercator(tileKey->m_x,
                                    tileKey->m_y,
                                    tileKey->m_zoomLevel,
                                    0.0,
                                    0.0,
                                    &mercatorX,
                                    &mercatorY);
    mercatorReverse(mercatorX,
                    mercatorY,
                    &(region.boundingBox.topLeft.latitude),
                    &(region.boundingBox.topLeft.longitude));

    // Convert the tile key of bottom right to coordinate.
    NB_SpatialConvertTileToMercator(tileKey->m_x + 1,
                                    tileKey->m_y + 1,
                                    tileKey->m_zoomLevel,
                                    0.0,
                                    0.0,
                                    &mercatorX,
                                    &mercatorY);
    mercatorReverse(mercatorX,
                    mercatorY,
                    &(region.boundingBox.bottomRight.latitude),
                    &(region.boundingBox.bottomRight.longitude));

    // Create an instance of search parameters.
    NB_SingleSearchParameters* searchParameters = NULL;
    error = NB_SingleSearchParametersCreateByResultStyleType(m_context,
                                                            &region,
                                                            NULL,
                                                            "tcs-single-search-2",
                                                            INCIDENT_SEARCH_SLICE_SIZE,
                                                            NB_EXT_WantContentTypes,
                                                            m_language ? (m_language->c_str()) : "",
                                                            NB_SRST_SingleSearch,
                                                            &searchParameters);


    if ((error != NE_OK) || (!searchParameters))
    {
        return;
    }

    char severityString[12]; /* ten digits + minus + terminating null */
    nsl_memset(severityString, 0, sizeof(severityString));
    nsl_sprintf(severityString, "%d", NB_TrafficIncident_LowImpact);
    error = NB_SingleSearchParametersAddSearchFilterKeyValue(searchParameters, "severity", severityString);
    if (error != NE_OK)
    {
        // Destroy the created search parameters.
        NB_SingleSearchParametersDestroy(searchParameters);
        return;
    }

    error = NB_SingleSearchParametersAddWantContentType(searchParameters, NB_SWCT_TrafficIncident);
    if (error != NE_OK)
    {
        // Destroy the created search parameters.
        NB_SingleSearchParametersDestroy(searchParameters);
        return;
    }

    // Request traffic incidents and ignore the returned error.
    error = NB_SingleSearchHandlerStartRequest(m_singleSearchHandler, searchParameters);

    // Destroy the created search parameters.
    NB_SingleSearchParametersDestroy(searchParameters);


    // @TRICKY: Normally, single search callback will be called in another task in CCC
    //          thread. However, there may be special cases (for example network
    //          error), callback can be called synchronously in
    //          NB_SingleSearchHandlerStartRequest(). When this happens, m_pendingList
    //          will be modified in callback, and we need to check this list again
    //          here.

    if (error == NE_OK && !m_pendingList.empty())
    {
        // Pop the pending tile key from back and push it to front if search start
        // succeeds.
        TileKeyPtr key = m_pendingList.back();
        if (key && (*key == *tileKey))
        {
            m_pendingList.pop_back();
            m_pendingList.push_front(tileKey);
        }
    }
}

/*! Add the search result and show pins

    This function takes ownership of the search information.

    @return None
*/
void
TrafficIncidentLayer::AddSearchResult(NB_SingleSearchInformation* searchInformation,  /*!< Search information to add */
                                      TileKeyPtr tileKey                        /*!< A tile key associated with the search information */
                                      )
{
    // Check if the search information is valid.
    if (!searchInformation)
    {
        return;
    }

    /* Get the count of search result.

       TRICKY: The search result needs to be added to avoid refreshing again,
               even if the count of traffic incident is 0.
    */
    uint32 searchResultCount = 0;
    NB_Error error = NB_SingleSearchInformationGetResultCount(searchInformation,
                                                            &searchResultCount);

    if (error != NE_OK || searchResultCount <= 0)
    {
        // Destroy the search information and ignore the returned error.
        NB_SingleSearchInformationDestroy(searchInformation);
        searchInformation = NULL;
        return;
    }

    // Check if the tile key is valid.
    if (!tileKey)
    {
        // Destroy the search information and ignore the returned error.
        NB_SingleSearchInformationDestroy(searchInformation);
        searchInformation = NULL;
        return;
    }

    // Create an object of incident result.
    IncidentResultPtr incidentResult(CCC_NEW IncidentResult(searchInformation,
                                                            tileKey));
    if (!incidentResult)
    {
        // Destroy the search information and ignore the returned error.
        NB_SingleSearchInformationDestroy(searchInformation);
        searchInformation = NULL;
        return;
    }

    // Add the object of incident result to the map of valid results.
    pair<map<TileKey, IncidentResultPtr>::iterator, bool> insertedResult =
    m_validResults.insert(pair<TileKey, IncidentResultPtr>(*tileKey,
                                                           incidentResult));

    if (insertedResult.second)
    {
        ShowPinsBySearchResult(searchInformation,
                               incidentResult->m_pinIds);

        if (searchResultCount > 0)
        {
            // Notify that the traffic incidents are updated.
            NotifyToUpdateTrafficIncidents();
        }
    }
}

/*! Show pins by the search result

    @return None
*/
void
TrafficIncidentLayer::ShowPinsBySearchResult(NB_SingleSearchInformation* searchInformation,   /*!< Search information to add pins */
                                             vector<shared_ptr<string> >& pinIdsToReturn/*!< On return the pin IDs of added pins */
                                             )
{
    // Check if the search result and bubble provider are valid.
    if ((!searchInformation) || (!m_bubbleProbider))
    {
        return;
    }

    // Create the pin layer for traffic incidents if it does not exist.
    if ((!m_pinLayer) && m_pinManager)
    {
        // Create a string of layer ID for the pin layer.
        shared_ptr<string> pinLayerId(CCC_NEW string(INCIDENT_PIN_LAYER_ID));
        if (pinLayerId)
        {
            m_pinLayer = m_pinManager->AddPinLayer(pinLayerId);
            if (m_pinLayer)
            {
                // Set config of traffic incidents.
                PinLayerImpl* pinLayerImpl = static_cast<PinLayerImpl*>(m_pinLayer.get());
                if (pinLayerImpl)
                {
                    pinLayerImpl->SetMinZoom(INCIDENT_MIN_ZOOM_LEVEL);
                    pinLayerImpl->SetMaxZoom(DEFAULT_MAX_SUPPORTED_ZOOM_LEVEL);
                    pinLayerImpl->SetReferenceTileGridLevel(INCIDENT_REFERENCE_TILE_GRID_LEVEL);
                    pinLayerImpl->SetEnabled(IsEnabled());
                }
            }
        }
    }

    /* Add the pins of traffic incidents.

       TRICKY: The bubble getter is platform-dependent and implemented in nbui. If pins are
               added in nbservices, we need to fix some classes to template (TrafficManager,
               TrafficIncidentLayer and so on). I think it is simple to make nbui to add
               pins of traffic incidents here. Because nbui knows the T exactly.
    */
    if (m_pinLayer)
    {
        m_bubbleProbider->AddTrafficIncidentPins(m_pinLayer, searchInformation, pinIdsToReturn, m_pinCushion);
    }
}

/*! Refresh expired search results

    @return None
*/
void
TrafficIncidentLayer::RefreshSearchResults(const vector<TileKeyPtr>& tileKeys)
{
    if (tileKeys.empty())
    {
        return;
    }

    // Get all pin IDs from expired search results.
    vector<shared_ptr<string> > pinIdsToRefresh;

    //@todo: this loop should be optimized...
    vector<TileKeyPtr>::const_iterator tileIterator = tileKeys.begin();
    vector<TileKeyPtr>::const_iterator tileEnd = tileKeys.end();
    for (;tileIterator != tileEnd; ++tileIterator)
    {
        //Convert the tile key from traffic layer
        TileKeyPtr convertedKey;
        int distance = (*tileIterator)->m_zoomLevel - m_tileLayerInfo->refTileGridLevel;
        if (distance > 0)
        {
            convertedKey = (TileKeyPtr)(CCC_NEW TileKey((*tileIterator)->m_x >> distance,
                                                    (*tileIterator)->m_y >> distance,
                                                    m_tileLayerInfo->refTileGridLevel));
        }
        else
        {
            convertedKey = *tileIterator;
        }

        map<TileKey, IncidentResultPtr>::iterator resultIterator = m_validResults.find(*convertedKey);
        map<TileKey, IncidentResultPtr>::const_iterator resultEnd = m_validResults.end();
        if (resultIterator == resultEnd)
        {
            continue;
        }

        IncidentResultPtr incidentResult = resultIterator->second;
        if (!incidentResult)
        {
            continue;
        }

        // Add the pin IDs to refresh pin tiles.
        pinIdsToRefresh.insert(pinIdsToRefresh.end(),
                               incidentResult->m_pinIds.begin(),
                               incidentResult->m_pinIds.end());
        m_validResults.erase(resultIterator);
    }

    if (m_pinLayer)
    {
        if (!(pinIdsToRefresh.empty()))
        {
            // Remove all expired pins. This function notifies to refresh tiles of pins.
            m_pinLayer->RemovePins(pinIdsToRefresh);

            // If there are pins removed, notify that the traffic incidents are updated.
            NotifyToUpdateTrafficIncidents();
        }
    }
}

/*! Get the traffic incidents from the search results

    @return NE_OK if success
*/
NB_Error
TrafficIncidentLayer::GetTrafficIncidentsFromSearchResults(const map<TileKey, IncidentResultPtr>& searchResults,/*!< The search results to get the traffic incidents */
                                                           vector<NB_TrafficIncident>& trafficIncidentsToReturn /*!< On return the traffic incidents if success */
                                                           )
{
    map<TileKey, IncidentResultPtr>::const_iterator resultIterator = searchResults.begin();
    map<TileKey, IncidentResultPtr>::const_iterator resultEnd = searchResults.end();
    for (; resultIterator != resultEnd; ++resultIterator)
    {
        IncidentResultPtr incidentResult = resultIterator->second;
        if (!incidentResult)
        {
            continue;
        }

        // Get the search information.
        NB_SingleSearchInformation* searchInformation = incidentResult->m_singleSearchInformation;
        if (!searchInformation)
        {
            continue;
        }

        // Get the count of search result.
        uint32 searchResultCount = 0;
        NB_Error error = NB_SingleSearchInformationGetResultCount(searchInformation,
                                                                  &searchResultCount);
        if (error != NE_OK)
        {
            return error;
        }

        if (searchResultCount <= 0)
        {
            // There is no traffic incidents.
            continue;
        }

        // Get the traffic incidents from the search result.
        for (uint32 i = 0; i < searchResultCount; ++i)
        {
            NB_TrafficIncident trafficIncident;
            memset(&trafficIncident, 0, sizeof(trafficIncident));
            error = NB_SingleSearchInformationGetTrafficIncident(searchInformation,
                                                           i,
                                                           &trafficIncident,
                                                           NULL);
            if (error != NE_OK)
            {
                return error;
            }

            // Add the traffic incident to return.
            trafficIncidentsToReturn.push_back(trafficIncident);
        }
    }

    return NE_OK;
}

/*! Notify that the traffic incidents are updated

    @return None
*/
void
TrafficIncidentLayer::NotifyToUpdateTrafficIncidents()
{
    // There is no need to get the current traffic incidents if there is no listeners.
    if (m_listeners.empty())
    {
        return;
    }

    // Get the current traffic incidents.
    vector<NB_TrafficIncident> trafficIncidents = GetTrafficIncidents();

    // Notify each listener that the traffic incidents are updated.
    set<TrafficIncidentListener*>::const_iterator listenerIterator = m_listeners.begin();
    set<TrafficIncidentListener*>::const_iterator listenerEnd = m_listeners.end();
    for (; listenerIterator != listenerEnd; ++listenerIterator)
    {
        TrafficIncidentListener* listener = *listenerIterator;
        if (listener)
        {
            listener->TrafficIncidentsUpdated(trafficIncidents);
        }
    }
}

/*! Callback function of search handler

    @see NB_RequestHandlerCallbackFunction
    @return None
*/
void
TrafficIncidentLayer::SearchIncidentCallback(NB_SingleSearchHandler* handler,
                                             NB_RequestStatus status,
                                             NB_Error error,
                                             nb_boolean up,
                                             int /*percent*/,
                                             TrafficIncidentLayer* layer)
{
    if ((!handler) || up || (!layer))
    {
        return;
    }

    if (error != NE_OK)
    {
        // @todo: Should retry to search?
        if (!(layer->m_pendingList.empty()))
        {
            // Pop the front tile key which is the requested item.
            layer->m_pendingList.pop_front();
        }

        // Process next pending search request.
        layer->ProcessPendingSearch();
        return;
    }

    switch (status)
    {
        case NB_NetworkRequestStatus_Success:
        {
            if (layer->m_pendingList.empty())
            {
                // There is no pending requests. Return directly.
                return;
            }

            // Get the search result.
            NB_SingleSearchInformation* newSearchInformation = NULL;
            error = NB_SingleSearchHandlerGetInformation(handler,
                                                         &newSearchInformation);
            if ((error == NE_OK) && newSearchInformation)
            {
                /* Add the search result and show pins. Below function takes ownership
                   of the search information. */
                layer->AddSearchResult(newSearchInformation,
                                       layer->m_pendingList.front());
                newSearchInformation = NULL;
            }

            // Destroy the search information if it is not NULL.
            if (newSearchInformation)
            {
                // Ignore the returned error.
                NB_SingleSearchInformationDestroy(newSearchInformation);
                newSearchInformation = NULL;
            }

            // Pop the front tile key which is the requested item.
            layer->m_pendingList.pop_front();

            // Process next pending search request.
            layer->ProcessPendingSearch();
            break;
        }
        case NB_NetworkRequestStatus_Canceled:
        case NB_NetworkRequestStatus_Failed:
        case NB_NetworkRequestStatus_TimedOut:
        {
            // @todo: Should retry to search?
            if (!(layer->m_pendingList.empty()))
            {
                // Pop the front tile key which is the requested item.
                layer->m_pendingList.pop_front();
            }

            // Process next pending search request.
            layer->ProcessPendingSearch();
            break;
        }
        case NB_NetworkRequestStatus_Progress:
        default:
        {
            // Nothing to do here.
            break;
        }
    }
}

// IncidentResult functions .....................................................................

/* See header file for description */
TrafficIncidentLayer::IncidentResult::IncidentResult(NB_SingleSearchInformation* searchInformation,
                                                     TileKeyPtr tileKey)
: m_singleSearchInformation(searchInformation),
  m_tileKey(tileKey),
  m_pinIds()
{
    // Nothing to do here.
}

/* See header file for description */
TrafficIncidentLayer::IncidentResult::~IncidentResult()
{
    if (m_singleSearchInformation)
    {
        // Destroy the search information and ignore the returned error.
        NB_SingleSearchInformationDestroy(m_singleSearchInformation);
        m_singleSearchInformation = NULL;
    }
}

/* See description in header file. */
void TrafficIncidentLayer::GetTile(TileKeyPtr tileKey,
                                   TileKeyPtr convertedKey,
                                   shared_ptr <AsyncCallbackWithRequest <TileKeyPtr, TilePtr> > callback,
                                   uint32 /*priority*/,
                                   bool cachedOnly)
{
    //@note: cachedOnly only applies to raster tiles.
    //@todo: Check if this is correct.

    if (cachedOnly)
    {
        return;
    }

    /* If the zoom level of parameter tile key is less than reference tile grid
       zoom level of this layer, return NE_NOENT directly without search
       traffic incidents.

       @todo: I think we should not convert and add tile keys to the pending
       search list if the zoom level of parameter tile key is less than
       reference tile grid zoom level. Because this parameter tile key
       could be converted to a lot of tile keys of reference tile grid
       zoom level to search. And there is no this case now. Because the
       reference tile grid zoom level is 7 and min zoom level is 8.
    */

    TileKey& tileKeyReference = *convertedKey;

    // Check if this tile key already exists in valid search results.
    map<TileKey, IncidentResultPtr>::iterator resultIterator = m_validResults.find(tileKeyReference);
    map<TileKey, IncidentResultPtr>::const_iterator resultEnd = m_validResults.end();
    if ((resultIterator == resultEnd) ||
        (!(tileKeyReference == resultIterator->first)))
    {
        // Check if this tile key already exists in search pending list.
        FindTileKeyPointerFunctor functor(convertedKey);
        list<TileKeyPtr>::const_iterator pendingIterator = find_if(m_pendingList.begin(),
                                                                   m_pendingList.end(),
                                                                   functor);
        if (pendingIterator == m_pendingList.end())
        {
            // Add this tile key to the back of pending search list.
            m_pendingList.push_back(convertedKey);
        }

        // Process pending search requests.
        ProcessPendingSearch();
    }

    // Return NE_NOENT. Because there is no tiles in this layer.
    if (callback)
    {
        callback->Error(tileKey, NE_NOENT);
    }
}

/* See description in header file. */
void TrafficIncidentLayer::PostLayerToggled()
{
    // Set the enabled flag to the pin layer.
    if (m_pinLayer)
    {
        PinLayerImpl* pinLayerImpl = static_cast<PinLayerImpl*>(m_pinLayer.get());
        if (pinLayerImpl)
        {
            pinLayerImpl->SetEnabled(IsEnabled());
        }
    }
    // No need to cancel downloads.
}


/*! @} */
