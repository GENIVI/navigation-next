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
    @file     SingleSearchInformation.cpp
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

#include "paltypes.h"
#include "nberror.h"
#include "NBProtocolSearchFilterSerializer.h"
#include "NBProtocolSuggestListSerializer.h"
#include "NBProtocolBinaryDataSerializer.h"
#include "NBProtocolPoiAttributeCookieSerializer.h"
#include "NBProtocolPlaceEventCookieSerializer.h"
#include "NBProtocolGoldenCookieSerializer.h"
#include "NBProtocolResultStyle.h"
#include "nbpersistentdataprotected.h"
#include "nbpersistentdata.h"
#include "SingleSearchOnboardInformation.h"
#include "cslutil.h"

using namespace nbsearch;

//namespace nbsearch
//{
//const char* POI_CONTENT_KEY_CUISINES          = "cuisines";
//const char* POI_CONTENT_KEY_DESCRIPTION       = "description";
//const char* POI_CONTENT_KEY_FEATURES          = "features";
//const char* POI_CONTENT_KEY_HOURSOFOPERATION  = "hours-of-operation";
//const char* POI_CONTENT_KEY_PARKING           = "parking";
//const char* POI_CONTENT_KEY_PRICE             = "price";
//const char* POI_CONTENT_KEY_PAYEMENTMETHOD    = "payment-method";
//const char* POI_CONTENT_KEY_RESERVATIONS      = "reservations";
//const char* POI_CONTENT_KEY_SITEURL           = "site-url";
//const char* POI_CONTENT_KEY_SPECIALFEATURES   = "special-features";
//const char* POI_CONTENT_KEY_PHOTOURL          = "photo-url";
//const char* POI_CONTENT_KEY_THUMBNAILPHOTOURL = "thumbnail-photo-url";
//const char* POI_CONTENT_KEY_WIFI              = "wifi";
//const char* POI_CONTENT_KEY_DRESSCODE         = "dress-code";
//const char* POI_CONTENT_KEY_SMOKING           = "smoking";
//const char* POI_CONTENT_KEY_SHUTTLESERVICE    = "shuttle-service";
//const char* POI_CONTENT_KEY_TIPS              = "tips";
//}

#define SAFE_DELETE(param)       \
        delete param;            \
        param = NULL;


static std::string UtfToStdString(const Ncdb::UtfString& s)
{
    const char* t;
    t = s.GetBuffer();
    std::string x = (t != 0) ? t : "";
    return x;
}

template <typename T>
void OnboardDeleteVector(std::vector<T*>& param)
{
    for (typename std::vector<T*>::const_iterator it = param.begin(); it != param.end(); ++it)
    {
        delete *it;
    }
    param.clear();
}

std::string nbsearch::OnboardCreateBased64DataByTpsElement(const protocol::TpsElementPtr& tpsElem)
{
    std::string result;

    NB_PersistentData* persistentData = NULL;
    tpselt cElement = tpsElem->GetTPSElement();
    NB_Error err = NB_PersistentDataCreateByTpsElement(cElement, &persistentData);
    te_dealloc(cElement);
    cElement = NULL;
    if (err == NE_OK)
    {
        uint8* serializedData = NULL;
        nb_size serializedDataSize = 0;
        err = NB_PersistentDataSerialize(persistentData, (const uint8**)&serializedData, &serializedDataSize);
        NB_PersistentDataDestroy(persistentData);

        if (err == NE_OK)
        {
            char* encodedSerializedData = NULL;
            err = encode_base64((const char*)serializedData, serializedDataSize, &encodedSerializedData);
            if (err == NE_OK)
            {
                result = std::string(encodedSerializedData);
                nsl_free(encodedSerializedData);
            }

            nsl_free(serializedData);
        }
    }

    return result;
}

protocol::TpsElementPtr nbsearch::OnboardCreateTpsElementByBase64Data(const std::string& serializedData)
{
    protocol::TpsElementPtr tpsElement;

    if (serializedData.empty())
    {
        return tpsElement;
    }

    uint8* decodedSerializedData = (uint8*)nsl_malloc(serializedData.length()); // Base64-encoded data size is about 135.1% of Original data size. (See also Wiki)
    // So, serializedData.length() is enough for decoded data.
    if (decodedSerializedData == NULL)
    {
        return tpsElement;
    }

    nsl_memset(decodedSerializedData, 0, serializedData.length());
    int decodeSerializedDataSize = decode_base64(serializedData.c_str(), (char*)decodedSerializedData);
    if (decodeSerializedDataSize == -1)
    {
        nsl_free(decodedSerializedData);
        return tpsElement;
    }

    NB_PersistentData* persistentData = NULL;
    NB_Error err = NB_PersistentDataCreate(decodedSerializedData, decodeSerializedDataSize, NULL, &persistentData);
    nsl_free(decodedSerializedData);

    if (err != NE_OK)
    {
        return tpsElement;
    }

    tpselt tps = NULL;
    err = NB_PersistentDataGetToTpsElement(persistentData, &tps);
    if (err != NE_OK)
    {
        NB_PersistentDataDestroy(persistentData);
        return tpsElement;
    }

    tpsElement.reset(new protocol::TpsElement(tps));

    NB_PersistentDataDestroy(persistentData);
    return tpsElement;
}

protocol::SearchFilterSharedPtr nbsearch::OnboardCreateSearchFilterInProtocolBySearchFilter(const nbsearch::SearchFilter& searchFilter)
{
    protocol::SearchFilterSharedPtr searchFilterInProtocol(new protocol::SearchFilter());
    if (searchFilter.GetSerialized().empty())
    {
        const std::vector<StringPair>& pairs = searchFilter.GetPairs();
        std::vector<StringPair>::const_iterator pairIter = pairs.begin();
        for (; pairIter != pairs.end(); ++pairIter) {
            protocol::PairSharedPtr pair(new protocol::Pair());
            pair->SetKey(CHAR_PTR_TO_UTF_STRING_PTR((*pairIter).GetKey()));
            pair->SetValue(CHAR_PTR_TO_UTF_STRING_PTR((*pairIter).GetValue()));
            searchFilterInProtocol->GetSearchKeyArray()->push_back(pair);
        }
        protocol::ResultStyleSharedPtr resultStyle(new protocol::ResultStyle());
        resultStyle->SetKey(CHAR_PTR_TO_UTF_STRING_PTR(searchFilter.GetResultStyle()));
        searchFilterInProtocol->SetResultStyle(resultStyle);
    }
    else
    {
        searchFilterInProtocol = protocol::SearchFilterSerializer::deserialize(nbsearch::OnboardCreateTpsElementByBase64Data(searchFilter.GetSerialized()));
    }

    return searchFilterInProtocol;
}

protocol::SuggestMatchSharedPtr nbsearch::OnboardCreateSuggestMatchInProtocolBySuggestionMatch(const nbsearch::SuggestionMatch& suggestionMatch)
{
    protocol::SuggestMatchSharedPtr suggestMatch(new protocol::SuggestMatch());

    // Line 1, 2, 3
    suggestMatch->SetDistance(static_cast<float>(suggestionMatch.GetDistance()));
    suggestMatch->SetLine1(CHAR_PTR_TO_UTF_STRING_PTR(suggestionMatch.GetLine1()));
    suggestMatch->SetLine2(CHAR_PTR_TO_UTF_STRING_PTR(suggestionMatch.GetLine2()));
    suggestMatch->SetLine3(CHAR_PTR_TO_UTF_STRING_PTR(suggestionMatch.GetLine3()));

    // Match Type
    SuggestionMatchType smt = suggestionMatch.GetMatchType();
    switch (smt)
    {
        case SMT_POI:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("poi"));
            break;
        case SMT_Address:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("address"));
            break;
        case SMT_Airport:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("airport"));
            break;
        case SMT_Category:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("category"));
            break;
        case SMT_Gas:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("gas"));
            break;
        case SMT_Local:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("local"));
            break;
        case SMT_Movie:
            suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR("movie"));
            break;
        default:
            break;
    }

    // Icon IDs
    const std::vector<std::string>& iconIds = suggestionMatch.GetIconIDs();
    std::vector<std::string>::const_iterator iconIdIter = iconIds.begin();
    for (; iconIdIter != iconIds.end(); ++iconIdIter)
    {
        protocol::PairSharedPtr pair(new protocol::Pair());
        pair->SetKey(CHAR_PTR_TO_UTF_STRING_PTR("icon-id"));
        pair->SetValue(CHAR_PTR_TO_UTF_STRING_PTR(*iconIdIter));

        suggestMatch->GetPairArray()->push_back(pair);
    }

    // Search Filter
    suggestMatch->SetSearchFilter(nbsearch::OnboardCreateSearchFilterInProtocolBySearchFilter(*suggestionMatch.GetSearchFilter()));

    return suggestMatch;
}

protocol::SuggestListSharedPtr nbsearch::OnboardCreateSuggestListInProtocolBySuggestionList(const nbsearch::SuggestionList& suggestionList)
{
    protocol::SuggestListSharedPtr suggestList(new protocol::SuggestList());

    suggestList->SetName(CHAR_PTR_TO_UTF_STRING_PTR(suggestionList.GetName()));

    const std::vector<SuggestionMatch*>& suggestionMatches = suggestionList.GetSuggestionMatches();
    std::vector<SuggestionMatch*>::const_iterator suggestionMatchIter = suggestionMatches.begin();

    for (; suggestionMatchIter != suggestionMatches.end(); ++suggestionMatchIter)
    {
        suggestList->GetSuggestMatchArray()->push_back(nbsearch::OnboardCreateSuggestMatchInProtocolBySuggestionMatch(**suggestionMatchIter));
    }

    return suggestList;
}

PoiAttributeCookie
OnboardPoiAttributeCookieUtil::toPoiAttributeCookie(const protocol::PoiAttributeCookieSharedPtr& cookie)
{
    PoiAttributeCookie result;
    result.m_provideId = cookie->GetProviderId()->c_str();
    result.m_state = std::string((const char*)cookie->GetState()->GetData());
    result.m_serialized = Serialize(cookie);
    return result;
}

std::string OnboardPoiAttributeCookieUtil::Serialize(const protocol::PoiAttributeCookieSharedPtr& poiAttributeCookieProtocol)
{
    return std::string(nbsearch::OnboardCreateBased64DataByTpsElement(protocol::PoiAttributeCookieSerializer::serialize(poiAttributeCookieProtocol)));
}

