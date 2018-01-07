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

    @file     locationcachedb.h
    @date     07/12/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    LocationCacheDb class declaration
        This class manages Sqlite DB which contains location tiles.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#pragma once

#include "base.h"

extern "C" {
  #include "nberror.h"
  #include "palradio.h"
  #include "navpublictypes.h"
  #include "nblocationtypes.h"
  #include "paldb.h"
}

const int MAC_ADDR_LENGTH = 6;
// this is tile TimeToLive in unixepoch format
const uint32 DEFAULT_TILE_TIME_TO_LIVE = 604800;     // in seconds = 10080 minutes = 168 hours = 7 days
const uint32 MAX_DATABASE_DATA_SIZE = 102400;        // in bytes = 100 KB
const char DATABASE_VERSION[] = "1.0.2";

enum CellType
{
    CT_CDMA = 0,
    CT_GSM
};

typedef struct _CellIdRecord
{
    // Common parameters
    CellType cellType;            // Cell type
    double   antennaLatitude;     // Antenna latitude, in degrees
    double   antennaLongitude;    // Antenna longitude, in degrees
    uint16   antennaHEPE;         // Antenna location accuracy, in meters
    int32    antennaOrientation;  // Antenna orientation, in degrees
    int32    antennaOpening;      // Antenna opening, in degrees
    uint32   antennaMAR;          // Maximum antenna range, in meters

    // CDMA network parameters
    uint16 systemId;       // System ID
    uint16 networkId;      // Network ID
    uint32 baseStationId;  // Extended base station ID

    // GSM network parameters
    uint16 mobileCountryCode; // Mobile Country Code
    uint16 mobileNetworkCode; // Mobile Network Code
    uint16 locationAreaCode;  // Location Area Code
    uint16 cellId;            // Cell ID

} CellIdRecord;

typedef struct _WifiRecord
{
    uint8    mac[MAC_ADDR_LENGTH];  // Wifi AP MAC address
    uint16   errorRadius;           // Wifi AP error radius, in meters

    double   antennaLatitude;       // Antenna latitude, in degrees
    double   antennaLongitude;      // Antenna longitude, in degrees

} WiFiRecord;

typedef struct _LocationCacheDbConfiguration
{
    uint32    initialTileTimeToLive;        // tile time to live in seconds(default: 7 days)
    int32     maxDataSize;                  // data size limit in bytes(if not needed, pass -1)

      // Another parameters

} LocationCacheDbConfiguration;

typedef struct _TableVersionInfo
{
    // Caller must free this pointer.
    char* versionInfo;
} TableVersionInfo;


class LocationCacheDb : public Base
{
public:
    LocationCacheDb(PAL_Instance* pal);
    LocationCacheDb(PAL_Instance* pal, LocationCacheDbConfiguration config );

    ~LocationCacheDb(void);

    // New tile ID will be returned to "tileID" parameter
    // If tile with the same coordinates exists, then we'll delete found and create a new one
    NB_Error AddTile( uint32 tileX, uint32 tileY, const uint8* dbFragment, uint32 dbFragmentLength, uint32& tileID );
    NB_Error SetCoordinates( uint32 tileID, const PAL_RadioCDMAInfo& cdmaInfo, const NB_LS_Location& position );
    NB_Error SetCoordinates( uint32 tileID, const PAL_RadioGSMInfo& gsmInfo, const NB_LS_Location& position );

    NB_Error UpdateTimestamp( uint32 tileID );

    NB_Error ClearDatabase();
    NB_Error GetVersion( TableVersionInfo& tableInfo );

    // If record not found, FindMatches() returns NE_NOENT.
    NB_Error FindMatches( const PAL_RadioCDMAInfo& cdmaInfo, CellIdRecord& cellIdRecord, uint32& tileID, NB_LS_Location& position );
    NB_Error FindMatches( const PAL_RadioGSMInfo& gsmInfo, CellIdRecord& cellIdRecord, uint32& tileID, NB_LS_Location& position );

    NB_Error FindMatches( const PAL_RadioWifiInfo& wifiInfo, WiFiRecord& wifiRecord, uint32& tileID );

private:
    PAL_DBConnect*  m_connection;
    PAL_Instance*   m_pal;
    LocationCacheDbConfiguration m_config;   // database parameters
    char *m_EncryptionKey;

    NB_Error PrepareDatabase();              // Open database file and try to create all tables.
    NB_Error RemoveExpiredTiles();           // Delete tiles with expired time to live
    NB_Error UpgradeDatabase();              // Upgrade to use new database version
    NB_Error GenerateEncryptionKey();
    // parse dbFragment and write data into appropriate tables
    NB_Error ParseAndWriteDBFragment( const uint8* dbFragment, uint32 dbFragmentLength, uint32 tileID );
};

/*! @} */
