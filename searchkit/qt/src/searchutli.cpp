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

/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "searchutil.h"

using namespace locationtoolkit;

#define QSTRING_FROMSTDSTRING(stdstring) QString(stdstring.c_str())
#define STDSTRING_FROMQSTRING(qstring) qstring.toStdString()

SearchStringPair
ToSearchStringPair(const nbsearch::StringPair& pair)
{
    SearchStringPair result;
    result.key = QSTRING_FROMSTDSTRING(pair.GetKey());
    result.value = QSTRING_FROMSTDSTRING(pair.GetValue());
    result.cookie = QSTRING_FROMSTDSTRING(pair.GetPoiAttributeCookie().GetSerialized());
    return result;
}

SearchResultInformationType
ToSearchResultInformationType(const nbsearch::SingleSearchInformationResultType& type)
{
    SearchResultInformationType result = SRIT_None;
    switch (type)
    {
    case nbsearch::SSIRT_None:
        result = SRIT_None;
        break;
    case nbsearch::SSIRT_POI:
        result = SRIT_POI;
        break;
    case nbsearch::SSIRT_SuggestionMatch:
        result = SRIT_Suggestion;
        break;
    case nbsearch::SSIRT_SuggestionList:
        result = SRIT_SuggestionList;
        break;
    case nbsearch::SSIRT_Movie:
        result = SRIT_Movie;
        break;
    case nbsearch::SSIRT_MovieShowing:
        result = SRIT_MovieShowing;
        break;
    case nbsearch::SSIRT_MovieTheater:
        result = SRIT_Theater;
        break;
    default:
        break;
    }
    return result;
}

SearchResultDescription
ToSearchResultDescription(const nbsearch::ResultDescription& description)
{
    SearchResultDescription result;
    result.lines.append(QSTRING_FROMSTDSTRING(description.GetLine1()));
    result.lines.append(QSTRING_FROMSTDSTRING(description.GetLine2()));
    return result;
}

SearchFuelProduct
ToSearchFuelProduct(const nbsearch::FuelProduct& product)
{
    SearchFuelProduct result;
    result.type.code = QSTRING_FROMSTDSTRING(product.GetFuelType()->GetCode());
    result.type.productName = QSTRING_FROMSTDSTRING(product.GetFuelType()->GetProductName());
    result.type.typeName = QSTRING_FROMSTDSTRING(product.GetFuelType()->GetTypeName());
    result.price.value = product.GetPrice()->GetValue();
    result.price.currency = QSTRING_FROMSTDSTRING(product.GetPrice()->GetCurrency());
    result.price.modTime = product.GetPrice()->GetModTime();
    result.units = QSTRING_FROMSTDSTRING(product.GetUnits());
    return result;
}

SearchFilter
ToSearchFilter(const nbsearch::SearchFilter& filter)
{
    SearchFilter result;
    result.serialized = QSTRING_FROMSTDSTRING(filter.GetSerialized());
    result.resultType = QSTRING_FROMSTDSTRING(filter.GetResultStyle());
    for(size_t i =0; i < filter.GetPairs().size(); i++)
    {
        result.pairs.append(ToSearchStringPair(filter.GetPairs()[i]));
    }
    return result;
}

SearchRelatedSearch
ToSearchRelatedSearch(const nbsearch::RelatedSearch& relatedSearch)
{
    SearchRelatedSearch result;
    result.line = QSTRING_FROMSTDSTRING(relatedSearch.GetLine1());
    if(relatedSearch.GetSearchFilter())
    {
        result.searchFilter = ToSearchFilter(*(relatedSearch.GetSearchFilter()));
    }
    return result;
}