protocol::PoiAttributeCookieSharedPtr OnboardPoiAttributeCookieUtil::Deserialize(const std::string& serializedData)
{
    return protocol::PoiAttributeCookieSerializer::deserialize(nbsearch::OnboardCreateTpsElementByBase64Data(serializedData));
}

StringPair
OnboardStringPairUtil::toStringPair(const protocol::PairSharedPtr &pair)
{
    StringPair result;
    result.m_key = *(pair->GetKey());
    result.m_value = *(pair->GetValue());
    if(pair->GetPoiAttributeCookie())
    {
        result.m_cookie = OnboardPoiAttributeCookieUtil::toPoiAttributeCookie(pair->GetPoiAttributeCookie());
    }
    return result;
}

OnboardPlaceEventCookieImpl::OnboardPlaceEventCookieImpl(const PlaceEventCookie& copy)
    : m_provideId(copy.GetProviderId()),
      m_state(copy.GetState()),
      m_serialized(copy.GetSerialized())
{
}

OnboardPlaceEventCookieImpl::OnboardPlaceEventCookieImpl(const OnboardPlaceEventCookieImpl& copy)
    : m_provideId(copy.m_provideId),
      m_state(copy.m_state),
      m_serialized(copy.m_serialized)
{
}

OnboardPlaceEventCookieImpl::OnboardPlaceEventCookieImpl(protocol::PlaceEventCookieSharedPtr cookie)
    : m_provideId(cookie->GetProvider()->c_str()),
      m_state((const char*)cookie->GetState()->GetData()),
      m_serialized(Serialize(cookie))
{
}

std::string OnboardPlaceEventCookieImpl::Serialize(const protocol::PlaceEventCookieSharedPtr& placeEventCookieProtocol)
{
    return std::string(nbsearch::OnboardCreateBased64DataByTpsElement(protocol::PlaceEventCookieSerializer::serialize(placeEventCookieProtocol)));
}

protocol::PlaceEventCookieSharedPtr OnboardPlaceEventCookieImpl::Deserialize(const std::string& serializedData)
{
    return protocol::PlaceEventCookieSerializer::deserialize(nbsearch::OnboardCreateTpsElementByBase64Data(serializedData));
}

OnboardCategoryImpl::OnboardCategoryImpl(const Category& copy)
: m_code(copy.GetCode()), m_name(copy.GetName())
{
    m_parent = copy.GetParentCategory() != NULL ? new OnboardCategoryImpl(*copy.GetParentCategory()) : NULL;
}


OnboardCategoryImpl::OnboardCategoryImpl(const OnboardCategoryImpl& copy)
    : m_code(copy.GetCode()), m_name(copy.GetName())
{
    m_parent = copy.m_parent != NULL ? new OnboardCategoryImpl(*copy.m_parent) : NULL;
}

OnboardCategoryImpl::OnboardCategoryImpl(protocol::CategorySharedPtr category)
    : m_code(*category->GetCategoryCode()), m_name(*category->GetCategoryName()), m_parent(NULL)
{
}

OnboardVendorContentImpl::~OnboardVendorContentImpl()
{
    OnboardDeleteVector<StringPair>(m_pairs);
}

OnboardVendorContentImpl::OnboardVendorContentImpl(protocol::VendorContentSharedPtr vendorContent)
    : m_name(""), m_ratingCount(0), m_averageRating(0.0)
{
    if (vendorContent->GetPairArray()->size() == 0)
    {
        return;
    }
    m_name = *vendorContent->GetName();
    if (vendorContent->GetOverAllRating())
    {
        m_ratingCount = vendorContent->GetOverAllRating()->GetTotalUsersRated();
        m_averageRating = vendorContent->GetOverAllRating()->GetAverageRating();
    }
    shared_ptr<std::vector<shared_ptr<protocol::Pair> > > pairs = vendorContent->GetPairArray();
    std::vector<shared_ptr<protocol::Pair> >::iterator it = pairs->begin();
    for (; it != pairs->end(); ++it)
    {
        m_pairs.push_back(new StringPair(OnboardStringPairUtil::toStringPair(*it)));
    }
}

OnboardVendorContentImpl::OnboardVendorContentImpl(const VendorContent& copy)
: m_name(copy.GetName()), m_ratingCount(copy.GetRatingCount()), m_averageRating(copy.GetAverageRating())
{
    if (copy.GetPairs().size() == 0)
    {
        return;
    }
    for (std::vector<StringPair*>::const_iterator iter = copy.GetPairs().begin(); iter != copy.GetPairs().end(); iter++)
    {
        m_pairs.push_back(new StringPair(**iter));
    }
}

SearchFilter
OnboardSearchFilterUtil::toSearchFilter(const protocol::SearchFilterSharedPtr &searchFilterProtocol)
{
    SearchFilter result;
    if(searchFilterProtocol->GetResultStyle() && searchFilterProtocol->GetResultStyle()->GetKey())
    {
        result.m_resultStyle = *(searchFilterProtocol->GetResultStyle()->GetKey());
    }
    if(searchFilterProtocol->GetSearchKeyArray())
    {
        for(size_t i = 0; i < searchFilterProtocol->GetSearchKeyArray()->size(); i++)
        {
            result.m_pairs.push_back(OnboardStringPairUtil::toStringPair(searchFilterProtocol->GetSearchKeyArray()->at(i)));
        }
    }
    result.m_serialized = Serialize(searchFilterProtocol);
    return result;
}

std::string OnboardSearchFilterUtil::Serialize(const protocol::SearchFilterSharedPtr& searchFilterProtocol)
{
    return std::string(nbsearch::OnboardCreateBased64DataByTpsElement(protocol::SearchFilterSerializer::serialize(searchFilterProtocol)));
}

protocol::SearchFilterSharedPtr OnboardSearchFilterUtil::Deserialize(const std::string& serializedData)
{
    return protocol::SearchFilterSerializer::deserialize(nbsearch::OnboardCreateTpsElementByBase64Data(serializedData));
}

OnboardRelatedSearchImpl::OnboardRelatedSearchImpl(protocol::RelatedSearchSharedPtr relatedSearch)
    : m_searchFilter(new SearchFilter(OnboardSearchFilterUtil::toSearchFilter(relatedSearch->GetSearchFilter()))),
      m_line1(*relatedSearch->GetLine1())
{
}

OnboardRelatedSearchImpl::OnboardRelatedSearchImpl(const OnboardRelatedSearchImpl& copy)
    : m_searchFilter(new SearchFilter(*copy.m_searchFilter)),
      m_line1(copy.m_line1)
{
}

OnboardRelatedSearchImpl::OnboardRelatedSearchImpl(const RelatedSearch& copy)
    : m_searchFilter(new SearchFilter(*copy.GetSearchFilter())),
      m_line1(copy.GetLine1())
{
}

OnboardRelatedSearchImpl::~OnboardRelatedSearchImpl()
{
    delete m_searchFilter;
    m_searchFilter = NULL;
}

OnboardResultDescriptionImpl::OnboardResultDescriptionImpl(protocol::ResultDescriptionSharedPtr resultDescription)
    : m_line1(*resultDescription->GetLine1()),
      m_line2(*resultDescription->GetLine2())
{
}

OnboardTimeRangeImpl::OnboardTimeRangeImpl(const OnboardTimeRangeImpl& copy)
    : m_dayOfWeek(copy.GetDayOfWeek()),
      m_startTime(copy.GetStartTime()),
      m_endTime(copy.GetEndTime())
{
}

OnboardTimeRangeImpl::OnboardTimeRangeImpl(protocol::TimeRangeSharedPtr timeRange)
    : m_dayOfWeek(timeRange->GetDayOfWeek()),
      m_startTime(timeRange->GetStartTime()),
      m_endTime(timeRange->GetEndTime())
{
}

OnboardFixedDateImpl::OnboardFixedDateImpl(const protocol::FixedDateSharedPtr fixedDate)
    : m_type(*fixedDate->GetType()),
      m_year(fixedDate->GetYear()),
      m_month(fixedDate->GetMonth()),
      m_day(fixedDate->GetDay())
{
}

OnboardHoursOfOperationImpl::OnboardHoursOfOperationImpl(protocol::HoursOfOperationSharedPtr hourOfOperation)
    : m_hasUtcOffset(false),
      m_utcOffset(0)
{
    shared_ptr<std::vector<shared_ptr<protocol::TimeRange> > > timeRanges = hourOfOperation->GetTimeRangeArray();
    std::vector<shared_ptr<protocol::TimeRange> >::iterator it = timeRanges->begin();
    for (; it != timeRanges->end(); ++it)
    {
        OnboardTimeRangeImpl* timeRange = new OnboardTimeRangeImpl(*it);
        m_timeRanges.push_back(timeRange);
    }

    m_hasUtcOffset = hourOfOperation->GetUtcOffset()->size() > 0 ? true : false;
    if (m_hasUtcOffset)
    {
        m_utcOffset = nsl_atoi(hourOfOperation->GetUtcOffset()->c_str());
    }
}

OnboardHoursOfOperationImpl::~OnboardHoursOfOperationImpl()
{
    OnboardDeleteVector<TimeRange>(m_timeRanges);
}

OnboardHoursOfOperationImpl::OnboardHoursOfOperationImpl(const HoursOfOperation& copy)
{
    m_utcOffset = 0;
    m_hasUtcOffset = copy.GetUTCOffset(m_utcOffset);
    copy.GetUTCOffset(m_utcOffset);
    for (std::vector<TimeRange*>::const_iterator iter = copy.GetTimeRanges().begin(); iter != copy.GetTimeRanges().end(); iter++)
    {
        m_timeRanges.push_back(new OnboardTimeRangeImpl(*(OnboardTimeRangeImpl*)*iter));
    }
}

OnboardHoursOfOperationImpl::OnboardHoursOfOperationImpl(const OnboardHoursOfOperationImpl& copy)
    : m_hasUtcOffset(copy.m_hasUtcOffset), m_utcOffset(0)
{
    copy.GetUTCOffset(m_utcOffset);
    for (std::vector<TimeRange*>::const_iterator iter = copy.GetTimeRanges().begin(); iter != copy.GetTimeRanges().end(); iter++)
    {
        m_timeRanges.push_back(new OnboardTimeRangeImpl(*(OnboardTimeRangeImpl*)*iter));
    }
}

bool OnboardHoursOfOperationImpl::GetUTCOffset(uint32& utcoffset) const
{
    if (!m_hasUtcOffset)
    {
        return false;
    }
    utcoffset = m_utcOffset;
    return true;
}

