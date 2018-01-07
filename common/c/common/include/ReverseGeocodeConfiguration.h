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
    @file     ReverseGeocodeConfiguration.h
    @defgroup nbcommon
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
#ifndef __REVERSEGEOCODECONFIGURATION_H__
#define __REVERSEGEOCODECONFIGURATION_H__

#include <string>

namespace nbcommon
{

typedef enum
{
    RGS_Address = 0,
    RGS_Country,
    RGS_State,
    RGS_City,
    RGS_PostalCode,
    RGS_None
} ReverseGeocodeScope;

class ReverseGeocodeConfiguration
{
public:
    ReverseGeocodeConfiguration()
        : m_onboardOnly(false), m_wantExtendedAddress(false), m_wantCompactAddress(false), m_routable(false), m_scale(0.0), m_scope(RGS_None), m_language("") {}
    virtual ~ReverseGeocodeConfiguration() {}
    ReverseGeocodeConfiguration(const ReverseGeocodeConfiguration& config) {
        m_wantCompactAddress = config.m_wantCompactAddress;
        m_wantExtendedAddress = config.m_wantExtendedAddress;
        m_routable = config.m_routable;
        m_scale = config.m_scale;
        m_requiredGeoField = config.m_requiredGeoField;
        m_language = config.m_language;
        m_scope = config.m_scope;
        m_onboardOnly = config.m_onboardOnly;
    }

    bool onboardOnly() const {return m_onboardOnly;}
    void setOnboardOnly(bool b) {m_onboardOnly = b;}

    bool wantExtendedAddress() const { return m_wantExtendedAddress;}
    void setWantExtendedAddress(bool b) { m_wantExtendedAddress = b; }

    bool wantCompactAddress() const { return m_wantCompactAddress;}
    void setWantCompactAddress(bool b) { m_wantCompactAddress = b; }

    float scale() const { return m_scale; }
    void setScale(float s) { m_scale = s; }

    const std::string& requiredGeoField() const { return m_requiredGeoField; }
    void setRequiredGeoField(const std::string& s) { m_requiredGeoField = s; }

    const std::string& language() const { return m_language; }
    void setLanguage(const std::string& l) { m_language = l; }

    ReverseGeocodeScope scope() const { return m_scope; }
    void setScope(ReverseGeocodeScope s) { m_scope = s; }

    bool routable() { return m_routable; }
    void setRoutable(bool b) { m_routable = b; }

private:
    bool m_onboardOnly;
    bool m_wantExtendedAddress;
    bool m_wantCompactAddress;
    bool m_routable;
    float m_scale;
    std::string m_requiredGeoField;
    std::string m_language;
    ReverseGeocodeScope m_scope;
};
} /* nbcommon */

#endif //__REVERSEGEOCODECONFIGURATION_H__

/*! @} */
