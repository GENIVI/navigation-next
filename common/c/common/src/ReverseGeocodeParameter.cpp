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
    @file     ReverseGeocodeParameter.cpp
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

#include "ReverseGeocodeParameter.h"
#include "smartpointer.h"

using namespace nbcommon;
using namespace protocol;
using namespace std;

ReverseGeocodeParameter::ReverseGeocodeParameter(const ReverseGeocodeConfiguration& config)
    : m_config(config)
{
    m_param.reset();
}

ReverseGeocodeParameter::~ReverseGeocodeParameter()
{
    m_param.reset();
}

const ReverseGeocodeConfiguration& ReverseGeocodeParameter::config()
{
    return m_config;
}


void ReverseGeocodeParameter::SetPosition(double latitude, double longtitude)
{
    m_position.SetLatitude(latitude);
    m_position.SetLongitude(longtitude);
}

double ReverseGeocodeParameter::GetLatitude()
{
    return m_position.GetLatitude();
}

double ReverseGeocodeParameter::GetLongitude()
{
    return m_position.GetLongitude();
}

ReverseGeocodeParametersSharedPtr ReverseGeocodeParameter::CreateParameters()
{
    m_param.reset(new ReverseGeocodeParameters());

    //language
    if (!m_config.language().empty())
    {
        shared_ptr<string> language(new string(m_config.language()));
        if (!language)
        {
            m_param.reset();
            return m_param;
        }
        m_param->SetLanguage(language);
    }

    //scale
    m_param->SetScale(m_config.scale());

//    //scope
//    if (m_config.m_scope != RGS_None)
//    {
//        m_param->SetScope((uint32)m_config.m_scope);
//    }

//    //require-geo-field
//    if (!m_config.m_requiredGeoField.empty())
//    {
//        m_param->SetRequiredGeoField(shared_ptr<string>(new string(m_config.m_requiredGeoField)));
//    }

    //position
    PositionSharedPtr position(new Position);
    if (!position)
    {
        m_param.reset();
        return m_param;
    }
    position->SetVariant(shared_ptr<string>(new std::string("point")));
    PointSharedPtr point(new Point);
    if (!point)
    {
        position.reset();
        m_param.reset();
        return m_param;
    }
    point->SetLat(m_position.GetLatitude());
    point->SetLon(m_position.GetLongitude());
    position->SetPoint(point);

    m_param->SetPosition(position);

    if (m_config.routable())
    {
        m_param->SetRouteable(true);
    }

    if (m_config.wantExtendedAddress())
    {
        m_param->SetWantExtendedAddress(true);
    }

    if (m_config.wantCompactAddress())
    {
        m_param->SetWantCompactAddress(true);
    }

    return m_param;
}

/*! @} */
