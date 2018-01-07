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
@file     PinCushion.h
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

#ifndef __PIN_CUSHION__
#define __PIN_CUSHION__

extern "C"
{
    #include "nberror.h"
}

#include "smartpointer.h"
#include "PinBubbleResolver.h"

namespace nbmap
{
/*! Provides a definition of a PinCushion within mapview. Allows a client of mapview to get displayable name
and toggle the display of the PinCushion.
*/
class PinCushion
{
public:

    PinCushion() {}

    virtual ~PinCushion() {}

    /*! Get a pin bubbleResolver

       @return current pin bubble resolver.
    */
    virtual shared_ptr<PinBubbleResolver> GetPinBubbleResolver() = 0;

    /*! Set a custom pin bubbleResolver

       @param bubbleResolver, a factory to create a custom pin bubbles
       @return NB_Error to identify if the bubbleResolver was set successfully
    */
    virtual NB_Error SetCustomPinBubbleResolver(shared_ptr<PinBubbleResolver> bubbleResolver) = 0;

    /*! Drop a pin on the map

       When a pin is dropped on the map, Mapview will call this function
       with the latitude and longitude as input parameters. PinCushion will create the
       pin object and return it.

       @param latitude, latitute for the pin
       @param lonitude, longitude for the pin
       @return PinID of the added Pin.
    */
    virtual shared_ptr<std::string>
    DropPin(double latitude, double longitude) = 0;


    /*! Remove a pin

       Remove a pin identified by a pointer to a pin object.

       @param pinID of the pin that needs to be removed.
       @return NB_Error to identify if the pin was removed successfully
    */
    virtual NB_Error
    RemovePin(shared_ptr<std::string> pinID) = 0;
};

typedef shared_ptr<PinCushion>   PinCushionPtr;


}
#endif

/*! @} */
