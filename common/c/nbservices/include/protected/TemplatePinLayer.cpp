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
  @file        TemplatePinLayer.hpp
  @defgroup

  Description: Implementation of template function of PinLayer.

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

#include "PinLayer.h"
#include "PinImpl.h"
#include <set>

using namespace std;
using namespace nbcommon;
using namespace nbmap;

/* See header file for description */
template <class T>
void
PinLayer::AddPins(vector<PinParameters<T> >& parametersVector)
{
    set<TileKey> uniqueTileKeys;
    vector<TileKeyPtr> tileKeysToRefresh;

    typename vector<PinParameters<T> >::iterator parametersIterator = parametersVector.begin();
    typename vector<PinParameters<T> >::const_iterator parametersEnd = parametersVector.end();
    for (; parametersIterator != parametersEnd; ++parametersIterator)
    {
        // Check if the type of pin is valid.
        PinType type = parametersIterator->m_type;
        if (type == PT_INVALID)
        {
            continue;
        }

        // Generate a pin ID.
        shared_ptr<string> pinId = GeneratePinID();
        if (!pinId)
        {
            continue;
        }

        // Create a new pin.
        PinPtr pin(new PinImpl<T>(parametersIterator->m_type,
                                  pinId,
                                  parametersIterator->m_groupId,
                                  parametersIterator->m_bubble,
                                  parametersIterator->m_customPinInformation,
                                  parametersIterator->m_latitude,
                                  parametersIterator->m_longitude,
                                  parametersIterator->m_circleRadius));
        if (!pin)
        {
            continue;
        }

        // Add this pin to the pin layer.
        TileKeyPtr referenceTileKey = AddPinPtr(pin);
        if (!referenceTileKey)
        {
            continue;
        }

        // Check if this tile key has already been inserted.
        pair<set<TileKey>::iterator, bool> insertedResult = uniqueTileKeys.insert(*referenceTileKey);
        if ((insertedResult.first != uniqueTileKeys.end()) &&
            insertedResult.second)
        {
            // This tile key is unique. Add it to refresh.
            tileKeysToRefresh.push_back(referenceTileKey);
        }

        // Set the pin ID to notify this pin is added successfully.
        parametersIterator->m_pinId = pinId;
    }

    if (!(tileKeysToRefresh.empty()))
    {
        // Notify changed pin tiles to refresh.
        RefreshPinsOfTiles(tileKeysToRefresh);
    }
}

/* See header file for description */
template <class T>
shared_ptr<T>
PinLayer::GetBubble(shared_ptr<string> pinID)
{
    shared_ptr<T> bubble;
    if (pinID && !pinID->empty())
    {
        PinPtr pin = GetPin(pinID);
        if (pin)
        {
            PinImpl<T> * realPin = static_cast<PinImpl<T> *>(pin.get());
            bubble = realPin->GetBubble();
        }
    }

    return bubble;
}

/*! @} */
