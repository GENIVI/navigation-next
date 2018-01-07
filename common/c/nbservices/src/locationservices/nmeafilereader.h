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

    @file     nmeafilereader.h
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        NMEAFileReader class declaration
        This class reads the locations from a text file in NMEA format.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#pragma once

#include "emufilereader.h"

extern "C" {
  #include "palfile.h"
}

class NMEAFileReader : public EmuFileReader
{
public:

    NMEAFileReader (PAL_Instance* pal, char* emulationFilename, int emulationPlayStart=0);
    virtual ~NMEAFileReader ();
    virtual bool IsInitializedOk();
    virtual NB_Error ReadNextLocation (NB_LS_Location& location, int& timeoutToNext);

private:
    bool ReadNextLine(uint8* buffer);
    bool GetLocationInfo(uint8* buffer, NB_LS_Location& location, int& hour, int& min, int& sec);
    void FastForward(int emulationPlayStart=0);
    static int GetTimeInterval(int hour, int min, int sec, int newhour, int newmin, int newsec);
    static int ToSeconds(int hour, int min, int sec);
    static double KnotsToMetersPerSecond(double knots);

    // Parsing of $GPRMC message
    bool ParseRMC(uint8* buffer, NB_LS_Location& location, int& hour, int& min, int& sec);
    bool RMCValid(uint8** buffer);
    bool RMCTime(uint8** buffer, int& hour, int& min, int& sec);
    bool RMCLatitude(uint8** buffer, double& latitude);
    bool RMCLongitude(uint8** buffer, double& longitude);
    bool RMCVelocity(uint8** buffer, double& velocity);
    bool RMCHeading(uint8** buffer, double& heading);
    int ReadNextWord(uint8* buffer, char* word);

    PAL_Instance* m_palInstance;
    PAL_File* m_emuFile;

    NB_LS_Location m_currentLocation;
    int m_hour;
    int m_min;
    int m_sec;

};

/*! @} */
