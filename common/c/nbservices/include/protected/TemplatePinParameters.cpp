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
  @file        TemplatePinParameters.cpp
  @defgroup

  Description: Implementation of template function of PinParameters.

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
#include "PinParameters.h"
#include "PinImpl.h"
#include <set>

using namespace std;
using namespace nbcommon;
using namespace nbmap;

// PinParameters functions ......................................................................

/* See header file for description */
template <class T>
PinParameters<T>::PinParameters(PinType type,
                                shared_ptr<T> bubble,
                                CustomPinInformationPtr customPinInformation,
                                double latitude,
                                double longitude,
                                std::string groupId,
                                uint16 circleRadius)
: Base(),
  m_type(type),
  m_pinId(),
  m_bubble(bubble),
  m_customPinInformation(customPinInformation),
  m_latitude(latitude),
  m_longitude(longitude),
  m_groupId(shared_ptr<std::string>(new std::string(groupId.c_str()))),
  m_circleRadius(circleRadius)
{
    // Nothing to do here.
}

/* See header file for description */
template <class T>
PinParameters<T>::~PinParameters()
{
    // Nothing to do here.
}

/* See header file for description */
template <class T>
PinParameters<T>::PinParameters(const PinParameters& parameters)
: m_type(parameters.m_type),
  m_pinId(parameters.m_pinId),
  m_groupId(parameters.m_groupId),
  m_bubble(parameters.m_bubble),
  m_customPinInformation(parameters.m_customPinInformation),
  m_latitude(parameters.m_latitude),
  m_longitude(parameters.m_longitude),
  m_circleRadius(parameters.m_circleRadius)
{
    // Nothing to do here.
}

/* See header file for description */
template <class T>
PinParameters<T>&
PinParameters<T>::operator=(const PinParameters& parameters)
{
    if ((&parameters) != this)
    {
        this->m_type                 = parameters.m_type;
        this->m_pinId                = parameters.m_pinId;
        this->m_groupId              = parameters.m_groupId;
        this->m_bubble               = parameters.m_bubble;
        this->m_customPinInformation = parameters.m_customPinInformation;
        this->m_latitude             = parameters.m_latitude;
        this->m_longitude            = parameters.m_longitude;
        this->m_circleRadius         = parameters.m_circleRadius;
    }

    return *this;
}
/*! @} */
