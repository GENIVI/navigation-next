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
   @file        TrafficBubbleProvider.h
   @defgroup    nbui

   Description: TrafficBubbleProvider accepts an NB_TrafficIncident and
                returns a TrafficBubble for this incident. This TrafficBubble
                will be associated with a traffic pin, and will pop up when
                the pin is clicked.

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

#ifndef _TRAFFICBUBBLEPROVIDER_H_
#define _TRAFFICBUBBLEPROVIDER_H_

extern "C"
{
#include "nbplace.h"
#include "nbsinglesearchinformation.h"
}

#include "smartpointer.h"
#include <string>
#include <vector>

namespace nbmap
{

class PinLayer;
class PinCushion;

class TrafficBubbleProvider
{
public:
    /*! TrafficBubbleProvider constructor */
    TrafficBubbleProvider();

    /*! TrafficBubbleProvider destructor */
    virtual ~TrafficBubbleProvider();

    /*! Get the instance of traffic bubble provider

        @return A shared pointer to the instance of traffic bubble provider
    */
    static shared_ptr<TrafficBubbleProvider>
    GetTrafficBubbleProvider();

    /*! Add the pins of traffic incidents to the specified pin layer

        TRICKY: The bubble getter is platform-dependent and implemented in nbui. If pins are
                added in nbservices, we need to fix some classes to template (TrafficManager,
                TrafficIncidentLayer and so on). I think it is simple to make nbui to add
                pins of traffic incidents here. Because nbui knows the T exactly.

        @return NE_OK if success
    */
    NB_Error
    AddTrafficIncidentPins(shared_ptr<PinLayer> pinLayer,                         /*!< The layer to add pins */
                           NB_SingleSearchInformation* searchInformation,         /*!< The single search result of traffic incidents */
                           std::vector<shared_ptr<std::string> >& pinIdsToReturn, /*!< On return the pin IDs of added pins */
                           shared_ptr<PinCushion> pinCushion = shared_ptr<PinCushion>() /*!< pin cushion for traffic incident bubble */
                           );

private:

    /* See source file for description */

    static void* GetBubbleView(NB_TrafficIncident* trafficIncident);
    static void DestroyBubbleView(void* bubbleView);
};

typedef shared_ptr<TrafficBubbleProvider> TrafficBubbleProviderPtr;

}

#endif /* _TRAFFICBUBBLEPROVIDER_H_ */

/*! @} */
