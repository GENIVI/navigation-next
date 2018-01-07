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

    @file     nbsolarcalculator.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbsolarcalculator.h"
#include "palmath.h"
#include <math.h>

#define TO_DEG(x) nsl_mul(x, 180/PI)
#define TO_RAD(x) nsl_mul(x, PI/180)

static double CalculateJulianDay(uint16 year, uint16 month, uint16 day);
static double CalculateTimeJulianCenturies(double JD);
static double CalculateJulianDayFromJulianCenturies(double time);
static double CalculateGeometricMeanLongSun(double time);
static double CalculateGeometricMeanAnomalySun(double time);
static double CalculateEccentricityEarthOrbit(double time);
static double CalculateSunEqOfCenter(double time);
static double CalculateSunTrueLong(double time);
//static double CalculateSunTrueAnomaly(double time);
//static double CalculateSunRadVector(double time);
static double CalculateSunApparentLong(double time);
static double CalculateMeanObliquityOfEcliptic(double time);
static double CalculateObliquityCorrection(double time);
//static double CalculateSunRightAscension(double time);
static double CalculateSunDeclination(double time);
static double CalculateEquationOfTime(double time);
static double CalculateHourAngleSunrise(double latitude, double solarDec);
static double CalculateHourAngleSunset(double latitude, double solarDec);
static double CalculateSunriseUTC(double JD, double latitude, double longitude);
static double CalculateSolarNoonUTC(double time, double longitude);
static double CalculateSunsetUTC(double JD, double latitude, double longitude);

NB_DEF NB_Error
NB_SolarCalculatorGetTime(double latitude, double longitude, int16 timeZone, nb_boolean useDaySaving,
                        uint16 year, uint16 month, uint16 day, double* sunriseTime, double* sunsetTime)
{
    double JD = CalculateJulianDay(year, month, day);
    // calculate sunrise for this date
    double riseTimeGMT = CalculateSunriseUTC(JD, latitude, longitude);
    // TODO: Check riseTimeGMT valid.
    // calculate sunset for this date
    double setTimeGMT = CalculateSunsetUTC(JD, latitude, longitude);
    // TODO: Check the setTimeGMT valid.

    double daySavings = useDaySaving ? 60.0 : 0.0;
    double zone = timeZone;

    if (!sunriseTime || !sunsetTime)
    {
        return NE_INVAL;
    }

    if (zone > 12 || zone < -12.5)
    {
        zone = 0;
    }

    *sunriseTime = riseTimeGMT - (60.0 * zone) + daySavings;
    *sunsetTime = setTimeGMT - (60.0 * zone)+ daySavings;
    return NE_OK;
}

double CalculateJulianDay(uint16 year, uint16 month, uint16 day)
{
    double A = 0.0;
    double B = 0.0;
    double JD = 0.0;

    if (month <= 2)
    {
        year -= 1;
        month += 12;
    }

    A = floor(year / 100.0);
    B = 2 - A + floor(A / 4.0);
    JD = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + B - 1524.5;
    return JD;
}

/*! Convert Julian Day to centuries since J2000.0.
 
    @JD        The Julian day to convert.
 
    @return    The time value corresponding to the Julian Day.
 */
double CalculateTimeJulianCenturies(double JD)
{
    return (JD - 2451545.0) / 36525.0;
}

/*! Convert centuries since J2000.0 to Julian day.
 
    @time         Number of Julian centuries since J2000.0.
 
    @return    The Julian Day correspoinding to the time value.
 */
double CalculateJulianDayFromJulianCenturies(double time)
{
    return time * 36525.0 + 2451545.0;
}

/*! Calculate the Geometric Mean Longitude of the Sun.
 
    @time         Number of Julian Centuries since J2000.0.
 
    @return    The Geometric Mean Longitude of the Sun in degrees.
 */
double CalculateGeometricMeanLongSun(double time)
{
    double L0 = 280.46646 + time * (36000.76983 + 0.0003032 * time);
    if (L0 > 360.0 || L0 < 0.0)
    {
        // 1 Avoid massive cycles.
        int count = (int)(floor(L0/360.0));
        L0 -= count*360.0;
        // 2 Check L0 again due to float precision, probably need one or two more iterations.
        while(L0 > 360.0)
        {
            L0 -= 360.0;
        }
        while(L0 < 0.0)
        {
            L0 += 360.0;
        }
    }
    return L0;
}

