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

    @defgroup TestTPS_c Unit Tests for CSL Logging

    Unit Tests for CSL Logging

    This file contains all unit tests for the Core logging component
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

#include "testtps.h"
#include "main.h"
#include "platformutil.h"
#ifdef WINCE
#include "winbase.h"
#endif

#include "tpslib.h"
#include "csdict.h"
#include "tpselt.h"
#include "tpsio.h"
                                           
// Local Functions 
static void TestLoadTPSLibrary(void);
static void TestTPSLibraryCaps(void);
static void TestTPSPack(void);
static void TestTPSUnpack(void);
static void TestTPSElements(void);

static const char tpslib_data[] = {

	82, 220, 158, 174, 49, 227, 106, 101, 96, 221, 94, 65, 7, 90, 
	90, 167, 67, 113, 30, 250, 0, 0, 0, 2, 116, 101, 115, 116, 99, 
	97, 112, 49, 0, 121, 101, 115, 0, 116, 101, 115, 116, 99, 97, 
	112, 50, 0, 110, 111, 0, 0, 0, 0, 13, 116, 101, 115, 116, 45, 
	101, 108, 101, 109, 101, 110, 116, 0, 0, 0, 0, 18, 101, 120, 
	97, 109, 112, 108, 101, 45, 105, 116, 101, 109, 0, 110, 97, 
	109, 101, 0, 0, 0, 0, 24, 101, 120, 97, 109, 112, 108, 101, 45,
	105, 116, 101, 109, 0, 110, 97, 109, 101, 0, 118, 97, 108, 117, 
	101, 0, 0, 0, 0, 18, 100, 101, 116, 97, 105, 108, 115, 0, 97, 0, 
	98, 0, 99, 0, 100, 0, 101, 0, 0, 0, 0, 16, 100, 101, 116, 97, 105, 
	108, 115, 0, 97, 0, 98, 0, 99, 0, 100, 0
};

/*! Add all your test functions here

    @return None
*/
void
TestTPS_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
   CU_add_test(pTestSuite, "TestLoadTPSLibrary", &TestLoadTPSLibrary);
   CU_add_test(pTestSuite, "TestTPSLibraryCaps", &TestTPSLibraryCaps);
   CU_add_test(pTestSuite, "TestTPSPack", &TestTPSPack);
   CU_add_test(pTestSuite, "TestTPSUnpack", &TestTPSUnpack);
   CU_add_test(pTestSuite, "TestTPSElements", &TestTPSElements);
};

/*! Add common initialization code here.

    @return 0

    @see TestNetwork_SuiteCleanup
*/
int
TestTPS_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

    @return 0

    @see TestTPS_SuiteSetup
*/
int 
TestTPS_SuiteCleanup()
{
    return 0;
}


struct tpslib*
TestLoadTPSLibraryImpl(void)
{
	struct tpslib *tl = NULL;
		
	tl = tpslib_preload(tpslib_data, sizeof(tpslib_data));
	
	CU_ASSERT_PTR_NOT_NULL(tl);
	
	return tl;
}

static void TestLoadTPSLibrary(void)
{
	struct tpslib *tl = TestLoadTPSLibraryImpl();	

    if (tl != NULL)
		tpslib_dealloc(tl);
}

tpselt
TestCreateSampleDataImpl(struct tpslib *tl)
{
	tpselt elt, elt2;
	
	elt = te_new("test-element");
	CU_ASSERT_PTR_NOT_NULL(elt);
	
	elt2 = te_new("example-item");
	CU_ASSERT_PTR_NOT_NULL(elt2);
	
	te_setattrc(elt2, "name", "test value");
	
	te_attach(elt, elt2);
	
	elt2 = te_new("details");
	CU_ASSERT_PTR_NOT_NULL(elt2);
	te_setattrc(elt2, "a", "something");
	te_setattrc(elt2, "b", "something else");
	te_setattrc(elt2, "c", "something more");
	te_setattrc(elt2, "d", "something interesting");
	
	te_attach(elt, elt2);
	
	return elt;
}

