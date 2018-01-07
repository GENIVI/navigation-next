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
    @file     NavApiSpeedLimitInformation.h
*/
/*
    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVAPISPEEDLIMITINFORMATION_H__
#define __NAVAPISPEEDLIMITINFORMATION_H__

#include "NavApiTypes.h"

namespace nbnav
{

/*! Non thread safe classes */

/*! SpeedLimitInformation

    Provides speed limit information
*/
class SpeedLimitInformation
{
public:
    static const std::string ZONE_TYPE_UNKNOWN;       /*!< Unknown zone */
    static const std::string ZONE_TYPE_SCHOOL;        /*!< School zone */
    static const std::string ZONE_TYPE_CONSTRUCTION;  /*!< Construction zone-Cone zone  */
    static const std::string ZONE_TYPE_ENHANCED;      /*!< Safety enhanced zone-Double fine zone */

    /*! Speed limit zone type, normal or school zone etc.

       @return type
    */
    std::string Type() const;

    /*! Speed limit information to be displayed, image buffer

        @return image
    */
    const Image& Information() const;

    /*! Is speed limit warning ON

        @return True if warning else not
    */
    bool IsWarning() const;

    /*! Speed limit on the current section of route, in meter per second

        @return speedlimit
    */
    double SpeedLimit() const;

    /*! User's current speed, in meters per second

        @return currentSpeed
    */
    double CurrentSpeed() const;

    /*! Constructor. For internal use only.

        @param impl
        @param data
    */
    SpeedLimitInformation(const NavigationImpl& impl, const void* data);
    SpeedLimitInformation();

private:
    std::string  m_type;
    Image        m_slImage;
    double       m_speedLimit;
    double       m_curSpeed;
    bool         m_isWarning;
};

}

#endif

/*! @} */