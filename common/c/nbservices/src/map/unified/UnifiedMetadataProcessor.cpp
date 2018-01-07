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
   @file        UnifiedMetadataProcessor.cpp
   @defgroup    nbmap

   Description: Implementation of UnifiedMetadataProcessor.

*/
/*
   (C) Copyright 2015 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */

extern "C"
{
#include "palfile.h"
#include "palclock.h"
#include "nbpersistentdataprotected.h"
}

#include "UnifiedMetadataProcessor.h"
#include "MetadataConfiguration.h"
#include "nbcontextprotected.h"
#include "TpsElement.h"
#include "NBProtocolMetadataSourceInformationSerializer.h"
#include "NBProtocolMetadataSourceParametersSerializer.h"
#include "StringUtility.h"
#include "nbmacros.h"

using namespace protocol;
using namespace nbmap;

// Some types which are always wanted by client.
static const char* WANTED_DATA_TYPES[] =
{
    "unified binary model", "raster map"
};

/*! Help functor to check if a WantedContent is of type. */
class ContentTypeFinder
{
public:
    ContentTypeFinder(shared_ptr<string> type)
            : m_type(type) {}
    virtual ~ContentTypeFinder() {}
    bool operator () (const protocol::WantedContentSharedPtr& content) const
    {
        return content && !nbcommon::StringUtility::CompareStdString(m_type, content->GetType());
    }
private:
    shared_ptr<string> m_type;
};

/*! Updates WantUnifiedMaps section.

  @return NB_OK if succeeded, or other value if failed.
*/
template <typename T1, typename T2>  NB_Error
UpdateWantOptions(T1* element,
                  void (T1::*Setter)(shared_ptr<T2>),
                  bool want)
{
    shared_ptr<T2> wantOption(CCC_NEW T2);
    if (want)
    {
        if (!wantOption)
        {
            return NE_NOMEM;
        }
        (element->*Setter)(wantOption);
    }
    return NE_OK;
}

/*! Updates WantedContent section.
*/
template <typename T1>  void
UpdateWantedContent(const shared_ptr<vector<shared_ptr<T1> > > array,
                    string type,
                    bool& want)
{
    shared_ptr<string> contentType(CCC_NEW string(type));

    typename vector<shared_ptr<T1> >::const_iterator it =
                        find_if(array->begin(), array->end(), ContentTypeFinder(contentType));

    want = (it !=  array->end()) ? true : false;
}

// Implementation of UnifiedMetadataProcessor .....................................................

/* See description in header file. */
UnifiedMetadataProcessor::UnifiedMetadataProcessor(NB_Context* context)
{
    m_pContext = context;
    m_currentRetryTimes = 0;
    m_lastSuccessTime = 0;
    m_metadataMaxRetryTime = 0;
    m_metadataRequestInProgress = false;
}

/* See description in header file. */
UnifiedMetadataProcessor::~UnifiedMetadataProcessor(void)
{
}


/* See description in header file. */
void
UnifiedMetadataProcessor::CheckMetadataChanges(shared_ptr<AsyncCallback<bool> > callback,
                                               shared_ptr<MetadataConfiguration> config,
                                               bool skipTimeInterval,
                                               bool forceUpdate)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (m_metadataRequestInProgress)
    {
        callback->Error(NE_BUSY);
        return;
    }

    if (!callback)
    {
        //No way to inform caller about the error, since callback is null. So just return.
        return;
    }

    if (m_pContext == NULL)
    {
        callback->Error(NE_NOTINIT);
        return;
    }


    if (!m_metadataInformation)
    {
        GetMetadataInformationFromPersistentData(m_metadataPath);
    }

    if (forceUpdate)
    {
        // Force to clean metadata first.
        Reset();
    }

    // Tell caller that metadata is not changed if configuration is not changed, and last
    // successful request is not longer than one hour.
    if (!IsNeedUpdateMetadataForTime(config, skipTimeInterval))
    {
        callback->Success(false);
        return;
    }

    if (config && !(*config == *m_pConfig))
    {
        // If we are request metadata using a different configuration, reset previously
        // received metadata which has been deprecated.
       m_metadataInformation.reset();
       *m_pConfig = *config;
    }

    NB_Error error = ConfigurationToSourceParameter(m_pConfig);
    if (error != NE_OK)
    {
        callback->Error(error);
        return;
    }

    StartMetadataRequestWithCallback(callback); // Send metadata request;
}

