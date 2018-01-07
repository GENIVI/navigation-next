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
@file     Pin.h
@defgroup nbmap

*/
/*
(C) Copyright 2011 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary
to TeleCommunication Systems, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of TeleCommunication Systems, is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __PIN__
#define __PIN__

#include "paltypes.h"
#include "nbexp.h"
#include "smartpointer.h"
#include "datastream.h"
#include "PinLayer.h"
#include <string>

using namespace std;

namespace nbmap
{

/*! Provides a definition of a Pin within mapview. Allows a client of MapView to get Pin
    properties and interact with the pin.
*/

class Pin
{
public:

    Pin() {}

    virtual ~Pin() {}

    /*! Get type

        @return Type of pin
    */
    virtual PinType GetType() const = 0;

    /*! Get latitude

        @return Latitude of pin
    */
    virtual double GetLatitude() const = 0;

    /*! Get longitude

        @return Longitude of pin
    */
    virtual double GetLongitude() const = 0;

    /*! Return radius of pin.

        For pins without radius, it returns 0.

        @return radius.
    */
    virtual uint16 GetRadius() const = 0;

    /*! Get the pin ID

        Returns the pinID associated with the pin. PinID is unique for all pins across
        all pin layers. This pinID will be passed in the callback function when a pin is
        clicked.

        @return A string corresponding to PinID.
    */
    virtual shared_ptr<std::string> GetPinID() const = 0;

    /*! Get the information to create a custom pin

        @return Information to create a custom pin
    */
    virtual CustomPinInformationPtr GetCustomPinInformation() const = 0;
};

typedef shared_ptr<Pin>   PinPtr;

}

#endif

/*! @} */
