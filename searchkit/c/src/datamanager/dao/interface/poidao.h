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

	@file poidao.h
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
#ifndef POI_DAO_H
#define POI_DAO_H

#include <map>
#include "datamanagerdefs.h"
#include "SingleSearchInformation.h"

#define RECENT_POI_RECORD_COUNT_LIMIT           100
#define FAVORITE_POI_RECORD_COUNT_LIMIT         200
typedef enum
{
    POI_TYPE_NONE = 0 ,   /*!< unuse POI type*/
    POI_TYPE_RECENT,      /*!< recent POI*/
    POI_TYPE_FAVORITE     /*!< favorite POI*/
} DMPOIType;

typedef struct
{
    nbsearch::POI* poi;             /*!< POI for sync */
    DMSyncStatusType syncStatus;    /*!< sync status */
    uint32 timestamp;               /*!< local time stamp */
    int64 serverId;                 /*!< server id */
    uint32 index;                   /*!< only avalible to favorites, always be 0 for recents */
    uint32 internalId;              /*!< local internal id */
    void*  userData;                /*!< user data for poi */
    int    userDataSize;            /*!< The size of user */
    DMPOIType type;                 /*!< POI type */
} POIForSync;

class IPOIDAO
{
public:
    virtual ~IPOIDAO() {}
    /*! Initialize the database
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error Initialize() PURE;
    /*! Get POIs by type.The results will be stroed in result list.
     *
     *  User is responsible to delete the result.
     *  @param type poi type to save
     *  @param result vector to storage result
     *  @param userData user data
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    GetPOI(DMPOIType type, std::vector<nbsearch::POI*>& result, std::map<uint32, std::map<std::string, std::string> >& userData) PURE;

    /*! Get all poi and sync status for them.
     *
     *  The results will be stroed in result list.</br>
     *  User is responsible to delete the result.
     *  @param type poi type to save
     *  @param result vector to storage result
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    GetPOIForSync(DMPOIType type, std::vector<POIForSync>& result) PURE;

    /*! Get the count of POI records
     *
     *  @param type poi type to save
     *  @return Returns SQLITE_OK if succeed.
     *  @see IPOIDAO::GetPOICount
     */
    virtual PAL_DB_Error
    GetPOICount(DMPOIType type, uint32 &result) PURE;

    /*! Add a POI into database. if the POI already exists, it will update the record in database.
     *  @param type poi type to save
     *  @param result vector to storage result
     *  @param userData user data
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    AddPOI(nbsearch::POI const* poi, DMPOIType type, const std::map<std::string, std::string>& userData) PURE;

    /*! remove a POI from database.
     *
     *  @param POI poi to remove
     *  @param type poi type to save
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    RemovePOI(nbsearch::POI const* POI, DMPOIType type) PURE;

    /*! Remove a POI from database.
     *
     *  @param ServerId Serverid of poi to be removed
     *  @param type poi type to save
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    RemovePOI(int64 ServerId, DMPOIType type) PURE;

    /*! Remove all POIs.
     *
     *  @param
     *  @return Returns SQLITE_OK if succeed.
     *  @see RemovePOI
     */
    virtual PAL_DB_Error
    RemovePOIs(DMPOIType type) PURE;

    /*! Update a POI data
     *
     *  @param POI a valid pointer to a POI object
     *  @param type poi type to save
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    UpdatePOI(nbsearch::POI const* POI, DMPOIType type) PURE;

    /*! Get syncStatus of a POI data
     *
     *  @param POI a valid pointer to a POI object
     *  @param type poi type to save
     *  @param syncStatus syncstatus
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    GetPOISyncStatus(nbsearch::POI const* POI, DMPOIType type, DMSyncStatusType& syncStatus) PURE;

    /*! Update a POI syncStatus
     *
     *  @param POI a valid pointer to a POI object
     *  @param type poi type to save
     *  @param syncStatus syncstatus
     *  @param serverId serverid is only needed when syncStatus is None
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    UpdatePOISyncStatus(nbsearch::POI const* POI, DMPOIType type, DMSyncStatusType syncStatus, int64 serverId = 0) PURE;
    virtual PAL_DB_Error
    UpdatePOISyncStatus(uint32 locationId, DMPOIType type, DMSyncStatusType syncStatus,  int64 serverId) PURE;

    /*! Update a POI timestamp
     *
     *  @param POI a valid pointer to a POI object
     *  @param type poi type to save
     *  @param timestamp from server
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    UpdatePOITimeStamp(nbsearch::POI const* POI, DMPOIType type, uint32 timeStamp) PURE;

    /*! Move a POI to specified index
     *
     *  @param POI a valid pointer to a POI object
     *  @param index index user want to move to
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    UpdateFavoritePOIIndex(nbsearch::POI const* POI, uint32 index ) PURE;

    /*! Brief Get data as string
     *  @param the key
     *  @pram the value
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    GetStringUserData(const char* key, std::string& data) PURE;

    /*! Brief Set string data by key
     *  @param the key
     *  @pram the value
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    SetStringUserData(const char* key, const char* value) PURE;

    /*! Brief remove all datas in userdata
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    RemoveAllInUserData() PURE;

    /*! Brief Update favorite name.
     *  @param POI a valid pointer to a POI object
     *  @param type poi type to update
     *  @param name poi new name
     *  @return Returns SQLITE_OK if succeed.
     */
    virtual PAL_DB_Error
    UpdateFavoriteName(nbsearch::POI const* POI, DMPOIType type, std::string& name) PURE;

    /*! Brief Check the poi exist in db or not.
     *  @param POI a valid pointer to a POI object
     *  @param type poi type to update
     *  @return Returns SQLITE_OK if exist.
     */
    virtual PAL_DB_Error
    CheckPOIExist(nbsearch::POI const* POI, DMPOIType type) PURE;


    /*!
     * @brief Set the order of the poi if the poi exist.
     *
     * @param POI a valid pointer to a POI object.
     * @param index the order of favorite.
     *
     * @return Returns SQLITE_OK if exist.
     */
    virtual PAL_DB_Error
    SetFavoriteOrder(nbsearch::POI const* POI, uint32 index) PURE;
};

#endif

/*! @} */
