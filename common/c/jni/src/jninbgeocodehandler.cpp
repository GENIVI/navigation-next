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

#include "jninbgeocodehandler.h"
#include "jninbgeocodeinformation.h"
#include "paltaskqueue.h"
#include "logutil.h"

JNINBGeocodeHandler::JNINBGeocodeHandler(JNINBContext* jniNBContext)
{
	this->listener = 0;
	this->nb_geocodehandler = 0;
	this->jniNBContext = jniNBContext;
}

JNINBGeocodeHandler::~JNINBGeocodeHandler()
{
	delete this->listener;
}

void JNINBGeocodeHandler::setListener(JNINBHandlerListener* listener)
{
	this->listener = listener;
}

void JNINBGeocodeHandler::startRequest(JNINBGeocodeParameters *parameters)
{
	LOGI("---> JNINBGeocodeHandler::startRequest\n");

	this->jniNBGeocodeParameters = parameters;

    // Schedule test function call
    TaskId tid = 0;
	PAL_Error err = PAL_EventTaskQueueAdd(jniNBContext->pal_instance, startRequestCallback, this, &tid);

	LOGI("JNINBGeocodeHandler::startRequest err=%u, tid=%u\n", err, tid);

	if (err)
	{
		this->listener->notify(REQUEST_ERROR, NULL);
	}

	LOGI("<--- JNINBGeocodeHandler::startRequest\n");
}

void JNINBGeocodeHandler::startRequestCallback(PAL_Instance* pal, void* userData) {
	LOGI("---> JNINBGeocodeHandler::startRequestCallback\n");

	JNINBGeocodeHandler* me = static_cast<JNINBGeocodeHandler*>(userData);
    if (me)
    {
		me->doStartRequest();
    }

	LOGI("<--- JNINBGeocodeHandler::startRequestCallback\n");
}

void JNINBGeocodeHandler::doStartRequest()
{
	LOGI("---> JNINBGeocodeHandler::doStartRequest\n");

	// Create CCC Geocode Handler
    NB_Error err = NE_OK;
	NB_RequestHandlerCallback callback = { requestHandlerCallback, this};

	err = NB_GeocodeHandlerCreate(this->jniNBContext->nb_context, &callback, &nb_geocodehandler);

	if (err || nb_geocodehandler == 0)
	{
		listener->notify(REQUEST_ERROR, NULL);
		return;
	}

	LOGI("JNINBGeocodeHandler::doStartRequest creating parameters\n");
	NB_GeocodeParameters* parameters = 0;
	switch (jniNBGeocodeParameters->getRequestType())
	{
	case GEOCODE_REQUEST_ADDRESS:
		err = NB_GeocodeParametersCreateAddress(
			this->jniNBContext->nb_context,
			&jniNBGeocodeParameters->getAddress()->toNB_Address(),
			jniNBGeocodeParameters->getSize(),
			&parameters);
		break;
	case GEOCODE_REQUEST_AIRPORT:
		err = NB_GeocodeParametersCreateAirport(
			this->jniNBContext->nb_context,
			jniNBGeocodeParameters->getAirport(),
			jniNBGeocodeParameters->getSize(),
			&parameters);
		break;
	case GEOCODE_REQUEST_FREEFORM:
		err = NB_GeocodeParametersCreateFreeForm(
			this->jniNBContext->nb_context,
			jniNBGeocodeParameters->getFreeformAddress(),
			jniNBGeocodeParameters->getCountry(),
			jniNBGeocodeParameters->getSize(),
			&parameters);
		break;
	case GEOCODE_REQUEST_INTERSECTION:
		err = NB_GeocodeParametersCreateIntersection(
			this->jniNBContext->nb_context,
			&jniNBGeocodeParameters->getAddress()->toNB_Address(),
			jniNBGeocodeParameters->getCrossStreet(),
			jniNBGeocodeParameters->getSize(),
			&parameters);
		break;
	}

	LOGI("JNINBGeocodeHandler::doStartRequest parameters created err=%u\n", err);
	if (err || parameters == 0)
	{
		listener->notify(REQUEST_ERROR, NULL);
		return;
	}

	// On request started
	listener->notify(REQUEST_STARTED, NULL);
	LOGI("JNINBGeocodeHandler::doStartRequest starting request\n");
	err = NB_GeocodeHandlerStartRequest(nb_geocodehandler, parameters);
	LOGI("JNINBGeocodeHandler::doStartRequest request started err=%u\n", err);

	if (err)
	{
		NB_GeocodeParametersDestroy(parameters);
		listener->notify(REQUEST_ERROR, NULL);
		return;
	}

	err = NB_GeocodeParametersDestroy(parameters);

	LOGI("<--- JNINBGeocodeHandler::doStartRequest\n");
}

