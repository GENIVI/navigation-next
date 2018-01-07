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

    @file     nbmetadatadatabaseoperator.h
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

#ifndef NBMETADATADATABASEOPERATOR_H
#define NBMETADATADATABASEOPERATOR_H

/*!
    @addtogroup nbmetadatadatabaseoperator
    @{
*/

#include "nbmetadatatypes.h"
#include "nberror.h"
#include "datautil.h"
#include "pal.h"

// Public Constants ..............................................................................


// Public Types ..................................................................................

typedef struct NB_MetadataDatabaseOperator NB_MetadataDatabaseOperator;


// Public Functions ..............................................................................

/*! Create a new instance of a NB_MetadataDatabaseOperator object

@return NB_OK if success
@see NB_MetadataDatabaseOperatorDestroy
*/
NB_Error NB_MetadataDatabaseOperatorCreate(
    PAL_Instance* pal,                                      /*!< PAL instance */
    const char* databaseFullFilename,                       /*!< Database full filename */
    NB_MetadataDatabaseOperator** instance                  /*!< On success, returns pointer to metadata database operator object */
    );

/*! Destroy an instance of a NB_MetadataDatabaseOperator object

@return NB_OK if success
@see NB_MetadataDatabaseOperatorCreate
*/
NB_Error NB_MetadataDatabaseOperatorDestroy(
    NB_MetadataDatabaseOperator* instance                   /*!< Metadata database operator instance */
    );

/*! Delete database file and close database connection

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorMasterClear(
    NB_MetadataDatabaseOperator* instance                   /*!< Metadata database operator instance */
    );

/*! Set update status by content type in metadata table.

    @return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorSetMetadataUpdateStatusByContentType(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int updateStatus,                                       /*!< NB_EnhancedContentUpdateStatus */
    int contentType                                         /*!< NB_EnhancedContentDataType */
    );

/*! Set all rows deleted in metadata table for preparing to update.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorSetAllMetadataDeleted(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    nb_boolean deleted                                      /*!< Is deleted */
    );

/*! Set a row deleted in metadata table for found.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorSetMetadataDeletedById(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    nb_boolean deleted,                                     /*!< Is deleted. */
    int64 id                                                /*!< Database-generated id */
    );

/*! Get a NB_MetadataSourceReply object by id from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceReplyById(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 id,                                               /*!< Index to Metadata record for dataset info */
    NB_MetadataSourceReply* reply                           /*!< On return metadata source reply */
    );

/*! Get NB_MetadataSourceReply objects by manifest state from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByManifestState(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int manifestState,                                      /*!< NB_EnhancedContentManifestState */
    CSL_Vector* vectorReplies                               /*!< On return vector of NB_MetadataSourceReply */
    );

/*! Get NB_MetadataSourceReply objects by content type from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentType(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int contentType,                                        /*!< NB_EnhancedContentDataType */
    CSL_Vector* vectorReplies                               /*!< On return vector of NB_MetadataSourceReply */
    );

/*! Get a NB_MetadataSourceReply object by content type and dataset id from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceReplyByContentTypeAndDatasetId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int contentType,                                        /*!< NB_EnhancedContentDataType */
    const char* datasetId,                                  /*!< Data identifier */
    NB_MetadataSourceReply* reply                           /*!< On return metadata source reply */
    );

/*! Get NB_MetadataSourceReply objects by content type and selected from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByContentTypeAndSelected(
   NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
   int contentType,                                        /*!< NB_EnhancedContentDataType */
   nb_boolean selected,                                    /*!< Is selected */
   CSL_Vector* vectorReplies                               /*!< On return vector of NB_MetadataSourceReply */
   );

/*! Get NB_MetadataSourceReply objects by deleted from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataSourceRepliesByDeleted(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    nb_boolean deleted,                                     /*!< Is deleted */
    CSL_Vector* vectorReplies                               /*!< On return vector of NB_MetadataSourceReply */
    );

/*! Update a row by id in metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorUpdateMetadata(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const NB_MetadataSourceReply* reply                     /*!< Metadata source reply */
    );

/*! Insert a row in metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorInsertMetadata(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const NB_MetadataSourceReply* reply                     /*!< Metadata source reply */
    );

/*! Delete all rows in metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorDeleteAllMetadata(
    NB_MetadataDatabaseOperator* instance                   /*!< Metadata database operator instance */
    );

/*! Delete rows by deleted in metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorDeleteMetadataByDeleted(
    NB_MetadataDatabaseOperator* instance                   /*!< Metadata database operator instance */
    );

/*! Get base url in metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetUrlBase(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    uint32 maxUrlBaseSize,                                  /*!< Maximum size of constructed url base */
    char* urlBase                                           /*!< Buffer to receive constructed url base */
    );

/*! Get NB_EnhancedContentCityData objects of all cities from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetAvailableCities(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const char* motorwayJunctionObjectsDatasetId,           /*!< Dataset id for motorway junction objects */
    CSL_Vector* vectorCities                                /*!< On return vector of NB_EnhancedContentCityData */
    );