static void TestTPSLibraryCaps(void)
{
	int capIndex;
	const char *keyPtr;
	char *valPtr;
	size_t capSize;
	struct CSL_Dictionary *caps; 
	
	struct tpslib *tl = TestLoadTPSLibraryImpl();	
	
	capIndex = 0;
	
	caps = tpslib_getcaps(tl);
	
	CU_ASSERT_PTR_NOT_NULL(caps);
	
	if (caps != NULL) {
		
		CSL_DictionaryNext(caps, &capIndex, &keyPtr, &valPtr, &capSize);
		
		CU_ASSERT_STRING_EQUAL("testcap1", keyPtr);
		CU_ASSERT_STRING_EQUAL("yes", valPtr);
		
		CSL_DictionaryNext(caps, &capIndex, &keyPtr, &valPtr, &capSize);
		
		CU_ASSERT_STRING_EQUAL("testcap2", keyPtr);
		CU_ASSERT_STRING_EQUAL("no", valPtr);
	}
	
	if (tl != NULL)
		tpslib_dealloc(tl);
}

struct test_pack_data {

	char*	data;
	size_t	size;
	size_t  used;
};

enum tpsio_status 
test_pack_reader(void *arg, char *doc, size_t doclen)
{
	struct test_pack_data* data = arg;
	
	if ((data->size - data->used) < doclen) {
	
		data->data = nsl_realloc(data->data, data->used+doclen);
		
		if (data->data != NULL) {
		
			data->size = data->used+doclen;
		}
		else {
			
			data->size = 0;
			return TIO_NOMEM;
		}
	}
	
	memcpy(data->data + data->used, doc, doclen);
	data->used += doclen;
	
	return TIO_READY;
};

static void TestTPSPack(void)
{
	tpselt elt;
	struct test_pack_data data;

	enum tpsio_status status;
	
	struct tpslib *tl = TestLoadTPSLibraryImpl();
	
	if (tl != NULL) {
		
		elt = TestCreateSampleDataImpl(tl);
		
		if (elt != NULL) {
			
			data.data = NULL;
			data.size = 0;
			data.used = 0;
			
			status = tps_pack(tl, elt, test_pack_reader, &data);

			CU_ASSERT_EQUAL(TIO_READY, status);
			
			te_dealloc(elt);
			
			if (data.data != NULL)
				nsl_free(data.data);
		}
		
		tpslib_dealloc(tl);
	}
}


static void TestTPSUnpack(void)
{
	tpselt elt, elt_c1, elt_c2, elt_c3;
	struct test_pack_data data;
	
	enum tpsio_status status;
	
	struct tps_unpackstate *ups;
	size_t len;
	int iter;
	
	struct tpslib *tl = TestLoadTPSLibraryImpl();
	
	if (tl != NULL) {
		
		elt = TestCreateSampleDataImpl( tl);
		
		if (elt != NULL) {
			
			data.data = NULL;
			data.size = 0;
			data.used = 0;
			
			status = tps_pack(tl, elt, test_pack_reader, &data);
			
			CU_ASSERT_EQUAL(TIO_READY, status);
			
			te_dealloc(elt);
			
			if (status == TIO_READY) {
				
				iter = 0;
				
				ups = tps_unpack_start(tl, data.used);
				len = tps_unpack_feed(ups, data.data, data.used);
				
				CU_ASSERT(data.used == len);
				
				status = tps_unpack_result(ups, &elt);
				
				CU_ASSERT_EQUAL(TIO_READY, status);
				CU_ASSERT_PTR_NOT_NULL(elt);
				
				if (status == TIO_READY && elt != NULL)
				{
					elt_c1 = te_nextchild(elt, &iter);
					CU_ASSERT_PTR_NOT_NULL(elt_c1);
					
					elt_c2 = te_nextchild(elt, &iter);
					CU_ASSERT_PTR_NOT_NULL(elt_c2);
					
					elt_c3 = te_nextchild(elt, &iter);
					
					CU_ASSERT_STRING_EQUAL("test-element", te_getname(elt));
					CU_ASSERT_STRING_EQUAL("example-item", te_getname(elt_c1));
					CU_ASSERT_STRING_EQUAL("details", te_getname(elt_c2));
					CU_ASSERT_STRING_EQUAL("test value", te_getattrc(elt_c1, "name"));

					CU_ASSERT_STRING_EQUAL("something", te_getattrc(elt_c2, "a"));
					CU_ASSERT_STRING_EQUAL("something else", te_getattrc(elt_c2, "b"));
					CU_ASSERT_STRING_EQUAL("something more", te_getattrc(elt_c2, "c"));
					CU_ASSERT_STRING_EQUAL("something interesting", te_getattrc(elt_c2, "d"));
					
					te_dealloc(elt);
				}
			}
			
			if (data.data != NULL)
				nsl_free(data.data);
		}
		
		tpslib_dealloc(tl);
	}
}

