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

///*!--------------------------------------------------------------------------
//
//    @file       testpinmanager.cpp
//
//    See header file for description.
//*/
///*
//    (C) Copyright 2012 by TeleCommunications Systems, Inc.
//
//    The information contained herein is confidential, proprietary
//    to TeleCommunication Systems, Inc., and considered a trade secret as
//    defined in section 499C of the penal code of the State of
//    California. Use of this information by anyone other than
//    authorized employees of TeleCommunication Systems is granted only
//    under a written non-disclosure agreement, expressly
//    prescribing the scope and manner of such use.
//
//---------------------------------------------------------------------------*/
//
///*! @{ */
//
//extern "C"
//{
//    #include "networkutil.h"
//}
//
//#include "testpinmanager.h"
//#include "PinManagerImpl.h"
//#include "PinServices.h"
//#include "MapServicesConfiguration.h"
//
//using namespace std;
//using namespace nbmap;
//
//// Local Constants ..............................................................................
//
//const uint32 TEST_PIN_MANAGER_NBGM_VERSION = 23;
//const char* TEST_LAYER_ID[] = {
//    "PinLayer1",
//    "PinLayer2",
//    "PinLayer3",
//    "PinLayer4",
//    NULL
//};
//
//
//// Local Types ..................................................................................
//
///*! Collection of all instances used for the unit tests */
//class TestPinManagerInstances : public Base
//{
//public:
//    /* See definition for description */
//
//    TestPinManagerInstances();
//    virtual ~TestPinManagerInstances();
//
//    bool IsInitialized() const;
//
//    PAL_Instance* m_pal;                            /*!< A PAL instance */
//    NB_Context* m_context;                          /*!< Pointer to current context */
//    shared_ptr<PinManager<string> > m_pinManager;   /*!< A pin manager */
//};
//
//
//
//
//// Helper functions .............................................................................
//
//
//// TestPinManagerInstances functions ............................................................
//
///*! TestPinManagerInstances constructor */
//TestPinManagerInstances::TestPinManagerInstances()
//: Base(),
//  m_pal(NULL),
//  m_context(NULL),
//  m_pinManager()
//{
//    // Create a PAL instance and NB Context.
//    uint8 result = CreatePalAndContext(&m_pal, &m_context);
//    if ((result == 0) || (!m_pal) || (!m_context))
//    {
//        CU_FAIL("Creating pal or contest failed");
//        return;
//    }
//
//    // Initialize a pin manager.
//    MapServicesConfiguration configuration(TEST_PIN_MANAGER_NBGM_VERSION);
//    LayerManagerPtr layerManager = configuration.GetLayerManager(m_context);
//    if (!layerManager)
//    {
//        CU_FAIL("MapServicesConfiguration::GetLayerManager failed");
//        return;
//    }
//    m_pinManager = PinServices<string>::GetPinManager(m_context, layerManager);
//    if (!m_pinManager)
//    {
//        CU_FAIL("MapServicesConfiguration::GetPinManager failed");
//        return;
//    }
//}
//
///*! TestPinManagerInstances destructor */
//TestPinManagerInstances::~TestPinManagerInstances()
//{
//    m_pinManager.reset();
//
//    if (m_context)
//    {
//        NB_Error error = NE_OK;
//
//        error = NB_ContextDestroy(m_context);
//        CU_ASSERT_EQUAL(error, NE_OK);
//        m_context = NULL;
//    }
//
//    if (m_pal)
//    {
//        PAL_Destroy(m_pal);
//        m_pal = NULL;
//    }
//}
//
///*! Check if this object is initialized
//
//    @return Flag if this object is initialized
//*/
//bool
//TestPinManagerInstances::IsInitialized() const
//{
//    if (m_pal && m_context && m_pinManager)
//    {
//        return true;
//    }
//
//    return false;
//}
//
//// Test Functions ...............................................................................
//
///*! Test initializing a pin manager
//
//  @return None. CUnit Asserts get called on failures.
//*/
//void
//TestPinManagerInitialization(void)
//{
//    // Initialize a pin manager in constructor.
//    TestPinManagerInstances instances;
//}
//
//void
//TestPinManagerAddPinLayer()
//{
//    TestPinManagerInstances instance;
//    shared_ptr<string> layerId;
//    const char* id = NULL;
//    int         i  = 0;
//    while ((id = TEST_LAYER_ID[i++]) != NULL)
//    {
//        layerId = shared_ptr<string>(new string(id));
//        if (!layerId)
//        {
//            CU_FAIL("No mem.");
//        }
//        else
//        {
//            LayerPtr layer = instance.m_pinManager->AddPinLayer(layerId);
//            if (!layer)
//            {
//                CU_FAIL("PinManager::AddPinLayer() failed.");
//            }
//        }
//    }
//}
//
//void
//TestPinManagerRemovePinLayer()
//{
//    TestPinManagerInstances instance;
//    shared_ptr<string> layerId;
//    const char* id = NULL;
//    int         i  = 0;
//    while ((id = TEST_LAYER_ID[i++]) != NULL)
//    {
//        layerId = shared_ptr<string>(new string(id));
//        if (!layerId)
//        {
//            CU_FAIL("No mem.");
//        }
//        else
//        {
//            LayerPtr layer = instance.m_pinManager->AddPinLayer(layerId);
//            if (!layer)
//            {
//                CU_FAIL("PinManager::AddPinLayer() failed.");
//            }
//        }
//    }
//
//    // Get a PinLayer from PinManager.
//    id = TEST_LAYER_ID[1];
//    layerId = shared_ptr<string>(new string(id));
//    shared_ptr<PinLayer<string> > layer = instance.m_pinManager->AddPinLayer(layerId);
//    if (!layer)
//    {
//        CU_FAIL("PinManager::AddPinLayer() failed.");
//    }
//    else //@todo: RemovePinLayer() has no return value.
//    {
//        instance.m_pinManager->RemovePinLayer(layer);
//    }
//}
//
//
//// Public Functions .............................................................................
//
///*! Add all your test functions here
//
//  @return None
//*/
//void
//TestPinManager_AddAllTests(CU_pSuite pTestSuite)
//{
//    // ! Add all your function names here !
//    CU_add_test(pTestSuite, "TestPinManagerInitialization", &TestPinManagerInitialization);
//    // CU_add_test(pTestSuite, "TestPinManagerAddPinLayer", &TestPinManagerAddPinLayer);
//    CU_add_test(pTestSuite, "TestPinManagerRemovePinLayer", &TestPinManagerRemovePinLayer);
//};
//
///*! Add common initialization code here
//
//  @return 0
//*/
//int
//TestPinManager_SuiteSetup()
//{
//    return 0;
//}
//
///*! Add common cleanup code here
//
//  @return 0
//*/
//int
//TestPinManager_SuiteCleanup()
//{
//    return 0;
//}
//
///*! @} */
