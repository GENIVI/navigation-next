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

    @file     locationlistener.h
    @date     07/29/2014
    @addtogroup locationkit

    Location listener definition
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#ifndef LOCATION_LISTENER_H
#define LOCATION_LISTENER_H
#include "location.h"

namespace locationtoolkit
{
class LocationListener
{
public:
    enum LocationProviderState
    {
        LPS_Undefined,               /*!< Not initialized */
        LPS_InitializedPending,      /*!< Initializing */
        LPS_DestoryPending,          /*!< Destroy pending */
        LPS_Initialzed,              /*!< Stop Initializing */
        LPS_TrackingNetwork,         /*!< Network tracking location fix */
        LPS_TrackingGPS,             /*!< GPS tracking location fix */
        LPS_Suspended,               /*!< Location fix was suspended */
        LPS_Resume,                  /*!< Location fix was resumed */
        LPS_TemporarilyUnavailable,  /*!< Location fix temporarily unavailable */
        LPS_OutOfService             /*!< Location fix out of service */
    };

    LocationListener() {}
    virtual ~LocationListener() {}

    /**
	 * Called by the LocationProvider to which this listener is registered.
	 * @param  location the location to which the event relates, i.e. the new position
	 */
    virtual void LocationUpdated( const Location& location ) = 0;

    /**
    * Called by the LocationProvider to which this listener is registered if the state of the LocationProvider has changed.
    * @param newState The new state of the LocationProvider. This value is one of the constants for the state defined in the LocationProvider class.
    */
    virtual void ProviderStateChanged( LocationProviderState state ) = 0;

    /**
    * Called by the LocationProvider to notify listener that an error has occurred.
    * @param errorCode Error code from locationProvider.It is possible that system can recover from temporary error.
    */
    virtual void OnLocationError(int errorCode)  = 0;
};
}

#endif
/*! @} */