static void TestTPSElements(void)
{
	tpselt elt;
    tpselt celt;
    tpselt parelt;
    tpselt clone;
    tpselt temp;
    int i, index;
    char chBuf[64];
    int ret;
    int iter;
    char *pAttr;
    const char *pAttrC;
    size_t outSize;
    uint32 ui32 = 12345;
    uint64 ui64 = 20000000;
    double dblVal = 123.45;

    // Create a new element
    elt = te_new("Parent");
    CU_ASSERT_PTR_NOT_NULL(elt);

    // Try all setattr
    ret = te_setattr(elt, "SetAttr", "SetAttrValue", 12);
    CU_ASSERT_EQUAL(ret, 1);
    ret = te_setattrc(elt, "SetAttrC", "SetAttrCValue");
    CU_ASSERT_EQUAL(ret, 1);
    ret = te_setattru(elt, "SetAttrU", ui32);
    CU_ASSERT_EQUAL(ret, 1);
    ret = te_setattru64(elt, "SetAttrU64", ui64);
    CU_ASSERT_EQUAL(ret, 1);
    ret = te_setattrf(elt, "SetAttrF", dblVal);
    CU_ASSERT_EQUAL(ret, 1);
    dblVal = 6789.1234;
    ret = te_setattrd(elt, "SetAttrD", dblVal);
    CU_ASSERT_EQUAL(ret, 1);

    //Check the attributes set
    ret = te_getattr(elt, "SetAttr", &pAttr, &outSize);
    CU_ASSERT_EQUAL(ret, 1);
    CU_ASSERT_STRING_EQUAL(pAttr, "SetAttrValue");
    pAttrC = (const char *)te_getattrc(elt, (const char *)"SetAttrC");
    CU_ASSERT_STRING_EQUAL(pAttrC, "SetAttrCValue");
    ui32 = te_getattru(elt, "SetAttrU");
    CU_ASSERT_EQUAL(ui32, 12345);
    ui64 = te_getattru(elt, "SetAttrU64");
    CU_ASSERT_EQUAL(ui64, 20000000);
    ret = te_getattrf(elt, "SetAttrF", &dblVal);
    CU_ASSERT_EQUAL(ret, 1);
    CU_ASSERT_DOUBLE_EQUAL(dblVal, 123.45, .001);
    ret = te_getattrd(elt, "SetAttrD", &dblVal);
    CU_ASSERT_EQUAL(ret, 1);
    CU_ASSERT_DOUBLE_EQUAL(dblVal, 6789.1234, .001);

    // Try to delete an attribute and see if it is successful
    te_delattr(elt, "SetAttr");
    ret = te_getattr(elt, "SetAttr", &pAttr, &outSize);
    CU_ASSERT_EQUAL(ret, 0);

    // Try attaching children
    for(i = 0; i < 64; i++)
    {
        nsl_sprintf(chBuf, "Child %d", i+1);
        celt = te_new(chBuf);
        CU_ASSERT_PTR_NOT_NULL(celt);
        // Add an attribute
        ret = te_setattrc(celt, "Name", chBuf);
        CU_ASSERT_EQUAL(ret, 1);
        // Attach it to the parent element
        ret = te_attach(elt, celt);
        CU_ASSERT_EQUAL(ret, 1);
    }

    // Check all of the children (look-up on name)
    for(i = 0; i < 64; i++)
    {
        nsl_sprintf(chBuf, "Child %d", i+1);
        celt = te_getchild(elt, chBuf);
        CU_ASSERT_PTR_NOT_NULL(celt);
        if(celt != NULL)
        {
            // Check the element name
            pAttrC = te_getname(celt);
            CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

            // Check that the parent is correct
            parelt = te_getparent(celt);
            CU_ASSERT_PTR_NOT_NULL(parelt);
            pAttrC = te_getname(parelt);
            CU_ASSERT_STRING_EQUAL(pAttrC, "Parent");

            // Check the attribute we added
            pAttrC = (const char *)te_getattrc(celt, (const char *)"Name");
            CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);
        }
    }

    // Check all of the children (look-up on index)
    for(i = 0; i < 64; i++)
    {
        celt = te_nthchild(elt, i);
        CU_ASSERT_PTR_NOT_NULL(celt);
        if(celt != NULL)
        {
            // Name to check for
            nsl_sprintf(chBuf, "Child %d", i+1);

            // Check the element name
            pAttrC = te_getname(celt);
            CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

            // Check that the parent is correct
            parelt = te_getparent(celt);
            CU_ASSERT_PTR_NOT_NULL(parelt);
            pAttrC = te_getname(parelt);
            CU_ASSERT_STRING_EQUAL(pAttrC, "Parent");

            // Check the attribute we added
            pAttrC = (const char *)te_getattrc(celt, (const char *)"Name");
            CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);
        }
    }

    // Check all of the children (use iterator)
    iter = 0;
    celt = te_nextchild(elt, &iter);
    while(celt != NULL)
    {
        // Name to check for
        nsl_sprintf(chBuf, "Child %d", iter);

        // Check the element name
        pAttrC = te_getname(celt);
        CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

        // Check that the parent is correct
        parelt = te_getparent(celt);
        CU_ASSERT_PTR_NOT_NULL(parelt);
        pAttrC = te_getname(parelt);
        CU_ASSERT_STRING_EQUAL(pAttrC, "Parent");

        // Check the attribute we added
        pAttrC = (const char *)te_getattrc(celt, (const char *)"Name");
        CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

        // Get the next child
        celt = te_nextchild(elt, &iter);
    }

    // Ensure that we can unlink an element
    nsl_strcpy(chBuf, "Child 10");
    celt = te_getchild(elt, chBuf);
    CU_ASSERT_PTR_NOT_NULL(celt);
    te_unlink(celt);

    // Check the parent as it should not have one anymore
    parelt = te_getparent(celt);
    CU_ASSERT_PTR_NULL(parelt);

    // Make a copy and ensure that it is correct
    clone = te_clone(celt);
    pAttrC = te_getname(clone);
    CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);
    pAttrC = (const char *)te_getattrc(clone, (const char *)"Name");
    CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

    // destroy the detached child and clone
    te_dealloc(celt);
    te_dealloc(clone);

    // Ensure that all of the children can still be accessed after the unlink
    iter = 0;
    temp = te_nextchild(elt, &iter);
    while(temp != NULL)
    {
        index = iter;

        if(iter >= 10)
        {
            index++;
        }

        // Name to check for
        nsl_sprintf(chBuf, "Child %d", index);

        // Check the element name
        pAttrC = te_getname(temp);
        CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

        // Check that the parent is correct
        parelt = te_getparent(temp);
        CU_ASSERT_PTR_NOT_NULL(parelt);
        pAttrC = te_getname(parelt);
        CU_ASSERT_STRING_EQUAL(pAttrC, "Parent");

        // Check the attribute we added
        pAttrC = (const char *)te_getattrc(temp, (const char *)"Name");
        CU_ASSERT_STRING_EQUAL(pAttrC, chBuf);

        // Get the next child
        temp = te_nextchild(elt, &iter);
    }

    // destroy the parent
    te_dealloc(elt);
}

/*! @} */


