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
 (C) Copyright 2012 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/
/*! @{ */

#ifndef nav_NavApiRouteSummary_h
#define nav_NavApiRouteSummary_h

#include <vector>
#include "NavApiTypes.h"

namespace nbnav
{
/**
 * Route summary infromation class.
 * The class contains all data of route summary information.
 */
class RouteSummaryInformation
{
    public:
    /**
     * Provides the driving time for the route, including traffic delays.
     * @return unsigned int travel time.
     */
    unsigned int GetTime() const;

    /**
     * Provides the driving distance in meters.
     * @return unsigned int distance.
     */
    unsigned int GetDistance() const;

    /**
     * Provides the name of the longest segment in the route.
     * @return string via.
     */
    std::string GetVia() const;

    /**
     * Provides the delay time for the route.
     * @return unsigned int delay time.
     */
    unsigned int GetDelayTime() const;

    /**
     * Set Origin place
     */
    void setOrigin(Place& place);

    /**
     * Get Origin place
     */
    Place getOrigin();

    /** Set Destination
     */
    void setDestination(Place& place);

    /** get destination.
     */
    Place getDestination();

    /** is route summary valid
     */
    bool valid();

    /**
     * Constructor.
     * For internal use only.
     */
    RouteSummaryInformation(int time, int distance, int delay, std::string via);

private:
    unsigned int m_travelTime;
    unsigned int m_distance;
    std::string  m_via;
    unsigned int m_delayTime;
    Place        m_origin;
    Place        m_destination;
};
}

#endif

/*! @} */