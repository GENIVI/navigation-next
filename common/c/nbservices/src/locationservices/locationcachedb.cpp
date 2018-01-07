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

    @file     locationcachedb.cpp
    @date     07/12/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        LocationCacheDb class implementation

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

#include "locationcachedb.h"

extern "C" {
  #include "paldb.h"
  #include "cslaes.h"
  #include "paldberror.h"
  #include "palclock.h"
  #include "nbgpstypes.h"
}


const char* DATABASE_NAME = "LocationCacheDb.db3";
const uint16 DEFAULT_BLOCK_SIZE = 16;

#define NE_FAIL NE_API //NB_Error doesn't have got NE_FAIL - general fail

// convert byte order from big endian to used on this platform
#define BE_UINT16_TO_CPU(x) nsl_ntohs( (x) )
#define BE_UINT32_TO_CPU(x) nsl_ntohl( (x) )
#define BE_UINT64_TO_CPU( x, y )                          \
  do {                                                    \
      PACKED struct INT64 {                               \
        uint32 p1;                                        \
        uint32 p2;                                        \
      } *val1 = NULL, *val2 = NULL;                       \
      uint32 tmp_val = 1;                                 \
      val1 = reinterpret_cast< struct INT64* >( &x );     \
      val2 = reinterpret_cast< struct INT64* >( &y );     \
      if (0 !=                                            \
          *reinterpret_cast<unsigned char* >( &tmp_val ) )\
      {                                                   \
          tmp_val = nsl_ntohl( val1->p2 );                \
          val2->p2 = nsl_ntohl( val1->p1 );               \
          val2->p1 = tmp_val;                             \
      }                                                   \
      else{                                               \
          *val2 = *val1;                                  \
      }                                                   \
  } while(0)

#define ENCRYPTED_DATA_SIZE( DATA_TYPE )                  \
  ( sizeof( DATA_TYPE )%DEFAULT_BLOCK_SIZE ) ? ( ( ( sizeof( DATA_TYPE )/DEFAULT_BLOCK_SIZE ) + 1 ) * DEFAULT_BLOCK_SIZE ) : sizeof( DATA_TYPE )


#pragma pack (push, 1) // without it sizeof( TileDBFragmentHeader ) = 12 and sizeof( CellRecordStructure ) = 40
// this structure describes header information that is placed in beginning of dbFragment
typedef PACKED struct{
    uint8 cellIDType;
    uint16 cellidOffset;
    uint16 numCellid;
    uint16 wifiOffset;
    uint16 numWiFi;
    uint16 tileSize;
} TileDBFragmentHeader;

// this structures are used to get data from byte array
typedef PACKED struct {
    double   antennaLatitude;     // Antenna latitude, in degrees
    double   antennaLongitude;    // Antenna longitude, in degrees
    uint16   antennaHEPE;         // Antenna location accuracy, in meters
    int32    antennaOrientation;  // Antenna orientation, in degrees
    int32    antennaOpening;      // Antenna opening, in degrees
    uint32   antennaMAR;          // Maximum antenna range, in meters
} CellParameters;

typedef PACKED struct {
    double   antennaLatitude;     // Antenna latitude, in degrees
    double   antennaLongitude;    // Antenna longitude, in degrees
    uint16   errorRadius;         // Wifi AP error radius, in meters
} WiFiParameters;

typedef PACKED struct {
    uint16 systemId;       // System ID
    uint16 networkId;      // Network ID
    uint32 baseStationId;  // Extended base station ID
    CellParameters params; // Common parameters
} CdmaInfoRecord;

typedef PACKED struct {
    uint16 mobileCountryCode; // Mobile Country Code
    uint16 mobileNetworkCode; // Mobile Network Code
    uint16 locationAreaCode;  // Location Area Code
    uint16 cellId;            // Cell ID
    CellParameters params;    // Common parameters
} GsmInfoRecord;

typedef PACKED struct {
    uint8  mac[MAC_ADDR_LENGTH];  // Wifi AP MAC address
    WiFiParameters params;
} WiFiInfoRecord;

#pragma pack (pop)

static NB_Error PalDBErrorToNBError( PAL_DB_Error err );
static void FillCellIDRecord( CellIdRecord& record, const CellParameters& data );


LocationCacheDb::LocationCacheDb( PAL_Instance* pal ):
m_connection( NULL ), m_pal( pal ), m_EncryptionKey( NULL )
{
    GenerateEncryptionKey();
    PrepareDatabase();
    m_config.initialTileTimeToLive = DEFAULT_TILE_TIME_TO_LIVE;
    m_config.maxDataSize = MAX_DATABASE_DATA_SIZE;
}

LocationCacheDb::LocationCacheDb(PAL_Instance* pal, LocationCacheDbConfiguration config )
: m_connection( NULL ), m_pal( pal ), m_config( config ), m_EncryptionKey( NULL )
{
    GenerateEncryptionKey();
    PrepareDatabase();
}

LocationCacheDb::~LocationCacheDb(void)
{
    if ( m_connection )
    {
        PAL_DBClose( m_connection );
    }

    if ( m_EncryptionKey )
    {
        nsl_free( m_EncryptionKey );
        m_EncryptionKey = NULL;
    }
}

