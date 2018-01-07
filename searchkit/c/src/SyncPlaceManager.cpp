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
 @file     SyncPlaceManager.h
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

#include "SyncPlaceManager.h"
#include "contextbasedsingleton.h"
#include <string>
#include "palstdlib.h"
#include "palclock.h"
#include "offboard/SingleSearchOffboardInformation.h"
#include "TpsAtlasbookProtocol.h"
#include "TpsSyncPlacesHandler.h"
#include "TpsSyncStatusHandler.h"
#include <sstream>
#include "dbdaofactory.h"
#include "StringUtility.h"

#define SYNC_TIMER_INTERVAL 60000

using namespace nbsearch;
using namespace nbmap;
using namespace protocol;

static const char* SYNC_NAMES[] = {"recents", "favorites"};
namespace nbsearch
{
    const char* LTKSPM_RecentcsGenID = "recentsGenID";
    const char* LTKSPM_RecentcsDBID = "recentsDBID";
    const char* LTKSPM_FavoritesGenID = "favoritesGenID";
    const char* LTKSPM_FavoritesDBID = "favoritesDBID";
}

SyncPlaceTask::SyncPlaceTask(SyncPlaceManager* handler, TaskFunction function, uint32 type)
    : m_type(type)
    , m_handler(handler)
    , m_function(function)
{
}

SyncPlaceTask::~SyncPlaceTask()
{
}

void SyncPlaceTask::Execute(void)
{
    if (m_handler)
    {
        (m_handler->*m_function)(this);
    }

    delete this;
}

shared_ptr<SyncPlaceManager> SyncPlaceManager::GetInstance(NB_Context* context, IPOIDAO* poiDAO)
{
    nbcommon::Tuple<TYPELIST_1(IPOIDAO*)> otherParameters = nbcommon::MakeTuple(poiDAO);
    SyncPlaceManagerPtr syncPlaceManager = ContextBasedSingleton<SyncPlaceManager>::getInstance(context, otherParameters);
    return syncPlaceManager;
}

SyncPlaceManager::SyncPlaceManager(NB_Context* context, IPOIDAO* poiDAO)
    : m_context(context), m_poiDAO(poiDAO), m_syncType(0)
{
    m_placesForSync.clear();
    m_handler.reset();
    m_statusHandler.reset();
    m_eventQueue = shared_ptr<EventTaskQueue>(new EventTaskQueue(NB_ContextGetPal(context)));
    m_handlerPool.clear();
    m_statusHandlerPool.clear();
}

SyncPlaceManager::~SyncPlaceManager()
{
    PAL_TimerCancel(NB_ContextGetPal(m_context), SyncTimerCallback, this);
}

void SyncPlaceManager::SyncRequestIfNeeded()
{
    uint32 syncType = NeedSync();
    if (syncType)
    {
        SyncRequest(syncType);
    }
}

void SyncPlaceManager::SyncStatus()
{
    NB_Error err = NE_OK;
    err = CreateSyncStatusHandler();
    if(err != NE_OK)
    {
        return;
    }

    SyncStatusParametersSharedPtr param = SyncStatusParametersSharedPtr(new SyncStatusParameters());
    for (int i = 0; i < (sizeof(SYNC_NAMES) / sizeof(char*)); ++i)
    {
        SyncPlacesDBStatusQuerySharedPtr dbplace = SyncPlacesDBStatusQuerySharedPtr(new SyncPlacesDBStatusQuery());
        dbplace->SetName(shared_ptr<std::string>(new std::string(SYNC_NAMES[i])));
        param->GetSyncPlacesDBStatusQueryArray()->push_back(dbplace);
    }

    m_statusHandler->Request(param, m_statusHandlerPool[m_statusHandler]);
}

