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
   @file        TestLamProcessor.cpp
   @defgroup    UnitTests

   Description: Implementation of TestLamProcessor

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */
#include "CUnit.h"

extern "C"
{
#include "networkutil.h"
#include "platformutil.h"
#include "TestLamProcessor.h"
}

#include "LamProcessor.h"

using namespace nbmap;

#define MAX_FULL_PATH_LENGTH 256

// In this UnitTest, a sample LAM file is provided, along with the original data that are
// sued to generate this LAM file. This UnitTest will try to load and decoded LAM file, and
// check if LamProcessor is calculating correctly.

static const char* TEST_LAM_SAMPLE_FILENAME = "LamSample.png";

#define MAX_PATH_LENGTH        256
#define MAX_ENTRY_PER_TYPE       10

#define LAM_INDEX_LM3D      4
#define LAM_INDEX_B3D       5
#define LAM_INDEX_DVR       6
#define LAM_INDEX_DVA       7

#define TOTAL_TEST_ENTRIES       4


struct TestEntry
{
    int x;
    int y;
};


struct TestLayerEntry
{
    int lamIndex;
    TestEntry validEntries[MAX_ENTRY_PER_TYPE];
    TestEntry invalidEntries[MAX_ENTRY_PER_TYPE];
};


// Following data is provided by mobious team.
TestLayerEntry g_TestEntries[TOTAL_TEST_ENTRIES] =
{
    // LM3D
    {
        LAM_INDEX_LM3D,
        {
            {9766, 12191}, {9766, 12192}, {9767, 12188}, {9767, 12192}, {9767, 12194},
            {9768, 12192}, {9768, 12193}, {9768, 12194}, {9768, 12195}, {9769, 12192}
        },
        {
            {9766, 12193}, {9766, 12193}, {9767, 12183}, {9767, 12193}, {9767, 12193},
            {9768, 12191}, {9768, 12120}, {9768, 12126}, {9768, 12191}, {9769, 12189},
        }
    },
    // B3D
    {
        LAM_INDEX_B3D,
        {
            {9733, 12185}, {9739, 12181}, {9745, 12126}, {9755, 12066}, {9758, 12064},
            {9761, 12232}, {9767, 12192}, {9767, 12193}, {9767, 12250}, {9768, 12192},
        },
        {
            {9733, 12193}, {9739, 12193}, {9745, 12193}, {9755, 12093}, {9758, 12093},
            {9761, 12293}, {9767, 12150}, {9767, 12150}, {9767, 12296}, {9768, 12196},
        }
    },
    // DVR
    {
        LAM_INDEX_DVR,
        {
            {9728, 12032}, {9728, 12033}, {9728, 12034}, {9728, 12035}, {9728, 12050},
            {9728, 12051}, {9728, 12052}, {9728, 12053}, {9728, 12054}, {9728, 12055},
        },
        {
            {9728, 12036}, {9728, 12037}, {9728, 12038}, {9728, 12039}, {9728, 12040},
            {9728, 12041}, {9728, 12042}, {9728, 12043}, {9728, 12044}, {9728, 12045},
       }
    },
    // DVA
    {
        LAM_INDEX_DVA,
        {
            {9728, 12074}, {9728, 12075}, {9728, 12076}, {9728, 12077}, {9728, 12078},
            {9728, 12086}, {9728, 12087}, {9728, 12088}, {9728, 12089}, {9728, 12090},
        },
        {
            {9728, 12079}, {9728, 12080}, {9728, 12081}, {9728, 12082}, {9728, 12083},
            {9728, 12084}, {9728, 12085}, {9728, 12091}, {9728, 12092}, {9728, 12093},
        }
    }
};


/*! Test Instance. */
class TestLamProcessorInstance
{
public:
    TestLamProcessorInstance();
    virtual ~TestLamProcessorInstance();

    NB_Error Initialize();
    void Reset();

    NB_Error CheckLamIndexPositive();
    NB_Error CheckLamIndexNegative();
    NB_Error DecodeLamFromFile();

private:
    NB_Context*   m_pContext;
    PAL_Instance* m_pPal;
    LamProcessor*  m_pLamProcessor;
    PNG_Data*     m_decodedData;
};