SearchPOIContent
ToSearchPOIContent(const nbsearch::POIContent& content)
{
    SearchPOIContent result;
    result.poiContentId = QSTRING_FROMSTDSTRING(content.GetId());
    if(content.GetGoldenCookie())
    {
        result.goldenCookie = QSTRING_FROMSTDSTRING(content.GetGoldenCookie()->GetSerialized());
    }
    std::string tagLine;
    bool hasTagLine = content.GetTagLine(tagLine);
    if(hasTagLine)
    {
        result.tagLine = QSTRING_FROMSTDSTRING(tagLine);
    }
    if(content.GetHoursOfOperation())
    {
        uint32 utcOffset = 0;
        result.hoursOfOperation.hasUtcOffset = content.GetHoursOfOperation()->GetUTCOffset(utcOffset);
        if(result.hoursOfOperation.hasUtcOffset)
        {
            result.hoursOfOperation.utcOffset = utcOffset;
        }
        else
        {
            result.hoursOfOperation.utcOffset = 0;
        }
        for(size_t i = 0; i < content.GetHoursOfOperation()->GetTimeRanges().size(); i++)
        {
            SearchTimeRange range;
            nbsearch::TimeRange* timeRange = content.GetHoursOfOperation()->GetTimeRanges()[i];
            range.dayOfWeek = (SearchDayOfWeek) timeRange->GetDayOfWeek();
            range.startTime = timeRange->GetStartTime();
            range.endTime = timeRange->GetEndTime();
            result.hoursOfOperation.timeRange.append(range);
        }
    }
    for(size_t i = 0; i < content.GetVendorContents().size(); i++)
    {
        SearchVendorContent vendorContent;
        nbsearch::VendorContent* searchContent = content.GetVendorContents()[i];
        vendorContent.name = QSTRING_FROMSTDSTRING(searchContent->GetName());
        vendorContent.ratingCount = searchContent->GetRatingCount();
        vendorContent.averageRating = searchContent->GetAverageRating();
        for(size_t j = 0; j < searchContent->GetPairs().size(); j++)
        {
            vendorContent.pairs.append(ToSearchStringPair(*(searchContent->GetPairs()[j])));
        }
        result.vendorContents.append(vendorContent);
    }
    for(size_t i = 0; i < content.GetStringPairs().size(); i++)
    {
        nbsearch::StringPair* stringPair = content.GetStringPairs()[i];
        if(stringPair == NULL)
        {
            continue;
        }
        nbsearch::POIKey enumKey = nbsearch::POIContent::convertStringToKey(stringPair->GetKey());
        if(enumKey == (nbsearch::POIKey) - 1)
        {
            continue;
        }
        SearchPOIContentKeyValuePair pair;
        pair.key = (SearchPOIConentKey) (enumKey);
        pair.value = QSTRING_FROMSTDSTRING(stringPair->GetValue());
        pair.attributeCookie = QSTRING_FROMSTDSTRING(stringPair->GetPoiAttributeCookie().GetSerialized());
        result.pairs.append(pair);
    }
    return result;
}

SearchLocation
ToSearchLocation(const nbsearch::Location& location)
{
    SearchLocation result;
    result.name = QSTRING_FROMSTDSTRING(location.GetName());
    result.streetNumer = QSTRING_FROMSTDSTRING(location.GetNumber());
    result.street = QSTRING_FROMSTDSTRING(location.GetStreet());
    result.crossStreet = QSTRING_FROMSTDSTRING(location.GetCrossStreet());
    result.city = QSTRING_FROMSTDSTRING(location.GetCity());
    result.county = QSTRING_FROMSTDSTRING(location.GetCounty());
    result.state = QSTRING_FROMSTDSTRING(location.GetState());
    result.postal = QSTRING_FROMSTDSTRING(location.GetZipCode());
    result.country = QSTRING_FROMSTDSTRING(location.GetCountry());
    result.airport = QSTRING_FROMSTDSTRING(location.GetAirport());
    result.countryName = QSTRING_FROMSTDSTRING(location.GetCountryName());
    result.freeForm = QSTRING_FROMSTDSTRING(location.GetFreeForm());
    result.coordinate.latitude = location.GetCoordinates().GetLatitude();
    result.coordinate.longitude = location.GetCoordinates().GetLongitude();
    result.type = (SearchLocationType) (location.GetType());
    size_t len = location.GetExtendedAddress().GetLines().size();
    for(size_t i = 0; i < len; i++)
    {
        result.extendedAddress.addressLines.append(QSTRING_FROMSTDSTRING(location.GetExtendedAddress().GetLines()[i]));
    }

    result.compactAddress.lineSeparator = QSTRING_FROMSTDSTRING(location.GetCompactAddress().GetLineSeparator());
    result.compactAddress.formattedLine = QSTRING_FROMSTDSTRING(location.GetCompactAddress().GetAddressLine());
    result.compactAddress.addressLines.append(QSTRING_FROMSTDSTRING(location.GetCompactAddress().GetLine1()));
    result.compactAddress.addressLines.append(QSTRING_FROMSTDSTRING(location.GetCompactAddress().GetLine2()));

    result.UTCOffset = location.GetUTCOffset();
    return result;
}

