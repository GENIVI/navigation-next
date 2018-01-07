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

    @file     testgpstypes.c
    @date     06/07/2009
    @defgroup TestGPSTypes_h System Tests for GPS Types

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

#include "testgpstypes.h"
#include "main.h"
#include "abpalgps.h"
#include "nbgpstypes.h"

void TestGPSTypesMatch(void);

/*! Add all your test functions here

@return None
*/
void
TestGPSTypes_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestGPSTypesMatch", TestGPSTypesMatch);
};


/*! Add common initialization code here.

@return 0

@see TestNetwork_SuiteCleanup
*/
int
TestGPSTypes_SuiteSetup()
{
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNetwork_SuiteSetup
*/
int
TestGPSTypes_SuiteCleanup()
{
	return 0;
}

/*! @brief Test to make sure that the ABPAL and NBSERVICES GPS types (which need to be binary compatible) actually match.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestGPSTypesMatch(void)
{
    ABPAL_GpsLocation abGPSLocation = {0};
    NB_GpsLocation    nbGPSLocation = {0};

    abGPSLocation.status = 0x12345678;
    abGPSLocation.gpsTime = 0x23456781;
    abGPSLocation.valid = 0x23456789;
    abGPSLocation.latitude = 33.123456;
    abGPSLocation.longitude = -118.3234523;
    abGPSLocation.heading = 89.2343;
    abGPSLocation.horizontalVelocity = 34.54;
    abGPSLocation.altitude = 45.4321;
    abGPSLocation.verticalVelocity = -1.234;
    abGPSLocation.horizontalUncertaintyAngleOfAxis = 345.9;
    abGPSLocation.horizontalUncertaintyAlongAxis = 132.23;
    abGPSLocation.horizontalUncertaintyAlongPerpendicular = 234.0;
    abGPSLocation.verticalUncertainty = 300.0;
    abGPSLocation.utcOffset = -14;
    abGPSLocation.numberOfSatellites = 5; 

    CU_ASSERT_EQUAL(sizeof(abGPSLocation), sizeof(nbGPSLocation));

    nsl_memcpy(&nbGPSLocation, &abGPSLocation, sizeof(nbGPSLocation));

    CU_ASSERT_EQUAL(abGPSLocation.status, nbGPSLocation.status);
    CU_ASSERT_EQUAL(abGPSLocation.gpsTime, nbGPSLocation.gpsTime);
    CU_ASSERT_EQUAL(abGPSLocation.valid, nbGPSLocation.valid);
    CU_ASSERT_EQUAL(abGPSLocation.latitude, nbGPSLocation.latitude);
    CU_ASSERT_EQUAL(abGPSLocation.longitude, nbGPSLocation.longitude);
    CU_ASSERT_EQUAL(abGPSLocation.heading, nbGPSLocation.heading);
    CU_ASSERT_EQUAL(abGPSLocation.horizontalVelocity, nbGPSLocation.horizontalVelocity);
    CU_ASSERT_EQUAL(abGPSLocation.altitude, nbGPSLocation.altitude);
    CU_ASSERT_EQUAL(abGPSLocation.verticalVelocity, nbGPSLocation.verticalVelocity);
    CU_ASSERT_EQUAL(abGPSLocation.horizontalUncertaintyAngleOfAxis, nbGPSLocation.horizontalUncertaintyAngleOfAxis);
    CU_ASSERT_EQUAL(abGPSLocation.horizontalUncertaintyAlongAxis, nbGPSLocation.horizontalUncertaintyAlongAxis);
    CU_ASSERT_EQUAL(abGPSLocation.horizontalUncertaintyAlongPerpendicular, nbGPSLocation.horizontalUncertaintyAlongPerpendicular);
    CU_ASSERT_EQUAL(abGPSLocation.verticalUncertainty, nbGPSLocation.verticalUncertainty);
    CU_ASSERT_EQUAL(abGPSLocation.utcOffset, nbGPSLocation.utcOffset);
    CU_ASSERT_EQUAL(abGPSLocation.numberOfSatellites, nbGPSLocation.numberOfSatellites);

    CU_ASSERT_EQUAL(PGV_None, NGV_None);
    CU_ASSERT_EQUAL(PGV_Latitude, NGV_Latitude);
    CU_ASSERT_EQUAL(PGV_Longitude, NGV_Longitude);
    CU_ASSERT_EQUAL(PGV_Altitude, NGV_Altitude);
    CU_ASSERT_EQUAL(PGV_Heading, NGV_Heading);
    CU_ASSERT_EQUAL(PGV_HorizontalVelocity, NGV_HorizontalVelocity);
    CU_ASSERT_EQUAL(PGV_VerticalVelocity, NGV_VerticalVelocity);
    CU_ASSERT_EQUAL(PGV_HorizontalUncertainty, NGV_HorizontalUncertainty);
    CU_ASSERT_EQUAL(PGV_AxisUncertainty, NGV_AxisUncertainty);
    CU_ASSERT_EQUAL(PGV_PerpendicularUncertainty, NGV_PerpendicularUncertainty);
    CU_ASSERT_EQUAL(PGV_VerticalUncertainty, NGV_VerticalUncertainty);
    CU_ASSERT_EQUAL(PGV_UTCOffset, NGV_UTCOffset);
    CU_ASSERT_EQUAL(PGV_SatelliteCount, NGV_SatelliteCount);
}

/*! @} */
