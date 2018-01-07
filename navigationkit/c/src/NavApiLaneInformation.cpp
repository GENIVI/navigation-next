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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "NavApiLaneInformation.h"
#include "NavigationImpl.h"
extern "C"
{
#include "nbnaturallaneguidanceinterface.h"
}

namespace nbnav
{

using namespace std;

string Lane::GetHighlighted() const
{
    return m_highlighted;
}

string Lane::GetNonHighlighted() const
{
    return m_nonHighlighted;
}

Lane::Lane(const void* laneItem)
{
    const NB_Lane* item = reinterpret_cast<const NB_Lane*>(laneItem);
    for (int j = 0; j < LANE_ARROW_MAX; j++)
    {
        if (item->arrows[j].character != 0 && item->arrows[j].character != NB_AC_NO_ENT)
        {
            m_nonHighlighted.push_back(static_cast<char>(item->arrows[j].character));
        }
        if (item->selectArrows[j].character != 0 && item->selectArrows[j].character != NB_AC_NO_ENT)
        {
            m_highlighted.push_back(static_cast<char>(item->selectArrows[j].character));
        }
    }
}

unsigned int LaneInformation::SelectedLanePosition() const
{
    return m_selectedLanePosition;
}

unsigned int LaneInformation::SelectedLanes() const
{
    return m_selectedLanes;
}

vector<Lane> LaneInformation::Lanes() const
{
    return m_lanes;
}

LaneInformation::LaneInformation(const NavigationImpl& impl, unsigned long manueverIndex,
                                 unsigned long laneCount, void* laneItems)
{
    NB_Lane* items = reinterpret_cast<NB_Lane*>(laneItems);
    for (unsigned long  i = 0; i < laneCount; ++i)
    {
        Lane lane(&items[i]);
        m_lanes.push_back(lane);
    }
    NB_RouteInformationGetLaneInfo(impl.GetNbNavRouteInfo(), (uint32)manueverIndex,
                                   &m_selectedLanes, &m_selectedLanePosition);
}

LaneInformation::LaneInformation()
{
}

}