SearchPhone
ToSearchPhone(const nbsearch::Phone& phone)
{
    SearchPhone result;
    result.type = (SearchPhoneType) phone.GetPhoneType();
    result.country = QSTRING_FROMSTDSTRING(phone.GetCountry());
    result.area = QSTRING_FROMSTDSTRING(phone.GetArea());
    result.number = QSTRING_FROMSTDSTRING(phone.GetNumber());
    result.formattedText = QSTRING_FROMSTDSTRING(phone.GetFormattedNumber());
    return result;
}

SearchCategory*
ToSearchCategory(const nbsearch::Category& category)
{
    SearchCategory* result = new SearchCategory();
    result->code = QSTRING_FROMSTDSTRING(category.GetCode());
    result->name = QSTRING_FROMSTDSTRING(category.GetName());
    if(category.GetParentCategory())
    {
        SearchCategory* parent = ToSearchCategory(*(category.GetParentCategory()));
        result->parent = QSharedPointer<SearchCategory>(parent);
    }
    else
    {
        result->parent.clear();
    }
    return result;
}

SearchWeatherConditions
ToSearchWeatherConditions(const nbsearch::WeatherConditions& conditions)
{
    SearchWeatherConditions result;
    result.temperature = conditions.GetTemperature();
    result.dewPoint = conditions.GetDewPoint();
    result.relativeHumidity = conditions.GetRelHumidity();
    result.windSpeed = conditions.GetWindSpeed();
    result.windDirection = conditions.GetWindDirection();
    result.windGust = conditions.GetWindGust();
    result.pressure = conditions.GetPressure();
    result.condition = QSTRING_FROMSTDSTRING(conditions.GetWeatherCondition());
    result.conditionCode = conditions.GetConditionCode();
    result.sky = QSTRING_FROMSTDSTRING(conditions.GetSkyCover());
    result.ceiling = conditions.GetCloudBaseHeight();
    result.visibility = conditions.GetVisibility();
    result.heatIndex = conditions.GetHeatIndex();
    result.windChill = conditions.GetWindChill();
    result.snowDepth = conditions.GetSnowDepth();
    result.maxTemp6Hour = conditions.GetMaxTempInLast6Hours();
    result.maxTemp24Hour = conditions.GetMaxTempInLast24Hours();
    result.minTemp6Hour = conditions.GetMinTempInLast6Hours();
    result.minTemp24hour = conditions.GetMinTempInLast24Hours();
    result.precipitation3Hour = conditions.GetPrecipitatinInLast3Hours();
    result.precipitation6Hour = conditions.GetPrecipitatinInLast6Hours();
    result.precipitation24Hour = conditions.GetPrecipitationInLast24Hours();
    result.updateTime = conditions.GetUpdateTime();
    result.utcOffset = conditions.GetUTCOffset();
    return result;
}