NB_Error LocationCacheDb::AddTile( uint32 tileX, uint32 tileY, const uint8* dbFragment, uint32 dbFragmentLength, uint32& tileID )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    uint32 fileSize = 0;
    const char* g_lpstrSQL_InsertTileTable = "INSERT INTO 'TileTable' ( tileX, tileY, timeToLive, lastUpdateTime ) VALUES( @tileX, @tileY, @timeToLive + JULIANDAY('NOW'), JULIANDAY( 'NOW' ) );";
    PAL_DBCommand* pCommand_InsertTileTable = NULL;

    const char* g_lpstrSQL_SelectTileID = "SELECT tileID FROM 'TileTable' WHERE tileX = @tileX AND tileY = @tileY ;";
    PAL_DBCommand* pCommand_SelectTileID = NULL;

    const char* g_lpstrSQL_RemoveTileRecordsTables[] = { "DELETE FROM 'WiFi_infoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE tileX = @tileX AND tileY = @tileY );",
                                                         "DELETE FROM 'GSM_cellInfoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE tileX = @tileX AND tileY = @tileY );",
                                                         "DELETE FROM 'CDMA_cellInfoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE tileX = @tileX AND tileY = @tileY );",
                                                         "DELETE FROM 'TileTable' WHERE tileX = @tileX AND tileY = @tileY;"
                                                       };
    const int deleteQueryCount = 4;
    PAL_DBCommand* pCommand = NULL;


    for ( int i = 0; i < deleteQueryCount && err == PAL_DB_OK; i++ )
    {
        err = PAL_DBInitCommand( m_connection, g_lpstrSQL_RemoveTileRecordsTables[i], &pCommand );
        err = err ? err : PAL_DBBindIntParam( pCommand, "@tileX", tileX );
        err = err ? err : PAL_DBBindIntParam( pCommand, "@tileY", tileY );
        err = err ? err : PAL_DBExecuteNonQueryCommand( pCommand );
        err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
        tmp_error = PAL_DBCommandFinalize( pCommand, NULL );
        pCommand = NULL;
        err = err ? err : tmp_error;
    }

    ret = ret ? ret : PalDBErrorToNBError( err );

    if ( !dbFragment || dbFragmentLength == 0 )
    {
        // this tile is empty
        return NE_BADDATA;
    }

    if ( PAL_Ok != PAL_FileGetSize( m_pal, DATABASE_NAME, &fileSize ) )
    {
        return NE_FAIL;
    }
    // actually even if we clear all database file size will not change
    if ( m_config.maxDataSize != -1 && 
         fileSize > static_cast< uint32 >( m_config.maxDataSize ) )
    {
        // Delete most old tiles and all information about them
        const char* g_lpstrSQL_RemoveOldRecordsWiFiTable = "DELETE FROM 'WiFi_infoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE lastUpdateTime = ( SELECT MIN( lastUpdateTime ) FROM 'TileTable' ) );";
        const char* g_lpstrSQL_RemoveOldRecordsGSMTable = "DELETE FROM 'GSM_cellInfoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE lastUpdateTime = ( SELECT MIN( lastUpdateTime ) FROM 'TileTable' ) );";
        const char* g_lpstrSQL_RemoveOldRecordsCDMATable = "DELETE FROM 'CDMA_cellInfoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE lastUpdateTime = ( SELECT MIN( lastUpdateTime ) FROM 'TileTable' ) );";
        const char* g_lpstrSQL_RemoveOldRecordsTileTable = "DELETE FROM 'TileTable' WHERE lastUpdateTime = ( SELECT MIN( lastUpdateTime ) FROM 'TileTable' );";

        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveOldRecordsWiFiTable );
        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveOldRecordsGSMTable );
        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveOldRecordsCDMATable );
        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveOldRecordsTileTable );
    }

    // fill in TileTable
    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_InsertTileTable, &pCommand_InsertTileTable );
    err = err ? err : PAL_DBBindIntParam( pCommand_InsertTileTable, "@tileX", tileX );
    err = err ? err : PAL_DBBindIntParam( pCommand_InsertTileTable, "@tileY", tileY );
    err = err ? err : PAL_DBBindLongParam( pCommand_InsertTileTable, "@timeToLive", m_config.initialTileTimeToLive );
    err = err ? err : PAL_DBExecuteNonQueryCommand( pCommand_InsertTileTable );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_InsertTileTable, NULL );
    err = err ? err : tmp_error;

    // get tileID
    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_SelectTileID, &pCommand_SelectTileID );
    err = err ? err : PAL_DBBindIntParam( pCommand_SelectTileID, "@tileX", tileX );
    err = err ? err : PAL_DBBindIntParam( pCommand_SelectTileID, "@tileY", tileY );
    err = err ? err : PAL_DBExecuteInt( pCommand_SelectTileID, reinterpret_cast< int* >(&tileID) );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_SelectTileID, NULL );

    err = err ? err : tmp_error;

    ret = ret ? ret : PalDBErrorToNBError( err );

    // parse dbFragment and write records in tables
    ret = ret ? ret : ParseAndWriteDBFragment( dbFragment, dbFragmentLength, tileID );

    return ret;
}