OnboardEventContentImpl::OnboardEventContentImpl(protocol::EventContentSharedPtr eventContent)
    : m_name(*eventContent->GetEventName()),
      m_url(eventContent->GetUrl() ? *eventContent->GetUrl()->GetValue() : ""),
      m_mpaa(eventContent->GetRating() ? *eventContent->GetRating()->GetMpaa() : ""),
      m_text(eventContent->GetDataToUser() ? ( eventContent->GetDataToUser()->GetText() ? *eventContent->GetDataToUser()->GetText()->GetData() : "") : ""),
      m_startRating(eventContent->GetRating() ? eventContent->GetRating()->GetStarRating() : 0),
      m_fixDate(eventContent->GetFixedDate() ? new OnboardFixedDateImpl(eventContent->GetFixedDate()) : NULL),
      m_cookie(eventContent->GetPlaceEventCookie() ? new OnboardPlaceEventCookieImpl(eventContent->GetPlaceEventCookie()) : NULL)
{
    if (eventContent->GetType()->compare("movie") == 0)
    {
        m_type = ECT_Movie;
    }
    else if (eventContent->GetType()->compare("event") == 0)
    {
        m_type = ECT_Event;
    }
    protocol::PairSharedPtrList pairs = eventContent->GetEventDataArray();
    protocol::PairSharedPtrArray::iterator it = pairs->begin();
    for (; it != pairs->end(); ++it)
    {
        m_pairs.push_back(new StringPair(OnboardStringPairUtil::toStringPair(*it)));
    }
}

OnboardEventContentImpl::~OnboardEventContentImpl()
{
    OnboardDeleteVector<Category>(m_categories);
    OnboardDeleteVector<StringPair>(m_pairs);
    SAFE_DELETE(m_fixDate);
    SAFE_DELETE(m_cookie);
}

OnboardEventPerformanceImpl::OnboardEventPerformanceImpl(const protocol::EventPerformanceSharedPtr eventPerformace)
    : m_startTime(eventPerformace->GetStartTime()),
      m_endTime(eventPerformace->GetEndTime()),
      m_utcOffset(nsl_atoi(eventPerformace->GetUtcOffset()->c_str()))
{
}

OnboardEventImpl::OnboardEventImpl(const protocol::EventSharedPtr event)
    : m_eventContent(new OnboardEventContentImpl(event->GetEventContent()))
{
    if (!event->GetEventPerformanceArray()->empty())
    {
        protocol::EventPerformanceSharedPtrList performances = event->GetEventPerformanceArray();
        protocol::EventPerformanceSharedPtrArray::iterator it = performances->begin();
        for (; it != performances->end(); ++it)
        {
            m_eventPerformances.push_back(new OnboardEventPerformanceImpl(*it));
        }
    }
}

OnboardEventImpl::~OnboardEventImpl()
{
    SAFE_DELETE(m_eventContent);
    OnboardDeleteVector<EventPerformance>(m_eventPerformances);
}

OnboardProxMatchContentImpl::OnboardProxMatchContentImpl(protocol::ProxMatchContentSharedPtr proxMatchContent)
    : m_searchFilter(NULL),
      m_eventContent(NULL),
      m_relatedSearch(NULL)
{
    if (proxMatchContent->GetSearchFilter())
    {
        m_searchFilter = new SearchFilter(OnboardSearchFilterUtil::toSearchFilter(proxMatchContent->GetSearchFilter()));
    }

    if (proxMatchContent->GetRelatedSearch())
    {
        m_relatedSearch = new OnboardRelatedSearchImpl(proxMatchContent->GetRelatedSearch());
    }

    if (proxMatchContent->GetEventContent())
    {
        m_eventContent = new OnboardEventContentImpl(proxMatchContent->GetEventContent());
    }
}

OnboardProxMatchContentImpl::~OnboardProxMatchContentImpl()
{
    SAFE_DELETE(m_searchFilter);
    SAFE_DELETE(m_eventContent);
    SAFE_DELETE(m_relatedSearch);
}

OnboardPhoneImpl::OnboardPhoneImpl(const Phone& copy)
    : m_type(copy.GetPhoneType()),
      m_country(copy.GetCountry()),
      m_area(copy.GetArea()),
      m_number(copy.GetNumber()),
      m_formattedNumber(copy.GetFormattedNumber())
{
}

OnboardPhoneImpl::OnboardPhoneImpl(const OnboardPhoneImpl& copy)
    : m_type(copy.GetPhoneType()),
      m_country(copy.GetCountry()),
      m_area(copy.GetArea()),
      m_number(copy.GetNumber()),
      m_formattedNumber(copy.GetFormattedNumber())
{
}

OnboardPhoneImpl::OnboardPhoneImpl(const Ncdb::UtfString& phone)
    : m_country(""),
      m_area(""),
      m_number(""),
      m_formattedNumber(UtfToStdString(phone))
{
    m_type = PT_Primary;
}

OnboardAddressImpl::OnboardAddressImpl(const OnboardAddressImpl& copy)
    : m_number(copy.GetNumber()),
      m_street(copy.GetStreet()),
      m_city(copy.GetCity()),
      m_county(copy.GetCounty()),
      m_state(copy.GetState()),
      m_zipcode(copy.GetZipCode()),
      m_country(copy.GetCountry())
{
}

ExtendedAddress
OnboardExtendedAddressUtil::toExtendedAddress(const Ncdb::FormattedAddressAutoArray& extendedAddress)
{
    ExtendedAddress result;
    int count = extendedAddress.size();
    for (int i = 0; i < count; i++) {
        result.m_lines.push_back(UtfToStdString(extendedAddress[i]));
    }
    return result;
}

CompactAddress
OnboardCompactAddressUtil::toCompactAddress(const Ncdb::FormattedAddressAutoArray& compactAddress)
{
    CompactAddress result;
    int size = compactAddress.size();
    // sanity check
    if (size > 2) size = 2;
    std::string lines[2] = {""};
    for (int i = 0; i < size; ++i)
    {
        lines[i] = UtfToStdString(compactAddress[i]);
    }
    result.m_lineSeparator = " ";
    result.m_addressLine += lines[0] + (lines[1].length() > 0 ? ", " + lines[1] : "");
    result.m_line1 = lines[0];
    result.m_line2 = lines[1];
    return result;
}

bool
OnboardLocationUtil::isEqual(const Location &location1, const Location &location2)
{
    bool result = false;
    result = location1.GetCoordinates().GetLatitude() == location2.GetCoordinates().GetLatitude() &&
             location1.GetCoordinates().GetLongitude() == location2.GetCoordinates().GetLongitude();
    result = result &&
             location1.GetName() == location2.GetName() &&
             location1.GetNumber() == location2.GetNumber() &&
             location1.GetStreet() == location2.GetStreet() &&
             location1.GetCrossStreet() == location2.GetCrossStreet() &&
             location1.GetCity() == location2.GetCity() &&
             location1.GetCounty() == location2.GetCounty() &&
             location1.GetState() == location2.GetState() &&
             location1.GetZipCode() == location2.GetZipCode() &&
             location1.GetCountry() == location2.GetCountry() &&
             location1.GetAirport() == location2.GetAirport() &&
             location1.GetFreeForm() == location2.GetFreeForm() &&
             location1.GetCountryName() == location2.GetCountryName();
    return result;
}

Location*
OnboardLocationUtil::toLocation(Ncdb::SearchMatch* sm)
{
    Location* result = new Location();
    result->m_type = LT_Address;
    result->m_point = LatLonPoint(sm->GeocodedLocation.y, sm->GeocodedLocation.x);
    result->m_name = UtfToStdString(sm->MatchName);
    if (sm->MatchType == Ncdb::GCMatch_Airport) {
        result->m_name = UtfToStdString(sm->AddressOut.AirportName);
        result->m_type = LT_Airport;
    }
    result->m_number = UtfToStdString(sm->AddressOut.StartNumber);
    result->m_street = UtfToStdString(sm->AddressOut.StreetName);
    result->m_crossStreet = UtfToStdString(sm->AddressOut.CrossStreet);
    result->m_city = UtfToStdString(sm->AddressOut.City);
    if (result->m_city == "") {
        result->m_city = UtfToStdString(sm->AddressOut.OutAdmin);
    }
    result->m_county = UtfToStdString(sm->AddressOut.County);
    result->m_state = UtfToStdString(sm->AddressOut.State);
    if (result->m_state == "") {
        result->m_state = UtfToStdString(sm->AddressOut.OutState);
    }
    result->m_zipcode = UtfToStdString(sm->AddressOut.Postal);
    result->m_country = UtfToStdString(sm->AddressOut.Country);
    result->m_airport = UtfToStdString(sm->AddressOut.AirportCode);
    result->m_countryName = UtfToStdString(sm->AddressOut.Country);
    result->m_freeForm = UtfToStdString(sm->AddressOut.FormattedAddress);
    if (sm->AddressOut.compactFormattedAddress.size()) {
        result->m_compAddress = OnboardCompactAddressUtil::toCompactAddress(sm->AddressOut.compactFormattedAddress);
    }
    if (sm->AddressOut.extendedFormattedAddress.size()) {
        result->m_extAddress = OnboardExtendedAddressUtil::toExtendedAddress(sm->AddressOut.extendedFormattedAddress);
    }
/*    typedef enum search_type
    {
        SIMPLE_SUGGEST	= 0x0,						//suggestions for both POI and Streets
        SIMPLE_SEARCH	= 0x1,						//search for both POI and Streets
        GEOCODE_SEARCH	= 0x2,						//Geocode only search: can also include admin based search
        POI_SEARCH		= 0x3,						//POI only search:can also include airports
        AIRPORT_SEARCH	= 0x4,						//airport only search: Planned for future
        GEOCODE_SUGGEST = 0x7,						//Geocode only suggest: can also include admin based search
        POI_SUGGEST		= 0x8						//POI only suggest:can also include airports
    } SEARCH_TYPE_VALUES;

    if (location->GetAddress()->GetType()->compare("address") == 0)
    {
        result.m_type = LT_Address;
    }
    else if (location->GetAddress()->GetType()->compare("intersect") == 0)
    {
        result.m_type = LT_Intersection;
    }
    else if (location->GetAddress()->GetType()->compare("airport") == 0)
    {
        result.m_type = LT_Airport;
    }
    else if (location->GetAddress()->GetType()->compare("freeform") == 0)
    {
        result.m_type = LT_AddressFreeForm;
    }
    if (location->GetAddress()->GetExtendedAddress() != NULL)
    {
        result.m_extAddress = OnboardExtendedAddressUtil::toExtendedAddress(location->GetAddress()->GetExtendedAddress());
    }
    if (location->GetAddress()->GetCompactAddress() != NULL)
    {
        result.m_compAddress = OnboardCompactAddressUtil::toCompactAddress(location->GetAddress()->GetCompactAddress());
    }
*/
    return result;
}