void SyncPlaceManager::SyncRequest(uint32 syncType)
{
    m_syncType = 0;
    if (!NeedSync())
    {
        return;
    }
    NB_Error err = NE_OK;
    err = CreateSyncHandler();
    if(err != NE_OK)
    {
        return;
    }

    std::vector<POIForSync>::iterator iter = m_placesForSync.begin();
    while ( iter != m_placesForSync.end() )
    {
        delete (*iter).poi;
        nsl_free((*iter).userData);
        iter ++;
    }
    m_placesForSync.clear();
    SyncPlacesParametersSharedPtr param = SyncPlacesParametersSharedPtr(new SyncPlacesParameters());
    for (unsigned short type = POI_TYPE_RECENT; type <= POI_TYPE_FAVORITE; type++)
    {
        if (syncType & (1 << type))
        {
            std::string dbname;
            std::string generationName;
            std::string databaseName;
            if (type == POI_TYPE_RECENT)
            {
                dbname = std::string("recents");
                generationName = std::string(LTKSPM_RecentcsGenID);
                databaseName   = std::string(LTKSPM_RecentcsDBID);
            }
            else if (type == POI_TYPE_FAVORITE)
            {
                dbname = std::string("favorites");
                generationName = std::string(LTKSPM_FavoritesGenID);
                databaseName   = std::string(LTKSPM_FavoritesDBID);
            }

            if (!dbname.empty())
            {
                std::string generationId;
                m_poiDAO->GetStringUserData(generationName.c_str(), generationId);
                std::string dbId;
                m_poiDAO->GetStringUserData(databaseName.c_str(), dbId);
                SyncPlacesDBQuerySharedPtr dbQuery = SyncPlacesDBQuerySharedPtr(new SyncPlacesDBQuery());
                dbQuery->SetName(shared_ptr<std::string>(new std::string(dbname)));
                dbQuery->SetDbID(nsl_atoi(dbId.c_str()));
                dbQuery->SetLastSyncGeneration(nsl_atoi(generationId.c_str()));
                if (SetSyncPlaceParameter(dbQuery, (DMPOIType)type) == NE_OK)
                {
                    param->GetSyncPlacesDBQueryArray()->push_back(dbQuery);
                }
            }
        }
    }
    param->SetWantPlaceDetails(true);
    param->SetWantCompactAddress(true);
    m_handler->Request(param, m_handlerPool[m_handler]);
}

void SyncPlaceManager::CancelSyncStatus()
{
    if (m_statusHandler.get())
    {
        m_statusHandler->Cancel();
    }
    if (m_handler.get())
    {
        m_handler->Cancel();
    }
}

void SyncPlaceManager::CancelAllSyncRequest()
{
    std::map<shared_ptr<protocol::Handler<protocol::SyncPlacesParameters, protocol::SyncPlacesInformation> >, shared_ptr<SyncPlacesAsyncCallback> >::iterator it = m_handlerPool.begin();
    for (; it != m_handlerPool.end(); ++it)
    {
        it->second->Reset();
        it->first->Cancel();
    }
}

NB_Error SyncPlaceManager::CreateSyncStatusHandler()
{
    NB_Error err = NE_OK;

    TpsAtlasbookProtocol tpsAtlasbookProtocol(m_context);
    m_statusHandler = tpsAtlasbookProtocol.GetSyncStatusHandler();
    if (!m_statusHandler)
    {
        err = NE_NOMEM;
    }

    shared_ptr<SyncStatusAsyncCallback> callback(new SyncStatusAsyncCallback(this));
    if (!callback)
    {
        m_statusHandler.reset();
        return NE_NOMEM;
    }
    m_statusHandlerPool[m_statusHandler] = callback;

    return err;
}

NB_Error SyncPlaceManager::CreateSyncHandler()
{
    NB_Error err = NE_OK;

    TpsAtlasbookProtocol tpsAtlasbookProtocol(m_context);
    m_handler = tpsAtlasbookProtocol.GetSyncPlacesHandler();
    if (!m_handler)
    {
        return NE_NOMEM;
    }
    shared_ptr<SyncPlacesAsyncCallback> callback(new SyncPlacesAsyncCallback(this));
    if (!callback)
    {
        m_handler.reset();
        return NE_NOMEM;
    }
    m_handlerPool[m_handler] = callback;

    return err;
}

uint32 SyncPlaceManager::NeedSync()
{
    uint32 syncType = 0;
    if (m_poiDAO)
    {
        std::vector<POIForSync> recentsForSync;
        std::vector<POIForSync> favoritesForSync;
        m_poiDAO->GetPOIForSync(POI_TYPE_RECENT, recentsForSync);
        m_poiDAO->GetPOIForSync(POI_TYPE_FAVORITE, favoritesForSync);
        if (!recentsForSync.empty() || !favoritesForSync.empty())
        {
            std::vector<POIForSync>::iterator iter = recentsForSync.begin();
            while ( iter != recentsForSync.end() )
            {
                delete (*iter).poi;
                nsl_free((*iter).userData);
                iter ++;
                syncType |= 1 << POI_TYPE_RECENT;
            }
            iter = favoritesForSync.begin();
            while ( iter != favoritesForSync.end() )
            {
                delete (*iter).poi;
                nsl_free((*iter).userData);
                iter ++;
                syncType |= 1 << POI_TYPE_FAVORITE;
            }
            recentsForSync.clear();
            favoritesForSync.clear();
            return syncType;
        }
        //check whether not sync with server, if not, do sync.
        std::string recentcsGenID;
        std::string favoritesGenID;
        m_poiDAO->GetStringUserData(LTKSPM_RecentcsGenID, recentcsGenID);
        m_poiDAO->GetStringUserData(LTKSPM_FavoritesGenID, favoritesGenID);
        if (recentcsGenID.empty())
        {
            syncType |= 1 << POI_TYPE_RECENT;
        }
        if (favoritesGenID.empty())
        {
            syncType |= 1 << POI_TYPE_FAVORITE;
        }
    }
    return syncType;
}

