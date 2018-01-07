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

    @file     NMEAFileReader.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
        NMEAFileReader class implementation

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

extern "C" {
  #include "paldebuglog.h"
}
#include "palmath.h"

  #include "palmath.h"
#include "nmeafilereader.h"


#define CARRIAGE_RETURN              0x0D
#define LINE_FEED                    0x0A
#define NMEA_STRING_MAX_LENGTH        255
#define NMEA_WORD_MAX_LENGTH           20
#define NMEA_DATATYPE_RMC       "$GPRMC,"
#define NMEA_DATATYPE_LENGTH            7
#define NMEA_VALID_INFORMATION        'A'

NMEAFileReader::NMEAFileReader (PAL_Instance* pal, char* emulationFilename, int emulationPlayStart)
    : m_palInstance(pal),
      m_emuFile(NULL),
      m_hour(-1),
      m_min(-1),
      m_sec(-1)
{
    nsl_memset(&m_currentLocation, 0, sizeof(m_currentLocation));

    if (emulationFilename &&
        (PAL_FileExists(m_palInstance, emulationFilename) == PAL_Ok)) // Assume, we got valid context and PAL
    {
        PAL_FileOpen(m_palInstance, emulationFilename, PFM_Read, &m_emuFile);
        FastForward(emulationPlayStart);
    }
}

NMEAFileReader::~NMEAFileReader ()
{
    if (m_emuFile)
    {
        PAL_FileClose(m_emuFile);
    }
}

bool NMEAFileReader::IsInitializedOk()
{
    if (m_emuFile && m_hour != -1)
    {
        return TRUE;
    }

    return FALSE;
}

NB_Error NMEAFileReader::ReadNextLocation (NB_LS_Location& location, int& timeoutToNext)
{
    NB_Error retStatus = NE_OK;
    uint8 buffer[NMEA_STRING_MAX_LENGTH] = {0};

    // Invalidate timeout to next location
    timeoutToNext = -1;

    if (m_hour != -1)
    {
        // At least timestamp is valid.
        // This location is to be returned to user.
        location = m_currentLocation;

        // We need to find out the timeout to next location,
        // so have to read next location in advance
        while (ReadNextLine(buffer))
        {
            int newhour, newmin, newsec;
            NB_LS_Location newLocation = {0};

            // Is there a new fix info ?
            if (GetLocationInfo(buffer, newLocation, newhour, newmin, newsec))
            {
                // Got a new fix string. Hom much time passed?
                timeoutToNext = GetTimeInterval(m_hour, m_min, m_sec,
                        newhour, newmin, newsec);
                m_hour = newhour;
                m_min = newmin;
                m_sec = newsec;
                m_currentLocation = newLocation;
                break;
            } // GetLocationInfo(buffer, hour, min, sec)
        }
        // Probably, we failed to read new location (e.g. reached the end of file).
        // In this case, timeoutToNext will just remain invalid.
    }
    else
    {
        // This means, we were unable to read any location data from emu file
        retStatus = NEGPS_GENERAL;
    }

    return retStatus;
}

bool NMEAFileReader::ReadNextLine(uint8* buffer)
{
    uint8 nextSymbol = 0x0;
    uint32 bytesRead = 1;
    int symbolCount = 0;
    PAL_Error err = PAL_Ok;

    // Not very nice (better for the caller to do it), but's let nullify it here to
    // keep the high-level function more clear. We know this buffer is of a fixed size.
    nsl_memset(buffer, 0, NMEA_STRING_MAX_LENGTH);

    while ((nextSymbol != LINE_FEED) && (symbolCount < NMEA_STRING_MAX_LENGTH) && (err == PAL_Ok) && (bytesRead == 1))
    {
        err = PAL_FileRead(m_emuFile, &nextSymbol, 1, &bytesRead);
        if ((err == PAL_Ok) && (bytesRead == 1)) // If we reach the end of file, there is still PAL_Ok
        {
            buffer[symbolCount] = nextSymbol;
            symbolCount++;
        }
        else
        {
            DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]%s", "Unable to read from emu file anymore"));
        }
    }

    if (symbolCount)
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]%s", buffer));
    }

    return ((err == PAL_Ok) && symbolCount);
}

bool NMEAFileReader::GetLocationInfo(uint8* buffer, NB_LS_Location& location, int& hour, int& min, int& sec)
{
    if (nsl_strncmp((char*)buffer, NMEA_DATATYPE_RMC, NMEA_DATATYPE_LENGTH) == 0)
    {
        return ParseRMC(buffer + NMEA_DATATYPE_LENGTH, location, hour, min, sec);
    }

    return FALSE;
}