NB_Error LocationCacheDb::SetCoordinates( uint32 tileID, const PAL_RadioCDMAInfo& cdmaInfo, const NB_LS_Location& position )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    const char* g_lpstrSQL_SetCDMAPosition = "UPDATE 'CDMA_cellInfoTable' SET  positionInfo = @positionInfo, lastUpdateTime = JULIANDAY('NOW') WHERE tile = @tileID AND systemID = @systemID AND networkID = @networkID AND cellID = @cellID;";
    PAL_DBCommand* pCommand_SetCDMAPosition = NULL;

    char* encryptedData = NULL;
    char* dataToEncrypt = NULL;
    int size = 0;

    if ( !( ( position.valid & NBPGV_Latitude ) &&
         ( position.valid & NBPGV_Longitude ) &&
         ( position.valid & NBPGV_AxisUncertainty ) &&
         ( position.valid & NBPGV_PerpendicularUncertainty ) ) )
    {
        return NE_BADDATA;
    }

    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_SetCDMAPosition, &pCommand_SetCDMAPosition );
    err = err ? err : PAL_DBBindIntParam( pCommand_SetCDMAPosition, "@tileID", tileID );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetCDMAPosition, "@systemID", cdmaInfo.systemId );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetCDMAPosition, "@networkID", cdmaInfo.networkId );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetCDMAPosition, "@cellID", cdmaInfo.cellId );
 
    dataToEncrypt = reinterpret_cast< char* >( const_cast< NB_LS_Location* > ( &position ) );
    size = ENCRYPTED_DATA_SIZE( NB_LS_Location );

    encryptedData =  static_cast< char* >( nsl_malloc( size ) );
    if ( !encryptedData )
    {
        ret = NE_NOMEM;
    }

    ret = ret ? ret : PalDBErrorToNBError( err );
    // we need to pass in size of dataToEncrypt
    size = sizeof( NB_LS_Location );
    ret = ret ? ret : CSL_AesEncrypt( dataToEncrypt, encryptedData, &size, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
    if ( ret == NE_OK )
    {
        err = err ? err : PAL_DBBindBlobParam( pCommand_SetCDMAPosition, "@positionInfo", static_cast< void* >( encryptedData ), size );
    }

    if ( encryptedData )
    {
        nsl_free( encryptedData );
        encryptedData = NULL;
    }

    err = err ? err : PAL_DBExecuteInt( pCommand_SetCDMAPosition, reinterpret_cast< int* >( &tileID ) );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_SetCDMAPosition, NULL );
    err = err ? err : tmp_error;
    return ret ? ret : PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::SetCoordinates( uint32 tileID, const PAL_RadioGSMInfo& gsmInfo, const NB_LS_Location& position )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    const char* g_lpstrSQL_SetGSMPosition = "UPDATE 'GSM_cellInfoTable' SET positionInfo = @positionInfo, lastUpdateTime = JULIANDAY('NOW') WHERE tile = @tileID AND countryCode = @countryCode AND networkCode = @networkCode AND localAreaCode = @localAreaCode AND cellID = @cellID;";
    PAL_DBCommand* pCommand_SetGSMPosition = NULL;

    char* encryptedData = NULL;
    char* dataToEncrypt = NULL;
    int size = 0;

    if ( !( ( position.valid & NBPGV_Latitude ) &&
         ( position.valid & NBPGV_Longitude ) &&
         ( position.valid & NBPGV_AxisUncertainty ) &&
         ( position.valid & NBPGV_PerpendicularUncertainty ) ) )
    {
        return NE_BADDATA;
    }

    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_SetGSMPosition, &pCommand_SetGSMPosition );
    err = err ? err : PAL_DBBindIntParam( pCommand_SetGSMPosition, "@tileID", tileID );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetGSMPosition, "@countryCode", gsmInfo.mobileCountryCode );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetGSMPosition, "@networkCode", gsmInfo.mobileNetworkCode );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetGSMPosition, "@localAreaCode", gsmInfo.locationAreaCode );
    err = err ? err : PAL_DBBindLongParam( pCommand_SetGSMPosition, "@cellID", gsmInfo.cellId );

    dataToEncrypt = reinterpret_cast< char* >( const_cast< NB_LS_Location* > ( &position ) );
    size = ENCRYPTED_DATA_SIZE( NB_LS_Location );

    encryptedData =  static_cast< char* >( nsl_malloc( size ) );
    if ( !encryptedData )
    {
        ret = NE_NOMEM;
    }

    ret = ret ? ret : PalDBErrorToNBError( err );
    // we need to pass in size of dataToEncrypt
    size = sizeof( NB_LS_Location );
    ret = ret ? ret : CSL_AesEncrypt( dataToEncrypt, encryptedData, &size, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
    if ( ret == NE_OK )
    {
        err = err ? err : PAL_DBBindBlobParam( pCommand_SetGSMPosition, "@positionInfo", static_cast< void* >( encryptedData ), size );
    }

    if ( encryptedData )
    {
        nsl_free( encryptedData );
        encryptedData = NULL;
    }

    err = err ? err : PAL_DBExecuteInt( pCommand_SetGSMPosition, reinterpret_cast< int* >( &tileID ) );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_SetGSMPosition, NULL );
    err = err ? err : tmp_error;
    return ret ? ret : PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::UpdateTimestamp( uint32 tileID )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;

    const char* g_lpstrSQL_UpdateTileTable = "UPDATE 'TileTable' SET  lastUpdateTime = JULIANDAY('NOW') WHERE tileID = @tileID;";

    PAL_DBCommand* pCommand_UpdateTable = NULL;

    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_UpdateTileTable, &pCommand_UpdateTable );
    err = err ? err : PAL_DBBindIntParam( pCommand_UpdateTable, "@tileID", tileID );
    err = err ? err : PAL_DBExecuteNonQueryCommand( pCommand_UpdateTable );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_UpdateTable, NULL );
    err = err ? err : tmp_error;

    return PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::ClearDatabase()
{
    PAL_DB_Error err = PAL_DB_OK;

    const char* g_lpstrSQL_ClearTileTable = "DELETE FROM 'TileTable';";
    const char* g_lpstrSQL_ClearGSMTable  = "DELETE FROM 'GSM_cellInfoTable';";
    const char* g_lpstrSQL_ClearCDMATable = "DELETE FROM 'CDMA_cellInfoTable';";
    const char* g_lpstrSQL_ClearWiFiTable = "DELETE FROM 'WiFi_infoTable';";
    const char* g_lpstrSQL_ClearVersionTable = "DELETE FROM 'versionInfoTable' WHERE NOT ( lastUpdateTime = ( SELECT MAX( lastUpdateTime ) FROM 'versionInfoTable' ) );";

    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearTileTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearGSMTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearCDMATable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearWiFiTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearVersionTable );

    return PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::GetVersion( TableVersionInfo& tableInfo )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;

    const char* g_lpstrSQL_SelectCurrentVersion = "SELECT version FROM 'versionInfoTable' WHERE lastUpdateTime = ( SELECT MAX( lastUpdateTime ) FROM 'versionInfoTable' );";
    PAL_DBCommand* pCommand_SelectCurrentVersion = NULL;

    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_SelectCurrentVersion, &pCommand_SelectCurrentVersion );
    err = err ? err : PAL_DBExecuteString( pCommand_SelectCurrentVersion, &tableInfo.versionInfo );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_SelectCurrentVersion, NULL );
    err = err ? err : tmp_error;

    return PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::FindMatches( const PAL_RadioCDMAInfo& cdmaInfo, CellIdRecord& cellIdRecord,
                                      uint32& tileID, NB_LS_Location& position )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    const char* g_lpstrSQL_FindCDMARecord = "SELECT tile, locationInfo, positionInfo FROM 'CDMA_cellInfoTable' WHERE systemID = @systemID AND  networkID = @networkID AND cellID = @cellID;";
    PAL_DBCommand* pCommand_FindCDMARecord = NULL;
    PAL_DBReader* pReader = NULL;
    byte* cellBlob = NULL;
    byte* positionBlob = NULL;
    int cellRecordLen = 0;
    int positionLen = 0;
    int size = 0;
    char* data = NULL;
    CellParameters* cellInfo = NULL;

    ret = ret ? ret : RemoveExpiredTiles();

    if ( ret != NE_OK )
    {
        return ret;
    }

    nsl_memset( &position, 0, sizeof( NB_LS_Location ) );

    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_FindCDMARecord, &pCommand_FindCDMARecord );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindCDMARecord, "@systemID", cdmaInfo.systemId );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindCDMARecord, "@networkID", cdmaInfo.networkId );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindCDMARecord, "@cellID", cdmaInfo.cellId );
    err = err ? err : PAL_DBExecuteReader( pCommand_FindCDMARecord, &pReader );
    err = err ? err : PAL_DBReaderRead( pReader );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;

    err = err ? err : PAL_DBReaderGetInt( pReader, "tile", reinterpret_cast< int* >( &tileID ) );
    err = err ? err : PAL_DBReaderGetblob( pReader, "locationInfo", &cellBlob, &cellRecordLen );
    err = err ? err : PAL_DBReaderGetblob( pReader, "positionInfo", &positionBlob, &positionLen );

    tmp_error = PAL_DBCommandFinalize( pCommand_FindCDMARecord, pReader );
    err = err ? err : tmp_error;

    ret = ret ? ret : PalDBErrorToNBError( err );

    // Decrypt data
    if ( cellRecordLen > 0 && ret == NE_OK )
    {
        // Record found
        // Decrypt cell data
        size = ENCRYPTED_DATA_SIZE( CellParameters );

        data = static_cast< char* >( nsl_malloc( size ) );
        if ( !data )
        {
            ret = ret ? ret : NE_NOMEM;
        }

        ret = ret ? ret : CSL_AesDecrypt( reinterpret_cast< char const* >( cellBlob ), data, 
                                          cellRecordLen, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            cellInfo = reinterpret_cast< CellParameters* >( data );
            FillCellIDRecord( cellIdRecord, *cellInfo );

            cellIdRecord.cellType = CT_CDMA;

            cellIdRecord.systemId = static_cast< uint16 >( cdmaInfo.systemId );
            cellIdRecord.networkId = static_cast< uint16 >( cdmaInfo.networkId );
            cellIdRecord.baseStationId = cdmaInfo.cellId;
        }

        if ( data )
        {
            nsl_free( data );
            data = NULL;
            cellInfo = NULL;
        }

        // Decrypt position data
        size = ENCRYPTED_DATA_SIZE( NB_LS_Location );

        data = static_cast< char* >( nsl_malloc( size ) );
        if ( !data )
        {
            ret = ret ? ret : NE_NOMEM;
        }

        ret = ret ? ret : CSL_AesDecrypt( reinterpret_cast< char const* >( positionBlob ), data, 
                                          positionLen, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            position = *(reinterpret_cast< NB_LS_Location* >( data ));

        }

        if ( data )
        {
            nsl_free( data );
            data = NULL;
            cellInfo = NULL;
        }
    }
    else
    {
        ret = ret ? ret : NE_NOENT;
    }

    if ( cellBlob )
    {
        nsl_free( cellBlob );
    }

    if ( positionBlob )
    {
        nsl_free( positionBlob );
    }

    return ret;
}