OnboardWeatherConditionsImpl::OnboardWeatherConditionsImpl()
{
}

OnboardWeatherConditionsImpl::OnboardWeatherConditionsImpl(const WeatherConditions& copy)
    : m_temperature(copy.GetTemperature()),
      m_dewPoint(copy.GetDewPoint()),
      m_relHumidity(copy.GetRelHumidity()),
      m_windSpeed(copy.GetWindSpeed()),
      m_windDirection(copy.GetWindDirection()),
      m_windGust(copy.GetWindGust()),
      m_pressure(copy.GetPressure()),
      m_condition(copy.GetWeatherCondition()),
      m_conditonCode(copy.GetConditionCode()),
      m_sky(copy.GetSkyCover()),
      m_ceiling(copy.GetCloudBaseHeight()),
      m_visibility(copy.GetVisibility()),
      m_heatIndex(copy.GetHeatIndex()),
      m_windChill(copy.GetWindChill()),
      m_snowDepth(copy.GetSnowDepth()),
      m_maxTemp6Hour(copy.GetMaxTempInLast6Hours()),
      m_maxTemp24Hour(copy.GetMaxTempInLast24Hours()),
      m_minTemp6Hour(copy.GetMinTempInLast6Hours()),
      m_minTemp24Hour(copy.GetMinTempInLast24Hours()),
      m_precipitation3Hour(copy.GetPrecipitatinInLast3Hours()),
      m_precipitation6Hour(copy.GetPrecipitatinInLast6Hours()),
      m_precipitation24Hour(copy.GetPrecipitationInLast24Hours()),
      m_updateTime(copy.GetUpdateTime()),
      m_utcOffset(copy.GetUTCOffset())
{
}

OnboardWeatherConditionsImpl::OnboardWeatherConditionsImpl(const OnboardWeatherConditionsImpl& copy)
    : m_temperature(copy.GetTemperature()),
      m_dewPoint(copy.GetDewPoint()),
      m_relHumidity(copy.GetRelHumidity()),
      m_windSpeed(copy.GetWindSpeed()),
      m_windDirection(copy.GetWindDirection()),
      m_windGust(copy.GetWindGust()),
      m_pressure(copy.GetPressure()),
      m_condition(copy.GetWeatherCondition()),
      m_conditonCode(copy.GetConditionCode()),
      m_sky(copy.GetSkyCover()),
      m_ceiling(copy.GetCloudBaseHeight()),
      m_visibility(copy.GetVisibility()),
      m_heatIndex(copy.GetHeatIndex()),
      m_windChill(copy.GetWindChill()),
      m_snowDepth(copy.GetSnowDepth()),
      m_maxTemp6Hour(copy.GetMaxTempInLast6Hours()),
      m_maxTemp24Hour(copy.GetMaxTempInLast24Hours()),
      m_minTemp6Hour(copy.GetMinTempInLast6Hours()),
      m_minTemp24Hour(copy.GetMinTempInLast24Hours()),
      m_precipitation3Hour(copy.GetPrecipitatinInLast3Hours()),
      m_precipitation6Hour(copy.GetPrecipitatinInLast6Hours()),
      m_precipitation24Hour(copy.GetPrecipitationInLast24Hours()),
      m_updateTime(copy.GetUpdateTime()),
      m_utcOffset(copy.GetUTCOffset())
{
}

OnboardWeatherConditionsImpl::OnboardWeatherConditionsImpl(protocol::WeatherConditionsSharedPtr conditons)
    : m_temperature(conditons->GetTemperature()),
      m_dewPoint(conditons->GetDewPoint()),
      m_relHumidity(conditons->GetRelHumidity()),
      m_windSpeed(conditons->GetWindSpeed()),
      m_windDirection(conditons->GetWindDirection()),
      m_windGust(conditons->GetWindGust()),
      m_pressure(conditons->GetPressure()),
      m_condition(conditons->GetWeatherCondition()->c_str()),
      m_conditonCode(conditons->GetConditionCode()),
      m_sky(conditons->GetSkyCover()->c_str()),
      m_ceiling(conditons->GetCloudBaseHeight()),
      m_visibility(conditons->GetVisibility()),
      m_heatIndex(conditons->GetHeatIndex()),
      m_windChill(conditons->GetWindChill()),
      m_snowDepth(conditons->GetSnowDepth()),
      m_maxTemp6Hour(conditons->GetMaxTempInLast6hr()),
      m_maxTemp24Hour(conditons->GetMaxTempInLast24hr()),
      m_minTemp6Hour(conditons->GetMinTempInLast6hr()),
      m_minTemp24Hour(conditons->GetMinTempInLast24hr()),
      m_precipitation3Hour(conditons->GetPrecipitationInLast3hr()),
      m_precipitation6Hour(conditons->GetPrecipitationInLast6hr()),
      m_precipitation24Hour(conditons->GetPrecipitationInLast24hr()),
      m_updateTime(conditons->GetUpdateTime()),
      m_utcOffset(conditons->GetUtcOffset())
{
}

OnboardWeatherForecastImpl::OnboardWeatherForecastImpl()
{
}

OnboardWeatherForecastImpl::OnboardWeatherForecastImpl(const OnboardWeatherForecastImpl& copy)
    : m_date(copy.GetDate()),
      m_highTemperature(copy.GetHighTemperature()),
      m_lowTemperature(copy.GetLowTemperature()),
      m_windSpeed(copy.GetWindSpeed()),
      m_windDirection(copy.GetWindDirection()),
      m_precipitationProbability(copy.GetPrecipitationProbability()),
      m_relativeHumidity(copy.GetRelativeHumidity()),
      m_condition(copy.GetWeatherCondition()),
      m_conditonCode(copy.GetConditionCode()),
      m_uvIndex(copy.GetUVIndex()),
      m_uvDescription(copy.GetUVDescription()),
      m_sunriseTime(copy.GetSunriseTime()),
      m_sunsetTime(copy.GetSunsetTime()),
      m_moonriseTime(copy.GetMoonriseTime()),
      m_moonsetTime(copy.GetMoonsetTime()),
      m_moonPhase(copy.GetMoonPhase()),
      m_updateTime(copy.GetUpdateTime()),
      m_utcOffset(copy.GetUTCOffset())

{
}

OnboardWeatherForecastImpl::OnboardWeatherForecastImpl(const WeatherForecast& copy)
    : m_date(copy.GetDate()),
      m_highTemperature(copy.GetHighTemperature()),
      m_lowTemperature(copy.GetLowTemperature()),
      m_windSpeed(copy.GetWindSpeed()),
      m_windDirection(copy.GetWindDirection()),
      m_precipitationProbability(copy.GetPrecipitationProbability()),
      m_relativeHumidity(copy.GetRelativeHumidity()),
      m_condition(copy.GetWeatherCondition()),
      m_conditonCode(copy.GetConditionCode()),
      m_uvIndex(copy.GetUVIndex()),
      m_uvDescription(copy.GetUVDescription()),
      m_sunriseTime(copy.GetSunriseTime()),
      m_sunsetTime(copy.GetSunsetTime()),
      m_moonriseTime(copy.GetMoonriseTime()),
      m_moonsetTime(copy.GetMoonsetTime()),
      m_moonPhase(copy.GetMoonPhase()),
      m_updateTime(copy.GetUpdateTime()),
      m_utcOffset(copy.GetUTCOffset())
{
}

OnboardWeatherForecastImpl::OnboardWeatherForecastImpl(protocol::WeatherForecastSharedPtr forecast)
    : m_date(forecast->GetDate()),
      m_highTemperature(forecast->GetHighTemperature()),
      m_lowTemperature(forecast->GetLowTemperature()),
      m_windSpeed(forecast->GetWindSpeed()),
      m_windDirection(forecast->GetWindDirection()),
      m_precipitationProbability(forecast->GetPrecipitationProbability()),
      m_relativeHumidity(forecast->GetRelativeHumidity()),
      m_condition(forecast->GetWeatherCondition()->c_str()),
      m_conditonCode(forecast->GetConditionCode()),
      m_uvIndex(forecast->GetUvIndex()),
      m_uvDescription(forecast->GetUvDescription()->c_str()),
      m_sunriseTime(forecast->GetSunrise()->c_str()),
      m_sunsetTime(forecast->GetSunset()->c_str()),
      m_moonriseTime(forecast->GetMoonrise()->c_str()),
      m_moonsetTime(forecast->GetMoonset()->c_str()),
      m_moonPhase(forecast->GetMoonPhase()),
      m_updateTime(forecast->GetUpdateTime()),
      m_utcOffset(forecast->GetUtcOffset())
{
}

OnboardWeatherConditionsSummaryImpl::OnboardWeatherConditionsSummaryImpl()
    : m_temperature(0.0f),
      m_conditonCode(0),
      m_maxTemp24Hour(0.0f),
      m_minTemp24Hour(0.0f),
      m_utcOffset(0)
{
}

OnboardWeatherConditionsSummaryImpl::OnboardWeatherConditionsSummaryImpl(const WeatherConditionsSummary& copy)
    : m_temperature(copy.GetTemperature()),
      m_condition(copy.GetCondition()),
      m_conditonCode(copy.GetConditionCode()),
      m_maxTemp24Hour(copy.GetMaxTemperatureIn24Hours()),
      m_minTemp24Hour(copy.GetMinTemperatureIn24Hours()),
      m_utcOffset(copy.GetUTCOffset())
{
}

OnboardWeatherConditionsSummaryImpl::OnboardWeatherConditionsSummaryImpl(const OnboardWeatherConditionsSummaryImpl& copy)
    : m_temperature(copy.GetTemperature()),
      m_condition(copy.GetCondition()),
      m_conditonCode(copy.GetConditionCode()),
      m_maxTemp24Hour(copy.GetMaxTemperatureIn24Hours()),
      m_minTemp24Hour(copy.GetMinTemperatureIn24Hours()),
      m_utcOffset(copy.GetUTCOffset())

{
}