bool NMEAFileReader::ParseRMC(uint8* buffer, NB_LS_Location& location, int& hour, int& min, int& sec)
{
    RMCTime(&buffer, hour, min, sec);

    if (hour == -1)
    {
        // We failed even to extract the time - a corrupted record
        return FALSE;
    }

    if (RMCValid(&buffer)) // Data could be invalid because of GPS reception issues,
    {                      // but if we got something with timestamp, we consider it a fix that's ok for "replay"
        if (RMCLatitude(&buffer, location.latitude))
        {
            location.valid |= NBPGV_Latitude;
        }

        if (RMCLongitude(&buffer, location.longitude))
        {
            location.valid |= NBPGV_Longitude;
        }

        if (RMCVelocity(&buffer, location.horizontalVelocity))
        {
            location.valid |= NBPGV_HorizontalVelocity;
        }

        if (RMCHeading(&buffer, location.heading))
        {
            location.valid |= NBPGV_Heading;
        }
    }

    return TRUE;
}

bool NMEAFileReader::RMCValid(uint8** buffer)
{
    char word[NMEA_WORD_MAX_LENGTH] = {0};
    int count = ReadNextWord(*buffer, word);

    (*buffer) += count;

    if (count && (word[0] == NMEA_VALID_INFORMATION))
    {
        return TRUE;
    }

    return FALSE;
}

bool NMEAFileReader::RMCLatitude(uint8** buffer, double& latitude)
{
    char word[NMEA_WORD_MAX_LENGTH] = {0};
    bool retStatus = TRUE;

    // First, read the latutude value
    int count = ReadNextWord(*buffer, word);
    (*buffer) += count;

    if (count > 1) // If we failed to find the satellites, the field could be omitted: ","
    {
        char helpstr[NMEA_WORD_MAX_LENGTH] = {0};

        // Degrees
        nsl_strncpy(helpstr, word, 2);
        latitude = (double)(nsl_atoi(helpstr));
        nsl_memset(helpstr, 0, NMEA_WORD_MAX_LENGTH);

        // Minutes
        nsl_strncpy(helpstr, word+2, 2);
        double minutes = (double)(nsl_atoi(helpstr));
        nsl_memset(helpstr, 0, NMEA_WORD_MAX_LENGTH);

        if (word[4] == '.') // There are fractions of minutes
        {
            nsl_strncpy(helpstr, word+5, count - 5 /* int number, with dec.point*/ - 1 /* comma*/);
            minutes = nsl_add(minutes, nsl_div((double)(nsl_atoi(helpstr)), nsl_pow((double)10.0, (double)(count-5-1))));
            //minutes += (double)(nsl_atoi(helpstr)) / nsl_pow((double)10.0, (double)(count-5-1));
        }

        latitude = nsl_add(latitude, nsl_div(minutes, 60.0));
        //latitude += minutes / 60.0;
    }
    else
    {
        retStatus = FALSE;
    }

    // Now the latitude sign
    nsl_memset(word, 0, NMEA_WORD_MAX_LENGTH);
    count = ReadNextWord(*buffer, word);
    (*buffer) += count;

    if (count > 1)
    {
        if (word[0] == 'S')
        {
            // Southern latitude has negative value
            latitude = -latitude;
        }
    }
    else
    {
        retStatus = FALSE;
    }

    return retStatus;
}

