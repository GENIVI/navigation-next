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
    @file     SingleSearchParameter.cpp
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

#include "SingleSearchParameters.h"
#include "cslutil.h"
#include "offboard/SingleSearchOffboardInformation.h"

using namespace nbsearch;
using namespace protocol;
using namespace std;

const static char* SearchSource_MainScreen    = "main-screen";
const static char* SearchSource_AddressScreen = "address-screen";
//const static char* SearchSource_PlaceScreen   = "place-screen";
//const static char* SearchSource_AirportScreen = "airport-screen";
const static char* SearchSource_MovieScreen   = "movie-screen";
const static char* SearchSource_ExploreScreen = "explore-screen";
const static char* SearchSource_PoiLayer      = "poi-layer";

const static char* ContentTypes[] = {"poi", "address", "movie", "fuel-price"};

const static char* Command_Start = "start";
const static char* Command_Next  = "next";
const static char* Command_Prev  = "prev";

const static char* ResultStyle_Suggest             = "suggest";
const static char* ResultStyle_SingleSearch        = "single-search";
const static char* ResultStyle_ListInterestSuggest = "list-interest-suggest";
const static char* ResultStyle_AddInterestSuggest  = "add-interest-suggest";
const static char* ResultStyle_MovieList           = "movie-list";
const static char* ResultStyle_MovieTheater        = "movie-theater";
const static char* ResultStyle_AddressOnly         = "address-only";

const static char* MovieShowingType_NowInTheater    = "NowInTheater";
const static char* MovieShowingType_OpeningThisWeek = "OpeningThisWeek";
const static char* MovieShowingType_ComingSoon      = "ComingSoon";

const static char* MovieSortBy_MostPopular          = "MostPopular";
const static char* MovieSortBy_MostRecent           = "MostRecent";

const static char* Movie_Showing_Type               = "showing";
const static char* Movie_Sort_By                    = "sort-by";
const static char* Movie_ID                         = "movie-id";
const static char* Theater_ID                       = "theater-id";

const static char* WeatherType_All                  = "all";
const static char* WeatherType_Current              = "current";
const static char* WeatherType_Forecast             = "forecast";

SingleSearchParameters::SingleSearchParameters(const SingleSearchConfiguration& config)
{
    m_searchScheme          = config.m_scheme;
    m_sliceSize             = config.m_sliceSize;
    m_language              = config.m_language;
    m_searchSource          = config.m_searchSource;
    m_direction             = config.m_direction;
    m_extendedConfiguration = config.m_extendedConfiguration;
    m_searchInputMethod     = config.m_inputMethod;
    m_searchPoint           = LatitudeLongitude();
    m_originPoint           = LatitudeLongitude();

    m_type                  = WT_None;
    m_leftTopPoint          = LatitudeLongitude();
    m_rightBottomPoint      = LatitudeLongitude();
    m_onboardOnly           = config.m_onboardOnly;
    m_resultStyleType       = NB_SRST_None;
}

SingleSearchParameters::SingleSearchParameters(const SingleSearchParameters& param)
{
    m_name                  = param.m_name;
    m_resultState           = param.m_resultState;
    m_searchScheme          = param.m_searchScheme;
    m_categoryCode          = param.m_categoryCode;
    m_sliceSize             = param.m_sliceSize;
    m_language              = param.m_language;
    m_searchSource          = param.m_searchSource;
    m_direction             = param.m_direction;
    m_searchPoint           = param.m_searchPoint;
    m_originPoint           = param.m_originPoint;
    m_extendedConfiguration = param.m_extendedConfiguration;
    m_searchInputMethod     = param.m_searchInputMethod;
    m_type                  = param.m_type;
    m_leftTopPoint          = param.m_leftTopPoint;
    m_rightBottomPoint      = param.m_rightBottomPoint;
    m_onboardOnly           = param.m_onboardOnly;
    m_resultStyleType       = param.m_resultStyleType;
}

const char* SingleSearchParameters::GetIteratorCommand() const
{
    if (m_direction == SD_Prev)
    {
        return Command_Prev;
    }
    else if (m_direction == SD_Next)
    {
        return Command_Next;
    }
    return Command_Start;
}

