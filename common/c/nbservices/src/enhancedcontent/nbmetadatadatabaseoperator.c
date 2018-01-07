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

    @file     nbmetadatadatabaseoperator.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "nbmetadatadatabaseoperator.h"
#include "paldberror.h"
#include "paldb.h"
#include "palfile.h"

/*! @{ */

// Constants .....................................................................................
#define MEMORY_ALLOCATED_UNIT  64
/*! Database fields of metadata table */
static const char DB_FIELD_ID[] = "id";
static const char DB_FIELD_CONTENT_TYPE[] = "contentType";
static const char DB_FIELD_PROJECTION_TYPE[] = "projectionType";
static const char DB_FIELD_MANIFEST_STATE[] = "manifestState";
static const char DB_FIELD_UPDATE_STATUS[] = "updateStatus";
static const char DB_FIELD_SELECTED[] = "selected";
static const char DB_FIELD_DELETED[] = "deleted";
static const char DB_FIELD_DATASET_SIZE[] = "datasetSize";
static const char DB_FIELD_DOWNLOADED_SIZE[] = "downloadedSize";
static const char DB_FIELD_FAILED_SIZE[] = "failedSize";
static const char DB_FIELD_DATUM[] = "datum";
static const char DB_FIELD_BOUNDING_LATITUDE1[] = "boundingLatitude1";
static const char DB_FIELD_BOUNDING_LONGITUDE1[] = "boundingLongitude1";
static const char DB_FIELD_BOUNDING_LATITUDE2[] = "boundingLatitude2";
static const char DB_FIELD_BOUNDING_LONGITUDE2[] = "boundingLongitude2";
static const char DB_FIELD_ORIGIN_LATITUDE[] = "originLatitude";
static const char DB_FIELD_ORIGIN_LONGITUDE[] = "originLongitude";
static const char DB_FIELD_SCALE_FACTOR[] = "scaleFactor";
static const char DB_FIELD_FALSE_EASTING[] = "falseEasting";
static const char DB_FIELD_FALSE_NORTHING[] = "falseNorthing";
static const char DB_FIELD_Z_OFFSET[] = "zOffset";
static const char DB_FIELD_DATASET_ID[] = "datasetId";
static const char DB_FIELD_MANIFEST_VERSION[] = "manifestVersion";
static const char DB_FIELD_URL_BASE[] = "urlBase";
static const char DB_FIELD_URL_ARGS_TEMPLATE[] = "urlArgsTemplate";
static const char DB_FIELD_DISPLAY_NAME[] = "displayName";
static const char DB_FIELD_COUNTRY[] = "country";


/*! Added database fields of manifest table */
static const char DB_FIELD_METADATA_ID[] = "metadataId";
static const char DB_FIELD_DOWNLOAD_STATUS[] = "downloadStatus";
static const char DB_FIELD_FILE_SIZE[] = "fileSize";
static const char DB_FIELD_FILE_ID[] = "fileId";
static const char DB_FIELD_FILE_VERSION[] = "fileVersion";


/*! Database parameters */
static const char DB_PARAMETER_ID[] = "@id";
static const char DB_PARAMETER_CONTENT_TYPE[] = "@contentType";
static const char DB_PARAMETER_PROJECTION_TYPE[] = "@projectionType";
static const char DB_PARAMETER_MANIFEST_STATE[] = "@manifestState";
static const char DB_PARAMETER_UPDATE_STATUS[] = "@updateStatus";
static const char DB_PARAMETER_SELECTED[] = "@selected";
static const char DB_PARAMETER_DELETED[] = "@deleted";
static const char DB_PARAMETER_DATASET_SIZE[] = "@datasetSize";
static const char DB_PARAMETER_DOWNLOADED_SIZE[] = "@downloadedSize";
static const char DB_PARAMETER_FAILED_SIZE[] = "@failedSize";
static const char DB_PARAMETER_DATUM[] = "@datum";
static const char DB_PARAMETER_BOUNDING_LATITUDE1[] = "@boundingLatitude1";
static const char DB_PARAMETER_BOUNDING_LONGITUDE1[] = "@boundingLongitude1";
static const char DB_PARAMETER_BOUNDING_LATITUDE2[] = "@boundingLatitude2";
static const char DB_PARAMETER_BOUNDING_LONGITUDE2[] = "@boundingLongitude2";
static const char DB_PARAMETER_ORIGIN_LATITUDE[] = "@originLatitude";
static const char DB_PARAMETER_ORIGIN_LONGITUDE[] = "@originLongitude";
static const char DB_PARAMETER_SCALE_FACTOR[] = "@scaleFactor";
static const char DB_PARAMETER_FALSE_EASTING[] = "@falseEasting";
static const char DB_PARAMETER_FALSE_NORTHING[] = "@falseNorthing";
static const char DB_PARAMETER_Z_OFFSET[] = "@zOffset";
static const char DB_PARAMETER_DATASET_ID[] = "@datasetId";
static const char DB_PARAMETER_MANIFEST_VERSION[] = "@manifestVersion";
static const char DB_PARAMETER_URL_BASE[] = "@urlBase";
static const char DB_PARAMETER_URL_ARGS_TEMPLATE[] = "@urlArgsTemplate";
static const char DB_PARAMETER_DISPLAY_NAME[] = "@displayName";
static const char DB_PARAMETER_COUNTRY[] = "@country";


/*! Added database parameters of manifest table */
static const char DB_PARAMETER_METADATA_ID[] = "@metadataId";
static const char DB_PARAMETER_DOWNLOAD_STATUS[] = "@downloadStatus";
static const char DB_PARAMETER_FILE_SIZE[] = "@fileSize";
static const char DB_PARAMETER_FILE_ID[] = "@fileId";
static const char DB_PARAMETER_FILE_VERSION[] = "@fileVersion";


/*! Added database parameters for multiple conditions */
static const char DB_PARAMETER_CONTENT_TYPE1[] = "@contentType1";
static const char DB_PARAMETER_CONTENT_TYPE2[] = "@contentType2";

static const char DB_PARAMETER_DOWNLOAD_STATUS1[] = "@downloadStatus1";
static const char DB_PARAMETER_DOWNLOAD_STATUS2[] = "@downloadStatus2";

/*!< SQL for metadata table */
static const char METADATA_TABLE_CREATE_SQL[] =
"CREATE TABLE IF NOT EXISTS 'metadata' ( \
    'id' INTEGER PRIMARY KEY NOT NULL, \
    'contentType' INTEGER, \
    'projectionType' INTEGER, \
    'manifestState' INTEGER, \
    'updateStatus' INTEGER, \
    'selected' INTEGER, \
    'deleted' INTEGER, \
    'datasetSize' INTEGER, \
    'downloadedSize' INTEGER, \
    'failedSize' INTEGER, \
    'datum' INTEGER, \
    'boundingLatitude1' DOUBLE, \
    'boundingLongitude1' DOUBLE, \
    'boundingLatitude2' DOUBLE, \
    'boundingLongitude2' DOUBLE, \
    'originLatitude' DOUBLE, \
    'originLongitude' DOUBLE, \
    'scaleFactor' DOUBLE, \
    'falseEasting' DOUBLE, \
    'falseNorthing' DOUBLE, \
    'zOffset' DOUBLE, \
    'datasetId' VARCHAR, \
    'manifestVersion' VARCHAR, \
    'urlBase' VARCHAR, \
    'urlArgsTemplate' VARCHAR, \
    'displayName' VARCHAR, \
    'country' VARCHAR);";

