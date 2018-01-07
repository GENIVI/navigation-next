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

 @file ltkcontextimpl.cpp
 @date 08/04/2014
 @addtogroup common

 */
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
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

#include "ltkcontextimpl.h"
#include "buildconfig.h"
#include "palconfig.h"
#include "nbcontextaccess.h"
#include <string>
#include "palevent.h"
#include "paldisplay.h"
#include "paltaskqueue.h"
#include "tpslib.h"
#include "qatpslib.h"
#include "QaLog.h"
#include "qaloglistenerimpl.h"
#include <QSettings>
#include <QList>
#include <QDebug>
#include "version.h"
#include "nbcversion.h"

using namespace locationtoolkit;

const static QString gSettingsOrganizationName = "Telesyscom";
const static QString gSettingsApplicationName = "ltk-qt";
const static QString gSettingsGUIDKey = "guid";

LTKContext* LTKContext::CreateInstance( const QString& credential, float dpi)
{
    return new LTKContextImpl(credential,
                               DEFAULT_COUNTRY,
                               DEFAULT_LANGUAGE,
                               DEFAULT_MDN,
                               DEFAULT_PRODUCT_NAME,
                               DEFAULT_PUSH_MESSAGE_GUID,
                               dpi);
}

LTKContext* LTKContext::CreateInstance( const QString& credential, const QString& language, float dpi)
{
    return new LTKContextImpl(credential,
                               DEFAULT_COUNTRY,
                               language,
                               DEFAULT_MDN,
                               DEFAULT_PRODUCT_NAME,
                               DEFAULT_PUSH_MESSAGE_GUID,
                               dpi);
}

LTKContext* LTKContext::CreateInstance( const QString& credential,
            const QString& language,
            const QString& countryCode,
            float dpi)
{
    return new LTKContextImpl(credential,
                               countryCode,
                               language,
                               DEFAULT_MDN,
                               DEFAULT_PRODUCT_NAME,
                               DEFAULT_PUSH_MESSAGE_GUID,
                               dpi);
}

LTKContext* LTKContext::CreateInstance( const QString& credential,
            const QString& countryCode,
            const QString& language,
            qint64 mdn,
            const QString& productName,
            const QString& pushMessageGUID,
            float dpi)
{
    return new LTKContextImpl(credential,
                               countryCode,
                               language,
                               mdn,
                               productName,
                               pushMessageGUID,
                               dpi);
}

class MasterClearParameters
{
public:
    MasterClearParameters(NB_Context* context, PAL_Event* event)
    :mContext(context), mEvent(event) {}
    virtual ~MasterClearParameters() {}

    NB_Context*  mContext;
    PAL_Event*   mEvent;
};

static void CCCMasterClear(PAL_Instance* pal, void* userData)
{
    MasterClearParameters* param = static_cast<MasterClearParameters*>(userData);
    if (param)
    {
        NB_ContextSendMasterClear(param->mContext);
        PAL_Event* ev = param->mEvent;
        delete param;
        PAL_EventSet(ev);
    }
}

static void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    if (event == NB_NetworkNotifyEvent_ClientGuidAssigned)
    {
        NB_NetworkNotifyEventClientGuid* guiddata = (NB_NetworkNotifyEventClientGuid*) data;
        if (guiddata)
        {
            QByteArray value;
            value.setRawData( (const char *)guiddata->guidData, guiddata->guidDataLength );
            QSettings settings( gSettingsOrganizationName, gSettingsApplicationName );
            settings.setValue( gSettingsGUIDKey, value );
        }
    }
}