NB_Error SyncPlaceManager::SetSyncPlaceParameter(SyncPlacesDBQuerySharedPtr& dbQuery, DMPOIType type)
{
    NB_Error err = NE_OK;
    if (m_poiDAO)
    {
        std::vector<POIForSync>  placesForSync;
        m_poiDAO->GetPOIForSync(type, placesForSync);
        std::vector<POIForSync>::const_iterator it = placesForSync.begin();
        for (; it != placesForSync.end(); ++it)
        {
            POIForSync poi = *it;
            switch (poi.syncStatus)
            {
                case SYNC_STATUS_PLACE_MODIFIED:
                case SYNC_STATUS_TIME_STAMP_MODIFIED:
                {
                    SyncModifyItemSharedPtr modifyItem = SyncModifyItemSharedPtr(new SyncModifyItem());
                    modifyItem->SetId(poi.serverId);
                    modifyItem->SetModtime(PAL_ClockGetGPSTime());

                    PlaceSharedPtr place = GetPlaceToSync(poi);

                    modifyItem->SetPlace(place);
                    dbQuery->GetSyncModifyItemArray()->push_back(modifyItem);
                }
                    break;
                case SYNC_STATUS_DELETED:
                {
                    SyncDeleteItemSharedPtr deleteItem = SyncDeleteItemSharedPtr(new SyncDeleteItem());
                    deleteItem->SetId(poi.serverId);
                    dbQuery->GetSyncDeleteItemArray()->push_back(deleteItem);
                }
                    break;
                case SYNC_STATUS_ADDED:
                {
                    SyncAddItemSharedPtr addItem = SyncAddItemSharedPtr(new SyncAddItem());
                    std::stringstream ss;
                    ss << poi.internalId;
                    std::string id = ss.str();
                    addItem->SetId(shared_ptr<std::string>(new std::string(id)));
                    addItem->SetModtime(PAL_ClockGetGPSTime());

                    PlaceSharedPtr place = GetPlaceToSync(poi);

                    addItem->SetPlace(place);
                    dbQuery->GetSyncAddItemArray()->push_back(addItem);
                }
                    break;
                default:
                    break;
            }
        }
        m_placesForSync.insert(m_placesForSync.end(), placesForSync.begin(), placesForSync.end());
    }

    return err;
}