/*! Get all regions from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetAvailableRegions(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    CSL_Vector* vectorRegions                               /*!< On return vector of regions */
    );


/*! Get NB_EnhancedContentCityData objects by selected from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetCitiesBySelectedOrderByDatasetId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    nb_boolean selected,                                    /*!< Is selected */
    CSL_Vector* vectorCities                                /*!< On return vector of NB_EnhancedContentCityData */
    );

/*! Get NB_EnhancedContentCityData objects by selected from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetRegionsBySelectedOrderByDatasetId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    nb_boolean selected,                                    /*!< Is selected */
    CSL_Vector* vectorRegions);                             /*!< On return vector of NB_ContentEntryData */

/*! Update selected of rows by dataset id in metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorSelectCity(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const char* cityId,                                     /*!< City identifier */
    nb_boolean selected                                     /*!< Is city selected for download? */
    );

/*! Get a NB_EnhancedContentCityData object by city id from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetCityByCityId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const char* cityId,                                     /*!< City identifier */
    NB_EnhancedContentCityData* city                        /*!< On return city data */
    );

/*! Get a NB_ContentEntryData object by region id from metadata table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetRegionById(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const char* regionId,                                   /*!< region identifier */
    NB_ContentEntryData* region                             /*!< On return region data */
    );

/*! Check if NB_MetadataManifestReply objects of metadata id exist in manifest table.

@return NB_OK if exist, NB_NOENT if not exist
*/
NB_Error NB_MetadataDatabaseOperatorMetadataManifestReplyOfMetadataIdExist(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId                                        /*!< Index to Metadata record for dataset info */
    );

/*! Get NB_MetadataManifestReply objects by metadata id from manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId,                                       /*!< Index to Metadata record for dataset info */
    CSL_Vector* vectorReplies                               /*!< On return vector of NB_MetadataManifestReply */
    );

/*! Get NB_MetadataManifestReply objects by metadata id of non downloaded from manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataManifestRepliesByMetadataIdAndDownloadStatus(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId,                                       /*!< Index to Metadata record for dataset info */
    int downloadStatus,                                     /*!< NB_EnhancedContentDownloadStatus */
    CSL_Vector* vectorReplies                               /*!< On return vector of NB_MetadataManifestReply */
    );

/*! Get a NB_MetadataManifestReply object by metadata id and file id from manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetMetadataManifestReplyByMetadataIdAndFileId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId,                                       /*!< Index to Metadata record for dataset info */
    const char* fileId,                                     /*!< Data file identifier */
    NB_MetadataManifestReply* reply                         /*!< On return Metadata manifest reply */
    );

/*! Update downloaded status of rows in manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorUpdateManifestDownloadStatusByMetadataId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId,                                       /*!< Index to Metadata record for dataset info */
    int downloadStatus,                                     /*!< NB_EnhancedContentDownloadStatus */
    uint32 downloadedSize                                   /*!< Downloaded size */
    );

/*! Update a row by id in manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorUpdateManifest(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const NB_MetadataManifestReply* reply                   /*!< Metadata manifest reply */
    );

/*! Insert a row in manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorInsertManifest(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const NB_MetadataManifestReply* reply                         /*!< Metadata manifest reply */
    );

/*! Delete all rows in manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorDeleteAllManifest(
    NB_MetadataDatabaseOperator* instance                   /*!< Metadata database operator instance */
    );

/*! Delete rows by metadata id in manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorDeleteManifestByMetadataId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId                                        /*!< Index to Metadata record for dataset info */
    );

/*! Delete a row by metadata id and file id in manifest table.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorDeleteManifestByMetadataIdAndFileId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int64 metadataId,                                       /*!< Index to Metadata record for dataset info */
    const char* fileId                                      /*!< Data file identifier */
    );

/*! Get the existing (downloaded and not deleted) regional file id for NCDB.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorGetExistMapRegionFileIds(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    int32* count,                                           /*!< on return the count of file ids */
    char*** fileIds                                         /*!< on return the pointer to file ids */
    );

/*! check if the dataset's download status is paused.

@return TRUE if paused, FALSE if not
*/
nb_boolean NB_MetadataDatabaseOperatorCheckDownloadStatusIsPausedByDatasetId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    const char* datasetId);                                 /*!< dataset id */

/*!Update download status by dataset id.

@return NB_OK if success
*/
NB_Error NB_MetadataDatabaseOperatorUpdateDownloadStatusByDatasetId(
    NB_MetadataDatabaseOperator* instance,                  /*!< Metadata database operator instance */
    NB_EnhancedContentDownloadStatus downloadStatus,        /*!< new download status */
    const char* datasetId);                                 /*!< dataset id */

/*! begin a transaction. */
NB_Error NB_MetadataDatabaseOperatorTransactionBegin(NB_MetadataDatabaseOperator* instance);

/*! end a transaction. */
NB_Error NB_MetadataDatabaseOperatorTransactionCommit(NB_MetadataDatabaseOperator* instance);

/*! rollback a transaction. */
NB_Error NB_MetadataDatabaseOperatorTransactionRollback(NB_MetadataDatabaseOperator* instance);

/*! @} */

#endif
