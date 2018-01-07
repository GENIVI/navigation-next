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

#include "sharedptr_test.h"
#include "nbresharedptr.h"
#include "utility.h"
#include "nbretypes.h"

class Stuff
{
public:
    Stuff(int i = 0):mValue(i){ ++mCount; }
    ~Stuff(){ --mCount; }

public:
    static int GetCount() { return mCount; }

public:
    int mValue;

private:
    static int mCount;
};

int Stuff::mCount = 0;

static int
Init(void)
{
    return 0;
}

static int
CleanUp(void)
{
    return 0;
}

void
TestSharedPtrFreeMethod()
{
    NBRE_SharedPtr<uint8> sp((uint8*)nsl_malloc(1024), NBRE_SPFM_FREE);
    CU_ASSERT(sp.Get() != NULL);
    CU_ASSERT(sp.GetPointer() != NULL);
    CU_ASSERT(sp.GetUseCountPointer() != NULL);
    CU_ASSERT(sp.GetUseCount() == 1)
    CU_ASSERT(sp.IsUnique());
    CU_ASSERT(!sp.IsNull());
    CU_ASSERT(sp.GetFreeMethod() == NBRE_SPFM_FREE);

    sp.SetNull();
    CU_ASSERT(sp.IsNull());
    CU_ASSERT(sp.Get() == NULL);
    CU_ASSERT(sp.GetPointer() == NULL);
    CU_ASSERT(sp.GetUseCountPointer() == NULL);
}

void
TestSharedPtrDeleteMethod(void)
{
    NBRE_SharedPtr<Stuff> sp(NBRE_NEW Stuff(123), NBRE_SPFM_DELETE);
    CU_ASSERT(sp.Get() != NULL);
    CU_ASSERT(sp.GetPointer() != NULL);
    CU_ASSERT(sp.GetUseCountPointer() != NULL);
    CU_ASSERT(sp.GetUseCount() == 1)
    CU_ASSERT(sp.IsUnique());
    CU_ASSERT(!sp.IsNull());
    CU_ASSERT(sp.GetFreeMethod() == NBRE_SPFM_DELETE);
    CU_ASSERT(sp->mValue == 123);
    CU_ASSERT((*sp).mValue == 123);
    CU_ASSERT(Stuff::GetCount() == 1);

    sp.SetNull();
    CU_ASSERT(sp.IsNull());
    CU_ASSERT(sp.Get() == NULL);
    CU_ASSERT(sp.GetPointer() == NULL);
    CU_ASSERT(sp.GetUseCountPointer() == NULL);
    CU_ASSERT(Stuff::GetCount() == 0);
}

void
TestSharedPtrDeleteArrayMethod()
{
    {
        NBRE_SharedPtr<Stuff> sp(NBRE_NEW Stuff[100], NBRE_SPFM_DELETE_ARRAY);
        CU_ASSERT(sp.Get() != NULL);
        CU_ASSERT(sp.GetPointer() != NULL);
        CU_ASSERT(sp.GetUseCountPointer() != NULL);
        CU_ASSERT(sp.GetUseCount() == 1)
        CU_ASSERT(sp.IsUnique());
        CU_ASSERT(!sp.IsNull());
        CU_ASSERT(sp.GetFreeMethod() == NBRE_SPFM_DELETE_ARRAY);
        CU_ASSERT(Stuff::GetCount() == 100);
    }
    CU_ASSERT(Stuff::GetCount() == 0);
}


void
TestSharedPtrCopyConstruct(void)
{
    NBRE_SharedPtr<Stuff> sp1(NBRE_NEW Stuff(100), NBRE_SPFM_DELETE);
    NBRE_SharedPtr<Stuff> sp2 = sp1;
    // Warning:
    // Stuff *p = NBRE_NEW Stuff(100);
    // NBRE_SharedPtr<Stuff> sp1(p, NBRE_SPFM_DELETE);
    // NBRE_SharedPtr<Stuff> sp2(p, NBRE_SPFM_DELETE);
    // will cause double delete

    CU_ASSERT(!sp1.IsNull());
    CU_ASSERT(!sp2.IsNull());
    CU_ASSERT(sp1.GetPointer() == sp2.GetPointer());
    CU_ASSERT(sp1.Get() == sp2.Get());
    CU_ASSERT(sp1.GetFreeMethod() == sp2.GetFreeMethod());
    CU_ASSERT(!sp1.IsUnique());
    CU_ASSERT(!sp2.IsUnique());
    CU_ASSERT((*sp1).mValue == 100);
    CU_ASSERT((*sp2).mValue == 100);
    CU_ASSERT(sp1->mValue == 100);
    CU_ASSERT(sp2->mValue == 100);
    CU_ASSERT(sp1.GetFreeMethod() == NBRE_SPFM_DELETE);
    CU_ASSERT(sp2.GetFreeMethod() == NBRE_SPFM_DELETE);
    CU_ASSERT(sp1.GetUseCount() == 2);
    CU_ASSERT(sp2.GetUseCount() == 2);
    CU_ASSERT(Stuff::GetCount() == 1);

    sp2->mValue = 200;
    CU_ASSERT(sp1->mValue == 200);

    sp1.SetNull();
    CU_ASSERT(sp1.IsNull());
    CU_ASSERT(sp1.Get() == NULL);
    CU_ASSERT(sp1.GetPointer() == NULL);
    CU_ASSERT(sp1.GetUseCountPointer() == NULL);
    CU_ASSERT(!sp2.IsNull());
    CU_ASSERT(sp2.Get() != NULL);
    CU_ASSERT(sp2.GetPointer() != NULL);
    CU_ASSERT(sp2.GetUseCountPointer() != NULL);
    CU_ASSERT(sp2.IsUnique());
    CU_ASSERT((*sp2).mValue == 200);
    CU_ASSERT(sp2->mValue == 200);
    CU_ASSERT(sp2.GetFreeMethod() == NBRE_SPFM_DELETE);
    CU_ASSERT(sp2.GetUseCount() == 1);
    CU_ASSERT(Stuff::GetCount() == 1);

    sp2.SetNull();
    CU_ASSERT(Stuff::GetCount() == 0);
}

