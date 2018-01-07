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

#include "internallistenermanager.h"
#include <QMutexLocker>
using namespace locationtoolkit;

InternalListenerManager::InternalListenerManager() :
    mInterLock(QMutex::NonRecursive)
{
}

InternalListenerManager::~InternalListenerManager()
{}

void InternalListenerManager::addListener( LocationListener& listener )
{
    QMutexLocker locker( &mInterLock );
    mListeners.insert( &listener );
}

void InternalListenerManager::removeListener(LocationListener& listener )
{
    QMutexLocker locker( &mInterLock );
    if( mListeners.contains( &listener ) )
    {
        mListeners.remove( &listener );
    }
}

void InternalListenerManager::notifyLocationChange( const Location& location )
{
    QMutexLocker locker( &mInterLock );
    QSet<LocationListener*>::iterator listIterator;
    for (listIterator = mListeners.begin(); listIterator != mListeners.end(); ++listIterator)
    {
        (*listIterator)->LocationUpdated( location );
    }
}

void InternalListenerManager::notifyProviderStateChange( LocationListener::LocationProviderState state )
{
    QMutexLocker locker( &mInterLock );
    QSet<LocationListener*>::iterator listIterator;
    for (listIterator = mListeners.begin(); listIterator != mListeners.end(); ++listIterator)
    {
        (*listIterator)->ProviderStateChanged( state );
    }
}

void InternalListenerManager::notifyLocationError( int errorCode )
{
    QMutexLocker locker( &mInterLock );
    QSet<LocationListener*>::iterator listIterator;
    for (listIterator = mListeners.begin(); listIterator != mListeners.end(); ++listIterator)
    {
        (*listIterator)->OnLocationError( errorCode );
    }
}
