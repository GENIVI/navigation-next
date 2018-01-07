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
 @file         NKUILocation.h
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#ifndef __NAVIGATIONUIKIT__LOCATIONIMPL_H__
#define __NAVIGATIONUIKIT__LOCATIONIMPL_H__

#include "NavApiTypes.h"

namespace nkui
{
/**
 * Location implement interface of navkit loaction.
 */
class NKUILocation : public nbnav::Location
{
public:
    NKUILocation(double latitude, double longitude, double heading,
                 unsigned int gpsTime, double horizontalVelocity,
                 double horizontalUncertainAlongAxis, bool isGpsFix,
                 int valid)
        :m_longitude(longitude), m_latitude(latitude), m_heading(heading),
         m_gpsTime(gpsTime), m_horizontalVelocity(horizontalVelocity),
         m_horizontalUncertainAlongAxis(horizontalUncertainAlongAxis),
         m_isGpsFix(isGpsFix), m_valid(valid)
    {
    };

    /* please reference Location. */
    virtual double Altitude() const { return 0;};
    virtual unsigned int GpsTime() const { return m_gpsTime;};
    virtual double Heading() const { return m_heading;};
    virtual double HorizontalUncertaintyAlongAxis() const { return m_horizontalUncertainAlongAxis;};
    virtual double HorizontalUncertaintyAlongPerpendicular() const { return 0;};
    virtual double HorizontalUncertaintyAngleOfAxis() const { return 0;};
    virtual double HorizontalVelocity() const { return m_horizontalVelocity;};
    virtual double Latitude() const { return m_latitude;};
    virtual double Longitude() const { return m_longitude;};
    virtual int NumberOfSatellites() const { return 0;};
    virtual int UtcOffset() const { return 0;};
    virtual unsigned int Valid() const { return m_valid;};
    virtual double VerticalUncertainty() const { return 0;};
    virtual double VerticalVelocity() const { return 0;};
    virtual bool IsGpsFix() const { return m_isGpsFix;};

    double m_longitude;
    double m_latitude;
    double m_heading;
    unsigned int m_gpsTime;
    double m_horizontalVelocity;
    double m_horizontalUncertainAlongAxis;
    bool   m_isGpsFix;
    int    m_valid;

};
}

#endif

/*! @} */