static NB_NetworkOptionsDetailed CreateNetworkOptions(const QString credential,
                                                      const QString countryCode,
                                                      const QString language,
                                                      const quint64 mdn,
                                                      const QString pushMessageGUID,
                                                      QList< shared_ptr<std::string> >& stringBuffer,
                                                      QByteArray& guidBytes )
{
	NB_NetworkOptionsDetailed options = {0};
	    char* guidData = NULL;
    size_t guidDataSize = 0;

    QSettings settings( gSettingsOrganizationName, gSettingsApplicationName );
    QVariant var = settings.value( gSettingsGUIDKey );
    if( !var.isNull() )
    {
        guidBytes = var.toByteArray();
        guidData = guidBytes.data();
        guidDataSize = guidBytes.size();
    }

    QString serverToken, domain;
    int index = credential.indexOf('.');

    if(index == -1)
    {
        serverToken = credential;
        domain = DEFAULT_DOMAIN;
    }
    else
    {
        serverToken = credential.left(index);
        domain = credential.mid(index+1);
    }

    shared_ptr<std::string> stdCredential( new std::string( serverToken.toStdString() ) );
    shared_ptr<std::string> stdLanguage( new std::string( language.toStdString() ) );
    shared_ptr<std::string> stdDomain( new std::string( domain.toStdString() ) );
    options.credential         = stdCredential->c_str();
    options.hostname           = DEFAULT_HOST_NAME;
    options.device             = DEFAULT_DEVICE;
    options.domain             = stdDomain->c_str();
    options.language           = stdLanguage->c_str();
    stringBuffer.push_back( stdCredential );
    stringBuffer.push_back( stdLanguage );
    stringBuffer.push_back(stdDomain);

    options.protocol           = NB_NetworkProtocol_TCP;
    options.port               = DEFAULT_PORT_TCP;
    options.tpslibData         = (const char*)tpslib;
    options.tpslibLength       = sizeof(tpslib);
    options.clientGuidData     = guidData;
    options.clientGuidLength   = guidDataSize;

    options.mdn                = mdn;
    options.pin                = "80b053fccead422934b9a3140b72983c450f8832";
    options.adsAppId           = "N/A";
    options.classId            = 0;
    options.firmwareVersion    = "N/A";
    options.min                = 0;
    options.platformId         = 0;
    options.priceType          = 0;
    options.isEmulator         = false;
    options.mcc                = 0;
    options.mnc                = 0;
    shared_ptr<std::string> stdCountryCode( new std::string( countryCode.toStdString() ) );
    options.countryCode        = stdCountryCode->c_str();
    options.manufacturer       = "N/A";

    stringBuffer.push_back( stdCountryCode );
    if( pushMessageGUID.length() > 0 )
    {
        shared_ptr<std::string> stdMessageGuid( new std::string( pushMessageGUID.toStdString() ) );
        options.pushMessageGuid = stdMessageGuid->c_str();
        options.pushMessageGuidLength = stdMessageGuid->length();
        stringBuffer.push_back( stdMessageGuid );
    }
    else
    {
        options.pushMessageGuid = NULL;
        options.pushMessageGuidLength = 0;
    }

	return options;
}

LTKContextImpl::LTKContextImpl(const QString credential,
							const QString countryCode,
							const QString language,
							const quint64 mdn,
							const QString productName,
							const QString pushMessageGUID,
							float dpi)
    : mPALInstance(NULL)
    , mContext(NULL)
    , mQalogListener(NULL)
    , mQaLogDelegate(NULL)
    , mVersion( VERSION )
    , mLanguage(language)
{
    /* there is only one context in CCC */
    InitializeNBContext( credential, countryCode, language, mdn, productName, pushMessageGUID, dpi );
}

LTKContextImpl::~LTKContextImpl()
{
    if(mContext != NULL)
    {
        if( mQalogListener != NULL )
        {
            QaLog::GetInstance(mContext)->ReleaseListener( mQalogListener );
            delete mQalogListener;
        }

        NB_GpsHistory *history = NB_ContextGetGpsHistory(mContext);
        if (history)
        {
            NB_GpsHistoryDestroy(history);
        }
        NB_ContextDestroy(mContext);
    }

    if(mPALInstance != NULL)
    {
        PAL_Destroy(mPALInstance);
    }

    if( mQaLogDelegate != NULL )
    {
        delete mQaLogDelegate;
    }
}

PAL_Instance *LTKContextImpl::GetPalInstance() const
{
    return mPALInstance;
}

