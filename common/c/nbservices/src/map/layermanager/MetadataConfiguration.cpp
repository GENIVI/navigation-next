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
   @file         MetadtaConfigurationImpl.cpp
   @defgroup     nbmap
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#include "MetadataConfiguration.h"
#include "UnifiedService.h"
#include "UnifiedMetadataProcessor.h"
#include "contextbasedsingleton.h"
#include "paldisplay.h"
#include "StringUtility.h"
#include "nbmacros.h"

using namespace nbmap;

static const string DEFAULT_LANGUAGE("en-US");
static const string DEFAULT_PRODUCT_CLASS("default");

/* See description in header file. */
MetadataConfiguration::MetadataConfiguration()
        : m_pLanguage(CCC_NEW string(DEFAULT_LANGUAGE)),
          m_wantLableLayers(false),
          m_wantOptionalLayers(true),
          m_wantWeatherLayer(false),
          m_wantLoc(true),
          m_wantDAM(true),
          m_want2DBuildings(true),
          m_wantLod(true),
          m_wantPoiLayers(false),
          m_wantSatelliteLayers(false),
          m_screenHeight(0),
          m_screenWidth(0),
          m_screenDPI(0),
          m_wantNewCategories(true),
          m_wantTod(true),
          m_wantThemes(true),
          m_productClass(CCC_NEW string(DEFAULT_PRODUCT_CLASS))

{
    (void)PAL_DisplayGetInfo(PD_Main, &m_screenWidth, &m_screenHeight, &m_screenDPI);
}

/* See description in header file. */
MetadataConfiguration::~MetadataConfiguration()
{
}

/* See description in header file. */
bool MetadataConfiguration::operator ==(const MetadataConfiguration& config) const
{
    return !nbcommon::StringUtility::CompareStdString(m_pLanguage, config.m_pLanguage) &&
            !nbcommon::StringUtility::CompareStdString(m_productClass, config.m_productClass) &&
            m_wantLableLayers    == config.m_wantLableLayers    &&
            m_wantOptionalLayers == config.m_wantOptionalLayers &&
            m_wantWeatherLayer   == config.m_wantWeatherLayer   &&
            m_wantLoc            == config.m_wantLoc            &&
            m_wantDAM            == config.m_wantDAM            &&
            m_want2DBuildings    == config.m_want2DBuildings    &&
            m_wantLod            == config.m_wantLod            &&
            m_wantPoiLayers      == config.m_wantPoiLayers      &&
            m_wantSatelliteLayers== config.m_wantSatelliteLayers&&
            m_screenHeight       == config.m_screenHeight       &&
            m_screenWidth        == config.m_screenWidth        &&
            m_screenDPI          == config.m_screenDPI          &&
            m_wantNewCategories  == config.m_wantNewCategories  &&
            m_wantTod            == config.m_wantTod            &&
            m_wantThemes         == config.m_wantThemes;
}

MetadataConfiguration::MetadataConfiguration(shared_ptr <string> pLanguage,
                                             bool wantLableLayers,
                                             bool wantOptionalLayers,
                                             bool wantWeatherLayer,
                                             bool want2DBuildings,
                                             uint32 screenHeight,
                                             uint32 screenWidth,
                                             shared_ptr<string> productClass)
        : m_pLanguage(pLanguage),
          m_wantLableLayers(wantLableLayers),
          m_wantOptionalLayers(wantOptionalLayers),
          m_wantWeatherLayer(wantWeatherLayer),
          m_want2DBuildings(want2DBuildings),
          m_screenHeight(screenHeight),
          m_screenWidth(screenWidth),
          m_wantNewCategories(true),
          m_wantTod(true),
          m_wantThemes(true),
          m_productClass(productClass)
{
}

/* See description in header file. */
MetadataConfiguration& MetadataConfiguration::operator =(const MetadataConfiguration& config)
{
    if (this != &config)
    {
        if (config.m_pLanguage)
        {
            m_pLanguage.reset(CCC_NEW string(*(config.m_pLanguage)));
        }
        else
        {
            m_pLanguage.reset();
        }

        if (config.m_productClass && !config.m_productClass->empty())
        {
            m_productClass.reset(CCC_NEW string(*(config.m_productClass)));
        }
        else
        {
            m_productClass.reset();
        }

        m_wantLableLayers    = config.m_wantLableLayers;
        m_wantOptionalLayers = config.m_wantOptionalLayers;
        m_wantWeatherLayer   = config.m_wantWeatherLayer;
        m_wantLoc            = config.m_wantLoc;
        m_wantDAM            = config.m_wantDAM;
        m_want2DBuildings    = config.m_want2DBuildings;
        m_wantLod            = config.m_wantLod;
        m_screenHeight       = config.m_screenHeight;
        m_screenWidth        = config.m_screenWidth;
        m_screenDPI          = config.m_screenDPI;
        m_wantPoiLayers      = config.m_wantPoiLayers;
        m_wantSatelliteLayers= config.m_wantSatelliteLayers;
        m_wantNewCategories  = config.m_wantNewCategories;
        m_wantTod            = config.m_wantTod;
        m_wantThemes         = config.m_wantThemes;
    }
    return *this;
}


/*! @} */
