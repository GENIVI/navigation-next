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

*/
/*
    See header

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
#include "platformutil.h"
#include "networkutil.h"
#include "spatialvector.h"


// Local Constants ...............................................................................


// Local Types ...................................................................................

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance* pal;
    NB_Context* context;

} Instances;


// Local Functions ...............................................................................

// Test functions
static void TestSpatialVector(void);
static void TestSpatialPointInPolygon(void);
static void TestSpatialPolygonIntersect1(void);
static void TestSpatialPolygonIntersect2(void);

// Helper functions

static nb_boolean CreateInstances(Instances* instances);
static void DestroyInstances(Instances* instances);



// Public Functions ..............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestSpatial_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestSpatialVector",            &TestSpatialVector);
    CU_add_test(pTestSuite, "TestSpatialPointInPolygon",    &TestSpatialPointInPolygon);
    CU_add_test(pTestSuite, "TestSpatialPolygonIntersect1",  &TestSpatialPolygonIntersect1);
    CU_add_test(pTestSuite, "TestSpatialPolygonIntersect2",  &TestSpatialPolygonIntersect2);
};


/*! Add common initialization code here.

    @return 0
*/
int
TestSpatial_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

    @return 0
*/
int 
TestSpatial_SuiteCleanup()
{
    return 0;
}


// Local Functions ...............................................................................

/*! General Vector test functions.

    @return None, but CU-asserts are called on errors.
*/
void 
TestSpatialVector(void)
{
    #define START_LATITUDE      34.123456789012345
    #define START_LONGITUDE     -118.123456789012345

    NB_LatitudeLongitude position = {START_LATITUDE, START_LONGITUDE};
    NB_LatitudeLongitude result = {0};
    NB_Vector test = {0};

    // Convert to mercator
    vector_from_lat_lon(&test, position.latitude, position.longitude);

    // Convert back
    vector_to_lat_lon(&result.latitude, &result.longitude, &test);

    // We get rounding errors at some point. Not sure what the granularity should be.
    CU_ASSERT_DOUBLE_EQUAL(result.latitude, START_LATITUDE, 12);
    CU_ASSERT_DOUBLE_EQUAL(result.longitude, START_LONGITUDE, 12);
}

/*! Test point-in-polygon.

    @return None, but CU-asserts are called on errors.
*/
void 
TestSpatialPointInPolygon(void)
{
    #define POINT_COUNT 4

    NB_Vector temp = {0};

    NB_Vector point1 = {2, 2, 0};       // Inside
    NB_Vector point2 = {4.5, 4.9, 0};   // Inside
    NB_Vector point3 = {1, 1, 0};       // On corner

    NB_Vector point4 = {-0.5, 2, 0};    // To the left
    NB_Vector point5 = {5.5, 2.5, 0};   // To the right
    NB_Vector point6 = {2, -5, 0};      // Below
    NB_Vector point7 = {3, 10, 0};      // Above
    
    // Convext counter-clockwise-ordering polygon
    NB_Vector polygon[POINT_COUNT] = 
    {
        1,1,0,      // z-value is ignored
        5,1,0,
        5,5,0,
        1,5,0,
    };

    // Inside
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point1, polygon, POINT_COUNT), TRUE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point2, polygon, POINT_COUNT), TRUE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point3, polygon, POINT_COUNT), TRUE);

    // Outside
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point4, polygon, POINT_COUNT), FALSE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point5, polygon, POINT_COUNT), FALSE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point6, polygon, POINT_COUNT), FALSE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point7, polygon, POINT_COUNT), FALSE);

    // Change to clockwise ordering
    temp = polygon[3];
    polygon[3] = polygon[1];
    polygon[1] = temp;

    // Inside
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point1, polygon, POINT_COUNT), TRUE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point2, polygon, POINT_COUNT), TRUE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point3, polygon, POINT_COUNT), TRUE);

    // Outside
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point4, polygon, POINT_COUNT), FALSE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point5, polygon, POINT_COUNT), FALSE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point6, polygon, POINT_COUNT), FALSE);
    CU_ASSERT_EQUAL(vector_is_point_in_polygon(&point7, polygon, POINT_COUNT), FALSE);
}

