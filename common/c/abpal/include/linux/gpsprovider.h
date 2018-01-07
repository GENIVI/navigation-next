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

    @file     gpsprovider.h
    @date     03/27/2012
    @defgroup GPSPROVIDER_H Gps Provider

    Gps Provider
*/
/*
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

#ifndef GPSPROVIDER_H
#define GPSPROVIDER_H

#include "abpalgps.h"
#include "locationlistener.h"
#include "pallock.h"
#include <list>

class GpsProvider
{
    public:
        virtual PAL_Error Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 configCount) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void updateCriteria(const ABPAL_GpsCriteria* criteria);
        virtual nb_boolean IsEnable();
        virtual PAL_Error enableBackgroundGeolocation(nb_boolean isEnabled);

        void AddListner(LocationListner* listner);
        void RemoveListner(LocationListner* listner);

        virtual void RemoveInstance();

    protected:
        GpsProvider(PAL_Instance* pal);
        virtual ~GpsProvider();

        virtual void NotifyListeners(PAL_Error error, const ABPAL_GpsLocation& location, bool isGpsFix);

        nb_boolean m_bEnabled;
        ABPAL_GpsCriteria m_criteria;

        PAL_Lock* m_pLock;
        std::list<LocationListner*> m_pList;
};

#endif

/*! @} */