OnboardWeatherConditionsSummaryImpl::OnboardWeatherConditionsSummaryImpl(protocol::WeatherConditionsSummarySharedPtr summary)
    : m_temperature(summary->GetTemperature()),
      m_condition(summary->GetWeatherCondition()->c_str()),
      m_conditonCode(summary->GetConditionCode()),
      m_maxTemp24Hour(summary->GetMaxTempInLast24hr()),
      m_minTemp24Hour(summary->GetMinTempInLast24hr()),
      m_utcOffset(summary->GetUtcOffset())
{
}

OnboardPlaceImpl::OnboardPlaceImpl()
    : m_location(NULL),
      m_theaterId(NULL),
      m_weatherConditons(NULL),
      m_cookie(NULL)
{
}

OnboardPlaceImpl::OnboardPlaceImpl(const OnboardPlaceImpl& copy)
    : m_name(copy.m_name),
      m_location(copy.m_location != NULL ? new Location(*copy.m_location) : NULL),
      m_theaterId(copy.m_theaterId),
      m_weatherConditons(copy.m_weatherConditons != NULL ? new OnboardWeatherConditionsImpl(*copy.m_weatherConditons) : NULL),
      m_cookie(copy.m_cookie != NULL ? new OnboardPlaceEventCookieImpl(*copy.m_cookie) : NULL)
{
    for (std::vector<Phone*>::const_iterator iter = copy.m_phones.begin(); iter != copy.m_phones.end(); iter ++)
    {
        m_phones.push_back(new OnboardPhoneImpl(*(OnboardPhoneImpl*)*iter));
    }
    for (std::vector<Category*>::const_iterator iter = copy.m_categories.begin(); iter != copy.m_categories.end(); iter++)
    {
        m_categories.push_back(new OnboardCategoryImpl(*(OnboardCategoryImpl*)*iter));
    }
    for (std::vector<Event*>::const_iterator iter = copy.m_events.begin(); iter != copy.m_events.end(); iter++)
    {
        m_events.push_back(new OnboardEventImpl(*(OnboardEventImpl*)*iter));
    }
    for (std::vector<WeatherConditionsSummary*>::const_iterator iter = copy.m_weatherConditonSummaries.begin(); iter != copy.m_weatherConditonSummaries.end(); iter++)
    {
        m_weatherConditonSummaries.push_back(new OnboardWeatherConditionsSummaryImpl(*(OnboardWeatherConditionsSummaryImpl*)*iter));
    }
    for (std::vector<WeatherForecast*>::const_iterator iter = copy.m_weatherForecast.begin(); iter != copy.m_weatherForecast.end(); iter++)
    {
        m_weatherForecast.push_back(new OnboardWeatherForecastImpl(*(OnboardWeatherForecastImpl*)*iter));
    }
}

OnboardPlaceImpl::OnboardPlaceImpl(const Place& copy)
    : m_name(copy.GetName()),
      m_location(copy.GetLocation() != NULL ? new Location(*copy.GetLocation()) : NULL),
      m_theaterId(NULL),
      m_weatherConditons(copy.GetWeatherConditions() != NULL ? new OnboardWeatherConditionsImpl(*copy.GetWeatherConditions()) : NULL),
      m_cookie(copy.GetPlaceEventCookie() != NULL ? new OnboardPlaceEventCookieImpl(*copy.GetPlaceEventCookie()) : NULL)
{
    std::string theaterId;
    if (copy.GetTheaterId(theaterId))
    {
        m_theaterId = new std::string(theaterId);
    }
    for (std::vector<Phone*>::const_iterator iter = copy.GetPhoneNumbers().begin(); iter != copy.GetPhoneNumbers().end(); iter ++)
    {
        m_phones.push_back(new OnboardPhoneImpl(**iter));
    }
    for (std::vector<Category*>::const_iterator iter = copy.GetCategories().begin(); iter != copy.GetCategories().end(); iter++)
    {
        m_categories.push_back(new OnboardCategoryImpl(**iter));
    }
    for (std::vector<Event*>::const_iterator iter = copy.GetEvents().begin(); iter != copy.GetEvents().end(); iter++)
    {
        m_events.push_back(new OnboardEventImpl(*(OnboardEventImpl*)*iter));
    }
    for (std::vector<WeatherConditionsSummary*>::const_iterator iter = copy.GetWeatherConditionsSummaries().begin(); iter != copy.GetWeatherConditionsSummaries().end(); iter++)
    {
        m_weatherConditonSummaries.push_back(new OnboardWeatherConditionsSummaryImpl(*(OnboardWeatherConditionsSummaryImpl*)*iter));
    }
    for (std::vector<WeatherForecast*>::const_iterator iter = copy.GetWeatherForecast().begin(); iter != copy.GetWeatherForecast().end(); iter++)
    {
        m_weatherForecast.push_back(new OnboardWeatherForecastImpl(*(OnboardWeatherForecastImpl*)*iter));
    }
}

OnboardPlaceImpl::OnboardPlaceImpl(Ncdb::SearchMatch* sm)
    : m_name(UtfToStdString(sm->MatchName)),
      m_location(OnboardLocationUtil::toLocation(sm)),
      m_theaterId(NULL),
      m_weatherConditons(NULL),
      m_cookie(NULL)
{
    Ncdb::AutoArray<Ncdb::UtfString>& phones = sm->TelephoneNumbers;
    for (size_t i = 0; i < phones.size(); ++i)
    {
        m_phones.push_back(new OnboardPhoneImpl(phones[i].GetBuffer()));
    }

    // if sm->MatchName is empty as in the case of Airport
    if ("" == m_name) {
        m_name = m_location->GetName();
    }
/*
    shared_ptr<std::vector<shared_ptr<protocol::Category> > > categories = place->GetCategoryArray();
    std::vector<shared_ptr<protocol::Category> >::iterator it = categories->begin();
    for (; it != categories->end(); ++it)
    {
        OnboardCategoryImpl* category = new OnboardCategoryImpl(*it);
        if((*it)->GetParentCategory())
        {
            category->m_parent = new OnboardCategoryImpl((*it)->GetParentCategory());
        }

        m_categories.push_back(category);
    }

    shared_ptr<std::vector<shared_ptr<protocol::Phone> > > phones = place->GetPhoneArray();
    std::vector<shared_ptr<protocol::Phone> >::iterator phoneIterator = phones->begin();
    for (; phoneIterator != phones->end(); ++phoneIterator)
    {
        m_phones.push_back(new OnboardPhoneImpl(*phoneIterator));
    }

    protocol::EventSharedPtrList events = place->GetEventArray();
    protocol::EventSharedPtrArray::iterator eventIterator = events->begin();
    for (; eventIterator != events->end(); ++eventIterator)
    {
        m_events.push_back(new OnboardEventImpl(*eventIterator));
    }

    protocol::WeatherConditionsSummarySharedPtrList summaries = place->GetWeatherConditionsSummaryArray();
    protocol::WeatherConditionsSummarySharedPtrArray::iterator summaryIterator = summaries->begin();
    for (; summaryIterator != summaries->end(); ++summaryIterator)
    {
        m_weatherConditonSummaries.push_back(new OnboardWeatherConditionsSummaryImpl(*summaryIterator));
    }

    protocol::WeatherForecastSharedPtrList forecast = place->GetWeatherForecastArray();
    protocol::WeatherForecastSharedPtrArray::iterator forecastIterator = forecast->begin();
    for(; forecastIterator != forecast->end(); ++forecastIterator)
    {
        m_weatherForecast.push_back(new OnboardWeatherForecastImpl(*forecastIterator));
    }

    if (!place->GetPairArray()->empty())
    {
        protocol::PairSharedPtrList pairs = place->GetPairArray();
        protocol::PairSharedPtrArray::iterator it = pairs->begin();
        for (; it != pairs->end(); ++it)
        {
            if ((*it)->GetKey()->compare("theater-id") == 0)
            {
                m_theaterId = new std::string(*(*it)->GetValue());
                break;
            }
        }
    }
    */
}

bool OnboardPlaceImpl::GetTheaterId(std::string& theaterId) const
{
    return m_theaterId != NULL ? (theaterId = *m_theaterId, true) : false;
}

OnboardPlaceImpl::~OnboardPlaceImpl()
{
    SAFE_DELETE(m_location);
    SAFE_DELETE(m_theaterId);
    SAFE_DELETE(m_cookie);
    OnboardDeleteVector<Phone>(m_phones);
    OnboardDeleteVector<Category>(m_categories);
    OnboardDeleteVector<Event>(m_events);
    OnboardDeleteVector<WeatherConditionsSummary>(m_weatherConditonSummaries);
}

OnboardGoldenCookieImpl::OnboardGoldenCookieImpl(const GoldenCookie& copy)
    : m_provideId(copy.GetProviderId()),
      m_state(copy.GetState()),
      m_serialized(copy.GetSerialized())
{
}

OnboardGoldenCookieImpl::OnboardGoldenCookieImpl(const OnboardGoldenCookieImpl& copy)
    : m_provideId(copy.m_provideId),
      m_state(copy.m_state),
      m_serialized(copy.m_serialized)
{
}

OnboardGoldenCookieImpl::OnboardGoldenCookieImpl(protocol::GoldenCookieSharedPtr cookie)
    : m_provideId(cookie->GetProvider()->c_str()),
      m_state((const char*)cookie->GetState()->GetData()),
      m_serialized(Serialize(cookie))
{
}

std::string OnboardGoldenCookieImpl::Serialize(const protocol::GoldenCookieSharedPtr& goldenCookieProtocol)
{
    return std::string(nbsearch::OnboardCreateBased64DataByTpsElement(protocol::GoldenCookieSerializer::serialize(goldenCookieProtocol)));
}

protocol::GoldenCookieSharedPtr OnboardGoldenCookieImpl::Deserialize(const std::string& serializedData)
{
    return protocol::GoldenCookieSerializer::deserialize(nbsearch::OnboardCreateTpsElementByBase64Data(serializedData));
}

OnboardPOIContentImpl::OnboardPOIContentImpl()
    : m_tagLine(NULL),
      m_hourOfOperation(NULL),
      m_cookie(NULL)
{
}