SingleSearchParametersByResultStyle::SingleSearchParametersByResultStyle(const SingleSearchConfiguration& config)
    : SingleSearchParameters(config)
{
    m_resultStyleType   = config.m_resultStyleType;
    m_param.reset();
}

SingleSearchParametersByResultStyle::~SingleSearchParametersByResultStyle()
{
    if (m_param)
    {
        m_param.reset();
    }
}

SingleSearchParametersByResultStyle::SingleSearchParametersByResultStyle(const SingleSearchParametersByResultStyle& param)
    : SingleSearchParameters(param)
{
    m_resultStyleType       = param.m_resultStyleType;
}

SingleSearchParametersByResultStyle* SingleSearchParametersByResultStyle::clone() const
{
    return new SingleSearchParametersByResultStyle(*this);
}

void SingleSearchParametersByResultStyle::SetOriginPoint(double latitude, double longitude)
{
    m_originPoint.SetLatitude(latitude);
    m_originPoint.SetLongitude(longitude);
}

void SingleSearchParametersByResultStyle::SetSearchPoint(double latitude, double longitude)
{
    m_searchPoint.SetLatitude(latitude);
    m_searchPoint.SetLongitude(longitude);
}


void SingleSearchParametersByResultStyle::SetIteratorCommand(SearchDirection direction, string resultState)
{
    m_direction   = direction;
    m_resultState = resultState;
}

const char* SingleSearchParametersByResultStyle::GetResultStyle() const
{
    if (m_resultStyleType == NB_SRST_Suggestion)
    {
        return ResultStyle_Suggest;
    }
    else if(m_resultStyleType == NB_SRST_ListInterestSuggestion)
    {
        return ResultStyle_ListInterestSuggest;
    }
    else if(m_resultStyleType == NB_SRST_AddInterestSuggestion)
    {
        return ResultStyle_AddInterestSuggest;
    }
    else if(m_resultStyleType == NB_SRST_MovieList)
    {
        return ResultStyle_MovieList;
    }
    else if(m_resultStyleType == NB_SRST_MovieTheater)
    {
        return ResultStyle_MovieTheater;
    }
    else if(m_resultStyleType == NB_SRST_AddressOnly)
    {
        return ResultStyle_AddressOnly;
    }

    return ResultStyle_SingleSearch;
}

const char* SingleSearchParametersByResultStyle::GetSearchFilterPairValueBySource() const
{
    if (m_searchSource == NB_SS_Address)
    {
        return SearchSource_AddressScreen;
    }
    else if (m_searchSource == NB_SS_Movie)
    {
        return SearchSource_MovieScreen;
    }
    else if (m_searchSource == NB_SS_Explore)
    {
        return SearchSource_ExploreScreen;
    }
    else if (m_searchSource == NB_SS_POILayer)
    {
        return SearchSource_PoiLayer;
    }
    return SearchSource_MainScreen;
}

