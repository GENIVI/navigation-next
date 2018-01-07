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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbgmtestsuitefactory.h"
#include "nbgmtestsuite.h"
#include "palstdlib.h"
#include "gl_render_pal_test.h"
#include "entity_test.h"
#include "mesh_test.h"
#include "png_test.h"
#include "test_core_filestream.h"
#include "test_core_memorystream.h"
#include "sharedptr_test.h"
#include "gl_texture_test.h"
#include "test_nbm_archive.h"

NBRE_TestSuiteFactory::NBRE_TestSuiteFactory()
{
    SetUp();
}

NBRE_TestSuiteFactory::~NBRE_TestSuiteFactory()
{
    for(size_t i=0; i<mTestSuiteList.size(); ++i)
    {
        delete mTestSuiteList[i];
    }
}

NBRE_TestSuite* NBRE_TestSuiteFactory::GetTestSuite( const char* name )
{
    for(size_t i=0; i<mTestSuiteList.size(); ++i)
    {
        NBRE_TestSuite* suite = mTestSuiteList[i];
        if(nsl_strcmp(suite->GetName(), name) == 0)
        {
            return suite;
        }
    }
    return NULL;
}

NBRE_TestCase* NBRE_TestSuiteFactory::GetTestCase( const char* /*suiteName*/, const char* /*caseName*/ )
{
    nbre_assert(0);
    return NULL;
}

void NBRE_TestSuiteFactory::AddTestSuite( NBRE_TestSuite* suite )
{
    for(size_t i=0; i<mTestSuiteList.size(); ++i)
    {
        NBRE_TestSuite* oldSuite = mTestSuiteList[i];
        if(nsl_strcmp(oldSuite->GetName(), suite->GetName()) == 0)
        {
            oldSuite->Merge(*suite);
            suite = NULL;
            break;
        }
    }
    if(suite != NULL)
    {
        mTestSuiteList.push_back(suite);
    }
}

void NBRE_TestSuiteFactory::AddTestCase( NBRE_TestCase* /*suite*/ )
{
    //TODO
    nbre_assert(0);
}


void NBRE_TestSuiteFactory::SetUp()
{
    AddTestSuite(new GLRenderPalTest());
    AddTestSuite(new MeshTest());
    AddTestSuite(new EntityTest());
    AddTestSuite(new PngTest());
    AddTestSuite(new TestCoreFileStream());
    AddTestSuite(new TestCoreMemoryStream());
    AddTestSuite(new SharedPtrTest());
    AddTestSuite(new GLTextureTest());
}