bool LTKContextImpl::InitializeNBContext(const QString credential,
							const QString countryCode,
							const QString language,
							const quint64 mdn,
							const QString productName,
							const QString pushMessageGUID,
                            float dpi)
{
    PAL_Config palConfig = { 0 };
    /*
     :KLUDGE:
     Set the multi-threaded flag to TRUE! This should be taken from a platform-specific configuration!
     */
    palConfig.multiThreaded = TRUE;
	palConfig.workpath = "./";

    palConfig.dpi = dpi;
    mPALInstance = PAL_Create(&palConfig);
    if (mPALInstance == NULL)
    {
        return false;
    }

    PAL_Error palResult = PAL_NetInitialize(mPALInstance);
    if (palResult != PAL_Ok)
    {
        return false;
    }

    QList< shared_ptr<std::string> > stdStringBuffer;
    QByteArray guidBytes;
    NB_NetworkOptionsDetailed options = CreateNetworkOptions( credential,
                                                              countryCode,
                                                              language,
                                                              mdn,
                                                              pushMessageGUID,
                                                              stdStringBuffer,
                                                              guidBytes );


    bool bReturn = CreateContext(options);
    if( bReturn)
    {
        // create qalog
        NB_NetworkOptionsDetailed qaLogOptionDetailed = options;
        qaLogOptionDetailed.credential = "EPVSQrhmn2zvyMvfAH1YChO3mi2RcQB2drzC/KOG";
        qaLogOptionDetailed.domain = "nimupdate.com";
        qaLogOptionDetailed.hostname = "qalog";
        qaLogOptionDetailed.pin = "1f9a1570e522a2e2a42ee0771fd2ac3541c795a9";
        qaLogOptionDetailed.tpslibData = (const char *) qatpslib;
        qaLogOptionDetailed.tpslibLength = sizeof(qatpslib);

        NB_NetworkConfiguration* qalogNetworkConfiguration = NULL;
        NB_Error err = NB_NetworkConfigurationCreate(&qaLogOptionDetailed, &qalogNetworkConfiguration);

        if(err == NE_OK)
        {
            QaLog::GetInstance(mContext)->Initialize(qalogNetworkConfiguration, mPALInstance);
            //temp code, qalog configuration should be generated by build.
            AB_QaLogHeader qalogHeader = { 0 };
            qalogHeader.productName = "";
            qalogHeader.productVersion.major = 0;
            qalogHeader.productVersion.minor = 0;
            qalogHeader.productVersion.feature = 0;
            qalogHeader.productVersion.build = 0;
            qalogHeader.platformId = 0;
            qalogHeader.mobileDirectoryNumber = mdn;
            qalogHeader.mobileInformationNumber = 0;

            QaLog::GetInstance(mContext)->SetQaLogConfiguration( &qalogHeader, "QaLog" );
            QaLogListenerImpl* logListenerImpl = new QaLogListenerImpl();
            QaLog::GetInstance(mContext)->RegisterListener( logListenerImpl );
            mQalogListener = logListenerImpl;
            mQaLogDelegate = new QaLogListenerDelegate( NULL, logListenerImpl );
        }
        else
        {
            mQalogListener = NULL;
        }
    }
    return bReturn;
}

NB_Context* LTKContextImpl::GetNBContext() const
{
    return mContext;
}

void LTKContextImpl::MasterClear()
{
    ClearCache();
    ClearLocalGUID();
}