OnboardPOIContentImpl::OnboardPOIContentImpl(protocol::PoiContentSharedPtr poiContent)
    : m_tagLine(NULL),
      m_hourOfOperation(NULL),
      m_cookie(NULL)
{
    m_id = *poiContent->GetId();
    if (poiContent->GetHoursOfOperation())
    {
        m_hourOfOperation = new OnboardHoursOfOperationImpl(poiContent->GetHoursOfOperation());
    }

    if (poiContent->GetTagline() && poiContent->GetTagline()->GetText())
    {
        m_tagLine = new std::string(poiContent->GetTagline()->GetText()->c_str());
    }

    if (poiContent->GetGoldenCookie())
    {
        m_cookie = new OnboardGoldenCookieImpl(poiContent->GetGoldenCookie());
    }

    protocol::PairSharedPtrList pairs = poiContent->GetDataMapArray();
    protocol::PairSharedPtrArray::iterator it = pairs->begin();
    for (; it != pairs->end(); ++it)
    {
        StringPair* pairImpl = new StringPair(OnboardStringPairUtil::toStringPair(*it));
        m_pairs.push_back(pairImpl);
    }

    shared_ptr<std::vector<shared_ptr<protocol::VendorContent> > > vendorContents = poiContent->GetVendorContentArray();
    std::vector<shared_ptr<protocol::VendorContent> >::iterator vendorIterator = vendorContents->begin();
    for (; vendorIterator != vendorContents->end(); ++vendorIterator)
    {
        m_vendorContents.push_back(new OnboardVendorContentImpl(*vendorIterator));
    }
}


OnboardPOIContentImpl::OnboardPOIContentImpl(const POIContent& copy)
    : m_tagLine(NULL),
      m_id(copy.GetId()),
      m_hourOfOperation(copy.GetHoursOfOperation() != NULL ? new OnboardHoursOfOperationImpl(*copy.GetHoursOfOperation()) : NULL),
      m_cookie(copy.GetGoldenCookie() != NULL ? new OnboardGoldenCookieImpl(*copy.GetGoldenCookie()) : NULL)
{
    std::vector<StringPair*> pairs = copy.GetStringPairs();
    for (std::vector<StringPair*>::const_iterator iter = pairs.begin(); iter != pairs.end(); iter++)
    {
        m_pairs.push_back(new StringPair(**iter));
    }
    std::string* tagline = new std::string();
    if (!copy.GetTagLine(*tagline))
    {
        delete tagline;
        tagline = NULL;
    }
    m_tagLine = tagline;

    for (std::vector<VendorContent*>::const_iterator iter = copy.GetVendorContents().begin(); iter != copy.GetVendorContents().end(); iter++)
    {
        m_vendorContents.push_back(new OnboardVendorContentImpl(**iter));
    }
}

OnboardPOIContentImpl::OnboardPOIContentImpl(const OnboardPOIContentImpl& copy)
    : m_tagLine(copy.m_tagLine != NULL ? new std::string(*copy.m_tagLine) : NULL),
      m_id(copy.m_id),
      m_hourOfOperation(copy.m_hourOfOperation != NULL ? new OnboardHoursOfOperationImpl(*copy.m_hourOfOperation) : NULL),
      m_cookie(copy.m_cookie != NULL ? new OnboardGoldenCookieImpl(*copy.m_cookie) : NULL)
{
    for (std::vector<VendorContent*>::const_iterator iter = copy.m_vendorContents.begin(); iter != copy.m_vendorContents.end(); iter++)
    {
        m_vendorContents.push_back(new OnboardVendorContentImpl(*(OnboardVendorContentImpl*)*iter));
    }
    std::vector<StringPair*> pairs = copy.GetStringPairs();
    for (std::vector<StringPair*>::const_iterator iter = pairs.begin(); iter != pairs.end(); iter++)
    {
        m_pairs.push_back(new StringPair(**iter));
    }
}

OnboardPOIContentImpl::~OnboardPOIContentImpl()
{
    SAFE_DELETE(m_tagLine);
    SAFE_DELETE(m_hourOfOperation);
    SAFE_DELETE(m_cookie)
    OnboardDeleteVector<VendorContent>(m_vendorContents);
    OnboardDeleteVector<StringPair>(m_pairs);
}

//POIKey POIContent::convertStringToKey(const std::string &key)
//{
//    POIKey enumKey = (POIKey)-1;
//    if (key.compare(POI_CONTENT_KEY_CUISINES) == 0)
//    {
//        enumKey = POIK_Cuisines;
//    }
//    else if (key.compare(POI_CONTENT_KEY_DESCRIPTION) == 0)
//    {
//        enumKey = POIK_Description;
//    }
//    else if (key.compare(POI_CONTENT_KEY_FEATURES) == 0)
//    {
//        enumKey = POIK_Features;
//    }
//    else if (key.compare(POI_CONTENT_KEY_HOURSOFOPERATION) == 0)
//    {
//        enumKey = POIK_HoursOfOperation;
//    }
//    else if (key.compare(POI_CONTENT_KEY_PARKING) == 0)
//    {
//        enumKey = POIK_Parking;
//    }
//    else if (key.compare(POI_CONTENT_KEY_PRICE) == 0)
//    {
//        enumKey = POIK_Price;
//    }
//    else if (key.compare(POI_CONTENT_KEY_PAYEMENTMETHOD) == 0)
//    {
//        enumKey = POIK_PaymentMethod;
//    }
//    else if (key.compare(POI_CONTENT_KEY_RESERVATIONS) == 0)
//    {
//        enumKey = POIK_Reservations;
//    }
//    else if (key.compare(POI_CONTENT_KEY_SITEURL) == 0)
//    {
//        enumKey = POIK_SiteUrl;
//    }
//    else if (key.compare(POI_CONTENT_KEY_SPECIALFEATURES) == 0)
//    {
//        enumKey = POIK_SpecialFeatures;
//    }
//    else if (key.compare(POI_CONTENT_KEY_THUMBNAILPHOTOURL) == 0)
//    {
//        enumKey = POIK_ThumbnailPhotoUrl;
//    }
//    else if (key.compare(0, nsl_strlen(POI_CONTENT_KEY_PHOTOURL), POI_CONTENT_KEY_PHOTOURL) == 0)
//    {
//        enumKey = POIK_PhotoUrl;
//    }
//    else if (key.compare(POI_CONTENT_KEY_WIFI) == 0)
//    {
//        enumKey = POIK_Wifi;
//    }
//    else if (key.compare(POI_CONTENT_KEY_DRESSCODE) == 0)
//    {
//        enumKey = POIK_Dresscode;
//    }
//    else if (key.compare(POI_CONTENT_KEY_SMOKING) == 0)
//    {
//        enumKey = POIK_Smoking;
//    }
//    else if (key.compare(POI_CONTENT_KEY_SHUTTLESERVICE) == 0)
//    {
//        enumKey = POIK_ShuttleService;
//    }
//    else if (key.compare(POI_CONTENT_KEY_TIPS) == 0)
//    {
//        enumKey = POIK_Tips;
//    }
//    return enumKey;
//}

const std::vector<POIKey> OnboardPOIContentImpl::GetKeys() const
{
    std::vector<POIKey> keys;
    for (std::vector<StringPair*>::const_iterator iter = m_pairs.begin();
         iter != m_pairs.end(); iter++)
    {
        POIKey enumKey = POIContent::convertStringToKey((*iter)->GetKey());
        if (enumKey == (POIKey)-1)
        {
            continue;
        }
        if (std::find(keys.begin(), keys.end(), enumKey) != keys.end())
        {
            continue;
        }
        keys.push_back(enumKey);
    }
    return keys;
}

bool OnboardPOIContentImpl::GetKeyValue(POIKey poiKey, std::vector<std::string>& outValue) const
{
    bool found = false;
    for (std::vector<StringPair*>::const_iterator iter = m_pairs.begin();
         iter != m_pairs.end(); iter++)
    {
        POIKey enumKey = POIContent::convertStringToKey((*iter)->GetKey());
        if (enumKey == poiKey)
        {
            found = true;
            outValue.push_back((*iter)->GetValue());
        }
    }
    return found;
}

bool OnboardPOIContentImpl::GetTagLine(std::string &_tagLine) const
{
    if (m_tagLine == NULL)
    {
        return false;
    }
    _tagLine = *m_tagLine;
    return true;
}

const std::vector<std::string> OnboardPOIContentImpl::GetKeysOfString() const
{
    std::vector<std::string> keys;
    for(std::vector<StringPair*>::const_iterator it = m_pairs.begin(); it != m_pairs.end(); ++it)
    {
        std::string key = (*it)->GetKey();
        if (std::find(keys.begin(), keys.end(), key) == keys.end())
        {
            keys.push_back(key);
        }
    }
    return keys;
}

const vector<std::string> OnboardPOIContentImpl::GetValuesByKey(const std::string& key) const
{
    std::vector<std::string> result;
    for(std::vector<StringPair*>::const_iterator it = m_pairs.begin(); it != m_pairs.end(); ++it)
    {
        std::string rkey = (*it)->GetKey();
        if (key == rkey)
        {
            result.push_back((*it)->GetValue());
        }
    }
    return result;
}

const std::vector<StringPair*> OnboardPOIContentImpl::GetStringPairs() const
{
    return m_pairs;
}

OnboardFuelTypeImpl::OnboardFuelTypeImpl(const FuelType& copy)
    : m_productName(copy.GetProductName()),
      m_code(copy.GetCode()),
      m_typeName(copy.GetTypeName())
{
}

OnboardFuelTypeImpl::OnboardFuelTypeImpl(const OnboardFuelTypeImpl& copy)
    : m_productName(copy.m_productName),
      m_code(copy.m_code),
      m_typeName(copy.m_typeName)
{
}

OnboardFuelTypeImpl::OnboardFuelTypeImpl(protocol::FuelTypeSharedPtr type)
    : m_productName(*type->GetProductName()),
      m_code(*type->GetCodeTypeofFuel()),
      m_typeName(*type->GetTypeName())
{
}

OnboardPriceImpl::OnboardPriceImpl(const Price& copy)
    : m_value(copy.GetValue()),
      m_currency(copy.GetCurrency()),
      m_modTime(copy.GetModTime())
{
}

OnboardPriceImpl::OnboardPriceImpl(const OnboardPriceImpl& copy)
    : m_value(copy.m_value),
      m_currency(copy.m_currency),
      m_modTime(copy.m_modTime)
{
}

OnboardPriceImpl::OnboardPriceImpl(protocol::PriceSharedPtr price)
    : m_value(price->GetValue()),
      m_currency(*price->GetCurrency()),
      m_modTime(price->GetLastUpdated())
{
}

OnboardFuelProductImpl::OnboardFuelProductImpl(const FuelProduct& copy)
    : m_price(copy.GetPrice() != NULL ? new OnboardPriceImpl(*copy.GetPrice()) : NULL),
      m_type(copy.GetFuelType() != NULL ? new OnboardFuelTypeImpl(*copy.GetFuelType()) : NULL),
      m_units(copy.GetUnits())
{
}

