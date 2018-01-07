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
 @file         GPSInterpolator.cpp
 @defgroup     NavigationUIKit
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#include <cmath>
#include "GPSInterpolator.h"
#include "NKUITimerCallbackGuard.h"
extern "C"
{
#include "paluitaskqueue.h"
#include "palclock.h"
}

using namespace nkui;

/*! implement LinearInterpolator class */
LinearInterpolator::LinearInterpolator()
{
}
void LinearInterpolator::SetOriginalData(long timeDuration, double startValue, double endValue)
{
    m_timeDuration = timeDuration;
    m_startValue   = startValue;
    m_endValue     = endValue;
}

/*! implement AngleLinearInterpolator class */
double AngleLinearInterpolator::GetInterpolatedValue(long timeDelta)
{
    double interpolatedValue = m_endValue;
    if (timeDelta < m_timeDuration)
    {
        double distance = m_endValue - m_startValue;
        while (distance > 180)
        {
            distance -= 360;
        }

        while (distance < -180)
        {
            distance += 360;
        }
        interpolatedValue = m_startValue + timeDelta * (distance / m_timeDuration);
        interpolatedValue = fmod(interpolatedValue + 360.0, 360.0);
    }
    return interpolatedValue;
}

/*! implement CoordinateLinearInterpolator */
double CoordinateLinearInterpolator::GetInterpolatedValue(long timeDelta)
{
    double interpolatedValue = m_endValue;
    if (timeDelta < m_timeDuration)
    {
        interpolatedValue = m_startValue + timeDelta * ((m_endValue - m_startValue) / m_timeDuration);
    }
    return interpolatedValue;
}

/*! implement class GPSInterpolator */
/*! implement class Location */
GPSInterpolator::Location::Location()
    :m_heading(0.0),
     m_gpsTime(0)
{
    nsl_memset(&m_coordinate, 0 ,sizeof(nbnav::Coordinates));
}
GPSInterpolator::Location::Location(nbnav::Coordinates coordinate,
                                    double heading,
                                    uint32 gpsTime)
    :m_coordinate(coordinate),
     m_heading(heading),
     m_gpsTime(gpsTime)
{
}

void GPSInterpolator::Location::SetLocation(double latitude,
                                            double longitude,
                                            double heading,
                                            uint32 gpsTime)
{
    m_coordinate.latitude   = latitude;
    m_coordinate.longitude  = longitude;
    m_heading               = heading;
    m_gpsTime               = gpsTime;
}

GPSInterpolator::GPSInterpolator(PAL_Instance* pal)
    : m_interpolateCount(0),
      m_interpolateTotalCount(0),
      m_isEnabled(false),
      m_listerer(NULL),
      m_pal(pal),
      m_isValid(NKUI_NEW bool(true))
{
}

GPSInterpolator::~GPSInterpolator()
{
    m_listerer = NULL;
    if (m_isValid)
    {
        *m_isValid = false;
    }
    PAL_TimerCancel(m_pal, (PAL_TimerCallback *)EventThread_InterpolatorTimerCallback, this);
}

void GPSInterpolator::SetPositionListener(InterpolatedGPSListener* listener)
{
    m_listerer = listener;
}

void GPSInterpolator::StartInterpolate(int timeInterval)
{
    m_isEnabled                 = true;
    m_interpolateTimeInterval   = timeInterval;

}
void GPSInterpolator::StopInterpolate()
{
    m_isEnabled = false;
    PAL_TimerCancel(m_pal, (PAL_TimerCallback *)EventThread_InterpolatorTimerCallback, this);
}