PlaceSharedPtr SyncPlaceManager::GetPlaceToSync(POIForSync poi)
{
    PlaceSharedPtr place = PlaceSharedPtr(new protocol::Place());
    place->SetName(shared_ptr<std::string>(new std::string(poi.poi->GetPlace() ? poi.poi->GetPlace()->GetName() : "")));

    if (poi.poi && poi.poi->GetPlace())
    {
        if (poi.poi->GetPlace()->GetLocation())
        {
            LocationSharedPtr location = LocationSharedPtr(new protocol::Location());
            location->SetName(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetName())));

            PointSharedPtr point = PointSharedPtr(new protocol::Point());
            point->SetLat(poi.poi->GetPlace()->GetLocation()->GetCoordinates().GetLatitude());
            point->SetLon(poi.poi->GetPlace()->GetLocation()->GetCoordinates().GetLongitude());
            location->SetPoint(point);

            AddressSharedPtr address = AddressSharedPtr(new protocol::Address());
            std::string addressType;
            switch (poi.poi->GetPlace()->GetLocation()->GetType())
            {
                case LT_Address:
                    addressType = "address";
                    break;
                case LT_Intersection:
                    addressType = "intersect";
                    break;
                case LT_Airport:
                    addressType = "airport";
                    break;
                case LT_AddressFreeForm:
                    addressType = "freeform";
                    break;
                default:
                    addressType = "";
                    break;
            }
            address->SetType(shared_ptr<std::string>(new std::string(addressType)));
            address->SetAirport(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetAirport())));
            address->SetCity(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCity())));
            address->SetStreetAddress(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetNumber())));
            address->SetStreet(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetStreet())));
            address->SetCrossStreet(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCrossStreet())));
            address->SetCounty(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCounty())));
            address->SetState(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetState())));
            address->SetPostal(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetZipCode())));
            address->SetCountry(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCountry())));
            address->SetCountryName(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCountryName())));

            ExtendedAddressSharedPtr extendedAddress = ExtendedAddressSharedPtr(new protocol::ExtendedAddress());
            std::vector<std::string>::const_iterator it = poi.poi->GetPlace()->GetLocation()->GetExtendedAddress().GetLines().begin();
            for (; it != poi.poi->GetPlace()->GetLocation()->GetExtendedAddress().GetLines().end(); ++it)
            {
                AddressLineSharedPtr addresslines = AddressLineSharedPtr(new AddressLine());
                addresslines->SetText(shared_ptr<std::string>(new std::string(*it)));
                extendedAddress->GetAddressLineArray()->push_back(addresslines);
            }
            address->SetExtendedAddress(extendedAddress);

            CompactAddressSharedPtr compactAddress = CompactAddressSharedPtr(new protocol::CompactAddress());
            LineSeparatorSharedPtr lineSeparator = LineSeparatorSharedPtr(new protocol::LineSeparator());
            lineSeparator->SetSeparator(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCompactAddress().GetLineSeparator())));
            compactAddress->SetLineSeparator(lineSeparator);
            AddressLineSharedPtr line1 = AddressLineSharedPtr(new AddressLine());
            line1->SetText(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCompactAddress().GetLine1())));
            compactAddress->GetAddressLineArray()->push_back(line1);
            AddressLineSharedPtr line2 = AddressLineSharedPtr(new AddressLine());
            line2->SetText(shared_ptr<std::string>(new std::string(poi.poi->GetPlace()->GetLocation()->GetCompactAddress().GetLine2())));
            compactAddress->GetAddressLineArray()->push_back(line2);
            address->SetCompactAddress(compactAddress);

            
            location->SetAddress(address);

            place->SetLocation(location);
        }


        std::vector<nbsearch::Category*> localCategories = poi.poi->GetPlace()->GetCategories();
        for (int i = 0; i < (int)localCategories.size(); ++i)
        {
            CategorySharedPtr category = CategorySharedPtr(new protocol::Category());
            category->SetCategoryCode(shared_ptr<std::string>(new std::string(localCategories[i]->GetCode())));
            category->SetCategoryName(shared_ptr<std::string>(new std::string(localCategories[i]->GetName())));
            if (localCategories[i]->GetParentCategory())
            {
                CategorySharedPtr parentCategory = CategorySharedPtr(new protocol::Category());
                parentCategory->SetCategoryCode(shared_ptr<std::string>(new std::string(localCategories[i]->GetParentCategory()->GetCode())));
                parentCategory->SetCategoryName(shared_ptr<std::string>(new std::string(localCategories[i]->GetParentCategory()->GetName())));
                category->SetParentCategory(parentCategory);
            }
            place->GetCategoryArray()->push_back(category);
        }

        std::vector<nbsearch::Phone*> localPhones = poi.poi->GetPlace()->GetPhoneNumbers();
        for (int i = 0; i < (int)localPhones.size(); ++i)
        {
            PhoneSharedPtr phone = PhoneSharedPtr(new protocol::Phone);
            phone->SetNumber(shared_ptr<std::string>(new std::string(localPhones[i]->GetNumber())));
            phone->SetAreaCode(shared_ptr<std::string>(new std::string(localPhones[i]->GetArea())));
            phone->SetCountryCode(shared_ptr<std::string>(new std::string(localPhones[i]->GetCountry())));
            phone->SetExtension(shared_ptr<std::string>(new std::string(localPhones[i]->GetFormattedNumber())));
            std::string kind;
            switch (localPhones[i]->GetPhoneType())
            {
                case PT_Fax:
                    kind = "fax";
                    break;
                case PT_Mobile:
                    kind = "mobile";
                    break;
                case PT_National:
                    kind = "national";
                    break;
                case PT_Secondary:
                    kind = "secondary";
                    break;
                case PT_Primary:
                    kind = "primary";
                    break;
                default:
                    kind = "";
                    break;
            }
            phone->SetKind(shared_ptr<std::string>(new std::string(kind)));

            place->GetPhoneArray()->push_back(phone);
        }

        std::vector<nbsearch::Event*> localEvents = poi.poi->GetPlace()->GetEvents();
        for (int i = 0; i < (int)localEvents.size(); ++i)
        {
            EventSharedPtr event = EventSharedPtr(new protocol::Event());
            if (localEvents[i]->GetEventContent())
            {
                EventContentSharedPtr eventContent = EventContentSharedPtr(new protocol::EventContent());
                switch (localEvents[i]->GetEventContent()->GetType())
                {
                    case ECT_Event:
                        eventContent->SetType(shared_ptr<std::string>(new std::string("event")));
                        break;
                    case ECT_Movie:
                        eventContent->SetType(shared_ptr<std::string>(new std::string("movie")));
                    default:
                        break;
                }
                eventContent->SetEventName(shared_ptr<std::string>(new std::string(localEvents[i]->GetEventContent()->GetName())));
                //url
                UrlSharedPtr url = UrlSharedPtr(new protocol::Url());
                url->SetValue(shared_ptr<std::string>(new std::string(localEvents[i]->GetEventContent()->GetURL())));
                eventContent->SetUrl(url);
                //mpaa
                //start rating
                RatingSharedPtr rating = RatingSharedPtr(new protocol::Rating());
                rating->SetMpaa(shared_ptr<std::string>(new std::string(localEvents[i]->GetEventContent()->GetMPAARating())));
                rating->SetStarRating(localEvents[i]->GetEventContent()->GetStartRating());
                eventContent->SetRating(rating);
                //text
                FormattedTextSharedPtr formattedText = FormattedTextSharedPtr(new protocol::FormattedText());
                TextSharedPtr text = TextSharedPtr(new protocol::Text());
                text->SetData(shared_ptr<std::string>(new std::string(localEvents[i]->GetEventContent()->GetFormatedText())));
                formattedText->SetText(text);
                eventContent->SetDataToUser(formattedText);
                //fixed date
                if (localEvents[i]->GetEventContent()->GetFixedDate())
                {
                    FixedDateSharedPtr fixedDate = FixedDateSharedPtr(new protocol::FixedDate());
                    fixedDate->SetYear(localEvents[i]->GetEventContent()->GetFixedDate()->GetYear());
                    fixedDate->SetMonth(localEvents[i]->GetEventContent()->GetFixedDate()->GetMonth());
                    fixedDate->SetDay(localEvents[i]->GetEventContent()->GetFixedDate()->GetDay());
                    fixedDate->SetType(shared_ptr<std::string>(new std::string(localEvents[i]->GetEventContent()->GetFixedDate()->GetType())));
                    eventContent->SetFixedDate(fixedDate);
                }
                //eventData pair
                std::vector<StringPair*> pairs = localEvents[i]->GetEventContent()->GetPairs();
                for (int k = 0; k < (int)pairs.size(); ++i)
                {
                    PairSharedPtr pair = PairSharedPtr(new protocol::Pair());
                    pair->SetKey(shared_ptr<std::string>(new std::string(pairs[i]->GetKey())));
                    pair->SetValue(shared_ptr<std::string>(new std::string(pairs[i]->GetValue())));
                    eventContent->GetEventDataArray()->push_back(pair);
                }
                event->SetEventContent(eventContent);
            }

            std::vector<nbsearch::EventPerformance*> performances = localEvents[i]->GetEventPerformances();
            for (int j = 0; j < (int)performances.size(); ++j)
            {
                EventPerformanceSharedPtr performance = EventPerformanceSharedPtr(new protocol::EventPerformance());

                event->GetEventPerformanceArray()->push_back(performance);
            }
            place->GetEventArray()->push_back(event);
        }
    }

    std::string theaterId;
    if (poi.poi && poi.poi->GetPlace() && poi.poi->GetPlace()->GetTheaterId(theaterId))
    {
        PairSharedPtr pairTheaterID = PairSharedPtr(new protocol::Pair());
        pairTheaterID->SetKey(shared_ptr<std::string>(new std::string("theaterId")));
        pairTheaterID->SetValue(shared_ptr<std::string>(new std::string(theaterId)));
        place->GetPairArray()->push_back(pairTheaterID);
    }

    if (poi.poi && poi.poi->GetSearchFilter())
    {
        PairSharedPtr pairSearchFilter = PairSharedPtr(new protocol::Pair());
        pairSearchFilter->SetKey(shared_ptr<std::string>(new std::string("searchfilter")));
        pairSearchFilter->SetValue(shared_ptr<std::string>(new std::string(poi.poi->GetSearchFilter()->GetSerialized())));
        place->GetPairArray()->push_back(pairSearchFilter);
    }

    if (poi.userData)
    {
        std::map<std::string, std::string> udata;
        Deserialize<std::map<std::string, std::string> >(udata, poi.userData, poi.userDataSize);
        if (!udata.empty())
        {
            std::map<std::string, std::string>::iterator ite = udata.begin();
            for (; ite != udata.end(); ite++)
            {
                PairSharedPtr pairUserData = PairSharedPtr(new protocol::Pair());
                pairUserData->SetKey(shared_ptr<std::string>(new std::string(ite->first)));
                pairUserData->SetValue(shared_ptr<std::string>(new std::string(ite->second)));
                place->GetPairArray()->push_back(pairUserData);
            }
        }
        //for favorite need sync favorite order
        if (poi.type == POI_TYPE_FAVORITE)
        {
            PairSharedPtr pairOrder = PairSharedPtr(new protocol::Pair());
            pairOrder->SetKey(shared_ptr<std::string>(new std::string("favoriteOrder")));
            pairOrder->SetValue(shared_ptr<std::string>(new std::string(nbcommon::StringUtility::NumberToString<uint32>(poi.index))));
            place->GetPairArray()->push_back(pairOrder);
        }
    }

    return place;
}

