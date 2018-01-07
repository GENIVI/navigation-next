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

    @file     TestNavigation.h
    @date     02/03/2009
    @defgroup TestNavigation_h System tests for Navigation functions

    System tests for Navigation functions

    This file contains all system tests for the Navigation functionality in 
    NAVBuilder Services SDK.
*/
/*
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

#ifndef _TEST_NAVIGATION_H_
#define _TEST_NAVIGATION_H_

#include "CUnit.h"
#include "main.h"
#include "palclock.h"
#include "palfile.h"
#include "nbcontext.h"
#include "nbrouteparameters.h"
#include "nbguidanceinformation.h"
#include "nbnavigation.h"

typedef struct
{
    nb_boolean  showPlaces;
    nb_boolean  showCameras;
    uint32      manualTrafficPollIntervalMinutes;
} TestNavigationSessionOptions;

void TestNavigation_AddTests(CU_pSuite pTestSuite, int level);

// Cleanup for suite
int TestNavigation_SuiteSetup(void);
int TestNavigation_SuiteCleanup(void);


typedef void (NB_NavigationSessionTestCallback) (PAL_Instance* pal, NB_Context* context, NB_Navigation* navigation, NB_GuidanceInformation* guidance, NB_GpsLocation* gpsFix, NB_NavigationPublicState* publicState, void* userData);

/*! Function for creating and executing testnavigationsession. Use in testspeedcameras.
*/
void NavigationTestNavigationSession(NB_Place* destination, NB_RouteOptions* routeOptions, NB_RouteConfiguration* routeConfiguration, 
                                const char* gpsFileName, int fixDelayMilliseconds, uint32 startTime, uint32 endTime, NB_NavigationSessionTestCallback* testCallback, void* testCallbackUserData,
                                const char* testName, const char* kmlFileName, TestNavigationSessionOptions* sessionOptions);

/*! Setup initial parameters for routeConfiguration structure
*/
void SetupRouteConfigurationVehicle(NB_RouteConfiguration* routeConfiguration);

#endif //_TEST_NAVIGATION_H_

/*! @} */

