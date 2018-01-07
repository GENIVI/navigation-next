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

@file     TestSms.c
@date     04/01/2009
@defgroup TestSms_h System tests for AB_Sms functions

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

#include "testsms.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"

#include "absms.h"


// Local Functions ...........................................................

// All tests. Add all tests to the TestSms_AddAllTests function
static void TestSms1(void);
static void TestValidateHMAC(void);


// Subfunctions of tests


// Constants .................................................................

static const char* SMS_KEY = "AykaNSVCEcrnhiqacLY+F5aeisd7pmiATyTGMuRS";

static const char* SMS_TEST_SMS_COMMAND_MESSAGES[] = {
    "005646690d1fd5afe23ceEBSWHdqb3p8Wg4jFBkdAAApMEUkWhkZUEJcAEJZXXRgBWB1MUFgLw==",
    "015e89f1c293fce70bc3dk5YU3dkbnZ2UQ4jFBkdAAApMEUkWhkZUEJRAkVVWHJmBWd1MUZgLw==",
    "07f51993174f2e480b2ceEBSWHdqb3p8Wg4jFBkdAAApMEUkWhkZUEJcAEJYUHhhBWBwMUFlLw==",
    "08b5136d34e5268fe62feEBSWHdqb3p8Wg4jFBkdAAApMEUkWhkZUEJcAENfUHlsBWFyMUBnLw==",
    
    "90eaf27b80344f161561PTcXVnlgYXZ8WkJcXkdAU1B/bB9wAlVcR0NSBkJDWHNnTWFxek18fG43IiwAEgB5aw0bAwYIBxYOMA==",
    "401ebe61e200402f7079PTcXVX5rZXJ1VUVaX0dAU1B1bxx3AFJUR0pRAURDWHFsTmBzfVsQAQQuNDIDAHguIBccDx4V",
    "1832c067d3557c624401PTcXVX5rZXJ1VUdeUUdAU1B1bxx0DVRXR0BWBURDWHFsTmBzfVsQAQQuNDIDAHguIBccDx4V",
    "1466c0258afe4dce6cc6PTcXVX5rZXJ1VUVaX0dAU1B1bxx1BFNQR0dWBkZDWHFsTmBzfVsQAQQuNDIDAHguIBccDx4V",
};    


// Variables .................................................................


/*! Add all your test functions here

@return None
*/
void 
TestSms_AddTests( CU_pSuite pTestSuite, int level)
{
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestSms1", TestSms1);
	ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestValidateHMAC", TestValidateHMAC);
};