/*! Calculate the Geometric Mean Anomaly of the Sun.
 
    @time         Number of Julian centuries since J2000.0.
 
    @return    The Geometric Mean Anomaly of the Sun in degrees.
 */
double CalculateGeometricMeanAnomalySun(double time)
{
    return 357.52911 + time * (35999.05029 - 0.0001537 * time);
}

/*! Calculate the eccentricity of earth's orbit.
 
    @time         Number of Julian centuries since J2000.0.
 
    @return    The unitless eccentricity.
 */
double CalculateEccentricityEarthOrbit(double time)
{
    return 0.016708634 - time * (0.000042037 + 0.0000001267 * time);
}

double CalculateSunEqOfCenter(double time)
{
    double mDegree = CalculateGeometricMeanAnomalySun(time);
    double mRad = TO_RAD(mDegree);
    double sinm = nsl_sin(mRad);
    double sin2m = nsl_sin(mRad + mRad);
    double sin3m = nsl_sin(mRad + mRad + mRad);
    double centerSun = sinm * (1.914602 - time * (0.004817 + 0.000014 * time)) + sin2m * (0.019993 - 0.000101 * time) + sin3m * 0.000289;
    return centerSun;
}

/*! calculate the true longitude of the sun.
 
    @time         number of Julian centuries since J2000.0
 
    @return    sun's true longitude in degrees.
 */
double CalculateSunTrueLong(double time)
{
    double longitudeSun = CalculateGeometricMeanLongSun(time);
    double centerSun = CalculateSunEqOfCenter(time);
    double o = longitudeSun + centerSun;
    return o;
}

/*! calculate the true anamoly of the sun.
 
    @time         number of Julian centuries since J2000.0
 
    @return    sun's true anamoly in degrees.
 */
//double CalculateSunTrueAnomaly(double time)
//{
//    double m = CalculateGeometricMeanAnomalySun(time);
//    double c = CalculateSunEqOfCenter(time);
//    double v = m + c;
//    return v;
//}

/*! calculate the distnace to the sun in AU.
 
    @time         number of Julian centuries since J2000.0
 
    @return    sun radius vector in AUs.
 */
//double CalculateSunRadVector(double time)
//{
//    double v = CalculateSunTrueAnomaly(time);
//    double e = CalculateEccentricityEarthOrbit(time);
//    double R = (1.000001018 * (1 - e * e)) / (1 + e * nsl_cos(TO_RAD(v)));
//    return R;
//}

/*! calculte the apparent longitude of the sun.
 
    @time         number of Julian centuries since J2000.0
 
    @return    sun's apparent longitude in degrees.
 */
double CalculateSunApparentLong(double time)
{
    double o = CalculateSunTrueLong(time);
    double omega = 125.04 - 1934.136 * time;
    double lambda = o - 0.00569 - 0.00478 * nsl_sin(TO_RAD(omega));
    return lambda;
}

/*! calculate the mean obliquity of the ecliptic.
 
    @time         number of Julian centuries since J2000.0
 
    @return    mean obliquity in degrees.
 */
double CalculateMeanObliquityOfEcliptic(double time)
{
    double seconds = 21.448 - time * (46.8150 + time * (0.00059 - time * 0.001813));
    double e0 = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
    return e0;
}

/*! calculate the corrected obliquity of the ecliptic.
 
    @time         number of Julian centuries since J2000.0.
 
    @return    corrected obliquity in degrees.
 */
double CalculateObliquityCorrection(double time)
{
    double e0 = CalculateMeanObliquityOfEcliptic(time);
    double omega = 125.04 - 1934.136 * time;
    double e = e0 + 0.00256 * nsl_cos(TO_RAD(omega));
    return e;
}

/*! Calculate the right ascension of the sun.
 
    @time         The number of Julian centuries since J2000.0.
 
    @return    Sun's right ascension in degrees
 */
