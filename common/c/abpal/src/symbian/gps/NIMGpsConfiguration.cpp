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

#include <utf.h>
#include "palstdlib.h"
#include "NIMGpsConfiguration.h"

CNIMGpsConfiguration* CNIMGpsConfiguration::NewL(uint32 aConfigCount, const ABPAL_GpsConfig* aConfig)
	{
	CNIMGpsConfiguration* self = NewLC(aConfigCount, aConfig);
	CleanupStack::Pop(self);
	return self;
	}

CNIMGpsConfiguration* CNIMGpsConfiguration::NewLC(uint32 aConfigCount, const ABPAL_GpsConfig* aConfig)
	{
	CNIMGpsConfiguration* self = new (ELeave) CNIMGpsConfiguration();
	CleanupStack::PushL(self);
	self->ConstructL(aConfigCount, aConfig);
	return self;
	}

CNIMGpsConfiguration::CNIMGpsConfiguration()
	{
	iProviderType = ENIMGpsProviderTypeGpsId;
	}

CNIMGpsConfiguration::~CNIMGpsConfiguration()
	{
	delete [] iFileName;
	}

void CNIMGpsConfiguration::ConstructL(uint32 aConfigCount, const ABPAL_GpsConfig* aConfig)
	{
	for (uint32 i=0; i<aConfigCount; ++i)
		{
		const ABPAL_GpsConfig* config = aConfig + i;
		ParseL(config->name, config->value);
		}
	}

CNIMGpsConfiguration::TNIMGpsProviderType CNIMGpsConfiguration::ProviderType() const
	{
	return iProviderType;
	}

const char* CNIMGpsConfiguration::GpsFileName() const
	{
	return iFileName;
	}

void CNIMGpsConfiguration::ParseL(const char* aName, const char* aValue)
	{
	const char* KProviderType = "providertype";
	const char* KGpsFilename = "gpsfilename";

	if (nsl_strcmp(aName, KGpsFilename) == 0)
		{
		delete [] iFileName;
		iFileName = NULL;
		TInt len = nsl_strlen(aValue);
		iFileName = new (ELeave) char[len+1];
		nsl_memset(iFileName, 0, len+1);
		nsl_memcpy(iFileName, aValue, len);
		}
	else if (nsl_strcmp(aName, KProviderType) == 0)
		{
		ParseProviderType(aValue);
		}
	}

void CNIMGpsConfiguration::ParseProviderType(const char* aType)
	{
	const char* KGpsTypeEmu = "emu";
	const char* KGpsTypeGps = "gpsid";

	if (nsl_strcmp(aType, KGpsTypeEmu) == 0)
		{
		iProviderType = ENIMGpsProviderTypeEmu;
		}
	else if (nsl_strcmp(aType, KGpsTypeGps) == 0)
		{
		iProviderType = ENIMGpsProviderTypeGpsId;
		}
	}