void SyncPlaceManager::CCC_SyncRequest(const SyncPlaceTask* pTask)
{
    SyncRequest(pTask->m_type);
}

void SyncPlaceManager::HandlerSyncStatusReply(const protocol::SyncStatusInformationSharedPtr response)
{
    SyncStatusDBStatusReplySharedPtrList replies = response->GetSyncStatusDBStatusReplyArray();
    SyncStatusDBStatusReplySharedPtrArray::iterator it = replies->begin();
    uint32 syncType = 0;
    for (; it != replies->end(); ++it)
    {
        uint32 generationId = (*it)->GetGeneration();
        uint32 currentGenerationID = 0;
        std::string strGenID;
        const char* tmpStringGenID = NULL;
        DMPOIType type = POI_TYPE_NONE;
        if ((*it)->GetName()->compare("recents") == 0)
        {
            tmpStringGenID = LTKSPM_RecentcsGenID;
            type = POI_TYPE_RECENT;
        }
        else if ((*it)->GetName()->compare("favorites") == 0)
        {
            tmpStringGenID = LTKSPM_FavoritesGenID;
            type = POI_TYPE_FAVORITE;
        }
        else
        {
            continue;
        }
        m_poiDAO->GetStringUserData(tmpStringGenID, strGenID);
        currentGenerationID = nsl_atoi(strGenID.c_str());
        if (generationId != currentGenerationID)
        {
            syncType |= 1 << type;
        }
    }

    if (syncType && m_eventQueue)
    {
        m_eventQueue->AddTask(new SyncPlaceTask(this, &SyncPlaceManager::CCC_SyncRequest, syncType));
    }
}

