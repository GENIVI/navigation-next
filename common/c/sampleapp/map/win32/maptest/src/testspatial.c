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

    @file     TestSpatial.c
    @date     03/24/2009
    @defgroup TestSpatial_h System tests for spatial API
*/
/*
    See description in header file.

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

#include "testspatial.h"
#include "main.h"
#include "nbspatial.h"
#include "palmath.h"


// Private Functions .............................................................................

// All tests. Add all tests to the TestSpatial_AddTests function
static void TestSpatialDistance(void);
static void TestSpatialMercatorConversion(void);
static void TestSpatialMercatorToTile(void);
static void TestSpatialConvertPointToFromLatLong(void);
static void TestSpatialBoundingMap(void);


// Public Functions ..............................................................................

/*! Add all your test functions here

@return None
*/
void 
TestSpatial_AddTests( CU_pSuite pTestSuite, int level )
{
	// ! Add all your function names here !
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSpatialDistance", TestSpatialDistance);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpatialMercatorConversion", TestSpatialMercatorConversion);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpatialMercatorToTile", TestSpatialMercatorToTile);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpatialConvertPointToFromLatLong", TestSpatialConvertPointToFromLatLong);
	ADD_TEST(level, TestLevelIntermediate, pTestSuite, "TestSpatialBoundingMap", TestSpatialBoundingMap);

};


