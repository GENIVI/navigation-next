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

    @file     locationprovider.h
    @date     07/29/2014
    @addtogroup locationkit

    Location provider class definistion
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
#ifndef LOCATION_PROVIDER_H
#define LOCATION_PROVIDER_H

#include "locationconfiguration.h"
#include "locationlistener.h"

namespace locationtoolkit
{
class LocationProvider
{
public:
    virtual ~LocationProvider();

    /**
    * Factory method. Creates a new {@code LocationProvider} object on first call.
    * Return link to existing instance on every subsequence call.
    * @param context Application context, must be created by client and pass here.
    * @param config {@link LocationConfiguration} object
    */
    static LocationProvider& GetInstance( const LocationConfiguration& config );

    /**
    * Stops receiving location fixes.
    * Simple Interface: use default criteria.
    * @param listener {@link LocationListener} object
    */
    virtual void StopReceivingFixes(LocationListener& listener) = 0;

    /**
    * Starts receiving location fixes.
    * Simple Interface: use default criteria.
    * @param listener { @link LocationListener} object
    */
    virtual void StartReceivingFixes(LocationListener& listener) = 0;

    /**
    * Gets one Location fix.
    * Simple Interface: use default criteria.
    * @param listener {@link LocationListener} object
    * @param fixType integer flag with the following possible values:
    * LOCATION_FIX_FAST, LOCATION_FIX_NORMAL and LOCATION_FIX_ACCURATE
    */
    virtual void GetOneFix(LocationListener& listener, Location::LocationFixType fixType) = 0;

    /**
    * Get the internal state.
    * @return A enumeration value of {@link LocationListener::LocationProviderState}
    */
    LocationListener::LocationProviderState GetState() const;

    /**
     * Cancels single location fix
     * @param listener {@link LocationListener} object that is to be detached
     */
    virtual void CancelGetLocation( LocationListener& listener ) = 0;

    /**
     * Gets API level.
     * @return A number representing the supported API level for the version of LocationKit being used.
     */
    int GetApiLevel();

protected:
    LocationProvider( /*ltkContext& context, */ );
    virtual void Initialize( /*ltkContext& context, */const LocationConfiguration& config ) = 0;

protected:
    LocationListener::LocationProviderState mState;

private:
    static LocationProvider* mInternalInstance;
};
}

#endif

/*! @} */
