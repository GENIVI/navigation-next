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

@file locationproviderfactory.cpp
*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*!
    @addtogroup abpalgpswinmobile
    @{
*/

#include <algorithm>
#include <map>
#include "gpsconfignames.h"
#include "gpsid.h"
#include "agpsid.h"
#include "lpsgpsid.h"
#include "locationproviderfactory.h"
#include "locationprovider.h"


LocationProvider* LocationProviderFactory::Create(GpsContext* gpsContext, const ABPAL_GpsConfig* config, uint32 count)
{
    LocationProvider* provider = 0;
    std::map<std::string, std::wstring> values;

    // copy config key/value pairs
    for (uint32 i = 0; i < count; i++)
    {
        std::string name = config[i].name;
        std::string value = config[i].value;

        std::wstring wideValue;
        wideValue.resize(::mbstowcs(NULL, &value[0], value.length()));
        ::mbstowcs(&wideValue[0], &value[0], value.length());
        values[name] = wideValue;
    }

    std::wstring providerName = values[GPS_CONFIG_PROVIDER_TYPE];
    if (providerName.compare(GPS_PROVIDER_TYPE_GPSID) == 0)
    {
        provider = new GpsId(gpsContext);
    }
    else if (providerName.compare(GPS_PROVIDER_TYPE_AKU4GPSID) == 0)
    {
        provider = new AGpsId(gpsContext, values);
    }
    else if (providerName.compare(GPS_PROVIDER_TYPE_LPSGPSID) == 0)
    {
        provider = new LpsGpsId(gpsContext, values);
    }

    return provider;
}


/*! @} */