static const char METADATA_UPDATE_UPDATE_STATUS_BY_CONTENT_TYPE_SQL[] =
"UPDATE metadata SET updateStatus = @updateStatus WHERE contentType = @contentType;";

static const char METADATA_UPDATE_DELETED_SQL[] =
"UPDATE metadata SET deleted = @deleted;";

static const char METADATA_UPDATE_DELETED_BY_ID_SQL[] =
"UPDATE metadata SET deleted = @deleted WHERE id = @id;";

static const char METADATA_UPDATE_SELECTED_BY_DATASET_ID_SQL[] =
"UPDATE metadata SET selected = @selected WHERE datasetId = @datasetId;";

static const char METADATA_UPDATE_ALL_BY_ID_SQL[] =
"UPDATE metadata SET \
    contentType = @contentType, \
    projectionType = @projectionType, \
    manifestState = @manifestState, \
    updateStatus = @updateStatus, \
    selected = @selected, \
    deleted = @deleted, \
    datasetSize = @datasetSize, \
    downloadedSize = @downloadedSize, \
    failedSize = @failedSize, \
    datum = @datum, \
    boundingLatitude1 = @boundingLatitude1, \
    boundingLongitude1 = @boundingLongitude1, \
    boundingLatitude2 = @boundingLatitude2, \
    boundingLongitude2 = @boundingLongitude2, \
    originLatitude = @originLatitude, \
    originLongitude = @originLongitude, \
    scaleFactor = @scaleFactor, \
    falseEasting = @falseEasting, \
    falseNorthing = @falseNorthing, \
    zOffset = @zOffset, \
    datasetId = @datasetId, \
    manifestVersion = @manifestVersion, \
    urlBase = @urlBase, \
    urlArgsTemplate = @urlArgsTemplate, \
    displayName = @displayName, \
    country = @country WHERE id = @id;";

static const char METADATA_SELECT_BY_ID_SQL[] =
"SELECT * FROM metadata WHERE id = @id;";

static const char METADATA_SELECT_BY_MANIFEST_STATE_SQL[] =
"SELECT * FROM metadata WHERE manifestState = @manifestState;";

static const char METADATA_SELECT_BY_CONTENT_TYPE_SQL[] =
"SELECT * FROM metadata WHERE contentType = @contentType;";

static const char METADATA_SELECT_BY_DATASET_ID_SQL[] =
"SELECT * FROM metadata WHERE datasetId = @datasetId;";

static const char METADATA_SELECT_BY_CONTENT_TYPE_AND_DATASET_ID_SQL[] =
"SELECT * FROM metadata WHERE contentType = @contentType AND datasetId = @datasetId;";

static const char METADATA_SELECT_BY_CONTENT_TYPE_AND_SELECTED_SQL[] =
"SELECT * FROM metadata WHERE contentType = @contentType AND selected = @selected;";

static const char METADATA_SELECT_URL_BASE_SQL[] =
"SELECT urlBase from metadata limit 0,1;";

static const char METADATA_SELECT_AVAILABLE_CITIES_SQL[] =
"SELECT * FROM metadata WHERE (contentType = @contentType1 OR contentType = @contentType2) AND datasetId != @datasetId ORDER BY datasetId;";

static const char METADATA_SELECT_AVAILABLE_REGIONS_SQL[] =
"SELECT * FROM metadata WHERE contentType = @contentType1 ORDER BY datasetId;";

static const char METADATA_SELECT_BY_SELECTED_ORDER_BY_DATASET_ID_SQL[] =
"SELECT * FROM metadata WHERE contentType = @contentType1 AND selected = @selected ORDER BY datasetId;";

static const char METADATA_SELECT_BY_DELETED_SQL[] =
"SELECT * FROM metadata WHERE deleted = @deleted;";

static const char METADATA_INSERT_SQL[] =
"INSERT INTO metadata ( \
    'contentType', \
    'projectionType', \
    'manifestState', \
    'updateStatus', \
    'selected', \
    'deleted', \
    'datasetSize', \
    'downloadedSize', \
    'failedSize', \
    'datum', \
    'boundingLatitude1', \
    'boundingLongitude1', \
    'boundingLatitude2', \
    'boundingLongitude2', \
    'originLatitude', \
    'originLongitude', \
    'scaleFactor', \
    'falseEasting', \
    'falseNorthing', \
    'zOffset', \
    'datasetId', \
    'manifestVersion', \
    'urlBase', \
    'urlArgsTemplate', \
    'displayName', \
    'country' \
) VALUES ( \
    @contentType, \
    @projectionType, \
    @manifestState, \
    @updateStatus, \
    @selected, \
    @deleted, \
    @datasetSize, \
    @downloadedSize, \
    @failedSize, \
    @datum, \
    @boundingLatitude1, \
    @boundingLongitude1, \
    @boundingLatitude2, \
    @boundingLongitude2, \
    @originLatitude, \
    @originLongitude, \
    @scaleFactor, \
    @falseEasting, \
    @falseNorthing, \
    @zOffset, \
    @datasetId, \
    @manifestVersion, \
    @urlBase, \
    @urlArgsTemplate, \
    @displayName, \
    @country);";

static const char METADATA_DELETE_ALL_SQL[] =
"DELETE FROM metadata;";

static const char METADATA_DELETE_BY_DELETED_SQL[] =
"DELETE FROM metadata WHERE deleted = @deleted;";


/*!< SQL for manifest table */
static const char MANIFEST_TABLE_CREATE_SQL[] =
"CREATE TABLE IF NOT EXISTS 'manifest' ( \
    'id' INTEGER PRIMARY KEY NOT NULL, \
    'metadataId' INTEGER, \
    'downloadStatus' INTEGER, \
    'fileSize' INTEGER, \
    'downloadedSize' INTEGER, \
    'boundingLatitude1' DOUBLE, \
    'boundingLongitude1' DOUBLE, \
    'boundingLatitude2' DOUBLE, \
    'boundingLongitude2' DOUBLE, \
    'fileId' VARCHAR, \
    'fileVersion' VARCHAR);";

static const char MANIFEST_UPDATE_ALL_BY_ID_SQL[] =
"UPDATE manifest SET \
    metadataId = @metadataId, \
    downloadStatus = @downloadStatus, \
    fileSize = @fileSize, \
    downloadedSize = @downloadedSize, \
    boundingLatitude1 = @boundingLatitude1, \
    boundingLongitude1 = @boundingLongitude1, \
    boundingLatitude2 = @boundingLatitude2, \
    boundingLongitude2 = @boundingLongitude2, \
    fileId = @fileId, \
    fileVersion = @fileVersion WHERE id = @id;";

static const char MANIFEST_UPDATE_DOWNLOAD_STATUS_BY_METADATA_ID_SQL[] =
"UPDATE manifest SET downloadStatus = @downloadStatus, downloadedSize = @downloadedSize WHERE metadataId = @metadataId;";

static const char MANIFEST_SELECT_BY_METADATA_ID_SQL[] =
"SELECT * FROM manifest WHERE metadataId = @metadataId;";

static const char MANIFEST_SELECT_BY_METADATA_ID_AND_DOWNLOAD_STATUS_SQL[] =
"SELECT * FROM manifest WHERE metadataId = @metadataId AND downloadStatus = @downloadStatus;";

