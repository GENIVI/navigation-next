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

    @file     nbspatial.h
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*!
    @addtogroup nbspatial
    @{
*/

#ifndef NIM_SPATIAL_H
#define	NIM_SPATIAL_H

#include "paltypes.h"
#include "nbexp.h"
#include "navpublictypes.h"

typedef struct 
{
    double x;
    double y;
    double z;
} NB_Vector;

// Public Functions ..............................................................................

/*! Calculates the distance between two points.

    This function takes two latitude/longitude coordinates and return the curved distance between 
    the points.

    @return Distance between the two points
*/
NB_DEC double	
NB_SpatialGetLineOfSightDistance(double startLatitude,     /*!< Latitude of starting point */
                                 double startLongitude,    /*!< Longitude of starting point */
                                 double endLatitude,       /*!< Latitude of end point */
                                 double endLongitude,      /*!< Longitude of end point */
                                 double* heading           /*!< On return the heading in degree from the start to the end point. 
                                                                Optional, set to NULL if not needed. */
                                 );

/*! Calculate map/tile and zoom level which will conatin two given points.

    This function takes two points (as latitude/longitude values) and calculates the zoom level (meter in pixel) 
    and the center of a map which will contain both points.

    @return None
*/
NB_DEC void	
NB_SpatialCalculateBoundingMap(int mapWidth,               /*!< Width of the map, in pixel, which should contain both points */
                               int mapHeight,              /*!< Height of the map, in pixel, which should contain both points */
                               double latitude1,           /*!< Latitude of the first point the map should contain */
                               double longitude1,          /*!< Longitude of the first point the map should contain */
                               double latitude2,           /*!< Latitude of the second point the map should contain */
                               double longitude2,          /*!< Longitude of the second point the map should contain */
                               double rotate,              /*!< Desired rotation for the map in degree. North being zero. */
                               double* centerLatitude,     /*!< On return the latitude of the center point of the map which will contain both points */
                               double* centerLongitude,    /*!< On return the longitude of the center point of the map which will contain both points */
                               double* meterPerPixel       /*!< On return the meter per pixel of the map to contain both points. */
                               );

/*! Convert latitude/longitude to a mercator point.

    A mercator coordinate has a value between -PI to +PI.

    @return None

    @see NB_SpatialConvertMercatorToLatLong
*/
NB_DEC void	
NB_SpatialConvertLatLongToMercator(double latitude,        /*!< Latitude of point to convert */
                                   double longitude,       /*!< Longitude of point to convert */
                                   double* xMercator,      /*!< On return the x-coordinate of the point in mercator coordinates */
                                   double* yMercator       /*!< On return the y-coordinate of the point in mercator coordinates */
                                   );

/*! Convert a mercator point to latitude/longitude values.

    A mercator coordinate has a value between -PI to +PI.

    @return None

    @see NB_SpatialConvertLatLongToMercator
*/
NB_DEC void	
NB_SpatialConvertMercatorToLatLong(double xMercator,       /*!< x-coordinate of point to convert, in mercator coordinates */
                                   double yMercator,       /*!< y-coordinate of point to convert, in mercator coordinates */
                                   double* latitude,       /*!< On return the latitude of the converted point */
                                   double* longitude       /*!< On return the longitude of the converted point */
                                   );

/*! Convert latitude/longitude to a point in a rectangle (tile).

    This function takes a latitude/longitude value and finds the exact point (pixel)
    in a given rectangle considering the tile/rectangle rotation.

    @return None

    @see NB_SpatialConvertPointToLatLong
*/
NB_DEC void	
NB_SpatialConvertLatLongToPoint(double centerLatitude,     /*!< Latitude of center of given rectangle/tile */
                                double centerLongitude,    /*!< Longitude of center of given rectangle/tile */
                                double meterPerPixel,      /*!< scale of the tile in meters per pixel */
                                double rotate,             /*!< Rotation of the rectangle/tile in degree. North being zero degrees */
						        NB_Rectangle* rect,            /*!< Given rectangle/tile which contains the latitude/longitude to convert */
                                double latitude,           /*!< Latitude value to convert */
                                double longitude,          /*!< Longitude value to convert */
                                NB_Point* point           /*!< On return the point/pixel of the point in the rectangle */
                                );

/*! Convert a point in a rectangle (tile) to latitude/longitude values.

    This function takes a point in a rectangle/tile and return the latitude/longitude of that pixel. 
    It considers the rotation (if any) of the rectangle/tile.

    @return None

    @see NB_SpatialConvertLatLongToPoint
*/
NB_DEC void	
NB_SpatialConvertPointToLatLong(double centerLatitude,     /*!< Latitude of center of given rectangle/tile */
                                double centerLongitude,    /*!< Longitude of center of given rectangle/tile */
                                double meterPerPixel,      /*!< scale of the tile in meters per pixel */
                                double rotate,             /*!< Rotation of the rectangle/tile in degree. North being zero degrees */
                                NB_Rectangle* rect,            /*!< Rectangle/tile which contains the point/pixel to convert */
                                NB_Point* point,          /*!< Point/Pixel in rectangle/tile to convert */
                                double* latitude,          /*!< On return the latitude value of the point to convert */
                                double* longitude          /*!< On return the longitude value of the point to convert */
                                );

/*! Convert a (point in a) tile index to a mercator point.

    See NB_VectorMapGetTile() for a detailed description about zoom levels and tiles.

    This function converts a pixel in a given tile (or the top/left corner, if no offset is chosen)
    to mercator coordinates.
    
    @return None

    @see NB_VectorMapGetTile
    @see NB_SpatialConvertMercatorToTile
*/
NB_DEC void	
NB_SpatialConvertTileToMercator(int xTileIndex,            /*!< x-Index of the tile to convert */
                                int yTileIndex,            /*!< y-Index of the tile to convert */
                                int zoomLevel,             /*!< zoom level between 0-17 */
                                double xOffset,            /*!< x-offset from the top/left corner. Values between 0.0 and 1.0 */
                                double yOffset,            /*!< y-offset from the top/left corner. Values between 0.0 and 1.0 */
                                double* xMercator,         /*!< On return the mercator x-coordinate. Values between -PI and +PI */
                                double* yMercator          /*!< On return the mercator y-coordinate. Values between -PI and +PI */
                                );

/*! Convert a mercator coordinate/point to a tile index.

    See NB_VectorMapGetTile() for a detailed description about zoom levels and tiles.

    This function converts a mercator coordinate to a tile which contains the point.

    @return None

    @see NB_VectorMapGetTile
    @see NB_SpatialConvertTileToMercator
*/
NB_DEC void	
NB_SpatialConvertMercatorToTile(double xMercator,          /*!< x-coordinate of the mercator point to convert */
                                double yMercator,          /*!< y-coordinate of the mercator point to convert */
                                int zoomLevel,             /*!< zoom level between 0-17 */
                                int* xTileIndex,           /*!< On return the x-index of the tile which contains the mercator point */
                                int* yTileIndex            /*!< On return the y-index of the tile which contains the mercator point */
                                );


/*! Convert a mercator coordinate/point to a tile index offset.

See NB_VectorMapGetTile() for a detailed description about zoom levels and tiles.

This function converts a mercator coordinate to a tile top/left corner offset.

@return None
@see NB_VectorMapGetTile
@see NB_SpatialConvertTileToMercator
*/
NB_DEC void	
NB_SpatialConvertMercatorToTileOffset(
        int xTileIndex,             /*!< x-Index of the tile to convert */
        int yTileIndex,             /*!< y-Index of the tile to convert */
        int zoomLevel,              /*!< zoom level between 0-17 */
        double xMercator,           /*!< On return the mercator x-coordinate. Values between -PI and +PI */
        double yMercator,           /*!< On return the mercator y-coordinate. Values between -PI and +PI */
        double* xOffset,            /*!< x-offset from the top/left corner. Values between 0.0 and 1.0 */
        double* yOffset             /*!< y-offset from the top/left corner. Values between 0.0 and 1.0 */
        );

/*! Convert a distance in meters to a distance in Mercator units

@param meters The distance in meters
@param cosLatitudeRadius The cosine of the reference latitude multiplied by the radius of the earth in meters
@return double The Mercator distance
*/
NB_DEC double
NB_SpatialMetersToMercator(double meters, double cosLatitudeRadius);

NB_DEC void 
NB_SpatialMeterLengthToMercatorProjectionByTileIndex(int xTileIndex,int yTileIndex,int zoomLevel, int16 pxDist, double* mDist);

NB_DEC void
NB_SpatialMeterLengthToMercatorProjectionByLatitude(double latitude,int16 pxDist, double* mDist);


/*! Bound a latitude to the range -pi (inclusive) to pi (inclusive)
@param latitude The latitude to bound
@return The bound latitude
*/
NB_DEC double NB_SpatialBoundLatitude(double latitude);


/*! Bound a longitude to the range 0 (inclusive) to 2*pi (exclusive)
@param longitude The longitude to bound
@return The bound longitude
*/
NB_DEC double NB_SpatialBoundLongitude(double longitude);


/*! Bound a latitude to the range -89 degrees (in radians, inclusive) to +89 degrees (in radians, inclusive)
@param latitude The latitude to bound
@return The bound latitude
*/
NB_DEC double NB_SpatialBoundLatitudeRadians(double latitude);


/*! Bound a longitude to the range -pi (inclusive) to pi (exclusive)
@param longitude The longitude to bound
@return The bound longitude
*/
NB_DEC double NB_SpatialBoundLongitudeRadians(double longitude);


/*! Bound a mercator point value to the range -pi (inclusive) to pi (exclusive)
@param value The value to bound
@return The bound value
*/
NB_DEC double NB_SpatialBoundMercator(double value);


/*! User-defined function for polygon clipping
*/
typedef nb_boolean (*NB_SpatialClipPointFunction)(NB_Vector* point, void* userData);


/*! User-defined function for polygon intersections
*/
typedef void (*NB_SpatialIntersectFunction)(NB_Vector* point1, NB_Vector* point2, NB_Vector* pointOut, void* userData);


/*! Clip a polygon

Clip a polygon using the Sutherland-Hodgman clipping algorithm
@param vectorIn The input polygon
@param vectorInSize The number of points in the input polygon
@param vectorOut The output polygon
@param vectorOutSize The maximum number of points in the output polygon
@param clipper Function used to clip points
@param intersect Function used to determine an intersection
@param userData Opaque data passed to the clipper and intersect functions
@param points The number of points in the clipped polygon
@return NB_Error
*/
NB_DEC NB_Error NB_SpatialClipPolygon(NB_Vector* vectorIn, nb_size vectorInSize, NB_Vector* vectorOut, nb_size vectorOutSize, NB_SpatialClipPointFunction clipper, NB_SpatialIntersectFunction intersect, void* userData, nb_size* points);


/*! Clip function to bound the Z element of a polygon
@param point The point to be bound
@param userData A pointer to a double representing the minimum Z value
@return 
*/
NB_DEC nb_boolean NB_SpatialClipMinZ(NB_Vector* point, void* userData);


/*! Clip function to bound the Z element of a polygon
@param point The point to be bound
@param userData A pointer to a double representing the maximum Z value
@return 
*/
NB_DEC nb_boolean NB_SpatialClipMaxZ(NB_Vector* point, void* userData);


/*! Intersection function for the Z element of a polygon
@param point1
@param point2
@param pointOut
@param userData A pointer to a double representing the minimum Z value
@return 
*/
NB_DEC void NB_SpatialIntersectZ(NB_Vector* point1, NB_Vector* point2, NB_Vector* pointOut, void* userData);


#endif

/*! @} */