void
TestSharedPtrAssignment(void)
{
    {
        NBRE_SharedPtr<Stuff> sp1(NBRE_NEW Stuff(111), NBRE_SPFM_DELETE);
        NBRE_SharedPtr<Stuff> sp2(NBRE_NEW Stuff(222), NBRE_SPFM_DELETE);
        NBRE_SharedPtr<Stuff> sp3(sp2);

        CU_ASSERT(Stuff::GetCount() == 2);
        CU_ASSERT(sp1.GetUseCount() == 1);
        CU_ASSERT(sp2.GetUseCount() == 2);
        CU_ASSERT(sp3.GetUseCount() == 2);
        CU_ASSERT(sp1->mValue == 111);
        CU_ASSERT(sp2->mValue == 222);
        CU_ASSERT(sp3->mValue == 222);
        sp2 = sp1;
        CU_ASSERT(Stuff::GetCount() == 2);
        CU_ASSERT(sp1.GetUseCount() == 2);
        CU_ASSERT(sp2.GetUseCount() == 2);
        CU_ASSERT(sp3.GetUseCount() == 1);
        CU_ASSERT(sp1->mValue == 111);
        CU_ASSERT(sp2->mValue == 111);
        CU_ASSERT(sp3->mValue == 222);
    }
    CU_ASSERT(Stuff::GetCount() == 0);
}

void
TestSharedPtrBind()
{
     NBRE_SharedPtr<Stuff> sp;
     sp.Bind(NBRE_NEW Stuff(100), NBRE_SPFM_DELETE);
     CU_ASSERT(sp.Get() != NULL);
     CU_ASSERT(sp.GetPointer() != NULL);
     CU_ASSERT(sp.GetUseCountPointer() != NULL);
     CU_ASSERT(sp.GetUseCount() == 1)
     CU_ASSERT(sp.IsUnique());
     CU_ASSERT(!sp.IsNull());
     CU_ASSERT(sp.GetFreeMethod() == NBRE_SPFM_DELETE);
     CU_ASSERT(sp->mValue == 100);
     CU_ASSERT((*sp).mValue == 100);
     CU_ASSERT(Stuff::GetCount() == 1);

     sp.SetNull();
     CU_ASSERT(Stuff::GetCount() == 0);
     CU_ASSERT(sp.IsNull());
     CU_ASSERT(sp.Get() == NULL);
     CU_ASSERT(sp.GetPointer() == NULL);
     CU_ASSERT(sp.GetUseCountPointer() == NULL);
}

void
TestSharedPtrContainer()
{
   NBRE_SharedPtr<Stuff> sp1(NBRE_NEW Stuff(), NBRE_SPFM_DELETE);
   NBRE_SharedPtr<Stuff> sp2(NBRE_NEW Stuff(), NBRE_SPFM_DELETE);
   NBRE_SharedPtr<Stuff> sp3(NBRE_NEW Stuff(), NBRE_SPFM_DELETE);

   CU_ASSERT(Stuff::GetCount() == 3);
   CU_ASSERT(sp1.GetUseCount() == 1);
   CU_ASSERT(sp2.GetUseCount() == 1);
   CU_ASSERT(sp3.GetUseCount() == 1);

   NBRE_Vector< NBRE_SharedPtr<Stuff> > vec;
   vec.push_back(sp1);
   vec.push_back(sp2);
   vec.push_back(sp3);

   CU_ASSERT(Stuff::GetCount() == 3);
   CU_ASSERT(sp1.GetUseCount() == 2);
   CU_ASSERT(sp2.GetUseCount() == 2);
   CU_ASSERT(sp3.GetUseCount() == 2);

   sp1.SetNull();
   sp2.SetNull();
   sp3.SetNull();
   CU_ASSERT(Stuff::GetCount() == 3);
   CU_ASSERT(vec[0].GetUseCount() == 1);
   CU_ASSERT(vec[1].GetUseCount() == 1);
   CU_ASSERT(vec[2].GetUseCount() == 1);

   vec.clear();
   CU_ASSERT(Stuff::GetCount() == 0);
}

SharedPtrTest::SharedPtrTest():NBRE_TestSuite("sharedptr_test", Init, CleanUp)
{
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_Bind",                  TestSharedPtrBind));
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_Construct_Free",        TestSharedPtrFreeMethod));
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_Construct_Delete",      TestSharedPtrDeleteMethod));
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_Construct_DeleteArray", TestSharedPtrDeleteArrayMethod));
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_CopyConstruct",         TestSharedPtrCopyConstruct));
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_Assignment",            TestSharedPtrAssignment));
    AddTestCase(new NBRE_TestCase("Test_SharedPtr_Container",             TestSharedPtrContainer));
}
