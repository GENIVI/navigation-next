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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#ifndef NCDB_POIS_H
#define NCDB_POIS_H


#include "NcdbTypes.h"

namespace Ncdb {

class Session;

//! Database POI Search Class
class NCDB_API Poi
{
public:
	//! Constructor
	Poi(Session &Session);
	//! This API will return the list of Cat ID stored in the DB
	int GetCatIDs();
	//! This API will return the sub catIDs for a given CatID in a given DB
	int SubCatIDs();
	//! This API will return a Cat name for a given Cat ID in a given language.
	int GetCatName();
	//! This API will return a Sub cat name for a given Sub Cat ID in a given language.
	int GetSubCatName();
	//! This API will begin POI search session for a given sub-string, Lat/Long, Cat ID and sub catID are optional
	int BeginPoiSearch();
	//! This API will get the next API that closely matches the condition in item 83 above.
	int GetNextPOI();
	//! This API will start a session to get POIs along the route. This will take as input routelist, route index, current pos, distance, Cat[], sub-catID[]
	int BeginSearchPOIAlongRoute();
	//! This API will get the next POI along the route.
	int GetNextPOIAlongRoute();
	//! This will close the POI search along route.
	int ClosePoisearchAlongRoute();
	//! This API will close the POI search.
	int ClosePoiSearch();

private:
	//! The database to access.
	Session &m_Session;

};


};
#endif // NCDB_POIS_H
