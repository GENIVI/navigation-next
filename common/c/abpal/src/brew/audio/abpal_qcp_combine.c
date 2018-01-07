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

/*-
 * qcp_combine.c: created 2004/08/19 by Mark Goddard.
 *
 * Copyright (c) 2003 Networks In Motion, Inc.
 * All rights reserved.  This file and associated materials are the
 * trade secrets, confidential information, and copyrighted works of
 * Networks In Motion, Inc.
 *
 * This intellectual property is for the internal use only by Networks
 * In Motion, Inc.  This source code contains proprietary information
 * of Networks In Motion, Inc., and shall not be used, copied by, or
 * disclosed to, anyone other than a Networks In Motion, Inc.,
 * certified employee that has written authorization to view or modify
 * said source code.
 *
 */

#include "palstdlib.h"
#include "abpal_qcp_combine.h"
#include "audbuf.h"

static uint32
get_uint32(byte* pdata)
{
	uint32 retval;

	nsl_memcpy(&retval, pdata, 4);

	return retval;
}

static void
set_uint32(byte* pdata, uint32 val)
{
	nsl_memcpy(pdata, &val, 4);
}

static void
set_uint16_be(byte* pdata, uint16 val)
{
	uint16 val_be = nsl_htons(val);

	nsl_memcpy(pdata, &val_be, 2);
}

static void
set_uint32_be(byte* pdata, uint32 val)
{
	uint32 val_be = nsl_htonl(val);

	nsl_memcpy(pdata, &val_be, 4);
}

static PAL_Error
find_riff_chunk(const char* type, byte* pdata, uint32 size, byte** ppchunkdata, uint32* pchunksize)
{
	byte* ppos;

	if (pdata == NULL)
		return PAL_Failed;

	for (ppos = pdata; ppos < (pdata + size - 8); ppos += 2) {

		if (nsl_memcmp(ppos, type, 4) == 0) {

			*pchunksize = get_uint32(ppos+4);

			*ppchunkdata = (byte*) nsl_malloc(*pchunksize);

			if (*ppchunkdata == NULL) 
				return PAL_ErrNoMem;

			if (size-((ppos+8)-pdata) >= *pchunksize)
				nsl_memcpy(*ppchunkdata, ppos+8, *pchunksize);
			else
				return PAL_ErrBadParam;

			return PAL_Ok;
		}
	}

	*ppchunkdata = NULL;
	*pchunksize = 0;

	return PAL_Ok;
}

abpal_qcp_combine* 
abpal_qcp_combine_create()
{
	abpal_qcp_combine* qcp = (abpal_qcp_combine*) nsl_malloc(sizeof(abpal_qcp_combine));

	if (!qcp)
		return NULL;

	qcp->fmt_data = NULL;
	qcp->fmt_size = 0;

	qcp->vrat_data = NULL;
	qcp->vrat_size = 0;
	
	if ( AudioBufferNew(&qcp->data, 1024) != PAL_Ok )
	{
		nsl_free(qcp);
		return NULL;
	}

	return qcp;
}

PAL_Error
abpal_qcp_combine_destroy(abpal_qcp_combine* qcp)
{
	if (qcp->fmt_data != NULL)
		nsl_free(qcp->fmt_data);

	if (qcp->vrat_data != NULL)
		nsl_free(qcp->vrat_data);

	AudioBufferDelete(&qcp->data);

	return PAL_Ok;
}

