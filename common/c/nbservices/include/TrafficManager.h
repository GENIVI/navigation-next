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
  @file        TrafficManager.h
  @defgroup    nbmap
  Description: Interface class for TrafficManager, which manage traffic layers.
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

#ifndef _TRAFFICMANAGER_H_
#define _TRAFFICMANAGER_H_

extern "C"
{
#include "nbplace.h"
}

#include "nbexp.h"
#include "smartpointer.h"

namespace nbmap
{

class TrafficManagerListener;

/*! TrafficManager to manage layers contains traffics */
class TrafficManager
{
public:
    /*! Default constructor. */
    TrafficManager(){};

    /*! Default destructor. */
    virtual ~TrafficManager(){};

    /*! Enable traffic

        @return None
    */
    virtual void
    EnableTraffic() = 0;

    /*! Disable traffic

        @return None
    */
    virtual void
    DisableTraffic() = 0;

    /*! Check if traffic is enabled

        @return true if traffic is enabled, false otherwise.
    */
    virtual bool
    IsTrafficEnabled() = 0;

    /*! Get the traffic incidents

        This function should be called to get the current traffic incidents. User should also
        register a listener to monitor the traffic incidents updated.

        @todo: This function returns all traffic incidents so far. I think it should return the
               visible traffic incidents on map.

        @return A vector of the traffic incidents
    */
    virtual std::vector<NB_TrafficIncident>
    GetTrafficIncidents() = 0;

    /*! Register a listener

        @return None
    */
    virtual void
    RegisterListener(TrafficManagerListener* listener       /*!< A listener to register */
                     ) = 0;

    /*! Unregister a listener

        User should always unregister the listener when the object of listener is destroyed.

        @return None
    */
    virtual void
    UnregisterListener(TrafficManagerListener* listener     /*!< A listener to unregister */
                       ) = 0;
};

typedef shared_ptr<TrafficManager>   TrafficManagerPtr;

};


/*! @} */
#endif /* _TRAFFICMANAGER_H_ */
