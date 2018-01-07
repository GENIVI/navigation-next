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

	@file POIDBDAO.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#pragma once
#ifndef POI_DBDAO_H
#define POI_DBDAO_H

#include <map>
#include "poidao.h"
#include "smartpointer.h"
#include "SingleSearchInformation.h"

class DBReader;
class DBCommand;
class DBConnection;
class DBManager;
namespace nbsearch
{
    class POIImpl;
}

class POIDBDAO
    : public IPOIDAO
{
public:
    POIDBDAO(shared_ptr<DBManager> manager);
    virtual ~POIDBDAO();
    virtual PAL_DB_Error Initialize();
    virtual PAL_DB_Error GetPOI(DMPOIType type,std::vector<nbsearch::POI*>& result, std::map<uint32, std::map<std::string, std::string> >& userData);
    virtual PAL_DB_Error GetPOI(const char* name, DMPOIType type, std::vector<nbsearch::POI*>& result);
    virtual PAL_DB_Error GetPOIForSync(DMPOIType type, std::vector<POIForSync>& result);
    virtual PAL_DB_Error GetPOICount(DMPOIType type, uint32 &result);
    virtual PAL_DB_Error AddPOI(nbsearch::POI const* poi, DMPOIType type, const std::map<std::string, std::string>& userData);
    virtual PAL_DB_Error RemovePOI(nbsearch::POI const* POI, DMPOIType type);
    virtual PAL_DB_Error RemovePOI(int64 serverId, DMPOIType type);
    virtual PAL_DB_Error RemovePOIs(DMPOIType type);
    virtual PAL_DB_Error UpdatePOI(nbsearch::POI const* POI, DMPOIType type );
    virtual PAL_DB_Error GetPOISyncStatus(nbsearch::POI const* POI, DMPOIType type,  DMSyncStatusType& syncStatus);
    virtual PAL_DB_Error UpdatePOISyncStatus(nbsearch::POI const* POI, DMPOIType type, DMSyncStatusType syncStatus,  int64 serverId);
    virtual PAL_DB_Error UpdatePOISyncStatus(uint32 locationId, DMPOIType type, DMSyncStatusType syncStatus,  int64 serverId);
    virtual PAL_DB_Error UpdatePOITimeStamp(nbsearch::POI const* POI, DMPOIType type, uint32 timeStamp);
    virtual PAL_DB_Error UpdateFavoritePOIIndex(nbsearch::POI const* POI, uint32 index);
    virtual PAL_DB_Error GetStringUserData(const char* key, std::string& data);
    virtual PAL_DB_Error SetStringUserData(const char* key, const char* value);
    virtual PAL_DB_Error RemoveAllInUserData();
    virtual PAL_DB_Error UpdateFavoriteName(nbsearch::POI const* POI, DMPOIType type, std::string& name);
    virtual PAL_DB_Error CheckPOIExist(nbsearch::POI const* POI, DMPOIType type);
    virtual PAL_DB_Error SetFavoriteOrder(nbsearch::POI const* POI, uint32 index);
private:
    PAL_DB_Error GetPOISyncStatus(DBConnection* pConnection, uint32 internalId, DMPOIType type, DMSyncStatusType& syncStatus);
    PAL_DB_Error UpdatePOISyncStatus(DBConnection* pConnection, uint32 internalId, DMPOIType type, DMSyncStatusType syncStatus, int64 serverId = 0);
    PAL_DB_Error UpdatePOITimeStamp(DBConnection* pConnection, uint32 internalId, DMPOIType type, uint32 timeStamp);
    PAL_DB_Error UpdateFavoritePOIIndex(DBConnection* pConnection, uint32 internalId, uint32 index );
    PAL_DB_Error ExtractAPOI(DBReader* pReader, nbsearch::POIImpl* &result, uint32& internalId, DMSyncStatusType& syncStatus, uint32& timeStamp, int64& serverId, std::map<std::string, std::string>& userdata);
    PAL_DB_Error ExtractACategory(DBReader* pReader, nbsearch::POIImpl* POI, uint32 internalId);
    PAL_DB_Error ExtractAPhoneNumber(DBReader* pReader, nbsearch::POIImpl* POI, uint32 internalId);
    PAL_DB_Error GetAllMatchedCategories(DBConnection* pConnection, DMPOIType type,         nbsearch::POIImpl* POI, uint32 internalId);
    PAL_DB_Error GetAllMatchedPhoneNumbers(DBConnection* pConnection, DMPOIType type, nbsearch::POIImpl *POI, uint32 internalId);
    uint32 CheckDuplicatePOI(DBConnection* pConnection,nbsearch::POI const* pPOI, DMPOIType type);
    uint32
    CheckDuplicatePOI(DBConnection* pConnection, int64 serverId, DMPOIType type);
    PAL_DB_Error SetAllPOIParam(DBCommand& cmd, nbsearch::POI const* pPOI, DMSyncStatusType syncStatus, uint32 timeStamp, const std::map<std::string, std::string>* userData);
    PAL_DB_Error SetAllPhoneParam( DBCommand& cmd, uint32 locationID, nbsearch::Phone const* pPhone);
    PAL_DB_Error SetAllCategoryParam( DBCommand& cmd, uint32 internalID, nbsearch::Category const* pCategory);
    PAL_DB_Error UpdateAPOI(DBConnection* pConnection, uint32 internalID,
      nbsearch::POI const* pPOI, uint32 timeStamep, DMPOIType type, DMSyncStatusType syncStatus, const std::map<std::string, std::string>* userData);
    PAL_DB_Error InsertPOICategories(DBConnection* pConnection,uint32 internalID,  nbsearch::POI const* pPOI, DMPOIType type);
    PAL_DB_Error InsertPOIPhoneNumbers(DBConnection* pConnection, uint32 internalID, nbsearch::POI const* pPOI, DMPOIType type);
    PAL_DB_Error ExecuteOneLineCommand(DBConnection* pConnetion, const char* command);
    PAL_DB_Error GetFavoriteOrderIndex(DBConnection* pConnection, uint32 internalId, uint32& index);
    PAL_DB_Error SetFavoriteOrder(DBConnection* pConnection, uint32 locationId, uint32 index);

    PAL_DB_Error GetBinaryUserData(const char* key, void **result, int& size);
    PAL_DB_Error SetBinaryUserData(const char* key, void* data, uint32 size);

    shared_ptr<DBManager> m_dbmanager;
};

#endif

/*! @} */