//double CalculateSunRightAscension(double time)
//{
//    double e = CalculateObliquityCorrection(time);
//    double lambda = CalculateSunApparentLong(time);
//    double tananum = nsl_cos(TO_RAD(e)) * nsl_sin(TO_RAD(lambda));
//    double tanadenom = nsl_cos(TO_RAD(lambda));
//    double atan2num = 0.0;
//    double alpha = 0.0;
//    /* improve atan2(tananum, tanadenom)*/
//    if (tanadenom < 0)
//    {
//        atan2num = nsl_atan(tananum / tanadenom) + PI;
//    }
//    else
//    {
//        atan2num = nsl_atan(tananum / tanadenom);
//    }
//    alpha = TO_DEG(atan2num);
//    return alpha;
//}

/*! Calculate the declination of the sun.
 
    @time         The number of Julian centuries since J2000.0.
 
    @return    The sun's declination in degrees.
 */
double CalculateSunDeclination(double time)
{
    double e = CalculateObliquityCorrection(time);
    double lambda = CalculateSunApparentLong(time);
    double sint = nsl_sin(TO_RAD(e)) * nsl_sin(TO_RAD(lambda));
    double cost = nsl_acos(sint);
    double sin = PI/2 - cost;
    double theta = TO_DEG(sin);
    return theta;
}

/*! Calculate the difference between true solar time and mean solar time.
 
    @time         The number of Julian centuries since J2000.0.
 
    @return    The equation of time in minutes of time.
 */
double CalculateEquationOfTime(double time)
{
    double epsilon = CalculateObliquityCorrection(time);
    double l0 = CalculateGeometricMeanLongSun(time);
    double e = CalculateEccentricityEarthOrbit(time);
    double m = CalculateGeometricMeanAnomalySun(time);
    double y = nsl_tan(TO_RAD(epsilon) / 2.0);

    double sin210 = nsl_sin(2.0 * TO_RAD(l0));
    double sinm = nsl_sin(TO_RAD(m));
    double cos210 = nsl_cos(2.0 * TO_RAD(l0));
    double sin410 = nsl_sin(4.0 * TO_RAD(l0));
    double sin2m = nsl_sin(2.0 * TO_RAD(m));

    double Etime = 0.0;
    y *= y;
    Etime = y * sin210 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos210 - 0.5 * y * y * sin410 - 1.25 * e * e * sin2m;
    return TO_DEG(Etime) * 4.0;
}

/*! Calculate the hour angle of the sun at sunrise for the latitude.
 
    @latitude    The latitude of observer in degrees.
    @solarDec    The declination angle of sun in degrees.
 
    @return      Hour angle of sunrise in radians.
 */
double CalculateHourAngleSunrise(double latitude, double solarDec)
{
    double latRad = TO_RAD(latitude);
    double sdRad = TO_RAD(solarDec);
    double HA    = 0;

    double val = nsl_cos(TO_RAD(90.833)) / (nsl_cos(latRad) * nsl_cos(sdRad)) - nsl_tan(latRad) * nsl_tan(sdRad);
    if(val > 1.0)
    {
        val = 1.0;
    }
    else if (val < -1.0)
    {
        val = -1.0;
    }

    HA = nsl_acos(val);
    return HA;
}

/*! Calculate the hour angle of the sun at sunset for the latitude.
 
    @latitude    latitude of observer in degrees.
    @solarDec    Declination angle of sun in degrees.
 
    @return      hour angle of sunset in radians.
 */
double CalculateHourAngleSunset(double latitude, double solarDec)
{
    double latRad = TO_RAD(latitude);
    double sdRad = TO_RAD(solarDec);
    double HA    = 0;

    double val = nsl_cos(TO_RAD(90.833)) / (nsl_cos(latRad) * nsl_cos(sdRad)) - nsl_tan(latRad) * nsl_tan(sdRad);
    if(val > 1.0)
    {
        val = 1.0;
    }
    else if (val < -1.0)
    {
        val = -1.0;
    }

    HA = nsl_acos(val);
    return -HA;
}

/*! calculate the Universal Coordinated Time (UTC) of sunrise for the given day at the given location on earth.
 
    @JD           Julian Day
    @latitude     latitude of observer in degrees.
    @longitude    longitude of observer in degrees.
 
    @return       time in minutes from zero Z.
 */
