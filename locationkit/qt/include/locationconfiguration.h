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

    @file     locationconfiguration.h
    @date     07/29/2014
    @addtogroup locationkit

    Location configuration definition
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

/*! @{ */

#ifndef LOCATIONCONFIG_H
#define LOCATIONCONFIG_H
#include <QString>

namespace locationtoolkit
{
struct LocationConfiguration
{
    LocationConfiguration() :
        emulationMode( false ),
        locationFilename( "" ),
        roaming( false ),
        emuPlayStart( 0 ),
        warmUpFix( true ),
        collectWiFiProbes( false ),
        useOwnNetworkLocation( true ),
        allowMockLocation( false ),
        loopMode( true )
    {}
    ~LocationConfiguration() {}

    bool operator == ( const LocationConfiguration& obj )
    {
        if( emulationMode != obj.emulationMode )
        {
            return false;
        }
        if( locationFilename != obj.locationFilename )
        {
            return false;
        }
        if( roaming != obj.roaming )
        {
            return false;
        }
        if( emuPlayStart != obj.emuPlayStart )
        {
            return false;
        }
        if( warmUpFix != obj.warmUpFix )
        {
            return false;
        }
        if( collectWiFiProbes != obj.collectWiFiProbes )
        {
            return false;
        }
        if( useOwnNetworkLocation != obj.useOwnNetworkLocation )
        {
            return false;
        }
        if( allowMockLocation != obj.allowMockLocation )
        {
            return false;
        }
        if( loopMode != obj.loopMode )
        {
            return false;
        }
        return true;
    }

    bool operator != ( const LocationConfiguration& obj )
    {
        return !( *this == obj );
    }

    bool    emulationMode;	       /*!< If true, do emulation*/
    QString locationFilename;      /*!< Path to the emulation file that contains location fixes*/
    bool    roaming;
    qint32  emuPlayStart;          /*!< Number of seconds to skip the playback file*/
    bool    warmUpFix;             /*!< If true, LK will request a fix during initialization to warm up the Hardware*/
    bool    collectWiFiProbes;	   /*!< If true, then allow collection of WiFi probes*/
    bool    useOwnNetworkLocation; /*!< If true use its own networlocation request. False use shared networklocation requests*/
    bool    allowMockLocation;     /*!< if false, does not return GPS location when mock location setting is on. true returns gps location regardless of mock location setting*/
    bool    loopMode;              /*!< If false, all fixes in the emulation file will not be returned as a loop. true by default*/
};
}

#endif // LOCATIONCONFIG_H

/*! @} */