OnboardFuelProductImpl::OnboardFuelProductImpl(const OnboardFuelProductImpl& copy)
    : m_price(copy.m_price != NULL ? new OnboardPriceImpl(*copy.m_price) : NULL),
      m_type(copy.m_type != NULL ? new OnboardFuelTypeImpl(*copy.m_type) : NULL),
      m_units(copy.m_units)
{
}

OnboardFuelProductImpl::OnboardFuelProductImpl(protocol::FuelProductSharedPtr product)
    : m_price(new OnboardPriceImpl(product->GetPrice())),
      m_type(new OnboardFuelTypeImpl(product->GetFuelType())),
      m_units(*product->GetMeasurementUnit())
{
}

OnboardFuelProductImpl::~OnboardFuelProductImpl()
{
    delete m_price;
    delete m_type;
}

OnboardFuelSummaryImpl::OnboardFuelSummaryImpl(protocol::FuelPriceSummarySharedPtr summary)
  : m_average(NULL), m_low(NULL)
{
    protocol::FuelProductSharedPtrList products = summary->GetFuelProductArray();
    protocol::FuelProductSharedPtrArray::iterator it = products->begin();
    for (; it != products->end(); ++it)
    {
        if ((*it)->GetAverage() && m_average == NULL)
        {
            m_average = new OnboardFuelProductImpl(*it);
        }
        else if ((*it)->GetLow() && m_low == NULL)
        {
            m_low = new OnboardFuelProductImpl(*it);
        }
    }
}

OnboardFuelSummaryImpl::~OnboardFuelSummaryImpl()
{
    SAFE_DELETE(m_average);
    SAFE_DELETE(m_low);
}

OnboardFuelDetailsImpl::OnboardFuelDetailsImpl(protocol::FuelProductSharedPtrList products)
{
    protocol::FuelProductSharedPtrArray::iterator it = products->begin();
    for (; it != products->end(); ++it)
    {
        m_products.push_back(new OnboardFuelProductImpl(*it));
    }
}

OnboardFuelDetailsImpl::OnboardFuelDetailsImpl(const FuelDetails& copy)
{
    for (std::vector<FuelProduct*>::const_iterator iter = copy.GetFuelProducts().begin(); iter != copy.GetFuelProducts().end(); iter++)
    {
        m_products.push_back(new OnboardFuelProductImpl(**iter));
    }
}

OnboardFuelDetailsImpl::OnboardFuelDetailsImpl(const OnboardFuelDetailsImpl& copy)
{
    for (std::vector<FuelProduct*>::const_iterator iter = copy.m_products.begin(); iter != copy.m_products.end(); iter++)
    {
        m_products.push_back(new OnboardFuelProductImpl(*((OnboardFuelProductImpl*)*iter)));
    }
}

OnboardFuelDetailsImpl::~OnboardFuelDetailsImpl()
{
    OnboardDeleteVector<FuelProduct>(m_products);
}

OnboardSuggestionMatchImpl::~OnboardSuggestionMatchImpl()
{
    SAFE_DELETE(m_searchFilter);
}

OnboardSuggestionMatchImpl::OnboardSuggestionMatchImpl(const SuggestionMatch& copy)
    :m_distance(copy.GetDistance()),
     m_line1(copy.GetLine1()),
     m_line2(copy.GetLine2()),
     m_line3(copy.GetLine3()),
     m_type(copy.GetMatchType()),
     m_iconIDs(copy.GetIconIDs()),
     m_searchFilter(NULL)
{
    if (copy.GetSearchFilter() != NULL)
    {
        m_searchFilter = new SearchFilter(*copy.GetSearchFilter());
    }
}

OnboardSuggestionMatchImpl::OnboardSuggestionMatchImpl(const OnboardSuggestionMatchImpl& copy)
    :m_distance(copy.m_distance),
     m_line1(copy.m_line1),
     m_line2(copy.m_line2),
     m_line3(copy.m_line3),
     m_type(copy.m_type),
     m_iconIDs(copy.m_iconIDs),
     m_searchFilter(NULL)
{
    if (copy.m_searchFilter != NULL)
    {
        m_searchFilter = new SearchFilter(*copy.m_searchFilter);
    }
}


OnboardSuggestionMatchImpl::OnboardSuggestionMatchImpl(protocol::SuggestMatchSharedPtr suggestMatch)
    : m_distance(suggestMatch->GetDistance()),
      m_line1(*suggestMatch->GetLine1()),
      m_line2(*suggestMatch->GetLine2()),
      m_line3(*suggestMatch->GetLine3()),
      m_type(SMT_None),
      m_searchFilter(NULL)
{
    if (suggestMatch->GetMatchType()->compare("poi") == 0)
    {
        m_type = SMT_POI;
    }
    else if (suggestMatch->GetMatchType()->compare("address") == 0)
    {
        m_type = SMT_Address;
    }
    else if (suggestMatch->GetMatchType()->compare("airport") == 0)
    {
        m_type = SMT_Airport;
    }
    else if (suggestMatch->GetMatchType()->compare("gas") == 0)
    {
        m_type = SMT_Gas;
    }
    else if (suggestMatch->GetMatchType()->compare("category") == 0)
    {
        m_type = SMT_Category;
    }
    else if (suggestMatch->GetMatchType()->compare("local") == 0)
    {
        m_type = SMT_Local;
    }
    else if (suggestMatch->GetMatchType()->compare("movie") == 0)
    {
        m_type = SMT_Movie;
    }
    else
    {
        m_type = SMT_None;
    }
    shared_ptr<std::vector<shared_ptr<protocol::Pair> > > pairs = suggestMatch->GetPairArray();
    std::vector<shared_ptr<protocol::Pair> >::iterator it = pairs->begin();
    for (; it != pairs->end(); ++it)
    {
        if ((*it)->GetKey()->compare("icon-id") == 0)
        {
            m_iconIDs.push_back(*(*it)->GetValue());
        }
    }

    m_searchFilter = new SearchFilter(OnboardSearchFilterUtil::toSearchFilter(suggestMatch->GetSearchFilter()));
}

OnboardResultStateImpl::OnboardResultStateImpl(const ResultState& copy)
    : m_serialized(copy.GetSerialized())
{
}

OnboardResultStateImpl::OnboardResultStateImpl(const OnboardResultStateImpl& copy)
    : m_serialized(copy.GetSerialized())
{
}

OnboardResultStateImpl::~OnboardResultStateImpl()
{
}

OnboardResultStateImpl& OnboardResultStateImpl::operator=(const ResultState& rhs)
{
    m_serialized = rhs.GetSerialized();
    return *this;
}

OnboardSuggestionListImpl::OnboardSuggestionListImpl(const protocol::SuggestListSharedPtr& suggestList): m_name(*suggestList->GetName())
{
    protocol::SuggestMatchSharedPtrList suggestMatches(suggestList->GetSuggestMatchArray());

    protocol::SuggestMatchSharedPtrArray::const_iterator suggestMatchIter = suggestMatches->begin();
    for (; suggestMatchIter != suggestMatches->end(); ++suggestMatchIter)
    {
        m_suggestMatches.push_back(new OnboardSuggestionMatchImpl(*suggestMatchIter));
    }
}

OnboardSuggestionListImpl::OnboardSuggestionListImpl(const SuggestionList& copy)
{
    this->m_name = copy.GetName();
    std::vector<SuggestionMatch*>::const_iterator suggestMatchIter = copy.GetSuggestionMatches().begin();
    for (; suggestMatchIter != copy.GetSuggestionMatches().end(); ++suggestMatchIter)
    {
        m_suggestMatches.push_back(new OnboardSuggestionMatchImpl(**suggestMatchIter));
    }
}

OnboardSuggestionListImpl::~OnboardSuggestionListImpl()
{
    OnboardDeleteVector<SuggestionMatch>(m_suggestMatches);
}

std::string OnboardSuggestionListImpl::Serialize(const protocol::SuggestListSharedPtr& suggestListProtocol)
{
    return nbsearch::OnboardCreateBased64DataByTpsElement(protocol::SuggestListSerializer::serialize(suggestListProtocol));
}

protocol::SuggestListSharedPtr OnboardSuggestionListImpl::Deserialize(const std::string& serializedData)
{
    return protocol::SuggestListSerializer::deserialize(nbsearch::OnboardCreateTpsElementByBase64Data(serializedData));
}

OnboardPOIImpl::OnboardPOIImpl(Ncdb::SearchMatch* sm) :
    m_type(POIT_Location)
  , m_place(NULL)
  , m_searchfilter(NULL)
  , m_poicontent(NULL)
  , m_fuelDetails(NULL)
  , m_relatedSearch(NULL)
  , m_distance(-1.0)
  , m_isPremiumPlacement(false)
  , m_isUnmappable(false)
  , m_isEnahancedPoi(false)
  , m_searchQueryEventId(0)
{
    this->m_distance = sm->MatchDistance;
    this->m_place = new OnboardPlaceImpl(sm);
/*
 *     {
        OnboardPOIImpl* poi = new OnboardPOIImpl();

        poi->m_isPremiumPlacement = (*it)->GetPremiumPlacement();
        poi->m_isUnmappable = (*it)->GetUnmappable();
        poi->m_isEnahancedPoi = (*it)->GetEnhancedPOI();
        poi->m_place = new OnboardPlaceImpl((*it)->GetPlace());
        poi->m_distance = (*it)->GetDistance();
        poi->m_searchQueryEventId = searchQueryEventId;
        if ((*it)->GetPoiContent() && (*it)->GetPoiContent().get())
        {
            poi->m_poicontent = new OnboardPOIContentImpl((*it)->GetPoiContent());
        }
        if ((*it)->GetPlace()->GetFuelProductArray() && (*it)->GetPlace()->GetFuelProductArray().get() && (*it)->GetPlace()->GetFuelProductArray()->size() > 0)
        {
            poi->m_fuelDetails = new OnboardFuelDetailsImpl((*it)->GetPlace()->GetFuelProductArray());
        }
        if ((*it)->GetSearchFilter() && (*it)->GetSearchFilter().get())
        {
            poi->m_searchfilter = new SearchFilter(OnboardSearchFilterUtil::toSearchFilter((*it)->GetSearchFilter()));
        }
        if ((*it)->GetRelatedSearch() && (*it)->GetRelatedSearch().get())
        {
            poi->m_relatedSearch = new OnboardRelatedSearchImpl((*it)->GetRelatedSearch());
        }
        */
}