bool NMEAFileReader::RMCLongitude(uint8** buffer, double& longitude)
{
    char word[NMEA_WORD_MAX_LENGTH] = {0};
    bool retStatus = TRUE;

    // First, read the longitude value
    int count = ReadNextWord(*buffer, word);
    (*buffer) += count;

    if (count > 1) // If we failed to find the satellites, the field could be omitted: ","
    {
        char helpstr[NMEA_WORD_MAX_LENGTH] = {0};

        // Degrees
        nsl_strncpy(helpstr, word, 3);
        longitude = (double)(nsl_atoi(helpstr));
        nsl_memset(helpstr, 0, NMEA_WORD_MAX_LENGTH);

        // Minutes
        nsl_strncpy(helpstr, word+3, 2);
        double minutes = (double)(nsl_atoi(helpstr));
        nsl_memset(helpstr, 0, NMEA_WORD_MAX_LENGTH);

        if (word[5] == '.') // There are fractions of minutes
        {
            nsl_strncpy(helpstr, word+6, count - 6 /* int number, with dec.point*/ - 1 /* comma*/);
            minutes = nsl_add(minutes, nsl_div((double)(nsl_atoi(helpstr)), nsl_pow((double)10.0, (double)(count-6-1))));
            //minutes += (double)(nsl_atoi(helpstr)) / nsl_pow((double)10.0, (double)(count-6-1));
        }
        longitude = nsl_add(longitude, nsl_div(minutes, 60.0));
        //longitude += minutes / 60.0;
    }
    else
    {
        retStatus = FALSE;
    }

    // Now the longitude sign
    nsl_memset(word, 0, NMEA_WORD_MAX_LENGTH);
    count = ReadNextWord(*buffer, word);
    (*buffer) += count;

    if (count > 1)
    {
        if (word[0] == 'W')
        {
            // Western longitude has negative value
            longitude = -longitude;
        }
    }
    else
    {
        retStatus = FALSE;
    }

    return retStatus;
}

bool NMEAFileReader::RMCVelocity(uint8** buffer, double& velocity)
{
    char word[NMEA_WORD_MAX_LENGTH] = {0};
    bool retStatus = TRUE;

    // First, read the velocity value
    int count = ReadNextWord(*buffer, word);
    (*buffer) += count;

    if (count > 1) // The field could be omitted: ","
    {
        char helpstr[NMEA_WORD_MAX_LENGTH] = {0};
        char* ptrDecimalPoint = nsl_strchr(word, '.');
        char* ptrComma = nsl_strchr(word, ','); // Assume, track is not corrupted, and we have this comma

        // Whole part
        if ((ptrDecimalPoint != NULL) && (ptrDecimalPoint == &(word[0]))) // Number like ".1234"
        {
            velocity = 0.0;
        }
        else if (ptrDecimalPoint != NULL) // "Usual" number like "12.34"
        {
            nsl_strncpy(helpstr, word, ptrDecimalPoint-word);
            velocity = (double)nsl_atoi(helpstr);
        }
        else // ptrDecimalPoint == NULL, so just a whole number
        {
            nsl_strncpy(helpstr, word, count-1); // Comma is the last
            velocity = (double)nsl_atoi(helpstr);
        }
        nsl_memset(helpstr, 0, NMEA_WORD_MAX_LENGTH);

        // Fractional part
        if (ptrDecimalPoint != NULL)
        {
            nsl_strncpy(helpstr, ptrDecimalPoint+1, ptrComma-ptrDecimalPoint-1);
            velocity = nsl_add(velocity, nsl_div((double)nsl_atoi(helpstr), nsl_pow((double)10.0, (double)(ptrComma-ptrDecimalPoint-1))));
            //velocity += (double)nsl_atoi(helpstr) / nsl_pow((double)10.0, (double)(ptrComma-ptrDecimalPoint-1));
        }

        // The velocity in the RMC is in knots.
        velocity = KnotsToMetersPerSecond(velocity);
    }
    else
    {
        retStatus = FALSE;
    }

    return retStatus;
}

bool NMEAFileReader::RMCHeading(uint8** buffer, double& heading)
{
    char word[NMEA_WORD_MAX_LENGTH] = {0};
    bool retStatus = TRUE;

    // First, read the heading value
    int count = ReadNextWord(*buffer, word);
    (*buffer) += count;

    if (count > 1) // The field could be omitted: ","
    {
        char helpstr[NMEA_WORD_MAX_LENGTH] = {0};
        char* ptrDecimalPoint = nsl_strchr(word, '.');
        char* ptrComma = nsl_strchr(word, ','); // Assume, track is not corrupted, and we have this comma

        // Whole part
        if ((ptrDecimalPoint != NULL) && (ptrDecimalPoint == &(word[0]))) // Number like ".1234"
        {
            heading = 0.0;
        }
        else if (ptrDecimalPoint != NULL) // "Usual" number like "12.34"
        {
            nsl_strncpy(helpstr, word, ptrDecimalPoint-word);
            heading = (double)nsl_atoi(helpstr);
        }
        else // ptrDecimalPoint == NULL, so just a whole number
        {
            nsl_strncpy(helpstr, word, count-1); // Comma is the last
            heading = (double)nsl_atoi(helpstr);
        }
        nsl_memset(helpstr, 0, NMEA_WORD_MAX_LENGTH);

        // Fractional part
        if (ptrDecimalPoint != NULL)
        {
            nsl_strncpy(helpstr, ptrDecimalPoint+1, ptrComma-ptrDecimalPoint-1);
            heading = nsl_add(heading, nsl_div((double)nsl_atoi(helpstr), nsl_pow((double)10.0, (double)(ptrComma-ptrDecimalPoint-1))));
            //heading += (double)nsl_atoi(helpstr) / nsl_pow((double)10.0, (double)(ptrComma-ptrDecimalPoint-1));
        }
    }
    else
    {
        retStatus = FALSE;
    }

    return retStatus;
}

