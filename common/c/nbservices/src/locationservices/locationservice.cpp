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
 
 @file     locationservice.cpp
 @date     03/15/2012
 @defgroup LOCATIONSERVICES_GROUP
 
 */
/*
 LocationService class implementation
 
 (C) Copyright 2012 by TeleCommunication Systems, Inc.
 
 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "locationservice.h"
#include "gpslocationkit.h"

LocationServiceInterface*
LocationServiceInterface::CreateLocationService(PAL_Instance* pal, NB_LocationKitType kitType)
{
    LocationService* service = new LocationService();
    service->InitializeLocationKit(pal, kitType);
    return (LocationServiceInterface*)service;
}

LocationService::LocationService():m_locationKit(NULL)
{
}

LocationService::~LocationService()
{
}

NB_Error 
LocationService::InitializeLocationKit(PAL_Instance* pal, NB_LocationKitType kitType)
{
    NB_Error err = NE_OK;
    switch (kitType) {
        case NB_LKT_Gps:
        {
            GpsLocationKit* kit = new GpsLocationKit();
            kit->Initialize(pal);
            m_locationKit = (LocationKitInterface*)kit;
            break;
        }   
        default:
            err = NE_NOSUPPORT;
            break;
    }
    return err;
}

void
LocationService::Release()
{
    delete this;
}

NB_Error
LocationService::StartNavigationTracking(ABPAL_GpsConfig *config, uint32 configSize, LocationServiceListenerInterface *listener, NB_GpsCriteria criteria)
{
    NB_Error error = m_locationKit->StartNavigationTracking(config, configSize, criteria, listener);
    return error;
}

NB_Error
LocationService::StopTracking(LocationServiceListenerInterface *listener)
{
    NB_Error error = m_locationKit->StopTracking(listener);
    return error;
}

NB_Error
LocationService::StartNonNavigationTracking(ABPAL_GpsConfig* config, uint32 configSize, LocationServiceListenerInterface *listener, NB_GpsCriteria criteria)
{
    NB_Error error = m_locationKit->StartNonNavigationTracking(config, configSize, criteria, listener);
    return error;
}

NB_Error
LocationService::NarrowToAccurate(ABPAL_GpsConfig* config, uint32 configSize, LocationServiceListenerInterface *listener, NB_GpsCriteria criteria)
{
    NB_Error error = m_locationKit->NarrowToAccurate(config, configSize, criteria, listener);
    return error;
}

NB_GpsLocation&
LocationService::GetLastLocation()
{
    return m_locationKit->GetLastLocation();
}

void
LocationService::SetLastLocation (NB_GpsLocation location)
{
    m_locationKit->SetLastLocation(location);
}

NB_Error
LocationService::UpdateCriteria(LocationServiceListenerInterface* listener, NB_GpsCriteria criteria)
{
    return m_locationKit->UpdateCriteria(listener, criteria);
}

NB_TrackingType
LocationService::CurrentTrackingType(LocationServiceListenerInterface* listener)
{
    return m_locationKit->CurrentTrackingType(listener);
}

/*! @} */