void SyncPlaceManager::HandlerSyncPlacesReply(const protocol::SyncPlacesInformationSharedPtr response)
{
    SyncPlacesDBReplySharedPtrList replies = response->GetSyncPlacesDBReplyArray();
    SyncPlacesDBReplySharedPtrArray::iterator it = replies->begin();
    for (; it != replies->end(); ++it)
    {
        if ((*it)->GetSyncError())
        {
            continue;
        }

        uint32 generationId = (*it)->GetNewSyncGeneration();
        uint32 databaseID = (*it)->GetDbID();
        bool databaseUpdated = false;

        char strDBID[32] = "";
        char strGenID[32] = "";
        nsl_sprintf(strDBID, "%d", databaseID);
        nsl_sprintf(strGenID, "%d", generationId);

        DMPOIType type = POI_TYPE_NONE;
        if ((*it)->GetName()->compare("recents") == 0)
        {
            m_poiDAO->SetStringUserData(LTKSPM_RecentcsDBID, strDBID);
            m_poiDAO->SetStringUserData(LTKSPM_RecentcsGenID, strGenID);
            type = POI_TYPE_RECENT;
        }
        else if ((*it)->GetName()->compare("favorites") == 0)
        {
            m_poiDAO->SetStringUserData(LTKSPM_FavoritesGenID, strGenID);
            m_poiDAO->SetStringUserData(LTKSPM_FavoritesDBID, strDBID);
            type = POI_TYPE_FAVORITE;
        }

        UpdateLocalDBStatus(type);

        SyncAddItemSharedPtrList additems = (*it)->GetSyncAddItemArray();
        SyncAddItemSharedPtrArray::iterator additem = additems->begin();
        for (; additem != additems->end(); ++additem)
        {
            POIForSync poiForSync;
            POIImpl* poiFromServer = new POIImpl();
            std::map<std::string, std::string> udata;

            poiFromServer->m_type = POIT_Location;
            poiFromServer->m_place = new PlaceImpl((*additem)->GetPlace());

            uint32 favoriteOrder = -1;
            PairSharedPtrList pairs = (*additem)->GetPlace()->GetPairArray();
            PairSharedPtrArray::iterator pair = pairs->begin();
            for (; pair != pairs->end(); ++pair)
            {
                if ((*pair)->GetKey()->compare("theaterId") == 0)
                {
                    poiFromServer->m_place->m_theaterId = new std::string((*pair)->GetValue()->c_str());
                }
                else if ((*pair)->GetKey()->compare("searchfilter") == 0)
                {
                    SearchFilter* searchFilter = new SearchFilter();
                    searchFilter->m_serialized = (*pair)->GetValue()->c_str();
                    poiFromServer->m_searchfilter = searchFilter;
                }
                else if ((*pair)->GetKey()->compare("favoriteOrder") == 0)
                {
                    nbcommon::StringUtility::StringToNumber(*(*pair)->GetValue().get(), favoriteOrder);
                }
                else
                {
                    udata[(*pair)->GetKey()->c_str()] = (std::string)((*pair)->GetValue()->c_str());
                }
            }

            poiForSync.poi = poiFromServer;
            poiForSync.internalId = nsl_atoi((*additem)->GetId()->c_str());

            m_poiDAO->AddPOI(poiForSync.poi, type, udata);
            m_poiDAO->UpdatePOISyncStatus(poiForSync.poi, type, SYNC_STATUS_NONE, poiForSync.internalId);
            m_poiDAO->UpdatePOITimeStamp(poiForSync.poi, type, (*additem)->GetModtime());
            if ((type == POI_TYPE_FAVORITE) && (favoriteOrder != -1))
            {
                m_poiDAO->SetFavoriteOrder(poiForSync.poi, favoriteOrder);
            }

            delete poiFromServer;
            databaseUpdated = true;
        }

        SyncDeleteItemSharedPtrList deleteitems = (*it)->GetSyncDeleteItemArray();
        SyncDeleteItemSharedPtrArray::iterator deleteitem = deleteitems->begin();
        for (; deleteitem != deleteitems->end(); ++deleteitem)
        {
            m_poiDAO->RemovePOI((*deleteitem)->GetId(), type);
            databaseUpdated = true;
        }

        SyncAssignItemIdSharedPtrList assignitems = (*it)->GetSyncAssignItemIdArray();
        SyncAssignItemIdSharedPtrArray::iterator assignitem = assignitems->begin();
        for (; assignitem != assignitems->end(); ++assignitem)
        {
            m_poiDAO->UpdatePOISyncStatus(nsl_atoi((*assignitem)->GetLocalId()->c_str()), type, SYNC_STATUS_NONE, nsl_atoi((*assignitem)->GetServerId()->c_str()));
        }

        SyncModifyItemSharedPtrList modifyitems = (*it)->GetSyncModifyItemArray();
        SyncModifyItemSharedPtrArray::iterator modifyitem = modifyitems->begin();
        for (; modifyitem != modifyitems->end(); ++modifyitem)
        {
            POIForSync poiForSync;
            POIImpl* poiFromServer = new POIImpl();
            std::map<std::string, std::string> udata;

            poiFromServer->m_type = POIT_Location;
            poiFromServer->m_place = new PlaceImpl((*modifyitem)->GetPlace());
            uint32 favoriteOrder = -1;
            PairSharedPtrList pairs = (*modifyitem)->GetPlace()->GetPairArray();
            PairSharedPtrArray::iterator pair = pairs->begin();
            for (; pair != pairs->end(); ++pair)
            {
                if ((*pair)->GetKey()->compare("theaterId") == 0)
                {
                    poiFromServer->m_place->m_theaterId = new std::string((*pair)->GetValue()->c_str());
                }
                else if ((*pair)->GetKey()->compare("searchfilter") == 0)
                {
                    SearchFilter* searchFilter = new SearchFilter();
                    searchFilter->m_serialized = (*pair)->GetValue()->c_str();
                    poiFromServer->m_searchfilter = searchFilter;
                }
                else if ((*pair)->GetKey()->compare("favoriteOrder") == 0)
                {
                    nbcommon::StringUtility::StringToNumber(*(*pair)->GetValue().get(), favoriteOrder);
                }
                else
                {
                    udata[(*pair)->GetKey()->c_str()] = (std::string)((*pair)->GetValue()->c_str());
                }
            }
            poiForSync.poi = poiFromServer;

            if (m_poiDAO->CheckPOIExist(poiForSync.poi, type))
            {
                m_poiDAO->AddPOI(poiForSync.poi, type, udata);
                if ((type == POI_TYPE_FAVORITE) && (favoriteOrder != -1))
                {
                    m_poiDAO->SetFavoriteOrder(poiForSync.poi, favoriteOrder);
                }
            }
            m_poiDAO->UpdatePOISyncStatus(poiForSync.poi, type, SYNC_STATUS_NONE, (*modifyitem)->GetId());
            m_poiDAO->UpdatePOITimeStamp(poiForSync.poi, type, (*modifyitem)->GetModtime());

            delete poiFromServer;
            databaseUpdated = true;
        }

        if (databaseUpdated && m_listener)
        {
            m_listener->SyncComplete(type);
        }
    }
}

