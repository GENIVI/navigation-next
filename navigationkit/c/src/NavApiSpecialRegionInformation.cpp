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
    @file     NavApiSpecialRegionInformation.cpp
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#include "NavApiSpecialRegionInformation.h"
#include "nbspecialregionprocessor.h"

using namespace std;
using namespace nbnav;

SpecialRegionInformation::SpecialRegionInformation()
    : m_image(NULL, 0, PNG),
      m_startManeuverIndex(0),
      m_startManeuverOffset(0),
      m_endManeuverIndex(0),
      m_endManeuverOffset(0),
      m_nextOccurStartTime(0),
      m_nextOccurEndTime(0)
{
}

SpecialRegionInformation::SpecialRegionInformation(const void* data) :
m_image(NULL, 0, PNG)
{
    const NB_SpecialRegionStateData* specialRegionData = static_cast<const NB_SpecialRegionStateData*>(data);
    if (specialRegionData)
    {
        if (specialRegionData->regionType)
        {
            m_regionType.append(specialRegionData->regionType);
        }
        if (specialRegionData->regionDescription)
        {
            m_regionDescription.append(specialRegionData->regionDescription);
        }
        m_startManeuverIndex = specialRegionData->startManeuverIndex;
        m_startManeuverOffset = specialRegionData->startManeuverOffset;
        m_endManeuverIndex = specialRegionData->endManeuverIndex;
        m_endManeuverOffset = specialRegionData->endManeuverOffset;
        m_nextOccurStartTime = specialRegionData->nextOccurStartTime;
        m_nextOccurEndTime = specialRegionData->nextOccurEndTime;
    }
}

string SpecialRegionInformation::GetType() const
{
    return m_regionType;
}

string SpecialRegionInformation::GetDescription() const
{
    return m_regionDescription;
}

const Image& SpecialRegionInformation::GetImage() const
{
    return m_image;
}

int SpecialRegionInformation::GetStartManeuverIndex() const
{
    return m_startManeuverIndex;
}

double SpecialRegionInformation::GetStartManeuverOffset() const
{
    return m_startManeuverOffset;
}

int SpecialRegionInformation::GetEndManeuverIndex() const
{
    return m_endManeuverIndex;
}

double SpecialRegionInformation::GetEndManuverOffset() const
{
    return m_endManeuverOffset;
}

int SpecialRegionInformation::GetNextOccurStartTime() const
{
    return m_nextOccurStartTime;
}

int SpecialRegionInformation::GetNextOccurEndTime() const
{
    return m_nextOccurEndTime;
}

/*! @} */