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

#include "palstdlib.h"
#include "paldebuglog.h"
#include "GpsSingleton.h"

CGpsSingleton* CGpsSingleton::NewL()
	{
	CGpsSingleton* self = CGpsSingleton::NewLC();
	CleanupStack::Pop(self);
	return self;
	}

CGpsSingleton* CGpsSingleton::NewLC()
	{
	CGpsSingleton* self = new (ELeave) CGpsSingleton();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CGpsSingleton::CGpsSingleton()
	{
	}

CGpsSingleton::~CGpsSingleton()
	{
	if (iPositionServerConnectedCount > 0)
		{
		iPositionServer.Close();
		}
	iPositionServerConnectedCount = 0;
	iGpsFileOffset = 0;
	delete [] iGpsFilename;
	iGpsFilename = NULL;
	}

void CGpsSingleton::ConstructL()
	{
	}

RPositionServer& CGpsSingleton::PositionServer()
	{
	return iPositionServer;
	}

const char* CGpsSingleton::GpsFileName() const
	{
	return iGpsFilename;
	}

TInt CGpsSingleton::GpsFileOffset() const
	{
	return iGpsFileOffset;
	}

void CGpsSingleton::ConnectPositionServerL()
	{
	debugf("ConnectPositionServerL In, current count = %d", iPositionServerConnectedCount);
	if (iPositionServerConnectedCount == 0)
		{
		User::LeaveIfError(iPositionServer.Connect());
		}
	++iPositionServerConnectedCount;
	}

void CGpsSingleton::ReleasePositionServer()
	{
	debugf("ReleasePositionServer In, current count = %d", iPositionServerConnectedCount);
	--iPositionServerConnectedCount;
	if (iPositionServerConnectedCount == 0)
		{
		iPositionServer.Close();
		}
	}

void CGpsSingleton::SetGpsFileL(const char* aGpsFilename)
	{
	TInt length = nsl_strlen(aGpsFilename);
	char* name = new (ELeave) char[length + 1];
	nsl_memset(name, 0, length + 1);
	nsl_memcpy(name, aGpsFilename, length);
	delete [] iGpsFilename;
	iGpsFilename = name;
	}

void CGpsSingleton::SetGpsFileOffset(TInt aOffset)
	{
	iGpsFileOffset = aOffset;
	}