void SyncPlaceManager::UpdateLocalDBStatus(DMPOIType type)
{
    std::vector<POIForSync>::const_iterator it = m_placesForSync.begin();
    for (; it != m_placesForSync.end(); ++it)
    {
        POIForSync poiForSync = *it;
        if (poiForSync.type == type)
        {
            switch(poiForSync.syncStatus)
            {
                case SYNC_STATUS_DELETED:
                    m_poiDAO->RemovePOI(poiForSync.serverId, poiForSync.type);
                    break;
                case SYNC_STATUS_PLACE_MODIFIED:
                case SYNC_STATUS_TIME_STAMP_MODIFIED:
                case SYNC_STATUS_ADDED:
                    m_poiDAO->UpdatePOISyncStatus(poiForSync.poi, poiForSync.type, SYNC_STATUS_NONE, poiForSync.serverId);
                    break;
                default:
                    break;
            }
        }

    }
}

void SyncPlaceManager::CompleteSyncPlace(SyncPlacesAsyncCallback* callback)
{
    std::map<shared_ptr<protocol::Handler<protocol::SyncPlacesParameters, protocol::SyncPlacesInformation> >, shared_ptr<SyncPlacesAsyncCallback> >::iterator it = m_handlerPool.begin();
    for (; it != m_handlerPool.end(); ++it)
    {
        if (it->second.get() == callback)
        {
            m_handlerPool.erase(it);
            break;
        }
    }
}