/* See description in header file. */
MetadataSourceInformationSharedPtr
UnifiedMetadataProcessor::GetMetadataSourceInformation(void)
{
    return m_metadataInformation;
}

/* See description in header file. */
void UnifiedMetadataProcessor::Reset(void)
{
    // Clear persistent metadata.
    if (m_pContext)
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (pal && m_metadataPath && (!(m_metadataPath->empty())))
        {
            const char* metadataPath = m_metadataPath->c_str();
            if (metadataPath && (nsl_strlen(metadataPath) > 0))
            {
                // Remove file for persistent metadata and ignore returned error.
                PAL_FileRemove(pal, metadataPath);
            }
        }
    }

    // Reset other statistics.
    m_currentRetryTimes         = 0;
    m_lastSuccessTime           = 0;
    m_metadataMaxRetryTime      = 0;
    m_metadataRequestInProgress = false;
    m_metadataInformation.reset();
}

/* See description in header file. */
NB_Context* UnifiedMetadataProcessor::GetContext(void)
{
    return  m_pContext;
}

/* See description in header file. */
void UnifiedMetadataProcessor::SetMetadataRetryTimes(int number)
{
    m_metadataMaxRetryTime = number;
}

/* See description in header file. */
NB_Error UnifiedMetadataProcessor::SetPersistentMetadataPath(shared_ptr<string> metadataPath)
{
    NB_Error error = NE_OK;
    do
    {
        // Check if the full path of persistent metadata is valid.
        if ((!metadataPath) || (metadataPath->empty()))
        {
            error = NE_INVAL;
            break;
        }

        if (!m_pContext)
        {
            error = NE_NOTINIT;
            break;
        }

        // Get the PAL instance.
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (!pal)
        {
            error = NE_NOTINIT;
            break;
        }

        // Check if the full path of persistent metadata is a file not a directory.
        if ((PAL_FileExists(pal, metadataPath->c_str()) == PAL_Ok) &&
            (PAL_FileIsDirectory(pal, metadataPath->c_str())))
        {
            error = NE_INVAL;
            break;
        }

        m_metadataPath = metadataPath;

    } while (0);

    return error;
}

/* See description in header file. */
shared_ptr<MetadataConfiguration> UnifiedMetadataProcessor::CopyCurrentMetadataConfig(void)
{
    if (!m_pConfig)
    {
        GetMetadataInformationFromPersistentData(m_metadataPath);
    }

    nsl_assert(m_pConfig.get() != NULL);
    shared_ptr <MetadataConfiguration> config(CCC_NEW MetadataConfiguration(*m_pConfig));
    return config;
}

bool UnifiedMetadataProcessor::IsNeedUpdateMetadataForTime(shared_ptr<MetadataConfiguration> /*config*/,
                                                           bool /*skipTimeInterval*/)
{
    return false;
}

