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
    @file     SingleSearchConfiguration.cpp
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

#include "SingleSearchConfiguration.h"

using namespace nbsearch;
using namespace std;

static const int SLICE_RESULTS_DEFAULT = 10;
static const std::string SCHEME_DEFAULT = "tcs-single-search-2";
static const std::string LANGUAGE_DEFAULT = "en-US";
static const NB_SearchSource SEARCHSOURCE_DEFAULT = NB_SS_None;
static const NB_SearchResultStyleType SEARCHRESULT_STYLETPYE_DEFAULT = NB_SRST_None;
static const NB_POIExtendedConfiguration SEARCHEXT_DEFAULT = NB_EXT_None;
static const NB_SearchInputMethod SEARCHINPUTMETHOD_DEFAULT = NB_SIM_Undefined;

SingleSearchConfiguration::SingleSearchConfiguration(const std::string& lang)
{
    Initialize(lang);
}

SingleSearchConfiguration::SingleSearchConfiguration(const char* scheme,
                                                     const char* language,
                                                     int sliceSize,
                                                     NB_POIExtendedConfiguration extendedConfiguration,
                                                     NB_SearchInputMethod inputMethod,
                                                     NB_SearchResultStyleType resultStyleType)
{
    Initialize(language);
    m_scheme                = string(scheme);
    m_sliceSize             = sliceSize;
    m_extendedConfiguration = extendedConfiguration;
    m_inputMethod           = inputMethod;
    m_resultStyleType       = resultStyleType;
}

SingleSearchConfiguration::SingleSearchConfiguration(SingleSearchConfiguration const& copy)
{
    *this = copy;
}

SingleSearchConfiguration& SingleSearchConfiguration::operator=(SingleSearchConfiguration const& copy)
{
    m_sliceSize             = copy.m_sliceSize;
    m_extendedConfiguration = copy.m_extendedConfiguration;
    m_inputMethod           = copy.m_inputMethod;
    m_resultStyleType       = copy.m_resultStyleType;
    m_searchSource          = copy.m_searchSource;
    m_scheme                = copy.m_scheme;
    m_language              = copy.m_language;
    m_onboardOnly           = copy.m_onboardOnly;
    return *this;
}

void SingleSearchConfiguration::Initialize(const std::string& lang)
{
    m_scheme                = string(SCHEME_DEFAULT);
    m_language              = lang;
    m_sliceSize             = SLICE_RESULTS_DEFAULT;
    m_searchSource          = SEARCHSOURCE_DEFAULT;
    m_extendedConfiguration = SEARCHEXT_DEFAULT;
    m_inputMethod           = SEARCHINPUTMETHOD_DEFAULT;
    m_resultStyleType       = SEARCHRESULT_STYLETPYE_DEFAULT;
    m_direction             = SD_Start;
    m_weatherType           = WT_None;
    m_onboardOnly            = false;
}

/*! @} */