/*! Add common initialization code here.

@return 0

@see TestSms_SuiteCleanup
*/
int 
TestSms_SuiteSetup()
{
	return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestSms_SuiteSetup
*/
int 
TestSms_SuiteCleanup()
{
	return 0;
}

void TestSms1(void)
{
    AB_Sms* sms = NULL;
    NB_Error result = NE_OK;

    uint16 commandCount = 0;
    uint16 commandIndex = 0;

    result = AB_SmsCreate(&sms, (byte*)SMS_KEY, nsl_strlen(SMS_KEY) + 1);
    CU_ASSERT_EQUAL(result, NE_OK);
    if (result == NE_OK)
    {
        int n = 0;
#ifdef SYSTEM_TEST_VERBOSE_HIGH
        printf("\nTesting sample input SMS messages...\n");
#endif
        for (n = 0; n < sizeof(SMS_TEST_SMS_COMMAND_MESSAGES) / sizeof(SMS_TEST_SMS_COMMAND_MESSAGES[0]); n++)
        {
            result = AB_SmsDecodeMessage(sms, SMS_TEST_SMS_COMMAND_MESSAGES[n], &commandCount);
            CU_ASSERT_EQUAL(result, NE_OK);
            if (result != NE_OK)
            {
                continue;
            }

            for (commandIndex = 0; commandIndex < commandCount; commandIndex++)
            {
                AB_SmsCommandType commandType = AB_SmsGetCommandType(sms, commandIndex);
                CU_ASSERT_NOT_EQUAL(commandType, ASCT_Unknown);
                
                switch (commandType)
                {
                    case ASCT_InboundMessage:
                    {
                        AB_SmsCommandInboundMessage data;
                        result = AB_SmsGetInboundMessageCommandData(sms, commandIndex, &data);
                        CU_ASSERT_EQUAL(result, NE_OK);
#ifdef SYSTEM_TEST_VERBOSE_HIGH
                        printf("ASCT_InboundMessage: from '%s' fromname '%s', num %lu, generation %lu, time %lu\n",
                            data.msgFrom,
                            data.msgFromName,
                            data.msgGeneration,
                            data.msgNum,
                            data.msgTime
                        );
#endif
                        break;
                    }
                    case ASCT_MessageReceipt:
                    {
                        AB_SmsCommandMessageReceipt data;
                        result = AB_SmsGetMessageReceiptCommandData(sms, commandIndex, &data);
                        CU_ASSERT_EQUAL(result, NE_OK);
#ifdef SYSTEM_TEST_VERBOSE_HIGH
                        printf("ASCT_MessageReceipt: from '%s' fromname '%s', num %lu, generation %lu, time %lu\n",
                            data.msgFrom,
                            data.msgId,
                            data.msgResult
                        );
#endif
                        break;
                    }
                    case ASCT_SynchronizeDatabase:
                    {
                        AB_SmsCommandSynchronizeDatabase data;
                        result = AB_SmsGetSynchronizeDatabaseCommandData(sms, commandIndex, &data);
                        CU_ASSERT_EQUAL(result, NE_OK);
#ifdef SYSTEM_TEST_VERBOSE_HIGH
                        printf("AB_SmsCommandSynchronizeDatabase: from '%s' databases '%s'\n",
                            data.msgFrom,
                            data.syncDatabases
                        );
#endif
                        break;
                    }
                    case ASCT_TrafficSession:
                    {
                        AB_SmsCommandTrafficSession data;
                        result = AB_SmsGetTrafficSessionCommandData(sms, commandIndex, &data);
                        CU_ASSERT_EQUAL(result, NE_OK);
#ifdef SYSTEM_TEST_VERBOSE_HIGH
                        printf("AB_SmsCommandTrafficSession: from '%s' traffic session '%s'\n",
                            data.msgFrom,
                            data.trafficSession
                        );
#endif
                        break;
                    }
                    default:
                        CU_ASSERT_FALSE("unknown SMS command");
                        break;
                }
            }
        }
    }
    result = AB_SmsDestroy(sms);
    CU_ASSERT_EQUAL(result, NE_OK);
}

void TestValidateHMAC(void)
{
    // Test SMS message "//NIMNAV:AtlasBook;;P;I;BOFA;custom poi;cc56e974516871b86d0e"
    CU_ASSERT_EQUAL(AB_SmsValidateHmac((byte*)SMS_KEY, nsl_strlen(SMS_KEY), "AtlasBook;;P;I;BOFA;custom poi", "cc56e974516871b86d0e"), NE_OK);

    // Mismatched message and HMAC
    CU_ASSERT_EQUAL(AB_SmsValidateHmac((byte*)SMS_KEY, nsl_strlen(SMS_KEY), "AtlasBook;;P;I;BOFA;CUSTOM POI", "cc56e974516871b86d0e"), NE_INVAL);

    // Invalid parameters
    CU_ASSERT_EQUAL(AB_SmsValidateHmac((byte*)SMS_KEY, nsl_strlen(SMS_KEY), 0, "cc56e974516871b86d0e"), NE_INVAL);
    CU_ASSERT_EQUAL(AB_SmsValidateHmac((byte*)SMS_KEY, nsl_strlen(SMS_KEY), "AtlasBook;;P;I;BOFA;custom poi", 0), NE_INVAL);
    CU_ASSERT_EQUAL(AB_SmsValidateHmac(NULL, nsl_strlen(SMS_KEY), "AtlasBook;;P;I;BOFA;custom poi", "cc56e974516871b86d0e"), NE_INVAL);
    CU_ASSERT_EQUAL(AB_SmsValidateHmac((byte*)SMS_KEY, 0, "AtlasBook;;P;I;BOFA;custom poi", "cc56e974516871b86d0e"), NE_INVAL);
}


/*! @} */