SearchWeatherForcast
ToSearchWeatherForcast(const nbsearch::WeatherForecast& forecast)
{
    SearchWeatherForcast result;
    result.date = forecast.GetDate();
    result.highTemperature = forecast.GetHighTemperature();
    result.lowTemperature = forecast.GetLowTemperature();
    result.windSpeed = forecast.GetWindSpeed();
    result.windDirection = forecast.GetWindDirection();
    result.precipitationProbability = forecast.GetPrecipitationProbability();
    result.relativeHumidity = forecast.GetRelativeHumidity();
    result.condition = QSTRING_FROMSTDSTRING(forecast.GetWeatherCondition());
    result.conditionCode = forecast.GetConditionCode();
    result.uvIndex = forecast.GetUVIndex();
    result.uvDescription = QSTRING_FROMSTDSTRING(forecast.GetUVDescription());
    result.sunriseTime = QSTRING_FROMSTDSTRING(forecast.GetSunriseTime());
    result.sunsetTime = QSTRING_FROMSTDSTRING(forecast.GetSunsetTime());
    result.moonriseTime = QSTRING_FROMSTDSTRING(forecast.GetMoonriseTime());
    result.moonsetTime = QSTRING_FROMSTDSTRING(forecast.GetMoonsetTime());
    result.moonPhase = forecast.GetMoonPhase();
    result.updateTime = forecast.GetUpdateTime();
    result.utcOffset = forecast.GetUTCOffset();
    return result;
}

SearchWeatherConditionsSummary
ToSearchWeatherConditionsSummary(const nbsearch::WeatherConditionsSummary& summary)
{
    SearchWeatherConditionsSummary result;
    result.temperature = summary.GetTemperature();
    result.condition = QSTRING_FROMSTDSTRING(summary.GetCondition());
    result.conditionCode = summary.GetConditionCode();
    result.maxTemperatureInLast24hours = summary.GetMaxTemperatureIn24Hours();
    result.minTemperatureInLast24hours = summary.GetMinTemperatureIn24Hours();
    result.utcOffset = summary.GetUTCOffset();
    return result;
}

SearchPlace
ToSearchPlace(const nbsearch::Place& place)
{
    SearchPlace result;
    result.name = QSTRING_FROMSTDSTRING(place.GetName());
    if(place.GetPlaceEventCookie())
    {
        result.cookie = QSTRING_FROMSTDSTRING(place.GetPlaceEventCookie()->GetSerialized());
    }
    if(place.GetLocation())
    {
        result.location = ToSearchLocation(*(place.GetLocation()));
    }
    for(size_t i = 0; i < place.GetPhoneNumbers().size(); i++)
    {
        result.phones.append(ToSearchPhone(*(place.GetPhoneNumbers()[i])));
    }
    for(size_t i = 0; i < place.GetCategories().size(); i++)
    {
        QSharedPointer<SearchCategory> category(ToSearchCategory(*(place.GetCategories()[i])));
        result.categories.append(category);
    }
    if(place.GetWeatherConditions())
    {
        result.weatherConditions = ToSearchWeatherConditions(*(place.GetWeatherConditions()));
    }
    for(size_t i = 0; i < place.GetWeatherForecast().size(); i++)
    {
        result.weatherForestcast.append(ToSearchWeatherForcast(*(place.GetWeatherForecast()[i])));
    }
    for(size_t i = 0; i < place.GetWeatherConditionsSummaries().size(); i++)
    {
        result.weatherSummary.append((ToSearchWeatherConditionsSummary(*(place.GetWeatherConditionsSummaries()[i]))));
    }
    return result;
}

SearchPOI*
ToSearchPOI(const nbsearch::POI* poi)
{
    SearchPOI* result = new SearchPOI();
    switch(poi->GetPOIType())
    {
    case nbsearch::POIT_Location:
        result->type = SPOIT_Locaton;
        break;
    case nbsearch::POIT_POI:
    default:
        result->type = SPOIT_POI;
        break;
    }
    result->searchResultType = SRT_POI;
    result->distance = poi->GetDistance();
    result->isPremiumPlacement = poi->IsPremiumPlacement();
    result->isUnmappable = poi->IsUnmappable();
    result->isEnhancedPoi = poi->IsEnhancedPoi();
    if(poi->GetFuelDetails())
    {
        for(size_t i = 0; i < poi->GetFuelDetails()->GetFuelProducts().size(); i++)
        {
            result->fuelDetails.fuelProducts.append(ToSearchFuelProduct(*(poi->GetFuelDetails()->GetFuelProducts()[i])));
        }
    }
    if(poi->GetPlace())
    {
        result->place = ToSearchPlace(*(poi->GetPlace()));
    }
    if(poi->GetSearchFilter())
    {
        result->searchFilter = ToSearchFilter(*(poi->GetSearchFilter()));
    }
    if(poi->GetPOIContent())
    {
        result->poiContent = ToSearchPOIContent(*(poi->GetPOIContent()));
    }
    if(poi->GetRelatedSearch())
    {
        result->relatedSearch = ToSearchRelatedSearch(*(poi->GetRelatedSearch()));
    }
    return result;
}