static const char MANIFEST_SELECT_BY_METADATA_ID_AND_FILE_ID_SQL[] =
"SELECT * FROM manifest WHERE metadataId = @metadataId AND fileId = @fileId;";

static const char MANIFEST_INSERT_SQL[] =
"INSERT INTO manifest ( \
    'metadataId', \
    'downloadStatus', \
    'fileSize', \
    'downloadedSize', \
    'boundingLatitude1', \
    'boundingLongitude1', \
    'boundingLatitude2', \
    'boundingLongitude2', \
    'fileId', \
    'fileVersion' \
) VALUES ( \
    @metadataId, \
    @downloadStatus, \
    @fileSize, \
    @downloadedSize, \
    @boundingLatitude1, \
    @boundingLongitude1, \
    @boundingLatitude2, \
    @boundingLongitude2, \
    @fileId, \
    @fileVersion);";

static const char MANIFEST_DELETE_ALL_SQL[] =
"DELETE FROM manifest;";

static const char MANIFEST_DELETE_BY_METADATA_ID_SQL[] =
"DELETE FROM manifest WHERE metadataId = @metadataId;";

static const char MANIFEST_DELETE_BY_METADATA_ID_AND_FILE_ID_SQL[] =
"DELETE FROM manifest WHERE metadataId = @metadataId AND fileId = @fileId;";

static const char METADATA_MANIFEST_SELECT_BY_DOWNLOADED_AND_NOT_DELETED_SQL[] =
"SELECT fileId FROM manifest left join metadata on metadata.id = manifest.metadataId \
    WHERE metadata.contentType = @contentType1 AND \
    manifest.downloadStatus = @downloadStatus AND \
    metadata.deleted = @deleted;";

static const char METADATA_MANIFEST_SELECT_DOWNLOADSTATUS_BY_DATASETID_SQL[] =
"SELECT downloadStatus FROM manifest left join metadata on metadata.id = manifest.metadataId \
    WHERE metadata.datasetId = @datasetId";

static const char METADATA_MANIFEST_UPDATE_DOWNLOADSTATUS_BY_DATASETID_SQL[] =
"UPDATE manifest SET downloadStatus = @downloadStatus1 WHERE \
        manifest.downloadStatus != @downloadStatus2 AND \
        manifest.metadataId = \
        (SELECT metadata.id FROM metadata WHERE metadata.datasetId = @datasetId)";

// Local types ...................................................................................

/*! The enhanced content data source */
struct NB_MetadataDatabaseOperator
{
    PAL_Instance*       pal;                                                    /*!< PAL instance */
    PAL_DBConnect*      dbConnection;                                           /*!< Database connection */
    char                dbFilename[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1];       /*!< Database filename */
};

// Local Functions ...............................................................................

static NB_Error CheckDatabaseConnection(NB_MetadataDatabaseOperator* instance);
static NB_Error GetMetadataSourceReply(PAL_DBReader* reader, NB_MetadataSourceReply* reply);
static NB_Error GetMetadataManifestReply(PAL_DBReader* reader, NB_MetadataManifestReply* reply);
static NB_Error GetCity(PAL_DBReader* reader, NB_EnhancedContentCityData* city);
static NB_Error GetEntry(PAL_DBReader* reader, NB_ContentEntryData* entry, NB_EnhancedContentDataType dataType);
static NB_Error GetCities(PAL_DBReader* reader, CSL_Vector* vectorCities);
static NB_Error GetEntries(PAL_DBReader* reader, CSL_Vector* vectorRegions, NB_EnhancedContentDataType dataType);

// Public functions ..............................................................................

/* See header file for description */
NB_Error
NB_MetadataDatabaseOperatorCreate(PAL_Instance* pal,
                                  const char* databaseFullFilename,
                                  NB_MetadataDatabaseOperator** instance)
{
    NB_Error result = NE_OK;
    NB_MetadataDatabaseOperator* dbOperator = NULL;

    if (!pal || !databaseFullFilename || !instance)
    {
        return NE_INVAL;
    }

    dbOperator = nsl_malloc(sizeof(*dbOperator));
    if (!dbOperator)
    {
        return NE_NOMEM;
    }

    nsl_memset(dbOperator, 0, sizeof(*dbOperator));

    dbOperator->pal = pal;

    nsl_strlcpy(dbOperator->dbFilename, databaseFullFilename, sizeof(dbOperator->dbFilename));
    result = CheckDatabaseConnection(dbOperator);
    if (result != NE_OK)
    {
        nsl_free(dbOperator);
        return result;
    }

    *instance = dbOperator;

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_MetadataDatabaseOperatorDestroy(NB_MetadataDatabaseOperator* instance)
{
    if (instance)
    {
        if (instance->dbConnection)
        {
            PAL_DBClose(instance->dbConnection);
            instance->dbConnection = NULL;
        }

        nsl_free(instance);
        instance = NULL;
    }

    return NE_OK;
}

/* See header file for description */
NB_Error
NB_MetadataDatabaseOperatorMasterClear(NB_MetadataDatabaseOperator* instance)
{
    NB_Error result = NE_OK;
    PAL_Instance* pal = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    pal = instance->pal;

    // Close database connection
    if (instance->dbConnection)
    {
        PAL_DBClose(instance->dbConnection);
        instance->dbConnection = NULL;
    }

    // Delete database filename
    if (PAL_FileExists(pal, instance->dbFilename) == PAL_Ok)
    {
        result = PAL_FileRemove(pal, instance->dbFilename) == PAL_Ok ? NE_OK : NE_FSYS;
    }

    return result;
}

/* See header file for description */
NB_Error
NB_MetadataDatabaseOperatorSetMetadataUpdateStatusByContentType(NB_MetadataDatabaseOperator* instance,
                                                                int updateStatus,
                                                                int contentType)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_UPDATE_UPDATE_STATUS_BY_CONTENT_TYPE_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_UPDATE_STATUS, updateStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE, contentType);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error
NB_MetadataDatabaseOperatorSetAllMetadataDeleted(NB_MetadataDatabaseOperator* instance,
                                                 nb_boolean deleted)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_UPDATE_DELETED_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, (int) deleted);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error
