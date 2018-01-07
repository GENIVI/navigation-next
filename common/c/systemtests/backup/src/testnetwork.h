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

    @file     TestNetwork.h
    @date     01/22/2009
    @defgroup TestNetwork_h System Tests for Network

    System Tests for Network connection.

    This file contains all system tests for the network functionality in 
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

#ifndef _TEST_NETWORK_H_
#define _TEST_NETWORK_H_

#include "CUnit.h"
#include "nbcontext.h"
#include "nbnetwork.h"


/*! The name for the cache is also used as the subfolder for the persistent cache.  */
#define RASTER_TILE_CACHE_NAME      "mapCache"
#define VOICE_CACHE_NAME            "voiceCache"

#define VOICE_CACHE_MEMORY          64
#define VOICE_CACHE_PERSISTENT      64

#define RASTERTILE_CACHE_MEMORY     1000
#define RASTERTILE_CACHE_PERSISTENT 2000


void TestNetwork_AddTests(CU_pSuite pTestSuite, int level );

// Cleanup for suite
int TestNetwork_SuiteSetup(void);
int TestNetwork_SuiteCleanup(void);

// Public functions for all system tests
const char* TestNetwork_GetToken();
const char* TestNetwork_GetDomain();
NB_NetworkProtocol TestNetwork_GetNetProtocol();
uint16 TestNetwork_GetPort();

/*! Create a PAL and NB Context with specific persistent data

@param pal On success, receives a pointer to a PAL_Instance.  A valid object must be destroyed with PAL_DestroyInstance
@param context On success, receives a pointer to a NB_Context.  A valid object must be destroyed with DestroyContext
@returns Non-zero on success; zero on failure
*/
uint8 CreatePalAndContext(PAL_Instance**pal, NB_Context** context);
uint8 CreatePalAndContextMultiThread(PAL_Instance** pal, NB_Context** context);

/*! Destroy NB Context and related objects

@param context NB_Context
*/
void DestroyContext(NB_Context* context);

#endif // _TEST_NETWORK_H_


/*! @} */