SearchSuggestion*
ToSearchSuggestion(const nbsearch::SuggestionMatch* suggestion)
{
    SearchSuggestion* result = new SearchSuggestion();
    result->searchResultType = SRT_Suggest;
    result->lines.append(QSTRING_FROMSTDSTRING(suggestion->GetLine1()));
    result->lines.append(QSTRING_FROMSTDSTRING(suggestion->GetLine2()));
    result->lines.append(QSTRING_FROMSTDSTRING(suggestion->GetLine3()));
    for(size_t i = 0; i < suggestion->GetIconIDs().size(); i++)
    {
        result->iconIds.append(QSTRING_FROMSTDSTRING(suggestion->GetIconIDs()[i]));
    }
    result->distance = suggestion->GetDistance();
    result->type = (SearchSuggestionType) suggestion->GetSearchResultType();
    if(suggestion->GetSearchFilter())
    {
        result->filter = ToSearchFilter(*(suggestion->GetSearchFilter()));
    }
    return result;
}

SearchSuggestionList*
ToSearchSuggestionList(const nbsearch::SuggestionList* list)
{
    SearchSuggestionList* result = new SearchSuggestionList();
    result->searchResultType = SRT_SuggestList;
    result->name = QSTRING_FROMSTDSTRING(list->GetName());
    for(size_t i = 0; i < list->GetSuggestionMatches().size(); i++)
    {
        result->suggestions.append(*(ToSearchSuggestion(list->GetSuggestionMatches()[i])));
    }
    return result;
}

SearchEventContent
ToSearchEventContent(const nbsearch::EventContent& content)
{
    SearchEventContent result;
    result.name = QSTRING_FROMSTDSTRING(content.GetName());
    result.url = QSTRING_FROMSTDSTRING(content.GetURL());
    result.mpaaRating = QSTRING_FROMSTDSTRING(content.GetMPAARating());
    result.formattedText = QSTRING_FROMSTDSTRING(content.GetFormatedText());
    result.fixedDate = QDate(content.GetFixedDate()->GetYear(), content.GetFixedDate()->GetMonth(),
                             content.GetFixedDate()->GetDay());
    if(content.GetPlaceEventCookie())
    {
        result.cookie = QSTRING_FROMSTDSTRING(content.GetPlaceEventCookie()->GetSerialized());
    }
    for(size_t i = 0; i < content.GetCategories().size(); i++)
    {
        QSharedPointer<SearchCategory> category(ToSearchCategory(*(content.GetCategories()[i])));
        result.categories.append(category);
    }
    for(size_t i = 0; i < content.GetPairs().size(); i++)
    {
        result.pairs.append(ToSearchStringPair(*(content.GetPairs()[i])));
    }
    return result;
}