void GPSInterpolator::PositionUpdated(const NKUILocation& position)
{
    if (!m_isEnabled)
    {
        if (m_listerer)
        {
            nbnav::Coordinates coordinate;
            coordinate.latitude = position.Latitude();
            coordinate.longitude = position.Longitude();
            m_listerer->InterpolatedGPSUpdated(coordinate, position.Heading());
        }
    }
    else
    {
        /*! cancel timer which not fired */
        PAL_TimerCancel(m_pal, (PAL_TimerCallback *)EventThread_InterpolatorTimerCallback, this);
        uint32 currentGpsTime = PAL_ClockGetTimeMs();
        m_interpolateCount = 1;
        if (m_lastLocation.m_gpsTime > 0)
        {
            uint32 gpsTimeInterval = currentGpsTime - m_lastLocation.m_gpsTime;
            m_latitudeInterpolator.SetOriginalData(gpsTimeInterval,
                                                  m_lastLocation.m_coordinate.latitude,
                                                  position.Latitude());
            m_longitudeInterpolator.SetOriginalData(gpsTimeInterval,
                                                   m_lastLocation.m_coordinate.longitude,
                                                   position.Longitude());
            m_headingInterpolator.SetOriginalData(gpsTimeInterval,
                                                 m_lastLocation.m_heading,
                                                 position.Heading());

            m_interpolateTotalCount = (int)std::ceil((double)gpsTimeInterval/m_interpolateTimeInterval);
            /*! set timer to interpolate points */
            PAL_TimerSet(m_pal, m_interpolateTimeInterval,
                         (PAL_TimerCallback *)EventThread_InterpolatorTimerCallback, this);
            m_listerer->InterpolatedGPSUpdated(m_lastLocation.m_coordinate,m_lastLocation.m_heading);
        }
        else
        {
            m_lastLocation.SetLocation(position.Latitude(),
                                       position.Longitude(),
                                       position.Heading(),
                                       currentGpsTime);
        }
        m_lastLocation.m_gpsTime = currentGpsTime;
    }
}

void GPSInterpolator::EventThread_InterpolatorTimerCallback(PAL_Instance* pal,
                                                            void *userData,
                                                            PAL_TimerCBReason reason)
{
    if (pal && userData && (reason == PTCBR_TimerFired))
    {
        GPSInterpolator* pThis = static_cast<GPSInterpolator*>(userData);
        /*! as InterpolatedGPSUpdated should running in UIThread, but TimerCallback runs in event task thread,
         so we should switch threads */
        if (pThis->m_isValid && *pThis->m_isValid && pThis->m_isEnabled)
        {
            NKUITimerCallbackGuard<GPSInterpolator>* param = NKUI_NEW NKUITimerCallbackGuard<GPSInterpolator>(*pThis,
                                                                                                              pThis->m_isValid);
            if (param)
            {
                PAL_Error error = PAL_UiTaskQueueAdd(pal, UIThread_InterpolateGPSCallback, param);
                if (error != PAL_Ok)
                {
                    delete param;
                }
            }
        }
    }
}

void GPSInterpolator::UIThread_InterpolateGPSCallback(PAL_Instance* pal, void *userData)
{
    NKUITimerCallbackGuard<GPSInterpolator>* param = static_cast<NKUITimerCallbackGuard<GPSInterpolator>*>(userData);
    if (!pal || !param)
    {
        return;
    }
    if (param->m_isValid && *param->m_isValid && param->m_instance.m_listerer && param->m_instance.m_isEnabled)
    {
        nbnav::Coordinates coordinate;
        long deltaTime = param->m_instance.m_interpolateCount * param->m_instance.m_interpolateTimeInterval;
        coordinate.latitude = param->m_instance.m_latitudeInterpolator.GetInterpolatedValue(deltaTime);
        coordinate.longitude = param->m_instance.m_longitudeInterpolator.GetInterpolatedValue(deltaTime);
        double heading = param->m_instance.m_headingInterpolator.GetInterpolatedValue(deltaTime);
        param->m_instance.m_listerer->InterpolatedGPSUpdated(coordinate,heading);

        if (++(param->m_instance.m_interpolateCount) <= param->m_instance.m_interpolateTotalCount)
        {
            PAL_TimerSet(pal, param->m_instance.m_interpolateTimeInterval,
                         (PAL_TimerCallback *)EventThread_InterpolatorTimerCallback, &param->m_instance);
        }
        param->m_instance.m_lastLocation.SetLocation(coordinate.latitude,
                                  coordinate.longitude,
                                   heading,
                                   param->m_instance.m_lastLocation.m_gpsTime);
    }
    delete param;
}

/*! @} */
