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

@file     utility.c
@defgroup Utility file to include common code

Function declarations for general utility functions used for system test.
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


#include "utility.h"
#include "palerror.h"
#include "palfile.h"
#include "palstdlib.h"
#include "platformutil.h"

// Seperator for GPS fix file.
const char SEPERATOR = ',';

uint64 GetMobileDirectoryNumberFromFile(PAL_Instance* pal)
{
    uint64 mdn = DEFAULT_MOBILE_DIRECTORY_NUMBER;
    unsigned char* mdnBuffer = NULL;
    uint32 mdnBufferLength = 0;
    PAL_Error err = PAL_Ok;

    err = PlatformLoadFile(pal, "mdn.txt", &mdnBuffer, &mdnBufferLength);
    if (!err)
    {
        uint64 tempMdn = 0;
        char tempBuffer[20] = { 0 };
        uint32 tempLen = sizeof(tempBuffer) - 1;

        if (tempLen > mdnBufferLength)
        {
            tempLen = mdnBufferLength;
        }
        nsl_memcpy(tempBuffer, mdnBuffer, tempLen);

        tempMdn = nsl_strtouint64(tempBuffer);
        if (tempMdn != 0)
        {
            mdn = tempMdn;
        }

        nsl_free(mdnBuffer);
    }

    return mdn;
}


nb_version GetApplicationVersion()
{
    /// @todo Add version resource to application
    nb_version version = { 4, 9, 15, 10 };
    return version;
}