NB_Error LocationCacheDb::FindMatches( const PAL_RadioGSMInfo& gsmInfo, CellIdRecord& cellIdRecord,
                                      uint32& tileID, NB_LS_Location& position )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    const char* g_lpstrSQL_FindGSMRecord = "SELECT tile, locationInfo, positionInfo FROM 'GSM_cellInfoTable' WHERE countryCode = @countryCode AND networkCode = @networkCode AND localAreaCode = @localAreaCode AND cellID = @cellID;";
    PAL_DBCommand* pCommand_FindGSMRecord = NULL;
    PAL_DBReader* pReader = NULL;
    byte* cellBlob = NULL;
    byte* positionBlob = NULL;
    int cellRecordLen = 0;
    int positionLen = 0;
    int size = 0;
    char* data = NULL;
    CellParameters* cellInfo = NULL;

    ret = ret ? ret : RemoveExpiredTiles();

    if ( ret != NE_OK )
    {
        return ret;
    }

    nsl_memset( &position, 0, sizeof( NB_LS_Location ) );
    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_FindGSMRecord, &pCommand_FindGSMRecord );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindGSMRecord, "@countryCode", gsmInfo.mobileCountryCode );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindGSMRecord, "@networkCode", gsmInfo.mobileNetworkCode );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindGSMRecord, "@localAreaCode", gsmInfo.locationAreaCode );
    err = err ? err : PAL_DBBindLongParam( pCommand_FindGSMRecord, "@cellID", gsmInfo.cellId );
    err = err ? err : PAL_DBExecuteReader( pCommand_FindGSMRecord, &pReader );
    err = err ? err : PAL_DBReaderRead( pReader );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;

    err = err ? err : PAL_DBReaderGetInt( pReader, "tile", reinterpret_cast< int* >( &tileID ) );
    err = err ? err : PAL_DBReaderGetblob( pReader, "locationInfo", &cellBlob, &cellRecordLen );
    err = err ? err : PAL_DBReaderGetblob( pReader, "positionInfo", &positionBlob, &positionLen );
    tmp_error = PAL_DBCommandFinalize( pCommand_FindGSMRecord, pReader );
    err = err ? err : tmp_error;

    ret = ret ? ret : PalDBErrorToNBError( err );

    // Decrypt data
    if ( cellRecordLen > 0 && ret == NE_OK )
    {
        // Record found
        // Decrypt cell data
        size = ENCRYPTED_DATA_SIZE( CellParameters );

        data =  static_cast< char* >( nsl_malloc( size ) );
        if ( !data )
        {
            ret = ret ? ret : NE_NOMEM;
        }

        ret = ret ? ret : CSL_AesDecrypt( reinterpret_cast< char const* >( cellBlob ), data, 
                                          cellRecordLen, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            cellInfo = reinterpret_cast< CellParameters* >( data );
            // fill in CellIdRecord
            FillCellIDRecord( cellIdRecord, *cellInfo );

            cellIdRecord.cellType = CT_GSM;

            cellIdRecord.mobileCountryCode = (uint16)gsmInfo.mobileCountryCode;
            cellIdRecord.mobileNetworkCode = (uint16)gsmInfo.mobileNetworkCode;
            cellIdRecord.locationAreaCode = (uint16)gsmInfo.locationAreaCode;
            cellIdRecord.cellId = (uint16)gsmInfo.cellId;
        }

        if ( data )
        {
            nsl_free( data );
            data = NULL;
            cellInfo = NULL;
        }

        // Decrypt position data
        size = ENCRYPTED_DATA_SIZE( NB_LS_Location );

        data = static_cast< char* >( nsl_malloc( size ) );
        if ( !data )
        {
            ret = ret ? ret : NE_NOMEM;
        }

        ret = ret ? ret : CSL_AesDecrypt( reinterpret_cast< char const* >( positionBlob ), data, 
                                          positionLen, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            position = *(reinterpret_cast< NB_LS_Location* >( data ));

        }

        if ( data )
        {
            nsl_free( data );
            data = NULL;
            cellInfo = NULL;
        }
    }
    else
    {
        ret = ret ? ret : NE_NOENT;
    }

    if ( cellBlob )
    {
        nsl_free( cellBlob );
    }

    if ( positionBlob )
    {
        nsl_free( positionBlob );
    }

    return ret;
}
NB_Error LocationCacheDb::FindMatches( const PAL_RadioWifiInfo& wifiInfo, WiFiRecord& wifiRecord,
                                      uint32& tileID )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    const char* g_lpstrSQL_FindWiFiRecord = "SELECT tile, locationInfo FROM 'WiFi_infoTable' WHERE macAddress = @macAddress;";
    PAL_DBCommand* pCommand_FindWiFiRecord = NULL;
    PAL_DBReader* pReader = NULL;
    byte* resultBlob = NULL;
    int len = 0;
    int size = 0;
    char* data = NULL;
    WiFiParameters* wifiParams = NULL;

    ret = ret ? ret : RemoveExpiredTiles();

    if ( ret != NE_OK )
    {
        return ret;
    }

    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_FindWiFiRecord, &pCommand_FindWiFiRecord );
    err = err ? err : PAL_DBBindStringParam( pCommand_FindWiFiRecord, "@macAddress", reinterpret_cast< const char*>( wifiInfo.macAddress ) );
    err = err ? err : PAL_DBExecuteReader( pCommand_FindWiFiRecord, &pReader );
    err = err ? err : PAL_DBReaderRead( pReader );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;

    err = err ? err : PAL_DBReaderGetInt( pReader, "tile", reinterpret_cast< int* >( &tileID ) );
    err = err ? err : PAL_DBReaderGetblob( pReader, "locationInfo", &resultBlob, &len );
    tmp_error = PAL_DBCommandFinalize( pCommand_FindWiFiRecord, pReader );
    err = err ? err : tmp_error;

    ret = ret ? ret : PalDBErrorToNBError( err );

    // Decrypt data
    if ( len > 0 && ret == NE_OK )
    {
        size = ENCRYPTED_DATA_SIZE( WiFiParameters );

        data =  static_cast< char* >( nsl_malloc( size ) );
        if ( !data )
        {
            ret = ret ? ret : NE_NOMEM;
        }

        ret = ret ? ret : CSL_AesDecrypt(  reinterpret_cast< char const* >( resultBlob ), data, 
                                            len, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            wifiParams = reinterpret_cast< WiFiParameters* >( data );
            // fill in wifiRecord
            nsl_memset(& wifiRecord, 0, sizeof( wifiRecord ) );
            wifiRecord.antennaLatitude = wifiParams->antennaLatitude;
            wifiRecord.antennaLongitude = wifiParams->antennaLongitude;
            wifiRecord.errorRadius = wifiParams->errorRadius;
            nsl_memcpy( wifiRecord.mac, wifiInfo.macAddress, MAC_ADDR_LENGTH );
        }

        if ( data )
        {
            nsl_free( data );
            data = NULL;
            wifiParams = NULL;
        }
    }
    else
    {
        ret = ret ? ret : NE_NOENT;
    }

    if ( resultBlob )
    {
        nsl_free( resultBlob );
    }


    return ret;
}