/* See description in header file. */
NB_Error UnifiedMetadataProcessor::GetMetadataInformationFromPersistentData(shared_ptr<string> metadataPath)
{
    NB_Error error = NE_OK;
    do
    {
        /* Check the members to parse the metadata source information. The metadata source
           information should not be parsed if it already exists.
        */
        if ((!m_pContext) || (!metadataPath) || metadataPath->empty())
        {
            error = NE_NOTINIT;
            break;
        }

        if (m_metadataInformation)
        {
            // has been loaded, just return;
            break;
        }

        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (!pal)
        {
            error = NE_NOTINIT;
            break;
        }

        const char* storedPath = metadataPath->c_str();
        if ((!storedPath) || (nsl_strlen(storedPath) <= 0))
        {
            // The path to store metadata information has already be checked before.
            error = NE_INVAL;
            break;
        }

        // Check if the file is existing.
        if ((PAL_FileExists(pal, storedPath) != PAL_Ok) ||
            (PAL_FileIsDirectory(pal, storedPath)))
        {
            error = NE_INVAL;
            break;
        }

        // Get the data from the file.
        uint32         dataSize = 0;
        unsigned char* data     = NULL;
        if ((PAL_FileLoadFile(pal, storedPath, &data, &dataSize) != PAL_Ok) ||
            (!data))
        {
            error = NE_INVAL;
            break;
        }

        // Create a NB_PersistentData instance by the data of file and deserialize to a TPS
        // element.
        NB_PersistentData* persistentData = NULL;
        tpselt             cElement       = NULL;
        error = NB_PersistentDataCreate((const uint8*) data, (nb_size) dataSize,
                                        NULL, &persistentData);
        error = error ? error : NB_PersistentDataGetToTpsElement(persistentData, &cElement);

        // Destroy the data from the file and the persistent data object.
        nsl_free(data);
        data     = NULL;
        dataSize = 0;
        if (persistentData)
        {
            // Ignore the returned error.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;
        }

        // Check if the deserialized TPS element is NULL.
        if (!cElement)
        {
            error = NE_INVAL;
            break;
        }
        // Check the returned error.
        if (error != NE_OK)
        {
            if (cElement)
            {
                te_dealloc(cElement);
                cElement = NULL;
            }
            break;
        }


        // Create a TpsElement object.
        TpsElementPtr tpsElement(new TpsElement(cElement));
        te_dealloc(cElement);
        cElement = NULL;
        if (!tpsElement)
        {
            error = NE_INVAL;
            break;
        }

        shared_ptr<string> metadataSourceQuery(CCC_NEW string("metadata-source-query"));
        if (metadataSourceQuery)
        {
            m_pMetadataSourceParams = MetadataSourceParametersSerializer::deserialize(
                tpsElement->GetChild(metadataSourceQuery));
        }
        m_metadataInformation = MetadataSourceInformationSerializer::deserialize(tpsElement);

    } while (0);

    if (error == NE_OK)
    {
        m_pConfig = SourceParameterToConfiguration();
    }
    else
    {
        m_pConfig.reset(CCC_NEW MetadataConfiguration);
    }
    return error;
}

/* See description in header file. */
NB_Error
UnifiedMetadataProcessor::SetMetadataInformationToPersistentData(MetadataSourceInformationSharedPtr metadataInformation)
{
    NB_Error error = NE_OK;
    do
    {
        PAL_Instance* pal = NULL;
        if (!m_pContext || (!m_metadataPath) || m_metadataPath->empty() ||
            (pal = NB_ContextGetPal(m_pContext)) == NULL)
        {
            error = NE_NOTINIT;
            break;
        }

        static const uint32 MAX_PERSISTENT_METADATA_PATH_LENGTH = 512;
        if (m_metadataPath->size() >= MAX_PERSISTENT_METADATA_PATH_LENGTH)
        {
            error = NE_RANGE;
            break;
        }

        const char* storedPath = m_metadataPath->c_str();
        if ((!storedPath) || (nsl_strlen(storedPath) <= 0))
        {
            // The path to store metadata information has already be checked before.
            error = NE_UNEXPECTED;
            break;
        }

        // Clear the old metadata information and remove the file.
        m_metadataInformation.reset();
        if (PAL_FileExists(pal, storedPath) == PAL_Ok)
        {
            if (PAL_FileIsDirectory(pal, storedPath))
            {
                error = NE_FSYS;
                break;
            }

            if (PAL_FileRemove(pal, storedPath) != PAL_Ok)
            {
                error = NE_FSYS;
                break;
            }
        }

        if (!metadataInformation)
        {
            // There is no metadata information to store.
            error = NE_OK;
            break;
        }

        // Serialize the metadata information to a TPS element.
        TpsElementPtr tpsElement =
                MetadataSourceInformationSerializer::serialize(metadataInformation);
        if (!tpsElement)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Language is not contained in metadata-source-reply, but we need this to compare
        // whether it is the same as client passed in. So store it manually.
        if (m_pMetadataSourceParams)
        {
            tpsElement->Attach(MetadataSourceParametersSerializer::serialize(m_pMetadataSourceParams));
        }

        tpselt cElement = tpsElement->GetTPSElement();
        if (!cElement)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Create a persistent data instance by the TPS element and serialize to the data.
        NB_PersistentData* persistentData     = NULL;
        uint8*             serializedData     = NULL;
        nb_size            serializedDataSize = 0;
        NB_Error nbError = NB_PersistentDataCreateByTpsElement(cElement, &persistentData);
        nbError = nbError ? nbError : \
                  NB_PersistentDataSerialize(persistentData, (const uint8**) (&serializedData),
                                             &serializedDataSize);

        // Destroy the TPS element.
        te_dealloc(cElement);
        cElement = NULL;
        if (persistentData)
        {
            // Ignore the returned error.
            NB_PersistentDataDestroy(persistentData);
            persistentData = NULL;
        }

        // Check if there are any errors.
        if (nbError != NE_OK)
        {
            if (serializedData)
            {
                nsl_free(serializedData);
                serializedData = NULL;
            }
            serializedDataSize = 0;
            error = nbError;
            break;
        }

        if (!serializedData)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // Create the directories and open the file for writing the data.
        char pathWithoutFile[MAX_PERSISTENT_METADATA_PATH_LENGTH] = {0};
        nsl_strlcpy(pathWithoutFile, storedPath, sizeof(pathWithoutFile));
        PAL_File* file = NULL;
        PAL_Error palError = PAL_FileRemovePath(pal, pathWithoutFile);
        palError = palError ? palError : PAL_FileCreateDirectoryEx(pal, pathWithoutFile);
        palError = palError ? palError : PAL_FileOpen(pal, storedPath, PFM_Create, &file);
        if (palError != PAL_Ok)
        {
            nsl_free(serializedData);
            serializedData = NULL;
            serializedDataSize = 0;

            // Ignore the returned error.
            PAL_FileRemove(pal, storedPath);

            error = NE_FSYS;
            break;
        }

        // Write the serialized data to the file.
        uint32 bytesWritten = 0;
        palError = PAL_FileWrite(file, serializedData, (uint32) serializedDataSize,
                                 &bytesWritten);
        nsl_free(serializedData);
        serializedData = NULL;

        if ((palError != PAL_Ok) || (((uint32) serializedDataSize) != bytesWritten))
        {
            // Ignore the returned errors.
            PAL_FileClose(file);
            PAL_FileRemove(pal, storedPath);

            error = NE_FSYS;
            break;
        }

        palError = PAL_FileClose(file);
        if (palError != PAL_Ok)
        {
            // Ignore the returned error.
            PAL_FileRemove(pal, storedPath);

            error = NE_FSYS;
            break;
        }

        // Save the metadata information.
        m_metadataInformation = metadataInformation;
    } while (0);

    return error;
}


