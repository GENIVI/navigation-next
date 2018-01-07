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
 *  tps_tests.c
 *  NIMCoreServicesTest
 *
 *  Created by Mark Goddard on 8/20/08.
 *  Copyright 2008 Networks In Motion, Inc.. All rights reserved.
 *
 */

#include "tps_tests.h"

#include "nimtypes.h"
#include "nimalloc.h"
#include "tpslib.h"
#include "csdict.h"
#include "tpselt.h"
#include "tpsio.h"

// sample TPS library corresponding to the following TPS library
/*
capabilities:
testcap1: yes
testcap2: no

templates:
- test-element: []
- example-item: [name]
- example-item: [name, value]
- details: [a, b, c, d, e]
- details: [a, b, c, d]
*/

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

struct tpslib*
TestLoadLibraryImpl(CuTest* tc)
{
	struct tpslib *tl = NULL;
		
	tl = tpslib_preload(tpslib_data, sizeof(tpslib_data));
	
	CuAssertPtrNotNull(tc, tl);
	
	return tl;
}

tpselt
TestCreateSampleDataImpl(CuTest* tc, struct tpslib *tl)
{
	tpselt elt, elt2;
	
	elt = te_new("test-element");
	CuAssertPtrNotNull(tc, elt);
	
	elt2 = te_new("example-item");
	CuAssertPtrNotNull(tc, elt2);
	
	te_setattrc(elt2, "name", "test value");
	
	te_attach(elt, elt2);
	
	elt2 = te_new("details");
	
	te_setattrc(elt2, "a", "something");
	te_setattrc(elt2, "b", "something else");
	te_setattrc(elt2, "c", "something more");
	te_setattrc(elt2, "d", "something interesting");
	
	te_attach(elt, elt2);
	
	return elt;
}

void TestLoadLibrary(CuTest* tc)
{
	CuSuite ts;
	CuSuiteInit(&ts);

	struct tpslib *tl = TestLoadLibraryImpl(tc);	

	if (tl != NULL)
		tpslib_dealloc(tl);
}

void TestLibraryCaps(CuTest* tc)
{
	int capIndex;
	const char *keyPtr;
	char *valPtr;
	size_t capSize;
	struct CSL_Dictionary *caps; 
	
	struct tpslib *tl = TestLoadLibraryImpl(tc);	
	
	CuSuite ts;
	CuSuiteInit(&ts);
	
	capIndex = 0;
	
	caps = tpslib_getcaps(tl);
	
	CuAssertPtrNotNull(tc, caps);
	
	if (caps != NULL) {
		
		CSL_DictionaryNext(caps, &capIndex, &keyPtr, &valPtr, &capSize);
		
		CuAssertStrEquals(tc, "testcap1", keyPtr);
		CuAssertStrEquals(tc, "yes", valPtr);
		
		CSL_DictionaryNext(caps, &capIndex, &keyPtr, &valPtr, &capSize);
		
		CuAssertStrEquals(tc, "testcap2", keyPtr);
		CuAssertStrEquals(tc, "no", valPtr);
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

void TestTPSPack(CuTest* tc)
{
	tpselt elt;
	struct test_pack_data data;

	enum tpsio_status status;
	
	CuSuite ts;
	CuSuiteInit(&ts);
	
	struct tpslib *tl = TestLoadLibraryImpl(tc);
	
	if (tl != NULL) {
		
		elt = TestCreateSampleDataImpl(tc, tl);
		
		if (elt != NULL) {
			
			data.data = NULL;
			data.size = 0;
			data.used = 0;
			
			status = tps_pack(tl, elt, test_pack_reader, &data);

			CuAssertIntEquals(tc, TIO_READY, status);
			
			te_dealloc(elt);
			
			if (data.data != NULL)
				nsl_free(data.data);
		}
		
		tpslib_dealloc(tl);
	}
}


void TestTPSUnpack(CuTest* tc)
{
	tpselt elt, elt_c1, elt_c2, elt_c3;
	struct test_pack_data data;
	
	enum tpsio_status status;
	
	struct tps_unpackstate *ups;
	ssize_t len;
	int iter;
	
	CuSuite ts;
	CuSuiteInit(&ts);
	
	struct tpslib *tl = TestLoadLibraryImpl(tc);
	
	if (tl != NULL) {
		
		elt = TestCreateSampleDataImpl(tc, tl);
		
		if (elt != NULL) {
			
			data.data = NULL;
			data.size = 0;
			data.used = 0;
			
			status = tps_pack(tl, elt, test_pack_reader, &data);
			
			CuAssertIntEquals(tc, TIO_READY, status);
			
			te_dealloc(elt);
			
			if (status == TIO_READY) {
				
				iter = 0;
				
				ups = tps_unpack_start(tl, data.used);
				len = tps_unpack_feed(ups, data.data, data.used);
				
				CuAssertIntEquals(tc, data.used, len);
				
				status = tps_unpack_result(ups, &elt);
				
				CuAssertIntEquals(tc, TIO_READY, status);
				CuAssertPtrNotNull(tc, elt);
				
				if (status == TIO_READY && elt != NULL)
				{
					elt_c1 = te_nextchild(elt, &iter);
					CuAssertPtrNotNull(tc, elt_c1);
					
					elt_c2 = te_nextchild(elt, &iter);
					CuAssertPtrNotNull(tc, elt_c2);
					
					elt_c3 = te_nextchild(elt, &iter);
					CuAssertPtrEquals(tc, NULL, elt_c3);
					
					CuAssertStrEquals(tc, "test-element", te_getname(elt));
					CuAssertStrEquals(tc, "example-item", te_getname(elt_c1));
					CuAssertStrEquals(tc, "details", te_getname(elt_c2));
					CuAssertStrEquals(tc, "test value", te_getattrc(elt_c1, "name"));

					CuAssertStrEquals(tc, "something", te_getattrc(elt_c2, "a"));
					CuAssertStrEquals(tc, "something else", te_getattrc(elt_c2, "b"));
					CuAssertStrEquals(tc, "something more", te_getattrc(elt_c2, "c"));
					CuAssertStrEquals(tc, "something interesting", te_getattrc(elt_c2, "d"));
					
					te_dealloc(elt);
				}
			}
			
			if (data.data != NULL)
				nsl_free(data.data);
		}
		
		tpslib_dealloc(tl);
	}
}

CuSuite* CuGetTPSSuite(void)
{
	CuSuite* suite = CuSuiteNew();
	
	SUITE_ADD_TEST(suite, TestLoadLibrary);
	SUITE_ADD_TEST(suite, TestLibraryCaps);
	SUITE_ADD_TEST(suite, TestTPSPack);
	SUITE_ADD_TEST(suite, TestTPSUnpack);
	
	return suite;
}