/*! Geocode request handler callback

Request handler callback.  Reports progress, gets information on success and
signals that the request has completed

@param handler Handler invoking the callback
@param status Request status
@param err Error state
@param up Non-zero if progress report is for server query; zero for server reply
@param percent Percent of data transferred up or down
@param userData User data provided to request handler
*/
void JNINBGeocodeHandler::requestHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
	LOGI("---> JNINBGeocodeHandler::requestHandlerCallback\n");
    JNINBGeocodeHandler* me = static_cast<JNINBGeocodeHandler*>(userData);
    if (me)
    {
		me->doHandleRequestCallback(handler, status, err, up, percent);
    }
	LOGI("<--- JNINBGeocodeHandler::requestHandlerCallback\n");
}

void JNINBGeocodeHandler::doHandleRequestCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err0, uint8 up, int percent)
{
	LOGI("---> JNINBGeocodeHandler::doHandleRequestCallback status=%u, err0=%u, up=%u, percent=%u\n", status, err0, up,percent);
	switch (status)
	{
	case NB_NetworkRequestStatus_Success:
		{
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback SUCCESS\n");

			NB_GeocodeInformation* g_Information = 0;
			NB_Error err = NB_GeocodeHandlerGetGeocodeInformation((NB_GeocodeHandler *)handler, &g_Information);
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback got GeocodeInformation err=%u, p=%d\n", err, g_Information);

			err = NB_GeocodeHandlerDestroy((NB_GeocodeHandler *)handler);
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback destroyed handler err=%u\n", err);

			LOGI("JNINBGeocodeHandler::doHandleRequestCallback creating JNI Geocode Information\n");
			JNINBGeocodeInformation* jInformation = new JNINBGeocodeInformation(jniNBContext, g_Information);
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback created JNI Geocode Information jInformation=%d\n", jInformation);

			jobject res = jInformation->getJGeocodeInformation();

			err = NB_GeocodeInformationDestroy(g_Information);
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback destroyed geocode info err=%u\n", err);

			this->listener->notify(REQUEST_COMPLETED, res);
		}
		break;
	case NB_NetworkRequestStatus_Canceled:
		{
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback CANCELED\n");
			NB_Error err = NB_GeocodeHandlerDestroy((NB_GeocodeHandler *)handler);
			LOGI("doHandleRequestCallback::JNINBGeocodeHandler destroyed handler err=%u\n", err);
			this->listener->notify(REQUEST_CANCELED, NULL);
		}
		break;
	case NB_NetworkRequestStatus_Failed:
		{
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback FAILED\n");
			NB_Error err = NB_GeocodeHandlerDestroy((NB_GeocodeHandler *)handler);
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback destroyed handler err=%u\n", err);
			this->listener->notify(REQUEST_ERROR, NULL);
		}
		break;
	case NB_NetworkRequestStatus_TimedOut:
		{
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback TIMEDOUT\n");
			NB_Error err = NB_GeocodeHandlerDestroy((NB_GeocodeHandler *)handler);
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback destroyed handler err=%u\n", err);
			this->listener->notify(REQUEST_TIMEDOUT, NULL);
		}
		break;
	case NB_NetworkRequestStatus_Progress:
		{
			LOGI("JNINBGeocodeHandler::doHandleRequestCallback PROGRESS\n");
			this->listener->notify(REQUEST_PROGRESS, NULL);
		}
		break;
	}

	LOGI("<--- JNINBGeocodeHandler::doHandleRequestCallback\n");
}