SearchResult*
ToSearchResultFromProxMatchContent(const nbsearch::ProxMatchContent* content)
{
    SearchResult* result = NULL;
    if(content->GetEventContent()->GetType() == nbsearch::ECT_Movie)
    {
        SearchMovie* movie = new SearchMovie();
        movie->searchResultType = SRT_ProxMatchContent;
        if(content->GetSearchFilter())
        {
            movie->searchFilter = ToSearchFilter(*(content->GetSearchFilter()));
        }
        if(content->GetRelatedSearch())
        {
            movie->relatedSearch = ToSearchRelatedSearch(*(content->GetRelatedSearch()));
        }
        if(content->GetEventContent())
        {
            movie->movieContent.event = ToSearchEventContent(*(content->GetEventContent()));
            for(size_t i = 0; i < content->GetEventContent()->GetPairs().size(); i++)
            {
                nbsearch::StringPair* pair = content->GetEventContent()->GetPairs()[i];
                if(pair)
                {
                    if(pair->GetKey() == "movie-id")
                    {
                        continue;
                    }
                    else if(pair->GetKey() == "genre")
                    {
                        movie->movieContent.genre = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "duration")
                    {
                        movie->movieContent.duration = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "description")
                    {
                        movie->movieContent.movieDesciption = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "language")
                    {
                        movie->movieContent.language = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "actors")
                    {
                        movie->movieContent.actors = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "director")
                    {
                        movie->movieContent.director = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "film-format")
                    {
                        movie->movieContent.filmFormat = QSTRING_FROMSTDSTRING(pair->GetValue());
                    }
                    else if(pair->GetKey() == "poster-url")
                    {
                        movie->movieContent.posterURLs.append(QSTRING_FROMSTDSTRING(pair->GetValue()));
                    }
                    else if(pair->GetKey() == "photo-url")
                    {
                        movie->movieContent.photoURLs.append(QSTRING_FROMSTDSTRING(pair->GetValue()));
                    }
                }
            }
        }
        result = movie;
    }
    return result;
}

SearchResult*
ToSearchResult(const nbsearch::SearchResultBase* base)
{
    SearchResult* result = NULL;
    switch(base->GetSearchResultType())
    {
    case nbsearch::SRT_POI:
        result = ToSearchPOI(dynamic_cast<const nbsearch::POI*>(base));
        break;
    case nbsearch::SRT_SuggestMatch:
        result = ToSearchSuggestion(dynamic_cast<const nbsearch::SuggestionMatch*>(base));
        break;
    case nbsearch::SRT_SuggestList:
        result = ToSearchSuggestionList(dynamic_cast<const nbsearch::SuggestionList*>(base));
        break;
    case nbsearch::SRT_ProxMatchContent:
        result = ToSearchResultFromProxMatchContent(dynamic_cast<const nbsearch::ProxMatchContent*>(base));
        break;
    case nbsearch::SRT_None:
    default:
        break;
    }
    return result;
}

SearchInformation*
SearchTranslator::toSearchInformation(const nbsearch::SingleSearchInformation &information)
{
    SearchInformation* result = new SearchInformation();
    result->fuelSummary.reset();
    result->type = ToSearchResultInformationType(information.GetResultType());
    result->hasMore = information.HasMore();
    result->state.serialized = QSTRING_FROMSTDSTRING(information.GetResultState()->GetSerialized());
    for(size_t i = 0; i < information.GetResultDescriptions().size(); i++)
    {
        nbsearch::ResultDescription* description= information.GetResultDescriptions()[i];
        if(description)
        {
            result->descriptions.append(ToSearchResultDescription(*(description)));
        }
    }

    if(information.GetFuelSummary())
    {
        result->fuelSummary.reset(new SearchFuelSummary());
        result->type = SRIT_Fuel;
        const nbsearch::FuelProduct* product = information.GetFuelSummary()->GetAverage();
        if(product)
        {
            result->fuelSummary->average = ToSearchFuelProduct(*product);
        }
        product = information.GetFuelSummary()->GetLow();
        if(product)
        {
            result->fuelSummary->low = ToSearchFuelProduct(*product);
        }
    }

    for(int i = 0; i < information.GetResultCount(); ++i)
    {
        const nbsearch::SearchResultBase* base = information.GetResultAtIndex(i);
        if(base)
        {
            SearchResult* searchResult = ToSearchResult(base);
            if(searchResult)
            {
                result->results.append(QSharedPointer<SearchResult>(searchResult));
                if (base->GetSearchResultType() == nbsearch::SRT_POI)
                {
                    SearchExtendedAddress sea;
                    SearchCompactAddress sca;
                    SearchHoursOfOperation hoo;
                    const nbsearch::POI *poi = dynamic_cast<const nbsearch::POI*>(base);
                    if (poi)
                    {
                        const nbsearch::POIContent *poicontent = poi->GetPOIContent();
                        const SearchPOI *spoi = dynamic_cast<const SearchPOI *>(searchResult);
                    	if (poi->GetPlace())
                    	{
                        	result->extendedAddress.append(spoi->place.location.extendedAddress);
                        	result->compactAddress.append(spoi->place.location.compactAddress);
                    	}
                    	else
                    	{
                        	result->extendedAddress.append(sea);
                        	result->compactAddress.append(sca);
                    	}

                    	if (poicontent)
                    	{
                            if (poicontent->GetHoursOfOperation() && spoi)
                            {
                                result->hoursOfOperation.append(spoi->poiContent.hoursOfOperation);
                            }
                            else
                            {
                                result->hoursOfOperation.append(hoo);
                            }
                        }
                        else
                        {
                            result->hoursOfOperation.append(hoo);
                        }
                    }
                    else
                    {
                        result->extendedAddress.append(sea);
                        result->compactAddress.append(sca);
                        result->hoursOfOperation.append(hoo);
                    }
                }
            }
        }
    }
    return result;
}

