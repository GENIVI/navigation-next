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

#ifndef __ATLASBOOK_PROTOCOL__
#define __ATLASBOOK_PROTOCOL__

#include "nbexp.h"
#include "Handler.h"

namespace  protocol{

class NB_DLL AtlasbookProtocol{

public:
	AtlasbookProtocol() {};
	virtual ~AtlasbookProtocol(){};

	//virtual shared_ptr<Handler<MetadataSourceParameters, MetadataSourceInformation> > GetMetadataSourceHandler() = 0;
	//virtual shared_ptr<Handler<MapTileSourceParameters, MapTileSourceInformation> > GetMapTileSourceHandler() = 0;
	//virtual shared_ptr<Handler<ReverseGeocodeParameters, ReverseGeocodeInformation> > AtlasbookProtocol::GetReverseGeocodeHandler() = 0;
	//virtual shared_ptr<Handler<GeocodeParameters, GeocodeInformation> > AtlasbookProtocol::GetHandler(auto_ptr<GeocodeParameters>) = 0;
	//virtual shared_ptr<Handler<QALogUploadParameters, QALogUploadInformation> > AtlasbookProtocol::GetHandler(auto_ptr<QALogUploadParameters>) = 0;
	//virtual shared_ptr<Handler<MessageOfTheDayParameters, MessageOfTheDayInformation> > AtlasbookProtocol::GetHandler(auto_ptr<MessageOfTheDayParameters>) = 0;
	//virtual shared_ptr<Handler<MessageOfTheDayConfirmationParameters, MessageOfTheDayConfirmationInformation> > AtlasbookProtocol::GetHandler(auto_ptr<MessageOfTheDayConfirmationParameters>) = 0;
	//virtual shared_ptr<Handler<NavRequestParameters, NavRequestInformation> > AtlasbookProtocol::GetNavRequestHandler() = 0;

};

}

#endif //__ATLASBOOK_PROTOCOL__