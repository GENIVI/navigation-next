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

    @file       PinImpl.h

    Class PinImpl inherits from Pin interface.

    @todo: Add description of this class.

*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef PINIMPL_H
#define PINIMPL_H

/*!
    @addtogroup nbmap
    @{
*/

#include "base.h"
#include "datastream.h"
#include "Pin.h"

namespace nbmap
{

// Types ........................................................................................

/* TRICKY: Do not add any non-const functions (SET) in this class, because the
           shared_ptr<Pin> may be used in the multiple threads.
*/
template <class T>
class PinImpl : public Base,
                public Pin
{
public:  // this is public
	     //Just in case this is a comment about this being public

    // Public functions .........................................................................

    /*! PinImpl constructor */
    PinImpl(PinType type,                                   /*!< Type of pin */
            shared_ptr<std::string> pinID,                  /*!< pinID associated with the pin */
            shared_ptr<std::string> groupID,                /*!< Pin group id */
            shared_ptr<T> bubble,                           /*!< Pin bubble */
            CustomPinInformationPtr customPinInformation,   /*!< Information to create a custom pin. It is
                                                                 used if the pin type is PT_CUSTOM. */
            double latitude,                                /*!< Latitude */
            double longitude,                               /*!< Longitude */
            uint16 circleRadius                             /*!< Radius, if any */
            );

    /*! PinImpl destructor */
    virtual ~PinImpl();

    /* See description in Pin.h */
    virtual PinType GetType() const;
    virtual double GetLatitude() const;
    virtual void SetLatitude(double lat);
    virtual double GetLongitude() const;
    virtual void SetLongitude(double lon);
    virtual uint16 GetRadius() const;
    virtual shared_ptr<std::string> GetPinID() const;
    virtual CustomPinInformationPtr GetCustomPinInformation() const;

    /*! Get bubble

        TRICKY: I think this function could be called in multiple threads if retaining
                the same shared_ptr<Pin>. Reference the section "Thread Safety"
                in the document
                "http://www.boost.org/doc/libs/1_53_0/libs/smart_ptr/shared_ptr.htm".
                Currently this function is used to get the bubble from the
                shared_ptr<Pin> and display the bubble in the UI thread.

        @return Pin bubble
    */
    shared_ptr<T>
    GetBubble() const;


private:

    // Copy constructor and assignment operator are not supported.
    PinImpl(const PinImpl& pin);
    PinImpl& operator=(const PinImpl& pin);

    PinType m_type;                                 /*!< Type of pin */
    shared_ptr<std::string> m_pinID;                /*!< pinID associated with the pin */
    shared_ptr<std::string> m_groupID;
    shared_ptr<T> m_bubble;                         /*!< Pin bubble */
    CustomPinInformationPtr m_customPinInformation; /*!< Information to create a custom pin. It is
                                                         used if the pin type is PT_CUSTOM. */
    double m_latitude;                              /*!< Latitude */
    double m_longitude;                             /*!< Longitude */
    uint16 m_circleRadius;                          /*!< Radius */
};

}

#endif  // PINIMPL_H

/*! @} */