nbsearch::SearchFilter*
SearchTranslator::toSearchFilter(const SearchFilter& filter)
{
    nbsearch::SearchFilter* impl = new nbsearch::SearchFilter();
    impl->m_serialized = STDSTRING_FROMQSTRING(filter.serialized);
    impl->m_resultStyle = STDSTRING_FROMQSTRING(filter.resultType);
    for(size_t i = 0; i < filter.pairs.size(); i++)
    {
        impl->m_pairs.push_back(toStringPair(filter.pairs[i]));
    }
    return impl;
}

nbsearch::StringPair
SearchTranslator::toStringPair(const SearchStringPair &pair)
{
    nbsearch::StringPair impl;
    impl.m_key = STDSTRING_FROMQSTRING(pair.key);
    impl.m_value = STDSTRING_FROMQSTRING(pair.value);
    if(!pair.cookie.isEmpty())
    {
        impl.m_cookie.m_serialized = STDSTRING_FROMQSTRING(pair.cookie);
    }
    return impl;
}

nbsearch::Location*
SearchTranslator::toLocation(const SearchLocation &location)
{
    nbsearch::Location* impl = new nbsearch::Location();
    impl->m_type = (nbsearch::LocationType) location.type;
    impl->m_name = STDSTRING_FROMQSTRING(location.name);
    impl->m_number = STDSTRING_FROMQSTRING(location.streetNumer);
    impl->m_street = STDSTRING_FROMQSTRING(location.street);
    impl->m_crossStreet = STDSTRING_FROMQSTRING(location.crossStreet);
    impl->m_city = STDSTRING_FROMQSTRING(location.city);
    impl->m_country = STDSTRING_FROMQSTRING(location.country);
    impl->m_state = STDSTRING_FROMQSTRING(location.state);
    impl->m_zipcode = STDSTRING_FROMQSTRING(location.postal);
    impl->m_county = STDSTRING_FROMQSTRING(location.county);
    impl->m_airport = STDSTRING_FROMQSTRING(location.airport);
    impl->m_countryName = STDSTRING_FROMQSTRING(location.countryName);
    impl->m_freeForm = STDSTRING_FROMQSTRING(location.freeForm);
    impl->m_point = nbsearch::LatLonPoint(location.coordinate.latitude, location.coordinate.longitude);
    for(size_t i = 0; i < location.extendedAddress.addressLines.size(); i++)
    {
        impl->m_extAddress.m_lines.push_back(STDSTRING_FROMQSTRING(location.extendedAddress.addressLines[i]));
    }
    impl->m_compAddress.m_lineSeparator = STDSTRING_FROMQSTRING(location.compactAddress.lineSeparator);
    impl->m_compAddress.m_addressLine = STDSTRING_FROMQSTRING(location.compactAddress.formattedLine);
    if(location.compactAddress.addressLines.size() >=2)
    {
        impl->m_compAddress.m_line2 = STDSTRING_FROMQSTRING(location.compactAddress.addressLines[1]);
    }
    if(location.compactAddress.addressLines.size() >=1)
    {
        impl->m_compAddress.m_line1 = STDSTRING_FROMQSTRING(location.compactAddress.addressLines[0]);
    }
    return impl;
}