NB_MetadataDatabaseOperatorSetMetadataDeletedById(NB_MetadataDatabaseOperator* instance,
                                                  nb_boolean deleted,
                                                  int64 id)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_UPDATE_DELETED_BY_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, deleted);
    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_ID, id);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceReplyById(NB_MetadataDatabaseOperator* instance,
                                                               int64 id,
                                                               NB_MetadataSourceReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_ID, id);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetMetadataSourceReply(reader, reply);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByManifestState(NB_MetadataDatabaseOperator* instance,
                                                                            int manifestState,
                                                                            CSL_Vector* vectorReplies)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !vectorReplies)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_MANIFEST_STATE_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_MANIFEST_STATE, manifestState);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    while ( nbErr == NE_OK && err == PAL_DB_OK )
    {
        nsl_memset(&reply, 0, sizeof(reply));

        nbErr = GetMetadataSourceReply(reader, &reply);

        nbErr = nbErr ? nbErr : CSL_VectorAppend(vectorReplies, &reply) ? NE_OK : NE_NOMEM;
    }

    PAL_DBCommandFinalize(command, reader);

    // Ignore NE_NOENT. User should check count of vectorReplies.
    nbErr = nbErr == NE_NOENT ? NE_OK : nbErr;

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentType(NB_MetadataDatabaseOperator* instance,
                                                                          int contentType,
                                                                          CSL_Vector* vectorReplies)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !vectorReplies)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_CONTENT_TYPE_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE, contentType);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    while (nbErr == NE_OK && err == PAL_DB_OK)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        nbErr = GetMetadataSourceReply(reader, &reply);

        nbErr = nbErr ? nbErr : CSL_VectorAppend(vectorReplies, &reply) ? NE_OK : NE_NOMEM;
    }

    PAL_DBCommandFinalize(command, reader);

    // Ignore NE_NOENT. User should check count of vectorReplies.
    nbErr = nbErr == NE_NOENT ? NE_OK : nbErr;

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(NB_MetadataDatabaseOperator* instance,
                                                                                    int contentType,
                                                                                    const char* datasetId,
                                                                                    NB_MetadataSourceReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !datasetId || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_CONTENT_TYPE_AND_DATASET_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE, contentType);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, datasetId);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetMetadataSourceReply(reader, reply);
    }


    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentTypeAndSelected(NB_MetadataDatabaseOperator* instance,
                                                                                     int contentType,
                                                                                     nb_boolean selected,
                                                                                     CSL_Vector* vectorReplies)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !vectorReplies)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_CONTENT_TYPE_AND_SELECTED_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE, contentType);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_SELECTED, (int) selected);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    while (nbErr == NE_OK && err == PAL_DB_OK)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        nbErr = GetMetadataSourceReply(reader, &reply);

        nbErr = nbErr ? nbErr : CSL_VectorAppend(vectorReplies, &reply) ? NE_OK : NE_NOMEM;
    }

    PAL_DBCommandFinalize(command, reader);

    // Ignore NE_NOENT. User should check count of vectorReplies.
    nbErr = nbErr == NE_NOENT ? NE_OK : nbErr;

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByDeleted(NB_MetadataDatabaseOperator* instance,
                                                                      nb_boolean deleted,
                                                                      CSL_Vector* vectorReplies)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataSourceReply reply = {0};

    if (!instance || !vectorReplies)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_DELETED_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, deleted);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    while (nbErr == NE_OK && err == PAL_DB_OK)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        nbErr = GetMetadataSourceReply(reader, &reply);

        nbErr = nbErr ? nbErr : CSL_VectorAppend(vectorReplies, &reply) ? NE_OK : NE_NOMEM;
    }

    PAL_DBCommandFinalize(command, reader);

    // Ignore NE_NOENT. User should check count of vectorReplies.
    nbErr = nbErr == NE_NOENT ? NE_OK : nbErr;

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorUpdateMetadata(NB_MetadataDatabaseOperator* instance,
                                                   const NB_MetadataSourceReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_UPDATE_ALL_BY_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_ID, reply->id);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE, reply->contentType);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_PROJECTION_TYPE, reply->projectionType);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_MANIFEST_STATE, reply->manifestState);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_UPDATE_STATUS, reply->updateStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_SELECTED, reply->selected);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, reply->deleted);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DATASET_SIZE, reply->datasetSize);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOADED_SIZE, reply->downloadedSize);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_FAILED_SIZE, reply->failedSize);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DATUM, reply->datum);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE1, reply->boundingLatitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE1, reply->boundingLongitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE2, reply->boundingLatitude2);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE2, reply->boundingLongitude2);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_ORIGIN_LATITUDE, reply->originLatitude);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_ORIGIN_LONGITUDE, reply->originLongitude);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_SCALE_FACTOR, reply->scaleFactor);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_FALSE_EASTING, reply->falseEasting);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_FALSE_NORTHING, reply->falseNorthing);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_Z_OFFSET, reply->zOffset);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, reply->datasetId);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_MANIFEST_VERSION, reply->manifestVersion);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_URL_BASE, reply->urlBase);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_URL_ARGS_TEMPLATE, reply->urlArgsTemplate);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DISPLAY_NAME, reply->displayName);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_COUNTRY, reply->country);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorInsertMetadata(NB_MetadataDatabaseOperator* instance,
                                                   const NB_MetadataSourceReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_INSERT_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE, reply->contentType);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_PROJECTION_TYPE, reply->projectionType);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_MANIFEST_STATE, reply->manifestState);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_UPDATE_STATUS, reply->updateStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_SELECTED, reply->selected);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, reply->deleted);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DATASET_SIZE, reply->datasetSize);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOADED_SIZE, reply->downloadedSize);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_FAILED_SIZE, reply->failedSize);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DATUM, reply->datum);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE1, reply->boundingLatitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE1, reply->boundingLongitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE2, reply->boundingLatitude2);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE2, reply->boundingLongitude2);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_ORIGIN_LATITUDE, reply->originLatitude);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_ORIGIN_LONGITUDE, reply->originLongitude);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_SCALE_FACTOR, reply->scaleFactor);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_FALSE_EASTING, reply->falseEasting);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_FALSE_NORTHING, reply->falseNorthing);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_Z_OFFSET, reply->zOffset);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, reply->datasetId);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_MANIFEST_VERSION, reply->manifestVersion);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_URL_BASE, reply->urlBase);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_URL_ARGS_TEMPLATE, reply->urlArgsTemplate);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DISPLAY_NAME, reply->displayName);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_COUNTRY, reply->country);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorDeleteAllMetadata(NB_MetadataDatabaseOperator* instance)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_DELETE_ALL_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorDeleteMetadataByDeleted(NB_MetadataDatabaseOperator* instance)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_DELETE_BY_DELETED_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, (int) TRUE);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetUrlBase(NB_MetadataDatabaseOperator* instance,
                                               uint32 maxUrlBaseSize,
                                               char* urlBase)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    char* tmp = NULL;

    if (!instance || !urlBase)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_URL_BASE_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    err = err != PAL_DB_OK ? err : PAL_DBReaderRead(reader);
    if (err == PAL_DB_ROW)
    {
        err = PAL_DBReaderGetstring(reader, DB_FIELD_URL_BASE, &tmp);
    }
    PAL_DBCommandFinalize(command, reader);

    if ( err == PAL_DB_DONE)
    {
        return NE_NOENT;
    }

    if (err != PAL_DB_OK)
    {
        return NE_BADDATA;
    }

    if (maxUrlBaseSize < nsl_strlen(tmp) + 1)
    {
        nsl_free(tmp);
        return NE_RANGE;
    }

    nsl_memset(urlBase, 0, maxUrlBaseSize);
    nsl_strcpy(urlBase, tmp);
    nsl_free(tmp);

    return NE_OK;
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetAvailableCities(NB_MetadataDatabaseOperator* instance,
                                                       const char* motorwayJunctionObjectsDatasetId,
                                                       CSL_Vector* vectorCities)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !vectorCities)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_AVAILABLE_CITIES_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE1, (int) NB_ECDT_CityModel);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE2, (int) NB_ECDT_SpecificTexture);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, motorwayJunctionObjectsDatasetId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetCities(reader, vectorCities);
    }


    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetAvailableRegions(NB_MetadataDatabaseOperator* instance,
                                                        CSL_Vector* vectorRegions)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !vectorRegions)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_AVAILABLE_REGIONS_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE1, (int) NB_ECDT_MapRegions);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetEntries(reader, vectorRegions, NB_ECDT_MapRegions);
    }


    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetCitiesBySelectedOrderByDatasetId(NB_MetadataDatabaseOperator* instance,
                                                                        nb_boolean selected,
                                                                        CSL_Vector* vectorCities)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !vectorCities)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_SELECTED_ORDER_BY_DATASET_ID_SQL, &command);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE1, (int) NB_ECDT_CityModel);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_SELECTED, (int) selected);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetCities(reader, vectorCities);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetRegionsBySelectedOrderByDatasetId(NB_MetadataDatabaseOperator* instance,
                                                                         nb_boolean selected,
                                                                         CSL_Vector* vectorRegions)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !vectorRegions)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_SELECTED_ORDER_BY_DATASET_ID_SQL, &command);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_CONTENT_TYPE1, (int) NB_ECDT_MapRegions);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_SELECTED, (int) selected);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetEntries(reader, vectorRegions, NB_ECDT_MapRegions);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorSelectCity(NB_MetadataDatabaseOperator* instance,
                                               const char* cityId,
                                               nb_boolean selected)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_UPDATE_SELECTED_BY_DATASET_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_SELECTED, (int) selected);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, cityId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetCityByCityId(NB_MetadataDatabaseOperator* instance,
                                                    const char* cityId,
                                                    NB_EnhancedContentCityData* city)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !cityId || !city)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_DATASET_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, cityId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetCity(reader, city);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetRegionById(NB_MetadataDatabaseOperator* instance,
                                                  const char* regionId,
                                                  NB_ContentEntryData* region)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !regionId || !region)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_SELECT_BY_DATASET_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, regionId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetEntry(reader, region, NB_ECDT_MapRegions);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorMetadataManifestReplyOfMetadataIdExist(NB_MetadataDatabaseOperator* instance,
                                                                           int64 metadataId)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataManifestReply reply = {0};

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_SELECT_BY_METADATA_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nsl_memset(&reply, 0, sizeof(reply));
        nbErr = GetMetadataManifestReply(reader, &reply);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataId(NB_MetadataDatabaseOperator* instance,
                                                                           int64 metadataId,
                                                                           CSL_Vector* vectorReplies)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataManifestReply reply = {0};

    if (!instance || !vectorReplies)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_SELECT_BY_METADATA_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    while (nbErr == NE_OK && err == PAL_DB_OK)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        nbErr = GetMetadataManifestReply(reader, &reply);

        nbErr = nbErr ? nbErr : CSL_VectorAppend(vectorReplies, &reply) ? NE_OK : NE_NOMEM;
    }

    PAL_DBCommandFinalize(command, reader);

    // Ignore NE_NOENT. User should check count of vectorReplies.
    nbErr = nbErr == NE_NOENT ? NE_OK : nbErr;

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataIdAndDownloadStatus(NB_MetadataDatabaseOperator* instance,
                                                                                            int64 metadataId,
                                                                                            int downloadStatus,
                                                                                            CSL_Vector* vectorReplies)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    NB_MetadataManifestReply reply = {0};

    if (!instance || !vectorReplies)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_SELECT_BY_METADATA_ID_AND_DOWNLOAD_STATUS_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOAD_STATUS, downloadStatus);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);

    while (nbErr == NE_OK && err == PAL_DB_OK)
    {
        nsl_memset(&reply, 0, sizeof(reply));

        nbErr = GetMetadataManifestReply(reader, &reply);

        nbErr = nbErr ? nbErr : CSL_VectorAppend(vectorReplies, &reply) ? NE_OK : NE_NOMEM;
    }

    PAL_DBCommandFinalize(command, reader);

    // Ignore NE_NOENT. User should check count of vectorReplies.
    nbErr = nbErr == NE_NOENT ? NE_OK : nbErr;

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(NB_MetadataDatabaseOperator* instance,
                                                                                  int64 metadataId,
                                                                                  const char* fileId,
                                                                                  NB_MetadataManifestReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;

    if (!instance || !fileId || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_SELECT_BY_METADATA_ID_AND_FILE_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_FILE_ID, fileId);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        nbErr = GetMetadataManifestReply(reader, reply);
    }

    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorUpdateManifestDownloadStatusByMetadataId(NB_MetadataDatabaseOperator* instance,
                                                                             int64 metadataId,
                                                                             int downloadStatus,
                                                                             uint32 downloadedSize)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_UPDATE_DOWNLOAD_STATUS_BY_METADATA_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOAD_STATUS, downloadStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOADED_SIZE, downloadedSize);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorUpdateManifest(NB_MetadataDatabaseOperator* instance,
                                                   const NB_MetadataManifestReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_UPDATE_ALL_BY_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_ID, reply->id);
    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, reply->metadataId);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOAD_STATUS, reply->downloadStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_FILE_SIZE, reply->fileSize);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOADED_SIZE, reply->downloadedSize);

    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE1, reply->boundingLatitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE1, reply->boundingLongitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE2, reply->boundingLatitude2);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE2, reply->boundingLongitude2);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_FILE_ID, reply->fileId);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_FILE_VERSION, reply->fileVersion);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorTransactionBegin(NB_MetadataDatabaseOperator* instance)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error error = PAL_DB_OK;

    nbErr = CheckDatabaseConnection(instance);

    if (nbErr != NE_OK)
    {
        return nbErr;
    }
    error = PAL_DB_Transaction_Begin(instance->dbConnection);
    if(error != PAL_DB_OK)
    {
        nbErr = NE_BADDATA;
    }
    return nbErr;
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorTransactionCommit(NB_MetadataDatabaseOperator* instance)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error error = PAL_DB_OK;
    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }
    error = PAL_DB_Transaction_Commit(instance->dbConnection);
    if(error != PAL_DB_OK)
    {
        nbErr = NE_BADDATA;
    }
    return nbErr;
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorTransactionRollback(NB_MetadataDatabaseOperator* instance)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error error = PAL_DB_OK;
    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }
    error = PAL_DB_Transaction_Rollback(instance->dbConnection);
    if(error != PAL_DB_OK)
    {
        nbErr = NE_BADDATA;
    }
    return nbErr;
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorInsertManifest(NB_MetadataDatabaseOperator* instance,
                                                   const NB_MetadataManifestReply* reply)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance || !reply)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_INSERT_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, reply->metadataId);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOAD_STATUS, reply->downloadStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_FILE_SIZE, reply->fileSize);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOADED_SIZE, reply->downloadedSize);

    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE1, reply->boundingLatitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE1, reply->boundingLongitude1);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LATITUDE2, reply->boundingLatitude2);
    err = err != PAL_DB_OK ? err : PAL_DBBindDoubleParam(command, DB_PARAMETER_BOUNDING_LONGITUDE2, reply->boundingLongitude2);

    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_FILE_ID, reply->fileId);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_FILE_VERSION, reply->fileVersion);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorDeleteAllManifest(NB_MetadataDatabaseOperator* instance)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_DELETE_ALL_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorDeleteManifestByMetadataId(NB_MetadataDatabaseOperator* instance,
                                                               int64 metadataId)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_DELETE_BY_METADATA_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/* See header file for description */