/* See description in header file. */
MetadataConfigurationPtr UnifiedMetadataProcessor::SourceParameterToConfiguration()
{
    MetadataConfigurationPtr config(CCC_NEW MetadataConfiguration);
    if (config && m_pMetadataSourceParams)
    {
        config->m_screenHeight = m_pMetadataSourceParams->GetScreenHeight();
        config->m_screenWidth  = m_pMetadataSourceParams->GetScreenWidth();
        config->m_pLanguage    = m_pMetadataSourceParams->GetLanguage();
        config->m_productClass = m_pMetadataSourceParams->GetProductClass();

        WantUnifiedMapsSharedPtr wantUnifiedMaps = m_pMetadataSourceParams->GetWantUnifiedMaps();
        if (wantUnifiedMaps)
        {
            config->m_wantLableLayers    = wantUnifiedMaps->GetWantLabelLayers() ? true : false;
            config->m_wantOptionalLayers = wantUnifiedMaps->GetWantOptionalLayers() ? true : false;
            config->m_want2DBuildings    = wantUnifiedMaps->GetWant2DBuildings() ? true : false;
            config->m_wantPoiLayers      = wantUnifiedMaps->GetWantPoiLayers() ? true : false;
            config->m_wantSatelliteLayers= wantUnifiedMaps->GetWantSatelliteLayers() ? true : false;
            config->m_wantNewCategories  = wantUnifiedMaps->GetWantNewCategories() ? true : false;
            config->m_wantTod            = wantUnifiedMaps->GetWantTod() ? true : false;
            config->m_wantThemes         = wantUnifiedMaps->GetWantThemes() ? true : false;
        }

        shared_ptr<vector<shared_ptr<WantedContent> > > array =
                m_pMetadataSourceParams->GetWantedContentArray();
        if (array && !array->empty())
        {
            UpdateWantedContent<WantedContent>(array, "weather", config->m_wantWeatherLayer);
            UpdateWantedContent<WantedContent>(array, "data availability", config->m_wantDAM);
        }
    }

    //@todo: Add more codes to convert MetadataParameter to ConfigurationPtr here.

    return config;
}