SearchLocation
ToSearchLocation(const nbcommon::Location* geocodelocation)
{
    SearchLocation searchLocation;
    if(geocodelocation)
    {
        searchLocation.name = QString(geocodelocation->GetName());
        searchLocation.streetNumer = QString(geocodelocation->GetNumber());
        searchLocation.street = QString(geocodelocation->GetStreet());
        searchLocation.crossStreet = QString(geocodelocation->GetCrossStreet());
        searchLocation.city = QString(geocodelocation->GetCity());
        searchLocation.county = QString(geocodelocation->GetCountry());
        searchLocation.state = QString(geocodelocation->GetState());
        searchLocation.postal = QString(geocodelocation->GetZipCode());
        searchLocation.country = QString(geocodelocation->GetCountry());
        searchLocation.airport = QString(geocodelocation->GetAirport());
        searchLocation.freeForm = QString(geocodelocation->GetFreeForm());
        if(geocodelocation->GetCoordinates())
        {
            searchLocation.coordinate.latitude = geocodelocation->GetCoordinates()->GetLatitude();
            searchLocation.coordinate.longitude = geocodelocation->GetCoordinates()->GetLongitude();
        }
        searchLocation.type = (locationtoolkit::SearchLocationType)geocodelocation->GetType();
        if(geocodelocation->GetExtendedAddress())
        {
            for(size_t i = 0; i < geocodelocation->GetExtendedAddress()->GetLines().size(); i++)
            {
                searchLocation.extendedAddress.addressLines.append(QSTRING_FROMSTDSTRING(geocodelocation->GetExtendedAddress()->GetLines()[i]));
            }
        }
        if(geocodelocation->GetCompactAddress())
        {
            searchLocation.compactAddress.lineSeparator = QSTRING_FROMSTDSTRING(geocodelocation->GetCompactAddress()->GetLineSeparator());
            searchLocation.compactAddress.formattedLine = QSTRING_FROMSTDSTRING(geocodelocation->GetCompactAddress()->GetAddressLine());
            searchLocation.compactAddress.addressLines.append(QSTRING_FROMSTDSTRING(geocodelocation->GetCompactAddress()->GetLine1()));
            searchLocation.compactAddress.addressLines.append(QSTRING_FROMSTDSTRING(geocodelocation->GetCompactAddress()->GetLine2()));
        }
        searchLocation.UTCOffset = geocodelocation->GetUTCOffset();
    }
    return searchLocation;
}

SearchInformation *
SearchTranslator::toSearchInformation(const nbcommon::ReverseGeocodeInformation &information)
{
    SearchInformation* result = new SearchInformation();
    result->type = locationtoolkit::SRIT_POI;
    result->hasMore = false;
    QSharedPointer<SearchPOI> searchPOI(new SearchPOI());
    searchPOI->place.location = ToSearchLocation(information.GetLocation());
    searchPOI->type = locationtoolkit::SPOIT_Locaton;
    searchPOI->searchResultType = locationtoolkit::SRT_POI;
    result->results.append(searchPOI);
    return result;
}

SingelSearchInformationImpl::SingelSearchInformationImpl(const SearchInformation &information)
    :mResultState(NULL)
{
    mResultState = new ResultStateImpl();
    mResultState->mSerialized = STDSTRING_FROMQSTRING(information.state.serialized);
}

SingelSearchInformationImpl::~SingelSearchInformationImpl()
{
    if(mResultState)
    {
        delete mResultState;
    }
}