NB_Error NB_MetadataDatabaseOperatorDeleteManifestByMetadataIdAndFileId(NB_MetadataDatabaseOperator* instance,
                                                                        int64 metadataId,
                                                                        const char* fileId)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, MANIFEST_DELETE_BY_METADATA_ID_AND_FILE_ID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindLongParam(command, DB_PARAMETER_METADATA_ID, metadataId);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_FILE_ID, fileId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}


// Local Functions ...............................................................................

/*! Create database connection if NULL

@return NB_OK if success
*/
static NB_Error
CheckDatabaseConnection(NB_MetadataDatabaseOperator* instance   /*!< Metadata database operator instance */
                        )
{
    PAL_Error palErr = PAL_Ok;
    PAL_DB_Error dbErr = PAL_DB_OK;
    PAL_Instance* pal = NULL;
    PAL_DBConnect* connection = NULL;
    PAL_DBCommand* command = NULL;
    char pathWithoutFile[MAX_ENHANCED_CONTENT_PATH_LENGTH + 1] = {0};

    if (!instance || !instance->pal)
    {
        return NE_INVAL;
    }

    if (instance->dbConnection)
    {
        return NE_OK;
    }

    pal = instance->pal;

    nsl_memset(pathWithoutFile, 0, sizeof(pathWithoutFile));
    nsl_strlcpy(pathWithoutFile, instance->dbFilename, sizeof(pathWithoutFile));

    palErr = palErr ? palErr : PAL_FileRemovePath(pal, pathWithoutFile);
    palErr = palErr ? palErr : PAL_FileCreateDirectoryEx(pal, pathWithoutFile);
    if (palErr != PAL_Ok)
    {
        return NE_FSYS;
    }

    dbErr = PAL_DBOpen(pal, PAL_DB_Sqlite, instance->dbFilename, &connection);
    if (dbErr != PAL_DB_OK)
    {
        return NE_BADDATA;
    }

    // Create metadata table if not exists.
    dbErr = PAL_DBInitCommand(connection, METADATA_TABLE_CREATE_SQL, &command);
    dbErr= dbErr != PAL_DB_OK ? dbErr : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);
    command = NULL;

    // Create manifest table if not exists.
    dbErr= dbErr != PAL_DB_OK ? dbErr : PAL_DBInitCommand(connection, MANIFEST_TABLE_CREATE_SQL, &command);
    dbErr= dbErr != PAL_DB_OK ? dbErr : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);
    command = NULL;

    if (dbErr != PAL_DB_OK)
    {
        PAL_DBClose(connection);
        return NE_BADDATA;
    }

    instance->dbConnection = connection;

    return NE_OK;
}

