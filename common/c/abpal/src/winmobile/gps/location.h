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

@file location.h
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

#pragma once

#include "windows.h"
#include <string>
#include "paltypes.h"
#include "abpalgps.h"

/*! Location contains normalized location information */
class Location : public ABPAL_GpsLocation
{
public:   

    Location();

    /*! Resets the Location to an empty state */
    void                    Reset();

    /*! Validates the current fix */
    void                    Validate();
    
    /*! Returns whether or not the fix is considered valid */
    bool                    IsValid() const { return m_isValid; }

    /*! If available, gets more detailed error information regarding this location fix */
    std::string             GetErrorInfoXml() const { return m_errorInfoXml; }

    bool                    IsSameFix(const Location& other) const;

    // conversion constants (see source file for descriptions)
    static const double     LATITUDE_CONSTANT;
    static const double     LONGITUDE_CONSTANT;
    static const double     HEADING_CONSTANT;
    static const double     SPEED_CONSTANT;
    static const int        GPS_TO_UNIX_OFFSET;

    static nb_gpsTime       SystemTimeToGpsTime(const SYSTEMTIME* systemTime);

    void                    SetFixTime(nb_gpsTime fixTime) { m_fixTime = fixTime; }
    nb_gpsTime              FixTime() const                { return m_fixTime;    }

private:

    bool                    m_isValid;
    std::string             m_errorInfoXml;
    nb_gpsTime              m_fixTime;          /*<! System GPS time a fix was fetched, seconds */
};

/*! @} */
