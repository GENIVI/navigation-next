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
  @file        TemplatePinImpl.hpp
  @defgroup

  Description:

*/
/*
  (C) Copyright 2012 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */
#include "PinImpl.h"

using namespace nbcommon;
using namespace nbmap;

/* See header file for description */
template <class T>
PinImpl<T>::PinImpl(PinType type,
                    shared_ptr<std::string> pinID,
                    shared_ptr<std::string> groupID,
                    shared_ptr<T> bubble,
                    CustomPinInformationPtr customPinInformation,
                    double latitude,
                    double longitude,
                    uint16 circleRadius)
        : Base(),
          Pin(),
          m_type(type),
          m_pinID(pinID),
          m_groupID(groupID),
          m_bubble(bubble),
          m_customPinInformation(customPinInformation),
          m_latitude(latitude),
          m_longitude(longitude),
          m_circleRadius(circleRadius)
{
    // Nothing to do here.
}

/* See header file for description */
template <class T>
PinImpl<T>::~PinImpl()
{
    // Nothing to do here.
}

/* See description in Pin.h */
template <class T>
PinType
PinImpl<T>::GetType() const
{
    return m_type;
}

/* See description in Pin.h */
template <class T>
double
PinImpl<T>::GetLatitude() const
{
    return m_latitude;
}

/* See description in Pin.h */
template <class T>
double
PinImpl<T>::GetLongitude() const
{
    return m_longitude;
}

/* See description in Pin.h */
template <class T>
void
PinImpl<T>::SetLatitude(double lat)
{
    m_latitude = lat;
}

/* See description in Pin.h */
template <class T>
void
PinImpl<T>::SetLongitude(double lon)
{
    m_longitude = lon;
}

/* See description in Pin.h */
template <class T>
uint16
PinImpl<T>::GetRadius() const
{
    return m_circleRadius;
}

/* See description in Pin.h */
template <class T>
shared_ptr<std::string>
PinImpl<T>::GetPinID() const
{
    return m_pinID;
}

/* See description in Pin.h */
template <class T>
CustomPinInformationPtr
PinImpl<T>::GetCustomPinInformation() const
{
    return m_customPinInformation;
}

/* See header file for description */
template <class T>
shared_ptr<T>
PinImpl<T>::GetBubble() const
{
    return m_bubble;
}

/*! @} */
