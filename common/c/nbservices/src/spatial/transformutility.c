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

@file transformutility.c
*/
/*
    See file description in header file.

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

#include "palmath.h"
#include "transformutility.h"

static const double LAT_BOUND = 1.5533430342749537;

double  
TO_DEG(double rad)
{
    return (rad * RAD_TO_DEG_MUL);
}

double  
TO_RAD(double deg)
{
    return (deg * DEG_TO_RAD_MUL);
}

boolean	
compare_latlon(double lat1, double lon1, double lat2, double lon2, double tolerance)
{
	double dlat = nsl_fabs(lat1-lat2);
	double dlon = nsl_fabs(lon1-lon2);

	return (boolean) (dlat <= tolerance && dlon <= tolerance);
}


boolean
IsLatitudeValid(double lat)
{
    if ((lat >= -90) && (lat <= 90))
    {
        return TRUE;
    }

    return FALSE;
}

boolean
IsLongitudeValid(double lon)
{
    if ((lon >= -180) && (lon <= 180))
    {
        return TRUE;
    }

    return FALSE;
}

boolean
IsLatitudeLongitudeValid(NB_LatitudeLongitude* point)
{
    if (point && IsLatitudeValid(point->latitude) && IsLongitudeValid(point->longitude))
    {
        return TRUE;
    }

    return FALSE;
}

double boundLat2(double lat)
{
    while (lat < -LAT_BOUND)
        lat = -LAT_BOUND;
    while (lat > LAT_BOUND)
        lat = LAT_BOUND;
    
    return lat;
}

double boundLon2(double lon)
{
    while (lon < -PI)
        lon += TWO_PI;
    while (lon >= PI)
        lon -= TWO_PI;
    
    return lon;
}

double boundMercator(double x)
{
    if(x < -PI)
    {
        x = -PI;
    }
    if(x > PI)
    {
        x = PI;
    }
    return x;
}

void mercatorForward(double lat, double lon, double *x, double *y)
{
	double rlat = boundLat2(TO_RAD(lat));
    *x = boundLon2(TO_RAD(lon));
    *y = log(tan(rlat) + (1/cos(rlat)));
}

void mercatorReverse(double x, double y, double* lat, double* lon)
{
    x = boundMercator(x);
    y = boundMercator(y);
    if (lon != NULL)
    {
        *lon = TO_DEG(x);
    }
    
    if (lat != NULL)
    {
        *lat = TO_DEG(atan(sinh(y)));
    }
}


/*! @} */
