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
 @file         GPSInterpolator.h
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

#ifndef __NavigationUIKit__GPSInterpolator__
#define __NavigationUIKit__GPSInterpolator__

#include "NKUILocation.h"
#include "smartpointer.h"

extern "C"
{
#include "paltimer.h"
}

namespace nkui
{

class InterpolatedGPSListener
{
public:
    virtual ~InterpolatedGPSListener(){};

    /*! Interpolated position.
     *
     * @param coordinates - interpolated coordinate
     * @param speed       - current speed.(speed would not be interpolated)
     * @param heading     - interpolated heading.
     *
     * @return None
     */
    virtual void InterpolatedGPSUpdated(const nbnav::Coordinates& coordinate,
                                        double heading) = 0;

    /*! called if interpolation failed.
     *
     * @return None
     */
    virtual void InterpolatingError(){};
};

/*! class linear interpolator */
class LinearInterpolator
{
public:
    LinearInterpolator();
    virtual ~LinearInterpolator(){};
    /*! get distance which interpolated by the class.
     *
     * @param  timeDelta - delta time.
     * @return interpolates value.
     */
    virtual double GetInterpolatedValue(long timeDelta) = 0;

    /*! set original data
     *
     * @param timeDuration - the time during start time and end time.
     * @param startValue.
     * @param endValue.
     * @return none.
     */
    virtual void SetOriginalData(long timeDuration, double startValue, double endValue);
protected:
    /*! time during start time and end time*/
    long m_timeDuration;
    /*! start value*/
    double m_startValue;
    /*! end value */
    double m_endValue;
};

/*! class angle linear interpolator */
class AngleLinearInterpolator : public LinearInterpolator
{
public:
    /*! get angle which interpolated by the class.
     *
     * @param  timeDelta - delta time.
     * @return interpolates value.
     */
    virtual double GetInterpolatedValue(long timeDelta);
};

/*! class coordinate linear interpolator */
class CoordinateLinearInterpolator : public LinearInterpolator
{
public:
    /*! get coordiante which interpolated by the class.
     *
     * @param  timeDelta - delta time.
     * @return interpolates value.
     */
    virtual double GetInterpolatedValue(long timeDelta);
};

/*! class GPS linear interpolator */
class GPSInterpolator
{
public:
    GPSInterpolator(PAL_Instance* pal);
    virtual ~GPSInterpolator();

    /*! set the listener who will recieve the interpolated coordinate
     *
     * @param listener - listener who will recieve the interpolated coordinate
     *
     * @return none
     */
    void SetPositionListener(InterpolatedGPSListener* listener);

    /*! start the interpolation
     *
     * @param rate    - rate of interpolation. for example: 100 mean interpolate a point per 100ms.
     *
     * @return none
     */
    void StartInterpolate(int timeInterval = 50);
    /*! stop the interpolation
     * @return none
     */
    void StopInterpolate();

    void PositionUpdated(const NKUILocation& position);

private:
    static void EventThread_InterpolatorTimerCallback(PAL_Instance* pal,
                                                      void *userData,
                                                      PAL_TimerCBReason reason);

    static void UIThread_InterpolateGPSCallback(PAL_Instance* pal, void *userData);

    /*! define a new clss to encapsulate coordiante and heading,
        this class just used by class GPSInterpolator */
    class Location
    {
    public:
        Location();
        Location(nbnav::Coordinates coordinate,
                 double heading,
                 uint32 gpsTime);
        void SetLocation(double latitude,
                         double longitude,
                         double heading,
                         uint32 gpsTime);
        nbnav::Coordinates m_coordinate;
        double             m_heading;
        uint32             m_gpsTime;
    };
    /*! count interpolations between two gps locations */
    int                          m_interpolateCount;
    /*! total number of interpolated points between two gps locations */
    int                          m_interpolateTotalCount;
    /*! store the last location */
    Location                     m_lastLocation;
    /*! store the time inverval of interpolation*/
    int                          m_interpolateTimeInterval;
    /*! interpolation is enabled or not */
    bool                         m_isEnabled;
    InterpolatedGPSListener*     m_listerer;
    /*! the pal instance. */
    PAL_Instance*                m_pal;
    CoordinateLinearInterpolator m_latitudeInterpolator;
    CoordinateLinearInterpolator m_longitudeInterpolator;
    AngleLinearInterpolator      m_headingInterpolator;
    shared_ptr<bool>             m_isValid;
};

}

#endif /* defined(__NavigationUIKit__GPSInterpolator__) */
/*! @} */