PAL_Error	
abpal_qcp_combine_add(abpal_qcp_combine* qcp, byte* pdata, uint32 nsize)
{
	PAL_Error err = PAL_Ok;

	byte*		fmt_data = NULL;
	uint32	fmt_size = 0;

	byte*		vrat_data = NULL;
	uint32	vrat_size = 0;

	byte*		data_data = NULL;
	uint32	data_size = 0;

	err = find_riff_chunk("fmt ", pdata, nsize, &fmt_data, &fmt_size);

	if (err != PAL_Ok)
		goto errexit;

	err = find_riff_chunk("vrat", pdata, nsize, &vrat_data, &vrat_size);

	if (err != PAL_Ok)
		goto errexit;

	err = find_riff_chunk("data", pdata, nsize, &data_data, &data_size);

	if (err != PAL_Ok)
		goto errexit;

	if (fmt_data != NULL && data_data != NULL) {

		if (qcp->fmt_data != NULL && qcp->vrat_data != NULL) {

			if (qcp->fmt_size != fmt_size || nsl_memcmp(qcp->fmt_data, fmt_data, fmt_size) != 0) {
				
				err = PAL_ErrBadParam;
				goto errexit;
			}

			set_uint32(qcp->vrat_data+4, get_uint32(qcp->vrat_data+4) + get_uint32(vrat_data+4));
		}
		else {

			if (qcp->fmt_data != NULL || qcp->vrat_data != NULL) {

				err = PAL_ErrBadParam;
				goto errexit;
			}

			qcp->fmt_data = fmt_data;
			qcp->fmt_size = fmt_size;
			qcp->vrat_data = vrat_data;
			qcp->vrat_size = vrat_size;

			fmt_data = NULL;
			fmt_size = 0;
			vrat_data = NULL;
			vrat_size = 0;
		}
		
		AudioBufferCat(&qcp->data, data_data, data_size);

		err = AudioBufferError(&qcp->data);
	}
	else {

		err = PAL_ErrBadParam;
	}

errexit:

	if (data_data != NULL)
		nsl_free(data_data);

	if (fmt_data != NULL)
		nsl_free(fmt_data);

	if (vrat_data != NULL)
		nsl_free(vrat_data);

	return err;
}

PAL_Error	
abpal_qcp_combine_getdata_qcp(abpal_qcp_combine* qcp, byte** ppdata, uint32* pnsize)
{
	uint32 nsize;
	byte* ppos;

	if (AudioBufferLen(&qcp->data) == 0)
		return PAL_Failed;

	nsize = /* RIFF Header */	12 + 
			/* FMT Header */	8 + 
			/* FMT Data */		qcp->fmt_size + 
			/* VRAT Header */	8 + 
			/* VRAT Data */		qcp->vrat_size + 
			/* DATA Header */	8 + 
			/* DATA */			AudioBufferLen(&qcp->data) + 
			/* Padding */		AudioBufferLen(&qcp->data) % 2;

	*ppdata = (byte*) nsl_malloc(nsize);

	if (*ppdata == NULL)
		return PAL_ErrNoMem;
	
	ppos = *ppdata;

	/* RIFF Header */
	nsl_memcpy(ppos, "RIFF", 4);								ppos += 4;
	set_uint32(ppos, nsize - 8);							ppos += 4;
	nsl_memcpy(ppos, "QLCM", 4);								ppos +=	4;

	/* FMT Header */
	nsl_memcpy(ppos, "fmt ", 4);								ppos +=	4;
	set_uint32(ppos, qcp->fmt_size);						ppos += 4;

	/* FMT Data */
	nsl_memcpy(ppos, qcp->fmt_data, qcp->fmt_size);				ppos += qcp->fmt_size;

	/* VRAT Header */
	nsl_memcpy(ppos, "vrat", 4);								ppos +=	4;
	set_uint32(ppos, qcp->vrat_size);						ppos += 4;

	/* VRAT Data */
	nsl_memcpy(ppos, qcp->vrat_data, qcp->vrat_size);			ppos += qcp->vrat_size;

	/* DATA Header */
	nsl_memcpy(ppos, "data", 4);								ppos +=	4;
	set_uint32(ppos, AudioBufferLen(&qcp->data));					ppos += 4;

	/* DATA */
	nsl_memcpy(ppos, AudioBufferGet(&qcp->data), AudioBufferLen(&qcp->data));	ppos += AudioBufferLen(&qcp->data);

	/* Padding */
	if (AudioBufferLen(&qcp->data) % 2)
		*ppos = 0;

	*pnsize = nsize;

	return PAL_Ok;
}	

size_t
abpal_qcp_combine_append_audio(abpal_qcp_combine* qcp, byte* pdata, size_t event, size_t packets, boolean last)
{
	byte* ppos = pdata;
	const byte* data = AudioBufferGet(&qcp->data);

	*ppos = (event == 0) ? 0 : 50;					ppos += 1;
	*ppos = 0xFF;									ppos += 1;
	*ppos = 0xF1;									ppos += 1;
	set_uint16_be(ppos, (uint16) (7 + packets*35));	ppos += 2;
	*ppos = 0;										ppos += 1;
	*ppos = 0x44;									ppos += 1;
	set_uint32_be(ppos, last ? 0 : 887);			ppos += 4;
	*ppos = (event == 0) ? 0 : 1;					ppos += 1;
	nsl_memcpy(ppos, data + event*875, packets*35);		ppos += packets*35;

	return 12 + packets*35;
}