/*! Get a NB_MetadataSourceReply object by database reader

@return NB_OK if success
*/
static NB_Error
GetMetadataSourceReply(PAL_DBReader* reader,                    /*!< Database reader */
                       NB_MetadataSourceReply* reply            /*!< On return metadata source reply */
                       )
{
    PAL_DB_Error err = PAL_DB_OK;
    char* tmp = NULL;

    if (!reader || !reply)
    {
        return NE_INVAL;
    }

    err = PAL_DBReaderRead(reader);
    if (err == PAL_DB_ROW)
    {
        nsl_memset(reply, 0, sizeof(*reply));
        err = PAL_DBReaderGetLong(reader, DB_FIELD_ID, &reply->id);

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_CONTENT_TYPE, &reply->contentType);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_PROJECTION_TYPE, &reply->projectionType);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_MANIFEST_STATE, &reply->manifestState);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_UPDATE_STATUS, &reply->updateStatus);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_SELECTED, &reply->selected);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DELETED, &reply->deleted);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DATASET_SIZE, &reply->datasetSize);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOADED_SIZE, &reply->downloadedSize);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_FAILED_SIZE, &reply->failedSize);

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DATUM, &reply->datum);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE1, &reply->boundingLatitude1);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE1, &reply->boundingLongitude1);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE2, &reply->boundingLatitude2);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE2, &reply->boundingLongitude2);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_ORIGIN_LATITUDE, &reply->originLatitude);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_ORIGIN_LONGITUDE, &reply->originLongitude);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_SCALE_FACTOR, &reply->scaleFactor);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_FALSE_EASTING, &reply->falseEasting);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_FALSE_NORTHING, &reply->falseNorthing);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_Z_OFFSET, &reply->zOffset);

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_DATASET_ID, &tmp);
        if (err == PAL_DB_OK && tmp != NULL)
        {
            nsl_strlcpy(reply->datasetId, tmp, sizeof(reply->datasetId));
            nsl_free(tmp);
            tmp = NULL;
        }

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_MANIFEST_VERSION, &tmp);
        if (err == PAL_DB_OK && tmp != NULL)
        {
            nsl_strlcpy(reply->manifestVersion, tmp, sizeof(reply->manifestVersion));
            nsl_free(tmp);
            tmp = NULL;
        }

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_URL_BASE, &tmp);
        if (err == PAL_DB_OK && tmp != NULL)
        {
            nsl_strlcpy(reply->urlBase, tmp, sizeof(reply->urlBase));
            nsl_free(tmp);
            tmp = NULL;
        }

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_URL_ARGS_TEMPLATE, &tmp);
        if (err == PAL_DB_OK && tmp != NULL)
        {
            nsl_strlcpy(reply->urlArgsTemplate, tmp, sizeof(reply->urlArgsTemplate));
            nsl_free(tmp);
            tmp = NULL;
        }

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_DISPLAY_NAME, &tmp);
        if (err == PAL_DB_OK && tmp != NULL)
        {
            nsl_strlcpy(reply->displayName, tmp, sizeof(reply->displayName));
            nsl_free(tmp);
            tmp = NULL;
        }

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_COUNTRY, &tmp);
        if (err == PAL_DB_OK && tmp != NULL)
        {
            nsl_strlcpy(reply->country, tmp, sizeof(reply->country));
            nsl_free(tmp);
            tmp = NULL;
        }
    }

    if (err == PAL_DB_DONE)
    {
        return NE_NOENT;
    }

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/*! Get a NB_MetadataManifestReply object by database reader

@return NB_OK if success
*/
static NB_Error
GetMetadataManifestReply(PAL_DBReader* reader,                  /*!< Database reader */
                         NB_MetadataManifestReply* reply        /*!< On return metadata manifest reply */
                         )
{
    PAL_DB_Error err = PAL_DB_OK;
    char* tmp = NULL;

    if (!reader || !reply)
    {
        return NE_INVAL;
    }

    err = PAL_DBReaderRead(reader);
    if (err == PAL_DB_ROW)
    {
        nsl_memset(reply, 0, sizeof(*reply));
        err = PAL_DBReaderGetLong(reader, DB_FIELD_ID, &reply->id);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetLong(reader, DB_FIELD_METADATA_ID, &reply->metadataId);

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOAD_STATUS, &reply->downloadStatus);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_FILE_SIZE, &reply->fileSize);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOADED_SIZE, &reply->downloadedSize);

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE1, &reply->boundingLatitude1);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE1, &reply->boundingLongitude1);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE2, &reply->boundingLatitude2);
        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE2, &reply->boundingLongitude2);

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_FILE_ID, &tmp);
        if (err == PAL_DB_OK)
        {
            nsl_strlcpy(reply->fileId, tmp, sizeof(reply->fileId));
            nsl_free(tmp);
            tmp = NULL;
        }

        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_FILE_VERSION, &tmp);
        if (err == PAL_DB_OK)
        {
            nsl_strlcpy(reply->fileVersion, tmp, MAX_ENHANCED_CONTENT_CITY_VERSION_LENGTH + 1);
            nsl_free(tmp);
            tmp = NULL;
        }
    }

    if (err == PAL_DB_DONE)
    {
        return NE_NOENT;
    }

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}