/* See description in header file. */
NB_Error UnifiedMetadataProcessor::ConfigurationToSourceParameter(MetadataConfigurationPtr config)
{
    // Caller should make sure pConfig is not NULL!
    nsl_assert(config.get() != NULL);

    MetadataSourceParametersSharedPtr params(CCC_NEW MetadataSourceParameters());
    NB_Error error   = NE_OK;
    do
    {
        if (!params)
        {
            error = NE_NOMEM;
            break;
        }

        // Initiate some un-configurable parameters.
        params->SetWantExtendedMaps(false);
        params->SetWantSharedMaps(false);

        // Screen Width/Height/Resolution.
        params->SetScreenWidth(config->m_screenWidth);
        params->SetScreenHeight(config->m_screenHeight);
        params->SetScreenResolution(config->m_screenDPI);
        params->SetProductClass(config->m_productClass);

        // Language attribute
        params->SetLanguage(config->m_pLanguage);

        // Want-Loc
        error = error ? error : UpdateWantOptions<MetadataSourceParameters, WantLoc>(
            params.get(), &MetadataSourceParameters::SetWantLoc, config->m_wantLoc);

        // Update want-unified-maps.
        shared_ptr<WantUnifiedMaps> wantUnifiedMaps(CCC_NEW WantUnifiedMaps);
        if (!wantUnifiedMaps)
        {
            error = NE_NOMEM;
            break;
        }

        // Set feature set version.
        //@todo: Move this value to MetadataConfiguration if necessary.
        wantUnifiedMaps->SetFeatureSetVersion(1);

        // Want-Lod is not configurable at present.
        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantLod>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantLod, config->m_wantLod);

        // update want-lable-layers.
        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantLabelLayers>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantLabelLayers,
            config->m_wantLableLayers);

        // update want-optional-layers.
        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantOptionalLayers>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantOptionalLayers,
            config->m_wantOptionalLayers);

        // update want-2DBuildings.
        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, Want2DBuildings>(
             wantUnifiedMaps.get(), &WantUnifiedMaps::SetWant2DBuildings,
             config->m_want2DBuildings);

        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantPoiLayers>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantPoiLayers,
            config->m_wantPoiLayers);

        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantNewCategories>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantNewCategories,
            config->m_wantNewCategories);

        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantSatelliteLayers>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantSatelliteLayers,
            config->m_wantSatelliteLayers);

        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantTod>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantTod,
            config->m_wantTod);

        error = error ? error :  UpdateWantOptions<WantUnifiedMaps, WantThemes>(
            wantUnifiedMaps.get(), &WantUnifiedMaps::SetWantThemes,
            config->m_wantThemes);

        if (error != NE_OK)
        {
            break;
        }

        // Always set it back if no error detected.
        params->SetWantUnifiedMaps(wantUnifiedMaps);

        // Update wanted-contents
        shared_ptr<vector<shared_ptr<WantedContent> > > wantedContentArray =
                params->GetWantedContentArray();
        if (!wantedContentArray)
        {
            // We don't need to prepare this array, it should be created outside.
            error = NE_UNEXPECTED;
            break;
        }

        shared_ptr<string> dataSetId(CCC_NEW string(""));
        shared_ptr<string> country(CCC_NEW string("USA"));
        if (!dataSetId || !country)
        {
            error = NE_NOMEM;
            break;
        }

        shared_ptr<WantedContent> wantedContent;
        vector<string> wantedDataTypes(WANTED_DATA_TYPES,
                                       WANTED_DATA_TYPES + (sizeof(WANTED_DATA_TYPES) / sizeof(char*)));
        if (config->m_wantWeatherLayer)
        {
            wantedDataTypes.push_back("weather");
        }

        if (config->m_wantDAM)
        {
            wantedDataTypes.push_back("data availability");
        }

        vector<string>::const_iterator iter = wantedDataTypes.begin();
        for(; iter != wantedDataTypes.end(); iter++)
        {
            wantedContent.reset(CCC_NEW WantedContent());
            shared_ptr<string> type(CCC_NEW string((*iter).c_str()));
            if (!type || !wantedContent)
            {
                error = NE_NOMEM;
                break;
            }
            wantedContent->SetCountry(country);
            wantedContent->SetDatasetId(dataSetId);
            wantedContent->SetType(type);
            wantedContentArray->push_back(wantedContent);
        }

        if (error != NE_OK)
        {
            break;
        }

        params->SetTimeStamp(m_metadataInformation ?
                              (m_metadataInformation->GetTimeStamp()) : 0);

    } while (0);

    if (error == NE_OK)
    {
        m_pMetadataSourceParams = params;
    }

    return error;
}

/*! @} */