double CalculateSunriseUTC(double JD, double latitude, double longitude)
{
    double time      = CalculateTimeJulianCenturies(JD);
    double noonmin   = 0;
    double tnoon     = 0;
    double eqTime    = 0;
    double solarDec  = 0;
    double hourAngle = 0;
    double delta     = 0;
    double timeDiff  = 0;
    double timeUTC   = 0;
    double newt      = 0;

    // Find the time of solar noon at the location, and use that declination.
    // This is better than start of the Julian day.

    // This limit comes from http://www.esrl.noaa.gov/gmd/grad/solcalc/sunrise.html.
    if(latitude < -89.0)
    {
        latitude = -89.0;
    }

    if(latitude > 89.0)
    {
        latitude = 89.0;
    }

    noonmin = CalculateSolarNoonUTC(time, longitude);
    tnoon = CalculateTimeJulianCenturies(JD + noonmin / 1440.0);

    // First pass to approximate sunrise (using solar noon)
    eqTime = CalculateEquationOfTime(tnoon);
    solarDec = CalculateSunDeclination(tnoon);
    hourAngle = CalculateHourAngleSunrise(latitude, solarDec);

    delta = longitude - TO_DEG(hourAngle);
    timeDiff = 4 * delta;    // in minutes of time
    timeUTC = 720 + timeDiff - eqTime;   // in minutes

    // Second pass includes fractional jday in gamma calc.
    newt = CalculateTimeJulianCenturies(CalculateJulianDayFromJulianCenturies(time) + timeUTC / 1440.0);

    eqTime = CalculateEquationOfTime(newt);
    solarDec = CalculateSunDeclination(newt);
    hourAngle = CalculateHourAngleSunrise(latitude, solarDec);
    delta = longitude - TO_DEG(hourAngle);
    timeDiff = 4* delta;
    timeUTC = 720.0 + timeDiff - eqTime;    // in minutes
    return timeUTC;
}

/*! calculate the Universal Coordinated Time (UTC) of solar noon for the given day at the given location on earth.
 
    @time            number of Julian centuries since J2000.0
    @longitude    longitude of observer in degrees
 
    @return       time in minutes from zero Z.
 */
double CalculateSolarNoonUTC(double time, double longitude)
{
    // First pass uses approximate solar noon to calculate eqtime
    double tnoon = CalculateTimeJulianCenturies(CalculateJulianDayFromJulianCenturies(time) + longitude / 360.0);
    double eqTime = CalculateEquationOfTime(tnoon);
    double solNoonUTC = 720.0 + (longitude * 4.0) - eqTime; // in minutes
    double newt = CalculateTimeJulianCenturies(CalculateJulianDayFromJulianCenturies(time) - 0.5 + solNoonUTC / 1440.0);
    eqTime = CalculateEquationOfTime(newt);
    solNoonUTC = 720.0 + (longitude * 4.0) - eqTime;    // in minutes
    return solNoonUTC;
}

/*! calculate the Universal Coordinated Time (UTC) of sunset for the given day at the given location on earth.
 
    @JD           Julian day
    @latitude     latitude of observer in degrees.
    @longitude    longitude of observer in degrees.
 
    @return       time in minutes from zero Z.
 */
double CalculateSunsetUTC(double JD, double latitude, double longitude)
{
    double time = CalculateTimeJulianCenturies(JD);

    // Find the time of solar noon at the location, and use that declination.
    // This is better than start of the Julian day.
    double noonmin = CalculateSolarNoonUTC(time, longitude);
    double tnoon = CalculateTimeJulianCenturies(JD + noonmin / 1440.0);
    
    // First calculates sunrise and approx length of day
    double eqTime = CalculateEquationOfTime(tnoon);
    double solarDec = CalculateSunDeclination(tnoon);
    double hourAngle = CalculateHourAngleSunset(latitude, solarDec);
    double delta = longitude - TO_DEG(hourAngle);
    double timeDiff = 4.0 * delta;
    double timeUTC = 720.0 + timeDiff - eqTime;
    
    // first pass used to include fractional day in gamma calc
    double newt = CalculateTimeJulianCenturies(CalculateJulianDayFromJulianCenturies(time) + timeUTC / 1440.0);
    eqTime = CalculateEquationOfTime(newt);
    solarDec = CalculateSunDeclination(newt);
    hourAngle = CalculateHourAngleSunset(latitude, solarDec);
    delta = longitude - TO_DEG(hourAngle);
    timeDiff = 4.0 * delta;
    timeUTC = 720.0 + timeDiff - eqTime;
    return timeUTC;
}