PAL_Error	
abpal_qcp_combine_getdata_cmx(abpal_qcp_combine* qcp, byte** ppdata, uint32* pnsize)
{
	uint32 nsize;
	byte* ppos;

	size_t datalen = AudioBufferLen(&qcp->data);
	const byte* data = AudioBufferGet(&qcp->data);
	size_t numpackets;
	size_t packet;
	size_t event;
	size_t numfullaudioevents;
	size_t partialeventpackets;
	size_t trackchunklength;
	byte dt;

	if (datalen == 0)
		return PAL_Failed;

	/* Make sure that the data size is a even number of packets */
	if ((datalen % 35) != 0)
		return PAL_ErrBadParam;

	numpackets = datalen / 35;

	for (packet = 0; packet < numpackets; packet++) {

		if (data[packet*35] != 0x04)
			return PAL_ErrBadParam;
	}

	numfullaudioevents		=	numpackets / 25;
	partialeventpackets		=	numpackets % 25;

	trackchunklength		=	/* Full Audio Events */		887 * numfullaudioevents + 
								/* Partial Audio Event */	(partialeventpackets > 0 ? 12 : 0) + 
															partialeventpackets * 35 +
								/* "End" Message */			4;

	nsize = /* CMF File ID */			4 + 
			/* CMF File Length */		4 +
			/* CMF Header */			32 + 
			/* Trac Chunk Header */		8 +
			/* Trac Chunk Data */		trackchunklength;

	*ppdata = (byte*) nsl_malloc(nsize);

	if (*ppdata == NULL)
		return PAL_ErrNoMem;
	
	ppos = *ppdata;

	/* CMF File ID */
	nsl_memcpy(ppos, "cmid", 4);				ppos += 4;

	/* CMF File Length */
	set_uint32_be(ppos, nsize - 8);			ppos += 4;

	/* CMF Header */
	set_uint16_be(ppos, 30);				ppos += 2;
	*ppos = 2;								ppos += 1;
	*ppos = 2;								ppos += 1;
	*ppos = 1;								ppos += 1;
	nsl_memcpy(ppos, "vers", 4);				ppos += 4;
	set_uint16_be(ppos, 4);					ppos += 2;
	nsl_memcpy(ppos, "0201", 4);				ppos += 4;
	nsl_memcpy(ppos, "wave", 4);				ppos += 4;
	set_uint16_be(ppos, 1);					ppos += 2;
	*ppos = 1;								ppos += 1;
	nsl_memcpy(ppos, "cnts", 4);				ppos += 4;
	set_uint16_be(ppos, 4);					ppos += 2;
	nsl_memcpy(ppos, "WAVE", 4);				ppos += 4;

	/* Trac Chunk Header */
	nsl_memcpy(ppos, "trac", 4);				ppos += 4;
	set_uint32_be(ppos, trackchunklength);	ppos += 4;
	
	/* Partial Audio Events */
	for (event = 0; event < numfullaudioevents; event++) {

		ppos += abpal_qcp_combine_append_audio(qcp, ppos, event, 25, (partialeventpackets == 0 && event+1==numfullaudioevents) ? TRUE : FALSE);
	}

	/* Partial Audio Events */
	if (partialeventpackets > 0) {
		ppos += abpal_qcp_combine_append_audio(qcp, ppos, event, partialeventpackets, TRUE);
		dt = (byte) ((partialeventpackets * 50)/35);
	}
	else 
		dt = 50;

	/* "End" Message */
	*ppos = dt;										ppos += 1;
	*ppos = 0xFF;									ppos += 1;
	*ppos = 0xDF;									ppos += 1;
	*ppos = 0x00;									ppos += 1;

	*pnsize = nsize;

	return PAL_Ok	;
}	

PAL_Error	
abpal_qcp_combine_getdata(abpal_qcp_combine* qcp, abpal_qcp_combine_data_type type, byte** ppdata, uint32* pnsize)
{
	if (type == getdata_qcp) //AB FALSE && 
		return abpal_qcp_combine_getdata_qcp(qcp, ppdata, pnsize);
	else if (type == getdata_cmx) //AB TRUE || 
		return abpal_qcp_combine_getdata_cmx(qcp, ppdata, pnsize);
	return PAL_Ok;
}