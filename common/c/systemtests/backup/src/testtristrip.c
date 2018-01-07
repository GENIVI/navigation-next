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

@file     testtristrip.c
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

#include "testtristrip.h"
#include "main.h"
#include "platformutil.h"
#include "palfile.h"
#include "palstdlib.h"

#include "nbcontext.h"
#include "nbvectortile.h"
#include "nbtristrip.h"

// Constants .............................................................................

#define TRISTRIP_POINTS_PER_SEGMENT 2
#define TRISTRIP_POINTS_ENDCAP      5           // 4 points + null

#define TESTDATA_TILE_INDEX_X       11342
#define TESTDATA_TILE_INDEX_Y       26266
#define TESTDATA_TILE_ZOOMLEVEL     16


// Test data .............................................................................

// Tile index: 11342, 26266; zoom = 16
// Road: Katie Ave
// Polyline: 0
// Segments: 2
// Tristrips: 8
NB_MercatorPoint polylineOnePoints [] = 
{
    {-2.054150, 0.623341},
    {-2.054128, 0.623353}
};
NB_VectorTileRoadType polylineOneRoadType = NB_VMRT_Local;


// Tile index: 11342, 26266; zoom = 16
// Road: Carol Ln
// Polyline: 0
// Segments: 4
// Tristrips: 12
NB_MercatorPoint polylineTwoPoints [] = 
{
    {-2.054159, 0.623353},
    {-2.054154, 0.623356},
    {-2.054141, 0.623364},
    {-2.054138, 0.623366}
};
NB_VectorTileRoadType polylineTwoRoadType = NB_VMRT_Local;

// Tile index: 11342, 26266; zoom = 16
// Road: Julie Ave
// Polyline: 0
// Segments: 6
// Tristrips: 16
NB_MercatorPoint polylineThreePoints [] = 
{
    {-2.054147, 0.623325},
    {-2.054147, 0.623333},
    {-2.054148, 0.623338},
    {-2.054150, 0.623341},
    {-2.054159, 0.623353},
    {-2.054167, 0.623364}
};
NB_VectorTileRoadType polylineThreeRoadType = NB_VMRT_Local;



// Private Functions .............................................................................

static void TestTriStripPolylineOne(void);
static void TestTriStripPolylineMultiple(void);


void VerifyAndOutputTriStripInfo(NB_MercatorPolyline* polyline, NB_TriStrip* triStrip);


// Public Functions ..............................................................................

/*! Add all your test functions here

@return None
*/
void 
TestTriStrip_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestTriStripPolylineOne", TestTriStripPolylineOne);   
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestTriStripPolylineMultiple", TestTriStripPolylineMultiple);

};


/*! Add common initialization code here.

@return 0

@see TestTriStrip_SuiteCleanup
*/
int 
TestTriStrip_SuiteSetup()
{

    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestTriStrip_SuiteSetup
*/
int 
TestTriStrip_SuiteCleanup()
{
    return 0;
}


// Private Functions .............................................................................

/*! Convert one polyline to tristrip.

@return None. Asserts are called on failure.
*/
void 
TestTriStripPolylineOne(void)
{
    NB_Error result = NE_OK;
    NB_TriStrip* triStrip = 0;
    NB_MercatorPolyline polyline = {0};
    int expectedTriStripsCount = 0;
    NB_VectorTileIndex tileIndex = {TESTDATA_TILE_INDEX_X, TESTDATA_TILE_INDEX_Y};

    polyline.count = sizeof(polylineThreePoints) / sizeof(NB_MercatorPoint);
    polyline.points = &polylineThreePoints[0];

    expectedTriStripsCount = polyline.count * TRISTRIP_POINTS_PER_SEGMENT + TRISTRIP_POINTS_ENDCAP;
    expectedTriStripsCount--; // Not counting the last point


    result = NB_TriStripPolylineToMercatorCreate(polylineThreeRoadType, &polyline, tileIndex, TESTDATA_TILE_ZOOMLEVEL, TRUE, -1, &triStrip, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result == NE_OK)
    {
        CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripForeground->count);
        CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripBackground->count);
        VerifyAndOutputTriStripInfo(&polyline, triStrip);

        result = NB_TriStripDestroy(triStrip);
    }

}