/*! Delete all temporary test files in the current directory that begin with the given prefix.

@return None
*/
void 
DeleteTemporaryTestFiles(PAL_Instance* pal, const char* fileNamePrefix)
{
    PAL_Error err = PAL_Ok;
    PAL_FileEnum* fileEnum;

    err = PAL_FileEnumerateCreate(pal, ".", TRUE, &fileEnum);
    if (!err)
    {
        PAL_FileEnumInfo info = { 0 };

        do
        {
            err = PAL_FileEnumerateNext(fileEnum, &info);
            if (!err)
            {
                if ((info.attributes & PAL_FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    if (nsl_strncmp(info.filename, fileNamePrefix, nsl_strlen(fileNamePrefix)) == 0)
                    {
                        PAL_FileRemove(pal, info.filename);
                    }
                }
            }
        } while (!err);

        PAL_FileEnumerateDestroy(fileEnum);
    }
}

int LoadFile(PAL_Instance* pal, const char* directory, const char* name, nb_size name_len, char** pbuf, nb_size* psize)
{
    unsigned char*  file_data = NULL;
    size_t			file_length = 0;
    char			filename[255] = {0};
    char			nametmp[64];

    if (directory)
    {    
        nsl_strncpy(nametmp, name, name_len);
        nsl_strlcpy(filename, directory, sizeof(filename));

#if defined(WINCE) || defined(WIN32) || defined(__APPLE__)
        nsl_strcat(filename, "/");
        nsl_strcat(filename, nametmp);
#else
        nsl_strlcat(filename, "/", sizeof(filename));
        nsl_strlcat(filename, nametmp, sizeof(filename));
#endif
    }
    else
    {
        nsl_strlcpy(filename, name, name_len);
    }

    PlatformLoadFile(pal, filename, &file_data, (uint32*)&file_length);

    if (file_length <= 0)
    {
        return 0;
    }

    *pbuf  = (char*)	file_data;
    *psize = (nb_size)	file_length;    

    return file_length;
}


/*! Get the next GPS fix from a (file)buffer.

This function reads GPS time, latitude and logitude from a string buffer. The
values have to be comma-seperated (CSV file) and appear on one line in the form:

"GPSTime,Latitude,Longitude"

@return TRUE if GPS fix was found, FALSE otherwise
*/
nb_boolean
GetNextGpsFix(char** current,       /*!< Current position in char buffer to search for. Will be updated on return
                                    to point to the next GPS position. */
                                    int* gpsTime,         /*!< Returned GPS time in seconds */
                                    double* latitude,     /*!< Returned latitude */
                                    double* longitude     /*!< Returned longitude */
                                    )
{
    // Find first seperator
    char* position = *current;
    if (!position)
    {
        return FALSE;
    }

    // Convert GPS time
    *gpsTime = nsl_atoi(position);

    // Find first seperator
    position = nsl_strchr(position, SEPERATOR);
    if (!position)
    {
        return FALSE;
    }

    // Convert latitude
    *latitude = nsl_atof(position + 1);

    // Find next seperator
    position = nsl_strchr(position + 1, SEPERATOR);
    if (!position)
    {
        return FALSE;
    }

    // Convert longitude
    *longitude = nsl_atof(position + 1);

    // Find end of line
    position = nsl_strchr(position, '\n');
    if (!position)
    {
        // If we didn't find the end of line then we still return TRUE but set the
        // position to NULL. That way the next call will fail.
        *current = NULL;
        return TRUE;
    }

    // Set position to the first character after the end of line.
    *current = position + 1;
    return TRUE;
}

/*! Convert the GPSFileRecord data in to NBI_GpsLocation

This method is used to convert the GPSFileRecord in to GPSFile Location,
which is then used for Navigation as a new fix

@return none
*/
void
GPSInfo2GPSFix(GPSFileRecord* pos, NB_GpsLocation* cur)
{
    cur->valid = pos->fValid;
    cur->gpsTime = pos->dwTimeStamp;
    cur->status = pos->status;

    /* Convert the values stored in a standard NIM GPS File (BREW AEEGPSInfo Structures
       into standard units.  See the BREW SDK documentation and the Qualcomm GPSOne
       Developer Guide for more information */

    cur->status = pos->status;
    cur->gpsTime = pos->dwTimeStamp;
    cur->valid = pos->fValid;
    cur->latitude = pos->dwLat * 0.00000536441803;
    cur->longitude = pos->dwLon * 0.00000536441803;;
    cur->heading = pos->wHeading * 0.3515625;
    cur->horizontalVelocity = pos->wVelocityHor * 0.25;
    cur->altitude = pos->wAltitude - 500.0;
    cur->verticalVelocity = pos->bVelocityVer * 0.5;
    cur->horizontalUncertaintyAngleOfAxis = pos->bHorUncAngle * 5.625;
    cur->horizontalUncertaintyAlongAxis = UncDecode(pos->bHorUnc);
    cur->horizontalUncertaintyAlongPerpendicular = UncDecode(pos->bHorUncPerp);
    cur->verticalUncertainty = pos->wVerUnc;
    cur->utcOffset = 0;
    cur->numberOfSatellites = 0;

#if 0 /* This code is obsolete */
    if ((~cur->valid & (AEEGPS_VALID_AUNC | AEEGPS_VALID_HUNC | AEEGPS_VALID_PUNC)) == 0 &&
        cur->horizontalAccuracy > 0 && cur->horizontalUncertaintyPerpendicular > 0) {
            /*
            * XXX: Ideally, uncmag would be
            *   (uncaxis ** 2 + uncperp ** 2) ** .5   ,
            * but we want to avoid importing a square root function right now. A possible workaround is to
            * take the max of (uncaxis, uncperp) and multiply it by the square root of two, giving the
            * worst-case value (it's effectively assuming that uncaxis and uncperp are both equal to their
            * maximum), but we expect that the uncertainty ellipse to be significantly lopsided in one
            * direction (i.e., one of the values is much larger than the other). So far, we're just taking
            * the maximum and are relying on the maxerr/caperr parameters to sort out the mess.
            */
            // @TODO: Replacement for uncmag
            //cur->uncmag = max(cur->uncaxis, cur->uncperp);
    } else
    {
        // @TODO: Replacement for uncmag
        // cur->uncmag = -1.0;
    }
#endif
}

/*! This method is used for decoding purpose

for gpsfix from the Array defined.

@return double
*/
double
UncDecode(uint8 unc)
{
    if (unc < sizeof(unctbl) / sizeof(*unctbl))
        return (unctbl[unc]);
    else
        return (-2.0);
}

/*! This method is used for encoding purpose

for gpsfix from the Array defined.

@return uint8
*/
uint8
UncEncode(double v)
{
    uint8 n;

    for (n=0;n<(sizeof(unctbl) / sizeof(*unctbl));n++)
        if (v == unctbl[n])
            return n;

    return n-1;
}


/*! Max GPS fixes to be read from file */
int GetMaxFixes()
{
    if (Test_OptionsGet()->logLevel == LogLevelHigh)
    {
        return 2000;   //For High level logging use 2000 fixes
    }

    return 200;    //For low level logging use 200 fixes only
}

/*! @} */