SingleSearchSourceParametersSharedPtr SingleSearchParametersByResultStyle::CreateParameters()
{
    m_param = SingleSearchSourceParametersSharedPtr(new SingleSearchSourceParameters());

    //scheme
    shared_ptr<string> scheme(new string(m_searchScheme));
    if (!scheme)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetDataScheme(scheme);

    //language
    shared_ptr<string> language(new string(m_language));
    if (!language)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetLanguage(language);

    //position
    PositionSharedPtr position(new Position);
    if (!position)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    position->SetVariant(shared_ptr<string>(new std::string("point")));
    PointSharedPtr point(new Point);
    if (!point)
    {
        position.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    point->SetLat(m_searchPoint.GetLatitude());
    point->SetLon(m_searchPoint.GetLongitude());
    position->SetPoint(point);

    if(point->GetLat() != INVALID_LATITUDE && point->GetLon() != INVALID_LONGITUDE)
    {
        m_param->GetPositionArray()->push_back(position);
    }

    //geographic position
    PositionSharedPtr position2(new Position);
    if (!position2)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    position2->SetVariant(shared_ptr<string>(new std::string("geographic-position")));
    GeographicPositionSharedPtr geographicPositon(new GeographicPosition());
    if (!geographicPositon)
    {
        position2.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    geographicPositon->SetLatitude(m_originPoint.GetLatitude());
    geographicPositon->SetLongitude(m_originPoint.GetLongitude());
    position2->SetGeographicPosition(geographicPositon);

    if(geographicPositon->GetLatitude() != INVALID_LATITUDE && geographicPositon->GetLongitude() != INVALID_LONGITUDE)
    {
        m_param->GetPositionArray()->push_back(position2);
    }

    //iter-command
    IterCommandSharedPtr iterCommand(new IterCommand);
    if (!iterCommand)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }

    shared_ptr<string> command(new string(GetIteratorCommand()));
    if (!command)
    {
        iterCommand.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    iterCommand->SetCommand(command);
    iterCommand->SetObjectCount(m_sliceSize);
    if(!m_resultState.empty())
    {
        iterCommand->SetState(ResultStateImpl::Deserialize(m_resultState));
    }

    m_param->SetIterCommand(iterCommand);

    //search-filter
    SearchFilterSharedPtr searchFilter(new protocol::SearchFilter);
    if (!searchFilter)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    ResultStyleSharedPtr resultStyle(new ResultStyle);
    if (!resultStyle)
    {
        searchFilter.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    shared_ptr<string> resultStyleKey(new string(GetResultStyle()));
    if (!resultStyle)
    {
        resultStyle.reset();
        searchFilter.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    resultStyle->SetKey(resultStyleKey);
    searchFilter->SetResultStyle(resultStyle);

    if (m_name.length() > 0)
    {
        PairSharedPtr pair(new Pair);
        if (!pair)
        {
            searchFilter.reset();
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        shared_ptr<string> name(new string("name"));
        shared_ptr<string> value(new string(m_name));
        if (!name || !value)
        {

            resultStyle.reset();
            searchFilter.reset();
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        pair->SetKey(name);
        pair->SetValue(value);
        searchFilter->GetSearchKeyArray()->push_back(pair);
    }

    PairSharedPtr source(new Pair);
    if (!source)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    source->SetKey(shared_ptr<std::string>(new string("source")));
    source->SetValue(shared_ptr<std::string>(new string(GetSearchFilterPairValueBySource())));
    searchFilter->GetSearchKeyArray()->push_back(source);

    if (m_categoryCode.length() > 0)
    {
        shared_ptr<string> category(new string("category"));
        shared_ptr<string> categoryVaule(new string(m_categoryCode));
        PairSharedPtr categoryPair(new Pair);
        categoryPair->SetKey(category);
        categoryPair->SetValue(categoryVaule);
        if (!category || !categoryVaule || !categoryPair)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        searchFilter->GetSearchKeyArray()->push_back(categoryPair);
        searchFilter->SetResultStyle(resultStyle);
    }
    m_param->SetSearchFilter(searchFilter);

    //want-xxx configuration
    if ((m_extendedConfiguration & NB_EXT_WantContentTypes) != 0)
    {
        int size = sizeof(ContentTypes)/sizeof(const char*);
        shared_ptr<WantContentTypes> wantContentTypes(new WantContentTypes);
        if (!wantContentTypes)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        for (int i = 0; i < size; ++i)
        {
            shared_ptr<ContentType> contentType(new ContentType);
            if (!contentType)
            {
                m_param.reset();
                return SingleSearchSourceParametersSharedPtr();
            }
            contentType->SetType(shared_ptr<string>(new string(ContentTypes[i])));
            wantContentTypes->GetContentTypeArray()->push_back(contentType);
        }
        m_param->SetWantContentTypes(wantContentTypes);
    }

    m_param->SetWantCompactAddress((m_extendedConfiguration & NB_EXT_WantCompactAddress) != 0 ? true : false);
    m_param->SetWantExtendedAddress((m_extendedConfiguration & NB_EXT_WantExtendedAddress) != 0 ? true : false);
    m_param->SetWantDistanceToUser((m_extendedConfiguration & NB_EXT_WantDistanceToUser) != 0 ? true : false);
    m_param->SetWantStructuredHoursOfOperation((m_extendedConfiguration & NB_EXT_WantStructuredHoursOfOperation) != 0 ? true : false);
    m_param->SetWantIconId((m_extendedConfiguration & NB_EXT_WantIconId) != 0 ? true : false);
    m_param->SetWantSearchResultId((m_extendedConfiguration & NB_EXT_WantSearchResultId) != 0 ? true : false);
    m_param->SetWantResultDescription((m_extendedConfiguration & NB_EXT_WantResultDescription) != 0 ? true : false);
    m_param->SetWantRelatedSearch((m_extendedConfiguration & NB_EXT_WantRelatedSearch) != 0 ? true : false);
    m_param->SetWantSuggestDistance((m_extendedConfiguration & NB_EXT_WantSuggestDistance) != 0 ? true : false);
    m_param->SetWantPremiumPlacement((m_extendedConfiguration & NB_EXT_WantPremiumPlacement) != 0 ? true : false);
    m_param->SetWantPoiCookies((m_extendedConfiguration & NB_EXT_WantPoiCookies) != 0 ? true : false);

    return m_param;
}

SingleSearchParametersBySearchFilter::SingleSearchParametersBySearchFilter( const SingleSearchConfiguration& config):
    SingleSearchParameters(config)
{
    m_param.reset();
}

SingleSearchParametersBySearchFilter::~SingleSearchParametersBySearchFilter()
{
    if (m_param)
    {
        m_param.reset();
    }
}

SingleSearchParametersBySearchFilter::SingleSearchParametersBySearchFilter(const SingleSearchParametersBySearchFilter& param)
    : SingleSearchParameters(param)
{
    m_searchFilterData      = param.m_searchFilterData;
}

SingleSearchParametersBySearchFilter* SingleSearchParametersBySearchFilter::clone() const
{
    return new SingleSearchParametersBySearchFilter(*this);
}

void SingleSearchParametersBySearchFilter::SetSearchPoint(double latitude, double longitude)
{
    m_searchPoint.SetLatitude(latitude);
    m_searchPoint.SetLongitude(longitude);
}

void SingleSearchParametersBySearchFilter::SetOriginPoint(double latitude, double longitude)
{
    m_originPoint.SetLatitude(latitude);
    m_originPoint.SetLongitude(longitude);
}

void SingleSearchParametersBySearchFilter::SetSearchFilter(const nbsearch::SearchFilter* searchFilter)
{
    if (searchFilter)
    {
        m_searchFilterData = searchFilter->GetSerialized();
    }
}
void SingleSearchParametersBySearchFilter::SetIteratorCommand(SearchDirection direction, string resultState)
{
    m_direction   = direction;
    m_resultState = resultState;
}

SingleSearchSourceParametersSharedPtr SingleSearchParametersBySearchFilter::CreateParameters()
{
    m_param = SingleSearchSourceParametersSharedPtr(new SingleSearchSourceParameters());

    //scheme
    shared_ptr<string> scheme(new string(m_searchScheme));
    if (!scheme)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetDataScheme(scheme);

    //language
    shared_ptr<string> language(new string(m_language));
    if (!language)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetLanguage(language);

    //position
    PositionSharedPtr position(new Position);
    if (!position)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    position->SetVariant(shared_ptr<string>(new std::string("point")));
    PointSharedPtr point(new Point);
    if (!point)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    point->SetLat(m_searchPoint.GetLatitude());
    point->SetLon(m_searchPoint.GetLongitude());
    position->SetPoint(point);

    m_param->GetPositionArray()->push_back(position);

    //geographic position
    PositionSharedPtr position2(new Position);
    if (!position2)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    position2->SetVariant(shared_ptr<string>(new std::string("geographic-position")));
    GeographicPositionSharedPtr geographicPositon(new GeographicPosition());
    if (!geographicPositon)
    {
        position2.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    geographicPositon->SetLatitude(m_originPoint.GetLatitude());
    geographicPositon->SetLongitude(m_originPoint.GetLongitude());
    position2->SetGeographicPosition(geographicPositon);

    m_param->GetPositionArray()->push_back(position2);

    //iter-command
    IterCommandSharedPtr iterCommand(new IterCommand);
    shared_ptr<string> command(new string(GetIteratorCommand()));
    if (!command)
    {
        iterCommand.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    iterCommand->SetCommand(command);
    iterCommand->SetObjectCount(m_sliceSize);
    if(!m_resultState.empty())
    {
        iterCommand->SetState(ResultStateImpl::Deserialize(m_resultState));
    }
    m_param->SetIterCommand(iterCommand);

    //search-filter
    SearchFilterSharedPtr searchFilter = SearchFilterUtil::Deserialize(m_searchFilterData);
    m_param->SetSearchFilter(searchFilter);

    //want-xxx configuration
    if ((m_extendedConfiguration & NB_EXT_WantContentTypes) != 0)
    {
        int size = sizeof(ContentTypes)/sizeof(const char*);
        shared_ptr<WantContentTypes> wantContentTypes(new WantContentTypes);
        if (!wantContentTypes)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        for (int i = 0; i < size; ++i)
        {
            shared_ptr<ContentType> contentType(new ContentType);
            if (!contentType)
            {
                m_param.reset();
                return SingleSearchSourceParametersSharedPtr();
            }
            contentType->SetType(shared_ptr<string>(new string(ContentTypes[i])));
            wantContentTypes->GetContentTypeArray()->push_back(contentType);
        }
        m_param->SetWantContentTypes(wantContentTypes);
    }

    m_param->SetWantCompactAddress((m_extendedConfiguration & NB_EXT_WantCompactAddress) != 0 ? true : false);
    m_param->SetWantExtendedAddress((m_extendedConfiguration & NB_EXT_WantExtendedAddress) != 0 ? true : false);
    m_param->SetWantDistanceToUser((m_extendedConfiguration & NB_EXT_WantDistanceToUser) != 0 ? true : false);
    m_param->SetWantStructuredHoursOfOperation((m_extendedConfiguration & NB_EXT_WantStructuredHoursOfOperation) != 0 ? true : false);
    m_param->SetWantIconId((m_extendedConfiguration & NB_EXT_WantIconId) != 0 ? true : false);
    m_param->SetWantSearchResultId((m_extendedConfiguration & NB_EXT_WantSearchResultId) != 0 ? true : false);
    m_param->SetWantResultDescription((m_extendedConfiguration & NB_EXT_WantResultDescription) != 0 ? true : false);
    m_param->SetWantRelatedSearch((m_extendedConfiguration & NB_EXT_WantRelatedSearch) != 0 ? true : false);
    m_param->SetWantSuggestDistance((m_extendedConfiguration & NB_EXT_WantSuggestDistance) != 0 ? true : false);
    m_param->SetWantPremiumPlacement((m_extendedConfiguration & NB_EXT_WantPremiumPlacement) != 0 ? true : false);
    m_param->SetWantPoiCookies((m_extendedConfiguration & NB_EXT_WantPoiCookies) != 0 ? true : false);

    return m_param;
}

MovieSingleSearchParameters::MovieSingleSearchParameters(const SingleSearchConfiguration& config, MovieShowingType showingType)
    : SingleSearchParametersByResultStyle(config)
{
    m_idType      = SSRIT_None;
    m_showingType = showingType;
    m_startTime   = 0;
}

MovieSingleSearchParameters::MovieSingleSearchParameters(const SingleSearchConfiguration& config, SingleSearchResultIdType idType, std::string value, uint32 startTime)
    : SingleSearchParametersByResultStyle(config)
{
    m_idType      = idType;
    m_showingType = MST_None;
    m_startTime   = startTime;
    m_idValue     = value;
}

MovieSingleSearchParameters::MovieSingleSearchParameters(const MovieSingleSearchParameters& param)
    : SingleSearchParametersByResultStyle(param)
{

    m_idType      = param.m_idType;
    m_showingType = param.m_showingType;
    m_startTime   = param.m_startTime;
    m_idValue     = param.m_idValue;
}

MovieSingleSearchParameters::~MovieSingleSearchParameters()
{
}

MovieSingleSearchParameters* MovieSingleSearchParameters::clone() const
{
    return new MovieSingleSearchParameters(*this);
}

SingleSearchSourceParametersSharedPtr MovieSingleSearchParameters::CreateParameters()
{
    SingleSearchParametersByResultStyle::CreateParameters();

    if (m_showingType != MST_None)
    {
        PairSharedPtr pair = PairGenerator("genre", "All");
        if (!pair)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        m_param->GetSearchFilter()->GetSearchKeyArray()->push_back(pair);
    }

    PairSharedPtr pairShowing;
    PairSharedPtr pairSortBy;
    if (m_showingType == MST_OpeningThisWeek)
    {
        pairShowing = PairGenerator(Movie_Showing_Type, MovieShowingType_OpeningThisWeek);
        pairSortBy = PairGenerator(Movie_Sort_By, MovieSortBy_MostRecent);
    }
    else if (m_showingType == MST_NowInTheater)
    {
        pairShowing = PairGenerator(Movie_Showing_Type, MovieShowingType_NowInTheater);
        pairSortBy = PairGenerator(Movie_Sort_By, MovieSortBy_MostPopular);
    }
    else if (m_showingType == MST_ComingSoon)
    {
        pairShowing = PairGenerator(Movie_Showing_Type, MovieShowingType_ComingSoon);
        pairSortBy = PairGenerator(Movie_Sort_By, MovieSortBy_MostRecent);
    }

    if (pairShowing && pairSortBy)
    {
        m_param->GetSearchFilter()->GetSearchKeyArray()->push_back(pairShowing);
        m_param->GetSearchFilter()->GetSearchKeyArray()->push_back(pairSortBy);
    }

    const char* id = "";
    if (m_idType == SSRIT_Movie)
    {
        id = Movie_ID;
    }
    else if (m_idType == SSRIT_Theater)
    {
        id = Theater_ID;
    }

    if (nsl_strlen(id) > 0 && m_startTime != 0)
    {
        PairSharedPtr pairId = PairGenerator(id, m_idValue.c_str());
        if (!pairId)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        m_param->GetSearchFilter()->GetSearchKeyArray()->push_back(pairId);

        char buff[20];
        nsl_snprintf(buff, 20, "%u", m_startTime);
        PairSharedPtr pairStartTime = PairGenerator("start-time", buff);
        if (!pairStartTime)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        m_param->GetSearchFilter()->GetSearchKeyArray()->push_back(pairStartTime);
    }

    return m_param;
}

PairSharedPtr MovieSingleSearchParameters::PairGenerator(const char* key, const char* value)
{
    PairSharedPtr pair(new Pair);
    if (!pair)
    {
        return PairSharedPtr();
    }
    shared_ptr<string> pairName(new string(key));
    if (!pairName)
    {
        pair.reset();
        return PairSharedPtr();
    }
    shared_ptr<string> pairValue(new string(value));
    if (!pairValue)
    {
        pair.reset();
        pairName.reset();
        return PairSharedPtr();
    }
    pair->SetKey(pairName);
    pair->SetValue(pairValue);

    return pair;
}

StaticPoiSingleSearchParameters::StaticPoiSingleSearchParameters(const StaticPoiSingleSearchParameters& param)
    : SingleSearchParametersByResultStyle(param)
{
    m_poiId = param.m_poiId;
}

StaticPoiSingleSearchParameters::StaticPoiSingleSearchParameters(const SingleSearchConfiguration& config, const std::string& name, const std::string&  identify)
    : SingleSearchParametersByResultStyle(config)
{
    m_name = name;
    m_poiId = identify;
}

StaticPoiSingleSearchParameters::~StaticPoiSingleSearchParameters()
{
}

protocol::SingleSearchSourceParametersSharedPtr StaticPoiSingleSearchParameters::CreateParameters()
{
    SingleSearchParametersByResultStyle::CreateParameters();

    PairSharedPtr pair(new Pair);
    if (!pair)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    shared_ptr<string> pairName(new string("poi-id"));
    if (!pairName)
    {
        pair.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    shared_ptr<string> pairValue(new string(m_poiId));
    if (!pairValue)
    {
        pair.reset();
        pairName.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    pair->SetKey(pairName);
    pair->SetValue(pairValue);

    m_param->GetSearchFilter()->GetSearchKeyArray()->push_back(pair);

    return m_param;
}

StaticPoiSingleSearchParameters* StaticPoiSingleSearchParameters::clone() const
{
    return new StaticPoiSingleSearchParameters(*this);
}

WeatherSearchParameters::WeatherSearchParameters(const SingleSearchConfiguration& config)
    : SingleSearchParametersBySearchFilter(config)
{
}

WeatherSearchParameters::WeatherSearchParameters(const WeatherSearchParameters& param)
    : SingleSearchParametersBySearchFilter(param)
{
}

WeatherSearchParameters* WeatherSearchParameters::clone() const
{
    return new WeatherSearchParameters(*this);
}

protocol::SingleSearchSourceParametersSharedPtr WeatherSearchParameters::CreateParameters()
{
    m_param = SingleSearchSourceParametersSharedPtr(new SingleSearchSourceParameters());

    //scheme
    shared_ptr<string> scheme(new string(m_searchScheme));
    if (!scheme)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetDataScheme(scheme);

    //language
    shared_ptr<string> language(new string(m_language));
    if (!language)
    {
        m_param.reset();

    }
    m_param->SetLanguage(language);

    //iter-command
    IterCommandSharedPtr iterCommand(new IterCommand);
    shared_ptr<string> command(new string(GetIteratorCommand()));
    if (!command)
    {
        iterCommand.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    iterCommand->SetCommand(command);
    iterCommand->SetObjectCount(m_sliceSize);
    if(!m_resultState.empty())
    {
        iterCommand->SetState(ResultStateImpl::Deserialize(m_resultState));
    }
    m_param->SetIterCommand(iterCommand);

    if (!m_searchFilterData.empty())
    {
        //search-filter
        SearchFilterSharedPtr searchFilter = SearchFilterUtil::Deserialize(m_searchFilterData);
        m_param->SetSearchFilter(searchFilter);
    }
    else
    {
        //bounding box
        BoxSharedPtr box(new Box());
        if (!box)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        m_param->SetBox(box);

        PointSharedPtr leftTopPoint(new Point());
        if (!leftTopPoint)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        leftTopPoint->SetLat(m_leftTopPoint.GetLatitude());
        leftTopPoint->SetLon(m_leftTopPoint.GetLongitude());
        box->SetLeftCornerPoint(leftTopPoint);

        PointSharedPtr rightBottomPoint(new Point());
        if (!rightBottomPoint)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        rightBottomPoint->SetLat(m_rightBottomPoint.GetLatitude());
        rightBottomPoint->SetLon(m_rightBottomPoint.GetLongitude());
        box->SetRightCornerPoint(rightBottomPoint);

        SearchFilterSharedPtr searchFilter(new protocol::SearchFilter);
        if (!searchFilter)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        ResultStyleSharedPtr resultStyle(new ResultStyle);
        if (!resultStyle)
        {
            searchFilter.reset();
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        shared_ptr<string> resultStyleKey(new string("summary"));
        if (!resultStyle)
        {
            resultStyle.reset();
            searchFilter.reset();
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        resultStyle->SetKey(resultStyleKey);
        searchFilter->SetResultStyle(resultStyle);

        PairSharedPtr pair(new Pair);
        if (!pair)
        {
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        shared_ptr<string> pairName(new string("weather-type"));
        if (!pairName)
        {
            pair.reset();
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        shared_ptr<string> pairValue(new string(GetWeatherType()));
        if (!pairValue)
        {
            pair.reset();
            pairName.reset();
            m_param.reset();
            return SingleSearchSourceParametersSharedPtr();
        }
        pair->SetKey(pairName);
        pair->SetValue(pairValue);
        searchFilter->GetSearchKeyArray()->push_back(pair);

        m_param->SetSearchFilter(searchFilter);

        m_param->SetWantSearchResultId(true);
    }

    shared_ptr<WantContentTypes> wantContentTypes(new WantContentTypes);
    if (!wantContentTypes)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetWantContentTypes(wantContentTypes);

    shared_ptr<ContentType> contentType(new ContentType);
    if (!contentType)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    contentType->SetType(shared_ptr<string>(new string("weather")));
    wantContentTypes->GetContentTypeArray()->push_back(contentType);

    return m_param;
}

const char* WeatherSearchParameters::GetWeatherType() const
{
    const char* result = "";
    switch (m_type)
    {
        case WT_All:
            result = WeatherType_All;
            break;
        case WT_Forecast:
            result = WeatherType_Forecast;
        case WT_Current:
            result = WeatherType_Current;
        default:
            break;
    }
    return result;
}

AddressSearchParameters::AddressSearchParameters(const SingleSearchConfiguration& config)
    : SingleSearchParameters(config)
{
    m_param.reset();
    m_address.reset();
}

AddressSearchParameters::AddressSearchParameters(const AddressSearchParameters& param)
    : SingleSearchParameters(param)
{
    m_address = param.m_address;
}

AddressSearchParameters::~AddressSearchParameters()
{
    if  (m_address)
    {
        m_address.reset();
    }

    if (m_param)
    {
        m_param.reset();
    }
}

AddressSearchParameters* AddressSearchParameters::clone() const
{
    return new AddressSearchParameters(*this);
}

void AddressSearchParameters::SetSearchPoint(double latitude, double longitude)
{
    m_originPoint.SetLatitude(latitude);
    m_originPoint.SetLongitude(longitude);
}

void AddressSearchParameters::SetOriginPoint(double latitude, double longitude)
{
    m_searchPoint.SetLatitude(latitude);
    m_searchPoint.SetLongitude(longitude);
}

void AddressSearchParameters::SetSearchAddress(const nbsearch::Location* location)
{
    m_address.reset(new protocol::Address());
    switch (location->GetType())
    {
        case LT_Airport:
        {
            m_address->SetType(shared_ptr<std::string>(new std::string("airport")));
            m_address->SetAirport(shared_ptr<std::string>(new std::string(location->GetAirport())));
        }
            break;
        case LT_AddressFreeForm:
        {
            m_address->SetType(shared_ptr<std::string>(new std::string("freeform")));
            m_address->SetFreeform(shared_ptr<std::string>(new std::string(location->GetFreeForm())));
        }
            break;
        default:
            m_address->SetType(shared_ptr<std::string>(new std::string("street")));
            break;
    }
    m_address->SetStreetAddress(shared_ptr<std::string>(new std::string(location->GetNumber())));
    m_address->SetCrossStreet(shared_ptr<std::string>(new std::string(location->GetCrossStreet())));
    m_address->SetStreet(shared_ptr<std::string>(new std::string(location->GetStreet())));
    m_address->SetCity(shared_ptr<std::string>(new std::string(location->GetCity())));
    m_address->SetCounty(shared_ptr<std::string>(new std::string(location->GetCounty())));
    m_address->SetState(shared_ptr<std::string>(new std::string(location->GetState())));
    m_address->SetPostal(shared_ptr<std::string>(new std::string(location->GetZipCode())));
    m_address->SetCountry(shared_ptr<std::string>(new std::string(location->GetCountry())));
    m_address->SetCountryName(shared_ptr<std::string>(new std::string(location->GetCountryName())));
}

protocol::SingleSearchSourceParametersSharedPtr AddressSearchParameters::CreateParameters()
{
    m_param = SingleSearchSourceParametersSharedPtr(new SingleSearchSourceParameters());

    //scheme
    shared_ptr<string> scheme(new string(m_searchScheme));
    if (!scheme)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetDataScheme(scheme);

    //language
    shared_ptr<string> language(new string(m_language));
    if (!language)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    m_param->SetLanguage(language);

    //iter-command
    IterCommandSharedPtr iterCommand(new IterCommand);
    if (!iterCommand)
    {
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }

    shared_ptr<string> command(new string(GetIteratorCommand()));
    if (!command)
    {
        iterCommand.reset();
        m_param.reset();
        return SingleSearchSourceParametersSharedPtr();
    }
    iterCommand->SetCommand(command);
    iterCommand->SetObjectCount(m_sliceSize);
    if(!m_resultState.empty())
    {
        iterCommand->SetState(ResultStateImpl::Deserialize(m_resultState));
    }
    m_param->SetIterCommand(iterCommand);

    //search-filter
//    SearchFilterSharedPtr searchFilter(new protocol::SearchFilter);
//    if (!searchFilter)
//    {
//        m_param.reset();
//        return SingleSearchSourceParametersSharedPtr();
//    }
//    ResultStyleSharedPtr resultStyle(new ResultStyle);
//    if (!resultStyle)
//    {
//        searchFilter.reset();
//        m_param.reset();
//        return SingleSearchSourceParametersSharedPtr();
//    }
//    shared_ptr<string> resultStyleKey(new string("geocode"));
//    if (!resultStyle)
//    {
//        resultStyle.reset();
//        searchFilter.reset();
//        m_param.reset();
//        return SingleSearchSourceParametersSharedPtr();
//    }
//    resultStyle->SetKey(resultStyleKey);
//    searchFilter->SetResultStyle(resultStyle);
//    m_param->SetSearchFilter(searchFilter);

    //set address
    protocol::AddressSharedPtr address(new protocol::Address(*m_address.get()));
    m_param->SetAddress(address);

    m_param->SetWantCompactAddress(true);
    m_param->SetWantExtendedAddress(true);

    return m_param;
}


/*! @} */
