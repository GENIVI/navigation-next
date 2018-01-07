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
    @file         BreadCrumb.cpp
    @defgroup     nkui
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary to
    TeleCommunication Systems, Inc., and considered a trade secret as defined
    in section 499C of the penal code of the State of California. Use of this
    information by anyone other than authorized employees of TeleCommunication
    Systems is granted only under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
    --------------------------------------------------------------------------
*/

/*! @{ */

#include "BreadCrumb.h"
#include "palstdlib.h"
#include "palclock.h"

#include <map>
#include <algorithm>

using namespace nkui;

#define DEFAULT_CRUMB_INTERVAL      30
#define MINIMUM_UPDATE_INTERVAL     200
#define MAX_CONTINUOUS_CIRCLE_NUM   5

BreadCrumb::BreadCrumb(NKUIMapInterface* pMapInterface)
    : m_pMapInterface(pMapInterface)
    , m_maxCrumbsNum(~0)
    , m_crumbInterval(DEFAULT_CRUMB_INTERVAL)
    , m_lastUpdateTime(0)
    , m_continuousCircleNum(0)
{
    nsl_assert(pMapInterface);
}

BreadCrumb::~BreadCrumb()
{
    Clear();
}

void
BreadCrumb::AddPoint(const NKUILocation& location)
{
    m_locations.push_back(location);
    NKUICoordinate coordinate(location.Latitude(),location.Longitude());
    if (!m_breadCrumbLocations.empty())
    {
        // check whether we should add a new point on map
        const NKUICoordinate firstPoint(m_breadCrumbLocations.back().Latitude(),
                                        m_breadCrumbLocations.back().Longitude());
        ScreenPoint screenPoint1 = m_pMapInterface->ToScreenLocation(coordinate);
        ScreenPoint screenPoint2 = m_pMapInterface->ToScreenLocation(firstPoint);
        if (!CheckDistance(screenPoint1, screenPoint2))
        {
            return;
        }
    }
    // create new crumb
    CreateBreadCrumb(location, m_continuousCircleNum==MAX_CONTINUOUS_CIRCLE_NUM ? BCT_Arrow : BCT_Circle);
    m_breadCrumbLocations.push_back(location);
}

void BreadCrumb::Clear()
{
    m_pMapInterface->RemoveAllBreadCrumb();
    m_locations.clear();
}

void
BreadCrumb::Update()
{
    uint32 timeStamp = PAL_ClockGetTimeMs();
    if (timeStamp - m_lastUpdateTime < MINIMUM_UPDATE_INTERVAL)
    {
        return;
    }
    m_lastUpdateTime = timeStamp;
    typedef std::vector<ScreenPoint>            ScreenPointArray;
    typedef std::map<int32, ScreenPointArray>   ScreenPointMap;

    // create index for x axis and y axis
    // only check the points nearby to decide whether we add a point on map
    // so we can do a much smaller amount of calculation when filter the points
    ScreenPointMap screenXMap;
    ScreenPointMap screenYMap;
    m_breadCrumbLocations.clear();
    m_pMapInterface->RemoveAllBreadCrumb();

    uint32 index = 0;
    m_continuousCircleNum = 0;

    std::list<NKUILocation>::iterator iterLocation = m_locations.begin();
    while (iterLocation != m_locations.end())
    {
        const NKUICoordinate coordinate(iterLocation->Latitude(), iterLocation->Longitude());
        ScreenPoint screenPoint = m_pMapInterface->ToScreenLocation(coordinate);

        // find all existing points close to this point
        int32 xKey = (int32)(screenPoint.x / (float)m_crumbInterval);
        int32 yKey = (int32)(screenPoint.y / (float)m_crumbInterval);
        ScreenPointArray closePoints;
        for (int32 key = xKey - 1; key <= xKey + 1; ++key)
        {
            ScreenPointMap::iterator iter = screenXMap.find(key);
            if (iter != screenXMap.end())
            {
                ScreenPointArray& array = iter->second;
                closePoints.insert(closePoints.end(), array.begin(), array.end());
            }
        }
        for (int32 key = yKey - 1; key <= yKey + 1; ++key)
        {
            ScreenPointMap::iterator iter = screenYMap.find(key);
            if (iter != screenYMap.end())
            {
                ScreenPointArray& array = iter->second;
                closePoints.insert(closePoints.end(), array.begin(), array.end());
            }
        }

        // check space
        bool haveEnoughSpace = true;
        ScreenPointArray::iterator iterClosePoint = closePoints.begin();
        while (iterClosePoint != closePoints.end())
        {
            const ScreenPoint& point = *iterClosePoint;
            if (!CheckDistance(point, screenPoint))
            {
                haveEnoughSpace = false;
                break;
            }
            iterClosePoint++;
        }
        if (haveEnoughSpace)
        {
            // show new point
            m_breadCrumbLocations.push_back(*iterLocation);

            // add to index map
            screenXMap[xKey].push_back(screenPoint);
            screenYMap[yKey].push_back(screenPoint);
            if (index < m_maxCrumbsNum)
            {
                CreateBreadCrumb(*iterLocation,
                                 m_continuousCircleNum==MAX_CONTINUOUS_CIRCLE_NUM ?
                                 BCT_Arrow : BCT_Circle);
            }
            ++index;
        }
        iterLocation++;
    }
}

NKUIBreadCrumb* BreadCrumb::CreateBreadCrumb(const NKUILocation& location, NKUIBreadCrumbType breadCrumbType)
{
    if (breadCrumbType == BCT_Circle)
    {
        ++m_continuousCircleNum;
    }
    else
    {
        m_continuousCircleNum = 0;
    }
    NKUICoordinate coordinate(location.Latitude(), location.Longitude());
    NKUIBreadCrumbParameter param(coordinate, location.Heading() , true, breadCrumbType);
    return m_pMapInterface->AddBreadCrumb(param);
}

bool
BreadCrumb::CheckDistance(const ScreenPoint& p1, const ScreenPoint& p2)
{
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    if ((dx * dx + dy * dy) < ((float)m_crumbInterval * (float)m_crumbInterval))
    {
        return false;
    }
    return true;
}

/*! @} */
