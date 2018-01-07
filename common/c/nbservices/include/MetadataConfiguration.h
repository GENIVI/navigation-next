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
   @file         MetadataConfiguration.h
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
#ifndef _METADATACONFIGURATION_H_
#define _METADATACONFIGURATION_H_
extern "C"
{
#include "nbcontext.h"
}
#include "smartpointer.h"
namespace nbmap
{

/*! MetadataConfiguration -- Configuration used to retrieve metadata from server.

    Always use MetadataConfiguration::GetInstance() to create a MetadataConfiguration
    pointer and never create it on your own!
 */
class MetadataConfiguration
{
public:
    /*! Create a MetadataConfiguration instance for specified context.

      @return shared pointer of created configuration.
    */
    static shared_ptr<MetadataConfiguration> GetInstance(NB_Context* context);

    /*! Deprecated, Kept for backward compatibility only, do not use!! */
    MetadataConfiguration(shared_ptr<string> pLanguage, bool wantLableLayers,
                          bool wantOptionalLayers, bool wantWeatherLayer,
                          bool want2DBuildings, uint32 screenHeight,
                          uint32 screenWidth, shared_ptr<string> productClass);
    MetadataConfiguration();
    virtual ~MetadataConfiguration();
    bool operator== (const MetadataConfiguration& config) const;
    MetadataConfiguration& operator=(const MetadataConfiguration& config);
    shared_ptr<string> m_pLanguage;     /*!< Language for metadata-request, it will
                                          affect the layer names. */
    bool               m_wantLableLayers;    /*!< Flag: want Lable Layers or not */
    bool               m_wantOptionalLayers; /*!< Flag: want Optional Layers or not */
    bool               m_wantWeatherLayer;   /*!< Flag: want Weather Layer or not */
    bool               m_wantLoc;            /*!< Flag: want loc support */
    bool               m_wantDAM;            /*!< Flag: want DAM support */
    bool               m_want2DBuildings;    /*!< Flag: want 2DBuildings support */
    bool               m_wantLod;            /*!< Flag: want lod support */
    bool               m_wantPoiLayers;      /*!< Flag: want POI layer  */
    bool               m_wantSatelliteLayers;/*!< Flag: want satellite layer  */
    uint32             m_screenHeight;       /*!< Screen height in pixels. */
    uint32             m_screenWidth;        /*!< Screen width in pixels. */
    uint32             m_screenDPI;          /*!< DPI of screen. */
    bool               m_wantNewCategories;  /*!< Flag: want new categories.  */
    bool               m_wantTod;            /*!< Flag: want day*night mode.  */
    bool               m_wantThemes;         /*!< Flag: want theme materials.  */
    shared_ptr<string> m_productClass;
};

typedef shared_ptr<MetadataConfiguration> MetadataConfigurationPtr;

}
#endif /* _METADATACONFIGURATION_H_ */

/*! @} */
