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
    @file       TrafficBubbleProvider.mm

    See header file for description.
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

#include "TrafficBubbleProvider.h"
#include "PinLayer.h"
#include "datastream.h"
#import "BubbleInterface.h"
#import "PinCushion.h"

/*! @{ */

using namespace nbcommon;
using namespace nbmap;
using namespace std;

// Public functions .............................................................................

/* See header file for description */
TrafficBubbleProvider::TrafficBubbleProvider()
{
    // Nothing to do here.
}

/* See header file for description */
TrafficBubbleProvider::~TrafficBubbleProvider()
{
    // Nothing to do here.
}

/* See header file for description */
TrafficBubbleProviderPtr
TrafficBubbleProvider::GetTrafficBubbleProvider()
{
    // @todo: should we make this become singleton?
    return TrafficBubbleProviderPtr(new TrafficBubbleProvider());
}

/* See header file for description */
NB_Error
TrafficBubbleProvider::AddTrafficIncidentPins(PinLayerPtr pinLayer,
                                              NB_SingleSearchInformation* searchInformation,
                                              vector<shared_ptr<string> >& pinIdsToReturn,
                                              shared_ptr<PinCushion> pinCushion)
{
    // Check if the parameters are valid.
    if ((!pinLayer) || (!searchInformation))
    {
        return NE_INVAL;
    }

    // Get the count of search result.
    uint32 searchResultCount = 0;
    NB_Error error = NB_SingleSearchInformationGetResultCount(searchInformation,
                                                              &searchResultCount);
    if (error != NE_OK)
    {
        return error;
    }

    if (searchResultCount <= 0)
    {
        // There is no traffic incidents.
        return NE_OK;
    }

    // Set pin parameters by the search information.
    vector<PinParameters<void> > parametersVector;
    parametersVector.reserve(static_cast<size_t>(searchResultCount));
    for (int i = 0; i < searchResultCount; ++i)
    {
        // Get the coordinate of this traffic incident.
        NB_Place place;
        nsl_memset(&place, 0, sizeof(place));
        place.location.latitude = INVALID_LATLON;
        place.location.longitude = INVALID_LATLON;
        NB_ExtendedPlace* extendedPlace = NULL;
        double distance = 0.0;
        error = NB_SingleSearchInformationGetPlace(searchInformation,
                                                   i,
                                                   &place,
                                                   &distance,
                                                   &extendedPlace);
        if (error != NE_OK)
        {
            return error;
        }
        double pinLatitude = place.location.latitude;
        double pinLongitude = place.location.longitude;
        if ((pinLatitude == INVALID_LATLON) ||
            (pinLongitude == INVALID_LATLON))
        {
            continue;
        }

        // Get the severity of this traffic incident.
        NB_TrafficIncident trafficIncident = {0};
        error =  NB_SingleSearchInformationGetTrafficIncident(searchInformation,
                                                              i,
                                                              &trafficIncident,
                                                              NULL);
        if (error != NE_OK)
        {
            return error;
        }

        // Convert the severity to pin type.
        PinType pinType = PT_MINOR_INCIDENT;
        switch (trafficIncident.severity)
        {
            case NB_TrafficIncident_Severe:
            {
                pinType = PT_SEVERE_INCIDENT;
                break;
            }
            case NB_TrafficIncident_Major:
            {
                pinType = PT_MAJOR_INCIDENT;
                break;
            }
            case NB_TrafficIncident_Minor:
            case NB_TrafficIncident_LowImpact:
            {
                pinType = PT_MINOR_INCIDENT;
                break;
            }
            default:
            {
                break;
            }
        }

        shared_ptr<nbmap::BubbleInterface> bubble;
        if(pinCushion)
        {
            shared_ptr<nbmap::PinBubbleResolver> resolver = pinCushion->GetPinBubbleResolver();
            if(resolver)
            {
                bubble = resolver->GetTrafficIncidentBubble(trafficIncident.type, trafficIncident.severity, trafficIncident.entry_time, trafficIncident.start_time, trafficIncident.entry_time, std::string(trafficIncident.description), std::string(trafficIncident.road), pinLatitude, pinLongitude);
            }
        }
        parametersVector.push_back(PinParameters<void>(pinType,
                                                       bubble,
                                                       CustomPinInformationPtr(),
                                                       pinLatitude,
                                                       pinLongitude));
    }

    if (!(parametersVector.empty()))
    {
        // Add pins.
        pinLayer->AddPins(parametersVector);

        // Get the pin IDs to return.
        vector<PinParameters<void> >::const_iterator parametersIterator = parametersVector.begin();
        vector<PinParameters<void> >::const_iterator parametersEnd = parametersVector.end();
        for (; parametersIterator != parametersEnd; ++parametersIterator)
        {
            shared_ptr<string> pinId = parametersIterator->m_pinId;
            if (pinId && (!(pinId->empty())))
            {
                // Add the pin ID to return.
                pinIdsToReturn.push_back(pinId);
            }
        }
    }

    return NE_OK;
}


// Private functions ............................................................................

/*! Get the bubble view

    @return A pointer to the bubble view if success, NULL otherwise.
*/
void*
TrafficBubbleProvider::GetBubbleView(NB_TrafficIncident* trafficIncident)
{
    return NULL;
}

/*! Destroy the bubble view

    This function is used to destroy the bubble view for shared_ptr.

    @return None
*/
void
TrafficBubbleProvider::DestroyBubbleView(void* bubbleView)
{

}

/*! @} */
