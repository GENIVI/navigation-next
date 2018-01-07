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
    @file     SingleSearchParameters.h
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

#ifndef __SINGLESEARCHPARAMETERS_H__
#define __SINGLESEARCHPARAMETERS_H__

#include <string>
#include "CommonTypes.h"
#include "nbcontext.h"
#include "nbsinglesearchinformation.h"
#include "NBProtocolSingleSearchSourceParameters.h"
#include "nbsuggestmatch.h"
#include "SingleSearchConfiguration.h"
#include "SingleSearchRequest.h"
#include "NBProtocolAddress.h"

namespace nbsearch
{
using namespace nbcommon;

class SingleSearchParameters
{
public:
    SingleSearchParameters();
    SingleSearchParameters(const SingleSearchConfiguration& config);
    SingleSearchParameters(const SingleSearchParameters& param);
    virtual ~SingleSearchParameters() {}
    virtual SingleSearchParameters* clone() const = 0;
    virtual void SetSearchPoint(double latitude, double longitude) = 0;
    virtual void SetOriginPoint(double latitude, double longitude) = 0;
    virtual void SetSearchFilter(const SearchFilter* searchFilter) = 0;
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters() = 0;
    virtual void SetIteratorCommand(SearchDirection direction, std::string resultState) = 0;
    void SetInputMethod(NB_SearchInputMethod inputMethod) { m_searchInputMethod = inputMethod; }

    const char* GetIteratorCommand() const;

    NB_SearchSource searchSource() const {
        return m_searchSource;
    }

    void setSearchSource(NB_SearchSource ss) {
        m_searchSource = ss;
    }

    const std::string& categoryCode() const {
        return m_categoryCode;
    }

    void setCategoryCode(const std::string& cc) {
        m_categoryCode = cc;
    }

    bool onboardOnly() const {
        return m_onboardOnly;
    }

    void setOnboardOnly(bool b) {
        m_onboardOnly = b;
    }

    NB_SearchResultStyleType resultStyleType () const {
        return m_resultStyleType;
    }

    void setResultStyleType(NB_SearchResultStyleType srst) {
        m_resultStyleType = srst;
    }

    const std::string& name() const {
        return m_name;
    }

    void setName(const std::string& name) {
        m_name = name;
    }

    const std::string& language() const {
        return m_language;
    }

    std::string languageToLower() {
        std::string src = m_language;
        std::string dest;
        dest.resize(src.size());
        std::transform(src.begin(), src.end(), dest.begin(), (int (*)(int))std::tolower);
        return dest;
    }

    void setLanguage(const std::string& language) {
        m_language = language;
    }

    LatitudeLongitude               m_searchPoint;
    LatitudeLongitude               m_originPoint;
    std::string                     m_name;
    std::string                     m_searchScheme;
    std::string                     m_categoryCode;
    uint32                          m_sliceSize;
    std::string                     m_language;
    NB_SearchSource                 m_searchSource;
    NB_SearchInputMethod            m_searchInputMethod;
    NB_POIExtendedConfiguration     m_extendedConfiguration;
    SearchDirection                 m_direction;
    std::string                     m_resultState;
    LatitudeLongitude               m_leftTopPoint;
    LatitudeLongitude               m_rightBottomPoint;
    nbsearch::WeatherType           m_type;
    bool                            m_onboardOnly;
    NB_SearchResultStyleType        m_resultStyleType;

private:
    SingleSearchParameters& operator= (SingleSearchParameters const&);
};

class SingleSearchParametersByResultStyle : public SingleSearchParameters
{
public:
    SingleSearchParametersByResultStyle(const SingleSearchConfiguration& config);
    SingleSearchParametersByResultStyle(const SingleSearchParametersByResultStyle& param);
    virtual ~SingleSearchParametersByResultStyle();
    virtual SingleSearchParametersByResultStyle* clone() const;
    virtual void SetSearchPoint(double latitude, double longitude);
    virtual void SetOriginPoint(double latitude, double longitude);
    virtual void SetSearchFilter(const SearchFilter* searchFilter) {}
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters();
    virtual void SetIteratorCommand(SearchDirection direction, std::string resultState);

    protocol::SingleSearchSourceParametersSharedPtr      m_param;

private:
    const char* GetResultStyle() const;
    const char* GetSearchFilterPairValueBySource() const;
};

class SingleSearchParametersBySearchFilter : public SingleSearchParameters
{
public:
    SingleSearchParametersBySearchFilter(const SingleSearchConfiguration& config);
    SingleSearchParametersBySearchFilter(const SingleSearchParametersBySearchFilter& param);
    virtual ~SingleSearchParametersBySearchFilter();
    virtual SingleSearchParametersBySearchFilter* clone() const;
    virtual void SetSearchPoint(double latitude, double longitude);
    virtual void SetOriginPoint(double latitude, double longitude);
    virtual void SetSearchFilter(const SearchFilter* searchFilter);
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters();
    virtual void SetIteratorCommand(SearchDirection direction, std::string resultState);

    std::string                     m_searchFilterData;
    protocol::SingleSearchSourceParametersSharedPtr      m_param;
};

class MovieSingleSearchParameters : public SingleSearchParametersByResultStyle
{
public:
    MovieSingleSearchParameters(const SingleSearchConfiguration& config, MovieShowingType showingType);
    MovieSingleSearchParameters(const SingleSearchConfiguration& config, SingleSearchResultIdType idType, std::string value, uint32 startTime);
    MovieSingleSearchParameters(const MovieSingleSearchParameters& param);
    virtual ~MovieSingleSearchParameters();
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters();
    virtual MovieSingleSearchParameters* clone() const;

    SingleSearchResultIdType  m_idType;
    std::string m_idValue;
    MovieShowingType m_showingType;
    uint32 m_startTime;

private:
    protocol::PairSharedPtr PairGenerator(const char* key, const char* value);
};

class StaticPoiSingleSearchParameters : public SingleSearchParametersByResultStyle
{
public:
    StaticPoiSingleSearchParameters(const StaticPoiSingleSearchParameters& param);
    StaticPoiSingleSearchParameters(const SingleSearchConfiguration& config, const std::string& name, const std::string&  identify);
    virtual ~StaticPoiSingleSearchParameters();
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters();
    virtual StaticPoiSingleSearchParameters* clone() const;

private:
    std::string m_poiId;
};

class WeatherSearchParameters : public SingleSearchParametersBySearchFilter
{
public:
    WeatherSearchParameters(const SingleSearchConfiguration& config);
    WeatherSearchParameters(const WeatherSearchParameters& param);
    virtual ~WeatherSearchParameters() {}
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters();
    virtual WeatherSearchParameters* clone() const;

    const char* GetWeatherType() const;
};

class AddressSearchParameters : public SingleSearchParameters
{
public:
    AddressSearchParameters(const SingleSearchConfiguration& config);
    AddressSearchParameters(const AddressSearchParameters& param);
    virtual ~AddressSearchParameters();
    virtual AddressSearchParameters* clone() const;
    virtual void SetSearchPoint(double latitude, double longitude);
    virtual void SetOriginPoint(double latitude, double longitude);
    virtual void SetSearchFilter(const SearchFilter* searchFilter) {}
    virtual protocol::SingleSearchSourceParametersSharedPtr CreateParameters();
    virtual void SetIteratorCommand(SearchDirection direction, std::string resultState) {}
    virtual void SetSearchAddress(const nbsearch::Location* location);

    protocol::SingleSearchSourceParametersSharedPtr      m_param;
private:
    protocol::AddressSharedPtr m_address;
};

}
#endif //__SINGLESEARCHPARAMETERS_H__

/*! @} */