/*! Get a NB_EnhancedContentCityData object by database reader

@return NB_OK if success
*/
static NB_Error
GetCity(PAL_DBReader* reader,                                   /*!< Database reader */
        NB_EnhancedContentCityData* city                        /*!< On return city data */
        )
{
   return GetEntry(reader, city, NB_ECDT_CityModel);
}

static NB_Error GetEntry(PAL_DBReader* reader, NB_ContentEntryData* entry, NB_EnhancedContentDataType dataType)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    int contentType = (int) NB_ECDT_None;
    int selected = (int) FALSE;
    int deleted = (int) FALSE;
    int updateStatus = (int) NB_ECUS_UpdateStatusNew;
    int datasetSize = 0;
    int downloadedSize = 0;
    int failedSize = 0;
    char* tmp = NULL;

    if (!reader || !entry)
    {
        return NE_INVAL;
    }

    nsl_memset(entry, 0, sizeof(*entry));
    entry->updateStatus = NB_ECUS_UpdateStatusNew;

    while (nbErr == NE_OK)
    {
        err = PAL_DBReaderRead(reader);
        if (err == PAL_DB_ROW)
        {
            err = PAL_DBReaderGetstring(reader, DB_FIELD_DATASET_ID, &tmp);
            if (err == PAL_DB_OK)
            {
                if (nsl_strlen(entry->cityId) > 0 && nsl_strcmp(entry->cityId, tmp) != 0)
                {
                    nsl_free(tmp);
                    tmp = NULL;

                    nsl_memset(entry, 0, sizeof(*entry));
                    nbErr = NE_INVAL;
                    break;
                }
                nsl_strlcpy(entry->cityId, tmp, sizeof(entry->cityId));
                nsl_free(tmp);
                tmp = NULL;

                err = PAL_DBReaderGetInt(reader, DB_FIELD_CONTENT_TYPE, &contentType);
                switch ((NB_EnhancedContentDataType) contentType)
                {
                    case NB_ECDT_MapRegions:
                    case NB_ECDT_CityModel:
                    {
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_MANIFEST_VERSION, &tmp);
                        if (err == PAL_DB_OK)
                        {
                            nsl_strcpy(entry->version, tmp);
                            nsl_free(tmp);
                            tmp = NULL;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_UPDATE_STATUS, &updateStatus);
                        if (err == PAL_DB_OK)
                        {
                            entry->updateStatus = updateStatus;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DATASET_SIZE, &datasetSize);
                        if (err == PAL_DB_OK)
                        {
                            entry->cityModelDataSize = datasetSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOADED_SIZE, &downloadedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry->downloadedCityModelDataSize = downloadedSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_FAILED_SIZE, &failedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry->failedCityModelDataSize = failedSize;
                        }

                        // Get display name, selected, deleted, box and center of city model.
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_DISPLAY_NAME, &tmp);
                        if (err == PAL_DB_OK)
                        {
                            nsl_strcpy(entry->displayName, tmp);
                            nsl_free(tmp);
                            tmp = NULL;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_SELECTED, &selected);
                        if (err == PAL_DB_OK)
                        {
                            entry->selected = (nb_boolean)selected;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DELETED, &deleted);
                        if (err == PAL_DB_OK)
                        {
                            entry->deleted = (nb_boolean)deleted;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE1, &entry->box.point1.latitude);
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE1, &entry->box.point1.longitude);
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE2, &entry->box.point2.latitude);
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE2, &entry->box.point2.longitude);

                        if (err == PAL_DB_OK)
                        {
                            entry->center.latitude = (entry->box.point1.latitude + entry->box.point2.latitude) / 2.0;
                            entry->center.longitude = (entry->box.point1.longitude + entry->box.point2.longitude) / 2.0;
                        }

                        break;
                    }
                    case NB_ECDT_SpecificTexture:
                    {
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_MANIFEST_VERSION, &tmp);
                        if (err == PAL_DB_OK)
                        {
                            nsl_strcpy(entry->textureVersion, tmp);
                            nsl_free(tmp);
                            tmp = NULL;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DATASET_SIZE, &datasetSize);
                        if (err == PAL_DB_OK)
                        {
                            entry->textureDataSize = datasetSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOADED_SIZE, &downloadedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry->downloadedTextureDataSize = downloadedSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_FAILED_SIZE, &failedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry->failedTextureDataSize = failedSize;
                        }

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
        else if (err == PAL_DB_DONE)
        {
            if (nsl_strlen(entry->cityId) <= 0)
            {
                nbErr = NE_NOENT;
            }
            break;
        }

        if (err != PAL_DB_OK)
        {
            nbErr = NE_BADDATA;
            break;
        }
    }

    return nbErr;
}

/*! Get NB_EnhancedContentCityData objects by database reader

@return NB_OK if success
*/
static NB_Error
GetCities(PAL_DBReader* reader,                                 /*!< Database reader */
          CSL_Vector* vectorCities                              /*!< On return vector of NB_EnhancedContentCityData */
          )
{
    return GetEntries(reader, vectorCities, NB_ECDT_CityModel);
}
static NB_Error GetEntries(PAL_DBReader* reader, CSL_Vector* vectorEntries, NB_EnhancedContentDataType dataType)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    int contentType = (int) NB_ECDT_None;
    int selected = (int) FALSE;
    int deleted = (int) FALSE;
    int updateStatus = (int) NB_ECUS_UpdateStatusNew;
    int datasetSize = 0;
    int downloadedSize = 0;
    int failedSize = 0;
    char* tmp = NULL;
    NB_ContentEntryData entry = {0};

    if (!reader || !vectorEntries)
    {
        return NE_INVAL;
    }

    nsl_memset(&entry, 0, sizeof(entry));

    while (nbErr == NE_OK)
    {
        err = PAL_DBReaderRead(reader);
        if (err == PAL_DB_ROW)
        {
            err = PAL_DBReaderGetstring(reader, DB_FIELD_DATASET_ID, &tmp);
            if (err == PAL_DB_OK)
            {
                if (nsl_strlen(entry.cityId) > 0 && nsl_strcmp(entry.cityId, tmp) != 0)
                {
                    nbErr = CSL_VectorAppend(vectorEntries, &entry) ? NE_OK : NE_NOMEM;
                    if (nbErr != NE_OK)
                    {
                        nsl_free(tmp);
                        tmp = NULL;
                        break;
                    }

                    nsl_memset(&entry, 0, sizeof(entry));
                }
                nsl_strlcpy(entry.cityId, tmp, sizeof(entry.cityId));
                nsl_free(tmp);
                tmp = NULL;

                err = PAL_DBReaderGetInt(reader, DB_FIELD_CONTENT_TYPE, &contentType);
                switch ((NB_EnhancedContentDataType) contentType)
                {
                case NB_ECDT_CityModel:
                case NB_ECDT_MapRegions:
                    {
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_MANIFEST_VERSION, &tmp);
                        if (err == PAL_DB_OK)
                        {
                            nsl_strcpy(entry.version, tmp);
                            nsl_free(tmp);
                            tmp = NULL;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_UPDATE_STATUS, &updateStatus);
                        if (err == PAL_DB_OK)
                        {
                            entry.updateStatus = updateStatus;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DATASET_SIZE, &datasetSize);
                        if (err == PAL_DB_OK)
                        {
                            entry.cityModelDataSize = datasetSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOADED_SIZE, &downloadedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry.downloadedCityModelDataSize = downloadedSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_FAILED_SIZE, &failedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry.failedCityModelDataSize = failedSize;
                        }

                        // Get display name, selected, deleted, box and center of city model.
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_DISPLAY_NAME, &tmp);
                        if (err == PAL_DB_OK)
                        {
                            nsl_strcpy(entry.displayName, tmp);
                            nsl_free(tmp);
                            tmp = NULL;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_SELECTED, &selected);
                        if (err == PAL_DB_OK)
                        {
                            entry.selected = (nb_boolean)selected;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DELETED, &deleted);
                        if (err == PAL_DB_OK)
                        {
                            entry.deleted = (nb_boolean)deleted;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE1, &entry.box.point1.latitude);
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE1, &entry.box.point1.longitude);
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LATITUDE2, &entry.box.point2.latitude);
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetdouble(reader, DB_FIELD_BOUNDING_LONGITUDE2, &entry.box.point2.longitude);

                        if (err == PAL_DB_OK)
                        {
                            entry.center.latitude = (entry.box.point1.latitude + entry.box.point2.latitude) / 2.0;
                            entry.center.longitude = (entry.box.point1.longitude + entry.box.point2.longitude) / 2.0;
                        }

                        break;
                    }
                    case NB_ECDT_SpecificTexture:
                    {
                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetstring(reader, DB_FIELD_MANIFEST_VERSION, &tmp);
                        if (err == PAL_DB_OK)
                        {
                            nsl_strcpy(entry.textureVersion, tmp);
                            nsl_free(tmp);
                            tmp = NULL;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DATASET_SIZE, &datasetSize);
                        if (err == PAL_DB_OK)
                        {
                            entry.textureDataSize = datasetSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOADED_SIZE, &downloadedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry.downloadedTextureDataSize = downloadedSize;
                        }

                        err = err != PAL_DB_OK ? err : PAL_DBReaderGetInt(reader, DB_FIELD_FAILED_SIZE, &failedSize);
                        if (err == PAL_DB_OK)
                        {
                            entry.failedTextureDataSize = failedSize;
                        }

                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
        else if (err == PAL_DB_DONE)
        {
            if (nsl_strlen(entry.cityId) > 0)
            {
                nbErr = CSL_VectorAppend(vectorEntries, &entry) ? NE_OK : NE_NOMEM;
            }
            break;
        }

        if (err != PAL_DB_OK)
        {
            nbErr = NE_BADDATA;
            break;
        }
    }

    return nbErr;
}