NB_Error LocationCacheDb::PrepareDatabase()
{
    PAL_DB_Error err = PAL_DB_OK;
    NB_Error ret = NE_OK;

    // locationInfo contains all info related with record (see SDS). This field is encrypted.
    const char* g_lpstrSQL_CreateTileTable = "CREATE TABLE 'TileTable' ( 'tileID' INTEGER PRIMARY KEY, 'tileX' INTEGER, 'tileY' INTEGER, 'timeToLive' REAL, 'lastUpdateTime' REAL );";
    const char* g_lpstrSQL_CreateGSMTable  = "CREATE TABLE IF NOT EXISTS 'GSM_cellInfoTable' ( 'tile' INTEGER, 'countryCode' INTEGER, 'networkCode' INTEGER, 'localAreaCode' INTEGER, 'cellID' INTEGER, 'positionInfo' BLOB, 'locationInfo' BLOB, 'lastUpdateTime' REAL );";
    const char* g_lpstrSQL_CreateCDMATable = "CREATE TABLE IF NOT EXISTS 'CDMA_cellInfoTable' ( 'tile' INTEGER, 'systemID' INTEGER, 'networkID' INTEGER, 'cellID' INTEGER, 'positionInfo' BLOB, 'locationInfo' BLOB, 'lastUpdateTime' REAL );";
    const char* g_lpstrSQL_CreateWiFiTable = "CREATE TABLE IF NOT EXISTS 'WiFi_infoTable' ( 'tile' INTEGER, 'macAddress' BLOB, 'positionInfo' BLOB, 'locationInfo' BLOB, 'lastUpdateTime' REAL );";
    TableVersionInfo tableInfo = {0};

    err = err ? err : PAL_DBOpen( m_pal, PAL_DB_Sqlite, DATABASE_NAME, &m_connection );
    if ( m_connection && err == PAL_DB_OK )
    {

        ( void )GetVersion( tableInfo );
        if ( !tableInfo.versionInfo ||
             0 != nsl_strcmp( tableInfo.versionInfo, DATABASE_VERSION ) )
        {
            ret = UpgradeDatabase();

            if ( tableInfo.versionInfo )
            {
                nsl_free( tableInfo.versionInfo );
                tableInfo.versionInfo = NULL;
            }

            return ret;
        }

        if ( tableInfo.versionInfo )
        {
            nsl_free( tableInfo.versionInfo );
            tableInfo.versionInfo = NULL;
        }

        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_CreateTileTable );
        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_CreateGSMTable );
        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_CreateCDMATable );
        err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_CreateWiFiTable );
    }

    return PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::RemoveExpiredTiles()
{
    PAL_DB_Error err = PAL_DB_OK;

    const char* g_lpstrSQL_RemoveExpiredRecordsWiFiTable = "DELETE FROM 'WiFi_infoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE timeToLive < JULIANDAY('NOW') );";
    const char* g_lpstrSQL_RemoveExpiredRecordsGSMTable = "DELETE FROM 'GSM_cellInfoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE timeToLive < JULIANDAY('NOW') );";
    const char* g_lpstrSQL_RemoveExpiredRecordsCDMATable = "DELETE FROM 'CDMA_cellInfoTable' WHERE tile IN ( SELECT tileID FROM 'TileTable' WHERE timeToLive < JULIANDAY('NOW') );";
    const char* g_lpstrSQL_RemoveExpiredRecordsTileable = "DELETE FROM 'TileTable' WHERE timeToLive < JULIANDAY('NOW');";

    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveExpiredRecordsWiFiTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveExpiredRecordsGSMTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveExpiredRecordsCDMATable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_RemoveExpiredRecordsTileable );

    return PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::UpgradeDatabase()
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_FAIL;

    const char* g_lpstrSQL_ClearTileTable = "DROP TABLE IF EXISTS 'TileTable';";
    const char* g_lpstrSQL_ClearGSMTable  = "DROP TABLE IF EXISTS 'GSM_cellInfoTable';";
    const char* g_lpstrSQL_ClearCDMATable = "DROP TABLE IF EXISTS 'CDMA_cellInfoTable';";
    const char* g_lpstrSQL_ClearWiFiTable = "DROP TABLE IF EXISTS 'WiFi_infoTable';";
    const char* g_lpstrSQL_ClearVersionTable = "DROP TABLE IF EXISTS 'versionInfoTable';";

    const char* g_lpstrSQL_CreateVersionTable = "CREATE TABLE IF NOT EXISTS 'versionInfoTable' ( 'version' TEXT, 'lastUpdateTime' REAL );";

    const char* g_lpstrSQL_InsertVersionTable = "INSERT INTO 'versionInfoTable' VALUES( @version, JULIANDAY( 'NOW' ) );";
    PAL_DBCommand* pCommand_InsertVersionTable = NULL;

    // TODO: perform any actions to save data

    // Remove all records
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearTileTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearGSMTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearCDMATable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearWiFiTable );
    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_ClearVersionTable );


    err = err ? err : PAL_DBExecuteNonQuery( m_connection, g_lpstrSQL_CreateVersionTable );

    // Add new version record
    err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_InsertVersionTable, &pCommand_InsertVersionTable );
    err = err ? err : PAL_DBBindStringParam( pCommand_InsertVersionTable, "@version", DATABASE_VERSION );
    err = err ? err : PAL_DBExecuteNonQueryCommand( pCommand_InsertVersionTable );
    err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
    tmp_error = PAL_DBCommandFinalize( pCommand_InsertVersionTable, NULL );
    err = err ? err : tmp_error;

    // create NEW tables
    if (err == PAL_DB_OK)
    {
        ret = PrepareDatabase();
    }

    return ret ? ret : PalDBErrorToNBError( err );
}