OnboardPOIImpl::OnboardPOIImpl(const OnboardPOIImpl& copy)
    : m_type(copy.m_type),
      m_place(copy.m_place != NULL ? new OnboardPlaceImpl(*copy.m_place) : NULL),
      m_searchfilter(copy.m_searchfilter != NULL ? new SearchFilter(*copy.m_searchfilter) : NULL),
      m_poicontent(copy.m_poicontent != NULL ? new OnboardPOIContentImpl(*copy.m_poicontent) : NULL),
      m_fuelDetails(copy.m_fuelDetails != NULL ? new OnboardFuelDetailsImpl(*copy.m_fuelDetails) : NULL),
      m_relatedSearch(copy.m_relatedSearch != NULL ? new OnboardRelatedSearchImpl(*copy.m_relatedSearch) : NULL),
      m_distance(copy.m_distance),
      m_isPremiumPlacement(copy.m_isPremiumPlacement),
      m_isUnmappable(copy.m_isUnmappable),
      m_isEnahancedPoi(copy.m_isEnahancedPoi)
{
}

OnboardPOIImpl::OnboardPOIImpl(const POI& copy)
    : m_type(copy.GetPOIType()),
      m_place(copy.GetPlace() != NULL ? new OnboardPlaceImpl(*copy.GetPlace()) : NULL),
      m_searchfilter(copy.GetSearchFilter() != NULL ? new SearchFilter(*copy.GetSearchFilter()) : NULL),
      m_poicontent(copy.GetPOIContent() != NULL ? new OnboardPOIContentImpl(*copy.GetPOIContent()) : NULL),
      m_fuelDetails(copy.GetFuelDetails() != NULL ? new OnboardFuelDetailsImpl(*copy.GetFuelDetails()) : NULL),
      m_relatedSearch(copy.GetRelatedSearch() != NULL ? new OnboardRelatedSearchImpl(*copy.GetRelatedSearch()) : NULL),
      m_distance(copy.GetDistance()),
      m_isPremiumPlacement(copy.IsPremiumPlacement()),
      m_isUnmappable(copy.IsUnmappable()),
      m_isEnahancedPoi(copy.IsEnhancedPoi())
{
}

OnboardPOIImpl::~OnboardPOIImpl()
{
    SAFE_DELETE(m_place);
    SAFE_DELETE(m_searchfilter);
    SAFE_DELETE(m_poicontent);
}

OnboardSingleSearchInformationImpl::~OnboardSingleSearchInformationImpl()
{
    OnboardDeleteVector<SearchResultBase>(m_results);
    OnboardDeleteVector<ResultDescription>(m_resultDescriptions);
    SAFE_DELETE(m_fuelSummary);
    SAFE_DELETE(m_resultState);
    m_matches.clear();;
}

SingleSearchInformationResultType OnboardSingleSearchInformationImpl::GetResultType() const
{
    return m_type;
}

const SearchResultBase* OnboardSingleSearchInformationImpl::GetResultAtIndex(uint32 index) const
{
    if (index >= m_results.size())
    {
        return NULL;
    }
    return m_results.at(index);
}

const POI* OnboardSingleSearchInformationImpl::GetPOIAtIndex(uint32 index) const
{
    if (index >= m_results.size())
    {
        return NULL;
    }
    return static_cast<POI const*>(m_results.at(index));
}

const SuggestionMatch* OnboardSingleSearchInformationImpl::GetSuggestionMatchAtIndex(uint32 index) const
{
    if (index >= m_results.size())
    {
        return NULL;
    }
    return static_cast<SuggestionMatch const*>(m_results.at(index));
}

const std::vector<ResultDescription*>& OnboardSingleSearchInformationImpl::GetResultDescriptions() const
{
    return m_resultDescriptions;
}

const FuelSummary* OnboardSingleSearchInformationImpl::GetFuelSummary() const
{
    return m_fuelSummary;
}

const SuggestionList* OnboardSingleSearchInformationImpl::GetSuggestionListAtIndex(uint32 index) const
{
    if (index >= m_results.size())
    {
        return NULL;
    }
    return static_cast<SuggestionList const*>(m_results.at(index));
}

const ProxMatchContent* OnboardSingleSearchInformationImpl::GetProxMatchContentAtIndex(uint32 index) const
{
    if (index >= m_results.size())
    {
        return NULL;
    }
    return static_cast<ProxMatchContent const*>(m_results.at(index));
}

const ResultState* OnboardSingleSearchInformationImpl::GetResultState() const
{
    return m_resultState;
}

OnboardSingleSearchInformationImpl::OnboardSingleSearchInformationImpl(const Ncdb::AutoArray<Ncdb::AutoSharedPtr<Ncdb::SearchMatch> >& matches, uint32 searchQueryEventId)
    : m_more(false),
      m_fuelSummary(NULL),
      m_matches(matches),
      m_type(SSIRT_POI)
{
    for (int i = 0; i < (int)matches.size(); i++) {
        OnboardPOIImpl *poi = new OnboardPOIImpl(matches[i]);
        m_results.push_back(poi);
    }
    m_resultState = new OnboardResultStateImpl();
    /*
    if(!m_information->GetProxMatchSummaryArray()->empty())
    {
        protocol::ProxMatchSummarySharedPtrList summaries = m_information->GetProxMatchSummaryArray();
        protocol::ProxMatchSummarySharedPtrArray::iterator it = summaries->begin();
        if (it != summaries->end())
        {
            if ((*it)->GetType()->compare("movie-showing") == 0)
            {
                m_type = SSIRT_MovieShowing;
            }
            else if ((*it)->GetType()->compare("movie") == 0)
            {
                m_type = SSIRT_Movie;
            }
            else if ((*it)->GetType()->compare("movie-theater") == 0)
            {
                m_type = SSIRT_MovieTheater;
            }
        }
    }
    else if (!m_information->GetProxMatchArray()->empty())
    {
        m_type = SSIRT_POI;
    }
    else if(!m_information->GetSuggestMatchArray()->empty())
    {
        m_type = SSIRT_SuggestionMatch;
    }
    else if(!m_information->GetSuggestListArray()->empty())
    {
        m_type = SSIRT_SuggestionList;
    }

    m_more = !m_information->GetIterResult()->GetExhausted();
    if (m_information->GetIterResult())
    {
        m_resultState = new OnboardResultStateImpl(m_information->GetIterResult()->GetState());
    }

    protocol::FuelPriceSummarySharedPtr fuelSummaryPtr =  m_information->GetFuelPriceSummary();
    if (fuelSummaryPtr && fuelSummaryPtr.get())
    {
        m_fuelSummary = new OnboardFuelSummaryImpl(fuelSummaryPtr);
    }

    // ProxMatch Content
    if (!m_information->GetProxMatchContentArray()->empty())
    {
        protocol::ProxMatchContentSharedPtrList proxMatchContents = m_information->GetProxMatchContentArray();
        protocol::ProxMatchContentSharedPtrArray::iterator proxMatchContentIterator = proxMatchContents->begin();
        for (; proxMatchContentIterator != proxMatchContents->end(); ++proxMatchContentIterator)
        {
            m_results.push_back(new OnboardProxMatchContentImpl(*proxMatchContentIterator));
        }
    }

    // Poi
    protocol::ProxMatchSharedPtrList pois = m_information->GetProxMatchArray();
    protocol::ProxMatchSharedPtrArray::iterator it = pois->begin();
    for (int i = 0; it != pois->end(); ++it, ++i) // Here "i" is only to meet Analytics.
    {
        OnboardPOIImpl* poi = new OnboardPOIImpl();

        poi->m_isPremiumPlacement = (*it)->GetPremiumPlacement();
        poi->m_isUnmappable = (*it)->GetUnmappable();
        poi->m_isEnahancedPoi = (*it)->GetEnhancedPOI();
        poi->m_place = new OnboardPlaceImpl((*it)->GetPlace());
        poi->m_distance = (*it)->GetDistance();
        poi->m_searchQueryEventId = searchQueryEventId;
        if ((*it)->GetPoiContent() && (*it)->GetPoiContent().get())
        {
            poi->m_poicontent = new OnboardPOIContentImpl((*it)->GetPoiContent());
        }
        if ((*it)->GetPlace()->GetFuelProductArray() && (*it)->GetPlace()->GetFuelProductArray().get() && (*it)->GetPlace()->GetFuelProductArray()->size() > 0)
        {
            poi->m_fuelDetails = new OnboardFuelDetailsImpl((*it)->GetPlace()->GetFuelProductArray());
        }
        if ((*it)->GetSearchFilter() && (*it)->GetSearchFilter().get())
        {
            poi->m_searchfilter = new SearchFilter(OnboardSearchFilterUtil::toSearchFilter((*it)->GetSearchFilter()));
        }
        if ((*it)->GetRelatedSearch() && (*it)->GetRelatedSearch().get())
        {
            poi->m_relatedSearch = new OnboardRelatedSearchImpl((*it)->GetRelatedSearch());
        }

        m_results.push_back(poi);
    }

    // Suggest match
    protocol::SuggestMatchSharedPtrList suggestions = m_information->GetSuggestMatchArray();
    protocol::SuggestMatchSharedPtrArray::iterator suggestIterator = suggestions->begin();
    for (; suggestIterator != suggestions->end(); ++suggestIterator)
    {
        m_results.push_back(new OnboardSuggestionMatchImpl(*suggestIterator));
    }

    // Suggest list
    protocol::SuggestListSharedPtrList suggestionLists = m_information->GetSuggestListArray();
    protocol::SuggestListSharedPtrArray::iterator suggestListIterator = suggestionLists->begin();
    for (; suggestListIterator != suggestionLists->end(); ++suggestListIterator)
    {
        m_results.push_back(new OnboardSuggestionListImpl(*suggestListIterator));
    }

    // Result description
    if (!m_information->GetResultDescriptionArray()->empty())
    {
        protocol::ResultDescriptionSharedPtrList resultDescriptionArray = m_information->GetResultDescriptionArray();
        protocol::ResultDescriptionSharedPtrArray::iterator resultDescriptionIterator = resultDescriptionArray->begin();
        for (; resultDescriptionIterator != resultDescriptionArray->end(); ++resultDescriptionIterator)
        {
            m_resultDescriptions.push_back(new OnboardResultDescriptionImpl(*resultDescriptionIterator));
        }
    }
    */
}

/*! @} */