bool LTKContextImpl::CreateContext(NB_NetworkOptionsDetailed& networkOptions)
{
    NB_NetworkConfiguration* networkConfiguration = GetNetworkConfiguration(networkOptions);
    if(!networkConfiguration)
    {
        return false;
    }

    NB_NetworkNotifyEventCallback networkCallback = { NetworkNotifyEventCallback, this };

    NB_CacheConfiguration voiceCacheConfig = { 0 };
    voiceCacheConfig.cacheName = (char*)"voiceCache";
    voiceCacheConfig.maximumItemsInMemoryCache = 64;
    voiceCacheConfig.maximumItemsInPersistentCache = 64;
    voiceCacheConfig.obfuscateKey = (byte*) VOICE_CACHE_OBFUSCATE_KEY;
    voiceCacheConfig.obfuscateKeySize = sizeof(VOICE_CACHE_OBFUSCATE_KEY);
    voiceCacheConfig.configuration = NULL;

    // Voice cache settings
    NB_CacheConfiguration rasterTileCacheConfig = { 0 };
    rasterTileCacheConfig.cacheName = (char*)"mapCache";
    rasterTileCacheConfig.maximumItemsInMemoryCache = 1000;
    rasterTileCacheConfig.maximumItemsInPersistentCache = 2000;
    rasterTileCacheConfig.obfuscateKey = (byte*) RASTER_TILE_CACHE_OBFUSCATE_KEY;
    rasterTileCacheConfig.obfuscateKeySize = sizeof(RASTER_TILE_CACHE_OBFUSCATE_KEY);
    rasterTileCacheConfig.configuration = NULL;

    NB_PersistentData* persistentData = NULL;
    NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);

    NB_ContextCreate(mPALInstance,
                     networkConfiguration,
                     &networkCallback,
                     &voiceCacheConfig,
                     &rasterTileCacheConfig,
                     persistentData,
                     &mContext);

    if(NULL == mContext)
    {
        return false;
    }

    NB_TargetMapping targets[] =
    {
        { "geocode",            "geocode,global" },
        { "map",                "map,global" },
        { "nav",                "nav,global" },
        { "proxpoi",            "proxpoi,global" },
        { "reverse-geocode",    "reverse-geocode,global" },
        { "traffic-notify",     "traffic-notify,global" },
        { "vector-tile",        "vector-tile,global" },
        { "coupon",             "coupons" },
        { "metadata-source",    "metadata" },
        { "metadata-manifest",  "metadata" },
        { "traffic-notify",     "traffic-notify,global" }
    };

    NB_ContextSetTargetMappings(mContext, targets, sizeof(targets) / sizeof(targets[0]));

    if (NB_ContextInitializeGenericHttpDownloadManager(mContext, 10) != NE_OK)
    {
        return false;
    }

    NB_GpsHistory* history = NULL;

    NB_GpsHistoryCreate(mContext, 100 /* Max fixes */,
        5.5 /* m/s to consider heading valid */, &history);
    if(history)
    {
        NB_ContextSetGpsHistoryNoOwnershipTransfer(mContext, history);
    }
        
    return true;
}

NB_NetworkConfiguration* LTKContextImpl::GetNetworkConfiguration(NB_NetworkOptionsDetailed& options)
{
    NB_NetworkConfiguration* configuration = NULL;

    NB_NetworkConfigurationCreate(&options, &configuration);
    return configuration;
}

void LTKContextImpl::ClearCache()
{
    PAL_Event* ev = NULL;
    if (PAL_EventCreate(mPALInstance, &ev) == PAL_Ok)
    {
        // Post task to CCC thread
        uint32 taskId = 0;
        PAL_Error error = PAL_EventTaskQueueAdd(mPALInstance,
                                                CCCMasterClear,
                                                new MasterClearParameters(mContext, ev),
                                                &taskId);
        if (error == PAL_Ok)
        {
            // Wait for the event to finish master clear.
            PAL_EventWaitForEvent(ev);
        }
        PAL_EventDestroy(ev);
    }
}

void LTKContextImpl::ClearLocalGUID()
{
    QSettings settings( gSettingsOrganizationName, gSettingsApplicationName );
    settings.clear();
}

void LTKContextImpl::StartQaLog(bool verbose)
{
    QaLog::GetInstance( mContext )->StartLogging( verbose );
}

void LTKContextImpl::UploadQaLog()
{
    QaLog::GetInstance( mContext )->UploadLogFile();
}

QaLogListenerDelegate *LTKContextImpl::GetQaLogDelegate()
{
    return mQaLogDelegate;
}

QString LTKContext::GetVersion()
{
    return VERSION;
}

const QString& LTKContextImpl::GetLanguage() const
{
    return mLanguage;
}

static QString toString(const nb_version& nbv)
{
    QString ver = "";
    ver += QString::number(nbv.major);
    ver += ".";
    ver += QString::number(nbv.minor);
    ver += ".";
    ver += QString::number(nbv.feature);
    ver += ".";
    ver += QString::number(nbv.build);

    return ver;
}

QString LTKContext::GetCCCVersion()
{
    nb_version nbv;
    NBC_VersionGet(&nbv);
    return toString(nbv);
}

QString LTKContext::GetNCDBVersion()
{
    nb_version nbv;
    NBC_NCDBVersionGet(&nbv);
    return toString(nbv);
}