bool NMEAFileReader::RMCTime(uint8** buffer, int& hour, int& min, int& sec)
{
    bool retStatus = FALSE;
    hour = 0;
    min = 0;
    sec = 0;

    char word[NMEA_WORD_MAX_LENGTH] = {0};
    int count = ReadNextWord(*buffer, word);

    if (count >= 7) // Word for time should be, at least, "HHMMSS,"
    {               // otherwise, we got something corrupted
        char helpstr[NMEA_WORD_MAX_LENGTH] = {0};

        nsl_strncpy(helpstr, word, 2);
        hour = nsl_atoi(helpstr);
        nsl_strncpy(helpstr, word+2, 2);
        min = nsl_atoi(helpstr);
        nsl_strncpy(helpstr, word+4, 2);
        sec = nsl_atoi(helpstr);
        if (word[6] == '.') // there are fractions of seconds
        {
            // Round the seconds. No need to calculate exact fractional part of a second for the emulation.
            nsl_strncpy(helpstr, word+7, 1);
            sec = nsl_atoi(helpstr) >= 5 ? sec+1 : sec;
        }

        retStatus = TRUE;
    }

    (*buffer) += count;

    return retStatus;
}

int NMEAFileReader::ReadNextWord(uint8* buffer, char* word)
{
    // By "word", we consider something finished either by "," or by CR.
    int symbolCount = 0;

    // Read until we met a comma or CR.
    while ((*buffer != ',') && (*buffer != CARRIAGE_RETURN) && (symbolCount < NMEA_WORD_MAX_LENGTH))
    {
        word[symbolCount] = *buffer++;
        symbolCount++;
    }

    if (*buffer == ',')
    {
        word[symbolCount] = *buffer;
        symbolCount++;
    }

    // Return the word length, including the comma.
    // This will allow the parser to advance to next word properly.
    return symbolCount;
}

void NMEAFileReader::FastForward(int emulationPlayStart)
{
    uint8 buffer[NMEA_STRING_MAX_LENGTH] = {0};

    while (ReadNextLine(buffer))
    {
        int newhour, newmin, newsec;
        NB_LS_Location newLocation = {0};

        // Is there a new fix info ?
        if (GetLocationInfo(buffer, newLocation, newhour, newmin, newsec))
        {
            // Got a new fix string. Hom much time passed?
            if (m_hour != -1)
            {
                emulationPlayStart -= GetTimeInterval(m_hour, m_min, m_sec,
                    newhour, newmin, newsec);
            }

            m_hour = newhour;
            m_min = newmin;
            m_sec = newsec;
            m_currentLocation = newLocation;

            if (emulationPlayStart <= 0)
            {
                break;
            }
        } // GetLocationInfo(buffer, hour, min, sec)
    }
}

int NMEAFileReader::GetTimeInterval(int hour, int min, int sec, int newhour, int newmin, int newsec)
{
    int seconds1 = ToSeconds (hour, min, sec);
    int seconds2 = ToSeconds (newhour, newmin, newsec);
    int interval = 0;

    // We assume, (newhour, newmin, newsec) is always later than (hour, min, sec), as soon as track file is not corrupted;
    // we only need to handle the situations like (23:12:15) - (02:25:12)
    if (newhour >= hour)
    {
        // Two timestamps on the same day
        interval = seconds2 - seconds1;
    }
    else
    {
        interval = (ToSeconds(23,59,59) - seconds1 + 1) // Time remained to 0:00:00
            + seconds2;
    }

    return interval;
}

int NMEAFileReader::ToSeconds(int hour, int min, int sec)
{
    return (hour*3600 + min*60 + sec);
}

double NMEAFileReader::KnotsToMetersPerSecond(double knots)
{
    return nsl_mul(knots, 0.5144444);
    //return knots*0.5144444;
}

/*! @} */
