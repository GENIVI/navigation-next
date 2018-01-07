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
//    @file       testpinlayer.cpp
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
//    #include "platformutil.h"
//    #include "palfile.h"
//    #include "networkutil.h"
//}
//
//#include "testpinlayer.h"
//#include "PinLayerImpl.h"
//#include "AsyncCallback.h"
//#include "TileManager.h"
//
//using namespace std;
//using namespace nbmap;
//
//// Local Constants ..............................................................................
//
//const uint32 TEST_DATA_SIZE = 4096;
//
//const int TEST_TILE_X = 5900;
//const int TEST_TILE_Y = 12854;
//const int TEST_ZOOM_LEVEL = 15;
//
//const char TEST_LAYER_ID[] = "TestPinLayer";
//
//const char TEST_MATERIAL_CONFIG_FILENAME[] = "poi_material.xml";
//
//const int TEST_PIN_COUNT = 4;
//
//const double TEST_PIN_LATITUDE[TEST_PIN_COUNT] =
//{
//    36.11224,
//    36.11184,
//    36.11210,
//    36.11256
//};
//
//const double TEST_PIN_LONGITUDE[TEST_PIN_COUNT] =
//{
//    -115.17237,
//    -115.17214,
//    -115.17272,
//    -115.17242
//};
//
//const char* TEST_PIN_NAME[TEST_PIN_COUNT] =
//{
//    "name0",
//    "name1",
//    "name2",
//    "name3"
//};
//
//const char* TEST_PIN_ADDRESS[TEST_PIN_COUNT] =
//{
//    "address0",
//    "address1",
//    "address2",
//    "address3"
//};
//
//
//// Local Types ..................................................................................
//
///*! Collection of all instances used for the unit tests */
//class TestPinLayerInstances : public Base
//{
//public:
//    /* See definition for description */
//
//    TestPinLayerInstances();
//    virtual ~TestPinLayerInstances();
//
//    NB_Error AddPins();
//    NB_Error GetTile(int tileX,
//                     int tileY,
//                     int zoomLevel);
//    NB_Error WriteDataToFile(const char* filename,
//                             DataStream& dataStream);
//
//    // Copy constructor and assignment operator are not supported.
//    TestPinLayerInstances(const TestPinLayerInstances& testInstances);
//    TestPinLayerInstances& operator=(const TestPinLayerInstances& testInstances);
//
//    PAL_Instance* m_pal;                        /*!< Pointer to PAL instance */
//    NB_Context* m_context;                      /*!< Pointer to current context */
//    NB_Error m_error;                           /*!< An error of request */
//    shared_ptr<PinLayer<string> > m_pinLayer;   /*!< A pin layer */
//    vector<shared_ptr<Pin<string> > > m_pins;   /*!< Pins added in the pin layer */
//};
//
///*! Test callback for getting a pin tile */
//class TestPinLayerCallback : public AsyncCallbackWithRequest<TileKeyPtr, TilePtr>
//{
//public:
//    /* See definition for description */
//
//    TestPinLayerCallback(TestPinLayerInstances& testInstances);
//    virtual ~TestPinLayerCallback();
//
//    /* See description in AsyncCallback.h */
//    virtual void Success(TileKeyPtr request,
//                         TilePtr response);
//	virtual void Error(TileKeyPtr request,
//                       NB_Error error);
//
//    // Copy constructor and assignment operator are not supported.
//    TestPinLayerCallback(const TestPinLayerCallback& callback);
//    TestPinLayerCallback& operator=(const TestPinLayerCallback& callback);
//
//    TestPinLayerInstances& m_testInstances;     /*!< A test instance */
//    NB_Error m_error;                           /*!< An error of request */
//};
//
//
//// Test Functions ...............................................................................
//
//static void TestPinLayerInitialization(void);
//static void TestPinLayerAddPin(void);
//static void TestPinLayerGetTile(void);
//
//
//// Helper functions .............................................................................
//
//
//
//// Public Functions .............................................................................
//
///*! Add all your test functions here
//
//    @return None
//*/
//void
//TestPinLayer_AddAllTests(CU_pSuite pTestSuite)
//{
//    // ! Add all your function names here !
//    CU_add_test(pTestSuite, "TestPinLayerInitialization", &TestPinLayerInitialization);
//    CU_add_test(pTestSuite, "TestPinLayerAddPin", &TestPinLayerAddPin);
//    CU_add_test(pTestSuite, "TestPinLayerGetTile", &TestPinLayerGetTile);
//};
//
///*! Add common initialization code here
//
//    @return 0
//*/
//int
//TestPinLayer_SuiteSetup()
//{
//    return 0;
//}
//
///*! Add common cleanup code here
//
//    @return 0
//*/
//int
//TestPinLayer_SuiteCleanup()
//{
//    return 0;
//}
//
//
//// Test Functions ...............................................................................
//
///*! Test initializing a pin layer
//
//    @return None. CUnit Asserts get called on failures.
//*/
//void
//TestPinLayerInitialization(void)
//{
//    // Initialize a pin layer in constructor.
//    TestPinLayerInstances testInstances;
//}
//
///*! Test adding a pin
//
//    @return None. CUnit Asserts get called on failures.
//*/
//void
//TestPinLayerAddPin(void)
//{
//    // Initialize a pin layer in constructor.
//    TestPinLayerInstances testInstances;
//
//    // Test adding pins.
//    testInstances.AddPins();
//}
//
///*! Test getting a pin tile
//
//    @return None. CUnit Asserts get called on failures.
//*/
//void
//TestPinLayerGetTile(void)
//{
//    NB_Error error = NE_OK;
//
//    // Initialize a pin layer in constructor.
//    TestPinLayerInstances testInstances;
//
//    // Test adding pins.
//    error = testInstances.AddPins();
//    if (error != NE_OK)
//    {
//        return;
//    }
//
//    // Test getting a pin tile.
//    testInstances.GetTile(TEST_TILE_X,
//                          TEST_TILE_Y,
//                          TEST_ZOOM_LEVEL);
//}
//
//
//// Helper functions .............................................................................
//
//
//// TestPinLayerInstances functions ..............................................................
//
///*! TestPinLayerInstances constructor
//
//    CUnit Asserts get called on failures.
//*/
//TestPinLayerInstances::TestPinLayerInstances()
//: Base(),
//  m_pal(NULL),
//  m_context(NULL),
//  m_error(NE_OK),
//  m_pinLayer(),
//  m_pins()
//{
//    // Create a PAL and context.
//    if ((!CreatePalAndContext(&m_pal, &m_context)) ||
//        (!m_pal) ||
//        (!m_context))
//    {
//        CU_FAIL("Creating PAL and context failed");
//        return;
//    }
//
//    shared_ptr<string> layerID(new string(TEST_LAYER_ID));
//    if (!layerID)
//    {
//        CU_FAIL("Out of memory when allocated layer ID");
//        return;
//    }
//
//    // Get a XML file path contained material config information.
//    shared_ptr<string> materialConfigPath;
//
//    char* path = GetResourcePath(TEST_MATERIAL_CONFIG_FILENAME);
//    if (path)
//    {
//        materialConfigPath.reset(new string(path));
//        nsl_free(path);
//        path = NULL;
//
//        if (!materialConfigPath)
//        {
//            CU_FAIL("Out of memory when allocated material config path");
//            return;
//        }
//    }
//
//    m_pinLayer = shared_ptr<PinLayer<string> >(new PinLayerImpl<string>(layerID,
//                                                                        materialConfigPath,
//                                                                        NULL));
//    if (!m_pinLayer)
//    {
//        CU_FAIL("Out of memory when allocated pin layer");
//        return;
//    }
//}
//
///*! TestPinLayerInstances destructor */
//TestPinLayerInstances::~TestPinLayerInstances()
//{
//    m_pins.clear();
//    m_pinLayer.reset();
//
//    if (m_context)
//    {
//        NB_ContextDestroy(m_context);
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
///*! Test adding pins
//
//    @return NE_OK if success. CUnit Asserts get called on failures.
//*/
//NB_Error
//TestPinLayerInstances::AddPins()
//{
//    // Check if pin layer is initialized.
//    if (!m_pinLayer)
//    {
//        CU_FAIL("Pin layer is not initialized");
//        return NE_NOTINIT;
//    }
//
//    // Add pins to the pin layer.
//    for (int i = 0; i < TEST_PIN_COUNT; ++i)
//    {
//        // Create a pin name and address.
//        shared_ptr<string> name(new string(TEST_PIN_NAME[i]));
//        shared_ptr<string> address(new string(TEST_PIN_ADDRESS[i]));
//        if ((!name) || name->empty() ||
//            (!address) || address->empty())
//        {
//            CU_FAIL("Pin name or address is empty");
//            return NE_NOMEM;
//        }
//
//        // Add a pin to the pin layer.
//        shared_ptr<Pin<string> > pin = m_pinLayer->AddPin(PT_RED,
//                                                          TEST_PIN_LATITUDE[i],
//                                                          TEST_PIN_LONGITUDE[i],
//                                                          DataStreamPtr(),
//                                                          DataStreamPtr(),
//                                                          shared_ptr<string>());
//        if (!pin)
//        {
//            CU_FAIL("Adding pin failed");
//            return NE_UNEXPECTED;
//        }
//        m_pins.push_back(pin);
//    }
//
//    return NE_OK;
//}
//
///*! Test getting a pin tile
//
//    @return NE_OK if success. CUnit Asserts get called on failures.
//*/
//NB_Error
//TestPinLayerInstances::GetTile(int tileX,       /*!< Tile X position to request */
//                               int tileY,       /*!< Tile Y position to request */
//                               int zoomLevel    /*!< Zoom level to request */
//                               )
//{
//    // Check if pin layer is initialized.
//    if (!m_pinLayer)
//    {
//        CU_FAIL("Pin layer is not initialized");
//        return NE_NOTINIT;
//    }
//
//    // Create a tile key to request.
//    TileKeyPtr tileKey(new TileKey(tileX, tileY, zoomLevel));
//    if (!tileKey)
//    {
//        CU_FAIL("Out of memory when allocated tile key");
//        return NE_NOMEM;
//    }
//
//    // Create a callback.
//    shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback(new TestPinLayerCallback(*this));
//    if (!callback)
//    {
//        CU_FAIL("Out of memory when allocated callback");
//        return NE_NOMEM;
//    }
//
//    m_pinLayer->GetTile(tileKey,
//                        callback,
//                        HIGHEST_TILE_REQUEST_PRIORITY);
//    return m_error;
//}
//
///*! Write data to a file
//
//    @return NE_OK if success
//*/
//NB_Error
//TestPinLayerInstances::WriteDataToFile(const char* filename,        /*!< Filename */
//                                       DataStream& dataStream       /*!< Data to write */
//                                       )
//{
//    PAL_Error palError = PAL_Ok;
//    uint32 position = 0;
//    uint32 dataSizeGot = TEST_DATA_SIZE;
//    uint32 remainedSize = 0;
//    PAL_File* file = NULL;
//    uint8 data[TEST_DATA_SIZE] = {0};
//
//    if ((!filename) || (nsl_strlen(filename) == 0))
//    {
//        CU_FAIL("Filename is empty");
//        return NE_INVAL;
//    }
//
//    if (dataStream.GetDataSize() == 0)
//    {
//        CU_FAIL("No data to write");
//        return NE_INVAL;
//    }
//
//    if (!m_pal)
//    {
//        CU_FAIL("Pointer to PAL_Instance is NULL");
//        return NE_NOTINIT;
//    }
//
//    PAL_FileRemove(m_pal, filename);
//
//    palError = PAL_FileOpen(m_pal, filename, PFM_Create, &file);
//    if (palError != PAL_Ok)
//    {
//        CU_FAIL("Opening file failed");
//        return NE_FSYS;
//    }
//
//    remainedSize = dataStream.GetDataSize();
//    while (dataSizeGot == TEST_DATA_SIZE)
//    {
//        dataSizeGot = dataStream.GetData(data, position, TEST_DATA_SIZE);
//        if (dataSizeGot > 0)
//        {
//            uint32 bytesWritten = 0;
//
//            palError = PAL_FileWrite(file, data, dataSizeGot, &bytesWritten);
//            if (palError != PAL_Ok)
//            {
//                CU_FAIL("Writing file failed");
//                return NE_FSYS;
//            }
//
//            position += dataSizeGot;
//
//            palError = PAL_FileSetPosition(file, PFSO_Start, position);
//            if (palError != PAL_Ok)
//            {
//                CU_FAIL("Setting position of file failed");
//                return NE_FSYS;
//            }
//
//            remainedSize -= dataSizeGot;
//        }
//    }
//    CU_ASSERT_EQUAL(remainedSize, 0);
//
//    palError = PAL_FileClose(file);
//    if (palError != PAL_Ok)
//    {
//        CU_FAIL("Closing file failed");
//        return NE_FSYS;
//    }
//
//    return NE_OK;
//}
//
//
//// TestPinLayerCallback functions ...............................................................
//
///*! TestPinLayerCallback constructor */
//TestPinLayerCallback::TestPinLayerCallback(TestPinLayerInstances& testInstances /*!< A test instance */
//                                           )
//: m_testInstances(testInstances)
//{
//    // Reset error in test instance.
//    testInstances.m_error = NE_NOENT;
//}
//
///*! TestPinLayerCallback destructor */
//TestPinLayerCallback::~TestPinLayerCallback()
//{
//    // Nothing to do here.
//}
//
///* See description in AsyncCallback.h */
//void
//TestPinLayerCallback::Success(TileKeyPtr request,
//                              TilePtr response)
//{
//    if ((!request) || (!response))
//    {
//        m_testInstances.m_error = NE_INVAL;
//        CU_FAIL("Parameter is wrong");
//        return;
//    }
//
//    // Check content ID of tile.
//    shared_ptr<string> contentId = response->GetContentID();
//    if ((!contentId) || (contentId->empty()))
//    {
//        m_testInstances.m_error = NE_UNEXPECTED;
//        CU_FAIL("Content ID is empty");
//        return;
//    }
//    // Check data size of tile.
//    DataStreamPtr tileData = response->GetData();
//    if ((!tileData) || (tileData->GetDataSize() == 0))
//    {
//        m_testInstances.m_error = NE_UNEXPECTED;
//        CU_FAIL("Data of tile is empty");
//        return;
//    }
//
//    m_testInstances.m_error = m_testInstances.WriteDataToFile(contentId->c_str(),
//                                                              *tileData);
//}
//
///* See description in AsyncCallback.h */
//void
//TestPinLayerCallback::Error(TileKeyPtr request,
//                            NB_Error error)
//{
//    if ((!request) || (error == NE_OK))
//    {
//        error = NE_INVAL;
//        CU_FAIL("Parameter is wrong");
//        return;
//    }
//
//    m_testInstances.m_error = error;
//    CU_FAIL("Return an error in callback");
//}
//
///*! @} */
