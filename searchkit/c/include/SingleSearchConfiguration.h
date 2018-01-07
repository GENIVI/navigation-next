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
    @file     SingleSearchConfiguration.h
    @defgroup nbsearch
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

#ifndef __SINGLESEARCHCONFIGURATION_H__
#define __SINGLESEARCHCONFIGURATION_H__

#include <string>
#include "CommonTypes.h"
#include "SingleSearchInformation.h"

namespace nbsearch
{

typedef enum
{
    SD_Start = 0, /*!< Search start from the start. */
    SD_Next,      /*!< Search for the next set of results. */
    SD_Prev       /*!< Search for the previous set of results. */
} SearchDirection;

typedef enum
{
    WT_None = 0,
    WT_Current,     /*!< Return only current weather conditions. */
    WT_Forecast,    /*!< Return only forecast weather conditions. */
    WT_All          /*!< Return both current and forecast weather conditions. */
} WeatherType;

/*!
 * Single search configuration --configuration used to set up
 * search query.
 *
 */
class SingleSearchConfiguration
{

public:

    /*!
     * Create a new configuration with default settings.
     */
    SingleSearchConfiguration(const std::string& language = "en-US");

    SingleSearchConfiguration(const char* scheme,
                              const char* language,
                              int sliceSize,
                              NB_POIExtendedConfiguration extendedConfiguration,
                              NB_SearchInputMethod inputMethod,
                              NB_SearchResultStyleType resultStyleType);

    SingleSearchConfiguration(SingleSearchConfiguration const&);
    SingleSearchConfiguration& operator=(SingleSearchConfiguration const&);

    void setSearchSource(NB_SearchSource searchSource) {
        m_searchSource = searchSource;
    }

    NB_SearchSource getSearchSource() const {
        return m_searchSource;
    }

    /*!< Maximum number of results to return with each request */
    int m_sliceSize;
    /*!< Specifies the extended configuration for the request */
    NB_POIExtendedConfiguration m_extendedConfiguration;
    NB_SearchInputMethod m_inputMethod;
    NB_SearchResultStyleType    m_resultStyleType;
    NB_SearchSource m_searchSource;
    std::string m_scheme;
    std::string m_language;
    SearchDirection m_direction;
    WeatherType m_weatherType;
    bool m_onboardOnly;

private:
    void Initialize(const std::string& language);
};

}

#endif //__SINGLESEARCHCONFIGURATION_H__

/*! @} */
