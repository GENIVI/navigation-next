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

#include <jni.h>
#include <nbgeocodeparameters.h>
#include "jniaddress.h"

enum GeocodeRequestType
{
	GEOCODE_REQUEST_ADDRESS = 0,
	GEOCODE_REQUEST_AIRPORT = 1,
	GEOCODE_REQUEST_FREEFORM = 2,
	GEOCODE_REQUEST_INTERSECTION = 3,
};

class JNINBGeocodeParameters
{
public:
	JNINBGeocodeParameters(JNIEnv*, jobject);
	~JNINBGeocodeParameters();
	GeocodeRequestType getRequestType(){return requestType;}
	NB_IterationCommand getCommand(){return iterationCommand;}
	JNIAddress* getAddress(){return address;}
	char* getAirport(){return airport;}
	char* getFreeformAddress(){return freeformAddress;}
	char* getCountry(){return country;}
	char* getCrossStreet(){return crossStreet;}
	int getSize() {return size;}
private:
	JNINBGeocodeParameters();
	void initFields(JNIEnv*, jobject);
	GeocodeRequestType requestType;
	NB_IterationCommand iterationCommand;
	JNIAddress* address;   // <- GEOCODE_REQUEST_ADDRESS, GEOCODE_REQUEST_INTERSECTION
	char* airport;            // <- GEOCODE_REQUEST_AIRPORT
	char* freeformAddress;			  // <- GEOCODE_REQUEST_FREEFORM
	char* country;
	char* crossStreet;        // <- GEOCODE_REQUEST_INTERSECTION
	int   size;
};