void SyncPlaceManager::CompleteSyncStatus(SyncStatusAsyncCallback* callback)
{
    std::map<shared_ptr<protocol::Handler<protocol::SyncStatusParameters, protocol::SyncStatusInformation> >, shared_ptr<SyncStatusAsyncCallback> >::iterator it = m_statusHandlerPool.begin();
    for (; it != m_statusHandlerPool.end(); ++it)
    {
        if (it->second.get() == callback)
        {
            m_statusHandlerPool.erase(it);
            break;
        }
    }
}

void SyncPlaceManager::SyncDB(DMPOIType type)
{
    PAL_TimerCancel(NB_ContextGetPal(m_context), SyncTimerCallback, this);
    m_syncType |= 1 << type;
    PAL_TimerSet(NB_ContextGetPal(m_context), SYNC_TIMER_INTERVAL, SyncTimerCallback, this);
}

void SyncPlaceManager::SyncTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        SyncPlaceManager *pThis = static_cast<SyncPlaceManager*>(userData);
        if (pThis->m_syncType)
        {
            pThis->SyncRequest(pThis->m_syncType);
        }
    }
}

void SyncPlaceManager::SetListener(SyncCompleteListener *listener)
{
    m_listener = listener;
}

void SyncPlaceManager::UnCompleteSyncPlace(NB_Error error)
{
    if (m_listener)
    {
        m_listener->SyncError(error);
    }
}

//callback
SyncPlacesAsyncCallback::SyncPlacesAsyncCallback(SyncPlaceManager* syncPlacesManager)
    : m_syncPlacesManager(syncPlacesManager)
    , m_isValid(true)
{
}

void SyncPlacesAsyncCallback::Success(protocol::SyncPlacesParametersSharedPtr request,
                                      protocol::SyncPlacesInformationSharedPtr response)
{
    if (m_isValid)
    {
        m_syncPlacesManager->HandlerSyncPlacesReply(response);
    }

    m_syncPlacesManager->CompleteSyncPlace(this);
}

void SyncPlacesAsyncCallback::Error(protocol::SyncPlacesParametersSharedPtr request, NB_Error error)
{
    if (error == NESERVERX_EXTERNAL_REQUEST_TIMEOUT)
    {
        //@TODO: need retry ??
    }
    if (error == NESERVERX_UNAUTHORIZED_ACCESS)
    {
        m_syncPlacesManager->UnCompleteSyncPlace(error);
    }
    m_syncPlacesManager->CompleteSyncPlace(this);
}

bool SyncPlacesAsyncCallback::Progress(int percentage)
{
    return false;
}

SyncStatusAsyncCallback::SyncStatusAsyncCallback(SyncPlaceManager* syncPlacesManager)
    : m_syncPlacesManager(syncPlacesManager)
    , m_isValid(true)
{
}

void SyncStatusAsyncCallback::Success(protocol::SyncStatusParametersSharedPtr request,
                                      protocol::SyncStatusInformationSharedPtr response)
{
    if (m_isValid)
    {
        m_syncPlacesManager->HandlerSyncStatusReply(response);
    }
    m_syncPlacesManager->CompleteSyncStatus(this);
}

void SyncStatusAsyncCallback::Error(protocol::SyncStatusParametersSharedPtr request, NB_Error error)
{
    if (error == NESERVERX_UNAUTHORIZED_ACCESS)
    {
        m_syncPlacesManager->UnCompleteSyncPlace(error);
    }
    m_syncPlacesManager->CompleteSyncStatus(this);
}

bool SyncStatusAsyncCallback::Progress(int percentage)
{
    return false;
}


/*! @} */