/*! Convert a few polyline to tristrips

@return None. Asserts are called on failure.
*/
void 
TestTriStripPolylineMultiple(void)
{
    NB_Error result = NE_OK;
    NB_TriStrip* triStrip = 0;
    NB_MercatorPolyline polyline = {0};
    int expectedTriStripsCount = 0;
    NB_VectorTileIndex tileIndex = {TESTDATA_TILE_INDEX_X, TESTDATA_TILE_INDEX_Y};

    // PolylineOne
    polyline.count = sizeof(polylineOnePoints) / sizeof(NB_MercatorPoint);
    polyline.points = &polylineOnePoints[0];

    expectedTriStripsCount = polyline.count * TRISTRIP_POINTS_PER_SEGMENT + TRISTRIP_POINTS_ENDCAP;
    expectedTriStripsCount--; // Not counting the last point

    result = NB_TriStripPolylineToMercatorCreate(polylineOneRoadType, &polyline, tileIndex, TESTDATA_TILE_ZOOMLEVEL, TRUE, -1, &triStrip, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        return;
    }
    CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripForeground->count);
    CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripBackground->count);
    VerifyAndOutputTriStripInfo(&polyline, triStrip);
    result = NB_TriStripDestroy(triStrip);
    

    // PolylineTwo
    polyline.count = sizeof(polylineTwoPoints) / sizeof(NB_MercatorPoint);
    polyline.points = &polylineTwoPoints[0];

    expectedTriStripsCount = polyline.count * TRISTRIP_POINTS_PER_SEGMENT + TRISTRIP_POINTS_ENDCAP;
    expectedTriStripsCount--; // Not counting the last point

    result = NB_TriStripPolylineToMercatorCreate(polylineTwoRoadType, &polyline, tileIndex, TESTDATA_TILE_ZOOMLEVEL, TRUE, -1, &triStrip, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        return;
    }
    CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripForeground->count);
    CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripBackground->count);
    VerifyAndOutputTriStripInfo(&polyline, triStrip);
    result = NB_TriStripDestroy(triStrip);


    // PolylineThree
    polyline.count = sizeof(polylineThreePoints) / sizeof(NB_MercatorPoint);
    polyline.points = &polylineThreePoints[0];

    expectedTriStripsCount = polyline.count * TRISTRIP_POINTS_PER_SEGMENT + TRISTRIP_POINTS_ENDCAP;
    expectedTriStripsCount--; // Not counting the last point

    result = NB_TriStripPolylineToMercatorCreate(polylineThreeRoadType, &polyline, tileIndex, TESTDATA_TILE_ZOOMLEVEL, FALSE, 0.000040, &triStrip, NULL, NULL);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result != NE_OK)
    {
        return;
    }

    CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripForeground->count);
    CU_ASSERT_EQUAL(expectedTriStripsCount, triStrip->triStripBackground->count);
    VerifyAndOutputTriStripInfo(&polyline, triStrip);
    result = NB_TriStripDestroy(triStrip);
}


/*! Verify and output polyline and tristrip info

@return None. Asserts are called on failure.
*/
void
VerifyAndOutputTriStripInfo(
    NB_MercatorPolyline* polyline,
    NB_TriStrip* triStrip)
{
    int i = 0;
    double mx = 0;
    double my = 0;

    LOGOUTPUT(LogLevelHigh, ("\n\n Polyline segments:\n"));

    for (i = 0; i < polyline->count; i++)
    {
        LOGOUTPUT(LogLevelHigh, ("\t\t[%d] \t%f,\t%f\n", i, polyline->points[i].mx, polyline->points[i].my));
    }


    LOGOUTPUT(LogLevelHigh, ("\n Foreground TriStrips:\n"));

    for (i = 0; i < triStrip->triStripForeground->count; i++)
    {
        mx = triStrip->triStripForeground->points[i].mx;
        my = triStrip->triStripForeground->points[i].my;
        LOGOUTPUT(LogLevelHigh, ("\t\t[%d] \t%f,\t%f\n", i, mx, my));
    }

    LOGOUTPUT(LogLevelHigh, ("\n Background TriStrips:\n"));

    for (i = 0; i < triStrip->triStripBackground->count; i++)
    {
        mx = triStrip->triStripBackground->points[i].mx;
        my = triStrip->triStripBackground->points[i].my;
        LOGOUTPUT(LogLevelHigh, ("\t\t[%d] \t%f,\t%f\n", i, mx, my));
    }

    LOGOUTPUT(LogLevelHigh, ("\n-------------------------------------------------\n"));
}


/*! @} */