/*! Load and decode LAM from file.

  @return void
*/
void TestLamProcessorDecodeLamFromFile()
{
    TestLamProcessorInstance instance;
    NB_Error error = instance.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : instance.DecodeLamFromFile();

    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Positive check.

  @return void
*/
void TestLamProcessorCheckLamIndexPositive()
{
    TestLamProcessorInstance instance;
    NB_Error error = instance.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : instance.DecodeLamFromFile();

    CU_ASSERT_EQUAL(error, NE_OK);

    if (error)
    {
        return;
    }

    error = instance.CheckLamIndexPositive();
    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Negative check.

  @return void
*/
void TestLamProcessorCheckLamIndexNegative()
{
    TestLamProcessorInstance instance;
    NB_Error error = instance.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);

    error = error ? error : instance.DecodeLamFromFile();

    CU_ASSERT_EQUAL(error, NE_OK);
    if (error)
    {
        return;
    }

    error = instance.CheckLamIndexNegative();
    CU_ASSERT_EQUAL(error, NE_OK);

}


// Cleanup for suite
int TestLamProcessor_SuiteSetup(void)
{
    return 0;
}

int TestLamProcessor_SuiteCleanup(void)
{
    return 0;
}

void TestLamProcessor_AddAllTests(CU_pSuite pTestSuite)
{
    CU_add_test(pTestSuite, "TestLamProcessorDecodeLamFromFile",
                &TestLamProcessorDecodeLamFromFile);
    CU_add_test(pTestSuite, "TestLamProcessorCheckLamIndexPositive",
                &TestLamProcessorCheckLamIndexPositive);
    CU_add_test(pTestSuite, "TestLamProcessorCheckLamIndexNegative",
                &TestLamProcessorCheckLamIndexNegative);
}


// Implementation of TestLamProcessorInstance
/* See description in header file. */
TestLamProcessorInstance::TestLamProcessorInstance()
        : m_pPal(NULL),
          m_pContext(NULL),
          m_pLamProcessor(NULL),
          m_decodedData(NULL)
{
}

/* See description in header file. */
TestLamProcessorInstance::~TestLamProcessorInstance()
{
}

/* See description in header file. */
NB_Error TestLamProcessorInstance::Initialize()
{
    NB_Error error = NE_OK;
    do
    {
        //  Create PAL instance and context
        uint8 createResult = CreatePalAndContext(&(m_pPal), &(m_pContext));
        if (!createResult || !m_pPal || !m_pContext)
        {
            error = NE_NOTINIT;
            break;
        }

        m_pLamProcessor = new LamProcessor(m_pContext);
        if (!m_pLamProcessor)
        {
            error = NE_NOMEM;
            break;
        }
    } while (0);

    if (error)
    {
        Reset();
    }

    return error;
}

/* See description in header file. */
void TestLamProcessorInstance::Reset()
{
    if (m_decodedData)
    {
        if (m_pLamProcessor)
        {
            m_pLamProcessor->DestroyPNGData(&m_decodedData);
        }
        m_decodedData = NULL;
    }

    if (m_pLamProcessor)
    {
        delete m_pLamProcessor;
        m_pLamProcessor = NULL;
    }

    if (m_pContext)
    {
        NB_ContextDestroy(m_pContext);
        m_pContext = NULL;
    }

    if (m_pPal)
    {
        PAL_Destroy(m_pPal);
        m_pPal = NULL;
    }
}

/* See description in header file. */
NB_Error TestLamProcessorInstance::CheckLamIndexNegative()
{
    NB_Error error = NE_OK;
    TestLayerEntry* layerEntry = &g_TestEntries[0];
    for (size_t i = 0; i < TOTAL_TEST_ENTRIES; ++i, ++layerEntry)
    {
        uint32 lamIndex = layerEntry->lamIndex;

        TestEntry* entry = &(layerEntry->invalidEntries[0]);
        for (size_t j = 0; j < MAX_ENTRY_PER_TYPE; ++j, ++entry)
        {
            bool result = m_pLamProcessor->CheckLamIndex(m_decodedData, entry->x,
                                                        entry->y, lamIndex);
            CU_ASSERT(!result);
            if (result)
            {
                printf("ID: %u, %d, %d, --- %d returned true!\n",
                       j, entry->x,
                       entry->y, lamIndex);
                error = NE_UNEXPECTED;
            }
        }
    }

    return error;
}

/* See description in header file. */
NB_Error TestLamProcessorInstance::CheckLamIndexPositive()
{
    NB_Error error = NE_OK;
    TestLayerEntry* layerEntry = &g_TestEntries[0];
    for (size_t i = 0; i < TOTAL_TEST_ENTRIES; ++i, ++layerEntry)
    {
        uint32 lamIndex = layerEntry->lamIndex;

        TestEntry* entry = &(layerEntry->validEntries[0]);
        for (size_t j = 0; j < MAX_ENTRY_PER_TYPE; ++j, ++entry)
        {
            bool result = m_pLamProcessor->CheckLamIndex(m_decodedData, entry->x,
                                                        entry->y, lamIndex);
            CU_ASSERT(result);
            if (!result)
            {
                error = NE_UNEXPECTED;
            }
        }
    }

    return error;
}

/* See description in header file. */
NB_Error TestLamProcessorInstance::DecodeLamFromFile()
{

    NB_Error error = NE_OK;
    do
    {
        unsigned char* data        = NULL;
        uint32         dataSize = 0;
        if ((PAL_FileLoadFile(m_pPal, TEST_LAM_SAMPLE_FILENAME, &data, &dataSize) != PAL_Ok) || dataSize == 0)
        {
            error = NE_FSYS;
            break;
        }

        m_decodedData = m_pLamProcessor->DecodeLamFromFile(TEST_LAM_SAMPLE_FILENAME);
        if (!m_decodedData)
        {
            error = NE_FSYS;
        }
    } while (0);

    return error;
}

/*! @} */
