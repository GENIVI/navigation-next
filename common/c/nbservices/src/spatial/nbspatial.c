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

    @file     nbspatial.c
    @defgroup nbspatial Spatial
*/
/*
    See file description in header file.

    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "paltypes.h"
#include "palstdlib.h"
#include "palmath.h"
#include "nbspatial.h"
#include "spatialvector.h"
#include "transformutility.h"

/* 89 deg */
#define LAT_BOUND 1.5533430342749537


// Private Functions .............................................................................

static void rotate_around_point(double xi, double yi, double xc, double yc, double rotate, double* pxo, double* pyo);

// We currently don't expose this function as public. If the need arises then we can make it public
static void	
CalculateBoundingMapStrip(int mapWidth, 
                          int mapHeight, 
                          int topY, 
                          int bottomY, 
                          double latitude1, 
                          double longitude1, 
                          double latitude2, 
                          double longitude2, 
                          double rotate, 
                          double* latitudec, 
                          double* longitudec, 
                          double* meterPerPixel);


// Public Functions ..............................................................................

NB_DEF double	
NB_SpatialGetLineOfSightDistance(double startLatitude,
                                  double startLongitude,
                                  double endLatitude,
                                  double endLongitude,
                                  double* heading)
{
	// Equatorial Earth Radius is 6378.155 Km ==> 6378155 meters
	// Polar radius 6356 Km. All Calculations Use Equatorial Earth Radius         

	double lat_local	= NB_SpatialBoundLatitude(TO_RAD(startLatitude));
	double long_local	= NB_SpatialBoundLongitude(TO_RAD(startLongitude));
	double lat_remote	= NB_SpatialBoundLatitude(TO_RAD(endLatitude));
	double long_remote  = NB_SpatialBoundLongitude(TO_RAD(endLongitude));

	double alpha        = 0.0;
    double beta         = 0.0;
    double theta        = 0.0;
	double distance     = 0.0;

	// Input: lat_local, long_local and lat_remote, long_remote
	// Output: Curved Earth Distance Between Two Points

	double delta_phi = lat_remote - lat_local;
	double delta_lamda = long_remote - long_local;

	if (delta_phi == 0.0 && delta_lamda == 0.0)
		return 0.0;

    //DELTA LAMADA CASE

	if (delta_lamda >= 0) { // delta_lamda Is Positive Value
		/////////////// Trap #1 (Specific Cases Where The Local and Remote Terminals
		///////////////			 Are On Opposite Sides Of The Greenwich, England)
		if (delta_lamda > PI && delta_lamda <= TWO_PI) {
			delta_lamda  = delta_lamda - TWO_PI; // Radians
		}
		//else if( delta_lamda <= 3.14159265358979 ){
			// Positive Lambda: lambda <= 180 Degrees
			// Do Nothing lambda_long = lambda_long;
		//}
	} // End If Statement
	else { // delta_lamda Is Negative Value
		/////////////// Trap #1 (Specific Cases Where The Local and Remote Terminals
		///////////////			 Are On Opposite Sides Of The Greenwich, England)
		if (delta_lamda > -TWO_PI && delta_lamda < -PI) {
			delta_lamda = delta_lamda + TWO_PI; // Radians
		}
		//else if( (delta_lamda >= -3.14159265358979) ){
			// Positive Lambda: lambda <= 180 Degrees
			// Do Nothing lambda_long = lambda_long;
		//}
	} // End Else Statement

    //DELTA LAMADA CASE

	/////////////// Trap #2
	if (delta_lamda == 0) {
		if (delta_phi > 0) {
			alpha = 0.0;
		}
		if (delta_phi < 0) {
			alpha = PI;
		}
	}
	/////////////// End Trap #2
	else{

		/////////////// Beta Test For Trap #3
		//	beta = (cos(lat_local)*tan(lat_remote)-sin(lat_local)*cos(delta_lamda))/sin(delta_lamda);

		beta = ((nsl_cos(lat_local) * nsl_tan(lat_remote)) - nsl_sin(lat_local) * nsl_cos(delta_lamda)) / nsl_sin(delta_lamda);

		/////////////// End Beta Test For Trap #3

		/////////////// Trap #3
		if (beta > -1.0e-5 && beta < 1.0e-5) {
			if (delta_lamda > 0) {
				alpha = 1.5707963267949;
			}
			if (delta_lamda < 0) {
				alpha = 4.71238898038469;
			}
		} // End IF Statement
		/////////////// Trap #3
		else{	
			/////////////// SET ALPHA
			alpha = nsl_atan(1.0 / beta);
			/////////////// END SET ALPHA

			if (alpha > 0.0 && delta_lamda > 0.0) {
				// Do Nothing alpha = alpha
			}
			else if ((alpha > 0.0 && delta_lamda < 0.0) || (alpha < 0 && delta_lamda > 0.0)) {
				alpha = alpha + PI;
			}
			else if (alpha < 0.0 && delta_lamda < 0.0) {
				alpha = alpha + TWO_PI;
			}
		} // End Else Statement
	} // End Else Statement

	/////////////// Theta Calculation
	if (delta_lamda == 0.0) {
		if (delta_phi >= 0.0) { // Positive Quanity
			theta = delta_phi; // Radians
		}
		else{	// Negative Quanity
			theta = -delta_phi; // Radians
		}
	}
	else{ // lambda_long not equal to zero
		theta = nsl_sin(delta_lamda) * nsl_cos(lat_remote) / nsl_sin(alpha);

			// Calculates The Argument For The Sine Function
		if(theta >= 0){ // Positive Quanity
			//theta = asin(theta); // Radians
			//asin(X)=atan(X/SQRT(1-X^2))
			theta = nsl_atan(theta / nsl_sqrt(1.0 - theta * theta));
		}
		else{	// Negative Quanity [since asin(-x)=-asin(x)]
			//theta = -asin(theta); // Radians
			//asin(X)=atan(X/SQRT(1-X^2))
			theta = -nsl_atan(theta / nsl_sqrt(1.0 - theta * theta));
		}
	} // End Else Statement
	/////////////// End Theta Calculation

	distance = 6378155.0 * theta; // Distance in meters.
	//distance = 20925705.3805774*theta; //Distance in Feet.
	//distance = 3963.20177662*theta; //Distance in Miles

	if (heading != NULL)
    {
		*heading = TO_DEG(alpha);
    }

	return distance;
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialCalculateBoundingMap(int mapWidth, 
                                int mapHeight, 
                                double latitude1, 
                                double longitude1, 
                                double latitude2, 
                                double longitude2, 
                                double rotate, 
                                double* centerLatitude, 
                                double* centerLongitude, 
                                double* meterPerPixel)
{
	CalculateBoundingMapStrip(mapWidth, 
                              mapHeight, 
                              0, 
                              mapHeight, 
                              latitude1, 
                              longitude1, 
                              latitude2, 
                              longitude2, 
                              rotate, 
                              centerLatitude, 
                              centerLongitude, 
                              meterPerPixel);
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialConvertLatLongToMercator(double latitude,
                                    double longitude,
                                    double* xMercator,
                                    double* yMercator)
{
	double rlat = 0.0;

	*xMercator = NB_SpatialBoundLongitudeRadians(TO_RAD(longitude));

    rlat = NB_SpatialBoundLatitudeRadians(TO_RAD(latitude));
	*yMercator = nsl_log(nsl_tan(rlat)+nsl_sec(rlat));
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialConvertMercatorToLatLong(double xMercator,
                                    double yMercator,
                                    double* latitude,
                                    double* longitude)
{
	xMercator = NB_SpatialBoundMercator(xMercator);
	yMercator = NB_SpatialBoundMercator(yMercator);

	*longitude = TO_DEG(xMercator);
	*latitude = TO_DEG(nsl_atan(nsl_sinh(yMercator)));
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialConvertLatLongToPoint(double centerLatitude,
                                 double centerLongitude,
                                 double meterPerPixel,
                                 double rotate,
						         NB_Rectangle* rect,
                                 double latitude,
                                 double longitude,
                                 NB_Point* point)
{
	double fs = nsl_sin(TO_RAD(-rotate));
	double fc = nsl_cos(TO_RAD(-rotate));
	double c_x, c_y;
	double p_x, p_y;

	double dx_merc, dy_merc, dx_px, dy_px;
	double cos_lat =  nsl_cos(TO_RAD(centerLatitude));

    double max_int16 = 1 << 15;
    double min_int16 = -max_int16;
    
    double offset_x = 0;
    double offset_y = 0;

	NB_SpatialConvertLatLongToMercator(centerLatitude, centerLongitude, &c_x, &c_y);
	NB_SpatialConvertLatLongToMercator(latitude, longitude, &p_x, &p_y);

	dx_merc = p_x - c_x;
	dy_merc = p_y - c_y;

	dx_px = (dx_merc * RADIUS_EARTH_METERS * cos_lat) / meterPerPixel;
	dy_px = (-dy_merc * RADIUS_EARTH_METERS * cos_lat) / meterPerPixel;

    offset_x = dx_px * fc + dy_px * -fs;
    offset_y = dx_px * fs + dy_px * fc;
    
    offset_x = offset_x < min_int16 ? min_int16 : (offset_x > max_int16 ? max_int16 : offset_x);
    offset_y = offset_y < min_int16 ? min_int16 : (offset_y > max_int16 ? max_int16 : offset_y);
    
	point->x = rect->x + (rect->dx / 2) + (int16)offset_x;
	point->y = rect->y + (rect->dy / 2) + (int16)offset_y;
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialConvertPointToLatLong(double centerLatitude, 
                                 double centerLongitude, 
                                 double meterPerPixel, 
                                 double rotate, 
                                 NB_Rectangle* rect, 
                                 NB_Point* point, 
                                 double* latitude, 
                                 double* longitude)
{
	double fs = nsl_sin(TO_RAD(rotate));
	double fc = nsl_cos(TO_RAD(rotate));
	
	double dx_merc, dy_merc, dx_px, dy_px;
	double c_x, c_y;

	double x = point->x - (rect->x + (rect->dx / 2));
	double y = point->y - (rect->y + (rect->dy / 2));
	double cos_lat =  nsl_cos(TO_RAD(centerLatitude));

	NB_SpatialConvertLatLongToMercator(centerLatitude, centerLongitude, &c_x, &c_y);

	dx_px = x * fc + y * (-fs);
	dy_px = x * fs + y * fc;

	dx_merc = (dx_px * meterPerPixel) / (RADIUS_EARTH_METERS * cos_lat);
	dy_merc = (-dy_px * meterPerPixel) / (RADIUS_EARTH_METERS * cos_lat);

	NB_SpatialConvertMercatorToLatLong(c_x + dx_merc, c_y + dy_merc, latitude, longitude);
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialConvertTileToMercator(int xTileIndex,
                                 int yTileIndex,
                                 int zoomLevel,
                                 double xOffset,
                                 double yOffset,
                                 double* xMercator,
                                 double* yMercator)
{
	double tilex = (double)xTileIndex + xOffset;
	double tiley = (double)yTileIndex + yOffset;
	double tscale = (double)(1 << (zoomLevel - 1));

    if (xMercator)
    {
	    *xMercator = (tilex/tscale - 1.0) * PI;
    }

    if (yMercator)
    {
	    *yMercator = (1.0 - tiley/tscale) * PI;
    }
}

/* See header file for description. */ 
NB_DEF void	
NB_SpatialConvertMercatorToTile(double xMercator,
                                 double yMercator,
                                 int zoomLevel,
                                 int* xTileIndex,
                                 int* yTileIndex)
{
    double tscale = (double)(1 << (zoomLevel - 1));

    double xIndex = (xMercator / PI + 1.0) * tscale;
    double yIndex = (1.0 - yMercator / PI) * tscale;

    if (xTileIndex)
    {
        /* 
            Use floor() instead of just casting to int in order to work correctly with negative numbers.
            Negative values are not valid tile-indices, but it is up to the user to determine, if he wants to pass
            values passed -180/180 or -90/90
         
            The values would be negative passed "-1" anyway.
         */
	    *xTileIndex = (int)floor(xIndex);
    }

    if (yTileIndex)
    {
        // See comment above
	    *yTileIndex = (int)floor(yIndex);
    }
}

/* See header file for description. */ 
NB_DEF void
NB_SpatialConvertMercatorToTileOffset(
    int xTileIndex,
    int yTileIndex,
    int zoomLevel,
    double xMercator,
    double yMercator,
    double* xOffset,
    double* yOffset)
{
    double tscale = (double)(1 << (zoomLevel - 1));

    *xOffset = tscale * ((xMercator/PI) + 1) - xTileIndex;
    *yOffset = -tscale * ((yMercator/PI) - 1) - yTileIndex;
}

NB_DEF double
NB_SpatialMetersToMercator(double meters, double cosLatitudeRadius)
{
    return meters / cosLatitudeRadius;
}

// @todo (Bug 55870) - remove this hard coded value
#define METERS_PER_PIXEL 2

NB_DEF void
NB_SpatialMeterLengthToMercatorProjectionByTileIndex(
    int xTileIndex,
    int yTileIndex,
    int zoomLevel, 
    int16 pxDist, 
    double* mDist)
{
    double cos_lat = 0;
    double cos_lat_radius = 0;
    double xMercator = 0;
    double yMercator = 0;
    double lat = 0;
    double lon = 0;

    NB_SpatialConvertTileToMercator(xTileIndex, yTileIndex, zoomLevel, 0, 0, &xMercator, &yMercator);
    NB_SpatialConvertMercatorToLatLong(xMercator, yMercator, &lat, &lon);

    cos_lat = nsl_cos(TO_RAD(lat));
    cos_lat_radius = cos_lat * RADIUS_EARTH_METERS;

    if (mDist)
    {
        *mDist = (pxDist * METERS_PER_PIXEL) / (cos_lat_radius);
    }
}


NB_DEF void
NB_SpatialMeterLengthToMercatorProjectionByLatitude(
    double latitude,
    int16 pxDist, 
    double* mDist)
{
    double cos_lat = 0;
    double cos_lat_radius = 0;

    cos_lat = nsl_cos(TO_RAD(latitude));
    cos_lat_radius = cos_lat * RADIUS_EARTH_METERS;

    if (mDist)
    {
        *mDist = (pxDist * METERS_PER_PIXEL) / (cos_lat_radius);
    }
}

NB_DEF double
NB_SpatialBoundLatitude(double latitude) 
{
	while (latitude < -PI)
		latitude += TWO_PI;
	while (latitude > PI)
		latitude -= TWO_PI;
	return (latitude);
}

NB_DEF double
NB_SpatialBoundLongitude(double longitude)
{
	while (longitude < 0.0)
		longitude += TWO_PI;
	while (longitude >= TWO_PI)
		longitude -= TWO_PI;
	return (longitude);
}

NB_DEF double
NB_SpatialBoundLatitudeRadians(double latitude)
{
	while (latitude < -LAT_BOUND)
		latitude = -LAT_BOUND;
	while (latitude > LAT_BOUND)
		latitude = LAT_BOUND;
	return (latitude);
}

NB_DEF double
NB_SpatialBoundLongitudeRadians(double longitude)
{
	while (longitude < -PI)
		longitude += TWO_PI;
	while (longitude >= PI)
		longitude -= TWO_PI;
	return (longitude);
}

NB_DEF double
NB_SpatialBoundMercator(double x)
{
	if (x < -PI)
		x = -PI;
	if (x > PI)
		x = PI;
	return x;
}

void
rotate_around_point(double xi, double yi, double xc, double yc, double rotate, double* pxo, double* pyo)
{
	double fs = nsl_sin(TO_RAD(-rotate));
	double fc = nsl_cos(TO_RAD(-rotate));
	double x,y;

    x = xi - xc;
    y = yi - yc;

    *pxo = (fc * x + fs * y) + xc;
    *pyo = (-fs * x + fc * y) + yc;
}

void	
CalculateBoundingMapStrip(int mapWidth, 
                          int mapHeight, 
                          int topY, 
                          int bottomY, 
                          double latitude1, 
                          double longitude1, 
                          double latitude2, 
                          double longitude2, 
                          double rotate, 
                          double* latitudec, 
                          double* longitudec, 
                          double* meterPerPixel)
{
	// topY is the top of the horizontal strip (e.g. 0 is the top of the rectangle)
	// bottomY is the bottom of the horizontal strip (e.g. mapHeight is the bottom of the rectangle)
	double m1_x, m1_y, m2_x, m2_y;
	double centerLatitude, centerLongitude;
	double mc_x, mc_y;
	double r1_x, r1_y, r2_x, r2_y;
	double dx_merc, dy_merc;
	double mpp_x, mpp_y;
	double coslat;
	double pmp;
	NB_Point cPt;
	NB_Rectangle mapRect;

	NB_SpatialConvertLatLongToMercator(latitude1,longitude1,&m1_x, &m1_y);
    NB_SpatialConvertLatLongToMercator(latitude2,longitude2,&m2_x, &m2_y);
	NB_SpatialConvertMercatorToLatLong((m1_x + m2_x) / 2,(m1_y + m2_y) / 2, &centerLatitude, &centerLongitude);
	NB_SpatialConvertLatLongToMercator(centerLatitude, centerLongitude, &mc_x, &mc_y);
	rotate_around_point(m1_x, m1_y, mc_x, mc_y, rotate, &r1_x, &r1_y);
	rotate_around_point(m2_x, m2_y, mc_x, mc_y, rotate, &r2_x, &r2_y);
	
	dx_merc = nsl_fabs(r1_x - r2_x) * 1.4;
	dy_merc = nsl_fabs(r1_y - r2_y) * 1.4;

	if (topY > 0 && bottomY < mapHeight)
    {
		dy_merc *= 1.1;
    }
	else
    {
		dy_merc *= 1.4;
    }

	coslat = nsl_cos(TO_RAD(centerLatitude));

    mpp_x = (dx_merc * RADIUS_EARTH_METERS * coslat) / mapWidth;
    mpp_y = (-dy_merc * RADIUS_EARTH_METERS * coslat) / mapHeight;
	pmp = MAX(nsl_fabs(mpp_x), nsl_fabs(mpp_y));
	// Return latitude/longitude of the center of the strip, not the center of the mapRect
	// Should adjust rotation as well to match the new center, but is exact for no rotation, should be close if there is rotation 
	mapRect.x = mapRect.y = 0;
	mapRect.dx = (int16)mapWidth;
	mapRect.dy = (int16)mapHeight;
	cPt.x = (int16)(mapWidth/2);
	cPt.y = (int16)(mapHeight - (topY + bottomY)/2);	// This is the new center for y to get the strip lined up correctly
	NB_SpatialConvertPointToLatLong( centerLatitude, centerLongitude, pmp, rotate, &mapRect, &cPt, &centerLatitude, &centerLongitude ); 

	*meterPerPixel = pmp;
	*latitudec = centerLatitude;
	*longitudec = centerLongitude;
}

int OutputPolygonPoint(NB_Vector* pointIn, NB_Vector* pointOut, nb_size* index, int outsize)
{
	if (*index >= outsize)
    {
		return -1;
    }

	pointOut[(*index)++] = *pointIn;

	return 0;
}

NB_DEF NB_Error
NB_SpatialClipPolygon(NB_Vector* vectorIn, nb_size vectorInSize, NB_Vector* vectorOut, nb_size vectorOutSize, NB_SpatialClipPointFunction clipper, NB_SpatialIntersectFunction intersect, void* userData, nb_size* points)
{
    int temp = 0;

    if (!points)
    {
        return NE_INVAL;
    }

    temp = sutherland_hodgman_polygon_clip(vectorIn, (int)vectorInSize, vectorOut, (int)vectorOutSize, clipper, intersect, userData);
    *points = (nb_size)temp;

    return temp < 0 ? NE_RANGE : NE_OK;
}

NB_DEF nb_boolean
NB_SpatialClipMinZ(NB_Vector* point, void* userData)
{
	double zLimit = *((double*)userData);
	return (nb_boolean)(point->z >= zLimit);
}

NB_DEF nb_boolean
NB_SpatialClipMaxZ(NB_Vector* point, void* userData)
{
	double zLimit = *((double*)userData);
	return (nb_boolean)(point->z <= zLimit);
}

NB_DEF void
NB_SpatialIntersectZ(NB_Vector* point1, NB_Vector* point2, NB_Vector* pointOut, void* userData)
{
    double x1 = point1->x;
    double y1 = point1->y;
    double z1 = point1->z;
    double x2 = point2->x;
    double y2 = point2->y;
    double z2 = point2->z;
    double zClip = *((double*)userData);

    pointOut->x = x2 + ((zClip - z2) / (z1 - z2)) * (x1 - x2); 
    pointOut->y = y2 + ((zClip - z2) / (z1 - z2)) * (y1 - y2); 
    pointOut->z = zClip;
}

/*! @} */

