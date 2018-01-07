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

    @file       testpersistentdata.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testpersistentdata.h"
#include "platformutil.h"
#include "networkutil.h"
#include "nbpersistentdataprotected.h"

// Local Constants ..............................................................................

static const char TEST_PARENT_TPS_ELEMENT_NAME[] = "parent-tps-element";
static const char TEST_PARENT_ATTRIBUTE_NAME[] = "parent-attribute-name";
static const uint32 TEST_PARENT_ATTRIBUTE_DATA = 88;
static const char TEST_CHILD_TPS_ELEMENT_NAME[] = "child-tps-element";
static const char TEST_CHILD_ATTRIBUTE_NAME[] = "child-attribute-name";
static const char TEST_CHILD_ATTRIBUTE_DATA[] = "child-attribute-value";
static const char TEST_EMPTY_ATTRIBUTE_NAME[] = "empty-attribute";
static const char TEST_EMPTY_ATTRIBUTE_DATA[] = "";


// Local Types ..................................................................................

/*! Collection of all instances used for the unit tests */
typedef struct
{
    PAL_Instance* pal;  /*!< Pointer to a PAL instance */
    NB_Context* context;/*!< Pointer to current context */
    void* event;        /*!< Callback completed event */
} TestPersistentDataInstances;


// Test Functions ...............................................................................

static void TestPersistentDataSerializeTpsElement(void);


// Helper functions .............................................................................

static nb_boolean CreateInstances(TestPersistentDataInstances* instances);
static void DestroyInstances(TestPersistentDataInstances* instances);

static tpselt CreateTestTpsElement();
static nb_boolean IsTestTpsElementValid(tpselt tpsElement);


// Public Functions .............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestPersistentData_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestPersistentDataSerializeTpsElement", &TestPersistentDataSerializeTpsElement);
};

/*! Add common initialization code here

    @return 0
*/
int
TestPersistentData_SuiteSetup(void)
{
    return 0;
}


/*! Add common cleanup code here

    @return 0
*/
int
TestPersistentData_SuiteCleanup(void)
{
    return 0;
}


// Test Functions ...............................................................................

/*! Test serializing and deserializing a TPS element

    @return None. CUnit Asserts get called on failures.
*/
void
TestPersistentDataSerializeTpsElement(void)
{
    NB_Error error = NE_OK;
    nb_size dataSize = 0;
    uint8* data = NULL;
    tpselt tpsElement = NULL;
    NB_PersistentData* persistentData = NULL;
    TestPersistentDataInstances instances = {0};

    if (!CreateInstances(&instances))
    {
        return;
    }

    // Create a TPS element for testing.
    tpsElement = CreateTestTpsElement();
    if (!tpsElement)
    {
        DestroyInstances(&instances);
        return;
    }

    // Create a persistent data instance by a TPS element.
    error = NB_PersistentDataCreateByTpsElement(tpsElement, &persistentData);
    te_dealloc(tpsElement);
    tpsElement = NULL;
    if (error != NE_OK)
    {
        DestroyInstances(&instances);
        CU_FAIL("Calling NB_PersistentDataCreateByTpsElement failed");
        return;
    }

    // Serialize the data.
    error = NB_PersistentDataSerialize(persistentData, (const uint8**) (&data), &dataSize);
    NB_PersistentDataDestroy(persistentData);
    persistentData = NULL;
    if (error != NE_OK)
    {
        DestroyInstances(&instances);
        CU_FAIL("Calling NB_PersistentDataSerialize failed");
        return;
    }

    // Create a new persistent data instance by the data.
    error = NB_PersistentDataCreate(data, dataSize, NULL, &persistentData);
    nsl_free(data);
    data = NULL;
    dataSize = 0;
    if (error != NE_OK)
    {
        DestroyInstances(&instances);
        CU_FAIL("Calling NB_PersistentDataCreate failed");
        return;
    }

    // Get the data of the persistent data instance to a TPS element.
    error = NB_PersistentDataGetToTpsElement(persistentData, &tpsElement);
    NB_PersistentDataDestroy(persistentData);
    persistentData = NULL;
    if (error != NE_OK)
    {
        DestroyInstances(&instances);
        CU_FAIL("Calling NB_PersistentDataGetToTpsElement failed");
        return;
    }

    // Compare two TPS elements.
    if (!IsTestTpsElementValid(tpsElement))
    {
        te_dealloc(tpsElement);
        tpsElement = NULL;
        DestroyInstances(&instances);
        return;
    }

    te_dealloc(tpsElement);
    tpsElement = NULL;
    DestroyInstances(&instances);
}


// Helper functions .............................................................................

/*! Create a PAL, context and callback completed event.

    Shared by all unit tests.

    @return TRUE on success, FALSE otherwise.
*/
nb_boolean
CreateInstances(TestPersistentDataInstances* instances)
{
    if (!CreatePalAndContext(&instances->pal, &instances->context))
    {
        CU_FAIL("Creating PAL or context failed");
        DestroyInstances(instances);
        return FALSE;
    }

    instances->event = CreateCallbackCompletedEvent();
    if (!instances->event)
    {
        CU_FAIL("Creating callback completed event failed");
        DestroyInstances(instances);
        return FALSE;
    }

    return TRUE;
}