/*! Test polygon intersection.

    @return None, but CU-asserts are called on errors.
*/
void 
TestSpatialPolygonIntersect1(void)
{
    #define POINT_COUNT 4

    NB_Vector polygon1[POINT_COUNT] = 
    {
        1,  1,  0,      // z-value is ignored
        1,  5,  0,
        5,  5,  0,
        5,  1,  0,
    };

    NB_Vector polygon2[POINT_COUNT] = 
    {
        1, -1,  0,      // z-value is ignored
        5, -1,  0,
        5, -5,  0,
        1, -5,  0,
    };

    NB_Vector polygon3[POINT_COUNT] = 
    {
        8,  8,  0,      // z-value is ignored
        12, 4,  0,
        8,  0,  0,
        4.8,4,  0,
    };

    NB_Vector polygon4[POINT_COUNT] = 
    {
       -1,   2,  0,      // z-value is ignored
       -1,   3,  0,
       15, 2.5,  0,
       15, 3.5,  0,
    };

    // Pass in the same polygon. It should intersect.
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, TRUE, polygon1, POINT_COUNT, TRUE), TRUE);

    // Don't intersect
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, TRUE, polygon2, POINT_COUNT, TRUE), FALSE);

    // Last point of second polygon lies in first polygon
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, TRUE, polygon3, POINT_COUNT, FALSE), TRUE);

    // Special case. The two polygons have no common points but still intersect (they build a cross)
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, TRUE, polygon4, POINT_COUNT, FALSE), TRUE);
}

/*! More polygon intersection tests.

    @return None, but CU-asserts are called on errors.
*/
void 
TestSpatialPolygonIntersect2(void)
{
    /*
        Test a few more cases.
    */

    #define POINT_COUNT 4

    // test polygon
    NB_Vector polygon1[POINT_COUNT] = 
    {
        0,  3,  0,      // z-value is ignored
       -2,  0,  0,
        0, -3,  0,
        2,  0,  0,
    };

    // non-intersect
    NB_Vector polygon2[POINT_COUNT] = 
    {
      2.5,  0,  0,      // z-value is ignored
        0,3.5,  0,
        3,  5,  0,
        8,  0,  0,
    };

    // non-intersect
    NB_Vector polygon3[POINT_COUNT] = 
    {
        7,  0,  0,      // z-value is ignored
        1, -2,  0,
        1, -6,  0,
        7, -1,  0,
    };

    // intersect
    NB_Vector polygon4[POINT_COUNT] = 
    {
       -1,  -1,  0,      // z-value is ignored
       -5,  -1,  0,
       -5,  -3,  0,
       -2,  -7,  0,
    };

    // intersect
    NB_Vector polygon5[POINT_COUNT] = 
    {
       -1,   0,  0,      // z-value is ignored
       -3,   4,  0,
       -5,   2,  0,
       -3,   0,  0,
    };

    // intersect rectangle
    NB_Vector polygon6[POINT_COUNT] = 
    {
        1,   5,  0,      // z-value is ignored
        1,  -5,  0,
        2,  -5,  0,
        2,   5,  0,
    };

    // intersect (enclosing rectangle)
    NB_Vector polygon7[POINT_COUNT] = 
    {
       10,  10,  0,      // z-value is ignored
       10, -10,  0,
      -10, -10,  0,
      -10,  10,  0,
    };

    // Non intersection
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, FALSE, polygon2, POINT_COUNT, FALSE), FALSE);
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, FALSE, polygon3, POINT_COUNT, FALSE), FALSE);

    // Intersection, non-rectangles
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, FALSE, polygon4, POINT_COUNT, FALSE), TRUE);
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, FALSE, polygon5, POINT_COUNT, FALSE), TRUE);

    // Intersection, rectangles
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, FALSE, polygon6, POINT_COUNT, TRUE), TRUE);
    CU_ASSERT_EQUAL(vector_polygons_intersect(polygon1, POINT_COUNT, FALSE, polygon7, POINT_COUNT, TRUE), TRUE);
}

/*! @} */