NB_Error LocationCacheDb::ParseAndWriteDBFragment( const uint8* dbFragment,
                                                   uint32 dbFragmentLength,
                                                   uint32 tileID )
{
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DB_Error tmp_error = PAL_DB_OK;
    NB_Error ret = NE_OK;

    TileDBFragmentHeader* header = NULL;
    int i = 0, size  = 0;
    uint16 offset = 0, recordNum = 0;
    char* encryptedData = NULL;
    char* dataToEncrypt = NULL;
    CellParameters cellParams = {0};
    WiFiParameters wifiParams = {0};

    const char* g_lpstrSQL_InsertGSMTable = "INSERT INTO 'GSM_cellInfoTable' (tile, countryCode, networkCode, localAreaCode, cellID, locationInfo, lastUpdateTime ) VALUES( @tileID, @countryCode, @networkCode, @localAreaCode, @cellID, @locationInfo, JULIANDAY( 'NOW' ) );";
    const char* g_lpstrSQL_InsertCDMATable = "INSERT INTO 'CDMA_cellInfoTable' ( tile, systemID, networkID, cellID, locationInfo, lastUpdateTime ) VALUES( @tileID, @systemID, @networkID, @cellID, @locationInfo, JULIANDAY( 'NOW' ) );";
    const char* g_lpstrSQL_InsertWiFiTable = "INSERT INTO 'WiFi_infoTable' ( tile, macAddress, locationInfo, lastUpdateTime ) VALUES( @tileID, @macAddress, @locationInfo, JULIANDAY( 'NOW' ) );";

    const char* g_lpstrSQL_InsertTable = NULL;
    PAL_DBCommand* pCommand_InsertTable = NULL;

    if ( !dbFragment )
    {
        // nothing to parse
        return NE_BADDATA;
    }
    if ( dbFragmentLength < sizeof( TileDBFragmentHeader ) )
    {
         // internal data has unknown format
        return NE_BADDATA;
    }

    // for arrays transfered through network byte order is network byte order
    // so we need to change it according needs of current platform
    // for this purpose use BE_UINT16_TO_CPU and BE_UINT32_TO_CPU

    // first 11 bytes contain general information - it's header
    header = reinterpret_cast< TileDBFragmentHeader* >( const_cast< uint8* > ( dbFragment ) );

    // Cell ID records
    offset = BE_UINT16_TO_CPU(header->cellidOffset);
    recordNum = BE_UINT16_TO_CPU(header->numCellid);
    for ( i = 0; i < recordNum && ret == NE_OK; ++i )
    {
        switch( header->cellIDType )
        {
        case CT_CDMA:
            {
                PACKED CdmaInfoRecord* data = NULL;
                uint16 systemId = 0, networkId = 0;
                uint32 baseStationId = 0;

                // move pointer to cell data
                // offset we got, starts from 1 not 0!
                data = reinterpret_cast< CdmaInfoRecord* >( const_cast< uint8* >( dbFragment +
                    sizeof(TileDBFragmentHeader) + offset - 1 ) );
                data += i;

                systemId = BE_UINT16_TO_CPU(data->systemId);
                networkId = BE_UINT16_TO_CPU(data->networkId);
                baseStationId = BE_UINT32_TO_CPU(data->baseStationId);

                BE_UINT64_TO_CPU(data->params.antennaLatitude, cellParams.antennaLatitude);
                BE_UINT64_TO_CPU(data->params.antennaLongitude, cellParams.antennaLongitude);
                cellParams.antennaHEPE = BE_UINT16_TO_CPU(data->params.antennaHEPE);
                cellParams.antennaOrientation = BE_UINT32_TO_CPU(data->params.antennaOrientation);
                cellParams.antennaOpening = BE_UINT32_TO_CPU(data->params.antennaOpening);
                cellParams.antennaMAR = BE_UINT32_TO_CPU(data->params.antennaMAR);

                g_lpstrSQL_InsertTable = g_lpstrSQL_InsertCDMATable;
                err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_InsertTable, &pCommand_InsertTable );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@systemID", systemId );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@networkID", networkId  );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@cellID", baseStationId );
                break;
            }
        case CT_GSM:
            {
                GsmInfoRecord* data = NULL;
                uint16 mobileCountryCode = 0, mobileNetworkCode = 0;
                uint16 locationAreaCode = 0, cellId = 0;

                // move pointer to cell data
                // offset we got, starts from 1 not 0!
                data = reinterpret_cast< GsmInfoRecord* >( const_cast< uint8* >( dbFragment +
                    sizeof(TileDBFragmentHeader) + offset - 1 ) );
                data += i;

                locationAreaCode = BE_UINT16_TO_CPU(data->locationAreaCode);
                mobileCountryCode = BE_UINT16_TO_CPU(data->mobileCountryCode);
                mobileNetworkCode = BE_UINT16_TO_CPU(data->mobileNetworkCode);
                cellId = BE_UINT16_TO_CPU(data->cellId);

                BE_UINT64_TO_CPU(data->params.antennaLatitude, cellParams.antennaLatitude);
                BE_UINT64_TO_CPU(data->params.antennaLongitude, cellParams.antennaLongitude);
                cellParams.antennaHEPE = BE_UINT16_TO_CPU(data->params.antennaHEPE);
                cellParams.antennaOrientation = BE_UINT32_TO_CPU(data->params.antennaOrientation);
                cellParams.antennaOpening = BE_UINT32_TO_CPU(data->params.antennaOpening);
                cellParams.antennaMAR = BE_UINT32_TO_CPU(data->params.antennaMAR);

                g_lpstrSQL_InsertTable = g_lpstrSQL_InsertGSMTable;
                err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_InsertTable, &pCommand_InsertTable );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@countryCode", mobileCountryCode );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@networkCode", mobileNetworkCode );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@localAreaCode", locationAreaCode );
                err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@cellID", cellId );
                break;
            }
        default:
            return NE_INVAL;
        }
        err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@tileID", tileID );

        dataToEncrypt = reinterpret_cast< char* >( &cellParams );
        size = ENCRYPTED_DATA_SIZE( CellParameters );

        encryptedData =  static_cast< char* >( nsl_malloc( size ) );
        if ( !encryptedData )
        {
            ret = NE_NOMEM;
            break;
        }

        ret = ret ? ret : PalDBErrorToNBError( err );
        // we need to pass in size of dataToEncrypt
        size = sizeof( CellParameters );
        ret = ret ? ret : CSL_AesEncrypt( dataToEncrypt, encryptedData, &size, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            err = err ? err : PAL_DBBindBlobParam( pCommand_InsertTable, "@locationInfo",  static_cast< void* >( encryptedData ), size );
            err = err ? err : PAL_DBExecuteNonQueryCommand( pCommand_InsertTable );
            err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
            tmp_error = PAL_DBCommandFinalize( pCommand_InsertTable, NULL );
            pCommand_InsertTable = NULL;
            err = err ? err : tmp_error; 
        }

        if ( encryptedData )
        {
            nsl_free( encryptedData );
            encryptedData = NULL;
        }

        ret = ret ? ret : PalDBErrorToNBError( err );
    }

    if ( ret != NE_OK )
    {
        // error occured while cell records writing
        tmp_error = PAL_DBCommandFinalize( pCommand_InsertTable, NULL );
        err = err ? err : tmp_error;
        if ( encryptedData )
        {
            nsl_free( encryptedData );
            encryptedData = NULL;
        }
        return ret;
    }

    // Wifi records
    offset = BE_UINT16_TO_CPU(header->wifiOffset);
    recordNum = BE_UINT16_TO_CPU(header->numWiFi);

    for (i = 0; i < recordNum && ret == NE_OK; ++i)
    {
        WiFiInfoRecord* wifiData = NULL;

        // move pointer to wifi data
        wifiData = reinterpret_cast<  WiFiInfoRecord* >( const_cast< uint8* >( dbFragment +
            sizeof( TileDBFragmentHeader ) + offset - 1 ) );
        wifiData += i;

        err = err ? err : PAL_DBInitCommand( m_connection, g_lpstrSQL_InsertWiFiTable, &pCommand_InsertTable );
        err = err ? err : PAL_DBBindBlobParam( pCommand_InsertTable, "@macAddress", static_cast< void* >( wifiData->mac ), MAC_ADDR_LENGTH );
        err = err ? err : PAL_DBBindIntParam( pCommand_InsertTable, "@tileID", tileID );
        
        BE_UINT64_TO_CPU( wifiData->params.antennaLatitude, wifiParams.antennaLatitude );
        BE_UINT64_TO_CPU( wifiData->params.antennaLongitude, wifiParams.antennaLongitude );
        wifiParams.errorRadius = BE_UINT16_TO_CPU( wifiData->params.errorRadius );
        dataToEncrypt = reinterpret_cast< char* >( &wifiParams );

        size = ENCRYPTED_DATA_SIZE( WiFiParameters );

        encryptedData =  static_cast< char* >( nsl_malloc( size ) );
        if ( !encryptedData )
        {
            ret = NE_NOMEM;
            break;
        }

        ret = ret ? ret : PalDBErrorToNBError( err );
        // we need to pass in size of dataToEncrypt
        size = sizeof( WiFiParameters );
        ret = ret ? ret : CSL_AesEncrypt( dataToEncrypt, encryptedData, &size, m_EncryptionKey, nsl_strlen( m_EncryptionKey ) );
        if ( ret == NE_OK )
        {
            err = err ? err : PAL_DBBindBlobParam( pCommand_InsertTable, "@locationInfo",  static_cast< void* >( encryptedData ), size );
            err = err ? err : PAL_DBExecuteNonQueryCommand( pCommand_InsertTable );
            err = ( err == PAL_DB_DONE || err == PAL_DB_ROW ) ? PAL_DB_OK : err;
            tmp_error = PAL_DBCommandFinalize( pCommand_InsertTable, NULL );
            pCommand_InsertTable = NULL;
            err = err ? err : tmp_error;
        }

        if ( encryptedData )
        {
            nsl_free( encryptedData );
            encryptedData = NULL;
        }

        ret = ret ? ret : PalDBErrorToNBError( err );
    }

    if ( ret != NE_OK )
    {
        // error occured while wifi records writing
        tmp_error = PAL_DBCommandFinalize( pCommand_InsertTable, NULL );
        err = err ? err : tmp_error;
        if ( encryptedData )
        {
            nsl_free( encryptedData );
            encryptedData = NULL;
        }
        return ret;
    }

    return ret;
}


NB_Error PalDBErrorToNBError( PAL_DB_Error err )
{
    switch ( err )
    {
    case PAL_DB_OK:
    case PAL_DB_ROW:
    case PAL_DB_DONE:
      return NE_OK;
    case PAL_DB_ENOMEM:
      return NE_NOMEM;
    default:
      return NE_FAIL;
    };
}

void FillCellIDRecord( CellIdRecord& record, const CellParameters& data )
{
    // fill in CellIdRecord
    nsl_memset( &record, 0, sizeof( CellIdRecord ) );
    record.antennaLatitude =  data.antennaLatitude;
    record.antennaLongitude = data.antennaLongitude;
    record.antennaHEPE = data.antennaHEPE;
    record.antennaOrientation = data.antennaOrientation;
    record.antennaOpening = data.antennaOpening;
    record.antennaMAR = data.antennaMAR;
}

NB_Error LocationCacheDb::GenerateEncryptionKey()
{

    if ( m_EncryptionKey )
    {
        nsl_free( m_EncryptionKey );
        m_EncryptionKey = NULL;
    }

    m_EncryptionKey = nsl_getdeviceuniquestring();
    if ( !m_EncryptionKey )
    {
        return NE_FAIL;
    }

    return NE_OK;
}

/*! @} */