/*! Destroy the PAL, context and callback completed event.

    Shared by all unit tests.

    @return None
*/
void
DestroyInstances(TestPersistentDataInstances* instances)
{
    NB_Error error = NE_OK;

    if (instances->context)
    {
        error = NB_ContextDestroy(instances->context);
        instances->context = NULL;
        CU_ASSERT_EQUAL(error, NE_OK);
    }

    if (instances->pal)
    {
        PAL_Destroy(instances->pal);
        instances->pal = NULL;
    }

    if (instances->event)
    {
        DestroyCallbackCompletedEvent(instances->event);
        instances->event = NULL;
    }
}

/*! Create a TPS element for testing

    @return A TPS element
*/
tpselt
CreateTestTpsElement()
{
    tpselt parentTpsElement = NULL;
    tpselt childTpsElement = NULL;

    childTpsElement = te_new(TEST_CHILD_TPS_ELEMENT_NAME);
    if (!childTpsElement)
    {
        CU_FAIL("Creating child TPS element failed");
        return NULL;
    }

    if (te_setattrc(childTpsElement, TEST_CHILD_ATTRIBUTE_NAME, TEST_CHILD_ATTRIBUTE_DATA) == 0)
    {
        te_dealloc(childTpsElement);
        childTpsElement = NULL;
        CU_FAIL("Setting a valid attribute to the child TPS element failed");
        return NULL;
    }

    if (te_setattrc(childTpsElement, TEST_EMPTY_ATTRIBUTE_NAME, TEST_EMPTY_ATTRIBUTE_DATA) == 0)
    {
        te_dealloc(childTpsElement);
        childTpsElement = NULL;
        CU_FAIL("Setting an empty attribute to the child TPS element failed");
        return NULL;
    }

    parentTpsElement = te_new(TEST_PARENT_TPS_ELEMENT_NAME);
    if (!childTpsElement)
    {
        te_dealloc(childTpsElement);
        childTpsElement = NULL;
        CU_FAIL("Creating parent TPS element failed");
        return NULL;
    }

    if (te_attach(parentTpsElement, childTpsElement) == 0)
    {
        te_dealloc(childTpsElement);
        childTpsElement = NULL;
        te_dealloc(parentTpsElement);
        parentTpsElement = NULL;
        CU_FAIL("Attaching parent and child TPS elements failed");
        return NULL;
    }

    if (te_setattru(parentTpsElement, TEST_PARENT_ATTRIBUTE_NAME, TEST_PARENT_ATTRIBUTE_DATA) == 0)
    {
        te_dealloc(parentTpsElement);
        parentTpsElement = NULL;
        CU_FAIL("Setting a valid attribute to the parent TPS element failed");
        return NULL;
    }

    return parentTpsElement;
}

/*! Check if the test TPS element is valid

    @return TRUE if the test TPS element is valid, FALSE otherwise.
*/
nb_boolean
IsTestTpsElementValid(tpselt tpsElement)
{
    int iterator = 0;
    uint32 parentAttributeData = 0;
    const char* childAttributeData = NULL;
    const char* emptyAttributeData = NULL;
    tpselt childTpsElement = NULL;

    if (!tpsElement)
    {
        CU_FAIL("The TPS element is NULL");
        return FALSE;
    }

    parentAttributeData = te_getattru(tpsElement, TEST_PARENT_ATTRIBUTE_NAME);
    if (parentAttributeData != TEST_PARENT_ATTRIBUTE_DATA)
    {
        CU_FAIL("The parent attribute data is not right");
        return FALSE;
    }

    childTpsElement = te_nextchild(tpsElement, &iterator);
    if (!childTpsElement)
    {
        CU_FAIL("The child TPS element is NULL");
        return FALSE;
    }

    childAttributeData = te_getattrc(childTpsElement, TEST_CHILD_ATTRIBUTE_NAME);
    if ((!childAttributeData) || (nsl_strcmp(childAttributeData, TEST_CHILD_ATTRIBUTE_DATA) != 0))
    {
        CU_FAIL("The child attribute data is not right");
        return FALSE;
    }

    emptyAttributeData = te_getattrc(childTpsElement, TEST_EMPTY_ATTRIBUTE_NAME);
    if ((!emptyAttributeData) || (nsl_strcmp(emptyAttributeData, TEST_EMPTY_ATTRIBUTE_DATA) != 0))
    {
        CU_FAIL("The empty attribute data is not right");
        return FALSE;
    }

    childTpsElement = te_nextchild(tpsElement, &iterator);
    if (childTpsElement)
    {
        CU_FAIL("The count of children TPS elements are not right");
        return FALSE;
    }

    return TRUE;
}

/*! @} */