/*! Add common initialization code here.

@return 0

@see TestSpatial_SuiteCleanup
*/
int 
TestSpatial_SuiteSetup()
{
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestSpatial_SuiteSetup
*/
int 
TestSpatial_SuiteCleanup()
{
	return 0;
}


// Private Functions .............................................................................

/*! Test distance calculations.

    @return None. CUnit asserts get called on failure.
*/
void 
TestSpatialDistance(void)
{
    double heading = 0.0;
    double expectedRangeLow = 0.0;
    double expectedRangeHigh = 0.0;

    // Los Angeles to New York ...................................................................

    // Los Angeles
    double latitude1  = 34.052179;
    double longitude1 = -118.24333;

    // New York
    double latitude2  = 40.756021;
    double longitude2 = -73.986939;

    // Calculate distance between two point on the earth surface
    double distance = NB_SpatialGetLineOfSightDistance(latitude1, longitude1, latitude2, longitude2, &heading);

    // According to "mapcrow.info" it is 3932 kilometer
    expectedRangeLow = 3930 * 1000;
    expectedRangeHigh = 3945 * 1000;

    CU_ASSERT(distance >= expectedRangeLow);
    CU_ASSERT(distance <= expectedRangeHigh);

    // This is the range we expect for the heading (in degree)
    expectedRangeLow = 65;
    expectedRangeHigh = 67;

    CU_ASSERT(heading >= expectedRangeLow);
    CU_ASSERT(heading <= expectedRangeHigh);


    // San Francisco to Los Angeles ..............................................................

    // San Francisco
    latitude1  = 37.775133;
    longitude1 = -122.419203;

    // Los Angeles
    latitude2  = 34.052179;
    longitude2 = -118.24333;

    // Calculate distance between two point on the earth surface
    distance = NB_SpatialGetLineOfSightDistance(latitude1, longitude1, latitude2, longitude2, &heading);

    // According to "mapcrow.info" it is 558 kilometers.
    expectedRangeLow = 558 * 1000;
    expectedRangeHigh = 560 * 1000;

    CU_ASSERT(distance >= expectedRangeLow);
    CU_ASSERT(distance <= expectedRangeHigh);

    // This is the range we expect for the heading (in degree)
    expectedRangeLow = 135;
    expectedRangeHigh = 137;

    CU_ASSERT(heading >= expectedRangeLow);
    CU_ASSERT(heading <= expectedRangeHigh);
}


/*! Test mercator conversions to/from latitude/longitude.

    @return None. CUnit asserts get called on failure.
*/
void 
TestSpatialMercatorConversion(void)
{
    // Los Angeles
    double latitude  = 34.052179;
    double longitude = -118.24333;

    double xMercator = 0.0;
    double yMercator = 0.0;

    // Convert latitude/longitude to mercator coordinates
    NB_SpatialConvertLatLongToMercator(latitude, longitude, &xMercator, &yMercator);

    // I don't really know how to verify the result other than they have to be between PI and -PI.
    CU_ASSERT((xMercator < 0.0) && (xMercator > -PI));
    CU_ASSERT((yMercator > 0.0) && (yMercator < PI));

    latitude = 0.0;
    longitude = 0.0;

    // Convert mercator coordinates to latitude/longitude
    NB_SpatialConvertMercatorToLatLong(xMercator, yMercator, &latitude, &longitude);

    // The latitude/longitude should match the original values
    CU_ASSERT_DOUBLE_EQUAL(latitude, 34.052179, 0.0000001);
    CU_ASSERT_DOUBLE_EQUAL(longitude, -118.24333, 0.0000001);
}


/*! Test conversion from/to mercator coordinates to tile indices.

    @return None. CUnit asserts get called on failure.
*/
void 
TestSpatialMercatorToTile(void)
{
    // New York
    const double ORIGINAL_LATITUDE  = 40.756021;
    const double ORIGINAL_LONGITUDE = -73.986939;

    double latitude  = 0.0;
    double longitude = 0.0;

    double originalXMercator = 0.0;
    double originalYMercator = 0.0;

    double xMercator = 0.0;
    double yMercator = 0.0;

    int xTileIndex = 0;
    int yTileIndex = 0;

    int zoomLevel = 16;

    // First convert the latitude/longitude to mercator points
    NB_SpatialConvertLatLongToMercator(ORIGINAL_LATITUDE, ORIGINAL_LONGITUDE, &xMercator, &yMercator);

    // Now convert the mercator coordinates to a map tile
    NB_SpatialConvertMercatorToTile(xMercator, yMercator, zoomLevel, &xTileIndex, &yTileIndex);

    LOGOUTPUT(LogLevelLow, ("\nInput Lat: %f, Long: %f, Output: Mercator [%f, %f], Tile [%d, %d]\n",
           ORIGINAL_LATITUDE,
           ORIGINAL_LONGITUDE,
           xMercator,
           yMercator,
           xTileIndex,
           yTileIndex));

    // Save those mercator points for comparison later
    originalXMercator = xMercator;
    originalYMercator = yMercator;

    xMercator = 0.0;
    yMercator = 0.0;

    // Now convert it back to mercator points again. We pass in zero for the offset so the resulting
    // mercator point will be moved slightly to the left and up compared to the original points.
    NB_SpatialConvertTileToMercator(xTileIndex, yTileIndex, zoomLevel, 0, 0, &xMercator, &yMercator);

    // The mercator points should match the original ones (although slightly offset)
    CU_ASSERT_DOUBLE_EQUAL(xMercator, originalXMercator, 0.0001)
    CU_ASSERT_DOUBLE_EQUAL(yMercator, originalYMercator, 0.0001)

    LOGOUTPUT(LogLevelLow, ("Original Mercator: [%f, %f], Result Mercator: [%f, %f]\n",
           originalXMercator,
           originalYMercator,
           xMercator,
           yMercator));

    latitude = 0.0;
    longitude = 0.0;

    // Convert back to latitude/longitude
    NB_SpatialConvertMercatorToLatLong(xMercator, yMercator, &latitude, &longitude);

    // The latitude/longitude should match the original values (although slightly offset)
    CU_ASSERT_DOUBLE_EQUAL(latitude, ORIGINAL_LATITUDE, 0.01);
    CU_ASSERT_DOUBLE_EQUAL(longitude, ORIGINAL_LONGITUDE, 0.01);

    LOGOUTPUT(LogLevelLow, ("Original Lat/Long: [%f, %f], Result Lat/Long: [%f, %f]\n",
           ORIGINAL_LATITUDE,
           ORIGINAL_LONGITUDE,
           latitude,
           longitude));
}


/*! Test conversion from a point in a tile to a latitude/longitude.

    @return None. CUnit asserts get called on failure.
*/
void 
TestSpatialConvertPointToFromLatLong(void)
{
    // New York
    const double CENTER_LATITUDE  = 40.75;
    const double CENTER_LONGITUDE = -73.90;

    // Offset a little bit from the center
    const double ORIGINAL_LATITUDE  = 40.755;
    const double ORIGINAL_LONGITUDE = -73.905;

    double latitude  = 0.0;
    double longitude = 0.0;

    double meterPerPixel = 10.0;

    NB_Rectangle rect = {1000, 1000, 100, 100};
    NB_Point point = {0};

    // Get the point
    NB_SpatialConvertLatLongToPoint(CENTER_LATITUDE, 
                                     CENTER_LONGITUDE, 
                                     meterPerPixel, 
                                     0.0, 
                                     &rect, 
                                     ORIGINAL_LATITUDE, 
                                     ORIGINAL_LONGITUDE, 
                                     &point);

    // Conver the point back to latitude/longitude values
    NB_SpatialConvertPointToLatLong(CENTER_LATITUDE, 
                                     CENTER_LONGITUDE, 
                                     meterPerPixel, 
                                     0.0, 
                                     &rect, 
                                     &point, 
                                     &latitude, 
                                     &longitude);

    LOGOUTPUT(LogLevelLow, ("\nOriginal Lat/Long: [%f, %f], Result Lat/Long: [%f, %f], Rect: [%d, %d, %d, %d], Result Point: [%d, %d]\n",
           ORIGINAL_LATITUDE,
           ORIGINAL_LONGITUDE,
           latitude,
           longitude,
           rect.x,
           rect.y,
           rect.dx,
           rect.dy,
           point.x,
           point.y));

    // Verify the the original latitude/longitude match. There are rounding errors since point is an integer.
    CU_ASSERT_DOUBLE_EQUAL(latitude, ORIGINAL_LATITUDE, 0.001);
    CU_ASSERT_DOUBLE_EQUAL(longitude, ORIGINAL_LONGITUDE, 0.001);
}


/*! Test the calculation of a bounding map.

    The test function calculates a new map/tile based on two points. The new map 
    will contain both points and calculate the zoom level.

    @return None. CUnit asserts get called on failure.
*/
void 
TestSpatialBoundingMap(void)
{
    int mapWidth            = 300;          /*!< Width of the map which should contain both points */
    int mapHeight           = 500;          /*!< Height of the map which should contain both points */
    double latitude1        = 40.73;        /*!< Latitude of the first point the map should contain */
    double longitude1       = -73.90;       /*!< Longitude of the first point the map should contain */
    double latitude2        = 40.76;        /*!< Latitude of the second point the map should contain */
    double longitude2       = -73.95;       /*!< Longitude of the second point the map should contain */
    double rotate           = 300.0;        /*!< Desired rotation */

    double centerLatitude   = 0.0;
    double centerLongitude  = 0.0;
    double meterPerPixel    = 0.0;

    // Calculate center point and meter per pixel value
    NB_SpatialCalculateBoundingMap(mapWidth,
                                    mapHeight,
                                    latitude1,
                                    longitude1,
                                    latitude2,
                                    longitude2,
                                    rotate,
                                    &centerLatitude,
                                    &centerLongitude,
                                    &meterPerPixel);

    // It's difficult to verify the result but the center has to be between the two points.
    CU_ASSERT_DOUBLE_EQUAL(centerLatitude,  40.745, 0.001);
    CU_ASSERT_DOUBLE_EQUAL(centerLongitude, -73.925, 0.001);

    LOGOUTPUT(LogLevelLow, ("\nCalculate Map. Point 1: [%f, %f], Point 2: [%f, %f], Result, Center: [%f, %f], Rotation: %f\n",
           latitude1,
           longitude1,
           latitude2,
           longitude2,
           centerLatitude,
           centerLongitude,
           meterPerPixel));
}

/*! @} */