NB_Error NB_MetadataDatabaseOperatorGetExistMapRegionFileIds(NB_MetadataDatabaseOperator* instance,
                                                            int32* count,
                                                            char*** fileIds)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    char* fileId = NULL;
    PAL_DB_Error readError = PAL_DB_OK;
    int32 mallocSize = 0;
    if (!instance || !count || !fileIds)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection,
                            METADATA_MANIFEST_SELECT_BY_DOWNLOADED_AND_NOT_DELETED_SQL,
                            &command);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command,
                                                    DB_PARAMETER_CONTENT_TYPE1,
                                                    (int) NB_ECDT_MapRegions);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command,
                                                    DB_PARAMETER_DOWNLOAD_STATUS,
                                                    NB_ECDS_DownloadComplete);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DELETED, FALSE);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        *count = 0;
        readError = PAL_DBResetReader(reader);
        while (nbErr == NE_OK)
        {
            readError = PAL_DBReaderRead(reader);
            if (readError == PAL_DB_DONE)
            {
                nbErr = NE_OK;
                break;
            }
            //alloc a large area at first and then enlarge it if not enough.
            if ((*count + 1) > mallocSize)
            {
                mallocSize += MEMORY_ALLOCATED_UNIT;
                *fileIds = (char**)nsl_realloc(*fileIds, sizeof(char*) * (mallocSize));
            }

            if ((readError = readError == PAL_DB_ROW ?
                    PAL_DBReaderGetstring(reader, DB_FIELD_FILE_ID, &fileId) :
                    readError)
                    == PAL_DB_OK)
            {
                (*fileIds)[(*count)] = fileId;
                ++(*count);
                nbErr = NE_OK;
            }
            else
            {
                while (*count >= 0)
                {
                    nsl_free(*fileIds[--(*count)]);
                }
                nsl_free(*fileIds);
                nbErr = NE_BADDATA;
                break;
            }
        }
    }
    PAL_DBCommandFinalize(command, reader);

    return nbErr != NE_OK ? nbErr : (err != PAL_DB_OK ? NE_BADDATA : NE_OK);
}

nb_boolean NB_MetadataDatabaseOperatorCheckDownloadStatusIsPausedByDatasetId(
    NB_MetadataDatabaseOperator* instance,
    const char* datasetId)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;
    PAL_DBReader* reader = NULL;
    PAL_DB_Error readError = PAL_DB_OK;
    int status = 0;
    if (!instance || !datasetId)
    {
        return FALSE;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return FALSE;
    }

    err = PAL_DBInitCommand(instance->dbConnection,
                            METADATA_MANIFEST_SELECT_DOWNLOADSTATUS_BY_DATASETID_SQL,
                            &command);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_FIELD_DATASET_ID, datasetId);
    err = err != PAL_DB_OK ? err : PAL_DBExecuteReader(command, &reader);
    if (err == PAL_DB_OK)
    {
        readError = PAL_DBResetReader(reader);
        while (nbErr == NE_OK)
        {
            readError = PAL_DBReaderRead(reader);
            if (readError == PAL_DB_DONE)
            {
                nbErr = NE_OK;
                break;
            }
            if ((readError = readError == PAL_DB_ROW ?
                    PAL_DBReaderGetInt(reader, DB_FIELD_DOWNLOAD_STATUS, &status) :
                    readError)
                    == PAL_DB_OK)
            {
                if (status == NB_ECDS_DownloadPaused)
                {
                    return TRUE;
                }
                nbErr = NE_OK;
            }
            else
            {
                nbErr = NE_BADDATA;
                break;
            }
        }
    }
    PAL_DBCommandFinalize(command, reader);

    return FALSE;
}
NB_Error NB_MetadataDatabaseOperatorUpdateDownloadStatusByDatasetId(
    NB_MetadataDatabaseOperator* instance,
    NB_EnhancedContentDownloadStatus downloadStatus,
    const char* datasetId)
{
    NB_Error nbErr = NE_OK;
    PAL_DB_Error err = PAL_DB_OK;
    PAL_DBCommand* command = NULL;

    if (!instance)
    {
        return NE_INVAL;
    }

    nbErr = CheckDatabaseConnection(instance);
    if (nbErr != NE_OK)
    {
        return nbErr;
    }

    err = PAL_DBInitCommand(instance->dbConnection, METADATA_MANIFEST_UPDATE_DOWNLOADSTATUS_BY_DATASETID_SQL, &command);

    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOAD_STATUS1, downloadStatus);
    err = err != PAL_DB_OK ? err : PAL_DBBindIntParam(command, DB_PARAMETER_DOWNLOAD_STATUS2, NB_ECDS_DownloadComplete);
    err = err != PAL_DB_OK ? err : PAL_DBBindStringParam(command, DB_PARAMETER_DATASET_ID, datasetId);

    err = err != PAL_DB_OK ? err : PAL_DBExecuteNonQueryCommand(command);
    PAL_DBCommandFinalize(command, NULL);

    return (err == PAL_DB_OK ? NE_OK : NE_BADDATA);
}
/*! @} */
