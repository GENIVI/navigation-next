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

@file     Main.h
@date     01/20/2009
@defgroup SYSTEMTEST_H System Tests for NIM NAVBuilder Services

   Global constants for system tests

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

#include "platformutil.h"
#include "argtableutil.h"


// Macros and Constants ..........................................................................


#define LOGOUTPUT(severity, msg)                    \
    do {                                            \
    if (Test_OptionsGet()->logLevel >= severity)    \
            PRINTF msg;                             \
    } while (0)

#define ADD_TEST(level, testlevel, pTestSuite, methodstr, methodbname)                                        \
    do {                                                                                                      \
        if (level >= testlevel) {                                                                             \
            if (Test_OptionsGet()->startFromParticularTest) {                                                 \
                if (pTestSuite->uiNumberOfTests > 0 || !nsl_strcmp(Test_OptionsGet()->testName, methodstr)) { \
                    CU_add_test(pTestSuite, methodstr, &methodbname);                                         \
                }                                                                                             \
            }                                                                                                 \
            else {                                                                                            \
                CU_add_test(pTestSuite, methodstr, &methodbname);                                             \
            }                                                                                                 \
        }                                                                                                     \
    } while (0)

// Use the path delimiter to create directories for testing
#if WINCE || WIN32
#define PATH_DELIMITER "\\"
#else
#define PATH_DELIMITER "/"
#endif


/*! Method declarations */
#ifdef __cplusplus
extern "C"
	{
#endif

int test_main(int argc, char* argv[]);

#ifdef __cplusplus
	}
#endif

/*! @} */
