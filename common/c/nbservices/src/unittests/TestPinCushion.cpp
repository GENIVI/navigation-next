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

// /*!--------------------------------------------------------------------------
//   @file        TestPinCushion.cpp
//   @defgroup    test
//
//   Description: Refer to header file for more description.
//
//*/
///*
//   (C) Copyright 2012 by TeleCommunications Systems, Inc.
//
//   The information contained herein is confidential, proprietary to
//   TeleCommunication Systems, Inc., and considered a trade secret as defined
//   in section 499C of the penal code of the State of California. Use of this
//   information by anyone other than authorized employees of Networks In
//   Motion is granted only under a written non-disclosure agreement, expressly
//   prescribing the scope and manner of such use.
//
// --------------------------------------------------------------------------*/
///*! @{ */
//
//extern "C"
//{
//#include "nbcontext.h"
//#include "networkutil.h"
//#include "platformutil.h"
//}
//
//#include "TestPinCushion.h"
//#include "MapServicesConfiguration.h"
//#include "PinCushion.h"
//#include "PinManager.h"
//#include "PinCushionImpl.h"
//#include "PinServices.h"
//
//#define TEST_LATITUDE   36.10
//#define TEST_LONGITUDE  -115.12
//#define TEST_LATITUDE2  37.775239
//#define TEST_LONGITUDE2 -122.419170
//
//#define TEST_PIN_MANAGER_NBGM_VERSION 23
//
//using namespace std;
//using namespace nbmap;
//
///*! Listener of PinLayer, refer to base class for more description. */
//class PinLayerListener : public Base,
//                         public LayerManagerListener
//{
//public:
//    PinLayerListener(void* event)
//    {
//        m_event   = event;
//    }
//
//    virtual ~PinLayerListener()
//    {
//    }
//
//    virtual void RefreshPins()
//    {
//        if (m_event)
//        {
//            SetCallbackCompletedEvent(m_event);
//        }
//    }
//
//    virtual void RefreshTiles()
//    {
//    }
//
//private:
//    void* m_event;
//};
//
//typedef shared_ptr<PinLayerListener>   PinLayerListenerPtr;
//
//class TestPinCushionInstance
//{
//public:
//
//    /*! Default constructor. */
//    TestPinCushionInstance();
//
//    /*! Default destructor. */
//    virtual ~TestPinCushionInstance();
//
//    /*! Initializes this test instance.
//
//      @return NB_OK if things go well.
//    */
//    NB_Error Initialize();
//
//    /*! Test to drop a pin.
//
//       @return NE_OK if succeeded.
//    */
//    NB_Error TestDropPin();
//
//    /*! Test to Move Pin to other location.
//
//       @return NE_OK if succeeded.
//    */
//    NB_Error TestMovePin();
//
//    /*! Test to remove a pin.
//
//        @return NE_OK if succeeded.
//    */
//    NB_Error TestRemovePin();
//
//    /*! Helper function to release allocated resources.
//
//      @return void
//    */
//    void Reset();
//
//    // Variables.
//    PAL_Instance* m_pPal;
//    NB_Context*   m_pContext;
//    void*         m_event;
//
//    shared_ptr<PinCushion<string> > m_pPinCushion;
//    PinLayerListener* m_pListener;
//};
//
//TestPinCushionInstance::TestPinCushionInstance()
//{
//    m_pPal     = NULL;
//    m_pContext = NULL;
//    m_event    = NULL;
//}
//
//TestPinCushionInstance::~TestPinCushionInstance()
//{
//    Reset();
//}
//
//NB_Error TestPinCushionInstance::Initialize()
//{
//    // Create a PAL instance and NB Context.
//    uint8 result = CreatePalAndContext(&m_pPal, &m_pContext);
//    if ((result == 0) || (!m_pPal) || (!m_pContext))
//    {
//        CU_FAIL("Creating pal or contest failed");
//        return NE_NOTINIT;
//    }
//
//    m_event = CreateCallbackCompletedEvent();
//    CU_ASSERT_PTR_NOT_NULL(m_event);
//    if (!m_event)
//    {
//        Reset();
//        return NE_NOTINIT;
//    }
//
//    // Initialize a pin manager.
//    MapServicesConfiguration configuration(TEST_PIN_MANAGER_NBGM_VERSION);
//    LayerManagerPtr layerManager = configuration.GetLayerManager(m_pContext);
//    if (!layerManager)
//    {
//        CU_FAIL("Failed to create LayerManager.");
//        Reset();
//        return NE_NOTINIT;
//    }
//
//    m_pListener = new PinLayerListener(m_event);
//    if (!m_pListener)
//    {
//        Reset();
//        CU_FAIL("Failed to create listener.");
//        return NE_NOTINIT;
//    }
//    layerManager->RegisterListener(m_pListener);
//
//    m_pPinCushion = PinServices<string>::GetPinCushion(m_pContext, layerManager);
//    if (!m_pPinCushion)
//    {
//        CU_FAIL("MapServicesConfiguration::GetPinCushion failed");
//        Reset();
//        return NE_NOTINIT;
//    }
//
//    return NE_OK;
//}
//
//NB_Error TestPinCushionInstance::TestDropPin()
//{
//    NB_Error error = NE_OK;
//    shared_ptr<Pin<string> > pin = m_pPinCushion->DropPin(TEST_LATITUDE, TEST_LONGITUDE);
//    if (pin)
//    {
//        error = NE_OK;
//    }
//    else
//    {
//        CU_FAIL("Failed to DropPin()");
//    }
//    return error;
//}
//
//NB_Error TestPinCushionInstance::TestMovePin()
//{
//    NB_Error error = NE_OK;
//    shared_ptr<Pin<string> > pin = m_pPinCushion->DropPin(TEST_LATITUDE, TEST_LONGITUDE);
//    if (pin)
//    {
//        shared_ptr<string> pinID = pin->GetPinID();
//        if (pinID)
//        {
//            error = m_pPinCushion->MovePin(pinID, TEST_LATITUDE2, TEST_LONGITUDE2);
//            CU_ASSERT_EQUAL(error, NE_OK);
//        }
//        else
//        {
//            CU_FAIL("Pin::GetPinID() failed.");
//            error = NE_RES;
//        }
//    }
//    else
//    {
//        CU_FAIL("Failed to DropPin()");
//    }
//    return error;
//}
//
//NB_Error TestPinCushionInstance::TestRemovePin()
//{
//    NB_Error error = NE_OK;
//    shared_ptr<Pin<string> > pin = m_pPinCushion->DropPin(TEST_LATITUDE, TEST_LONGITUDE);
//    if (pin)
//    {
//        shared_ptr<string> pinID = pin->GetPinID();
//        if (pinID)
//        {
//            error = m_pPinCushion->RemovePin(pinID);
//            CU_ASSERT_EQUAL(error, NE_OK);
//        }
//        else
//        {
//            CU_FAIL("Pin::GetPinID() failed.");
//            error = NE_RES;
//        }
//    }
//    else
//    {
//        CU_FAIL("Failed to DropPin()");
//        error = NE_RES;
//    }
//    return error;
//}
//
//void TestPinCushionInstance::Reset()
//{
//    if (m_event)
//    {
//        DestroyCallbackCompletedEvent(m_event);
//        m_event = NULL;
//    }
//
//    if (m_pPinCushion)
//    {
//        m_pPinCushion.reset();
//    }
//
//    if (m_pContext)
//    {
//        NB_Error error = NE_OK;
//
//        error = NB_ContextDestroy(m_pContext);
//        CU_ASSERT_EQUAL(error, NE_OK);
//        m_pContext = NULL;
//    }
//
//    if (m_pPal)
//    {
//        PAL_Destroy(m_pPal);
//        m_pPal = NULL;
//    }
//
//    if (m_pListener)
//    {
//        delete m_pListener;
//    }
//}
//
//
//// Test Functions
//
//void
//TestPinCushionInitialization()
//{
//    TestPinCushionInstance instance;
//    NB_Error error = instance.Initialize();
//    CU_ASSERT_EQUAL(error, NE_OK);
//}
//
//void TestPinCushionDropPin()
//{
//    TestPinCushionInstance instance;
//    NB_Error error = instance.Initialize();
//    CU_ASSERT_EQUAL(error, NE_OK);
//    error = instance.TestDropPin();
//    CU_ASSERT_EQUAL(error, NE_OK);
//}
//
//void TestPinCushionMovePin()
//{
//    TestPinCushionInstance instance;
//    NB_Error error = instance.Initialize();
//    CU_ASSERT_EQUAL(error, NE_OK);
//    error = instance.TestMovePin();
//    CU_ASSERT_EQUAL(error, NE_OK);
//}
//
//void TestPinCushionRemovePin()
//{
//    TestPinCushionInstance instance;
//    NB_Error error = instance.Initialize();
//    CU_ASSERT_EQUAL(error, NE_OK);
//    error = instance.TestRemovePin();
//    CU_ASSERT_EQUAL(error, NE_OK);
//}
//
//void TestPinCushion_AddAllTests(CU_pSuite pTestSuite)
//{
//    CU_add_test(pTestSuite, "TestPinCushionInitialization", &TestPinCushionInitialization);
//    CU_add_test(pTestSuite, "TestPinCushionDropPin", &TestPinCushionDropPin);
//    CU_add_test(pTestSuite, "TestPinCushionMovePin", &TestPinCushionMovePin);
//    CU_add_test(pTestSuite, "TestPinCushionRemovePin", &TestPinCushionRemovePin);
//}
//
//// Cleanup for suite
//int TestPinCushion_SuiteSetup(void)
//{
//    return 0;
//}
//
//int TestPinCushion_SuiteCleanup(void)
//{
//    return 0;
//}
//
//
///*! @} */
