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

#include "internallocationproviderimpl.h"
#include "filelocationprovider.h"
#include <QMutexLocker>
#include <QDebug>

using namespace locationtoolkit;

InternalLocationProviderImpl::InternalLocationProviderImpl() :
    mInternalGpsInstance(NULL)
{
}
InternalLocationProviderImpl::~InternalLocationProviderImpl()
{
    if( mInternalGpsInstance != NULL )
    {
        delete mInternalGpsInstance;
    }
}

void InternalLocationProviderImpl::CreateInternalInstance( const LocationConfiguration& config )
{
    if( config.emulationMode == true && config.locationFilename.length() > 0 )
    {
        FileLocationProvider* pFileProvider = new FileLocationProvider( /*ltkContext& context, */&mListenerManager );
        if( pFileProvider->initialize( config ) )
        {
            mInternalGpsInstance = pFileProvider;
        }
        else
        {
            delete pFileProvider;
        }
    }
    else
    {
        //todo-create real Gps provider
    }
}

void InternalLocationProviderImpl::Initialize( const LocationConfiguration& config )
{
    if( mInternalGpsInstance == NULL )
    {
        CreateInternalInstance( /*ltkContext& context, */config );
    }
    else if( mCurrentLocationConfig != config )
    {
        if( mInternalGpsInstance != NULL )
        {
            delete mInternalGpsInstance;
            mInternalGpsInstance = NULL;
        }
        CreateInternalInstance( /*ltkContext& context, */config );
    }

    if( mInternalGpsInstance != NULL )
    {
        mCurrentLocationConfig = config;
    }
    else
    {
        qCritical( "LocationProvider::getInstance--create internal provider failed" );
    }
}

void InternalLocationProviderImpl::StopReceivingFixes(LocationListener& listener)
{
    mListenerManager.removeListener( listener );
    if( mInternalGpsInstance != NULL )
    {
        mInternalGpsInstance->stopGeneratingGpsFixes();
    }

    mState = LocationListener::LPS_Suspended;
    mListenerManager.notifyProviderStateChange( mState );
}

void InternalLocationProviderImpl::StartReceivingFixes(LocationListener& listener)
{
    mListenerManager.addListener( listener );
    if( mInternalGpsInstance != NULL )
    {
        mInternalGpsInstance->startGeneratingGpsFixes();
    }

    if( mState == LocationListener::LPS_Suspended )
    {
        mState = LocationListener::LPS_Resume;
    }
    else
    {
        mState = LocationListener::LPS_TrackingGPS;
    }
    mListenerManager.notifyProviderStateChange( mState );
}

void InternalLocationProviderImpl::GetOneFix(LocationListener& listener, Location::LocationFixType fixType)
{
    if( mInternalGpsInstance != NULL )
    {
        mInternalGpsInstance->getOneFix( listener, fixType );
    }
}

void InternalLocationProviderImpl::CancelGetLocation(LocationListener& listener )
{
    mListenerManager.removeListener( listener );
}
